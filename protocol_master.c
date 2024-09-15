/**
 * @brief Protocol Master Implementation
 * 
 * Author: Novel Alam
 */

#include "protocol.h"

// Global Variables
volatile uint8_t rx_buffer[BUFFER_SIZE];
volatile uint8_t rx_index = 0;

/**
 * @brief Initialize the master protocol with a given address.
 * 
 * @param address The address of the master device.
 */
void Protocol_Master_Init(uint8_t address) {
    /* Initialize GPIO, Timer, and Interrupts for Master */
    GPIO_Init();
    Timer_Init();
    Interrupt_Init();
}

/**
 * @brief Send a byte of data from the master.
 * 
 * @param data The byte of data to be sent.
 */
void Protocol_Master_SendByte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        if (data & 0x80) {
            /* Send '1' (TX_H = HIGH, TX_L = LOW) */
            GPIOA->BSRR = GPIO_BSRR_BS0;   // Set TX_H high
            GPIOA->BSRR = GPIO_BSRR_BR1;   // Set TX_L low
        } else {
            /* Send '0' (TX_H = LOW, TX_L = HIGH) */
            GPIOA->BSRR = GPIO_BSRR_BR0;   // Set TX_H low
            GPIOA->BSRR = GPIO_BSRR_BS1;   // Set TX_L high
        }
        data <<= 1;
        clock_pulse();
    }
}

/**
 * @brief Send an address from the master.
 * 
 * @param address The address to be sent.
 */
void Protocol_Master_SendAddress(uint8_t address) {
    Protocol_Master_SendByte(address);
}

/**
 * @brief Perform bus arbitration as a master.
 */
void Protocol_Master_Arbitrate(void) {
    /* Implement arbitration logic similar to CAN */
    handle_bus_arbitration(MASTER_ADDRESS);
}

/**
 * @brief Read a byte of data from the master.
 * 
 * @return The received byte of data.
 */
uint8_t Protocol_Master_ReadByte(void) {
    return rx_buffer[rx_index - 1];
}

/**
 * @brief Protocol interrupt handler.
 */
void Protocol_IRQHandler(void) {
    /* Clock Edge Interrupt Handler logic (already implemented) */
    EXTI2_IRQHandler();
}

// Private Functions

/**
 * @brief Initialize GPIO ports for communication.
 */
void GPIO_Init(void) {
    /* Enable GPIOA clock */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    
    /* Configure PA0 (TX_H) and PA1 (TX_L) as Output Push-Pull */
    GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_MODE1);
    GPIOA->CRL |= (GPIO_CRL_MODE0_1 | GPIO_CRL_MODE1_1);  // Output 2 MHz
    GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_CNF1);
    
    /* Configure PA2 (Clock) as Output Push-Pull */
    GPIOA->CRL &= ~GPIO_CRL_MODE2;
    GPIOA->CRL |= GPIO_CRL_MODE2_1;  // Output 2 MHz
    GPIOA->CRL &= ~GPIO_CRL_CNF2;
    
    /* Enable GPIOB clock for RX_H and RX_L */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    
    /* Configure PB0 (RX_H) and PB1 (RX_L) as Input Floating */
    GPIOB->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_MODE1);
    GPIOB->CRL |= (GPIO_CRL_CNF0_0 | GPIO_CRL_CNF1_0);  // Input floating
}

/**
 * @brief Initialize the timer for timing operations.
 */
void Timer_Init(void) {
    /* Enable TIM2 clock */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    
    /* Configure TIM2 */
    TIM2->PSC = 7200 - 1;  // Prescaler (assuming 72 MHz clock, 10 kHz timer)
    TIM2->ARR = 1000 - 1;  // Auto-reload for 10 ms period
    TIM2->CR1 |= TIM_CR1_CEN;  // Enable the timer
}

/**
 * @brief Initialize interrupts for protocol handling.
 */
void Interrupt_Init(void) {
    /* Enable AFIO clock */
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    
    /* Configure PA2 as EXTI2 (rising edge) for clock signal */
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI2_PA;  // Select PA2 as source
    EXTI->IMR |= EXTI_IMR_MR2;  // Unmask EXTI2 interrupt
    EXTI->RTSR |= EXTI_RTSR_TR2;  // Rising edge trigger
    NVIC_EnableIRQ(EXTI2_IRQn);  // Enable EXTI2 interrupt in NVIC
}

/**
 * @brief Generate a clock pulse for synchronization.
 */
void clock_pulse(void) {
    GPIOA->BSRR = GPIO_BSRR_BS2;  // Set Clock pin high
    for (volatile int i = 0; i < 100; i++);  // Short delay
    GPIOA->BSRR = GPIO_BSRR_BR2;  // Set Clock pin low
    for (volatile int i = 0; i < 100; i++);  // Short delay
}

/**
 * @brief Handle bus arbitration based on address.
 * 
 * @param address The address to be used for arbitration.
 */
void handle_bus_arbitration(uint8_t address) {
    /* Send address, but monitor the bus to detect collisions */
    for (int i = 0; i < 8; i++) {
        uint8_t bit = (address & 0x80) >> 7;
        
        /* Send the current bit via differential signaling */
        if (bit) {
            GPIOA->BSRR = GPIO_BSRR_BS0;   // TX_H high
            GPIOA->BSRR = GPIO_BSRR_BR1;   // TX_L low
        } else {
            GPIOA->BSRR = GPIO_BSRR_BR0;   // TX_H low
            GPIOA->BSRR = GPIO_BSRR_BS1;   // TX_L high
        }
        
        /* Clock pulse for arbitration */
        clock_pulse();
        
        /* Read the bus state to detect if another master is sending a lower address */
        uint8_t rx_high = GPIOB->IDR & GPIO_IDR_IDR0;
        uint8_t rx_low = GPIOB->IDR & GPIO_IDR_IDR1;
        
        if ((bit == 0 && rx_high) || (bit == 1 && !rx_high)) {
            /* Another master is sending a lower priority bit, give up the bus */
            break;
        }
        
        address <<= 1;
    }
}
