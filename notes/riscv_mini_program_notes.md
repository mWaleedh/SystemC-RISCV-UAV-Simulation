### What the Integrated Mini Program Tests
The integrated mini program tests every implemented instruction category which includes Arithmetic, Memory Access, Conditional Branching, and Unconditional Jumps. To test the working of the system, the program has trap instructions which overwrite register values. If the branching and jumping logic works perfectly, the CPU successfully skips these traps and computes the correct final values.

### How Arithmetic Instructions Work
- I-Type (ADDI): Adds the value of a source register (rs1_data) to a sign-extended immediate value (imm) extracted from the instruction.
- R-Type (ADD, SUB): Performs operations using data from two source registers (rs1_data and rs2_data). 
- The calculated result (alu_res) is then written into the destination register (rd) during the Write-Back (WB) stage.

### How Load/Store Instructions Work
- Address Calculation: Both LW and SW calculate their target memory address in the EX stage by adding an immediate offset to a base register rs1 (rs1_data + imm).
- Memory Access: During the MEM stage, SW pushes data to the data_bus_o to be saved in RAM, waits 2 cycles for the write to complete. LW sets read_en to true, waits for the memory to respond, and takes the data from data_bus_i and stores it into a mem_read_data variable.
- Write-Back (WB): SW does not update any registers. LW takes the mem_read_data variable and saves it into its destination register (rd).

### How Branch Instructions Work
- In the EX stage, the ALU compares the two source registers. If the condition is met (rs1_data == rs2_data for BEQ and rs1_data != rs2_data for BNE), the branch_taken flag is set to true.
- The pc_next variable is calculated by adding the B-Type immediate offset to the current Program Counter (pc + imm).
- The WB stage executes no register updates, and simply assigns the pc_next variable to the Program Counter.

### How Jump Instructions Work
- JAL (J-Type): Calculates the target address by adding a offset in the Program Counter (pc + imm).
- JALR (I-Type): Calculates the target address by using a base register stored in rs1, adding an offset to it, and finally setting lsb to 0 to align the address (rs1_data + imm & ~1).
- Return Address: Both instructions calculate pc + 4 and store it in alu_res during the EX stage. The WB stage then saves this return address into the destination register (rd).

### How the PC is Updated
The next Program Counter value (pc_next) is first calculated in the Execute (EX) stage depending on the type of instruction and whether it is true or not. Then at the the very end of the Write-Back (WB) stage we assign the pc_next to the Program Counter.

### How .hex Loading Works
Instead of manually inserting instructions one at a time, the system uses the ifstream library in the memory model. The testbench calls sys.load_hex("mini_program.hex"), which reads the text file line-by-line, converts the string hex code into 32-bit integers using, and loads them directly into the memory array. This method was chosen as it uses 0-cycles.

### Expected Final Register & Memory Values
- x1 (Base Addr) = 20
- x2 (BEQ Test) = 20
- x3 (SW Data) = 100
- x4 (LW Data) = 100
- x5 (ADD Math) = 200
- x6 (SUB Math) = 180
- x7 (JAL Return) = 48
- x8 (JALR Target) = 64
- x9 (JALR Return) = 60
- mem[20] (Storage) = 100

### Result Obtained
The CPU perfectly routed through the pipeline, memory transactions took the correct amount of clock cycles, and control flow bypassed all trap instructions and all 10 automated testbench checks returned PASS.

### Problems Faced
The problem was an incorrectly assembled instruction. Since the system was working perfectly on all previous testbenches, I realized quickly that the problem will most probably be in the .hex file. Correcting that single line of hex code immediately resolved all failures.

### What Will Be Added Next
Support for other instructions which includes LUI, AUIPC, AND, OR, XOR, SLT, SLL, SRL, SRA