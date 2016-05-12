#pragma once
#include <random>
#include <vector>
#include <math.h>
#include <utility>
#include <unordered_map>

#include <Ogre.h>

#include "Biome.h"
#include "CubeManager.h"
#include "StaticObject.h"
#include "Rand.h"
#include "HashCombine.h"

static int biomeGridSize = 1500;
static int positionVariance = 100;
static int minBiomeRadius = 600;
static int biomeRadiusVariance = 100;

class BiomeManager {
public:
	BiomeManager(Ogre::SceneManager*);
	~BiomeManager(void);

	Biome* inBiome(int, int);
	Ogre::Entity* getTerrain(CubeManager::CubeType);

	bool isTreeType(CubeManager::CubeType);

protected:
	Biome* createBiome(CubeManager::CubeType, int, int, int);
	std::string getBiomeName(int, int);
	std::unordered_map<std::pair<int, int>, Biome*> biomeGrid;
	std::vector<Biome*> worldBiomes;
	Ogre::SceneManager* mSceneManager;
};