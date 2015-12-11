#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <time.h>
#include <algorithm>
#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

bool logging=false;

void log(const std::string fmt_str, ...) {
	if(!logging){
		return;
	}
	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
	std::string str;
	std::unique_ptr<char[]> formatted;
	va_list ap;
	while(1) {
		formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
		strcpy(&formatted[0], fmt_str.c_str());
		va_start(ap, fmt_str);
		final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
		va_end(ap);
		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	cout<<std::string(formatted.get())<<endl;
}

std::string int_to_string(int a){
	std::stringstream ss;
	ss << a;
	return ss.str();
}

template <class T>
std::string any_to_string (const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

std::string random_string(size_t length)
{
	auto randchar = []() -> char
	{
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[ rand() % max_index ];
	};
	std::string str(length,0);
	std::generate_n( str.begin(), length, randchar );
	return str;
}
