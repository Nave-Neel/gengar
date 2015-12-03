#ifndef EVENT_H
#define EVENT_H

#include <vector>

#include "Global.h"

class Event
{
     	private:

	public:
		long start_time;
		//should be a pure virtual
		virtual std::vector<Event*>* handle() {};
};
 
#endif
