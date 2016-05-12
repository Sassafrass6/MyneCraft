#pragma once
#include <iostream>
#include "Rand.h"
#include <vector>

/* PsedoCode taken from https://en.wikipedia.org/wiki/Perlin_noise */

class Perlin {
private:
	const static int xMax = 256;
	const static int yMax = 256;
	int size;
	float density; /* Density domain is (0, 10+). 0 is unpopulated 10 is extremely dense. */
	float gradient[xMax][yMax][2];

public:
	Perlin(float);
	void generateGradientTrivial();
	float lerp(float, float, float);
	float dotGridGradient(int, int, float, float);
	float getPerlin(float, float);
};