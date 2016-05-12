#pragma once
#include <Ogre.h>

class CubeManager {
private:
	static CubeManager* singleton;
	static Ogre::SceneManager* mSceneManager;
	CubeManager();
	~CubeManager();

	Ogre::Entity* grassCube;
	Ogre::Entity* snowCube;
	Ogre::Entity* iceCube;
	Ogre::Entity* sandCube;
	Ogre::Entity* rockCube;
	Ogre::Entity* dirtCube;
	Ogre::Entity* woodCube;
	Ogre::Entity* leafCube;
	Ogre::Entity* cactusCube;
	Ogre::Entity* torchCube;
	Ogre::Entity* plankCube;
	Ogre::Entity* glassCube;
	Ogre::Entity* cloudCube;
	Ogre::Entity* stoneCube;
	Ogre::Entity* waterCube;
	Ogre::Entity* diamondCube;

public:
	enum CubeType{ SNOW, SAND, ROCK, GRASS, DIRT, TORCH, ICE, WOOD, LEAF, CACTUS, PLANK, GLASS, CLOUD, STONE, DIAMOND, WATER, AIR, NUM_CUBE_TYPES };

	static void injectSceneManager(Ogre::SceneManager*);
	static CubeManager* getSingleton();
	Ogre::Entity* getEntity(CubeType);
	Ogre::Entity* getNewEntity(CubeType);

};