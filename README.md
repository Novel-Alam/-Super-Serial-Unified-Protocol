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
- **End Sequence**: `00110` (binary)

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

## **API Functions**

### **Master Functions**

- **`void Protocol_Master_Init(uint8_t address);`**
  - Initializes the protocol in master mode.
  - **Parameters**: 
    - `address`: The address of the master device.
  
- **`void Protocol_Master_SendByte(uint8_t data);`**
  - Sends a byte of data to the minion(s) using differential signaling.
  - **Parameters**: 
    - `data`: The byte of data to send.
  
- **`void Protocol_Master_SendAddress(uint8_t address);`**
  - Sends an address to select a specific minion.
  - **Parameters**: 
    - `address`: The address of the target minion.
  
- **`void Protocol_Master_Arbitrate(void);`**
  - Performs bus arbitration to ensure access to the bus.
  
- **`uint8_t Protocol_Master_ReadByte(void);`**
  - Reads a byte of data from the buffer.
  - **Returns**: The received byte of data.

### **Minion Functions**

- **`void Protocol_Minion_Init(uint8_t address);`**
  - Initializes the protocol in minion mode.
  - **Parameters**: 
    - `address`: The address of the minion device.
  
- **`uint8_t Protocol_Minion_ReadByte(void);`**
  - Reads a byte of data from the buffer.
  - **Returns**: The received byte of data.
  
- **`void Protocol_Minion_SendByte(uint8_t data);`**
  - Sends a byte of data to the master using differential signaling.
  - **Parameters**: 
    - `data`: The byte of data to send.
  
- **`void Protocol_Minion_ProcessData(void);`**
  - Processes incoming data after it has been received.

### **Interrupt Handlers**

- **`void Protocol_IRQHandler(void);`**
  - Handles EXTI interrupts triggered by the clock signal.
  - Processes incoming bits and updates the buffer.

## **Configuration and Setup**

### **Pin Definitions**

- **Master:**
  - `TX_H`: High signal pin for differential signaling (e.g., PA0).
  - `TX_L`: Low signal pin for differential signaling (e.g., PA1).
  - `CLK`: Clock signal pin (e.g., PA2).

- **Minion:**
  - `RX_H`: High signal pin for differential signaling (e.g., PB0).
  - `RX_L`: Low signal pin for differential signaling (e.g., PB1).

### **Buffer Size**

- The buffer size for received data is defined by `BUFFER_SIZE` and can be adjusted as needed.

### **Timing Configuration**

- The clock and timer settings are configured in `Timer_Init()` and can be customized based on system requirements.

## **Usage Example**

### **Master Example**

```c
#include "protocol.h"

int main(void) {
    Protocol_Master_Init(MASTER_ADDRESS);
    Protocol_Master_SendAddress(MINION_ADDRESS);

    while (1) {
        Protocol_Master_SendByte(0x55); // Example data
        for (volatile int i = 0; i < 1000000; i++);  // Delay
    }
}
```

### **Minion Example**

```c
#include "protocol.h"

int main(void) {
    Protocol_Minion_Init(MINION_ADDRESS);

    while (1) {
        uint8_t received_data = Protocol_Minion_ReadByte();
        // Process received data
        for (volatile int i = 0; i < 1000000; i++);  // Delay
    }
}
```

## **Interrupts and Handling**

- **EXTI2_IRQHandler**: Handles the rising edge of the clock signal, processes received bits, and updates the buffer.

## **Additional Notes**

- Ensure to adapt the GPIO and timer settings based on your specific STM32 microcontroller model and clock configuration.
- The library uses simple delay loops for timing. For more precise timing, consider using hardware timers or other mechanisms.

## **Troubleshooting**

- **Communication Issues**: Verify pin connections and check if differential signaling is correctly implemented.
- **Buffer Overflow**: Ensure that the buffer size is adequate for your application, and monitor buffer indices to avoid overflows.

---

This documentation provides a clear overview of the protocol library’s features and usage, helping users integrate and deploy the protocol in their applications effectively. If you need any additional details or modifications, let me know!
