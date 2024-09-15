#ifndef PROTOCOL_CONFIG_H
#define PROTOCOL_CONFIG_H

#include "stm32f10x.h"

// Pin Definitions for Master and Minion
#define TX_H_PIN 0    // PA0 for TX_H
#define TX_L_PIN 1    // PA1 for TX_L
#define CLK_PIN  2    // PA2 for Clock
#define RX_H_PIN 0    // PB0 for RX_H
#define RX_L_PIN 1    // PB1 for RX_L

// Buffer Size
#define BUFFER_SIZE 256

// Addresses
#define MASTER_ADDRESS 0x12
#define MINION_ADDRESS 0x34

// Timing Constants (if needed)
#define CLOCK_DELAY 100

// Global Buffers
extern volatile uint8_t rx_buffer[BUFFER_SIZE];
extern volatile uint8_t rx_index;

#endif
