#include "Simulator.h"

int main(int argc, char* argv[]){
	Simulator simulator;
	simulator.set_number_nodes(3);
	simulator.set_network(1, 0.5);
	simulator.set_disk(5, 0.5);
	//various other parameters
	simulator.start();
}
