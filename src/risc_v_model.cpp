#include <iostream>
#include <systemc.h>
using namespace std;

SC_MODULE(risc_v_model) {
    // constants
    static const int WIDTH = 32;

    // input ports
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;
    sc_in<bool> irq_timer_i;
    sc_in<bool> irq_ext_i;
    sc_in<bool> irq_sw_i;

    // Instruction ports
    sc_in<sc_uint<WIDTH>> inst_bus_i;
    sc_out<bool> inst_read_en_o;
    sc_out<sc_uint<WIDTH>> inst_addr_bus_o;

    // Data ports
    sc_in<sc_uint<WIDTH>> data_bus_i;
    sc_out<bool> data_write_en_o;
    sc_out<bool> data_read_en_o;
    sc_out<sc_uint<WIDTH>> data_bus_o;
    sc_out<sc_uint<WIDTH>> data_addr_bus_o;

    // IF/ID Register
    struct IF_ID {
        sc_uint<WIDTH> pc;
        sc_uint<WIDTH> inst;
        bool valid;
    } if_id;

    // ID/EX Register
    struct ID_EX {
        sc_uint<WIDTH> pc;
        sc_uint<WIDTH> inst;
        bool valid;
        
        sc_uint<7> opcode;
        sc_uint<5> rd;
        sc_uint<5> rs1;
        sc_uint<5> rs2;
        sc_uint<3> funct3;
        sc_uint<7> funct7;
        
        sc_uint<WIDTH> rs1_data;
        sc_uint<WIDTH> rs2_data;
        sc_int<WIDTH> imm;

        bool reg_write;

        // CSR / Interrupt Signals
        bool is_mret_instruction;
        bool is_csr_instruction;
        bool csr_read_enable;
        bool csr_write_enable;
        sc_uint<WIDTH> csr_address;
        sc_uint<WIDTH> csr_operation;
    } id_ex;

    // EX/MEM Register
    struct EX_MEM {
        sc_uint<WIDTH> pc;
        sc_uint<WIDTH> inst;
        bool valid;
        
        sc_uint<7> opcode;
        sc_uint<3> funct3;
        sc_uint<5> rd;
        
        sc_uint<WIDTH> alu_res;
        sc_uint<WIDTH> store_data;

        bool reg_write;

        // CSR Signals for Write Back
        bool is_csr_instruction;
        bool csr_write_enable;
        sc_uint<WIDTH> csr_address;
        sc_uint<WIDTH> csr_old_value;
        sc_uint<WIDTH> csr_new_value;
    } ex_mem;

    // MEM/WB Register
    struct MEM_WB {
        sc_uint<WIDTH> pc;
        sc_uint<WIDTH> inst;
        bool valid;
        
        sc_uint<7> opcode;
        sc_uint<3> funct3;
        sc_uint<5> rd;
        
        sc_uint<WIDTH> alu_res;
        sc_uint<WIDTH> mem_data;

        bool reg_write;

        // CSR Signals for Write Back
        bool is_csr_instruction;
        bool csr_write_enable;
        sc_uint<WIDTH> csr_address;
        sc_uint<WIDTH> csr_old_value;
        sc_uint<WIDTH> csr_new_value;
    } mem_wb;

    struct MEM_WB_Old {
        bool valid;
        bool reg_write;
        sc_uint<5> rd;
        sc_uint<7> opcode;
        sc_uint<WIDTH> alu_res;
        sc_uint<WIDTH> mem_data;
    } mem_wb_old;

    // Pipeline Control Signals
    bool stall;
    bool flush;
    bool branch_redirect;
    bool interrupt_redirect;

    sc_uint<WIDTH> pc;
    bool in_interrupt;
    bool mem_stall;

    // Register File
    sc_uint<WIDTH> registers[WIDTH];

    // CSRs
    sc_uint<WIDTH> mstatus;
    sc_uint<WIDTH> mie;
    sc_uint<WIDTH> mip;
    sc_uint<WIDTH> mtvec;
    sc_uint<WIDTH> mepc;
    sc_uint<WIDTH> mcause;

    // ------------------------------------------------------------
    // Helper Functions
    // ------------------------------------------------------------

    sc_int<WIDTH> immediateGenerator(sc_uint<7> opcode, sc_uint<WIDTH> inst) {
        int immediate = 0;
        switch (opcode) {
        // R-type
        case 0x33:
            immediate = 0;
            break;

        // I-type
        case 0x13:
        case 0x3:
        case 0x67:
            // [11:0]
            immediate = (inst >> 20 ) & 0xFFF;

            // Sign extension
            if (immediate & 0x800) { 
                // [31:12]
                immediate |= 0xFFFFF000; 
            }
            break;

        // S-type
        case 0x23:
            // [11:5]
            immediate = ((inst >> 25 ) & 0x7F) << 5;
            // [4:0]
            immediate |= (inst >> 7) & 0x1F;

            // Sign extension
            if (immediate & 0x800) { 
                // [31:12]
                immediate |= 0xFFFFF000; 
            }
            break;

        // B-type
        case 0x63:
            // [12]
            immediate = ((inst >> 31) & 0x1) << 12;
            // [11]
            immediate |= ((inst >> 7) & 0x1) << 11;
            // [10:5]
            immediate |= ((inst >> 25) & 0x3F) << 5;
            // [4:1]
            immediate |= ((inst >> 8) & 0xF) << 1;

            // Sign extension
            if (immediate & 0x1000) {
                // [31:13]
                immediate |= 0xFFFFE000; 
            }
            break;

        // U-type
        case 0x37:
        case 0x17:
            // [31:12]
            immediate = ((inst >> 12) & 0xFFFFF) << 12;
            break;

        // J-type
        case 0x6F:        
            // [20]
            immediate = ((inst >> 31) & 0x1) << 20;
            // [19:12]
            immediate |= ((inst >> 12) & 0xFF) << 12;
            // [11]
            immediate |= ((inst >> 20) &0x1) << 11;
            // [10:1]
            immediate |= ((inst >> 21) & 0x3FF) << 1;

            // Sign extension
            if (immediate & 0x100000) {
                // [31:21]
                immediate |= 0xFFF00000; 
            }
            break;

        // CSR/MRET
        case 0x73:
            // [31:20]
            immediate = (inst >> 20) & 0xFFF;
            break;
            
        default:
            immediate = 0;
        }

        return immediate;
    }

    bool validInstruction(sc_uint<7> opcode) {
        switch (opcode) {
        // R-type
        case 0x33:
        // I-type
        case 0x13:
        case 0x3:
        case 0x67:
        // S-type
        case 0x23:
        // B-type
        case 0x63: 
        // U-type
        case 0x37:
        case 0x17:
        // J-type
        case 0x6F:         
        // CSR/MRET
        case 0x73:
            return true;
        default:
            return false;
        }
    }

    sc_uint<WIDTH> alu(sc_uint<7> opcode, sc_uint<3> funct3, sc_uint<7> funct7, sc_uint<WIDTH> rs1_data, sc_uint<WIDTH> rs2_data, sc_int<WIDTH> imm) {
        sc_uint<WIDTH> alu_result;

        switch (opcode) {
        // R-type
        case 0x33: 
            if (funct3 == 0x0) {
                // ADD
                if (funct7 == 0x00) {
                    alu_result = rs1_data + rs2_data;
                    cout << "@" << sc_time_stamp() << " ALU: " << rs1_data << " + " << rs2_data << " = " << alu_result << endl << endl;
                }
                // SUB
                else if (funct7 == 0x20) {
                    alu_result = rs1_data - rs2_data;
                    cout << "@" << sc_time_stamp() << " ALU: " << rs1_data << " - " << rs2_data << " = " << alu_result << endl << endl;
                }
            }
            // SLL
            else if (funct3 == 0x1) {
                alu_result = rs1_data << (rs2_data & 0x1F);
            }
            // SLT
            else if (funct3 == 0x2) {
                alu_result = ((sc_int<WIDTH>)rs1_data < (sc_int<WIDTH>)rs2_data) ? 1 : 0;
            }
            // SLTU
            else if (funct3 == 0x3) {
                alu_result = (rs1_data < rs2_data) ? 1 : 0;
            }
            // XOR
            else if (funct3 == 0x4) {
                alu_result = rs1_data ^ rs2_data;
            }
            else if (funct3 == 0x5) {
                // SRL
                if (funct7 == 0x00) {
                    alu_result = rs1_data >> (rs2_data & 0x1F);
                }
                // SRA
                else if (funct7 == 0x20) {
                    alu_result = (sc_int<WIDTH>)rs1_data >> (rs2_data & 0x1F);
                }
            }
            // OR
            else if (funct3 == 0x6) {
                alu_result = rs1_data | rs2_data;
            }
            // AND
            else if (funct3 == 0x7) {
                alu_result = rs1_data & rs2_data;
            }
            
            cout << "@" << sc_time_stamp() << " ALU (R-type): Result = " << alu_result << endl << endl;
            break;

        // I-type (ALU)
        case 0x13:
            // ADDI
            if (funct3 == 0x0) {
                alu_result = rs1_data + imm;
            }
            // SLLI
            else if (funct3 == 0x1) {
                alu_result = rs1_data << (imm & 0x1F);
            }
            // SLTI
            else if (funct3 == 0x2) {
                alu_result = ((sc_int<WIDTH>)rs1_data < imm) ? 1 : 0;
            }
            // SLTIU
            else if (funct3 == 0x3) {
                alu_result = (rs1_data < (sc_uint<WIDTH>)imm) ? 1 : 0;
            }
            // XORI
            else if (funct3 == 0x4) {
                alu_result = rs1_data ^ imm;
            }
            else if (funct3 == 0x5) {   
                // SRLI
                if (funct7 == 0x00) {
                    alu_result = rs1_data >> (imm & 0x1F);
                }
                // SRAI
                else if (funct7 == 0x20)  {
                    alu_result = (sc_int<WIDTH>)rs1_data >> (imm & 0x1F);
                }
            }
            // ORI
            else if (funct3 == 0x6) {
                alu_result = rs1_data | imm;
            }
            // ANDI
            else if (funct3 == 0x7) {
                alu_result = rs1_data & imm;
            }

            cout << "@" << sc_time_stamp() << " ALU (I-Type): Result = " << alu_result << endl << endl;
            break;
            
        // I-type (Load)
        case 0x3:
            alu_result = rs1_data + imm;
            cout << "@" << sc_time_stamp() << " ALU (Load): Target Address = 0x" << hex << alu_result << dec << endl << endl;
            break;
        
        // S-type
        case 0x23:
            alu_result = rs1_data + imm;
            cout << "@" << sc_time_stamp() << " ALU (Store): Target Address = 0x" << hex << alu_result << dec << endl << endl;
            break;
        
        // U-type (LUI)
        case 0x37:
            alu_result = imm;
            cout << "@" << sc_time_stamp() << " ALU (LUI): Result = 0x" << hex << alu_result << dec << endl << endl;
            break;

        // U-type (AUIPC)
            alu_result = pc + imm;
            cout << "@" << sc_time_stamp() << " ALU (AUIPC): Result = 0x" << hex << alu_result << dec << endl << endl;
            break;
            
        default:
            alu_result = 0;            
        }

       return alu_result;
    }

    sc_uint<WIDTH> read_csr(uint32_t csr_addr) {
        switch(csr_addr) {
            case 0x300: 
                return mstatus; 
            case 0x304: 
                return mie; 
            case 0x305: 
                return mtvec; 
            case 0x341: 
                return mepc; 
            case 0x342: 
                return mcause; 
            case 0x344:
                return mip;
            default: 
                cout << "@" << sc_time_stamp() << " Execute Error: Invalid CSR read at 0x" << hex << csr_addr << dec << endl << endl; 
        }

        return 0;
    }

    void write_csr(uint32_t csr_addr, uint32_t csr_old, uint32_t csr_new) {
        switch(csr_addr) {
            case 0x300: 
                mstatus = csr_new; 
                break;
            case 0x304: 
                mie = csr_new;
                break;
            case 0x305: 
                mtvec = csr_new;
                break;
            case 0x341:
                mepc = csr_new;
                break;
            case 0x342:
                mcause = csr_new;
                break;
            case 0x344:
                // Protect 7th bit of MIP from write
                mip = (csr_new & ~0x80) | (mip & 0x80);
            default: 
                break; 
        }

        cout << "@" << sc_time_stamp() << " Execute: CSR | Address: 0x" << hex << csr_addr << " | Old: 0x" << csr_old << " | New: 0x" << csr_new << dec << endl << endl;
    }

    // ------------------------------------------------------------
    // Pipeline
    // ------------------------------------------------------------

    // IF: Instruction Fetch
    // ------------------------------
    void fetch() {
        if (mem_stall) {
            // Request the same instruction for the next cycle
            inst_addr_bus_o.write(pc - 4); 
            inst_read_en_o.write(true);
            cout << "@" << sc_time_stamp() << " Fetch: Stalled for MEM at PC -> 0x" << hex << pc << dec << endl << endl;
            return;
        }

        // Read instruction sent by memory
        sc_uint<WIDTH> inst = inst_bus_i.read();

        // Check for stall
        if (stall) {
            // Request the same instruction for the next cycle
            inst_addr_bus_o.write(pc - 4); 
            inst_read_en_o.write(true);

            cout << "@" << sc_time_stamp() << " Fetch: Stalled at PC -> 0x" << hex << pc << dec << endl << endl;
            return;
        }

        cout << "@" << sc_time_stamp() << " Fetch: Received Inst -> 0x" << hex << inst << dec << endl << endl;

        // Check for flush
        if (flush) {
            // Insert bubble into IF/ID register
            if_id.pc = 0;
            if_id.inst = 0;
            if_id.valid = false;

            cout << "@" << sc_time_stamp() << " Fetch: Flushed. Bubble inserted into IF/ID." << endl << endl;
        }
        else {
            // Pass data to IF/ID register
            if_id.pc = pc - 4;
            if_id.inst = inst;
            if_id.valid = true;
        }

        // Request the instruction needed in the next cycle
        inst_addr_bus_o.write(pc);
        inst_read_en_o.write(true);
        
        cout << "@" << sc_time_stamp() << " Fetch: Requested next PC -> 0x" << hex << pc << dec << endl << endl;

        // Move to next instruction
        pc = pc + 4;
    }

    // ID: Instruction Decode
    // ------------------------------
    void decode() {
        if (mem_stall) {
            return;
        }

        // Check for flush or bubble
        if (flush || !if_id.valid) {
            id_ex.valid = false;
            return;
        }

        // Get instruction from previous stage
        sc_uint<WIDTH> inst = if_id.inst;

        // Extract source registers and opcode
        uint32_t rs1 = (inst >> 15) & 0x1F;
        uint32_t rs2 = (inst >> 20) & 0x1F;
        uint32_t opcode = inst & 0x7F;

        // Check if the current operation uses rs1 or rs2
        bool uses_rs1 = (opcode != 0x37 && opcode != 0x17 && opcode != 0x6F); 
        bool uses_rs2 = (opcode == 0x33 || opcode == 0x23 || opcode == 0x63);

        // Check for load-use hazard
        if (id_ex.valid && id_ex.opcode == 0x03 && id_ex.rd != 0) {
            if ((uses_rs1 && id_ex.rd == rs1) || (uses_rs2 && id_ex.rd == rs2)) {
                // Stall pipeline by one cycle
                stall = true;
                
                // Insert bubble in ID/EX Register
                id_ex.pc = 0;
                id_ex.inst = 0;
                id_ex.valid = false;
                id_ex.reg_write = false;
                
                return;
            }
        }

        stall = false;

        cout << "@" << sc_time_stamp() << " Decode: Instruction -> 0x" << hex << inst << dec << endl << endl;

        // Pass PC and instruction to next stage
        id_ex.pc = if_id.pc;
        id_ex.inst = inst;

        // Divide and instruction to ID/EX Register
        id_ex.opcode = opcode;
        id_ex.rd = (inst >> 7) & 0x1F;
        id_ex.funct3 = (inst >> 12) & 0x7;
        id_ex.rs1 = rs1;
        id_ex.rs2 = rs2;
        id_ex.funct7 = (inst >> 25) & 0x7F;

        // Extract immediate
        id_ex.imm = immediateGenerator(id_ex.opcode, inst);

        // Skip if invalid opcode
        if (!validInstruction(id_ex.opcode)) {
            // Insert bubble into ID/EX Register
            id_ex.pc = 0;
            id_ex.inst = 0;
            id_ex.valid = false;
            id_ex.reg_write = false;
            cout << "@" << sc_time_stamp() << " Decode Warning: Invalid Instruction. Bubble inserted into ID/EX" << endl << endl;
            return;
        }

        // Enable reg_write (used in forwarding)
        if (id_ex.opcode == 0x33 || id_ex.opcode == 0x13 || id_ex.opcode == 0x03 || id_ex.opcode == 0x37 || id_ex.opcode == 0x17 || id_ex.opcode == 0x6F || id_ex.opcode == 0x67 || id_ex.opcode == 0x73) {
            id_ex.reg_write = true;
        }
        else {
            id_ex.reg_write = false;
        }

        // Default CSR/MRET signals
        id_ex.is_csr_instruction = false;
        id_ex.is_mret_instruction = false;
        id_ex.csr_read_enable = false;
        id_ex.csr_write_enable = false;
        id_ex.csr_operation = 0;
        id_ex.csr_address = id_ex.imm;

        // Enable signals if opcode is 0x73 (MRET/CSR)
        if (id_ex.opcode == 0x73) {
            if (id_ex.funct3 == 0x0) {
                // MRET instruction
                if (id_ex.imm == 0x302) {
                    id_ex.is_mret_instruction = true;
                }
            } 
            else if (id_ex.funct3 == 0x1 || id_ex.funct3 == 0x2) {
                // CSR instructions (CSRRW/CSRRS)
                id_ex.is_csr_instruction = true;
                id_ex.csr_operation = id_ex.funct3;
                id_ex.csr_read_enable = true;

                // CSRRW always writes
                if (id_ex.funct3 == 0x1) {
                    id_ex.csr_write_enable = true;
                }
                // CSRRS only writes if rs1 != x0
                else if (id_ex.funct3 == 0x2) {
                    id_ex.csr_write_enable = (id_ex.rs1 != 0);
                }
            }
        }
        
        // Read Register File
        id_ex.rs1_data = registers[id_ex.rs1];
        id_ex.rs2_data = registers[id_ex.rs2];

        // Mark this stage as valid
        id_ex.valid = true;

        cout << "@" << sc_time_stamp() << " Decode:" << endl;
        cout << "1. opcode: 0x" << hex << id_ex.opcode << dec << endl;
        cout << "2. rd: " << id_ex.rd << endl;
        cout << "3. funct3: " << id_ex.funct3 << endl;
        cout << "4. rs1: " << id_ex.rs1 << endl;
        cout << "5. rs2: " << id_ex.rs2 << endl;
        cout << "6. funct7: " << id_ex.funct7 << endl;
        cout << "7. imm: " << id_ex.imm << endl;
        cout << "8. rs1_data: " << id_ex.rs1_data << endl;
        cout << "9. rs2_data: " << id_ex.rs2_data << endl << endl;
    }

    // EX: Instruction Execute
    // ------------------------------
    void execute() {
        if (mem_stall) {
            return;
        }

        if (!id_ex.valid) {
            ex_mem.pc = 0;
            ex_mem.inst = 0;
            ex_mem.valid = false;
            return;
        }

        cout << "@" << sc_time_stamp() << " Execute: Instruction 0x" << hex << id_ex.inst << dec << " is being executed" << endl << endl;

        // Default ALU inputs
        uint32_t alu_in_1 = id_ex.rs1_data;
        uint32_t alu_in_2 = id_ex.rs2_data;

        // MEM-to_EX Forwarding
        if (mem_wb_old.valid && mem_wb_old.reg_write && (mem_wb_old.rd != 0)) {
            // Forward to rs1
            if (mem_wb_old.rd == id_ex.rs1) {
                alu_in_1 = mem_wb_old.mem_data;
            }

            // Forward to rs2
            if (mem_wb_old.rd == id_ex.rs2) {
                alu_in_2 = mem_wb_old.mem_data;
            }
        }

        // EX-to-EX Forwarding
        if (ex_mem.valid && ex_mem.reg_write && (ex_mem.rd != 0)) {
            // Forward to rs1
            if (ex_mem.rd == id_ex.rs1) {
                alu_in_1 = ex_mem.alu_res;
            }

            // Forward to rs2
            if (ex_mem.rd == id_ex.rs2) {
                alu_in_2 = ex_mem.alu_res;
            }
        }

        // Perform ALU operation
        sc_uint<WIDTH> alu_res = alu(id_ex.opcode, id_ex.funct3, id_ex.funct7, alu_in_1, alu_in_2, id_ex.imm);

        bool branch_taken = false;
        sc_uint<WIDTH> target_pc;

        // Check if branch is taken or not
        if (id_ex.opcode == 0x63) {
            // BEQ
            if (id_ex.funct3 == 0x0) {
                branch_taken = (alu_in_1 == alu_in_2);
                
                cout << "@" << sc_time_stamp() << " Execute: BEQ x" << id_ex.rs1 << "(" << alu_in_1 << "), x" << id_ex.rs2 << "(" << alu_in_2 << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }
            // BNE
            else if (id_ex.funct3 == 0x1) {
                branch_taken = (alu_in_1 != alu_in_2);
                
                cout << "@" << sc_time_stamp() << " Execute: BNE x" << id_ex.rs1 << "(" << alu_in_1 << "), x" << id_ex.rs2 << "(" << alu_in_2 << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }
            // BLT
            else if (id_ex.funct3 == 0x4) {
                branch_taken = ((sc_int<WIDTH>)alu_in_1 < (sc_int<WIDTH>)alu_in_2);
                
                cout << "@" << sc_time_stamp() << " Execute: BLT x" << id_ex.rs1 << "(" << alu_in_1 << "), x" << id_ex.rs2 << "(" << alu_in_2 << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }
            // BGE
            else if (id_ex.funct3 == 0x5) {
                branch_taken = ((sc_int<WIDTH>)alu_in_1 >= (sc_int<WIDTH>)alu_in_2);
                
                cout << "@" << sc_time_stamp() << " Execute: BGE x" << id_ex.rs1 << "(" << alu_in_1 << "), x" << id_ex.rs2 << "(" << alu_in_2 << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }
            // BLTU
            else if (id_ex.funct3 == 0x6) {
                branch_taken = (alu_in_1 < alu_in_2);
                
                cout << "@" << sc_time_stamp() << " Execute: BLTU x" << id_ex.rs1 << "(" << alu_in_1 << "), x" << id_ex.rs2 << "(" << alu_in_2 << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }
            // BGEU
            else if (id_ex.funct3 == 0x7) {
                branch_taken = (alu_in_1 >= alu_in_2);
                
                cout << "@" << sc_time_stamp() << " Execute: BGEU x" << id_ex.rs1 << "(" << alu_in_1 << "), x" << id_ex.rs2 << "(" << alu_in_2 << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }

            // Find next instruction address
            if (branch_taken) {
                target_pc = id_ex.imm + id_ex.imm;
                cout << " | Target: 0x" << hex << target_pc << dec << endl << endl;
            }
        }
        // For JAL calculate the return address and the PC value
        else if (id_ex.opcode == 0x6F) {
            branch_taken = true;
            target_pc = id_ex.pc + id_ex.imm;
            alu_res = id_ex.pc + 4;
            
            cout << "@" << sc_time_stamp() << " Execute: JAL | Return Address: 0x" << hex << alu_res << " | PC: 0x" << target_pc << dec << endl << endl;
        }
        // For JALR calculate the return address and the PC value
        else if (id_ex.opcode == 0x67) {
            branch_taken = true;
            target_pc = (id_ex.rs1_data + id_ex.imm) & ~1; 
            alu_res = id_ex.pc + 4;
            
            cout << "@" << sc_time_stamp() << " Execute: JALR | Return Address: 0x" << hex << alu_res << " | PC: 0x" << target_pc << dec << endl << endl;
        }

        sc_uint<WIDTH> csr_new = 0;

        // For CSR/MRET Handling 
        if (id_ex.is_mret_instruction || id_ex.is_csr_instruction) {
            // MRET
            if (id_ex.is_mret_instruction) {
                // Restore PC
                target_pc = mepc;
                
                // Enable interrupts again
                mstatus = mstatus | 0x8;
                in_interrupt = false;
                
                cout << "@" << sc_time_stamp() << " Execute: MRET | Return Address: 0x" << hex << target_pc << dec << endl << endl;
            }
            // CSR
            else if (id_ex.is_csr_instruction) {
                sc_uint<WIDTH> csr_old = 0;

                // Read CSR
                if (id_ex.csr_read_enable) {
                    csr_old = read_csr(id_ex.csr_address);
                }
                
                csr_new = csr_old;

                // Calculate New CSR Value
                // CSRRW
                if (id_ex.csr_operation == 0x1) {
                    csr_new = id_ex.rs1_data;
                } 
                // CSRRS
                else if (id_ex.csr_operation == 0x2) {
                    csr_new |= id_ex.rs1_data; 
                }

                // Prepare to write old CSR value to destination register
                alu_res = csr_old;
            }
        }

        // Check if branch is taken or not
        if (branch_taken) {
            // Move to correct instruction and flush pipeline
            pc = target_pc;
            flush = true;
        }

        // Pass signals to EX/MEM Register
        ex_mem.alu_res = alu_res;
        ex_mem.store_data = alu_in_2;
        ex_mem.rd = id_ex.rd;
        ex_mem.opcode = id_ex.opcode;
        ex_mem.funct3 = id_ex.funct3;
        ex_mem.reg_write = id_ex.reg_write;
        
        // Pass CSR control signals to EX/MEM Registers
        ex_mem.is_csr_instruction = id_ex.is_csr_instruction;
        ex_mem.csr_write_enable = id_ex.csr_write_enable;
        ex_mem.csr_address = id_ex.csr_address;
        ex_mem.csr_new_value = csr_new;

        // Mark as valid for MEM stage
        ex_mem.valid = true;
    }

    // MEM: Memory/Peripheral Access
    // ------------------------------
    void memoryAccess() {
        // Check for bubble and pass it to next stage
        if (!ex_mem.valid) {
            mem_wb.pc = 0;
            mem_wb.inst = 0;
            mem_wb.valid = false;
            return;
        }

        // Pass necessary data to next stage register
        mem_wb.alu_res = ex_mem.alu_res;
        mem_wb.rd = ex_mem.rd;
        mem_wb.opcode = ex_mem.opcode;
        mem_wb.funct3 = ex_mem.funct3;
        mem_wb.reg_write = ex_mem.reg_write;

        // Pass CSR
        mem_wb.is_csr_instruction = ex_mem.is_csr_instruction;
        mem_wb.csr_write_enable = ex_mem.csr_write_enable;
        mem_wb.csr_address = ex_mem.csr_address;
        mem_wb.csr_new_value = ex_mem.csr_new_value;
        
        if (ex_mem.opcode == 0x03 || ex_mem.opcode == 0x23) {
            if (!mem_stall) {
                // Stall by one cycle
                mem_stall = true;
                data_addr_bus_o.write(ex_mem.alu_res);

                if (ex_mem.opcode == 0x03) {
                    data_read_en_o.write(true);
                    cout << "@" << sc_time_stamp() << " Memory Access: Requesting Load from 0x" << hex << ex_mem.alu_res << dec << endl << endl;
                } 
                else if (ex_mem.opcode == 0x23) {
                    data_write_en_o.write(true);
                    data_bus_o.write(ex_mem.store_data);
                    cout << "@" << sc_time_stamp() << " Memory Access: Storing to 0x" << hex << ex_mem.alu_res << dec << endl << endl;
                }
                
                mem_wb.pc = 0;
                mem_wb.inst = 0;
                mem_wb.valid = false;
                return; 
            } 
            else {
                // Wait one more cycle to read
                mem_stall = false;
            }
        }
        else {
            mem_stall = false;
            cout << "@" << sc_time_stamp() << " Memory Access: No Memory/Peripheral access needed" << endl << endl;
        }

        // Mark this stage as valid
        mem_wb.valid = true;
    }

    // WB: Write Back
    // ------------------------------
    void writeBack() {
        // Check for bubble
        if (!mem_wb.valid) {
            return;
        }

        sc_uint<WIDTH> write_data = mem_wb.alu_res;

        // Load requested data from Memory
        if (mem_wb.opcode == 0x3) {
            write_data = data_bus_i.read();
            cout << "@" << sc_time_stamp() << " Write Back: Loaded 0x" << hex << write_data << dec << " from memory" << endl << endl;
        }

        // Handle CSRs
        if (mem_wb.is_csr_instruction) {
            // Write new CSR value to selected register
            if (mem_wb.csr_write_enable) {
                write_csr(mem_wb.csr_address, mem_wb.alu_res, mem_wb.csr_new_value);
                cout << "@" << sc_time_stamp() << " Write Back: CSR updated to 0x" << hex << mem_wb.csr_new_value << dec << endl << endl;
            }
        }

        // Write data to Register File
        if (mem_wb.reg_write && mem_wb.rd != 0) {
            registers[mem_wb.rd] = write_data;
            cout << "@" << sc_time_stamp() << " Write Back: Register x" << mem_wb.rd << " updated to 0x" << hex << write_data << dec << endl << endl;
        } 
        else {
            cout << "@" << sc_time_stamp() << " Write Back: No Register File write back" << endl << endl;
        }
    }

    // ------------------------------------------------------------
    // Main Thread
    // ------------------------------------------------------------
    void mainThread() {
        // Reset/initial state logic
        // Reset PC
        pc = 0;

        // Reset Register File
        for (int i = 0; i < WIDTH; i++) {
            registers[i] = 0;
        }

        // Reset output ports
        inst_read_en_o.write(false);
        inst_addr_bus_o.write(0);

        data_write_en_o.write(false);
        data_read_en_o.write(false);
        data_addr_bus_o.write(0);
        data_bus_o.write(0);

        // Reset CSRs
        mstatus = 0x0;
        mie     = 0x0;
        mip     = 0x0;   
        mtvec   = 0x0;
        mepc    = 0x0;
        mcause  = 0x0;
        in_interrupt = false;

        // Reset pipeline valid bits
        if_id.valid = false;
        id_ex.valid = false;
        ex_mem.valid = false;
        mem_wb.valid = false;

        // Wait marking end of reset
        wait();

        // Main loop
        while (true) {
            // Reset memory flags to default
            inst_read_en_o.write(false);
            data_write_en_o.write(false);
            data_read_en_o.write(false);

            // Save old values before they get overwritten
            // For MEM-to-EX forwarding
            mem_wb_old.valid = mem_wb.valid;
            mem_wb_old.reg_write = mem_wb.reg_write;
            mem_wb_old.rd = mem_wb.rd;
            mem_wb_old.opcode = mem_wb.opcode;
            mem_wb_old.alu_res = mem_wb.alu_res;
            mem_wb_old.mem_data = ((mem_wb.opcode == 0x03) ? data_bus_i.read() : mem_wb.alu_res);

            // Disable flush after one cycle (if enabled)
            flush = false;

            // Check for interrupts
            if (irq_timer_i.read() == true) {
                mip = mip | 0x80;   // Set Bit 7 (timer interrupt)
            } 
            else {
                mip = mip & ~0x80;  // Clear Bit 7
            }

            // Handle interrupt if triggered
            if ((mip & 0x80) && (mie & 0x80) && (mstatus & 0x8) && in_interrupt == false) {
                mepc = pc;                  // Save PC value
                mcause = 0x80000007;        // Set cause as timer interrupt
                mstatus = mstatus & ~0x8;   // Disable global interrupts
                in_interrupt = true;
                
                pc = mtvec;     // Move to interrupt handling address
                flush = true;   // Flush entire pipeline

                cout << "@" << sc_time_stamp() << " CPU: Timer interrupt received" << endl;
                cout << "@" << sc_time_stamp() << " CPU: Jumping to interrupt handler\n" << endl << endl;
            }

            writeBack();
            memoryAccess();
            execute();
            decode();
            fetch();

            wait();
        }
    }

    SC_CTOR(risc_v_model) {
        SC_CTHREAD(mainThread, clk_i.pos());
        reset_signal_is(rst_i, true);
    }
};