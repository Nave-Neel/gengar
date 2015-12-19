#include <iostream>
#include <queue>
#include <set>
#include <vector>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
#include <chrono>
#include <assert.h>
 
using std::cout;
using std::endl;
using std::string;

#include "../common/util.h"

enum STATE{LEADER, FOLLOWER, CANDIDATE};

bool leader_selected = false;
long time_to_elect;

//some forward declarations for the globals
class Client;
class Node;
class TimeoutEvent;

extern TimeoutEvent* getTimeoutEvent(int, int, int);

//-----------------Globals---------------------//
std::default_random_engine* generator;
std::uniform_int_distribution<int>* timeout;
std::normal_distribution<double>* network_latency;
std::normal_distribution<double>* network_bandwidth;
std::normal_distribution<double>* disk_read_bandwidth;
std::normal_distribution<double>* disk_write_bandwidth;
std::normal_distribution<double>* disk_write_latency;
std::vector<Node*> cluster;
Client* client;
//-----------------Globals---------------------//

double get_random(std::normal_distribution<double>* distribution){
	return (*distribution)(*generator); 
}

double get_uniform(std::uniform_int_distribution<int>* distribution){
	return (*distribution)(*generator); 
}

class Command
{
	private:

	public:
		int command_id;
		std::string command_string;
		/* Not sure why but this constructor does not work
		   Command(int id, std::string command){
		   this->command_id = id;
		   this->command_string = command;
		   }*/
};

class Node
{
	private:
	public:
		long time;     
		int term;
		STATE state;
		int votes_received;
		int votes_not_received;
		int voted_for;
		long last_timestamp;
		std::vector<std::pair<int, Command> > log;

		Node(){
			time = 0;// Global time seen at the node 
			term = 0;
			state = FOLLOWER;
			votes_received = 0;
			votes_not_received = 0;
			voted_for = -1;
			last_timestamp = 0; // Last received the message from leader 
		}

		int get_last_index(){
			return log.size() -1;
		}

		int get_last_term(){
			if(log.size()>0){
				return log.back().first;
			}
			else{
				return -1;
			}
		}
};

class Event{
	private:
	public:
		std::vector<Event*> generated_events;
		int executed_on;
		long start_time;
		virtual void handle() = 0;//pure virtual
		~Event(){}//in the virtual destructor, we do not delete the dynamically allocated Event* as they are added to the event queue and deleted later
};

class ClientCommandEvent : public Event{
	private:
		Command command;
	public:
		ClientCommandEvent(Command command, long start_time){
			this->command = command;
			this->start_time = start_time;
		}

		virtual void handle() override {
			return;
		}
};

class AppendEntriesEvent: public Event {
	private:
	public:
		string cmd;
		int leader_term;
		int leader_node;
		int leader_prec_index;
		int leader_prec_term;
		int commitindex;
 		
		AppendEntriesEvent(int node_index, int start_time, string command, int leader_node, int leader_term, int leader_prec_term, int leader_prec_index, int commitindex){
			this->executed_on = node_index;
			this->start_time = start_time;
			this->cmd = command;
			this->leader_term = leader_term;
			this->leader_node = leader_node;
			this->leader_prec_index = leader_prec_index;
			this->leader_prec_term = leader_prec_term;
			this->commitindex = commitindex;
		}
		
		virtual void handle() override {
			log("AppendEntries Event");
			Node* executed_on_node = cluster.at(executed_on);
			//is this a strict greater than?
			if(leader_term >= executed_on_node->term){
				executed_on_node->state = FOLLOWER;
				executed_on_node->term = leader_term;
				executed_on_node->votes_received = 0;
				executed_on_node->votes_not_received = 0;
				executed_on_node->voted_for = -1;
			}
			executed_on_node->last_timestamp = start_time;
			//should generate a new timeout here
		}
		
};

class ReceiveVoteEvent: public Event {
	private:
	public:
		int voting_node;
		bool voted;
		bool update;
		int voting_node_term;

