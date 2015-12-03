#ifndef EVENT_H
#define EVENT_H

#include <vector>

#include "Global.h"

class Event
{
     	private:

	public:
		Node* event_node;
		long start_time;
		virtual std::vector<Event*> handle();
};
 
#endif
