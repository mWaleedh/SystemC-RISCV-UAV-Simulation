#include <iostream>
#include <systemc.h>
#include <fstream>
#include <string>
using namespace std;

SC_MODULE(memory_model) {
    // constants
    static const int WIDTH = 32;
    static const int SIZE = 1024;

    // input ports
    sc_in<bool> clk_i;
    sc_in<bool> rst_i;
    sc_in<bool> write_en_i;
    sc_in<bool> read_en_i;
    sc_in<sc_uint<WIDTH>> addr_bus_i;
    sc_in<sc_uint<WIDTH>> data_bus_i;

    // output ports
    sc_out<sc_uint<WIDTH>> data_bus_o;

    // local variables
    sc_uint<WIDTH> memory[SIZE];

    // Function to load testbench data
    void load_data(uint32_t addr, uint32_t data) {
        // Align address
        addr /= 4;

        // Write data to memory if within range
        if (addr < SIZE) {
            memory[addr] = data;
        } 
        else {
            cout << "Memory Error: Trying to access invalid memory 0x" << hex << addr << dec << endl << endl;
        }
    }

    // Function to load entire program 
    void load_file(const string& filename) {
        ifstream file(filename.c_str());
        if (!file.is_open()) {
            cout << "Error: Unable to open file " << filename << endl << endl;
            return;
        }

        string temp;
        uint32_t addr = 0;
        while (getline(file, temp)) {
            // Ignore empty lines
            if (temp.empty()) {
                continue;
            }
            
            // Convert hex string to integer
            uint32_t data = stoul(temp, nullptr, 16);
            
            if ( addr < SIZE) {
                memory[addr] = data;
                // Move to next address
                addr++;
            }
        }
        file.close();

        cout << "Memory: Loaded data from file " << filename << endl << endl;
    }

    // Main thread
    void mainThread() {
        // Reset/initial stage logic
        for(int i = 0; i < SIZE; i++) {
            memory[i] = 0;
        }
        data_bus_o.write(0);

        // Wait marking end of reset
        wait();

        // Main loop
        while (true) {
            // Align address
            uint32_t addr = addr_bus_i.read() / 4;

            // If address is out of bounds don't perform any action
            if (addr > SIZE - 1 && (read_en_i.read() == true || write_en_i.read() == true)) {
                // If it is a read operation, write 0 to data_bus
                if (read_en_i.read() == true) {
                    data_bus_o.write(0);
                }
                cout << "@" << sc_time_stamp() << " Memory Error: Trying to access invalid memory 0x" << hex << addr_bus_i.read() << dec << endl << endl;
            }
            else {
                // Give warning if both read and write are enabled
                if (write_en_i.read() == true && read_en_i.read() == true) {
                    cout << "@" << sc_time_stamp() << " Memory Warning: Both read_en and write_en are true\n" << endl;
                }

                // give preference to write
                if (write_en_i.read() == true) {
                    // write data to target address
                    memory[addr] = data_bus_i.read();

                    cout << "@" << sc_time_stamp() << " Memory Write: " << endl;
                    cout << "1. Address -> 0x" << hex << addr << endl;
                    cout << "2. Data -> 0x" << data_bus_i.read() << dec << endl << endl;
                }
                else if (read_en_i.read() == true) {
                    // read data from target address
                    data_bus_o.write(memory[addr]);

                    cout << "@" << sc_time_stamp() << " Memory Read: " << endl;
                    cout << "1. Address -> 0x" << hex << addr << endl;
                    cout << "2. Data -> 0x" << memory[addr] << dec << endl << endl;
                }
                else {
                    data_bus_o.write(0);
                }
            }

            wait();
        }
    }

    SC_CTOR(memory_model) {
        SC_CTHREAD(mainThread, clk_i.pos());
        reset_signal_is(rst_i, true);
    }
};