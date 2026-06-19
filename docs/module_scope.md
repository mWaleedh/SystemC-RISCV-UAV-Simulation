## RISC-V Model
### Purpose
Act as the central processing unit that fetches, decodes, and executes instructions provided to it based on the RISC-V ISA. It must be able to communicate with all other models in the system like UART, Memory, GPIO.

### Input Signals
- clk_i
- rst_i
- data_bus_i
- irq_ext_i
- irq_timer_i
- irq_sw_i

### Output Signals
- read_en_o
- write_en_o
- addr_bus_o
- data_bus_o

### Expected Behavior
- When system first starts, reset pin is set which resets all the register values and also sets the program counter to default value.
- On a rising_edge an instruction is fetched from memory based on value of program counter.
- The instruction is then decoded, and the required operations are performed (ALU, branch, memory access, etc).
- After execution the registers are updated and the program counter is also updated to the next instruction.
- If between execution an interrupt is called, the CPU halts the execution, saves the current context, and then handles the interrupt.

### Possible Faults to Test
- Try to load or store a 32-bit word from a memory address that is not a multiple of 4.
- Feeding the CPU an undefined opcode/instruction to ensure it correctly halts or raises an error rather than executing garbage logic.
- Writing a value to x0 register and verifying that later instructions still read 0 from it.

## UART Model
### Purpose
It is used for external communication between different devices such as CPU, terminal, microcontrollers, bluetooth adapters etc.

### Input Signals
- clk_i
- rst_i- gpio_pins_i
- rx_i
- data_bus_i

### Output Signals
- tx_o
- data_bus_o

### Expected Behavior
- Initially the reset bit is set so that both internal registers are reset, and tx pin is set to 1.
- When transmitting data, the tx pin is set to 0 for 1 cycle to signal the receiving end that data is about to be sent, and then bits are sent one by one through this pin. When all bits have been sent the tx pin is set to 1 again.
- When the rx pin sees 1 -> 0 transition it knows data is about to be transmitted, and after one cycle it starts reading all the data and sending it to the data_bus, discarding the parity bit (if any) and the final stop bit.

### Possible Faults to Test
- CPU attempting to read or write to an address that does not exist in memory space.
- CPU executing store instruction (SW) tries to write to a restricted address (for example an address having the compiled instructions).
- CPU throws a warning whenever it tries to read from an address that hasn't yet been initialized.

## GPIO Model
### Purpose- gpio_pins_i
This is the General Purpose Input Output model which allows the CPU to read or write/control external hardware such as sensors, leds, buttons, etc. This is used for devices that don't need complex protocols like UART.

### Input Signals
- clk_i
- rst_i
- write_en_i
- read_en_i
- addr_bus_i
- data_bus_i
- gpio_pins_i

### Output Signals
- data_bus_o
- gpio_pins_o

### Expected Behavior
- Contains a registers that are used to set pins as input or output and data registers that are used to hold pin values.
- When set as an Output, the CPU writes 1 or 0 to the Data Register using bus, which sets the physical gpio_pins_o to high or low.
- When set as an Input, the physical voltage on gpio_pins_i is stored. The CPU can then read the Data Register at any time to see the state of pins.

### Possible Faults to Test
- Reading a pin that is configured for output, or writing to a pin that is configured for input.
- Changing the configuration of a pin from input to output (or vice versa) at the exact same cycle the external device sends (or receives) something through that pin.

## Timer/PWM Model
### Purpose
This module is used to keep track of system time, generate timer interrupts and control the power delivered to devices by rapidly switching between ON and OFF signals. This technique is called PWM (Pulse Width Modulation).

### Input Signals
- clk_i
- rst_i
- write_en_i
- read_en_i
- addr_bus_i
- data_bus_i

### Output Signals
- data_bus_o
- pwm_o
- irq_timer_o

### Expected Behavior
- CPU first sets the target limit and a duty cycle, then the timer is enabled.
- The internal counter increments on every clock cycle.
- In Timer mode when the counter matches the target limit, it triggers irq_timer_o so the CPU knows how much time has passed.
- In PWM mode this module sets the pwm_o pin to high or low based on the set duty cycle, providing variable power to the motor.

### Possible Faults to Test
- CPU setting the counter limit or PWM period to 0 to make sure it doesn't cause a zero division error or infinite loop.
- Changing the PWM duty cycle from 0% to 100% instantly, to ensure that the signal goes high without dropping.

