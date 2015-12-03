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
#include <dirent.h>
#include <unistd.h>

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
