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
    sc_in<sc_uint<WIDTH>> data_bus_i;

    // output ports
    sc_out<bool> write_en_o;
    sc_out<bool> read_en_o;
    sc_out<sc_uint<WIDTH>> addr_bus_o;
    sc_out<sc_uint<WIDTH>> data_bus_o;

    // local variables
    sc_uint<WIDTH> pc;
    sc_uint<WIDTH> cur_inst;
    sc_uint<WIDTH> registers[WIDTH];
    sc_uint<7> opcode;
    sc_uint<5> rd;
    sc_uint<3> funct3;
    sc_uint<5> rs1;
    sc_uint<5> rs2;    
    sc_uint<7> funct7;
    sc_uint<WIDTH> rs1_data;
    sc_uint<WIDTH> rs2_data;
    sc_int<WIDTH> imm;
    sc_uint<WIDTH> alu_res;
    sc_uint<WIDTH> mem_data;
    sc_uint<WIDTH> pc_next;
    bool branch_taken;
    bool is_valid_inst;
    // CSRs
    sc_uint<WIDTH> mstatus;
    sc_uint<WIDTH> mie;
    sc_uint<WIDTH> mip;
    sc_uint<WIDTH> mtvec;
    sc_uint<WIDTH> mepc;
    sc_uint<WIDTH> mcause;
    // CSR/MRET signals
    bool is_mret_instruction;
    bool is_csr_instruction;
    bool csr_read_enable;
    bool csr_write_enable;
    sc_uint<WIDTH> csr_address;
    sc_uint<WIDTH> csr_old_value;
    sc_uint<WIDTH> csr_new_value;
    sc_uint<WIDTH> csr_operation;
    bool csr_register_write_enable;

    // ------------------------------------------------------------
    // Helper Functions
    // ------------------------------------------------------------

    sc_int<WIDTH> immediateGenerator() {
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
            immediate = (cur_inst >> 20 ) & 0xFFF;

            // Sign extension
            if (immediate & 0x800) { 
                // [31:12]
                immediate |= 0xFFFFF000; 
            }
            break;

        // S-type
        case 0x23:
            // [11:5]
            immediate = ((cur_inst >> 25 ) & 0x7F) << 5;
            // [4:0]
            immediate |= (cur_inst >> 7) & 0x1F;

            // Sign extension
            if (immediate & 0x800) { 
                // [31:12]
                immediate |= 0xFFFFF000; 
            }
            break;

        // B-type
        case 0x63:
            // [12]
            immediate = ((cur_inst >> 31) & 0x1) << 12;
            // [11]
            immediate |= ((cur_inst >> 7) & 0x1) << 11;
            // [10:5]
            immediate |= ((cur_inst >> 25) & 0x3F) << 5;
            // [4:1]
            immediate |= ((cur_inst >> 8) & 0xF) << 1;

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
            immediate = ((cur_inst >> 12) & 0xFFFFF) << 12;
            break;

        // J-type
        case 0x6F:        
            // [20]
            immediate = ((cur_inst >> 31) & 0x1) << 20;
            // [19:12]
            immediate |= ((cur_inst >> 12) & 0xFF) << 12;
            // [11]
            immediate |= ((cur_inst >> 20) &0x1) << 11;
            // [10:1]
            immediate |= ((cur_inst >> 21) & 0x3FF) << 1;

            // Sign extension
            if (immediate & 0x100000) {
                // [31:21]
                immediate |= 0xFFF00000; 
            }
            break;

        // CSR/MRET
        case 0x73:
            // [31:20]
            immediate = (cur_inst >> 20) & 0xFFF;
            break;
            
        default:
            immediate = 0;
            is_valid_inst = false;
            cout << "Warning: Invalid instruction type, skipping to next instruction" << endl << endl;
        }

        return immediate;
    }

    sc_uint<WIDTH> alu() {
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
                
            }
            // SLTU
            else if (funct3 == 0x3) {
                
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
                
            }
            // SLTIU
            else if (funct3 == 0x3) {
                
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
        // Send program counter value to memory
        addr_bus_o.write(pc);
        read_en_o.write(true);

        cout << "@" << sc_time_stamp() << " Fetch: PC -> 0x" << hex << pc << dec << endl << endl;

        wait();

        read_en_o.write(false);

        wait();

        // Read instruction sent by memory
        cur_inst = data_bus_i.read();
    }

    // ID: Instruction Decode
    // ------------------------------
    void decode() {
        cout << "@" << sc_time_stamp() << " Decode: Instruction -> 0x" << hex << cur_inst << dec << endl << endl;

        // Divide instruction into required parts
        opcode = cur_inst & 0x7F;
        rd = (cur_inst >> 7) & 0x1F;
        funct3 = (cur_inst >> 12) & 0x7;
        rs1 = (cur_inst >> 15) & 0x1F;
        rs2 = (cur_inst >> 20) & 0x1F;
        funct7 = (cur_inst >> 25) & 0x7F;

        is_valid_inst = true;

        // Extract immediate
        imm = immediateGenerator();

        // Skip if invalid opcode
        if (!is_valid_inst) {
            return;
        }

        // Default CSR/MRET signals
        is_csr_instruction = false;
        is_mret_instruction = false;
        csr_read_enable = false;
        csr_write_enable = false;
        csr_register_write_enable = false;
        csr_operation = 0;
        csr_address = imm;

        // Enable signals if opcode is 0x73 (MRET/CSR)
        if (opcode == 0x73) {
            if (funct3 == 0x0) {
                // MRET instruction
                if (imm == 0x302) {
                    is_mret_instruction = true;
                }
            } 
            else if (funct3 == 0x1 || funct3 == 0x2) {
                // CSR instructions (CSRRW/CSRRS)
                is_csr_instruction = true;
                csr_operation = funct3;
                csr_read_enable = true;
                
                // Disable WriteBack to Register File if rd = x0
                csr_register_write_enable = (rd != 0);

                // CSRRW always writes
                if (funct3 == 0x1) {
                    csr_write_enable = true;
                }
                // CSRRS only writes if rs1 != x0
                else if (funct3 == 0x2) {
                    csr_write_enable = (rs1 != 0);
                }
            }
        }

        // Read required registers
        rs1_data = registers[rs1];
        rs2_data = registers[rs2];

        cout << "@" << sc_time_stamp() << " Decode:" << endl;
        cout << "1. opcode: 0x" << hex << opcode << dec << endl;
        cout << "2. rd: " << rd << endl;
        cout << "3. funct3: " << funct3 << endl;
        cout << "4. rs1: " << rs1 << endl;
        cout << "5. rs2: " << rs2 << endl;
        cout << "6. funct7: " << funct7 << endl;
        cout << "7. imm: " << imm << endl;
        cout << "8. rs1_data: " << rs1_data << endl;
        cout << "9. rs2_data: " << rs2_data << endl << endl;

        wait();
    }

    // EX: Instruction Execute
    // ------------------------------
    void execute() {
        cout << "@" << sc_time_stamp() << " Execute: Instruction 0x" << hex << cur_inst << dec << " is being executed" << endl << endl;

        // Perform ALU operation
        alu_res = alu();

        branch_taken = false;

        // Check if branch is taken or not
        if (opcode == 0x63) {
            // BEQ
            if (funct3 == 0x0) {
                branch_taken = (rs1_data == rs2_data);
                
                cout << "@" << sc_time_stamp() << " Execute: BEQ x" << rs1 << "(" << rs1_data << "), x" << rs2 << "(" << rs2_data << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }
            // BNE
            else if (funct3 == 0x1) {
                branch_taken = (rs1_data != rs2_data);
                
                cout << "@" << sc_time_stamp() << " Execute: BNE x" << rs1 << "(" << rs1_data << "), x" << rs2 << "(" << rs2_data << ")";
                cout << " | Branch Taken: " << (branch_taken ? "YES" : "NO");
            }

            // Find next instruction address
            if (branch_taken) {
                pc_next = pc + imm;
            }
            else {
                pc_next = pc + 4;
            }

            cout << " | Target: 0x" << hex << pc_next << dec << endl << endl;
        }
        // For JAL calculate the return address and the PC value
        else if (opcode == 0x6F) {
            branch_taken = true;
            pc_next = pc + imm;
            alu_res = pc + 4;
            
            cout << "@" << sc_time_stamp() << " Execute: JAL | Return Address: 0x" << hex << alu_res << " | PC: 0x" << pc_next << dec << endl << endl;
        }
        // For JALR calculate the return address and the PC value
        else if (opcode == 0x67) {
            branch_taken = true;
            pc_next = (rs1_data + imm) & ~1; 
            alu_res = pc + 4;
            
            cout << "@" << sc_time_stamp() << " Execute: JALR | Return Address: 0x" << hex << alu_res << " | PC: 0x" << pc_next << dec << endl << endl;
        }
        // For CSR/MRET Handling 
        else if (is_mret_instruction || is_csr_instruction) {
            // MRET
            if (is_mret_instruction) {
                // Restore PC
                pc_next = mepc;
                
                // Enable interrupts again
                mstatus = mstatus | 0x8;
                
                cout << "@" << sc_time_stamp() << " Execute: MRET | Return Address: 0x" << hex << pc_next << dec << endl << endl;
            }
            // CSR
            else if (is_csr_instruction) {
                // Read CSR
                if (csr_read_enable) {
                    csr_old_value = read_csr(csr_address);
                } 
                else {
                    csr_old_value = 0;
                }
                
                csr_new_value = csr_old_value;

                // Calculate New CSR Value
                // CSRRW
                if (csr_operation == 0x1) {
                    csr_new_value = rs1_data;
                } 
                // CSRRS
                else if (csr_operation == 0x2) {
                    csr_new_value |= rs1_data; 
                }

                // Prepare to write old CSR value to destination register
                alu_res = csr_old_value;
                
                // Move to next instruction
                pc_next = pc + 4;
            }
        }
        // Move to next instruction by default
        else {
            pc_next = pc + 4;
        }

        wait();
    }

    // MEM: Memory/Peripheral Access
    // ------------------------------
    void memoryAccess() {
        // Load
        if (opcode == 0x3) {
            read_en_o.write(true);
            addr_bus_o.write(alu_res);

            wait();

            read_en_o.write(false);

            wait();

            mem_data = data_bus_i.read();

            cout << "@" << sc_time_stamp() << " Memory Access: Loaded 0x" << hex << mem_data << " from address 0x" << alu_res << dec << endl << endl;

            return;
        }
        // Store
        else if (opcode == 0x23) {
            write_en_o.write(true);
            addr_bus_o.write(alu_res);
            data_bus_o.write(rs2_data);

            wait();

            write_en_o.write(false);

            cout << "@" << sc_time_stamp() << " Memory Access: Stored 0x" << hex << rs2_data << " to address 0x" << alu_res << dec << endl << endl;
        }
        else {
            cout << "@" << sc_time_stamp() << " Memory Access: No memory/peripheral access needed" << endl << endl;
        }

        wait();
    }

    // WB: Write Back
    // ------------------------------
    void writeBack() {
        cout << "@" << sc_time_stamp() << " Write Back: Old PC value was 0x" << hex << pc << dec << endl << endl;

        // Write back old CSR value
        if (is_csr_instruction) {
            // Write CSR
            if (csr_write_enable) {
                write_csr(csr_address, csr_old_value, csr_new_value);
            }

            if (csr_register_write_enable) {
                registers[rd] = alu_res;
                cout << "@" << sc_time_stamp() << " Write Back: Register x" << rd << " updated to " << alu_res << endl << endl;
            }
        }
        // Write back alu_result to register file
        else if (opcode == 0x33 || opcode == 0x13 || opcode == 0x37 || opcode == 0x17) {
            // x0 register stays 0
            if (rd != 0) {
                registers[rd] = alu_res;
                cout << "@" << sc_time_stamp() << " Write Back: Register x" << rd << " updated to " << alu_res << endl << endl;
            }
        }
        // Write back mem_data to register file for Load
        else if (opcode == 0x3) {
            if (rd != 0) {
                registers[rd] = mem_data;
                cout << "@" << sc_time_stamp() << " Write Back: Register x" << rd << " updated to " << mem_data << endl << endl;
            }
        }
        // No write back for Branch
        else if (opcode == 0x63) {
            cout << "@" << sc_time_stamp() << " Write Back: No register write back for branch instruction" << endl << endl;
        }
        // Write back return address to destination register for Jump
        else if (opcode == 0x6F || opcode == 0x67) {
            if (rd != 0) {
                registers[rd] = alu_res;
                cout << "@" << sc_time_stamp() << " Write Back: Register x" << rd << " updated to " << alu_res << endl << endl;
            }
        }

        // Move to next instruction
        pc = pc_next;

        cout << "@" << sc_time_stamp() << " Write Back: New PC value is 0x" << hex << pc << dec << endl << endl;

        wait();
    }

    // ------------------------------------------------------------
    // Main Thread
    // ------------------------------------------------------------
    void mainThread() {
        // Reset/initial state logic
        pc = 0;
        cur_inst = 0;
        for (int i = 0; i < WIDTH; i++) {
            registers[i] = 0;
        }

        write_en_o.write(false);
        read_en_o.write(false);
        addr_bus_o.write(0);
        data_bus_o.write(0);

        mstatus = 0x0;
        mie     = 0x0;
        mip     = 0x0;   
        mtvec   = 0x0;
        mepc    = 0x0;
        mcause  = 0x0;

        // Wait marking end of reset
        wait();

        // Main loop
        while (true) {
            // Reset flags to default before executing each instruction
            write_en_o.write(false);
            read_en_o.write(false);

            if (irq_timer_i.read() == true) {
                mip = mip | 0x80; // Set Bit 7 (timer interrupt)
            } 
            else {
                mip = mip & ~0x80; // Clear Bit 7
            }

            if ((mip & 0x80) && (mie & 0x80) && (mstatus & 0x8)) {
                // Save PC value
                mepc = pc;

                // Set cause as timer interrupt
                mcause = 0x80000007;

                // Disable interrupts
                mstatus = mstatus & ~0x8;
                
                // Move to interrupt handling address
                pc = mtvec;

                cout << "@" << sc_time_stamp() << " CPU: Timer interrupt received" << endl;
                cout << "@" << sc_time_stamp() << " CPU: Jumping to interrupt handler\n" << endl << endl;
            }

            fetch();
            decode();

            // Skip instruction if invalid opcode
            if (!is_valid_inst) {
                pc += 4;
                continue;
            }

            execute();
            memoryAccess();
            writeBack();
        }
    }

    SC_CTOR(risc_v_model) {
        SC_CTHREAD(mainThread, clk_i.pos());
        reset_signal_is(rst_i, true);
    }
};
