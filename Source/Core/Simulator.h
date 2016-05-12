#pragma once

#include <btBulletDynamicsCommon.h>
#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreWindowEventUtilities.h>
#include <OgreRenderTargetListener.h>
#include <deque>
#include <unordered_map>

#include "CubeManager.h"

class GameObject;
class StaticObject;

class Simulator { 
protected: 
       btDefaultCollisionConfiguration* collisionConfiguration; 
       btCollisionDispatcher* dispatcher; 
       btBroadphaseInterface* overlappingPairCache; 
       btSequentialImpulseConstraintSolver* solver;
       btDiscreteDynamicsWorld* dynamicsWorld;
       btAlignedObjectArray<btCollisionShape*> collisionShapes;
       std::deque<GameObject*> objList; 
       std::deque<StaticObject*> objListStatic;
	std::unordered_map<const btCollisionObject*, StaticObject*> invertedObjectHash;
       btCollisionObject* player;
	   void removeAllColliders();
public: 
       Simulator(); 
       ~Simulator(); 

       void addObject(GameObject* o); 
       void addObject(StaticObject* o); 
       bool removeObject(GameObject* o); 
       void stepSimulation(const Ogre::Real elapsedTime, int maxSubSteps = 1, const Ogre::Real fixedTimestep = 1.0f/60.0f);
	   void removeObjects(); 
       void removeStaticObjects();
       bool rayHit(const btVector3& start, const btVector3& end, StaticObject*& obj, btVector3& hitNormal);
       void setGravity(float grav);
       GameObject* outer;
};
