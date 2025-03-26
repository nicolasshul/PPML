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

mill_timer compare(MillionaireProtocol millionaire_proc, int num_cmps, uint64_t *alice_data, uint64_t *bob_data) {
    uint8_t *res = new uint8_t[num_cmps];
    if (party == ALICE) {
        uint64_t *data = alice_data;
        uint64_t comm_start = iopack->get_comm();
        auto start = clock_start();

        mill_timer alice_timer = millionaire_proc.compare(res, data, num_cmps, bitlength,
                true, false, 4);
        
        long long t = time_from(start);
        uint64_t comm_end = iopack->get_comm();

        cout << "Comparison Time\t" << t * 1e6
         << " sec" << RESET << endl;
        cout << "ALICE communication\t" << BLUE
         << (comm_end - comm_start) * 8 << " bits" << RESET << endl;
        std::cout << "ALICE: Done Millionaire protocol execution" << std::endl;

        std::cout << "Checking Millionaire's correctness" << std::endl;
        uint8_t *bob_res = new uint8_t[num_cmps];

        mill_timer bob_timer;

        iopack->io->recv_data(bob_timer, sizeof(mill_timer));
        iopack->io->recv_data(bob_res, num_cmps * sizeof(uint8_t));

        for (int i = 0; i < num_cmps; i++) {
	    if ((data[i] > bob_data[i]) == (res[i] ^ bob_res[i])) {
		    printf("m success");
	    } 
	    else {
		    printf("m failed ");
	    }
	    printf(" res: %u", res[i] ^ bob_res[i]);
            std::cout << " ALICE data: " << data[i] << " BOB data: " << data_BOB[i] << std::endl;
        }
        printf("\n TIMING: \n");
        if (bitlength < MILL_PARAM) {
            printf("Bitlength < beta: %llu", alice_timer.bitlength_less_than_beta);
        } 
        else {
            printf("Total Time Alice: %llu\n", alice_timer.alice_total * 1e6);
            printf("Total Time Bob: %llu s\n". bob_timer.bob_total * 1e6);
            printf("Extracting data: %llu s\n", alice_timer.extracting_data * 1e6);
            printf("Setting Leaf OTs: %llu s\n", alice_timer.set_leaf_ots * 1e6);
            printf("Alice preforming Leaf OTs: %llu s\n", alice_timer.alice_preform_leaf_ots * 1e6);
            printf("Bob preforming Leaf OTs: %llu s\n", bob_timer.bob_preform_leaf_ots * 1e6);
            printf("Extracting results from OTs: %llu s\n", bob_timer.extracting_data * 1e6);
        }
    }
    else { // party == BOB
        uint64_t *data = bob_data;

        uint64_t comm_start = iopack->get_comm();
        auto start = clock_start();

        mill_timer bob_timer = millionaire_proc.compare(res, data, num_cmps, bitlength,
                true, false, 4);
        
        long long t = time_from(start);
        uint64_t comm_end = iopack->get_comm();

        cout << "Comparison Time\t" << RED << t * 1e6
         << " sec" << RESET << endl;
        cout << "BOB communication\t" << BLUE
         << (comm_end - comm_start) * 8 << " bits" << RESET << endl;
        std::cout << "BOB: Done Millionaire protocol execution" << std::endl;

        iopack->io->send_data(bob_timer, sizeof(mill_timer));
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

    compare(millionaire_proc, num_cmps, alice_data, bob_data);
}
    
