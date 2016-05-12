#include "CollisionContext.h"

CollisionContext::CollisionContext() {
	reset();
}

void CollisionContext::reset() {
	hit = false;
	body = NULL;
	theObject = NULL;
	distance = 0.0;
	velNorm = 0.0;
	point.setZero();
	normal.setZero();
	velocity.setZero();
}

GameObject* CollisionContext::getTheObject(void){
	return theObject;
}