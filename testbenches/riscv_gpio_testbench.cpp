 #include <systemc.h>
#include "../src/system_top.cpp"
using namespace std;

int sc_main(int argc, char* argv[]) {
    // Create sc_clock object
    sc_clock clk_s("clk");

    // Create signals to connect input ports
    sc_signal<bool> rst_s;
    sc_signal<bool> irq_timer_s;
    sc_signal<bool> irq_ext_s;
    sc_signal<bool> irq_sw_s;

    // Initialize system_top and connect input ports
    system_top sys("System_Top");
    sys.clk_i(clk_s);
    sys.rst_i(rst_s);
    sys.irq_timer_i(irq_timer_s);
    sys.irq_ext_i(irq_ext_s);
    sys.irq_sw_i(irq_sw_s);

    // VCD waveform trace
    sc_trace_file *wf = sc_create_vcd_trace_file("./waveforms/riscv_gpio_waveform");
    sc_trace(wf, clk_s, "clock");
    sc_trace(wf, rst_s, "reset");
    sc_trace(wf, sys.cpu->pc, "pc");
    sc_trace(wf, sys.cpu->cur_inst, "cur_inst");
    sc_trace(wf, sys.cpu->addr_bus_o, "cpu_addr");
    sc_trace(wf, sys.mem->read_en_i, "mem_read");
    sc_trace(wf, sys.mem->write_en_i, "mem_write");
    sc_trace(wf, sys.gpio->read_en_i, "gpio_read");
    sc_trace(wf, sys.gpio->write_en_i, "gpio_write");
    sc_trace(wf, sys.gpio->input_reg, "gpio_input_reg");
    sc_trace(wf, sys.gpio->output_reg, "gpio_output_reg");
    sc_trace(wf, sys.gpio->data_bus_i, "gpio_data_in");
    sc_trace(wf, sys.gpio->data_bus_o, "gpio_data_out");

    // Clear input ports
    irq_timer_s.write(false);
    irq_ext_s.write(false);
    irq_sw_s.write(false);

    // Reset
    cout << "@" << sc_time_stamp() << " Applying Reset..." << endl;
    rst_s.write(true);
    sc_start(5, SC_NS);

    // Release Reset
    cout << "@" << sc_time_stamp() << " Releasing Reset...\n" << endl;
    rst_s.write(false);

    // Load JAL instructions
    sys.load_file("./hex/riscv_gpio_program.hex");

    // Run system for JAL
    sc_start(46, SC_NS);

    // Verify results for JAL
    cout << "x3 = 1: " << (sys.cpu->registers[3] == 1 ? "PASS" : "FAIL") << endl;

    cout << "x0 = 0: " << (sys.cpu->registers[0] == 0 ? "PASS" : "FAIL") << endl << endl;

    cout << "@" << sc_time_stamp() << " Simulation complete!" << endl;

    return 0;
}