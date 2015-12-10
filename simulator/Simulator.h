#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>
#include <queue>

#include "Client.h"

class Simulator
{
     	private:
		std::priority_queue<Event*> event_queue;	

	public:
		Simulator(){
			client = new Client();
		}	
		void start(){
			event_queue.push(client->getCommand());
			while(event_queue.size() != 0){
				Event* event = event_queue.top();
				event->handle();
				event_queue.pop();
				if(ClientCommandEvent* clientCommandEvent = dynamic_cast<ClientCommandEvent*>(event)){
					std::cout<<"Client command received at " << clientCommandEvent->start_time << std::endl;
					event_queue.push(client->getCommand());
				}
				delete event;
			}
			std::cerr<<"No More Events to Simulate !!"<<std::endl;	
		}
		
		void set_number_nodes(int number_nodes){
			cluster = new std::vector<Node>(number_nodes);
		}

		void set_disk_write_bandwidth(double mean, double var){
			disk_write_bandwidth = new std::normal_distribution<double>(mean, var);
		}
		
		void set_disk_read_bandwidth(double mean, double var){
			disk_read_bandwidth = new std::normal_distribution<double>(mean, var);
		}

		void set_network_latency(double mean, double var){
			network_latency = new std::normal_distribution<double>(mean, var);
		}
		
		void set_network_bandwidth(double mean, double var){
			network_bandwidth = new std::normal_distribution<double>(mean, var);
		}
};
 
#endif
