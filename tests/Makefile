CXX = g++
CXXFLAGS = -I. -lsystemc
TARGET = run_sys
SRC = riscv_decode_testbench.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) -o $(TARGET) $(SRC) $(CXXFLAGS)

clean:
	rm -f $(TARGET) *.vcd