#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>
#include <queue>

#include "Event.h"

class Simulator
{
     	private:
		std::priority_queue<Event*> event_queue;	

	public:
		
		void start(){
			while(event_queue.size() != 0){
				Event* event = event_queue.top();
				event->handle();
				event_queue.pop();
				delete event;
			}
			std::cerr<<"No More Events to Simulate !!"<<std::endl;	
		}
		
		void set_number_nodes(int number_nodes){
			cluster = new std::vector<Node>(number_nodes);
		}

		void set_disk(double mean, double var){
			disk_bandwidth = new std::normal_distribution<double>(mean, var);
		}

		void set_network(double mean, double var){
			network_bandwidth = new std::normal_distribution<double>(mean, var);
		}
};
 
#endif
