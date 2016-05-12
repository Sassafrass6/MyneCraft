#pragma once

#include <btBulletDynamicsCommon.h>

// Forward declaration for GameObject pointer. We cannot include GameObject.h because there is ciruclar dependencies.
class GameObject;

//A struct to keep track of contact information.  Can vary depending on what needs to be tracked
struct CollisionContext {
	bool hit;
	const btCollisionObject* body;
	const btCollisionObject* lastBody;
	GameObject* theObject;
	float distance;
	float velNorm;
	btVector3 point;
	btVector3 normal;
	btVector3 velocity;

	CollisionContext();

	void reset();

public:
	GameObject* getTheObject(void);
};