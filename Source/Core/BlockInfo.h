#pragma once
#include "CubeManager.h"

struct BlockInfo {
	int x;
	int y;
	int z;
	CubeManager::CubeType type;
	BlockInfo(int x1, int y1, int z1, CubeManager::CubeType tp) : x(x1), y(y1), z(z1), type(tp) {}
};