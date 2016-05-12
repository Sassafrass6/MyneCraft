#pragma once
#include <OgreSceneManager.h>
#include <string>
#include "Application.h"

namespace SceneHelper {
	inline Ogre::Entity* getEntity(Ogre::SceneManager* mSceneManager, std::string name, int pos) {
		return ((Ogre::Entity*)(mSceneManager->getSceneNode(name)->getAttachedObject(pos)));
	}
};