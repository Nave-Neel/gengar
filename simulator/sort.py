input_file = open('times.txt', 'r')
times=[]
for line in input_file:
	times.append(int(line))
times.sort()
out_file = open('sorted_times.txt', 'w+')
for time in times:
	out_file.write(str(time))
	out_file.write("\n")