## Memory Model
### Purpose
This is the main storage that stores the process instructions and all the program data such as variables, stack, heap, etc. It directly communicates with the CPU using the address_bus and the data_bus.

### Input Signals
- clk_i
- rst_i
- write_en_i
- read_en_i
- addr_bus_i
- data_bus_i

### Output Signals
- data_bus_o

### Expected Behavior
- First the machine code generated by assembler is loaded into memory.
- When read_en is set to 1, it looks at the address_bus for the target address. It then goes to that address and fetches whatever is stored in that address and sends it over the data_bus.
- When write_en is set to 1, it first looks at the address_bus for the target address, it then looks at the data_bus for the data to be stored. It finally goes to the target address and stores this data.

### Possible Faults to Test
- CPU attempting to read or write to an address that does not exist in memory space.
- CPU executing store instruction (SW) tries to write to a restricted address (for example an address having the compiled instructions).
- CPU throws a warning whenever it tries to read from an address that hasn't yet been initialized.

## Interrupt Model
### Purpose
This module manages all the interrupt requests coming from UART, GPIO, and other hardware devices. It first collects the interrupt requests, it then prioritizes them, and then finally sends them over to the CPU using the external interrupt pin.

### Input Signals
- clk_i
- rst_i
- write_en_i
- read_en_i
- addr_bus_i
- data_bus_i
- irq_uart_i
- irq_gpio_i
- irq_timer_i

### Output Signals
- data_bus_o
- irq_ext_o

### Expected Behavior
- Monitors all incoming interrupt lines from peripherals (UART, GPIO, other external hardware devices).
- Contains control registers that allow the CPU to ignore specific interrupts, called masking.
- If an unmasked device asserts an interrupt, it raises irq_ext_o.
- The CPU reads this status register to find out who caused the interrupt, then handles the interrupt, and then writes to a register to clear the flag.

### Possible Faults to Test
- An external device continuously keeps its interrupt pin high, to see if the CPU gets trapped in an infinite interrupt loop.
- Two interrupts are asserted at the exact same cycle to make sure the controller forwards the most important one first.

## Control Logic Model
### Purpose
This acts as the decision maker for the physical hardware. It takes the CPU's software commands and the physical motor outputs (and evaluates them using environmental sensors), and then takes decisions that prioritize system safety (for example deciding whether to move, hold, avoid, or execute an emergency stop).

### Input Signals
- clk_i
- rst_i
- control_command_i
- sensor_status_i
- obstacle_flag_i
- system_status_i

### Output Signals
- pwm_command_o
- motor_enable_o
- decision_o
- safety_flag_o

### Expected Behavior
- It reads the control_command_i, verifies that the path is clear, sets motor_enable_o to 1, and converts that command into a specific pwm_command_o to drive the system.
- If obstacle_flag_i is high, the module immediately overrides the standard control_command_i. It updates decision_o to an "avoid" or "stop" state, and sets motor_enable_o to 0, and then sets the safety_flag_o to 1.
- If system_status_i reports a critical error, it forces the entire system into a "hold" or "safe" state regardless of what the CPU is commanding.

### Possible Faults to Test
- Setting the control_command_i to maximum speed while setting the obstacle_flag_i to 1 at the same time, to ensure that the hardware prioritizes safety and halts the motors.
- Sending a normal control_command_i while setting the sensor_status_i to an invalid or unknown state, to test if the system sets itself to a fallback state.

## Motor/System Response Model
### Purpose

### Input Signals
- clk_i
- rst_i
- pwm_i
- motor_enable_i

### Output Signals
- sensor_status_o
- obstacle_flag_o

### Expected Behavior
- When motor_enable_i is high, it reads pwm_i and slowly increases the internal speed. When enable drops to 0, the speed decreases back to zero.
- Based on the internal speed, it toggles the encoder_ticks_o pin (faster speed = faster toggling).
- As the motor moves, this module changes the environment. For example, after 1000 clock cycles of moving forward, it can set obstacle_flag_o to high inorder to force the Control Logic to react.

### Possible Faults to Test
- Randomly setting the obstacle_flag_o for just a single clock cycle to see if the Control Logic has proper sensor glitch handling, or if it instantly stops the system.