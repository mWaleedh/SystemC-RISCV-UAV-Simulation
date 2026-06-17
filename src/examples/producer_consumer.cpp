/* Problems:
Initially I was using SC_METHOD and put consumer and producer flags' rising edge into its sensitivity list.
So that is why I was doing consumer = True and immediately consumer = false to simulate a rising edge.
But since there was no delay between them sometimes the rising edge did register, so I decided to add a wait() between them.
But since I was using SC_METHOD, I couldn't place a wait() call in it so I had to switched it to SC_THREAD.

Another problem that I faced was, I was getting an error when I ran wait(consumer.pos()), it was giving invalid type error
*/

#include <iostream>
#include <systemc.h>
using namespace std;

SC_MODULE(producer) {
    // input/output ports
    sc_in<bool> consumed_i;
    sc_out<sc_uint<8>> item_o;
    sc_out<bool> produced_o;

    // local variables
    sc_uint<8> item_s;

    void produceItem() {
        while (true) {
            // wait for consumer to consume the item and set the flag to true
            wait(consumed_i.posedge_event());

            // generate new value and write to output port (produce)
            item_s = rand() % 255;
            item_o.write(item_s);

            cout << "PRODUCER: " << item_s << endl;

            // set flag to true first so that consumer is called
            produced_o.write(true);
            wait(1, SC_NS);
            // set it back to false immediately
            produced_o.write(false);
        }
    }   

    SC_CTOR(producer) {
        SC_THREAD(produceItem);
    }
};

SC_MODULE(consumer) {
    sc_in<bool> produced_i;
    sc_in<sc_uint<8>> item_i;
    sc_out<bool> consumed_o;

    sc_uint<8> item_s;

    void consumeItem() {
        while (true) {
            // wait for producer to produce an item and set the flag to true
            wait(produced_i.posedge_event());

            // read generated value (consume)
            item_s = item_i.read();

            cout << "CONSUMER: " << item_s << endl;

            // set flag to true first so that producer is called
            consumed_o.write(true);
            // set it back to false immediately
            consumed_o.write(false);
        }
    }

    SC_CTOR(consumer) {
        SC_THREAD(consumeItem);
    }
};

int sc_main(int argc, char* argv[]) {
    srand(time(0));

    // signals
    sc_signal<bool> consumed_s;
    sc_signal<bool> produced_s;
    sc_signal<sc_uint<8>> item_s;

    // input/output ports to signal for producer
    producer producer_inst("produer_inst");
        producer_inst.consumed_i(consumed_s);
        producer_inst.produced_o(produced_s);
        producer_inst.item_o(item_s);

    // input/output ports to signal for producer
    consumer consumer_inst("consumer_inst");
        consumer_inst.produced_i(produced_s);
        consumer_inst.consumed_o(consumed_s);
        consumer_inst.item_i(item_s);

    // initially both flags are set to false
    consumed_s.write(false);
    produced_s.write(false);

    sc_start(1, SC_NS);

    // set consumed flag to true (0 -> 1) so that producer function is called
    consumed_s.write(true);
    sc_start(1, SC_NS);
    consumed_s.write(false);

    sc_start(50, SC_NS);

    return 0;
}