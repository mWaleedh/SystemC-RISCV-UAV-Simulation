## clk_i
### Type
sc_in<<bool>>
### Purpose
This is the main clock that controls the entire system. In this RISC-V model, on every positive edge of this clock the main thread is called, and the full fetch, decode, execute, writeback pipeline is executed for the current instruction.

## rst_i
### Type
sc_in<<bool>>
### Purpose
This is the reset port of this system. When the system first starts, this pin is set to high, setting all the registers, flags, and busses to initial state (0). When it is set to high mid execution, the main thread halts everything and starts executing the reset section of code.

## data_bus_i
### Type
sc_in<sc_uint<32>>
### Purpose
Is a 32-but unidirectional data bus that carries all the information from the processor to memory, UART, or anyother outside module. It is used during the fetch stage to receive the next instruction to be executed. And it is used in the execute stage to receive any data that is needed for execution from memory, UART, etc.

## read_en
### Type
sc_out<bool>
### Purpose
This is a flag that is sent by the processor to memory. It is used to tell memory that the processor needs data, and it needs to look at the addr_bus_o to find the target address, and then send the data stored in that address using the data_bus_i.

## write_en
### Type
sc_out<bool>
### Purpose
This is a another flag that is send by the processor to memory. It tells memory that the processor is sending some data through the data_bus_o that needs to be stored at a target address, which is stored in the addr_bus_o.

## addr_bus_o
### Type
sc_out<sc_uint<32>>
### Purpose
This is the 32-bit unidirectional bus that carries target addresses from processor to memory. It is used during fetch stage for retrieving the next instruction to be executed, which is stored in the program counter. And it is used in execute stage when the CPU needs some data from memory using data_bus_i, or when it needs to store data at a particular address in memory using data_bus_o.

## data_bus_o
### Type
sc_out<sc_uint<32>>
### Purpose
This is a 32-bit unidirectional bus that carries data to memory from other modules such as memory, UART, etc. It is mostly used during the execute stage when it needs to store data into memory, or it needs to send something to an adapter or peripheral using UART.

## interrupt