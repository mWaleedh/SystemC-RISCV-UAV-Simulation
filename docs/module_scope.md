RISC-V Model
purpose
Act as the central processing unit that fetches, decodes, and executes instructions provided to it based on the RISC-V ISA. It must be able to communicate with all other models in the system like UART, Memory, GPIO.

input signals
clock 
reset
data_bus_in
interrupt
read_en

output signals
address_bus
data_bus_out
read_en
write_en

expected behavior
When system first starts, reset pin is set which resets all the register values and also sets the program counter to default value.
On a rising_edge an instruction is fetched from memory based on value of program counter.
The instruction is then decoded, and the required operations are performed (ALU, branch, memory access, etc).
After execution the registers are updated and the program counter is also updated to the next instruction.
If between execution an interrupt is called, the CPU halts the execution, saves the current context, and then handles the interrupt.

possible faults to test later

UART Model
purpose
It is used for external communication between the CPU and other devices such as terminal 
input signals
output signals
expected behavior
possible faults to test later

GPIO Model
purpose
input signals
output signals
expected behavior
possible faults to test later

Timer/PWM Model
purpose
input signals
output signals
expected behavior
possible faults to test later

Memory Model
purpose
input signals
output signals
expected behavior
possible faults to test later

Interrupt Model
purpose
input signals
output signals
expected behavior
possible faults to test later

Control Logic Model
purpose
input signals
output signals
expected behavior
possible faults to test later

Motor/System Response Model
purpose
input signals
output signals
expected behavior
possible faults to test later