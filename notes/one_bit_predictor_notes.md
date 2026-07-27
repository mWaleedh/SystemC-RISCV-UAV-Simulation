### How it works
A one-bit branch predictor relies on the principle that a branch is highly likely to do whatever it did last time. It utilizes a small memory array called a Branch History Table (BHT) to store the previous outcome of branches. 
- 1 = Predicted Taken
- 0 = Predicted Not Taken

### Architecture
- **Entries:** 32-entry table (1 bit per entry).
- **Indexing:** The table is indexed using bits `[6:2]` of the Program Counter (PC). Because RISC-V instructions are 32-bit (4-byte) aligned, bits `[1:0]` are always 00. Extracting bits `[6:2]` gives 32 unique indices, allowing multiple branches in a program to share the table.

## Implementation
1. **Instruction Fetch (IF) Stage**
Without a Branch Target Buffer (BTB), the earliest we can identify a branch is when the instruction arrives from memory. 
When an instruction arrives, the IF stage performs a quick check to see if the opcode is 0x63 (Branch). If so, it reads the BHT at the index using the instruction's PC. 
- If the BHT predicts Taken, the IF stage extracts the immediate offset, calculates the target address, instantly overrides the next PC request, and sets the ignore_next_fetch to High inorder to drop the sequential instruction arriving next cycle.
- It passes a predicted_taken flag down the pipeline.

2. **Instruction Execute (EX) Stage**
The EX stage evaluates the true condition of the branch and compares the actual outcome against the predicted_taken flag.
- The BHT bit is overwritten with the actual outcome.
- If the prediction was correct, execution continues normally (0 penalty cycles if Not Taken, 1 penalty cycle if Taken due to memory latency).
- If mispredicted, the EX stage triggers a flush. It redirects the PC to the calculated target (if it should have jumped) or back to sequential instruction (if it shouldn't have jumped).