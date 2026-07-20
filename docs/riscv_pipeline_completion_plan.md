### Current IF stage
- Fetches the next 32-bit instruction from instruction memory.
- Increments the Program Counter by 4 every cycle, predicting that branches are not taken.
- Checks the global flush signal. If High, it ignores the fetched instruction and passes a bubble to the next stage.
- Sends values to next stage through the IF_ID pipeline register and marks the instruction as valid.

### Current ID stage
- Decodes the fetched instruction from the IF_ID register into opcode, funct3, funct7, and control signals.
- Reads the values of the two source registers simultaneously from the register file.
- Uses the Immediate Generator to sign-extend or zero-extend immediate values based on instruction type.
- Checks the global flush signal. If High, it clears its output to prevent a wrong instruction from executing.
- Sends values to EX through ID_EX pipeline register with decoded data, register values, and the valid bit.

### Current EX stage
- Reads from the ID_EX register and uses the ALU to perform mathematical and logical operations.
- Calculates the exact branch conditions (using signed/unsigned comparisons) and calculates the actual branch/jump target address.
- If a branch is taken, a jump occurs, or an MRET is executed, it forcefully overwrites the PC with the target address and sets the global flush signal High.
- Sends values to MEM through EX_MEM pipeline register with the computed ALU result, store data (rs2), and CSR signals.

### Current MEM stage
- Reads from the EX_MEM register to handle memory-mapped peripheral and data memory requests.
- For Load operations, it sets the read_en_o High and puts the address on addr_bus_o but does not wait for the data to return.
- For Store operations, it sets the write_en_o High, puts the address on addr_bus_o, and places data on data_bus_o.
- Sends values to WB through MEM_WB pipeline register, passing the ALU result and destination register (rd) information.

### Current WB stage
- Reads from the MEM_WB register to write the final computed results back to the register file or CSRs.
- For Load instructions, it reads the returning data directly from data_bus_i.
- Ensures that register x0 remains zero by discarding any writes to it.
- Doesn't update the Program Counter, as PC logic is now handled in the IF and EX stages.

### Pipeline registers between stages
- IF_ID struct holds the fetched instruction and passes it to Decode.
- ID_EX struct holds decoded control signals, read register data, the sign-extended immediate, and opcode.
- EX_MEM struct connected the execution and memory, holding the ALU result, the store data (rs2), and CSR control signals.
- MEM_WB struct carries the ALU result or memory operation into the final writeback stage.

### Current valid bits
- Fully implemented across the datapath. Every pipeline struct (IF_ID, ID_EX, EX_MEM, MEM_WB) contains a valid boolean flag.
- These bits are used by every stage to differentiate between real instructions and pipeline bubbles, ensuring that old data doesn't accidentally overwrite memory or registers.

### Current flushing logic
- Fully implemented for branches/jumps.
- When the EX stage identifies a taken branch, it sets flush = true.
- Because this CPU executes the stage functions in reverse order (WB -> MEM -> EX -> ID -> IF), the IF and ID stages detect this flush flag on the exact same clock cycle and immediately insert bubbles into the pipeline.

### Current stall logic
- Unimplemented. The processor currently can not freeze the IF or ID stages.
- Because stalls are missing, the pipeline will currently fail if an instruction requires data from a memory load that hasn't arrived yet.

### Current forwarding logic
- Unimplemented. The datapath currently suffers from Data Hazards.
- If an instruction reads a register that a preceding instruction is currently computing in EX or MEM, it will read the old data from the register file.

### Missing hazard handling
- A Forwarding Unit is required to pass the updated data from the EX_MEM and MEM_WB registers into the ALU inputs in the EX stage.
- A Hazard Detection Unit is required to detect Load hazards and physically stall the PC and IF_ID register.

### Planned branch-prediction point
- Implemented statically in the IF stage.
- The fetch logic uses a "Branch Not Taken" prediction by assuming linear execution (pc = pc + 4) for every instruction.

### Planned branch-resolution point
- Implemented dynamically in the EX stage.
- The ALU evaluates the true branch condition. If the static prediction from the IF stage was incorrect, the EX stage triggers a pipeline flush, resulting in a 2-cycle performance penalty.