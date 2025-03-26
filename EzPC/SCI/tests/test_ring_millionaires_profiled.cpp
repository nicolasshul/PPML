#include "Millionaire/millionaire_profiled.h"
#include <fstream>
#include <iostream>
#include <thread>

using namespace sci;
using namespace std;

int party, port = 32000;
int bitlength = 64;
int num_threads = 1;
int num_cmps = 8;
string address = "127.0.0.1";


void compare(MillionaireProtocol millionaire_proc, int num_cmps, uint64_t *alice_data, uint64_t *bob_data, IOPack *iopack) {
    struct mill_timer timer = { 0 };
    struct mill_timer *p_timer = &timer;
    uint8_t *res = new uint8_t[num_cmps];
    if (party == ALICE) {
        uint64_t *data = alice_data;

        millionaire_proc.compare(res, data, num_cmps, bitlength, p_timer,
                true, false, 4);
        
        std::cout << "Checking Millionaire's correctness" << std::endl;
        
        uint8_t *bob_res = new uint8_t[num_cmps];

        double *bob_timer = new double[3];

        //iopack->io->recv_data(bob_timer, sizeof(mill_timer));
        iopack->io->recv_data(bob_res, num_cmps * sizeof(uint8_t));

        for (int i = 0; i < num_cmps; i++) {
	    if ((data[i] > bob_data[i]) == (res[i] ^ bob_res[i])) {
		    printf("m success");
	    } 
	    else {
		    printf("m failed ");
	    }
	    printf(" res: %u", res[i] ^ bob_res[i]);
            std::cout << " ALICE data: " << data[i] << " BOB data: " << bob_data[i] << std::endl;
        }
        printf("\n TIMING: \n");
        if (bitlength < MILL_PARAM) {
            printf("Bitlength < beta: %f", timer.bitlength_less_than_beta);
        } 
        else {
            printf("Total Time Alice: %f\n", timer.alice_total * 1e-3);
            printf("Total Time Bob: %f s\n", timer.bob_total * 1e-3);
            printf("Extracting data: %f s\n", timer.extracting_data * 1e-3);
            printf("Setting Leaf OTs: %f s\n", timer.set_leaf_ots * 1e-3);
            printf("Alice preforming Leaf OTs: %f s\n", timer.alice_preform_leaf_ots * 1e-3);
            printf("Bob preforming Leaf OTs: %f s\n", (timer.bob_total - timer.extract_result) * 1e-3);
            printf("Extracting results from OTs: %f s\n", timer.extract_result * 1e-3);
        }
    }
    else { // party == BOB
        uint64_t *data = bob_data;

        millionaire_proc.compare(res, data, num_cmps, bitlength, p_timer,
                true, false, 4);

        //double bob_timer_data[] = {bob_timer.bob_total, bob_timer.extract_result, bob_timer.bob_preform_leaf_ots};
        //iopack->io->send_data(bob_timer_data, sizeof(mill_timer));
        iopack->io->send_data(res, num_cmps * sizeof(uint8_t));
    }
}

int main(int argc, char **argv) {
    ArgMapping amap;
    amap.arg("r", party, "Role of party: ALICE = 1; BOB = 2");
    amap.arg("p", port, "Port Number");
    amap.arg("l", bitlength, "Bitlength of inputs");
    amap.arg("N", num_cmps, "Number of comparisons");
    amap.arg("ip", address, "IP Address of server (ALICE)");

    amap.parse(argc, argv);
    PRG128 prg;
    IOPack *iopack = new IOPack(party, port, address);

    OTPack otpack(iopack, party);

    MillionaireProtocol millionaire_proc(party, iopack, &otpack, bitlength, MILL_PARAM);

    uint64_t *alice_data = new uint64_t[num_cmps];
    uint64_t *bob_data = new uint64_t[num_cmps];

    prg.random_data(alice_data, num_cmps * sizeof(uint64_t));
    prg.random_data(bob_data, num_cmps * sizeof(uint64_t));

    compare(millionaire_proc, num_cmps, alice_data, bob_data, iopack);
}
