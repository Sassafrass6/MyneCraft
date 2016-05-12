//How this callback works in the code:
#pragma once

#include "BulletContactCallback.h"
#include "Simulator.h"
#include "CollisionContext.h"
#include "OgreMotionState.h"
#include "OISManager.h"
#include "GameManager.h"

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
#include <OgreParticleSystem.h>
#include "Biome.h"

class Chunk;

/* Static object is as the name implies, cannot be moved or translated but exists without a scenenode for fast allocation */
class StaticObject {
protected:
	Simulator* _simulator;
	static btCollisionShape* _shape;
	btTransform _tr;
	btVector3 _inertia;
	btCollisionObject* _collisionObject = nullptr;

	btScalar _mass;
	btScalar _restitution;
	btScalar _friction;
	btDefaultMotionState* _motionState;
	bool _kinematic;
	bool _needsUpdates;

	CubeManager::CubeType _objectType;

public:
	Ogre::Entity* _geom;
	Ogre::Vector3 _scale;
	Ogre::Vector3 _pos;
	Ogre::Quaternion _orientation{};
	CubeManager::CubeType _cubeType;
	Chunk* _chunk;
	StaticObject(Ogre::Entity* mesh, CubeManager::CubeType, Ogre::Vector3 scale, Ogre::Vector3 pos, Simulator* simulator, Chunk* chunk);
	~StaticObject();
	btCollisionObject* getBody() { return _collisionObject; }
	void addToSimulator();
	virtual void updateTransform();
	void setVelocity(float x, float y, float z);
	void setOrientation(Ogre::Quaternion qt);
	void cleanUpBody();

	virtual void update();
};
