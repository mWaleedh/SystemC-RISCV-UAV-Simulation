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
                // 31:13
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
        case 0x67:
            // [20]
            immediate = ((cur_inst >> 31) & 0x1) << 20;
            // [19:12]
            immediate |= ((cur_inst >> 12) & 0xFF) << 12;
            // [11]
            immediate |= ((cur_inst >> 20) &0x1) << 11;
            // [10:1]
            immediate |= ((cur_inst >> 21) & 0x3FF) << 1;

            // Sign extension
            if (immediate & 0x4FFFF) {
                // [31:21]
                immediate |= 0xFFF00000; 
            }
            break;

        default:
            immediate = 0;
            break;
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
            break;

        // I-type (ALU)
        case 0x13:
            // ADDI
            if (funct3 == 0x0) {
                alu_result = rs1_data + imm;
                cout << "@" << sc_time_stamp() << " ALU: " << rs1_data << " + " << imm << " = " << alu_result << endl << endl;
            }
            break;
            
        // I-type (Load)
        case 0x3:
            // LW
            if (funct3 == 0x2) {
                alu_result = rs1_data + imm;
                cout << "@" << sc_time_stamp() << " ALU (LW): " << rs1_data << " + " << imm << " = 0x" << hex << alu_result << dec << endl << endl;
            }
            break;
        
        // S-type
        case 0x23:
            // SW
            if (funct3 == 0x2) {
                alu_result = rs1_data + imm;
                cout << "@" << sc_time_stamp() << " ALU (SW): " << rs1_data << " + " << imm << " = 0x" << hex << alu_result << dec << endl << endl;
            }
            break;
            
        default:
            alu_result = 0;
       }

       return alu_result;
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

        // Extract immediate
        imm = immediateGenerator();

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

        // Write back alu_result to register file
        if (opcode == 0x33 || opcode == 0x13) {
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
                cout << "@" << sc_time_stamp() << " Write Back: Register x" << rd << " updated to " << alu_res << endl << endl;
            }
        }

        // Move to next instruction
        pc += 4;

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

        // Wait marking end of reset
        wait();

        // Main loop
        while (true) {
            // Reset flags to default before executing each instruction
            write_en_o.write(false);
            read_en_o.write(false);

            fetch();
            decode();
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
