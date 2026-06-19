# Week #1
## Work Completed
- Learned basics of SystemC.
- Wrote example programs (basic_adder.cpp, clocked_counter.cpp, producer_consumer.cpp) to get comfortable with SystemC.
- Wrote systemc_basic_notes.md, which contains notes about all things I learned this week related to SystemC.
- Wrote module_scope.md defining the purpose, input/output ports, expected behavior, and faults to test in the future for all modules (RISC V, UART, GPIO, Memory, Timer/PWM, Interrupt Controller, Control Logic, Motor/System Response).
- Wrote bus_signal_plan.md, which contains detailed information about each input and output port of risc_v_model.
- Wrote basic structure of risc_v_model.cpp, containing all input/output ports, main thread containing the infinite loop, reset logic, and basic calls to IF/ID/EX/MEM/WB. 
- Wrote a basic testbench in sc_main for risc_v_model.cpp which connects all the input and output ports to signals, and runs the CPU for a few cycles.

## Examples Completed
- basic_adder.cpp
- clocked_counter.cpp
- producer_consumer.cpp

## Understand of SystemC Basics
### SC_MODULE
The foundational C++ class/container in SystemC, is the same as a VHDL entity. It stores all input/output ports, internal signals, and processes.

### sc_signal
The SystemC equivalent of a VHDL signal, used to represent physical wires connecting different modules' ports together. Its value updates at the end of a cycle, which aligns with real world hardware delays.

### sc_clock
A built in, configurable object created in the testbench to generate a clock signal. We can define its period, duty cycle, start time, and the initial value.

### SC_METHOD
A synthesizable process block used for pure combinational logic that executes instantly whenever a signal in its sensitivity list changes. Because it executes without any time delays, it cannot contain wait() statements.

### SC_THREAD
A non synthesizable process designed for asynchronous execution, it typically has an infinite loop inside it that allows delay commands like wait(). Because it cannot be converted to physical hardware, it is primarily used for writing testbenches.

## RISC-V Model Structure

## Problems Faced

## Plan for Next Week
