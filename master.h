/* 
 * master.h 
 * 
 * This header file contains the declarations for the Master communication protocol,
 * designed to facilitate serial communication between multiple microcontrollers 
 * in an embedded system. 
 */

#ifndef MASTER_H
#define MASTER_H

#include <stdint.h>
#include <stdbool.h>

/* @brief Enumeration of the master states */
typedef enum {
    MASTER_IDLE,
    MASTER_START_SEQUENCE,
    MASTER_SEND_ADDRESS,
    MASTER_SEND_RECEIVE_DATA,
    MASTER_STOP_SEQUENCE,
    MASTER_ARB_LOST,
    MASTER_ARB_WON
} MasterState;

/* @brief Initializes the master communication protocol.
 * 
 * @param clockMaster Indicates if this instance is the clock master.
 * @param baudRate The communication baud rate.
 * @note This function configures GPIO pins for TX, RX, and CLK.
 */
void Master_Init(bool clockMaster, uint32_t baudRate);

/* @brief Sets the clock speed for communication.
 * 
 * @param baudRate The desired baud rate for communication.
 * @note Adjusts the timer's period to set the clock speed.
 */
void Master_SetClockSpeed(uint32_t baudRate);

/* @brief Starts the transmission process.
 * 
 * @param txData Pointer to the data to be transmitted.
 * @param address The address of the target slave device.
 * @param dataLength The length of the data to send.
 * @note This function prepares the master to send data to the selected slave.
 */
void Master_StartTransmission(uint8_t* txData, uint8_t* rxData, uint16_t address, uint8_t dataLength);

/* @brief Handles the transmission logic based on the master state.
 * 
 * @note This function should be called on each clock edge trigger to manage
 *       the state transitions and data transmission.
 */
void Master_Transmission_ISR(void);

/* @brief Checks for bus arbitration.
 * 
 * @note This function determines whether the master has lost or won arbitration 
 *       based on the signals on the bus.
 */
void Master_Arbitration(void);

/* Loads data into the transmission buffer */
/**
 * @brief Loads up to 8 bits of data into the transmission buffer.
 * @param data Pointer to the data to load.
 * @param length Length of the data in bits (maximum 8 bits).
 * @note This data is loaded into a buffer for later transmission.
 */
void Master_LoadData(uint8_t* data, uint8_t length);

/* Reads data from the old data buffer (overflow) */
/**
 * @brief Reads data from the old data buffer, in case of buffer overflow.
 * @param buffer Pointer to the buffer where data will be stored.
 * @param length Length of data to read.
 * @note This function retrieves data from the overflow buffer if it is full.
 */
void Master_ReadOldData(uint8_t* buffer, uint8_t length);

#endif /* MASTER_H */