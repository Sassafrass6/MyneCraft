#include "BiomeManager.h"

BiomeManager::BiomeManager(Ogre::SceneManager* smgr) : mSceneManager(smgr) {
	worldBiomes.push_back(createBiome(CubeManager::SAND, 0, 0, 0));
}


Biome* BiomeManager::inBiome(int x, int y) {
	std::pair<int, int> pair = std::pair<int, int>(x/biomeGridSize, y/biomeGridSize);

	pair.first = x < 0 ? pair.first-1 : pair.first;
	pair.second = y < 0 ? pair.second-1 : pair.second;

	if ( biomeGrid[pair] )
		return biomeGrid[pair];
	else {
		int cX = pair.first * biomeGridSize + biomeGridSize/2 + (Rand::rand()%positionVariance) - positionVariance/2;
		int cY = pair.second * biomeGridSize + biomeGridSize/2 + (Rand::rand()%positionVariance) - positionVariance/2;
		int rad = Rand::rand()%biomeRadiusVariance + minBiomeRadius;
		/* Range for rndType is the number of distinct biomes that can be chosen from. */
		int rndType = Rand::rand()%2;
		
		biomeGrid[pair] = createBiome((CubeManager::CubeType)(rndType), cX, cY, rad);
		return biomeGrid[pair];
	}
}

Biome* BiomeManager::createBiome(CubeManager::CubeType type, int x, int y, int r) {
	return new Biome(mSceneManager, type, x, y, r);
}

bool BiomeManager::isTreeType(CubeManager::CubeType type) {
	return type == CubeManager::WOOD || type == CubeManager::LEAF || type == CubeManager::ICE || type == CubeManager::CACTUS || type == CubeManager::CLOUD;
}