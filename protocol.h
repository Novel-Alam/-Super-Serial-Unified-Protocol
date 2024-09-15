/**
 * @brief Protocol API header
 * 
 * Author: Novel Alam
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "protocol_config.h"

/**
 * @brief Initialize the master protocol with a given address.
 * 
 * @param address The address of the master device.
 */
void Protocol_Master_Init(uint8_t address);

/**
 * @brief Send a byte of data from the master.
 * 
 * @param data The byte of data to be sent.
 */
void Protocol_Master_SendByte(uint8_t data);

/**
 * @brief Send an address from the master.
 * 
 * @param address The address to be sent.
 */
void Protocol_Master_SendAddress(uint8_t address);

/**
 * @brief Perform bus arbitration as a master.
 */
void Protocol_Master_Arbitrate(void);

/**
 * @brief Read a byte of data from the master.
 * 
 * @return The received byte of data.
 */
uint8_t Protocol_Master_ReadByte(void);

/**
 * @brief Initialize the minion protocol with a given address.
 * 
 * @param address The address of the minion device.
 */
void Protocol_Minion_Init(uint8_t address);

/**
 * @brief Read a byte of data from the minion.
 * 
 * @return The received byte of data.
 */
uint8_t Protocol_Minion_ReadByte(void);

/**
 * @brief Send a byte of data to the master from the minion.
 * 
 * @param data The byte of data to be sent.
 */
void Protocol_Minion_SendByte(uint8_t data);

/**
 * @brief Process received data on the minion.
 */
void Protocol_Minion_ProcessData(void);

/**
 * @brief Protocol interrupt handler.
 */
void Protocol_IRQHandler(void);

#endif
