#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
#include <chrono>

#include "ClientCommandEvent.h"
#include "../common/util.h"

class Client
{
     	private:
		std::normal_distribution<double>* next_request_time;
		std::default_random_engine* generator;	

	public:
		int requestID;
		long time;	
	
		Client(){
			time = 0;
			next_request_time = new std::normal_distribution<double>(2000, 1000);
			requestID = 0;
			unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  			generator = new std::default_random_engine(seed);
		}
		
		ClientCommandEvent* getCommand(){
			std::string random_command = random_string(10);	
			requestID++;
			Command* cmd = new Command(requestID, random_command);
			long random_time = (*next_request_time)(*generator);	
			time += random_time;
			return new ClientCommandEvent(cmd, time);
		}
};
 
#endif
