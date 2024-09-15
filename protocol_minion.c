#include "protocol.h"

// Global Variables for Minion
volatile uint8_t rx_buffer[BUFFER_SIZE];
volatile uint8_t rx_index = 0;
uint8_t minion_address = MINION_ADDRESS;

/**
 * @brief Initialize the minion protocol with a given address.
 * 
 * @param address The address of the minion device.
 */
void Protocol_Minion_Init(uint8_t address) {
    minion_address = address;
    GPIO_Init();
    Interrupt_Init();
}

/**
 * @brief Read a byte of data from the minion.
 * 
 * @return The received byte of data.
 */
uint8_t Protocol_Minion_ReadByte(void) {
    return rx_buffer[rx_index - 1];
}

/**
 * @brief Send a byte of data to the master from the minion.
 * 
 * @param data The byte of data to be sent.
 */
void Protocol_Minion_SendByte(uint8_t data) {
    /* Similar to Protocol_Master_SendByte, but called by the minion */
    Protocol_Master_SendByte(data);
}

/**
 * @brief Process received data on the minion.
 */
void Protocol_Minion_ProcessData(void) {
    /* Handle the received data in the buffer */
}

/**
 * @brief Protocol interrupt handler for the minion.
 */
void Protocol_IRQHandler(void) {
    /* Minion IRQ logic for receiving */
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
 * @brief Initialize interrupts for the minion protocol.
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
 * @brief EXTI2 interrupt handler for processing received data.
 */
void EXTI2_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PR2) {  // Check if EXTI2 is set
        EXTI->PR |= EXTI_PR_PR2;  // Clear the interrupt pending bit
        
        /* Read differential pair to determine received bit */
        uint8_t rx_high = GPIOB->IDR & GPIO_IDR_IDR0;
        uint8_t rx_low = GPIOB->IDR & GPIO_IDR_IDR1;
        uint8_t received_bit = (rx_high > rx_low) ? 1 : 0;
        
        /* Store the received bit in the buffer */
        rx_buffer[rx_index / 8] <<= 1;
        rx_buffer[rx_index / 8] |= received_bit;
        
        /* Increment buffer index */
        rx_index++;
        
        /* Reset index if buffer is full */
        if (rx_index >= BUFFER_SIZE * 8) {
            rx_index = 0;
        }
        
        Protocol_Minion_ProcessData();
    }
}
