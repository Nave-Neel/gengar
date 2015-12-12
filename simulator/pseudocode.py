TimeoutEvent(node, start_time, timeout_interval){
	node.time = starttime
	if(start_time - timeout_interval >= node.last_timestamp):
		#timeout has occured
		node.term++
		node.state = candidate
		node.votesReceived=1
		node.votedFor=node#Persistent
		#We assume the duration of the election timeout is the same as that of the normal timeout
		#We set the last_timestamp to the current time so that if the current election timesout, then we can compare the 
		node.last_timestamp = start_time
		for each other_node:
			network_delay = random_distribution()
			generate RequestVoteEvent(other_node, start_time + network_delay, node, node.term, node.log.end().term, node.log.end().index)#generate here means push back ot the events list that is present for each event
		timeout = random_distribution()
		generate TimeoutEvent(node, start_time + timeout, timeout) 
}
# The TimeoutEvent can also be set by the AppendRPCEvent - with some jittering - think how this would interact with this timeout

RequestVote(node, start_time, candidate, candidate_term, candidate_last_term, candidate_last_index){
	network_delay = random_distribution()
	#If voting node's term is strictly greater than candidate term, the candidate's term needs to be updated - note that this is different from updating the log
	#If a candidate's term is outdated, there is no way it can have entries from a later term(what if there are no entries from later term?), we can disregard the candidate then
	if(node.term > candidate_term):
		generate RequestVoteResponseEvent(candidate, start_time + network_delay, node, false, true, node.term)
	if(node.state == LEADER):
		if(node.term < candidate_term):
			#Leader needs to step down if the packet term is strictly greater - this is the case of the node receiveing the RequestVote being an old leader that has not observed a previous election
			node.state = FOLLOWER
			generate RequestVoteResponseEvent(candidate, start_time + network_delay, node, true, false, node.term)
		else:
			#Reject re-election as you are the leader - the candidate could have caused the election as it did not get the heartbeat due to some network failure but the leader still exists
			generate RequestVoteResponseEvent(candidate, start_time + network_delay, node, false, false, node.term)
	#Not already voted for someone else
	if(node.votedFor != NULL):
		generate RequestVoteResponseEvent(candidate, start_time + network_delay, node, false, false, node.term)	
	#Log consistency check
	if(node.log.end().term > candidate_last_term OR (node.log.end().term == candidate_last_term AND node.log.end().index > candidate_index)):
		generate RequestVoteResponseEvent(candidate, start_time + network_delay, node, false, false, node.term)
        else:
		#VOTE
		#If the RequestVote is coming from a candidate that has wrongly called for an election as some heartbeats did not reach it, then won't this case be hit and tus cause an unnecessary increase
		#in node term since the leader is still present? 
		#node.term++ - i don't think the voter increments its term - this only gets updated later from an AppendEntries call from the leader, assuming the voter does not become a candidate in the meantime
		node.votedFor = candidate #Persistent
		node.last_timestamp = start_time
		generate ResponseVote_Send_Event(10, from_node, target_node, true, false, node.term)
		generate TimeoutEvent(node, start_time + timeout, timeout) 
}


