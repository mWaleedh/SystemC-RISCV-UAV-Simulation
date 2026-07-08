- The architecture utilizes Memory-Mapped I/O (MMIO), meaning all hardware peripherals and memory modules share a single, unified 32-bit address space.
- The CPU uses standard load (LW) and store (SW) instructions to interact with both the main memory and the external hardware registers.

### Memory Regions
- 0x00000000 - 0x00000FFF: Reserved for Main Memory which is 4KB in size and stores both instructions and data.
- 0x10000000: Reserved for GPIO Output Register. We write 1 or 0 to toggle external LEDs or output pins through this.
- 0x10000004: Reserved for GPIO Input Register. We read the physical state of external buttons or input pins through this.
- 0x10000010: Reserved for Timer Count Register (Optional / Planned). Reads the current machine time.
- 0x10000014: Reserved for Timer Compare Register (Optional / Planned). Sets the threshold for timer interrupts.

### Access Guidelines
- All standard memory and MMIO accesses should be aligned to 4-byte boundaries (addresses ending in 0, 4, 8, or C).
