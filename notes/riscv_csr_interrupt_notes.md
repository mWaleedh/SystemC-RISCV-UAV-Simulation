## MIP Register (Machine Interrupt Pending) Control

The `mip` register contains both software-writable and hardware-controlled bits. Software access (CSRRW/CSRRS) must be masked to prevent corrupting hardware state.

* **Bit 7 (MTIP - Machine Timer Interrupt Pending):** Strictly HARDWARE CONTROLLED. 
  * Asserted (`1`) when the external timer interrupt wire is HIGH.
  * Cleared (`0`) when the external timer interrupt wire is LOW.
  * **Software Access:** Read-only. Software cannot clear this bit directly via CSR instructions. To clear MTIP, software must perform a memory-mapped write to the Timer Compare register (`0x10000018`), which causes the timer hardware to pull the interrupt wire LOW.
* **Bit 11 (MEIP - Machine External Interrupt Pending):** Hardware controlled (driven by external PLIC). Read-only to software.
* **Bit 3 (MSIP - Machine Software Interrupt Pending):** Software controlled. Can be written via CSR instructions or memory-mapped I/O depending on implementation.