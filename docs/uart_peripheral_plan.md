### Overview
This module implements a memory-mapped Universal Asynchronous Receiver-Transmitter (UART). Instead of setting physical pins, it sends memory writes to a specific address and prints the data directly to the terminal. This provides a simple console output mechanism for the RISC-V processor.

### Memory Map
The UART is allocated a specific memory region outside of the RAM address space.

- UART_TX_DATA: 0x10000020 | Write-Only | Transmit Data Register. Writing a byte here queues it for transmission.
UART_STATUS: 0x10000024 | Read-Only | Status Register. Indicates the whether the peripheral is ready or not.

### Register Details
- **UART_TX_DATA [7:0]:** The 8-bit ASCII character to be transmitted. The upper 24 bits of a 32-bit write are ignored.
- **UART_STATUS [0]:** TX_READY flag. 
    * 1 = UART is idle and ready to accept a new character.
    * 0 = UART is currently busy transmitting. 

### Flow
- **CPU Write:** The CPU executes a Store Byte (sb) or Store Word (sw) instruction targeting address 0x10000020.
- **Bus Interception:** The Memory/System Bus detects that the target address falls within the UART memory map instead of standard RAM.
- **Peripheral Action:** The bus routes the data to the UART module.
- **Terminal Output:** The UART module casts the lowest 8 bits to a char and prints it to the console.

### Current Limitations (Phase 1)
To keep the initial implementation lightweight and focused on CPU output verification:
* **No RX Logic:** The CPU cannot receive input from the user/terminal yet.
* **No Interrupts:** No external interrupt is generated upon transmission completion.