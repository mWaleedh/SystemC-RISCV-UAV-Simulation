### What is Branch Prediction?
Branch prediction is a hardware optimization technique where the processor guesses the outcome of a conditional branch instruction and its destination address before the branch condition is actually evaluated. This allows the CPU to  fetch and decode subsequent instructions without waiting.

### Why is it Needed in a Pipeline?
In a pipelined architecture, instructions are fetched every clock cycle. However, a branch instruction's condition and target are not known until it reaches further into the pipeline. If the CPU waits to find out the result, it must stop fetching new instructions, leaving empty gaps (bubbles) in the pipeline. Prediction keeps the pipeline full and maximizes throughput (Instructions Per Cycle).

### Branch Penalty Without Prediction
In our 5-stage RISC-V pipeline, if the CPU simply stalled Fetch until a branch was resolved in the Execute (EX) stage, it would waste 2 clock cycles for every single branch instruction.

### Where Prediction Occurs
Prediction occurs in the Instruction Fetch (IF) stage. When the CPU fetches a branch instruction, the predictor immediately guesses whether it will jump and provides the predicted next Program Counter (PC).

### Where Branch Outcome is Resolved
The actual outcome is resolved in the Execute (EX) stage. The ALU compares the source registers and calculates the exact target offset.

### How Misprediction is Detected
In the EX stage, the CPU compares the actual outcome against what was predicted. Since our current CPU predicts "Not Taken" (fetching PC + 4), a misprediction is detected simply whenever the ALU determines branch_taken == true. 

### How Wrong-Path Instructions are Flushed
When EX detects a misprediction, it sets the flush signal to High. Then during the next clock cycle:
- The global PC is forcefully updated to the correct target address calculated by EX.
- The pipeline registers holding the fetched instructions (IF/ID and ID/EX) are converted into bubbles.
- The correct instruction is fetched from the new PC.

## Planned Implementation Stages
### 1. Always-Not-Taken (Current State)
* **Mechanism:** Static prediction. The CPU assumes every conditional branch will evaluate to false.
* **Behavior:** Fetch simply requests PC + 4 sequentially. 
* **Penalty:** 0 cycles if not taken, 2 cycles (flush IF, ID) if taken.

### 2. One-Bit Predictor
* **Mechanism:** Dynamic prediction utilizing a Branch History Table (BHT). 
* **Behavior:** Each entry in the table holds 1 bit representing the outcome of the last time that specific branch was executed (1 = Taken, 0 = Not Taken). The CPU predicts it will do whatever it did last time.
* **Flaw:** In a typical loop, a 1-bit predictor mispredicts twice: once when the loop exits, and once when the loop is entered again (because the bit is still storing the last state).

### 3. Two-Bit Saturating Predictor
* **Mechanism:** Dynamic prediction using a 2-bit state machine for each BHT entry.
* **States:** 
  * 00 (Strongly Not Taken)
  * 01 (Weakly Not Taken)
  * 10 (Weakly Taken)
  * 11 (Strongly Taken)
* **Behavior:** It takes two consecutive mispredictions to flip the primary prediction direction. This solves the loop entry/exit mispredction of the 1-bit predictor.

### 4. Branch Target Buffer (BTB) (Optional)
* **Mechanism:** A cache that stores the actual calculated target address of previously executed branches.
* **Behavior:** Without a BTB, a Taken prediction still gives a 1 cycle penalty while the ID stage calculates the jump address. A BTB allows the IF stage to instantly fetch the target address on the exact same cycle the branch is predicted Taken.