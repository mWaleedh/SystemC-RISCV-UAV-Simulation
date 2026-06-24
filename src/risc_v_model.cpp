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

    // ------------------------------------------------------------
    // Pipeline
    // ------------------------------------------------------------

    // IF: Instruction Fetch
    // ------------------------------
    void fetch() {
        // Send program counter value to memory
        addr_bus_o.write(pc);
        read_en_o.write(true);

        wait();

        // Read instruction sent by memory
        cur_inst = data_bus_i.read();
        read_en_o.write(false);

        cout << "@" << sc_time_stamp() << " Fetch: Instruction -> 0x" << hex << cur_inst << dec << endl << endl;
    }    

    // ID: Instruction Decode
    // ------------------------------
    void decode() {
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
        cout << "@" << sc_time_stamp() << " Execute: Instruction is being executed" << endl << endl;

        wait();
    }

    // MEM: Memory/Peripheral Access
    // ------------------------------
    void memoryAccess() {
        // Only load/store require memory access
        if (opcode == 0x3 || opcode == 0x23) {
            cout << "@" << sc_time_stamp() << " Memory Access: Accessing memory/peripheral for load/store" << endl << endl;
        }
        else {
            cout << "@" << sc_time_stamp() << " Memory Access: No memory/peripheral access needed" << endl << endl;
        }

        wait();
    }

    // WB: Write Back
    // ------------------------------
    void writeBack() {
        // Move to next instruction
        pc += 4;
        cout << "@" << sc_time_stamp() << " Write Back: Program Counter -> 0x" << hex << pc << dec << endl << endl;

        wait();
    }
    // ------------------------------------------------------------
    // ------------------------------------------------------------
    // ------------------------------------------------------------

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
