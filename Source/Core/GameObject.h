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

// Simulator & GameObject inclue each other (circular reference).

//Some declarations within the game object class
class GameObject {
public: 
	enum objectType { CUBE_OBJECT, NO_TYPE};

protected:
	Ogre::String name;
	Ogre::SceneManager* sceneMgr;
	Ogre::SceneNode* rootNode;
	Ogre::Entity* geom;
	Ogre::Real scale;
	Ogre::Vector3 vscale;
	Ogre::Vector3 startPos;
	OgreMotionState* motionState;

	btCollisionShape* shape;
	btRigidBody* body;
	btTransform tr;
	btVector3 inertia;

	GameManager* gameManager;
	
	GameObject* previousHit;
	Ogre::Real lastHitTime;

	GameObject::objectType type;
	CollisionContext* context;

	btScalar mass;
	btScalar restitution;
	btScalar friction;
	bool kinematic;
	bool needsUpdates;

	Ogre::ParticleSystem* particle;

public:
	BulletContactCallback* cCallBack;
	bool canJump = false;
	bool isDead = false;
	bool water = false;

	Simulator* simulator;

	GameObject(Ogre::String nme, GameObject::objectType tp, Ogre::SceneManager* scnMgr, GameManager* ssm, Ogre::SceneNode* node, Ogre::Entity* ent, OgreMotionState* ms, Simulator* sim, Ogre::Real mss, Ogre::Real rest, Ogre::Real frict, Ogre::Real scal, bool kin);
	GameObject(Ogre::String nme, GameObject::objectType tp, Ogre::SceneManager* scnMgr, GameManager* ssm, Ogre::SceneNode* node, Ogre::Entity* ent, OgreMotionState* ms, Simulator* sim, Ogre::Real mss, Ogre::Real rest, Ogre::Real frict, Ogre::Vector3 scal, bool kin);
	virtual ~GameObject(){}
	inline btRigidBody* getBody() { return body; }
	void addToSimulator();
	virtual void updateTransform();
	void translate(float x, float y, float z);
	virtual void applyForce(float x, float y, float z);
	void applyImpulse(const btVector3& impulse, const btVector3& rel_pos);
	void setPosition(float x, float y, float z);
	void setPosition(const Ogre::Vector3& pos);
	virtual void setVelocity(float x, float y, float z);
	void setOrientation(Ogre::Quaternion qt);
	void reflect();
	Ogre::SceneNode* getNode();
	
	void reset();
	void reset(Ogre::Vector3 vec);
	void showColliderBox();

	virtual void update() = 0;
	virtual std::string getCoordinates();
	virtual GameObject::objectType getType();
	virtual Ogre::String getName(void);
};
