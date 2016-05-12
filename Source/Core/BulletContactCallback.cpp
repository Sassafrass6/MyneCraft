//BulletContactCallback.h - the callback code
#include "BulletContactCallback.h"
#include "GameObject.h"

//! Constructor, pass whatever context you want to have available when processing contacts
/*! You may also want to set m_collisionFilterGroup and m_collisionFilterMask
 *  (supplied by the superclass) for needsCollision() */
BulletContactCallback::BulletContactCallback(btRigidBody& tgtBody , CollisionContext& context, Simulator* sim /*, ... */)
	: btCollisionWorld::ContactResultCallback(), body(tgtBody), ctxt(context), _sim(sim) { }

bool BulletContactCallback::needsCollision(btBroadphaseProxy* proxy) const {
	// superclass will check m_collisionFilterGroup and m_collisionFilterMask
	if(!btCollisionWorld::ContactResultCallback::needsCollision(proxy))
		return false;
	// if passed filters, may also want to avoid contacts between constraints
	return body.checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
}

//! Called with each contact for your own processing
btScalar BulletContactCallback::addSingleResult(btManifoldPoint& cp,
	const btCollisionObject* colObj0, int partId0, int index0,
	const btCollisionObject* colObj1, int partId1, int index1) {

	//std::cout << "True Hit\n";
	ctxt.hit = true;
	ctxt.lastBody = ctxt.body;
	if(colObj0 == &body) {
		ctxt.point = cp.m_localPointA;
		ctxt.body = colObj1;
	} else {
		assert(colObj1 == &body && "body does not match either collision object");
		ctxt.point = cp.m_localPointB;
		ctxt.body = colObj0;
	}
	ctxt.theObject = static_cast<GameObject*>(ctxt.body->getUserPointer());
	ctxt.normal = cp.m_normalWorldOnB;
	ctxt.velocity = body.getLinearVelocity();
	ctxt.velNorm = ctxt.normal.dot(ctxt.velocity);
	_sim->outer->update();

	return 0;
}

#ifdef __linux__
// This is the version of addSingleResult called by the lab machine's version of Ogre
btScalar BulletContactCallback::addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0W, int partId0, int index0, const btCollisionObjectWrapper* colObj1W, int partId1, int index1) {
	const btCollisionObject* colObj0 = colObj0W->getCollisionObject();
	const btCollisionObject* colObj1 = colObj1W->getCollisionObject();

	return addSingleResult(cp, colObj0, partId0, index0, colObj1, partId1, index1);
}
#endif
