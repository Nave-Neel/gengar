#include <iostream>
#include <queue>
#include <vector>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
#include <chrono>

#include "../common/util.h"

enum STATE{LEADER, FOLLOWER};

//some forward declarations for the globals
class Client;
class Node;

//-----------------Globals---------------------//
std::default_random_engine generator;
std::normal_distribution<double>* network_latency;
std::normal_distribution<double>* network_bandwidth;
std::normal_distribution<double>* disk_read_bandwidth;
std::normal_distribution<double>* disk_write_bandwidth;
std::vector<Node>* cluster;
Client* client;
//-----------------Globals---------------------//

class Command
{
     	private:
		int command_id;
		std::string command_string;

	public:
		Command(int id, std::string command){
			this->command_id = id;
			this->command_string = command;
		}
};

class Node
{
     	private:
	public:
		long time;     
		int term;
		STATE state;
		int votes_received;
		int voted_for;
		long last_timestamp;
		std::vector<std::pair<int, Command> >* log;
		
		Node(){
			time = 0;
			term = 0;
			state = FOLLOWER;
			votes_received = 0;
			voted_for = -1;
			last_timestamp = 0;
			//log = new std::vector<std::pair<int, Command> >();
		}

		int get_last_index(){
			return log->size();
		}

		int get_last_term(){
			return log->back().first;
		}
};

class Event
{
     	private:

	public:
		std::vector<Event> generated_events;
		Node executed_on;
		long start_time;
		//should be a pure virtual
		virtual void handle() {};
};

class ClientCommandEvent : public Event{
     	
	private:
		Command* command;

	public:
		ClientCommandEvent(Command* command, long start_time){
			this->command = command;
			this->start_time = start_time;
		}
		
		virtual void handle() override {
			return;
		}
		//needs a virtual destructor to clean up the command on delete
		
		~ClientCommandEvent(){
			delete command;
		}
};


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

