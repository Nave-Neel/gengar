#ifndef EVENT_H
#define EVENT_H

#include <vector>

#include "Global.h"
//Every event does 2 things
//	- Changes state at a node
//	- Returns back a list of Events generated
//
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
 
#endif
