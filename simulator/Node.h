#ifndef NODE_H
#define NODE_H

enum STATE{LEADER, FOLLOWER};

class Command;

class Node
{
     	private:
	public:
		long time;     
		std::vector<std::pair<int, Command> >* log;

		Node(){
			time = 0;
			log = new std::vector<std::pair<int, Command> >();
		}
};
 
#endif
