#include "Biome.h"

Biome::Biome(Ogre::SceneManager* smgr, CubeManager::CubeType type, int x, int y, int r) : mSceneManager(smgr), biomeType(type), centerX(x), centerY(y), radius(r) {

}


bool Biome::inBiome(int x, int y) {
	return (abs((x < 0 ? -x : x) - (centerX < 0 ? -centerX : centerX)) < radius || abs((y < 0 ? -y : y) - (centerY < 0 ? -centerY : centerY)) < radius);
}

bool Biome::inRadius(float x, float y) {
	Ogre::Vector2 center(centerX, centerY);
	Ogre::Vector2 pos(x, y);

	return pos.distance(center) < radius;
}

float Biome::getSteepnessDivider(float x, float y) {
	Ogre::Vector2 center(centerX, centerY);
	Ogre::Vector2 pos(x, y);

	float d = (radius - pos.distance(center)) / 200.0f;

	if (d < 0) return 1;

	return pow(d, 1.5);
}

Ogre::Entity* Biome::getCubeEntity(int x, int y, int h, CubeManager::CubeType& type) {
	int dSq = ((centerX - x) * (centerX - x)) + ((centerY - y) * (centerY - y));
	float rSQoN = radius * radius / 9;
	int rnd = Rand::rand()%20;
	if ( (dSq < 2 * rSQoN)
			|| (dSq < 4 * rSQoN && rnd < 18)
			|| (dSq < 6 * rSQoN && rnd < 14)
			|| (dSq < 7 * rSQoN && rnd < 9)
			|| (dSq < 8 * rSQoN && rnd < 4)
			|| (dSq < 9 * rSQoN && rnd < 1) ) {
		type = biomeType;
		return CubeManager::getSingleton()->getEntity(biomeType);
	}
	else if ( h >= 36 && dSq > 6 * rSQoN ) {
		type = CubeManager::CubeType::SNOW;
		return CubeManager::getSingleton()->getEntity(CubeManager::SNOW);
	}
	else {
		type = CubeManager::CubeType::GRASS;
		return CubeManager::getSingleton()->getEntity(CubeManager::GRASS);
	}
}
