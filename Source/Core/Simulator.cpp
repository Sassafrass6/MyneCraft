#include "Simulator.h"
#include "GameObject.h"
#include "StaticObject.h"
#include <exception>

Simulator::Simulator() : objList(), collisionShapes(), objListStatic() { 
  collisionConfiguration = new btDefaultCollisionConfiguration(); 
  dispatcher = new btCollisionDispatcher(collisionConfiguration); 
  overlappingPairCache = new btDbvtBroadphase();
  //overlappingPairCache = new bt32BitAxisSweep3(btVector3(-15000,-15000,-15000), btVector3(15000, 15000, 15000));
  solver = new btSequentialImpulseConstraintSolver(); 
  dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration); 
  dynamicsWorld->setGravity(btVector3(0.0, -10000.0f, 0.0));
}

void Simulator::addObject (GameObject* o) { 
  objList.push_back(o); 
  dynamicsWorld->addRigidBody(o->getBody());     
  player = o->getBody();
}

void Simulator::addObject (StaticObject* o) { 
  objListStatic.push_back(o); 
  invertedObjectHash[o->getBody()] = o;
  dynamicsWorld->addCollisionObject(o->getBody());       
}

//Update the physics world state and any objects that have collision
void Simulator::stepSimulation(const Ogre::Real elapsedTime, int maxSubSteps, const Ogre::Real fixedTimestep) {
	dynamicsWorld->stepSimulation(elapsedTime, maxSubSteps, fixedTimestep);

	for(auto& var : objList) {
		outer = var;
		dynamicsWorld->contactTest(var->getBody(), *(var->cCallBack));
		var->cCallBack->ctxt.hit = false;
	}
}

void Simulator::removeObjects() {
	for (auto& var : objList) {
		dynamicsWorld->removeCollisionObject(var->getBody());
	}
	objList.clear();
}

void Simulator::removeStaticObjects() {

	removeAllColliders();

	for (auto& var : objListStatic) {
		var->cleanUpBody();
	}

	objListStatic.clear();
	invertedObjectHash.clear();
}

bool Simulator::rayHit(const btVector3& start, const btVector3& end, StaticObject*& obj, btVector3& hitNormal) {
	btCollisionWorld::AllHitsRayResultCallback RayCallback(start, end);

	dynamicsWorld->rayTest(start, end, RayCallback);
	btVector3 closest(1000000,100000,1000000);
	bool ret = false;

	if(RayCallback.hasHit()) {
		for(int i = 0 ; i < RayCallback.m_collisionObjects.size(); i++) {
			auto& var = RayCallback.m_collisionObjects[i];
			StaticObject* temp = invertedObjectHash[var];
			if(var != player && temp->_cubeType != CubeManager::WATER) {	
				if(RayCallback.m_hitPointWorld[i].distance(start) < closest.distance(start)) {
					closest = RayCallback.m_hitPointWorld[i];
					obj = temp;
					hitNormal = RayCallback.m_hitNormalWorld[i];
					ret = true;
				}
			}
		}
	}

	return ret;
}

void Simulator::removeAllColliders() {
	auto& arr = dynamicsWorld->getCollisionObjectArray();

	int j = 0;
	for (int i = 0; i < arr.size(); i++) {
		//	Do btDiscreteDynamicsWorld::removeCollisionObject
		auto collisionObject = arr[i];
		btRigidBody* body = btRigidBody::upcast(collisionObject);
		if (body) {
			arr[j++] = arr[i];
		}	
		else {
			btBroadphaseProxy* bp = collisionObject->getBroadphaseHandle();
			if (bp) {
				dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(bp, dispatcher);
				dynamicsWorld->getBroadphase()->destroyProxy(bp, dispatcher);
				collisionObject->setBroadphaseHandle(0);
			}
		}
	}
	arr.resize(j);
}

void Simulator::setGravity(float grav) {
 	dynamicsWorld->setGravity(btVector3(0.0, grav, 0.0));
}
