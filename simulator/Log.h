#ifndef LOG_H
#define LOG_H



#include <vector>



class Log {

	private :

	public :
	
	Log ()
	{
	
	}

};

Log::AppendToLeaderLogEvent(leader,latency_disk, entry_to_be_logged){
// Get write latency to write to disk 
// Write the entry 
// Generate the AppendToLeaderReturnEvent
//
}

Log::AppendToLeaderLogReturnEvent(){
// Get disk response latency (Similar to ACK)
//Generate AppendRPCEntries Event - To replicate logs in the follower  
//
}
Log::AppendRPCEvent() {
// Call AppendToFollowerLogEvent
for every follower 
	AppendToFollowerLogEvent(node, latency_disk , entry_to_be_logged)
}

Log::AppendToFollowerLogEvent() {
// Log Consistency checks -- Here or at the node ?
// Reply false if node_term < Current_term 
}

Log::AppendToFollowerLogReturnEvent() {

}

#endif

