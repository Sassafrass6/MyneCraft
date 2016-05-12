#pragma once
#include <Ogre.h>

#include <vector>
#include <utility>
#include <unordered_map>

#include "StaticObject.h"
#include "CubeManager.h"
#include "BiomeManager.h"
#include "Perlin.h"
#include "BlockInfo.h"
#include "Rand.h"
#include "HashCombine.h"

#define CHUNK_SIZE 16
#define CHUNK_SCALE 50
#define CHUNK_SCALE_FULL 100
#define WATER_LEVEL -10

static StaticObject* air = nullptr;

class Chunk {
private:
	typedef std::tuple<int, int, int> key;

	Ogre::StaticGeometry* _sg;
	std::string _name;
	Simulator* _simulator;
	Ogre::SceneManager* _mSceneManager;
	Biome* _biome;
	BiomeManager* _biomeMgr;

	int heights[CHUNK_SIZE + 2][CHUNK_SIZE + 2];
	int topHeights[CHUNK_SIZE + 2][CHUNK_SIZE + 2];
	int bottomHeights[CHUNK_SIZE + 2][CHUNK_SIZE + 2];
	int caveHeights[CHUNK_SIZE + 2][CHUNK_SIZE + 2];
	int caves[CHUNK_SIZE + 2][CHUNK_SIZE + 2];
	int snowLevel = 15;
	int waterLevel = WATER_LEVEL;

	std::unordered_map<key, Ogre::Light*> lights;

	bool createTerrainColumn(int, int, Ogre::Vector3&);
	void buildWaterBlock(int, Ogre::Vector3&);
	CubeManager::CubeType getCaveCubeType();
	void buildCaveBlock(int, int, key, Ogre::Vector3&, int, CubeManager::CubeType, int[CHUNK_SIZE+2][CHUNK_SIZE+2]); 
	bool createTree(const Ogre::Vector3&, CubeManager::CubeType);
	bool createCloud(const Ogre::Vector3&);
	key getKey(int x, int y, int z);
	key getKey(const Ogre::Vector3&);

	StaticObject* getObjFromChunks(const std::vector<Chunk*>&, key);
	void createAirNeighbors(const std::vector<Chunk*>&, StaticObject*);
	void interpolateBlock(int, int, int[CHUNK_SIZE+2][CHUNK_SIZE+2], Ogre::Vector3&);
	int computeMinNeighbor(int, int, int[CHUNK_SIZE+2][CHUNK_SIZE+2]);
	int computeMaxNeighbor(int, int, int[CHUNK_SIZE+2][CHUNK_SIZE+2]);

public:
	Chunk(int, int, Ogre::SceneManager*, BiomeManager*, Perlin*, Simulator*, bool);
	~Chunk();
	bool pointInChunk(float x, float y);
	void addChunksToSimulator(Ogre::Vector3& playerPos);
	void addBlockToStaticGeometry(CubeManager::CubeType, Ogre::Vector3, key);
	void addStaticObjectToGeometry(StaticObject*);
	StaticObject* getBlock(int x, int y, int z);
	CubeManager::CubeType getGeneratedType(CubeManager::CubeType objType, int height);
	void removeBlock(const std::vector<Chunk*>& chunks, StaticObject* obj);
	void addBlock(const std::vector<Chunk*>& chunks, StaticObject* obj, const btVector3& hitnormal, CubeManager::CubeType newType);
	std::string getName() { return _name; }

	std::unordered_map<key, StaticObject*> _staticObjects;
	BlockInfo getBlockInfo(key, CubeManager::CubeType type);
	void rebuildFromSave(const std::vector<BlockInfo>& blocks);
	std::string getKeyName(key thekey);
	void build();

	Ogre::Vector3 _scale;
	int _xStart = 0;
	int _xEnd = 0;
	int _yStart = 0;
	int _yEnd = 0;
	bool modified = false;
};

inline static std::string getChunkName(int x, int z) {
	char buf[64];
	sprintf(buf, "Chunk_%d_%d", x / CHUNK_SIZE, z / CHUNK_SIZE);
	return std::string(buf);
}