		ReceiveVoteEvent(int node_index, long start_time, int voting_node, bool voted, bool update, int voting_node_term) {
			this->executed_on = node_index;
			this->start_time = start_time;
			this->voting_node =  voting_node;
			this->voted= voted;
			this->update= update;
			this->voting_node_term = voting_node_term;
		}

		virtual void handle() override {
			log("ReceiveVote Event voting node:%i, vote:%i, update:%i, voting_node_term:%i", voting_node, voted, update, voting_node_term);
			Node* executed_on_node = cluster.at(executed_on);
			executed_on_node->time = start_time;
			if(update){
				//assert(voting_node_term > executed_on_node->term);
				executed_on_node->term = voting_node_term;
				executed_on_node->state = FOLLOWER;
				return;
			}
			if(voted) {
				if(executed_on_node->state == CANDIDATE) {
					executed_on_node->votes_received++;
					long max_delay = 0;
					if (executed_on_node->votes_received > cluster.size()/2) {
						executed_on_node->state = LEADER;
						// Client request replicas -- mapping each replica to how many are present in each follower 
						// Client request caches -- mapping already computed requests to results
						// Client next request -- mapping nodes to the next request to send to them
						for(int n=0; n<cluster.size(); ++n){
							if(n!=executed_on){
								long network_delay = get_random(network_latency);
								if(max_delay < network_delay){
									max_delay = network_delay;
								}
								string s;
								generated_events.push_back(new AppendEntriesEvent(n, start_time+network_delay, s, executed_on, executed_on_node->term, executed_on_node->get_last_term(), executed_on_node->get_last_index(), 0));
							}
						}
						leader_selected=true;
						time_to_elect = start_time+max_delay;
					}
				}
			}
			else{
				if(executed_on_node->state == CANDIDATE) {
					executed_on_node->votes_not_received++;
					if (executed_on_node->votes_not_received > cluster.size()/2) {
						//let's reset the current timeout
						executed_on_node->last_timestamp = start_time;
						long random_time = get_uniform(timeout);
						//let's cause another timeout at 2*timeout of the present timeout
						generated_events.push_back(reinterpret_cast<Event*>(getTimeoutEvent(executed_on, start_time+2*random_time, random_time)));
					}
				}
			}
		}
};

class DiskWriteOtherVoteEvent: public Event {
	private:
	public:
		int candidateVotedFor;
		DiskWriteOtherVoteEvent(int node_index, long start_time, int candidateVotedFor){
			this->executed_on = node_index;
			this->start_time = start_time;
			this->candidateVotedFor = candidateVotedFor;
		}

		virtual void handle() override{
			log("Disk write before voting Event");
			Node* executed_on_node = cluster.at(executed_on);
			assert( executed_on_node->state = FOLLOWER);
			executed_on_node->voted_for = candidateVotedFor;
			long network_delay = get_random(network_latency);
			generated_events.push_back(new ReceiveVoteEvent(candidateVotedFor, start_time+network_delay, executed_on, true, false, executed_on_node->term));	
		}
};

class RequestVoteEvent: public Event {
	private:
	public:
		int candidate_term;
		int candidate;
		int candidate_last_term;
		int candidate_last_index;

		RequestVoteEvent(int node_index, long start_time, int candidate , int candidate_term, int candidate_last_term, int candidate_last_index) { 
			this->executed_on = node_index;
			this->start_time = start_time;
			this->candidate = candidate;
			this->candidate_term = candidate_term;
			this->candidate_last_term = candidate_last_term;
			this->candidate_last_index = candidate_last_index;
		}

