/* Problems:
Initially I was using SC_METHOD and put consumer and producer flags' rising edge into its sensitivity list.
So that is why I was doing consumer = True and immediately consumer = false to simulate a rising edge.
But since there was no delay between them sometimes the rising edge did register, so I decided to add a wait() between them.
But since I was using SC_METHOD, I couldn't place a wait() call in it so I had to switched it to SC_THREAD.

Another problem that I faced was, I was getting an error when I ran wait(consumer.pos()), it was giving invalid type error

In my new implementation I was writing to a singla using two different functions (THREADS) inside a MODULE but it was giving me an error (sc_signal can't have more than one driver),
I fixed this by removing the write from the second function and making an sc_event that called the other function to write to it.
*/

#include <iostream>
#include <systemc.h>
#include <semaphore.h>
#include <mutex>
using namespace std;

SC_MODULE(producer) {
    sc_in<bool> full_i;
    sc_out<sc_uint<8>> item_o;
    sc_out<bool> produced_o;

    void produceItem() {
        while (true) {
            if (full_i.read()) {
                wait(full_i.negedge_event());
            }

            int item = rand() % 255;
            item_o.write(item);

            cout << "PRODUCER: " << item << endl;

            produced_o.write(true);
            wait(1, SC_NS);
            produced_o.write(false);

            wait(2, SC_NS);
        }
    }   

    SC_CTOR(producer) {
        SC_THREAD(produceItem);
    }
};

SC_MODULE(consumer) {
    sc_in<bool> produced_i;
    sc_in<sc_uint<8>> item_i;
    sc_out<bool> full_o;

    static const int BUFFER_SIZE = 5;
    sc_uint<8> buffer[BUFFER_SIZE];
    int count, head, tail;

    sc_event item_added;
    sc_event item_removed;

    void addItem() {
        while (true) {
            if (count == BUFFER_SIZE) {
                full_o.write(true);
                wait(item_removed);
                full_o.write(false);
            }    

            wait(produced_i.posedge_event());

            buffer[head] = item_i.read();
            head = (head + 1) % BUFFER_SIZE;
            count++;

            item_added.notify();
        }
    }

    void consumeItem() {
        while (true) {
            if (count == 0) {
                wait(item_added);
            }

            int item = buffer[tail];
            tail = (tail + 1) % BUFFER_SIZE;
            count--;

            item_removed.notify();

            cout << "CONSUMER: " << item << endl;

            wait(8, SC_NS);
        }
    }

    SC_CTOR(consumer) {
        count = 0;
        head = 0;
        tail = 0;

        SC_THREAD(addItem);
        SC_THREAD(consumeItem);
    }
};

int sc_main(int argc, char* argv[]) {
    srand(time(0));

    sc_signal<bool> full_s;
    sc_signal<bool> produced_s;
    sc_signal<sc_uint<8>> item_s;

    producer producer_inst("produer_inst");
    producer_inst.full_i(full_s);
    producer_inst.produced_o(produced_s);
    producer_inst.item_o(item_s);

    consumer consumer_inst("consumer_inst");
    consumer_inst.item_i(item_s);
    consumer_inst.produced_i(produced_s);
    consumer_inst.full_o(full_s);

    full_s.write(false);
    produced_s.write(false);

    sc_start(25, SC_NS);

    return 0;
}