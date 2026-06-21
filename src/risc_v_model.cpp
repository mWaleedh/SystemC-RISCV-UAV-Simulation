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

    sc_int<32> immediateGenerator() {
        int immediate = 0;
        switch (opcode) {
            // R-type
            case 0x33:
                immediate = 0;
                break;

            // I-type
            case 0x13:
                immediate = (cur_inst >> 20 ) & 0xFFF;

                // Sign extension
                if (immediate & 0x800) { 
                    immediate |= 0xFFFFF000; 
                }

            // S-type
            case 0x3:
            case 0x23:
                immediate = ((cur_inst >> 25 ) & 0x7F) << 5;
                immediate |= (cur_inst >> 7) & 0x1F;

                // Sign extension
                if (immediate & 0x800) { 
                    immediate |= 0xFFFFF000; 
                }

            // B-type
            case 0x63:
                immediate = ((cur_inst >> 32) & 0x1) << 12;
                immediate |= ((cur_inst >> 7) & 0x1) << 11;
                immediate |= ((cur_inst >> 25) & 0x3F) << 5;
                immediate |= ((cur_inst >> 8) & 0xF) << 1;

                // Sign extension
                if (immediate & 0x1000) { 
                    immediate |= 0xFFFFE000; 
                }

            // U-type
            case 0x37:
            case 0x17:

            // J-type
            case 0x6F:
            case 0x67:
                
            default:
                break;
        }
    }

    // ------------------------------------------------------------
    // Pipeline
    // ------------------------------------------------------------

    // IF: Instruction Fetch
    // ------------------------------
    void fetch() {
        addr_bus_o.write(pc);
        read_en_o.write(true);

        wait();

        cur_inst = data_bus_i.read();
        read_en_o.write(false);
    }    

    // ID: Instruction Decode
    // ------------------------------
    void decode() {
        opcode = cur_inst & 0x7F;
        rd = (cur_inst >> 7) & 0x1F;
        funct3 = (cur_inst >> 12) & 0x7;
        rs1 = (cur_inst >> 15) & 0x1F;
        rs2 = (cur_inst >> 20) & 0x1F;
        funct7 = (cur_inst >> 25) & 0x7F;

        imm = immediateGenerator();

        if (rs1 == 0)
            rs1_data = 0;
        else
            rs1_data = registers[rs1];

        if (rs2 == 0)
            rs2_data = 0;
        else
            rs2_data = registers[rs2];
    }

    // EX: Instruction Execute
    // ------------------------------
    void execute() {

    }

    // MEM: Memory/Peripheral Access
    // ------------------------------
    void memoryAccess() {

    }

    // WB: Write Back
    // ------------------------------
    void writeBack() {

    }
    // ------------------------------------------------------------
    // ------------------------------------------------------------
    // ------------------------------------------------------------

    // ------------------------------------------------------------
    // Main Thread
    // ------------------------------------------------------------
    void mainThread() {
        // reset/initial state logic
        pc = 0;
        cur_inst = 0;
        for (int i = 0; i < WIDTH; i++) {
            registers[i] = 0;
        }

        write_en_o.write(false);
        read_en_o.write(false);
        addr_bus_o.write(0);
        data_bus_o.write(0);

        // wait marking end of reset part
        wait();

        // main loop
        while (true) {
            // reset flags to default before executing each instruction
            write_en_o.write(false);
            read_en_o.write(false);

            fetch();

            decode();

            execute();
            
            memoryAccess();

            writeBack();

            wait();
        }
    }

    SC_CTOR(risc_v_model) {
        SC_CTHREAD(mainThread, clk_i.pos());
        reset_signal_is(rst_i, true);
    }
};

// ------------------------------------------------------------
// ------------------------------------------------------------
// ------------------------------------------------------------

// ------------------------------------------------------------
// Test Bench
// ------------------------------------------------------------
int sc_main(int argc, char* argv[]) {
    sc_clock clk_s("clk");

    sc_signal<bool> rst_s;
    sc_signal<bool> irq_timer_s;
    sc_signal<bool> irq_ext_s;
    sc_signal<bool> irq_sw_s;
    sc_signal<sc_uint<32>> data_bus_i_s;
    
    sc_signal<bool> write_en_s;
    sc_signal<bool> read_en_s;
    sc_signal<sc_uint<32>> addr_bus_o_s;
    sc_signal<sc_uint<32>> data_bus_o_s;

    risc_v_model cpu("cpu");

    cpu.clk_i(clk_s);
    cpu.rst_i(rst_s);
    cpu.irq_timer_i(irq_timer_s);
    cpu.irq_ext_i(irq_ext_s);
    cpu.irq_sw_i(irq_sw_s);
    cpu.data_bus_i(data_bus_i_s);
    
    cpu.write_en_o(write_en_s);
    cpu.read_en_o(read_en_s);
    cpu.addr_bus_o(addr_bus_o_s);
    cpu.data_bus_o(data_bus_o_s);

    irq_timer_s.write(false);
    irq_ext_s.write(false);
    irq_sw_s.write(false);
    data_bus_i_s.write(0);
    
    cout << "@" << sc_time_stamp() << " Resetting CPU..." << endl;
    rst_s.write(true);

    sc_start(2, SC_NS); 

    cout << "@" << sc_time_stamp() << " Starting Execution..." << endl;
    rst_s.write(false);

    sc_start(10, SC_NS);

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}