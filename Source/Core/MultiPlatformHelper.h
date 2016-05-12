#pragma once

#include <string>
#include <iostream>
#ifdef __linux__
#include <unistd.h>
#include <cstring>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

namespace MultiPlatformHelper {
	// Print to console
	inline void print(std::string s) {
		#ifdef __linux__
			std::cout << s;
		#endif

		#ifdef _WIN32
			OutputDebugString(s.c_str());
		#endif	
	}

	// Thread sleep
	inline void sleep(int sleepMs)
	{
		#ifdef __linux__
				usleep(sleepMs * 1000);   // usleep takes sleep time in us (1 millionth of a second)
		#endif
		#ifdef _WIN32
				Sleep(sleepMs);
		#endif
	}

	inline char* strtok(char *str, const char *delim, char **saveptr) {
		#ifdef __linux__
			return strtok_r(str, delim, saveptr);
		#endif
		#ifdef _WIN32
			return strtok_s(str, delim, saveptr);
		#endif
	}
}