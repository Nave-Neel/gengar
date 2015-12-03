#ifndef COMMANd_H
#define COMMAND_H

#include <string>

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
 
#endif
