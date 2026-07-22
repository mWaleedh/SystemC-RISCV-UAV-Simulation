## Pipeline Counters
- **Total Cycles**: The total number of clock ticks elapsed since reset. Acts as the denominator for time-based metrics.
- **Committed Instructions**: The count of valid instructions that successfully complete the Write Back (WB) stage. Flushed instructions and pipeline bubbles are excluded.
- **Pipeline Stalls**: The number of clock cycles where the pipeline is intentionally frozen. Triggered by Load-Use data hazards (Decode stage) and memory access latency (Memory stage).
- **Pipeline Flushes**: The number of flush events triggered to clear incorrect instructions from the pipeline. Initiated by control hazards (mispredicted branches, jumps) and asynchronous traps (interrupts, MRET).
- **Branches Executed**: The total number of conditional branch instructions (B-type: BEQ, BNE, BLT, etc.) processed by the Execute stage.
- **Branches Taken**: The number of executed conditional branches that evaluated to true, resulting in a PC jump.
- **Branch Mispredictions**: The number of times the static branch predictor guessed incorrectly. Since the current architecture utilizes a static "Always Not Taken" prediction, this value perfectly mirrors `branches_taken`.
- **Timer Interrupts Accepted**: The number of asynchronous timer interrupts successfully caught and processed by the CPU, resulting in a context switch to the trap handler.

## Derived Performance Metrics
### CPI (Cycles Per Instruction)
* **Formula:** Total Cycles / Committed Instructions
* **Target:** An ideal scalar 5-stage pipeline has a CPI of 1.0. 
* **Meaning:** Any value above 1.0 represents efficiency lost to structural, data, and control hazards. It indicates how many clock cycles it takes, on average, to finish a single instruction.

### IPC (Instructions Per Cycle)
* **Formula:** Committed Instructions / Total Cycles
* **Target:** 1.0 for a single-issue core.
* **Meaning:** The inverse of CPI. It measures the throughput of the processor. An IPC of 0.8 means the CPU successfully finishes an instruction on 80% of its clock cycles.

### Stall Rate
* **Formula:** (Pipeline Stalls / Total Cycles) * 100
* **Meaning:** The percentage of total execution time the CPU spent frozen, waiting for memory accesses to complete or resolving data dependencies. High stall rates indicate a need for a data cache or better compiler instruction scheduling.

### Branch Misprediction Rate
* **Formula:** (Branch Mispredictions / Branches Executed) * 100
* **Meaning:** Evaluates the success of the branch prediction strategy. Because the current core uses an "Always Not Taken" strategy, this directly shows the percentage of conditional loops and \if statements in the code that ultimately branch.