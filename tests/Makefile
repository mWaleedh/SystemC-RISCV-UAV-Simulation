CXX = g++
CXXFLAGS = -I. -lsystemc

all: memory_test decode_test load_store_test

memory_test: riscv_memory_testbench.cpp
	$(CXX) -o memory_test riscv_memory_testbench.cpp $(CXXFLAGS)

decode_test: riscv_decode_testbench.cpp
	$(CXX) -o decode_test riscv_decode_testbench.cpp $(CXXFLAGS)

load_store_test: riscv_load_store_testbench.cpp
	$(CXX) -o load_store_test riscv_load_store_testbench.cpp $(CXXFLAGS)

clean:
	rm -f memory_test decode_test load_store_test *.vcd