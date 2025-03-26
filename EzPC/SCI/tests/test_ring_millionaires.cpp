#include "Millionaire/millionaire.h"
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

//test_ring_argmax is good

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
    
    uint64_t *data = new uint64_t[num_cmps];
    uint8_t *res = new uint8_t[num_cmps];

    if (party == ALICE) {
        prg.random_data(data, num_cmps * sizeof(uint64_t));
        
        uint64_t comm_start = iopack->get_comm();
        auto start = clock_start();

        millionaire_proc.compare(res, data, num_cmps, bitlength,
                true, false, 4);
        
        long long t = time_from(start);
        uint64_t comm_end = iopack->get_comm();

        cout << "Comparison Time\t" << RED << t * 1e6
         << " sec" << RESET << endl;
        cout << "ALICE communication\t" << BLUE
         << (comm_end - comm_start) * 8 << " bits" << RESET << endl;
        std::cout << "ALICE: Done Millionaire protocol execution" << std::endl;

        std::cout << "Checking Millionaire's correctness" << std::endl;
        uint64_t *data_BOB = new uint64_t[num_cmps];
        uint8_t *res_BOB = new uint8_t[num_cmps];
        iopack->io->recv_data(data_BOB, num_cmps * sizeof(uint64_t));
        iopack->io->recv_data(res_BOB, num_cmps * sizeof(uint8_t));
        for (int i = 0; i < num_cmps; i++) {
	    if ((data[i] > data_BOB[i]) == (res[i] ^ res_BOB[i])) {
		printf("m success");
	    } 
	    else {
		printf("m failed ");
	    }
	    printf(" res: %u", res[i] ^ bob_res[i]);
            std::cout << " ALICE data: " << data[i] << " BOB data: " << data_BOB[i] << std::endl;
        }	
    }
    else { // party == BOB
        prg.random_data(data, num_cmps * sizeof(uint64_t));
        
        uint64_t comm_start = iopack->get_comm();
        auto start = clock_start();

        millionaire_proc.compare(res, data, num_cmps, bitlength,
                true, false, 4);
        
        long long t = time_from(start);
        uint64_t comm_end = iopack->get_comm();

        cout << "Comparison Time\t" << RED << t * 1e6
         << " sec" << RESET << endl;
        cout << "BOB communication\t" << BLUE
         << (comm_end - comm_start) * 8 << " bits" << RESET << endl;
        std::cout << "BOB: Done Millionaire protocol execution" << std::endl;

        iopack->io->send_data(data, num_cmps * sizeof(uint64_t));
        iopack->io->send_data(res, num_cmps * sizeof(uint8_t));
    }
    return 0;
}
    
