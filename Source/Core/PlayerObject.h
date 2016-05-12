#pragma once
#include "GameObject.h"

class PlayerObject : public GameObject {
public:
	PlayerObject(Ogre::String nme, GameObject::objectType tp, Ogre::SceneManager* scnMgr, GameManager* ssm, Ogre::SceneNode* node, Ogre::Entity* ent, OgreMotionState* ms, Simulator* sim, Ogre::Real mss, Ogre::Real rest, Ogre::Real frict, Ogre::Vector3 scal, bool kin);
	~PlayerObject();

	void update();
	virtual void updateTransform();
	virtual void applyForce(float x, float y, float z);
	virtual void setVelocity(float x, float y, float z);
private:

};