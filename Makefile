CXX = g++
CXXFLAGS = -I. -lsystemc

all: memory_test \
	 decode_test \
	 load_store_test \
	 branch_test \
	 jump_test \
	 mini_program_test \
	 gpio_test \
	 timer_test \
	 interrupt_test \
	 lui_test \
	 csr_interrupt_test \
	 csrrw_test \
	 csrrs_test \
	 csr_x0_test \
	 interrupt_enable_test \
	 mepc_return_test \
	 mret_test \
	 periodic_interrupt_test \
	 interrupt_masking_test \
	 load_use_test \
	 forwarding_test \
	 store_forwarding_test \
	 branch_forwarding_test \
	 always_not_taken_test

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

interrupt_test: ./testbenches/riscv_interrupt_testbench.cpp
	$(CXX) -o interrupt_test ./testbenches/riscv_interrupt_testbench.cpp $(CXXFLAGS)

lui_test: ./testbenches/riscv_lui_testbench.cpp
	$(CXX) -o lui_test ./testbenches/riscv_lui_testbench.cpp $(CXXFLAGS)

csr_interrupt_test: ./testbenches/riscv_csr_interrupt_testbench.cpp
	$(CXX) -o csr_interrupt_test ./testbenches/riscv_csr_interrupt_testbench.cpp $(CXXFLAGS)

csrrw_test: ./testbenches/riscv_csrrw_testbench.cpp
	$(CXX) -o csrrw_test ./testbenches/riscv_csrrw_testbench.cpp $(CXXFLAGS)

csrrs_test: ./testbenches/riscv_csrrs_testbench.cpp
	$(CXX) -o csrrs_test ./testbenches/riscv_csrrs_testbench.cpp $(CXXFLAGS)

csr_x0_test: ./testbenches/riscv_csr_x0_testbench.cpp
	$(CXX) -o csr_x0_test ./testbenches/riscv_csr_x0_testbench.cpp $(CXXFLAGS)

interrupt_enable_test: ./testbenches/riscv_interrupt_enable_testbench.cpp
	$(CXX) -o interrupt_enable_test ./testbenches/riscv_interrupt_enable_testbench.cpp $(CXXFLAGS)

mepc_return_test: ./testbenches/riscv_mepc_return_testbench.cpp
	$(CXX) -o mepc_return_test ./testbenches/riscv_mepc_return_testbench.cpp $(CXXFLAGS)

mret_test: ./testbenches/riscv_mret_testbench.cpp
	$(CXX) -o mret_test ./testbenches/riscv_mret_testbench.cpp $(CXXFLAGS)

periodic_interrupt_test: ./testbenches/periodic_interrupt_testbench.cpp
	$(CXX) -o periodic_interrupt_test ./testbenches/periodic_interrupt_testbench.cpp $(CXXFLAGS)

interrupt_masking_test: ./testbenches/interrupt_masking_testbench.cpp
	$(CXX) -o interrupt_masking_test ./testbenches/interrupt_masking_testbench.cpp $(CXXFLAGS)

load_use_test: ./testbenches/load_use_hazard_testbench.cpp
	$(CXX) -o load_use_test ./testbenches/load_use_hazard_testbench.cpp $(CXXFLAGS)

forwarding_test: ./testbenches/riscv_forwarding_testbench.cpp
	$(CXX) -o forwarding_test ./testbenches/riscv_forwarding_testbench.cpp $(CXXFLAGS)

store_forwarding_test: ./testbenches/store_forwarding_testbench.cpp
	$(CXX) -o store_forwarding_test ./testbenches/store_forwarding_testbench.cpp $(CXXFLAGS)

branch_forwarding_test: ./testbenches/branch_forwarding_testbench.cpp
	$(CXX) -o branch_forwarding_test ./testbenches/branch_forwarding_testbench.cpp $(CXXFLAGS)

always_not_taken_test: ./testbenches/always_not_taken_testbench.cpp
	$(CXX) -o always_not_taken_test ./testbenches/always_not_taken_testbench.cpp $(CXXFLAGS)

all_tests: all
	./memory_test
	./decode_test
	./load_store_test
	./branch_test
	./jump_test
	./mini_program_test
	./gpio_test
	./timer_test
	./interrupt_test
	./lui_test
	./csr_interrupt_test
	./csrrw_test
	./csrrs_test
	./csr_x0_test
	./interrupt_enable_test
	./mepc_return_test
	./mret_test
	./periodic_interrupt_test
	./interrupt_masking_test
	./load_use_test
	./forwarding_test
	./store_forwarding_test
	./branch_forwarding_test
	./always_not_taken_test

clean:
	rm -f memory_test \
		  decode_test \
		  load_store_test \
		  branch_test \
		  jump_test \
		  mini_program_test \
		  gpio_test \
		  timer_test \
		  interrupt_test \
		  lui_test \
		  csr_interrupt_test \
		  csrrw_test \
		  csrrs_test \
		  csr_x0_test \
		  interrupt_enable_test \
		  mepc_return_test \
		  mret_test \
		  periodic_interrupt_test \
		  interrupt_masking_test\
		  load_use_test \
		  forwarding_test \
		  store_forwarding_test \
		  branch_forwarding_test \
		  always_not_taken_test