#include "Simulator.h"

int main(int argc, char* argv[]){
	Simulator simulator;
	simulator.set_number_nodes(3);
	//1000 uS - 1 ms
	simulator.set_network_latency(7000, 200);
	//10 bytes/uS - 10Mbps
	simulator.set_network_bandwidth(10, 3);
	//100 bytes/uS - 100Mbps
	simulator.set_disk_read_bandwidth(100, 10);
	//30 bytes/uS - 30Mbps
	simulator.set_disk_write_bandwidth(30, 5);
	//various other parameters
	simulator.start();
}
