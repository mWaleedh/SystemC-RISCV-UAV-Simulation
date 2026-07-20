### Pipeline Data Hazards & Resolution
- In a pipelined architecture, multiple instructions execute simultaneously across different stages. When instructions depend on the results of immediately preceding instructions, the pipeline can read old data from the register file before the correct data has been written back. 
- These data dependencies create hazards that must be resolved through hardware forwarding and stalling.

### 1. RAW (Read-After-Write) Hazard
A RAW hazard occurs when an instruction attempts to read a register that a preceding instruction has not yet finished writing to the Register File.

**Example:**
ADDI x1, x0, 5
ADD  x2, x1, x1

**Problem:**
When the ADD instruction is in the Execute (EX) stage, the ADDI instruction is only in the Memory (MEM) stage. The value 5 has not yet been written to register x1. If the ADD instruction reads from the register file, it will compute using old, incorrect data.

### 2. Load-Use Hazard
A Load-Use hazard is a type of RAW hazard that occurs when an instruction depends on the result of a Load instruction immediately preceding it.

**Example:**
LW   x3, 0(x1)
ADD  x4, x3, x2

**Problem:**
When the ADD instruction is in the EX stage, the LW instruction is in the MEM stage. Because the memory module is just now being asked for the data, the data does not exist inside the processor yet.

### 3. Forwarding
Forwarding is the solution to RAW hazards. Instead of waiting for a value to be written to the Register File in the WB stage, the processor takes the computed data directly from the pipeline registers and routes it backward to the ALU inputs.

**Types:**
- EX-to-EX Forwarding: Routes the ALU result from the EX_MEM pipeline register directly into the ALU input for the next instruction.
- MEM-to-EX Forwarding: Routes the Writeback data from the MEM_WB pipeline register into the ALU input for an instruction that is two cycles behind.

### 4. Stalling
Because data from a Load instruction isn't available until the end of the MEM stage, Forwarding cannot solve a Load-Use hazard.

**Solution:**
- It freezes the Program Counter (pc) and the IF_ID register, preventing Fetch and Decode from moving forward.
- It forces all control signals in the ID_EX register to zero, inserting a Bubble or NOP into the Execute stage.
- This delays the dependent instruction by 1 cycle, allowing the Load instruction to reach the WB stage, at which point the MEM-to-EX Forwarding can route the data to the ALU.

### 5. Why WAR and WAW Hazards Do Not Exist Here
Advanced Processors suffer from Write-After-Read (WAR) and Write-After-Write (WAW) hazards when instructions are Executing Out-of-Order.

**Our 5-stage RISC-V core is In-Order:**
- No WAW (Write-After-Write): Instructions write to the register file only in the WB stage, exactly in the order they were fetched. A newer instruction can never get ahead of an older instruction to overwrite a register.
- No WAR (Write-After-Read): Instructions always read in the ID stage and write in the WB stage. An older instruction always completes its read way before a newer instruction reaches the WB stage to write.