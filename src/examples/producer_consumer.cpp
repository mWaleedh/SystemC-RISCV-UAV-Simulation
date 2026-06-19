/* Problems:
Initially I was using SC_METHOD and put consumer and producer flags' rising edge into its sensitivity list.
So that is why I was doing consumer = True and immediately consumer = false to simulate a rising edge.
But since there was no delay between them sometimes the rising edge did register, so I decided to add a wait() between them.
But since I was using SC_METHOD, I couldn't place a wait() call in it so I had to switched it to SC_THREAD.

Another problem that I faced was, I was getting an error when I ran wait(consumer.pos()), it was giving invalid type error

In my new implementation I was writing to a signal using two different functions (THREADS) inside a MODULE but it was giving me an error (sc_signal can't have more than one driver),
I fixed this by removing the write from the second function and making an sc_event that called the other function to write to it.
*/

#include <iostream>
#include <systemc.h>
#include <semaphore.h>
#include <mutex>
using namespace std;

SC_MODULE(producer) {
    // input ports
    sc_in<bool> full_i; // flag that indicates the buffer is full

    // output ports
    sc_out<sc_uint<8>> item_o; // item that is produced
    sc_out<bool> produced_o; // flag to notify consumer that item has been produced

    void produceItem() {
        while (true) {
            // if buffer is full stop
            if (full_i.read()) {
                // wait until consumer consumes an item from buffer
                wait(full_i.negedge_event());
            }

            // produce random item
            int item = rand() % 255;
            item_o.write(item);

            cout << "PRODUCER: " << item << endl;

            // notify consumer module that a new item has been produced and needs to be added to buffer
            produced_o.write(true);
            wait(1, SC_NS);
            produced_o.write(false);

            // producer takes 3 cycles total to produce an item
            wait(2, SC_NS);
        }
    }   

    SC_CTOR(producer) {
        SC_THREAD(produceItem);
    }
};

SC_MODULE(consumer) {
    // input ports
    sc_in<bool> produced_i; // flag that indicates that a new item has been produced
    sc_in<sc_uint<8>> item_i; // produced item

    // output ports
    sc_out<bool> full_o; // flag that tells producer that buffer is full

    // local variables
    static const int BUFFER_SIZE = 5;
    sc_uint<8> buffer[BUFFER_SIZE];
    int count, add, remove;

    // events to communicate between buffer adding function and consumer function
    sc_event item_added;
    sc_event item_removed;

    void addItem() {
        while (true) {
            // if buffer is full stop producer until consumer removes item
            if (count == BUFFER_SIZE) {
                full_o.write(true);
                wait(item_removed); // wait for consumer to notify using event that item has been removed
                full_o.write(false);
            }    

            // wait for producer to produce an item
            wait(produced_i.posedge_event());

            // add item to buffer
            buffer[add] = item_i.read();
            add = (add + 1) % BUFFER_SIZE;
            count++;

            // notify consumer that new item has been added to buffer
            item_added.notify();
        }
    }

    void consumeItem() {
        while (true) {
            // stop if buffer is empty
            if (count == 0) {
                // wait for bufferAdd function to notify that a new item has been added
                wait(item_added);
            }

            // remove from buffer
            int item = buffer[remove];
            remove = (remove + 1) % BUFFER_SIZE;
            count--;

            // notify bufferAdd that item has been consumed
            item_removed.notify();

            cout << "CONSUMER: " << item << endl;

            // consumer takes 8 cycles to consume an item
            wait(8, SC_NS);
        }
    }

    SC_CTOR(consumer) {
        count = 0;
        add = 0;
        remove = 0;

        SC_THREAD(addItem);
        SC_THREAD(consumeItem);
    }
};

int sc_main(int argc, char* argv[]) {
    srand(time(0));

    // signals
    sc_signal<bool> full_s;
    sc_signal<bool> produced_s;
    sc_signal<sc_uint<8>> item_s;

    // attach signals to input/output ports of producer
    producer producer_inst("produer_inst");
        producer_inst.full_i(full_s);
        producer_inst.produced_o(produced_s);
        producer_inst.item_o(item_s);

    // attach signals to input/output ports of consumer
    consumer consumer_inst("consumer_inst");
    consumer_inst.item_i(item_s);
    consumer_inst.produced_i(produced_s);
    consumer_inst.full_o(full_s);

    // initially the buffer is empty
    full_s.write(false);
    produced_s.write(false);

    // run simulation for 25 cycles
    sc_start(25, SC_NS);

    return 0;
}