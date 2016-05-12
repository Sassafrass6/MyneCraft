#include "StaticObject.h"
#include "MultiPlatformHelper.h"

btCollisionShape* StaticObject::_shape = new btBoxShape(btVector3(50, 50, 50));

StaticObject::StaticObject(Ogre::Entity* mesh, CubeManager::CubeType ot, Ogre::Vector3 scale, Ogre::Vector3 pos, Simulator* simulator, Chunk* chunk) :
_geom(mesh), _cubeType(ot), _scale(scale), _pos(pos), _simulator(simulator), _mass(1.0f), _friction(0.0f), _restitution(0.0f), _tr(), _chunk(chunk)
{
}

StaticObject::~StaticObject() {
	cleanUpBody();
}

void StaticObject::update() {}

void StaticObject::addToSimulator() {
	// using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	 updateTransform();
	 _collisionObject = new btCollisionObject();
	 _collisionObject->setCollisionShape(_shape);
	 _collisionObject->setWorldTransform(_tr);
	 _collisionObject->setCollisionFlags(_collisionObject->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

	 if(_cubeType == CubeManager::WATER) {
	 	 _collisionObject->setCollisionFlags(_collisionObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	 }

	 _collisionObject->setFriction(_friction);
	 _collisionObject->setRestitution(_restitution);

	_simulator->addObject(this);
}

void StaticObject::updateTransform() {
	_tr.setOrigin(btVector3(_pos.x, _pos.y, _pos.z));
	_tr.setRotation(btQuaternion(_orientation.x, _orientation.y, _orientation.z, _orientation.w));
}

void StaticObject::cleanUpBody() {
	if(_collisionObject != nullptr) {
		delete _collisionObject;
		_collisionObject = nullptr;	
	}
}
