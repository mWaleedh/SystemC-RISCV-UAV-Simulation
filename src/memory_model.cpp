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

    // Instruction Busses
    sc_in<bool> inst_read_en_i;
    sc_in<sc_uint<WIDTH>> inst_addr_bus_i;
    sc_out<sc_uint<WIDTH>> inst_bus_o;

    // Data Busses
    sc_in<bool> data_read_en_i;
    sc_in<bool> data_write_en_i;
    sc_in<sc_uint<WIDTH>> data_addr_bus_i;
    sc_in<sc_uint<WIDTH>> data_bus_i;
    sc_out<sc_uint<WIDTH>> data_bus_o;

    // local variables
    sc_uint<8> memory[SIZE];
    sc_uint<WIDTH> max_addr;

    // Function to load testbench data
    void load_data(uint32_t addr, uint32_t data) {
        // Write data to memory if within range
        if (addr + 3 < SIZE) {
            // Make sure it isn't overwriting a previous instruction
            if (addr >= max_addr) {
                // Write one byte at a time
                memory[addr] = data;
                memory[addr + 1] = data >> 8;
                memory[addr + 2] = data >> 16; 
                memory[addr + 3] = data >> 24;
            }
            else {
                cout << "Memory Error: Trying to access already occupied memory 0x" << hex << addr << dec << endl << endl;    
            }
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
        while (getline(file, temp)) {
            // Ignore empty lines
            if (temp.empty()) {
                continue;
            }
            
            // Convert hex string to integer
            uint32_t data = stoul(temp, nullptr, 16);
            
            if ( max_addr + 3 < SIZE) {
                // Write one byte at a time
                memory[max_addr] = data;
                memory[max_addr + 1] = data >> 8;
                memory[max_addr + 2] = data >> 16; 
                memory[max_addr + 3] = data >> 24;

                // Move to next address
                max_addr += 4;
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

        // Address of last instruction of .hex program
        max_addr = 0;

        // Wait marking end of reset
        wait();

        // Main loop
        while (true) {
            // Instruction Fetch
            uint32_t inst_addr = inst_addr_bus_i.read();

            // Check if CPU requested Instruction
            if (inst_read_en_i.read() == true) {
                if (inst_addr + 3 >= SIZE) {
                    inst_bus_o.write(0);
                    cout << "@" << sc_time_stamp() << " Memory Error (Instruction Port): Trying to access invalid memory 0x" << hex << inst_addr_bus_i.read() << dec << endl << endl;
                }
                else {
                    // Combine bytes 
                    uint32_t data = (memory[inst_addr]) | (memory[inst_addr + 1] << 8) | (memory[inst_addr + 2] << 16) | (memory[inst_addr + 3] << 24);

                    // Send Instruction to CPU
                    inst_bus_o.write(data);

                    cout << "@" << sc_time_stamp() << " Memory Read (IF): " << endl;
                    cout << "1. Address -> 0x" << hex << inst_addr_bus_i.read() << endl;
                    cout << "2. Data -> 0x" << data << dec << endl << endl;
                }
            }
            else {
                inst_bus_o.write(0);
            }

            // Data Fetch
            uint32_t data_addr = data_addr_bus_i.read();

            // If address is out of bounds don't perform any action
            if (data_addr + 3 >= SIZE && (data_read_en_i.read() == true || data_write_en_i.read() == true)) {
                // If it is a read operation, write 0 to data_bus
                if (data_read_en_i.read() == true) {
                    data_bus_o.write(0);
                }
                cout << "@" << sc_time_stamp() << " Memory Error (Data Port): Trying to access invalid memory 0x" << hex << data_addr_bus_i.read() << dec << endl << endl;
            }
            else {
                // Give warning if both read and write are enabled
                if (data_write_en_i.read() == true && data_read_en_i.read() == true) {
                    cout << "@" << sc_time_stamp() << " Memory Warning: Both read_en and write_en are true\n" << endl;
                }

                // Give preference to write
                if (data_write_en_i.read() == true) {
                    // write data to target address
                    uint32_t data = data_bus_i.read();
                    memory[data_addr] = data;
                    memory[data_addr + 1] = data >> 8;
                    memory[data_addr + 2] = data >> 16; 
                    memory[data_addr + 3] = data >> 24;

                    cout << "@" << sc_time_stamp() << " Memory Write (MEM): " << endl;
                    cout << "1. Address -> 0x" << hex << data_addr_bus_i.read() << endl;
                    cout << "2. Data -> 0x" << data_bus_i.read() << dec << endl << endl;
                }
                else if (data_read_en_i.read() == true) {
                    // Combine bytes 
                    uint32_t data = (memory[data_addr]) | (memory[data_addr + 1] << 8) | (memory[data_addr + 2] << 16) | (memory[data_addr + 3] << 24);

                    // Read data from target address
                    data_bus_o.write(data);

                    cout << "@" << sc_time_stamp() << " Memory Read (MEM): " << endl;
                    cout << "1. Address -> 0x" << hex << data_addr_bus_i.read() << endl;
                    cout << "2. Data -> 0x" << data << dec << endl << endl;
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