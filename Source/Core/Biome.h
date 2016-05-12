#pragma once

#include <random>
#include <math.h>
#include <Ogre.h>
#include "Rand.h"

#include "CubeManager.h"

class Biome {
public:
	Biome(Ogre::SceneManager*, CubeManager::CubeType, int, int, int);
	~Biome(void);

	bool inBiome(int, int);
	bool inRadius(float, float);
	float getSteepnessDivider(float, float);
	Ogre::Entity* getCubeEntity(int, int, int, CubeManager::CubeType&);
	CubeManager::CubeType getType() { return biomeType; }

protected:
	Ogre::SceneManager* mSceneManager;
	CubeManager::CubeType biomeType;
	int centerX;
	int centerY;
	int radius;
};