		virtual void handle () override {
			log("Request Vote Event");
			long network_delay = get_random(network_latency);
			Node* executed_on_node = cluster.at(executed_on);
			executed_on_node->time = start_time;
			if(executed_on_node->term > candidate_term) {
				generated_events.push_back(new ReceiveVoteEvent(candidate, start_time+network_delay, executed_on, false, true, executed_on_node->term));
				log("larger term");	
				return;
			}
			if(executed_on_node->state == LEADER) {
				if (executed_on_node->term < candidate_term) {
					//the candidate has a higher term, so the supossed LEADER should go to follower state - this is probably a zombie leader that has not observed a previous election
					executed_on_node->state = FOLLOWER;
					generated_events.push_back(new ReceiveVoteEvent(candidate, start_time+network_delay, executed_on, true, false, executed_on_node->term));
				}
				else {
					// this is the leader, inform the candidate you are not voting for it - the candidate has probably lost a AppendRPC packet
					generated_events.push_back(new ReceiveVoteEvent(candidate, start_time+network_delay, executed_on, false, false, executed_on_node->term));
				}
				return;
			}
			//this check should be -1 consistently
			if(executed_on_node->voted_for != -1) {
				generated_events.push_back(new ReceiveVoteEvent(candidate, start_time+network_delay, executed_on, false, false, executed_on_node->term));
				log("voted already");	
				return;
			}
			//candidate could have incomplete log without all the committed entries
			if((executed_on_node->get_last_term() > candidate_last_term)||((executed_on_node->get_last_term()== candidate_last_term) && (executed_on_node->get_last_index() > candidate_last_index))) {
				generated_events.push_back(new ReceiveVoteEvent(candidate, start_time+network_delay, executed_on, false, false, executed_on_node->term));	
				log("log out of date");	
			}
			else {
				//if a timeout happens while the write to disk is happening, then the timeout will see this last timestamp 
				executed_on_node->last_timestamp = start_time;
				long random_time = get_uniform(timeout);
				//wow a reinterpret_cast -- bad
				generated_events.push_back(reinterpret_cast<Event*>(getTimeoutEvent(executed_on, start_time+random_time, random_time)));
				generated_events.push_back(new DiskWriteOtherVoteEvent(executed_on, start_time+network_delay, candidate));	
				log("voting");	
			}
		}
};

class DiskWriteTimeoutEvent: public Event {
	private:
	public:
		DiskWriteTimeoutEvent(int node_index, long start_time){
			this->executed_on = node_index;
			this->start_time = start_time;
		}

		virtual void handle() override{
			log("Disk write before Requesting vote Event");
			Node* executed_on_node = cluster.at(executed_on);
			//check if still candidate
			if(executed_on_node->state = CANDIDATE){
				executed_on_node->votes_received=1;
				executed_on_node->voted_for=executed_on;
				for(int n=0; n<cluster.size(); ++n){
					if(n!=executed_on){
						long network_delay = get_random(network_latency);
						generated_events.push_back(new RequestVoteEvent(n, start_time+network_delay, executed_on, executed_on_node->term, executed_on_node->get_last_term(), executed_on_node->get_last_index()));
					}
				}
			}
		}
};

class TimeoutEvent : public Event{
	private:
	public:
		long timeout_interval;
		TimeoutEvent(int node_index, long start_time, long timeout_interval){
			this->executed_on = node_index;
			this->start_time = start_time;
			this->timeout_interval = timeout_interval;
		}

		virtual void handle() override {
			log("Handling timeout on %i at %lu ", executed_on, start_time);
			Node* executed_on_node = cluster.at(executed_on);
			executed_on_node->time = start_time;
			if(start_time - timeout_interval >= executed_on_node->last_timestamp){
				executed_on_node->last_timestamp=start_time;
				long random_time = get_uniform(timeout);
				executed_on_node->voted_for = -1;
				executed_on_node->votes_received = 0;
				executed_on_node->votes_not_received = 0;
				//should the timeout include the disk delay - i dont think so as this would introduce some jittering by itself which would be enough for the timeout jittering? 
				//and the paper seems to make no mention of this? - but then this is the second timeout where you are waiting for response from candidate right?, maybe they
				generated_events.push_back(new TimeoutEvent(executed_on, start_time + random_time, random_time));
				executed_on_node->term++;						
				executed_on_node->state = CANDIDATE;
				//simulate write to disk the votedFor information
				long disk_delay = get_random(disk_write_latency);
				generated_events.push_back(new DiskWriteTimeoutEvent(executed_on, start_time + disk_delay));
			}
		}
};

