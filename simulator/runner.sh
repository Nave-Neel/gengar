rm -rf times.txt
rm -rf sorted_times.txt
COUNTER=0
while [  $COUNTER -lt 1000 ]; do
	./exe_raft 2>> times.txt
	let COUNTER=COUNTER+1 
done
python sort.py
