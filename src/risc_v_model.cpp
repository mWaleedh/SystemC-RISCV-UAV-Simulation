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

    // Data input ports
    sc_in<bool> data_ready_i;
    sc_in<bool> data_error_i;
    sc_in<sc_uint<WIDTH>> data_bus_i;
    // Data output ports
    sc_out<bool> data_write_en_o;
    sc_out<bool> data_read_en_o;
    sc_out<sc_uint<3>> data_size_o;
    sc_out<sc_uint<WIDTH>> data_bus_o;
    sc_out<sc_uint<WIDTH>> data_addr_bus_o;

    // IF/ID Register
    struct IF_ID {
        sc_uint<WIDTH> pc;
        sc_uint<WIDTH> inst;
        bool valid;
        bool predicted_taken;
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
        
        sc_uint<WIDTH> alu_in_1;
        sc_uint<WIDTH> alu_in_2;
        sc_int<WIDTH> imm;

        uint32_t mem_size;
        bool mem_unsigned;

        bool predicted_taken;

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
        sc_uint<5> rd;
        
        sc_uint<WIDTH> alu_res;
        sc_uint<WIDTH> store_data;

        uint32_t mem_size;
        bool mem_unsigned;

        bool reg_write;

        // CSR Signals for Write Back
        bool is_csr_instruction;
        bool csr_write_enable;
        sc_uint<WIDTH> csr_address;
        sc_uint<WIDTH> csr_new_value;
    } ex_mem;

    // MEM/WB Register
    struct MEM_WB {
        sc_uint<WIDTH> pc;
        sc_uint<WIDTH> inst;
        bool valid;
        
        sc_uint<5> rd;
        sc_uint<WIDTH> alu_res;

        bool reg_write;

        // CSR Signals for Write Back
        bool is_csr_instruction;
        bool csr_write_enable;
        sc_uint<WIDTH> csr_address;
        sc_uint<WIDTH> csr_new_value;
    } mem_wb;

    struct MEM_WB_Old {
        bool valid;        
        bool reg_write;
        sc_uint<5> rd;
        sc_uint<WIDTH> alu_res;
    } mem_wb_old;

    enum mem_state_t { 
        IDLE, 
        REQUEST_SENT, 
        WAITING_FOR_RESPONSE, 
        RESPONSE_RECEIVED 
    } mem_state;

    // Global PC values
    sc_uint<WIDTH> pc;          // Holds PC of instruction we are requesting
    sc_uint<WIDTH> pc_stage_1;  // Holds PC of instruction whose request has been sent
    sc_uint<WIDTH> pc_stage_2;  // Holds PC of instruction arriving from Memory

    sc_uint<WIDTH> fetch_buffer;    // Holds data arriving in IF in case of stall
    bool buffer_full;

    // Pipeline Control Signals
    bool stall;
    bool mem_stall;
    bool branch_flush;
    bool trap_flush;
    bool ignore_fetch;

    uint8_t bht[32];

    // Register File
    sc_uint<WIDTH> registers[WIDTH];

    // CSRs
    sc_uint<WIDTH> mstatus;
    sc_uint<WIDTH> mie;
    sc_uint<WIDTH> mip;
    sc_uint<WIDTH> mtvec;
    sc_uint<WIDTH> mepc;
    sc_uint<WIDTH> mcause;

    // Performance Counters
    uint64_t total_cycles;
    uint64_t committed_instructions;
    uint64_t pipeline_stalls;
    uint64_t pipeline_flushes;
    uint64_t branches_executed;
    uint64_t branches_taken;
    uint64_t branch_mispredictions;
    uint64_t timer_interrupts;
    uint64_t sw_interrupts;

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

    void write_csr(uint32_t csr_addr, uint32_t csr_new) {
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
                // Protect 3rd and 7th bit of MIP from write
                mip = (csr_new & ~0x88) | (mip & 0x88);
            default: 
                break; 
        }        
    }

    void performanceStats() {
        double cpi = (double)total_cycles / committed_instructions;
        double ipc = (double)committed_instructions / total_cycles;
        double stall_rate = ((double)pipeline_stalls / total_cycles) * 100.0;
        double mispredict_rate = branches_executed > 0 ? ((double)branch_mispredictions / branches_executed) * 100.0 : 0.0;

        cout << "-----{ Pipeline Performance Stats }-----" << endl;
        cout << "Total Cycles:           " << total_cycles << endl;
        cout << "Committed Instructions: " << committed_instructions << endl;
        cout << "CPI:                    " << cpi << endl;
        cout << "IPC:                    " << ipc << endl;
        cout << "----------------------------------------" << endl;
        cout << "Pipeline Stalls:        " << pipeline_stalls << " (" << stall_rate << "% of cycles)" << endl;
        cout << "Pipeline Flushes:       " << pipeline_flushes << endl;
        cout << "----------------------------------------" << endl;
        cout << "Branches Executed:      " << branches_executed << endl;
        cout << "Branches Taken:         " << branches_taken << endl;
        cout << "Mispredictions:         " << branch_mispredictions << " (" << mispredict_rate << "% miss rate)" << endl;
        cout << "Timer Interrupts:       " << timer_interrupts << endl;
        cout << "----------------------------------------\n" << endl;
    }

    void passDataToWB() {
        mem_wb.valid = true;

        mem_wb.alu_res = ex_mem.alu_res;
        mem_wb.rd = ex_mem.rd;
        mem_wb.reg_write = ex_mem.reg_write;

        mem_wb.is_csr_instruction = ex_mem.is_csr_instruction;
        mem_wb.csr_write_enable = ex_mem.csr_write_enable;
        mem_wb.csr_address = ex_mem.csr_address;
        mem_wb.csr_new_value = ex_mem.csr_new_value;        
    }

    void saveOldWB() {
        mem_wb_old.valid = mem_wb.valid;

        mem_wb_old.reg_write = mem_wb.reg_write;
        mem_wb_old.rd = mem_wb.rd;
        mem_wb_old.alu_res = mem_wb.alu_res;
    }

    // ------------------------------------------------------------
    // Pipeline
    // ------------------------------------------------------------

    // IF: Instruction Fetch
    // ------------------------------
    void fetch() {
        // Check if Stall is triggered
        if (stall || mem_stall) {
            // Load arriving instruction into buffer
            if (!buffer_full) {
                fetch_buffer = inst_bus_i.read();
                buffer_full = true;

                cout << "@" << sc_time_stamp() << " Fetch: Stalled. Stored Inst -> 0x" << hex << fetch_buffer << " | PC -> 0x" << pc_stage_2 << dec  << " in buffer" << endl;
            }
            else {
                cout << "@" << sc_time_stamp() << " Fetch: Stalled. Holding Inst -> 0x" << hex << fetch_buffer << " | PC -> 0x" << pc_stage_2 << dec  << " in buffer" << endl;
            }

            // Request the next instruction again so it arrives in time (2-cycle delay)
            inst_addr_bus_o.write(pc_stage_1);
            inst_read_en_o.write(true);
            
            cout << "@" << sc_time_stamp() << " Fetch: Stalled. Re-requesting PC -> 0x" << hex << pc_stage_1 << dec << endl << endl;
            return;
        }

        // Read instruction sent by memory
        sc_uint<WIDTH> inst;
        if (!buffer_full) {
            inst = inst_bus_i.read();
            cout << "@" << sc_time_stamp() << " Fetch: Received Inst -> 0x" << hex << inst << " | PC -> 0x" << pc_stage_2 << dec << " from Memory" << endl << endl;
        }
        else {
            // Read buffer instead of data_bus
            inst = fetch_buffer;
            buffer_full = false;
            cout << "@" << sc_time_stamp() << " Fetch: Using Inst -> 0x" << hex << inst << " | PC -> 0x" << pc_stage_2 << dec << " from buffer" << endl << endl;
        }

        // Check for flush
        if (trap_flush || branch_flush || ignore_fetch) {            
            if_id.valid = false;
            if_id.pc = 0;
            if_id.inst = 0;

            if (ignore_fetch && !trap_flush && !branch_flush) {
                cout << "@" << sc_time_stamp() << " Fetch: Ignored Inst -> 0x" << hex << inst << dec  << " (Invalid)" << endl << endl;
                ignore_fetch = false;
            } 
            else {
                cout << "@" << sc_time_stamp() << " Fetch: Flushed Inst -> 0x" << hex << inst << dec << endl << endl;
            }
            
            // Turn flushes off
            trap_flush = false; 
            branch_flush = false;

            // Clear buffer as it's holding an invalid instruction
            buffer_full = false;
        }
        else {
            // Pass data to IF/ID register
            if_id.pc = pc_stage_2;
            if_id.inst = inst;
            if_id.valid = true;

            if_id.predicted_taken = false;

            // If it's a branch predict its outcome
            if ((inst & 0x7F) == 0x63) {
                uint32_t bht_index = (pc_stage_2 >> 2) & 0x1F;
                
                if (bht[bht_index] >= 2) {
                    if_id.predicted_taken = true;
                    
                    // Extract Immediate and redirect PC
                    sc_int<WIDTH> imm = immediateGenerator(0x63, inst);
                    pc = pc_stage_2 + imm;

                    // Stall IF stage so that we ignore the wrong instruction coming from memory
                    ignore_fetch = true;
                    
                    cout << "@" << sc_time_stamp() << " Fetch: BHT Predicted Branch Taken. Redirecting PC to 0x" << hex << pc << dec << endl << endl;
                }
                else {
                    cout << "@" << sc_time_stamp() << " Fetch: BHT Predicted Branch Not Taken. Continuing sequential execution" << endl << endl;   
                }
            }
        }

        // Request the instruction needed in the next cycle
        inst_addr_bus_o.write(pc);
        inst_read_en_o.write(true);

        pc_stage_2 = pc_stage_1;
        pc_stage_1 = pc;
        
        cout << "@" << sc_time_stamp() << " Fetch: Requested next PC -> 0x" << hex << pc << dec << endl << endl;

        // Move to next instruction
        pc = pc + 4;
    }

    // ID: Instruction Decode
    // ------------------------------
    void decode() {
        if (mem_stall) {
            cout << "@" << sc_time_stamp() << " Decode: Stalled. Holding PC at -> 0x" << hex << if_id.pc << dec << endl << endl;
            return;
        }

        // Check for bubble and forward it
        if (!if_id.valid) {
            id_ex.valid = false;

            if (branch_flush || trap_flush) {
                cout << "@" << sc_time_stamp() << " Decode: Flushed inst -> 0x" << hex << if_id.inst << dec << endl << endl;
            }
            else {
                cout << "@" << sc_time_stamp() << " Decode: Passed bubble forward" << endl << endl;
            }
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
                pipeline_stalls++;  // Increment counter
                
                // Insert bubble in ID/EX Register
                id_ex.valid = false;

                cout << "@" << sc_time_stamp() << " Decode: Stalling pipeline due to load-use hazard" << endl << endl;
                return;
            }
        }

        // Clear stall
        stall = false;

        cout << "@" << sc_time_stamp() << " Decode: Received Inst -> 0x" << hex << inst << dec << " from IF stage" << endl << endl;

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
            id_ex.valid = false;
            cout << "@" << sc_time_stamp() << " Decode Warning: Invalid Instruction. Bubble inserted into ID/EX" << endl << endl;
            return;
        }

        // Enable reg_write by default
        id_ex.reg_write = true;

        // Only store and branch don't write to register file
        // So don't forward their data to previous stage
        if (id_ex.opcode == 0x23 || id_ex.opcode == 0x63) {
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
        
        // Read Register File and set them as default values for forwarding
        id_ex.alu_in_1 = registers[id_ex.rs1];
        id_ex.alu_in_2 = registers[id_ex.rs2];

        // Default memory load/store values
        id_ex.mem_size = 4;
        id_ex.mem_unsigned = false;

        // Check if opcode is Load/Store
        if (id_ex.opcode == 0x03 || id_ex.opcode == 0x23) {
            switch (id_ex.funct3) {
                 // LB / SB
                case 0x0:
                    id_ex.mem_size = 1;
                    id_ex.mem_unsigned = false;
                    break;
                // LH / SH
                case 0x1: 
                    id_ex.mem_size = 2;
                    id_ex.mem_unsigned = false;
                    break;
                // LW / SW
                case 0x2:
                    id_ex.mem_size = 4;
                    id_ex.mem_unsigned = false;
                    break;
                // LBU
                case 0x4:
                    id_ex.mem_size = 1;
                    id_ex.mem_unsigned = true;
                    break;
                 // LHU
                case 0x5:
                    id_ex.mem_size = 2;
                    id_ex.mem_unsigned = true;
                    break;
                default:
                    id_ex.mem_size = 4;
                    id_ex.mem_unsigned = false;
                    break;
            }
        }

        // Pass prediction
        id_ex.predicted_taken = if_id.predicted_taken;

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
        cout << "8. rs1_data: " << id_ex.alu_in_1 << endl;
        cout << "9. rs2_data: " << id_ex.alu_in_2 << endl << endl;
    }

    // EX: Instruction Execute
    // ------------------------------
    void execute() {
        cout << "@" << sc_time_stamp() << " Execute: Instruction 0x" << hex << id_ex.inst << dec << " is being executed" << endl << endl;

        // We forward data first so that values don't get lost in case of stall
        // MEM-to_EX Forwarding
        if (mem_wb_old.valid && mem_wb_old.reg_write && (mem_wb_old.rd != 0)) {
            // Forward to rs1
            if (mem_wb_old.rd == id_ex.rs1) {
                id_ex.alu_in_1 = mem_wb_old.alu_res;
                cout << "@" << sc_time_stamp() << " Execute: Forwarded rd value to rs1 from MEM stage" << endl << endl;
            }

            // Forward to rs2
            if (mem_wb_old.rd == id_ex.rs2) {
                id_ex.alu_in_2 = mem_wb_old.alu_res;
                cout << "@" << sc_time_stamp() << " Execute: Forwarded rd value to rs2 from MEM stage" << endl << endl;
            }
        }

        // EX-to-EX Forwarding
        if (ex_mem.valid && ex_mem.reg_write && (ex_mem.rd != 0)) {
            // Forward to rs1
            if (ex_mem.rd == id_ex.rs1) {
                id_ex.alu_in_1 = ex_mem.alu_res;
                cout << "@" << sc_time_stamp() << " Execute: Forwarded rd value to rs1 from EX stage" << endl << endl;
            }

            // Forward to rs2
            if (ex_mem.rd == id_ex.rs2) {
                id_ex.alu_in_2 = ex_mem.alu_res;
                cout << "@" << sc_time_stamp() << " Execute: Forwarded rd value to rs2 from EX stage" << endl << endl;
            }
        }

        if (mem_stall) {
            cout << "@" << sc_time_stamp() << " Execute: Stalled. Holding PC at -> 0x" << hex << id_ex.pc << dec << endl << endl;
            return;
        }

        // Pass bubble forward
        if (!id_ex.valid) {
            ex_mem.valid = false;

            if (trap_flush) {
                cout << "@" << sc_time_stamp() << " Execute: Flushed inst -> " << hex << id_ex.inst << dec << endl << endl;
            }
            else {
               cout << "@" << sc_time_stamp() << " Execute: Passed bubble forward" << endl << endl; 
            }
            return;
        }

        // Perform ALU operation
        sc_uint<WIDTH> alu_res = alu(id_ex.opcode, id_ex.funct3, id_ex.funct7, id_ex.alu_in_1, id_ex.alu_in_2, id_ex.imm);

        bool branch_taken = false;
        sc_uint<WIDTH> target_pc;

        // Check if branch is taken or not
        if (id_ex.opcode == 0x63) {
            // Increment counter
            branches_executed++;

            // BEQ
            if (id_ex.funct3 == 0x0) {
                branch_taken = (id_ex.alu_in_1 == id_ex.alu_in_2);
                
                cout << "@" << sc_time_stamp() << " Execute: BEQ x" << id_ex.rs1 << "(" << id_ex.alu_in_1 << "), x" << id_ex.rs2 << "(" << id_ex.alu_in_2 << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }
            // BNE
            else if (id_ex.funct3 == 0x1) {
                branch_taken = (id_ex.alu_in_1 != id_ex.alu_in_2);
                
                cout << "@" << sc_time_stamp() << " Execute: BNE x" << id_ex.rs1 << "(" << id_ex.alu_in_1 << "), x" << id_ex.rs2 << "(" << id_ex.alu_in_2 << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }
            // BLT
            else if (id_ex.funct3 == 0x4) {
                branch_taken = ((sc_int<WIDTH>)id_ex.alu_in_1 < (sc_int<WIDTH>)id_ex.alu_in_2);
                
                cout << "@" << sc_time_stamp() << " Execute: BLT x" << id_ex.rs1 << "(" << id_ex.alu_in_1 << "), x" << id_ex.rs2 << "(" << id_ex.alu_in_2 << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }
            // BGE
            else if (id_ex.funct3 == 0x5) {
                branch_taken = ((sc_int<WIDTH>)id_ex.alu_in_1 >= (sc_int<WIDTH>)id_ex.alu_in_2);
                
                cout << "@" << sc_time_stamp() << " Execute: BGE x" << id_ex.rs1 << "(" << id_ex.alu_in_1 << "), x" << id_ex.rs2 << "(" << id_ex.alu_in_2 << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }
            // BLTU
            else if (id_ex.funct3 == 0x6) {
                branch_taken = (id_ex.alu_in_1 < id_ex.alu_in_2);
                
                cout << "@" << sc_time_stamp() << " Execute: BLTU x" << id_ex.rs1 << "(" << id_ex.alu_in_1 << "), x" << id_ex.rs2 << "(" << id_ex.alu_in_2 << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }
            // BGEU
            else if (id_ex.funct3 == 0x7) {
                branch_taken = (id_ex.alu_in_1 >= id_ex.alu_in_2);
                
                cout << "@" << sc_time_stamp() << " Execute: BGEU x" << id_ex.rs1 << "(" << id_ex.alu_in_1 << "), x" << id_ex.rs2 << "(" << id_ex.alu_in_2 << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }

            // Extract correct index and read its value
            uint32_t bht_index = (id_ex.pc >> 2) & 0x1F;
            uint8_t current_state = bht[bht_index];
            
            // Update BHT
            if (branch_taken) {
                // Increment count towards strongly taken
                if (current_state < 3) {
                    bht[bht_index]++;
                }
                branches_taken++;
            } 
            else {
                // Decrement count towards strongly not taken
                if (current_state > 0) {
                    bht[bht_index]--;
                }
            }

            // Check if prediction was correct or not
            if (branch_taken != id_ex.predicted_taken) {
                branch_mispredictions++;
                
                // Calculate actual PC value
                target_pc = branch_taken ? (id_ex.pc + id_ex.imm) : (id_ex.pc + 4);

                branch_taken = true;    // Give signal to flush pipeline
                
                cout << " | Mispredicted. Correct target 0x" << hex << target_pc << dec;
            } 
            else {
                // Don't flush pipeline since prediction is correct
                branch_taken = false;
                cout << " | Prediction correct";
            }
            cout << endl << endl;
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
            target_pc = (id_ex.alu_in_1 + id_ex.imm) & ~1; 
            alu_res = id_ex.pc + 4;
            
            cout << "@" << sc_time_stamp() << " Execute: JALR | Return Address: 0x" << hex << alu_res << " | PC: 0x" << target_pc << dec << endl << endl;
        }

        sc_uint<WIDTH> csr_new = 0;

        // For CSR/MRET Handling 
        if (id_ex.is_mret_instruction || id_ex.is_csr_instruction) {
            // MRET
            if (id_ex.is_mret_instruction) {
                target_pc = mepc;           // Restore PC
                mstatus = mstatus | 0x8;    // Enable global interrupts
                branch_taken = true;        // Flush pipeline
                
                cout << "@" << sc_time_stamp() << " Execute: MRET | Return Address: 0x" << hex << target_pc << dec << endl << endl;
            }
            // CSR
            else if (id_ex.is_csr_instruction) {
                sc_uint<WIDTH> csr_old = 0;

                // Read CSR
                if (id_ex.csr_read_enable) {
                    csr_old = read_csr(id_ex.csr_address);

                    // Forward old CSR value if needed
                    if (ex_mem.valid && ex_mem.is_csr_instruction && (ex_mem.csr_address == id_ex.csr_address)) {
                        csr_old = ex_mem.csr_new_value;
                        cout << "@" << sc_time_stamp() << " Execute: Forwarded old CSR value from EX stage" << endl << endl;
                    }
                }
                
                csr_new = csr_old;

                // Calculate New CSR Value
                // CSRRW
                if (id_ex.csr_operation == 0x1) {
                    csr_new = id_ex.alu_in_1;
                } 
                // CSRRS
                else if (id_ex.csr_operation == 0x2) {
                    csr_new |= id_ex.alu_in_1;
                }

                // Prepare to write old CSR value to destination register
                alu_res = csr_old;
            }
        }

        // Check if branch is taken or not
        if (branch_taken) {
            // Flush pipeline
            branch_flush = true;
            ignore_fetch = true;
            if_id.valid = false;

            // Increment counter
            pipeline_flushes++;

            // Move to correct instruction
            pc = target_pc;

            cout << "@" << sc_time_stamp() << " Execute: Flushed pipeline due to Branch/Jump/MRET" << endl;
            cout << "@" << sc_time_stamp() << " Execute: Jumping to correct instruction"<< endl << endl;
        }

        // Pass signals to EX/MEM Register
        ex_mem.alu_res = alu_res;
        ex_mem.store_data = id_ex.alu_in_2;
        ex_mem.rd = id_ex.rd;
        ex_mem.opcode = id_ex.opcode;
        ex_mem.reg_write = id_ex.reg_write;
        ex_mem.mem_size = id_ex.mem_size;
        ex_mem.mem_unsigned = id_ex.mem_unsigned;
        
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
        switch (mem_state) {
            case IDLE:
                // Check for bubble
                if (!ex_mem.valid) {
                    mem_wb.valid = false;
                    mem_stall = false;

                    cout << "@" << sc_time_stamp() << " Memory Access: Passed bubble forward" << endl << endl; 
                    return;
                }

                // Check if it's a Load or Store
                if (ex_mem.opcode == 0x03 || ex_mem.opcode == 0x23) {
                    // Send address over bus along with size of data
                    data_addr_bus_o.write(ex_mem.alu_res);
                    data_size_o.write(ex_mem.mem_size);

                    // Enable read_en for Load
                    if (ex_mem.opcode == 0x03) {
                        data_read_en_o.write(true);
                        cout << "@" << sc_time_stamp() << " Memory Access: Requesting Load from 0x" << hex << ex_mem.alu_res << dec << endl << endl;
                    } 
                    // Enable write_en and send data to bus for Store
                    else if (ex_mem.opcode == 0x23) {
                        data_write_en_o.write(true);
                        data_bus_o.write(ex_mem.store_data);
                        cout << "@" << sc_time_stamp() << " Memory Access: Storing data to 0x" << hex << ex_mem.alu_res << dec << endl << endl;
                    }
                    
                    // Freeze pipeline until response is received
                    mem_stall = true;
                    pipeline_stalls++;

                    cout << "@" << sc_time_stamp() << " Memory Access: Freezing pipeline until Memory responds" << endl;
                    cout << "@" << sc_time_stamp() << " Memory Access: Bubble sent to Write Back stage" << endl << endl;
                    
                    // Send bubble to the WB stage
                    mem_wb.valid = false;
                    
                    // Move to next state
                    mem_state = REQUEST_SENT;
                }
                else {                
                    mem_stall = false;
                    cout << "@" << sc_time_stamp() << " Memory Access: No Memory/Peripheral access needed" << endl << endl;
                    
                    // Send data to WB stage
                    passDataToWB();
                }
                break;

            case REQUEST_SENT:
                // After one cycle clear the read/write flags
                data_read_en_o.write(false);
                data_write_en_o.write(false);
                
                // Send bubble to WB
                mem_wb.valid = false;

                // Increment counter
                pipeline_stalls++;
                
                cout << "@" << sc_time_stamp() << " Memory Access: Load/Store request sent to memory" << endl;
                cout << "@" << sc_time_stamp() << " Read/Write enable flags have been set to false" << endl << endl;

                mem_state = WAITING_FOR_RESPONSE;
                break;

            case WAITING_FOR_RESPONSE:
                // Keep sending bubbles to WB until we get a response
                mem_wb.valid = false;

                // Check if data has been sent
                if (data_ready_i.read() == true) {
                    // Send data to WB stage
                    passDataToWB();

                    // Pass received data to WB if Load
                    if (ex_mem.opcode == 0x03) {
                        uint32_t loaded_data = data_bus_i.read(); 

                        // Byte Load (LB / LBU)
                        if (ex_mem.mem_size == 1) {
                            mem_wb.alu_res = loaded_data & 0xFF; 
                            
                            // Sign extend if signed and MSB MSB is 1
                            if (!ex_mem.mem_unsigned && (mem_wb.alu_res & 0x80)) {
                                mem_wb.alu_res |= 0xFFFFFF00; 
                            }
                        }
                        // Halfword Load (LH / LHU)
                        else if (ex_mem.mem_size == 2) {
                            mem_wb.alu_res = loaded_data & 0xFFFF; 
                            
                            // Sign extend if signed and MSB is 1
                            if (!ex_mem.mem_unsigned && (mem_wb.alu_res & 0x8000)) {
                                mem_wb.alu_res |= 0xFFFF0000;
                            }
                        }
                        else {
                            mem_wb.alu_res = loaded_data;
                        }

                        cout << "@" << sc_time_stamp() << " Memory Access: Successfully loaded 0x" << hex << mem_wb.alu_res << dec << " from memory" << endl << endl;
                    }
                    else if (data_error_i.read()) {
                        // Load access fault
                        if (ex_mem.opcode == 0x03) {
                            mcause = 5;
                        } 
                        // Store/AMO access fault
                        else {
                            mcause = 7;
                        }
                    }
                    else {
                        cout << "@" << sc_time_stamp() << " Memory Access: Successfully stored data at 0x" << hex << ex_mem.alu_res << dec << endl << endl;
                    }

                    // Clear stall since data has been received
                    mem_stall = false;

                    // Change state back to Idle
                    mem_state = IDLE;
                }
                else {
                    // Increment counter
                    pipeline_stalls++;
                    cout << "@" << sc_time_stamp() << " Memory Access: Waiting for response from memory" << endl << endl;
                }
                break;
        }
    }

    // WB: Write Back
    // ------------------------------
    void writeBack() {
        // Check for bubble
        if (!mem_wb.valid) {
            cout << "@" << sc_time_stamp() << " Write Back: Bubble was received" << endl << endl;
            return;
        }

        // Increment counter
        committed_instructions++;

        // Handle CSRs
        if (mem_wb.is_csr_instruction) {
            // Write new CSR value to selected register
            if (mem_wb.csr_write_enable) {
                write_csr(mem_wb.csr_address, mem_wb.csr_new_value);
                cout << "@" << sc_time_stamp() << " Write Back: CSR | Address: 0x" << hex << mem_wb.csr_address << " | Old: 0x" << mem_wb.alu_res << " | New: 0x" << mem_wb.csr_new_value << dec << endl << endl;
            }
        }

        // Write data to Register File
        if (mem_wb.reg_write && mem_wb.rd != 0) {
            registers[mem_wb.rd] = mem_wb.alu_res;
            cout << "@" << sc_time_stamp() << " Write Back: Register x" << mem_wb.rd << " updated to 0x" << hex << mem_wb.alu_res << dec << endl << endl;
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
        // Reset PC values
        pc = 0;
        pc_stage_1 = 0;
        pc_stage_2 = 0;

        // Reset Fetch stage buffer
        fetch_buffer = 0;
        buffer_full = false;

        // Reset Register File
        for (int i = 0; i < WIDTH; i++) {
            registers[i] = 0;
        }

        // Reset output ports
        // Instruction ports
        inst_read_en_o.write(false);
        inst_addr_bus_o.write(0);
        // Data ports
        data_write_en_o.write(false);
        data_read_en_o.write(false);
        data_addr_bus_o.write(0);
        data_bus_o.write(0);
        data_size_o.write(0);

        // Reset CSRs
        mstatus = 0x8;      // Enable global interrupts
        mie = 0x88;         // Enable Timer (Bit 7) and Software (Bit 3) interrupts
        mip = 0x0;
        mtvec = 0x0;
        mepc = 0x0;
        mcause = 0x0;

        // Reset pipeline valid bits
        if_id.valid = false;
        id_ex.valid = false;
        ex_mem.valid = false;
        mem_wb.valid = false;

        // Reset pipeline control signals
        stall = false;
        mem_stall = false;
        branch_flush = false;
        trap_flush = false;
        ignore_fetch = false;

        // Reset Performance Counters
        total_cycles = 0;
        committed_instructions = 0;
        pipeline_stalls = 0;
        pipeline_flushes = 0;
        branches_executed = 0;
        branches_taken = 0;
        branch_mispredictions = 0;
        timer_interrupts = 0;
        sw_interrupts = 0;

        // Reset Branch History Table
        for (int i = 0; i < 32; i++) {
            bht[i] = 0;
        }

        // Reset MEM stage state
        mem_state = IDLE;

        // Wait marking end of reset
        wait();

        // Main loop
        while (true) {
            // Increment counter
            total_cycles++;

            // Reset memory flags to default
            inst_read_en_o.write(false);
            data_write_en_o.write(false);
            data_read_en_o.write(false);

            // Save old values before they get overwritten (for MEM-EX forwarding)
            saveOldWB();

            // Check for software interrupts
            if (irq_sw_i.read() == true) {
                mip = mip | 0x8;    // Set Bit 3 (software interrupt)
            }
            else  {
                mip = mip & ~0x8;   // Clear Bit 3 (software interrupt)
            }

            // Check for timer interrupts
            if (irq_timer_i.read() == true) {
                mip = mip | 0x80;   // Set Bit 7 (timer interrupt)    
            }
            else {
                mip = mip & ~0x80;  // Clear Bit 7 (timer interrupt)
            }

            // Handle software interrupt if triggered (Higher priority)
            if ((mip & 0x8) && (mie & 0x8) && (mstatus & 0x8) && id_ex.valid) {
                // Increment counters
                sw_interrupts++;
                pipeline_flushes++;

                mepc = id_ex.pc;            // Save PC value of instruction that was sent to EX
                mcause = 0x80000003;        // Set cause as software interrupt
                mstatus = mstatus & ~0x8;   // Disable global interrupts
                pc = mtvec;                 // Move to interrupt handling address

                // Flush pipeline
                trap_flush = true;
                if_id.valid = false;
                id_ex.valid = false;

                // Ignore incorrect instruction coming from Memory
                ignore_fetch = true;

                cout << "@" << sc_time_stamp() << " CPU: Software interrupt received. Flushed pipeline" << endl;
                cout << "@" << sc_time_stamp() << " CPU: Jumping to interrupt handler\n" << endl;
            }
            // Handle timer interrupt if triggered (Lower priority)
            else if ((mip & 0x80) && (mie & 0x80) && (mstatus & 0x8) && id_ex.valid) {
                // Increment counters
                timer_interrupts++;
                pipeline_flushes++;

                mepc = id_ex.pc;            // Save PC value of instruction that was sent to EX
                mcause = 0x80000007;        // Set cause as timer interrupt
                mstatus = mstatus & ~0x8;   // Disable global interrupts
                pc = mtvec;                 // Move to interrupt handling address

                // Flush pipeline
                trap_flush = true;
                if_id.valid = false;
                id_ex.valid = false;

                // Ignore incorrect instruction coming from Memory
                ignore_fetch = true;

                cout << "@" << sc_time_stamp() << " CPU: Timer interrupt received. Flushed pipeline" << endl;
                cout << "@" << sc_time_stamp() << " CPU: Jumping to interrupt handler\n" << endl;
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