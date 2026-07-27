## How it works
The weakness of a 1-bit predictor is that it lacks memory of a trend. If a loop executes 100 times, the 1-bit predictor learns it is "Taken". But on the 101st time (loop exit), it evaluates "Not Taken", which causes a misprediction. Even worse, this flips the 1-bit table. The next time the loop starts, the first iteration will also mispredict because the table is remembering the loop exit.

A 2-bit saturating counter solves this by requiring two consecutive mispredictions before it changes its actual prediction direction.

## State Machine
The Branch History Table (BHT) stores a 2-bit unsigned integer (values 0-3) for each entry:
- 11 (3): Strongly Taken     -> Predicts Taken
- 10 (2): Weakly Taken       -> Predicts Taken
- 01 (1): Weakly Not Taken   -> Predicts Not Taken
- 00 (0): Strongly Not Taken -> Predicts Not Taken

## Update Rules (Saturation)
When the Execute stage calculates the branch result:
- **If Actually Taken:** Increment the counter. Stop it at 3 (Saturation).
- **If Actually Not Taken:** Decrement the counter. Stop it at 0 (Saturation).

## Architecture
- **Table Size:** 32 entries.
- **Data Type:** 2-bit integers (I'm using an 8-bit unsigned integer capped at 3).
- **Indexing:** Bits `[6:2]` of the PC.