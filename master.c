#include "master.h"
#include <string.h>
#include "stm32f4xx_hal.h"

#define START_SEQUENCE 0x19    /* 11001 binary */

static MasterState masterState = MASTER_IDLE;  /* Current state of the master */
static uint8_t txDataBuffer[8];                 /* Data to send */
static uint8_t rxDataBuffer[8];                 /* Data to receive */
static uint8_t bitPosition = 0;                  /* Position of the current bit being transmitted */
static uint16_t masterAddress = 0;               /* Address of the slave */
static uint8_t dataLength = 0;                   /* Length of the data to send */
static bool isClockMaster = false;                /* Flag to indicate if this is the clock master */
static bool arbitrationLost = false;              /* Flag to indicate arbitration loss */
static uint8_t oldDataBuffer[1024]; /* Old data buffer for overflow */
static uint8_t oldDataIndex = 0;    /* Index for old data buffer */

/* Pin positions for TX and RX differential pairs */
#define TX_POS_PIN 6     /* TX positive differential on GPIOB Pin 6 */
#define TX_NEG_PIN 7     /* TX negative differential on GPIOB Pin 7 */
#define RX_POS_PIN 4     /* RX positive differential on GPIOB Pin 4 */
#define RX_NEG_PIN 5     /* RX negative differential on GPIOB Pin 5 */
#define CLK_PIN 3        /* CLK on GPIOB Pin 3 */

void Master_Init(bool clockMaster, uint32_t baudRate) {
    isClockMaster = clockMaster;  /* Set the clock master status */

    /* Initialize TX, RX, and CLK pins as GPIO */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = MASTER_TX_PIN | MASTER_RX_PIN | MASTER_CLK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Configure the clock pin for external interrupt on rising edge */
    GPIO_InitStruct.Pin = MASTER_CLK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;  /* Trigger on rising edge */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Enable and set EXTI line interrupt priority */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);  // Adjust priority as needed
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);           // Adjust line number for your setup

    /* If this is the clock master, set up a timer for clock generation */
    if (isClockMaster) {
        Master_SetClockSpeed(baudRate);
        HAL_TIM_Base_Start_IT(&htim2);  /* Start timer with interrupts enabled */
    }
}

void Master_SetClockSpeed(uint32_t baudRate) {
    /* Set the clock speed for communication by adjusting the timer's period */
    uint32_t timerPeriod = (HAL_RCC_GetPCLK1Freq() / baudRate) - 1;
    __HAL_TIM_SET_AUTORELOAD(&htim2, timerPeriod);
}

void Master_StartTransmission(uint8_t* txData, uint8_t* rxData, uint16_t address, uint8_t dataLength) {
    strncpy((char*)txDataBuffer, (const char*)txData, dataLength); /* Copy data to be sent */
    memset(rxDataBuffer, 0, sizeof(rxDataBuffer));  /* Clear receive buffer */
    masterAddress = address;  /* Set the address for transmission */
    bitPosition = 0;  /* Reset bit position */
    arbitrationLost = false;  /* Reset arbitration loss flag */
    masterState = MASTER_START_SEQUENCE;  /* Set state to start sequence */
    dataLength = dataLength;  /* Set length of data to send */
}

inline void Master_Clock_Handle(void) {
    uint8_t receivedBitPositive = (GPIOB_IDR >> RX_POS_PIN) & 0x01;  /* Read positive RX line */
    uint8_t receivedBitNegative = (GPIOB_IDR >> RX_NEG_PIN) & 0x01;  /* Read negative RX line */

    if (receivedBitPositive == receivedBitNegative) {
        minionState = MINION_IDLE; /* Stop condition */
    }
    uint8_t receivedBit = receivedBitPositive;

    switch (masterState) {
        case MASTER_START_SEQUENCE:
            if (bitPosition < 5) {
                if (START_SEQUENCE & (1 << (4 - bitPosition))) {
                    Master_SendBit(1U);  /* Send bit '1' */
                } else {
                    Master_SendBit(0U);   /* Send bit '0' */
                }
                bitPosition++;
            } else {
                bitPosition = 0;
                masterState = MASTER_SEND_ADDRESS;  /* Move to address state */
            }
            break;

        case MASTER_SEND_ADDRESS:
            if (bitPosition < 10) {
                if (masterAddress & (1 << (9 - bitPosition))) {
                    Master_SendBit(1U);  /* Send bit '1' */
                    Master_Arbitration();  /* Check for arbitration loss */
                } else {
                    Master_SendBit(0U);   /* Send bit '0' */
                }
                bitPosition++;
            } else {
                bitPosition = 0;
                masterState = MASTER_SEND_RECEIVE_DATA;  /* Move to data state */
            }
            break;

        case MASTER_SEND_RECEIVE_DATA:
            if (bitIndex < 8) {
                uint8_t bitToSend = (txData[0] >> (7 - bitIndex)) & 0x01;
                Minion_SendBit(bitToSend);

                rxData[0] |= (receivedBit << (7 - bitIndex));
                bitIndex++;
            } else {
                if (oldDataIndex < 1024) {
                    oldDataBuffer[oldDataIndex++] = rxData[0];
                    masterState = MASTER_STOP_SEQUENCE;
                }
                memset(rxData, 0, sizeof(rxData));
                bitIndex = 0;
            }
            break;

        case MASTER_STOP_SEQUENCE:
            GPIOB_ODR |= (1 << RX_POS_PIN);   /* Set TX positive pin high */
            GPIOB_ODR |= (1 << RX_NEG_PIN);   /* Set TX negative pin high */
            break;

        case MASTER_ARB_LOST:
            masterState = MASTER_IDLE;
            break;

        case MASTER_IDLE:
        default:
            break;
    }
}

inline void Master_Arbitration(void) {
    if (HAL_GPIO_ReadPin(GPIOB, MASTER_ARB_PIN) == GPIO_PIN_RESET) {
        arbitrationLost = true;  /* Arbitration lost */
        masterState = MASTER_ARB_LOST;
    }
}

/* Loads up to 8 bits of data into the transmission buffer */
inline void Master_LoadData(uint8_t* data, uint8_t length) {
    if (length > 8) {
        length = 8;
    }
    memcpy(txDataBuffer, data, length);  /* Copy data to buffer */
}

/* Reads data from the old data buffer (overflow) */
inline void Master_ReadOldData(uint8_t* buffer, uint8_t length) {
    if (length > oldDataIndex) {
        length = oldDataIndex;
    }
    memcpy(buffer, oldDataBuffer, length);  /* Copy old data to buffer */
}