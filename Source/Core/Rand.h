#pragma once
#include <iostream>

class Rand {
private:
	Rand();
	static int next;

public:
	static const int RANDMAX = 32768;
	static int rand() {
		next = next * 1103515245 + 12345;
		return((unsigned)(next / 65536) % RANDMAX);
	}

	static void srand(unsigned seed) {
		next = seed;
	}
};