#include "OgreMotionState.h"

OgreMotionState::OgreMotionState(const btTransform &initialpos, Ogre::SceneNode* node) {
	mVisibleobj = node;
	mPos1 = initialpos;
}

OgreMotionState::~OgreMotionState() {
	std::cout << "~OgreMotionState()" << std::endl;
	delete mVisibleobj;
}

	//Provides flexibility in terms of object visibility
void OgreMotionState::setNode(Ogre::SceneNode* node) {
	mVisibleobj = node;
} 

void OgreMotionState::getWorldTransform(btTransform &worldTrans) const {
	worldTrans = mPos1;
}

void OgreMotionState::updateTransform(btTransform& newpos) {
	mPos1 = newpos;
}

// Bridge between Bullet physics position and Ogre scene node position, called once per simulation step
void OgreMotionState::setWorldTransform(const btTransform &worldTrans) {
	if (mVisibleobj == nullptr)
		return; // silently return before we set a node

		btQuaternion rot = worldTrans.getRotation();
		mVisibleobj->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
		btVector3 pos = worldTrans.getOrigin();
		mVisibleobj->setPosition(pos.x(), pos.y(), pos.z());
}