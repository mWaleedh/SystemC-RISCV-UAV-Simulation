CXX = g++
CXXFLAGS = -I. -lsystemc

all: memory_test decode_test load_store_test branch_test jump_test mini_program_test gpio_test timer_test

memory_test: ./testbenches/riscv_memory_testbench.cpp
	$(CXX) -o memory_test ./testbenches/riscv_memory_testbench.cpp $(CXXFLAGS)

decode_test: ./testbenches/riscv_decode_testbench.cpp
	$(CXX) -o decode_test ./testbenches/riscv_decode_testbench.cpp $(CXXFLAGS)

load_store_test: ./testbenches/riscv_load_store_testbench.cpp
	$(CXX) -o load_store_test ./testbenches/riscv_load_store_testbench.cpp $(CXXFLAGS)

branch_test: ./testbenches/riscv_branch_testbench.cpp
	$(CXX) -o branch_test ./testbenches/riscv_branch_testbench.cpp $(CXXFLAGS)

jump_test: ./testbenches/riscv_jump_testbench.cpp
	$(CXX) -o jump_test ./testbenches/riscv_jump_testbench.cpp $(CXXFLAGS)

mini_program_test: ./testbenches/riscv_mini_program_testbench.cpp
	$(CXX) -o mini_program_test ./testbenches/riscv_mini_program_testbench.cpp $(CXXFLAGS)

gpio_test: ./testbenches/riscv_gpio_testbench.cpp
	$(CXX) -o gpio_test ./testbenches/riscv_gpio_testbench.cpp $(CXXFLAGS)

timer_test: ./testbenches/riscv_timer_testbench.cpp
	$(CXX) -o timer_test ./testbenches/riscv_timer_testbench.cpp $(CXXFLAGS)

all_tests: all
	./memory_test
	./decode_test
	./load_store_test
	./branch_test
	./jump_test
	./mini_program_test
	./gpio_test
	./timer_test

clean:
	rm -f memory_test decode_test load_store_test branch_test jump_test mini_program_test gpio_test timer_test