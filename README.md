# **Protocol Library Documentation**

**Version**: 2.5  
**Author**: Novel Alam
---
## 1. Introduction

### Purpose

This document outlines the design of a comprehensive serial communication protocol that integrates the most effective features of SPI, I2C, and CAN. The protocol is designed to support robust multi-master, full-duplex communication in embedded systems, ensuring efficient data transfer among devices while maintaining high reliability and performance.

### Scope

This protocol is intended for applications requiring reliable communication between multiple microcontrollers, particularly in IoT and automation environments. It addresses the need for effective data transmission with minimal wiring and enhanced noise immunity.

---

## 2. Protocol Overview

### Architecture

The protocol utilizes a multi-master architecture, allowing multiple master devices to initiate communication with one or more slave devices. Each device is identified by a unique 10-bit address, facilitating selective data transfer.

### Addressing

- **10-Bit Addressing Scheme**: Each device on the bus is assigned a unique 10-bit address. Devices will only respond to data addressed to them, minimizing unnecessary communication.

---

## 3. Signal Design

### Clock Signal

- **Generation**: A designated master will be responsible for sending out a periodic clock pulse, providing synchronization for all devices on the bus. One of the masters will adjust the baud rate, ensuring all devices operate at the same communication speed.

### Data Transmission

- **TX Line**: 
  - Shared by all master devices for transmitting data, including addresses.
  - Connected to all slave devices to receive data.
- **RX Line**:
  - Used by slave devices to transmit data back to the master upon selection.
  - Monitored by master devices to detect bus arbitration outcomes.

---

## 4. Communication Flow

### Bus Arbitration

- **Mechanism**: When multiple masters attempt to send data simultaneously, bus arbitration is handled by comparing signal levels. A logic high from one master combined with a logic low from another indicates the loss of bus arbitration for the latter. The master that maintains a high signal continues communication, while the others cease transmission.

### Data Transfer Process

1. **Master Initiates Communication**: The master sends a 10-bit address followed by the data on the TX line.
2. **Slave Response**: Devices with matching addresses respond, enabling efficient data communication.

---

## 5. Interrupt Handling

### Rising Edge Interrupts

- **Interrupts are triggered on the rising edge of the clock signal for**:
  - **Masters**: Sampling the TX line to read incoming data.
  - **Slaves**:
    - Sending data bits if available.
    - Receiving data bits from the TX line upon selection.

### Data Flow Explanation

1. **Master Configuration**: The designated master sets the clock signal and monitors the TX line continuously for data transmission and arbitration.
2. **Transmission**: When the master sends data on the TX line, it also sends the 10-bit address. Slaves check this address and respond only if it matches their assigned address.
3. **Slave Transmission**: Selected slaves use the RX line to send data back to the master during their turn.

---

## 6. Unique Signaling Sequences

- **Start Sequence**: `11001` (binary)  

---

## 7. Differential Signaling

The protocol employs differential signaling for both TX and RX lines to enhance noise immunity and improve overall communication reliability, crucial for maintaining data integrity in noisy environments.

---

## 8. Implementation Considerations

### Responsibility of the Designated Master

- The designated master must maintain the periodic clock pulse. This involves adjusting the baud rate as needed to ensure all devices are synchronized.
## **Features**

1. **Master-Minion Architecture**:
   - **Master Mode**: Initiates communication, sends data, performs address-based arbitration.
   - **Minion Mode**: Receives data, processes incoming messages, and can send data back.

2. **Full-Duplex Communication**:
   - Allows simultaneous sending and receiving of data, improving system efficiency.

3. **Address-Based Communication**:
   - Supports address-based messaging, enabling specific communication between master and minions.

4. **Differential Signaling**:
   - Uses differential pairs (similar to CAN) for extended range and noise immunity.

5. **Multi-Master Support**:
   - Implements bus arbitration to allow multiple masters to share the communication bus, based on address priority.

6. **Interrupt-Driven Reception**:
   - Uses interrupts for efficient data reception at clock rising edges, with buffering to handle incoming data.

