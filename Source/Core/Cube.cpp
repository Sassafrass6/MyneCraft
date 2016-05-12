#include "Cube.h"

Cube::Cube(Ogre::String nme, GameObject::objectType tp, Ogre::SceneManager* scnMgr, GameManager* ssm, Ogre::SceneNode* node, Ogre::Entity* ent, OgreMotionState* ms, Simulator* sim, Ogre::Real mss, Ogre::Real rest, Ogre::Real frict, Ogre::Vector3 scal, bool kin) : 
GameObject(nme, tp, scnMgr, ssm, node, ent, ms, sim, mss, rest, frict, scal, kin) {
	auto var = ent->getBoundingBox();
	auto size = var.getSize();

	shape = new btBoxShape(btVector3((var.getSize().x*vscale.x)/2, (var.getSize().y*vscale.y)/2, (var.getSize().z*vscale.z)/2));
}

Cube::~Cube() {
	rootNode->detachObject(geom);
	sceneMgr->destroyEntity(geom);
	rootNode->removeAndDestroyAllChildren();
	sceneMgr->destroySceneNode(rootNode);
	std::cout << "Deleted CubeObject" << std::endl;
}

void Cube::update() {
	if(context->hit) {
		
	}
}