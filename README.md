# **Protocol Library Documentation**

## **Overview**

The Protocol Library I created provides a robust and flexible serial communication protocol combining the features of SPI, I2C, and CAN. It supports a master-minion architecture with multi-master capability, full-duplex communication, address-based messaging, and differential signaling. This documentation outlines the features, functions, and usage of the library.

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