7. **Customizable Timing**:
   - Timer configurations can be adjusted based on system requirements.



## Master Functions

### 1. Master_Init(bool clockMaster, uint32_t baudRate)

- **Description**: Initializes the master communication protocol, setting up the necessary GPIO pins for TX, RX, and CLK, and configuring interrupts for clock edges. If this instance is the clock master, it sets the clock speed.
- **Parameters**:
  - `clockMaster`: Boolean indicating if this instance is the clock master.
  - `baudRate`: The baud rate for communication.
- **Note**: Call this function at the start to initialize the master.

### 2. Master_SetClockSpeed(uint32_t baudRate)

- **Description**: Sets the clock speed by adjusting the timer’s period to match the desired baud rate.
- **Parameters**:
  - `baudRate`: The baud rate for communication.
- **Note**: Only relevant if the master is the clock master.

### 3. Master_StartTransmission(uint8_t* txData, uint8_t* rxData, uint16_t address, uint8_t dataLength)

- **Description**: Starts the transmission process, setting up the data and address for communication with the selected minion.
- **Parameters**:
  - `txData`: Pointer to the data to transmit.
  - `rxData`: Pointer to the buffer where received data will be stored.
  - `address`: Address of the target minion.
  - `dataLength`: Length of the data to transmit (in bits).
- **Note**: This function prepares the master to send data to the selected minion.

### 4. Master_Transmission_ISR(void)

- **Description**: Manages the transmission logic on each clock edge. This function handles state transitions between sending the start sequence, address, data, and the stop sequence.
- **Note**: This function should be called inside the interrupt handler for clock edge events.

### 5. Master_Arbitration(void)

- **Description**: Checks bus arbitration to determine if the master has won or lost control of the bus.
- **Note**: If arbitration is lost, the master must stop the transmission.

### 6. Master_LoadData(uint8_t* data, uint8_t length)

- **Description**: Loads up to 8 bits of data into the transmission buffer for sending.
- **Parameters**:
  - `data`: Pointer to the data to load.
  - `length`: Number of bits to load (maximum 8 bits).
- **Note**: This data will be loaded for future transmission.

### 7. Master_ReadOldData(uint8_t* buffer, uint8_t length)

- **Description**: Reads data from the old data buffer in case of overflow.
- **Parameters**:
  - `buffer`: Pointer to the buffer where old data will be stored.
  - `length`: Number of bits to read from the old buffer.
- **Note**: Use this function to retrieve data in case of buffer overflow.

## Minion Functions

### 1. Minion_LoadData(uint8_t* data, uint8_t length)

- **Description**: Loads up to 8 bits of data into the transmission buffer.
- **Parameters**:
  - `data`: Pointer to the data to load.
  - `length`: Number of bits to load (maximum 8 bits).
- **Note**: This data will be transmitted in response to a master’s request.

### 2. Minion_ReadOldData(uint8_t* buffer, uint8_t length)

- **Description**: Reads data from the old data buffer in case of overflow.
- **Parameters**:
  - `buffer`: Pointer to the buffer where old data will be stored.
  - `length`: Number of bits to read from the old buffer.
- **Note**: This function retrieves data from the overflow buffer if it is full.

## Example Code Usage

### Master Example

```c
#include "master.h"

int main(void) {
    uint8_t txData[8] = {0xAA};   // Data to send
    uint8_t rxData[8] = {0};      // Buffer for received data
    uint16_t minionAddress = 0x12; // Address of the minion
    uint8_t dataLength = 8;       // Data length in bits
    
    // Initialize the master (not the clock master)
    Master_Init(false, 9600);
    
    // Start transmission
    Master_StartTransmission(txData, rxData, minionAddress, dataLength);
    
    // Main loop
    while (1) {
        // Handle communication logic
    }
}

// Clock edge interrupt handler (if using STM32 HAL)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_3) {
        Master_Transmission_ISR();  // Handle transmission on clock edge
    }
}