TimeoutEvent* getTimeoutEvent(int node, int start_time, int timeout_interval){
	return new TimeoutEvent(node, start_time, timeout_interval);
}

class Client
{
	private:
		std::normal_distribution<double>* next_request_time;

	public:
		int requestID;
		long time;	

		Client(){
			time = 0;
			next_request_time = new std::normal_distribution<double>(2000, 1000);
			requestID = 0;
		}

		ClientCommandEvent* getCommand(){
			std::string random_command = random_string(10);	
			requestID++;
			long random_time = get_random(next_request_time);
			time += random_time;
			Command cmd;
			cmd.command_id = requestID;
			cmd.command_string = random_command;
			return new ClientCommandEvent(cmd, time);
		}

		~Client(){
			delete next_request_time;
		}
};


class Simulator
{
	private:
		//yes, it should be a priority queue of Event*, bit we run into all kinds of problems
		//	- cannot make a comparator on pointers
		//	- cannot make a comparator on abstract class
		//so, for now, taking the algorithmic hit and just iterating through to find the least
		std::set<Event*> event_queue;//has to be a pointer to events as we want to store events of different types	

	public:
		Simulator(){
			client = new Client();
			unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
			generator = new std::default_random_engine(seed);
		}	

		void start(){
			//event_queue.push(client->getCommand());
			long last_time;
			while(!event_queue.empty()){
				if(leader_selected){
					break;
				}
				std::set<Event*>::iterator it = event_queue.begin();
				Event* next = *it;
				for (; it != event_queue.end(); ++it){
					if(next->start_time > (*it)->start_time){
						next = *it;
					}
				}
				//remove event from event queue
				event_queue.erase(next);
				log("Executed on: %i", next->executed_on);
				last_time = next->start_time;
				log("Executed time: %lu", next->start_time);
				//handle the event - modifies state on the node
				next->handle();
				//getch();
				//add the generated events to the event queue
				for(int e=0; e<next->generated_events.size(); ++e){
					event_queue.insert(next->generated_events[e]);
				}
				//add to generated_events here 
				if(ClientCommandEvent* clientCommandEvent = dynamic_cast<ClientCommandEvent*>(next)){
					log("Client command received at %lu", clientCommandEvent->start_time);
					event_queue.insert(client->getCommand());
				}
				//handles the deletion of all the generated events
				delete next;
				log("==========================================");	
			}
			//std::cerr<<"No More Events to Simulate !!"<<std::endl;	
			//std::cerr<<"TIME TO ELECT: "<<time_to_elect<<std::endl;	
			std::cerr<<time_to_elect<<std::endl;	
		}

		void set_number_nodes(int number_nodes){
			for(int i=0; i<number_nodes; ++i){
				Node* node = new Node();
				cluster.push_back(node);
				long random_time = get_uniform(timeout);
				event_queue.insert(new TimeoutEvent(i, random_time, random_time));
			}
		}

		void set_disk_write_latency(double mean, double var){
			disk_write_latency = new std::normal_distribution<double>(mean, var);
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

		void set_timeout(int min, int max){
			timeout = new std::uniform_int_distribution<int>(min, max);
		}

		~Simulator(){
			delete client;
			delete generator;
			delete disk_write_bandwidth;
			delete disk_read_bandwidth;
			delete network_bandwidth;
			delete network_latency;
			delete timeout;
		}
};


int main(int argc, char* argv[]){
	Simulator simulator;
	//15ms
	simulator.set_network_latency(15, 4);
	//30000 uS - 3 ms
	simulator.set_timeout(150, 155);
	//20ms
	simulator.set_disk_write_latency(15, 1);
	/*
	//100 bytes/uS - 100Mbps
	simulator.set_disk_read_bandwidth(100, 10);
	//30 bytes/uS - 30Mbps
	simulator.set_disk_write_bandwidth(30, 5);
	//10 bytes/uS - 10Mbps
	simulator.set_network_bandwidth(10, 3);
	*/
	//various other parameters
	simulator.set_number_nodes(3);
	simulator.start();
}