ReceiveVoteEvent(node, start_time, voting_node, voted, update, voted_node.term){
	if(update):
		node.term = voted_node.term
		node.state = FOLLOWER
		return
	if(voted):
		#check still a candidate
		if(node.state == CANDIDATE):
			node.votesReceived++
			if(notes.votesReceived > cluster.size()/2
				node.state = LEADER
				node.client_request_replicas = new Hashmap(cmd, int)	
				node.client_request_cache = new Hashmap(cmd, string)
				node.next_request = new Hashmap(int, int)
				for each other_node:
					network_delay = random_distribution()
					#assert LEADER status
					#leader will immediately start to make followers logs consistent? - is this already mentioned in paper or can it be an optimization?
					#actually let's assume that the leader will only start to make the followers consistent once a command comes in
					other_node.next_request[node] = node.log.size()
					generate AppendEntriesEvent(other_node, start_time + network_delay, NULL, node.term, node, node.log.end().index, node.log.end().term, )				
}

AppendEntriesEvent(node, start_time, command, leader_term, leader_node, leader_preceding_index, leader_preceding_term, commitIndex?){
		if(leader_term > node.term):
			node.state = FOLLOWER
			node.term = leader_term
			node.votes_received=0
			node.voted_for=-1
		node.last_timestamp = start_time
		timeout = random_distribution()
		generate TimeoutEvent(node, start_time + timeout, timeout) 
}

ClientCommandEvent(node, start_time, command){
		if(node.state == LEADER){
			#check cache OR insert into cache
			if(command.id in node.client_request_cache and node.client_request_cache[command.id] != NULL){
				return node.client_request_cache[command.id]
			}
			else{
				node.client_request_replicas[command.id] = 1
				node.client_request_cache[command.id] = null
			}
			disk_latency = random_distribution() 
			log.add(command)
			for each other_node:
				#check only for nodes that are already upto date and send the new command only to these, for the rest, the normal operation of AppendRPC will take care of replicating this cmd
				node.next_request[other_node] == node.log.size():
					network_delay = random_distribution()
					#actually this is not even needed as the heartbeat will take care of propagating these commands - i guess this is like an optimziation to get the replication done faster
					generate AppendEntriesEvent(other_node, start_time + disk_latency + network_delay, command, node.term, node.log[command].prev().index, node.log[command].prev().term)	
		} 
		if(node.state == FOLLOWER){
			network_delay = random_distribution()
			generated ClientCommandEvent(node.leader, start_time + network_delay, command)
		}
		if(node.state == CANDIDATE){
			throw error("Not sure what to do")
		}		
}

#heartbeat needs to check if the log is consistent and if so send a null and if not send the next

#How does this handle heartbeats?
AppendEntriesEvent(node, start_time, leader_term, leader_node, leader_previous_index, leader_previous_term, entry, leader_commit){
		network_delay = random_distribution()
		disk_delay = random_distribution()
		if(leader_term < node.term):
			generate AppendEntresResponseEvent(leader_node, start_time + network_latency, node, false, node.term)
			return
		if(leader_term > node.term):#is this a strict greater than? what if 2 followers with the same term initially both timed out and incremented the counter, and then one won the election? - wont their 2 terms be equaivalent?
			node.state = FOLLOWER
			node.term = leader_term
			node.votes_received=0
			node.voted_for=-1
		node.last_timestamp = start_time
		timeout = random_distribution()
		generate TimeoutEvent(node, start_time + timeout, timeout)
		node.leader = leader_node;
		if(node.end().term == leader_previous_term && node.end().index == leader_previous_index):#perfectly consistent log
			if command != null:#not a heartbeat
				log.add(command)
				generate AppendEntresResponseEvent(leader_node, start_time + disk_latency + network_latency, node, true, node.term)
				if leader_commit > node.commit and leader_commit < log.size()-1:
					'''
					for(int c=node.commit+1; c<leader_commit; ++c){
						applytostatemachine(node.log[c])#dummy method, does't do anything as we dont model the time to execute a command
					}
					'''
					node.commit = leader_commit
		else{
			bool found = False
			i=0
			for(; i<node.log.size(); ++i){
				if(node.log[i].term == leader_previous_term && node.log[i].term == leader_previous_term)#the previous entry is there in the follower log
					found = True
					break
			}
			if found:
				#remove extra entries - its ok to remove these as these cannot be commited as they are not in the leader
				for(; i<node.log.size(); ++i){
					node.log.remove(i)
				}
				#commit the new command from after the previous point
				if command != null:#not a heartbeat
					log.add(command)
					generate AppendEntriesResponseEvent(leader_node, start_time + disk_latency + network_latency, node, true, node.term)
					if leader_commit > node.commit and leader_commit < log.size()-1:
						'''
						for(int c=node.commit+1; c<leader_commit; ++c){
							applytostatemachine(node.log[c])#dummy method, does't do anything as we dont model the time to execute a command
						}
						'''
						node.commit = leader_commit
					
			else:
				#could not find the previous, need to insert previous as well
				generate AppendEntresResponseEvent(leader_node, start_time + disk_latency + network_latency, node, leader_prev_index+1, false, node.term)
		}
}

AppendEntriesResponseEvent(node, start_time, follower_node, command_index, success, follower_term){
	assert command_index < node.log.size()
	network_delay = random_distribution()
	if success:
		#next_request is a misnomer, this is the last replicated cmd in reality
		node.next_request[follower_node] = command_index++
		#for(int i=0; i<node.next_request[follower_node]; ++i){#hmm..this will double count? -- how to keep track of it original nodes in the leaders log have been committed?
			node.client_request_replicas[command.id]++
		#}
		#command is committed - actually this needs a stronger check
		if node.client_request_replicas[command.id]++ > cluster.size()/2:
			commit_index = command_index#in the next heartbeat, this will tell the followers that this cmd has been committed - so no need to send separate message for that
			generate ClientResponseEvent()	
		#check if there is another command in the leader log which needs to be passed to the follower
		if node.next_request[follower_node] != node.log.size():
			generate AppendEntriesEvent(follower_node, start_time + network_delay, node.log[node.next_request[follower_node]], node.term, node.log[node.next_request[follower_node]-1].index,  node.log[node.next_request[follower_node]-1].term)	
		else:
			#HEARTBEAT_INTERVAL is a global constant
			generate AppendEntriesEvent(follower_node, start_time + HEARTBEAT_INTERVAL + network_delay, NULL, node.term, node.log.end().index, node.log.end().term, commitIndex)	
			
	else:
		#go back to follower
		if node.term < follower_term:
			node.state = FOLLOWER
			node.term = leader_term
			node.votes_received=0
			node.voted_for=-1
			return
		#the append failed as the previous was not in follower log, now try with previous
		else:
			node.next_request[follower_node] = command_index - 1;
			generate AppendEntriesEvent(follower_node, start_time + network_delay, node.log[node.next_request[follower_node]], node.term, node.log[node.next_request[follower_node]].prev().index,  node.log[node.next_request[follower_node]].prev().term)	
}

ClientResponseEvent(){
#implement this later - generate some string to put in node.client_request_cache
}
