# SC_MODULE
Is the most basic container in SystemC that works like a VHDL entity. It holds the input and output ports, internal signals/wires, and the processes that are used to simulate hardware behaviour. Its structure is almost like a C++ class with a semicolon at the end, and its object is also created like a C++ class object.

# SC_METHOD
This process is used for only combinational logic meaning it executes all its instructions without any delay, and it is executed only when a certain event occurs. We can add multiple events to its sensitivity list, and whenever one of them changes, the method is called. Since it's combinational it can not have a wait(), or any other command that delays the execution. This behavior is equivalent to a VHDL process block. It is also synthesizable, meaning it can be translated into a physical circuit.

# SC_THREAD
This type of process is designed for asynchronous execution of commands meaning it can contain delays, or wait() can be used within it. It runs once at the start, and typically has a infinite while loop running within it. Since it is not synthesizable, it can not be used for core implementations that need to be converted to hardware in the future. That's why it is mostly used for writing testbenches.

# SC_CTHREAD

# sc_signal
This is used to create variables that represent physical wires or signals. They are equivalent to a VHDL signal. Values are not updated immediately for these, instead they are updated after a specific time interval, which is how real wires work. They are typically used to connect input and output ports togather like in real hardware, and they can also be used to hold values if they aren't needed immediately.

# sc_clock
This is a built in class that provides a clock, and can be highly configured. For example we can set the interval of a cycle, the amount of time it stays high or low and its starting value. It is created once at the start of the testbench and is connected to the input ports of different modules to provide them with a clock.

# Testbench
The testbench is stored within the sc_main function, which is the first function that is called when a program is run. Or in VHDL terms it's called the top level entity. Here all the modules' objects are created and connected togather using signals. And all the simulation code to test the system is also written here.