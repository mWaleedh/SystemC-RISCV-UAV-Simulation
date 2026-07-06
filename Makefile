CXX = g++
CXXFLAGS = -I. -lsystemc

all: memory_test decode_test load_store_test branch_test jump_test

memory_test: ./tests/riscv_memory_testbench.cpp
	$(CXX) -o memory_test ./tests/riscv_memory_testbench.cpp $(CXXFLAGS)

decode_test: ./tests/riscv_decode_testbench.cpp
	$(CXX) -o decode_test ./tests/riscv_decode_testbench.cpp $(CXXFLAGS)

load_store_test: ./tests/riscv_load_store_testbench.cpp
	$(CXX) -o load_store_test ./tests/riscv_load_store_testbench.cpp $(CXXFLAGS)

branch_test: ./tests/riscv_branch_testbench.cpp
	$(CXX) -o branch_test ./tests/riscv_branch_testbench.cpp $(CXXFLAGS)

jump_test: ./tests/riscv_jump_testbench.cpp
	$(CXX) -o jump_test ./tests/riscv_jump_testbench.cpp $(CXXFLAGS)

clean:
	rm -f memory_test decode_test load_store_test branch_test jump_test