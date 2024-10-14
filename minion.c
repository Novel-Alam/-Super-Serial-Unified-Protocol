#include "minion.h"
#include <string.h>
#include "stm32f4xx_hal.h"

/* Minion state and data buffers */
static MinionState minionState = MINION_IDLE;
static uint8_t txData[8] = {0};    /* Transmission data buffer (8 bytes) */
static uint8_t rxData[8] = {0};    /* Reception data buffer (8 bytes) */
static uint8_t oldDataBuffer[1024]; /* Old data buffer for overflow */
static uint8_t oldDataIndex = 0;    /* Index for old data buffer */
static uint8_t bitIndex = 0;        /* Tracks the current bit being transmitted/received */
static uint16_t minionAddress = 0x00; /* Unique 10-bit address for the minion */

/* Predefined start sequence for recognition */
static const uint8_t startSequence = 0b11001;

/* Pin positions for TX and RX differential pairs */
#define TX_POS_PIN 6     /* TX positive differential on GPIOB Pin 6 */
#define TX_NEG_PIN 7     /* TX negative differential on GPIOB Pin 7 */
#define RX_POS_PIN 4     /* RX positive differential on GPIOB Pin 4 */
#define RX_NEG_PIN 5     /* RX negative differential on GPIOB Pin 5 */
#define CLK_PIN 3        /* CLK on GPIOB Pin 3 */

/* Register base address for GPIOB */
#define GPIOB_ODR *(volatile uint32_t*)(0x40020414) /* Output data register for GPIOB */
#define GPIOB_IDR *(volatile uint32_t*)(0x40020410) /* Input data register for GPIOB */

/* Function to initialize the minion with its specific address */
inline void Minion_Init(uint16_t address) {
    minionAddress = address;
    minionState = MINION_IDLE;
    bitIndex = 0;

    /* GPIO initialization for TX, RX, and CLK */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure RX negative pin as input */
    GPIO_InitStruct.Pin = (1 << RX_NEG_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Configure RX positive pin as input */
    GPIO_InitStruct.Pin = (1 << RX_POS_PIN);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Configure TX positive pin as output */
    GPIO_InitStruct.Pin = (1 << TX_POS_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Configure TX negative pin as output */
    GPIO_InitStruct.Pin = (1 << TX_NEG_PIN);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* Configure CLK pin as input with external interrupt */
    GPIO_InitStruct.Pin = (1 << CLK_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;  /* Trigger on rising edge */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Enable the NVIC interrupt for the clock pin */
    HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);  /* Adjust priority as necessary */
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);
}

/* External Interrupt handler for the clock pin */
inline void EXTI3_IRQHandler(void) {
    if (__HAL_GPIO_EXTI_GET_IT(1 << CLK_PIN) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(1 << CLK_PIN);  /* Clear the interrupt flag */
        Minion_Clock_Handle();  /* Call the ISR */
    }
}

/* ISR for clock rising edge to handle listening for data and responding */
inline void Minion_Clock_Handle(void) {
    /* Read both the positive and negative differential signals for RX */
    uint8_t receivedBitPositive = (GPIOB_IDR >> RX_POS_PIN) & 0x01;  /* Read positive RX line */
    uint8_t receivedBitNegative = (GPIOB_IDR >> RX_NEG_PIN) & 0x01;  /* Read negative RX line */

    /* Check for differential integrity for RX */
    if (receivedBitPositive == receivedBitNegative) {
        minionState = MINION_IDLE; /* Stop condition*/
    }
    uint8_t receivedBit = receivedBitPositive;

    switch (minionState) {
        case MINION_IDLE:
            /* Idle, waiting for start sequence */
            break;

        case MINION_LISTEN_START:
            /* Listen for the start sequence */
            if (bitIndex < 5) {
                uint8_t expectedBit = (startSequence >> (4 - bitIndex)) & 0x01;
                if (receivedBit == expectedBit) {
                    bitIndex++;
                } else {
                    bitIndex = 0;  /* Reset if mismatch */
                }
            } else {
                bitIndex = 0;
                minionState = MINION_LISTEN_ADDRESS;
            }
            break;

        case MINION_LISTEN_ADDRESS:
            /* Listen for the address */
            if (bitIndex < 10) {
                /* Accumulate the address bits */
                minionAddress = (minionAddress << 1) | receivedBit;
                bitIndex++;
            } else {
                if (minionAddress == minionAddress) {
                    minionState = MINION_SELECTED;
                } else {
                    minionState = MINION_IDLE;  /* Address mismatch */
                }
                bitIndex = 0;
            }
            break;

        case MINION_SELECTED:
            /* Handle data transmission/reception */
            minionState = MINION_SEND_RECEIVE_DATA;
         
        case MINION_SEND_RECEIVE_DATA:
            if (bitIndex < 8) {
                /* Send a bit of data from txData on the TX line */
                uint8_t bitToSend = (txData[0] >> (7 - bitIndex)) & 0x01;
                Minion_SendBit(bitToSend);

                /* Receive the corresponding bit from the RX line */
                rxData[0] |= (receivedBit << (7 - bitIndex));
                bitIndex++;
            } else {
                /* Data transmission complete */
                /* Check if the RX buffer fills up */
                if (oldDataIndex < 1024) {
                    /* Transfer current data to old data buffer if it fills */
                    oldDataBuffer[oldDataIndex++] = rxData[0]; /* Store old data */
                }
                memset(rxData, 0, sizeof(rxData));  /* Clear receive buffer */
                bitIndex = 0;
            }
            break;
    }
}

/* Function to send a bit on the TX line with differential pairing */
inline void Minion_SendBit(uint8_t bit) {
    if (bit) {
        GPIOB_ODR |= (1 << TX_POS_PIN);   /* Set TX positive pin high */
        GPIOB_ODR &= ~(1 << TX_NEG_PIN);  /* Set TX negative pin low */
    } else {
        GPIOB_ODR &= ~(1 << TX_POS_PIN);  /* Set TX positive pin low */
        GPIOB_ODR |= (1 << TX_NEG_PIN);   /* Set TX negative pin high */
    }
}

/* Function to load data into txData after 8 bits */
inline void Minion_LoadData(uint8_t* data, uint8_t length) {
    if (length > 8) length = 8; /* Limit to 8 bits */
    memcpy(txData, data, length);  /* Copy data into txData */
}

/* Function to read old data from the old data buffer */
inline void Minion_ReadOldData(uint8_t* buffer, uint8_t length) {
    if (length > oldDataIndex) length = oldDataIndex; /* Limit to the size of oldDataBuffer */
    strncpy((char*)buffer, (char*)oldDataBuffer, length); /* Copy old data from oldDataBuffer to buffer */
}
