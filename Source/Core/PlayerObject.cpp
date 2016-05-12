#include "PlayerObject.h"

PlayerObject::PlayerObject(Ogre::String nme, GameObject::objectType tp, Ogre::SceneManager* scnMgr, GameManager* ssm, Ogre::SceneNode* node, Ogre::Entity* ent, OgreMotionState* ms, Simulator* sim, Ogre::Real mss, Ogre::Real rest, Ogre::Real frict, Ogre::Vector3 scal, bool kin) :
GameObject(nme, tp, scnMgr, ssm, node, ent, ms, sim, mss, rest, frict, scal, kin) {
	auto var = ent->getBoundingBox();
	auto size = var.getSize();

	Ogre::Vector3 x(1,0,0);
	Ogre::Vector3 y(0,1,0);
	Ogre::Vector3 z(0,0,1);
	Ogre::Quaternion qt(x,y,z);

	tr.setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));

	shape = new btCapsuleShape(65, 255);
}

PlayerObject::~PlayerObject() {
	rootNode->detachObject(geom);
	sceneMgr->destroyEntity(geom);
	rootNode->removeAndDestroyAllChildren();
	sceneMgr->destroySceneNode(rootNode);
	std::cout << "Deleted playerObjecT" << std::endl;
}

void PlayerObject::update() {
	if(context->hit) {
		int x = round(context->normal.x());
		int y = round(context->normal.y());
		int z = round(context->normal.z());

		if(x == 0 && y == 1 && z == 0)
			canJump = true;

		if(context->body) {
			/* This is sort of a hack, we could look p the collisionobject in the simulator inverted hash but this is faster */
			if(!context->body->hasContactResponse()) {
				water = true;
			}
			else water = false;
		}

		// if(water) std::cout << "Water" << std::endl;
		// else std::cout << "Ground" << std::endl;
	}
}

void PlayerObject::applyForce(float x, float y, float z) {
	body->applyCentralForce(btVector3(x, y, z));
	updateTransform();
}

void PlayerObject::updateTransform() {
	Ogre::Vector3 pos = rootNode->getPosition();
	tr.setOrigin(btVector3(pos.x, pos.y, pos.z));

	motionState->updateTransform(tr);
}

void PlayerObject::setVelocity(float x, float y, float z) {
	body->setLinearVelocity(btVector3(x, y, z));
	updateTransform();
}