#ifndef MINION_H
#define MINION_H

#include <stdint.h>

/* Enum for Minion states */
typedef enum {
    MINION_IDLE,             /* Minion is idle */
    MINION_LISTEN_START,     /* Listening for start sequence */
    MINION_LISTEN_ADDRESS,   /* Listening for address */
    MINION_SELECTED,         /* Minion is selected for communication */
    MINION_SEND_RECEIVE_DATA /* Sending/receiving data */
} MinionState;

/* Initializes the Minion with a specific address */
/**
 * @brief Initializes the Minion with the specified address.
 * @param address 10-bit unique address assigned to the Minion.
 * @note This function configures GPIO pins and prepares the Minion for communication.
 */
void Minion_Init(uint16_t address);

/* External Interrupt handler for the clock pin */
/**
 * @brief Interrupt handler for rising edge on the clock pin.
 * @note This is triggered to handle the clock signal and invokes the clock handler.
 */
void EXTI3_IRQHandler(void);

/* ISR to handle clock rising edge for data reception/transmission */
/**
 * @brief Handles clock signal and manages data transfer between master and minion.
 * @note Listens for start sequences, addresses, and transmits/receives data bits.
 */
void Minion_Clock_Handle(void);

/* Sends a single bit on the TX line with differential signaling */
/**
 * @brief Sends a single bit using differential signaling.
 * @param bit The bit (0 or 1) to send on the TX line.
 * @note The function uses TX differential pairs to ensure noise-resistant transmission.
 */
void Minion_SendBit(uint8_t bit);

/* Loads data into the transmission buffer */
/**
 * @brief Loads up to 8 bits of data into the transmission buffer.
 * @param data Pointer to the data to load.
 * @param length Length of the data in bits (maximum 8 bits).
 * @note This data is loaded into a buffer for later transmission.
 */
void Minion_LoadData(uint8_t* data, uint8_t length);

/* Reads data from the old data buffer (overflow) */
/**
 * @brief Reads data from the old data buffer, in case of buffer overflow.
 * @param buffer Pointer to the buffer where data will be stored.
 * @param length Length of data to read.
 * @note This function retrieves data from the overflow buffer if it is full.
 */
void Minion_ReadOldData(uint8_t* buffer, uint8_t length);

#endif /* MINION_H */