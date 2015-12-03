#ifndef CLIENTCOMMANDEVENT_H
#define CLIENTCOMMANDEVENT_H

#include "Event.h"
#include "Command.h"

class ClientCommandEvent : public Event{
     	
	private:
		Command* command;

	public:
		ClientCommandEvent(Command* command, long start_time){
			this->command = command;
			this->start_time = start_time;
		}
		
		virtual std::vector<Event*>* handle() override {
			return new std::vector<Event*>();
		}
		//needs a virtual destructor to clean up the command on delete
		
		~ClientCommandEvent(){
			delete command;
		}
};
 
#endif
