# Supported RISC-V Instructions

## Integer Computational Instructions

### Arithmetic & Logical
| Instruction | Name | Format | Description |
|---|---|---|---|
| **ADD** | Add | R-Type | Adds registers `rs1` and `rs2` |
| **SUB** | Subtract | R-Type | Subtracts `rs2` from `rs1` |
| **ADDI** | Add Immediate | I-Type | Adds sign-extended 12-bit immediate to `rs1` |
| **SLT** | Set Less Than | R-Type | Sets `rd` to 1 if `rs1` < `rs2` (signed) |
| **SLTI** | Set Less Than Immediate | I-Type | Sets `rd` to 1 if `rs1` < immediate (signed) |
| **SLTU** | Set Less Than Unsigned | R-Type | Sets `rd` to 1 if `rs1` < `rs2` (unsigned) |
| **SLTIU** | Set Less Than Imm Unsigned | I-Type | Sets `rd` to 1 if `rs1` < immediate (unsigned) |
| **AND** | Logical AND | R-Type | Bitwise AND of `rs1` and `rs2` |
| **OR** | Logical OR | R-Type | Bitwise OR of `rs1` and `rs2` |
| **XOR** | Logical XOR | R-Type | Bitwise XOR of `rs1` and `rs2` |
| **ANDI** | AND Immediate | I-Type | Bitwise AND of `rs1` and immediate |
| **ORI** | OR Immediate | I-Type | Bitwise OR of `rs1` and immediate |
| **XORI** | XOR Immediate | I-Type | Bitwise XOR of `rs1` and immediate |
| **LUI** | Load Upper Immediate | U-Type | Loads 20-bit immediate into upper 20 bits of `rd` |

### Shifts
| Instruction | Name | Format | Description |
|---|---|---|---|
| **SLL** | Shift Left Logical | R-Type | Shifts `rs1` left by `rs2` (lower 5 bits) |
| **SLLI** | Shift Left Logical Imm | I-Type | Shifts `rs1` left by immediate |
| **SRL** | Shift Right Logical | R-Type | Shifts `rs1` right by `rs2` (zero-extends) |
| **SRLI** | Shift Right Logical Imm | I-Type | Shifts `rs1` right by immediate (zero-extends) |
| **SRA** | Shift Right Arithmetic | R-Type | Shifts `rs1` right by `rs2` (sign-extends) |
| **SRAI** | Shift Right Arith Imm | I-Type | Shifts `rs1` right by immediate (sign-extends) |

---

## Memory Instructions (Load/Store)

| Instruction | Name | Format | Description |
|---|---|---|---|
| **LW** | Load Word | I-Type | Loads 32-bit value from memory into `rd` |
| **SW** | Store Word | S-Type | Stores 32-bit value from `rs2` to memory |

---

## Control Flow Instructions

### Branches
| Instruction | Name | Format | Description |
|---|---|---|---|
| **BEQ** | Branch if Equal | B-Type | Branches to target if `rs1` == `rs2` |
| **BNE** | Branch if Not Equal | B-Type | Branches to target if `rs1` != `rs2` |
| **BLT** | Branch if Less Than | B-Type | Branches to target if `rs1` < `rs2` (signed) |
| **BGE** | Branch if Greater/Equal | B-Type | Branches to target if `rs1` >= `rs2` (signed) |
| **BLTU** | Branch if Less Than Unsigned | B-Type | Branches to target if `rs1` < `rs2` (unsigned) |
| **BGEU** | Branch if Greater/Eq Unsigned | B-Type | Branches to target if `rs1` >= `rs2` (unsigned) |

### Jumps
| Instruction | Name | Format | Description |
|---|---|---|---|
| **JAL** | Jump and Link | J-Type | Jumps to PC-relative offset, saves return address in `rd` |
| **JALR** | Jump and Link Register | I-Type | Jumps to `rs1` + immediate, saves return address in `rd` |

---

## System & Privilege Instructions

### CSR (Control and Status Register)
| Instruction | Name | Format | Description |
|---|---|---|---|
| **CSRRW** | CSR Read/Write | I-Type | Writes `rs1` to CSR, reads old CSR value into `rd` |
| **CSRRS** | CSR Read/Set | I-Type | Sets bits in CSR defined by `rs1`, reads old CSR into `rd` |

### System Exceptions
| Instruction | Name | Format | Description |
|---|---|---|---|
| **MRET** | Machine Return | I-Type | Returns from machine-level trap, restores MIE and PC |