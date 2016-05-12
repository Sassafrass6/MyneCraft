#include "Animation.h"

Animation::Animation(){
	_inAction = false;
}

Animation::~Animation(){}

void Animation::addAction(Action* action) {
	_actions.push_back(action);
}

void Animation::playAnimation() {
	if (!_inAction || _currentAction == nullptr)
		return;

	_inAction = _currentAction->playAction();
}

void Animation::createPickaxeAnimation(Ogre::SceneNode* sceneNode) {
	Action* action = new Action(sceneNode);
	static int numtimes = 4;
	static int dOffset = -10;
	static int tOffset = 2;
	for (int i = 0; i <= numtimes; ++i)
		action->addAction(Action::YAW, Ogre::Degree(dOffset * i), Ogre::Vector3(tOffset, 0, 0) * i);
	for (int i = numtimes; i >= 0; --i)
		action->addAction(Action::YAW, Ogre::Degree(dOffset * i), Ogre::Vector3(tOffset, 0, 0) * i);

	addAction(action);
}

void Animation::createSwordAnimation(Ogre::SceneNode* sceneNode) {
	createPickaxeAnimation(sceneNode);
}

void Animation::createTorchAnimation(Ogre::SceneNode* sceneNode) {
	createPickaxeAnimation(sceneNode);
}

void Animation::createBlockAnimation(Ogre::SceneNode* sceneNode) {
	Action* action = new Action(sceneNode);
	static int numtimes = 4;
	static int dOffset = 0;
	static int tOffset = -2;
	for (int i = 0; i <= numtimes; ++i)
		action->addAction(Action::YAW, Ogre::Degree(dOffset * i), Ogre::Vector3(-tOffset, 0, tOffset) * i);
	
	action->addAction(Action::YAW, Ogre::Degree(dOffset * 4), Ogre::Vector3(-tOffset*12, 0, tOffset* 4));
	action->addAction(Action::YAW, Ogre::Degree(dOffset * 4), Ogre::Vector3(-tOffset*13, 0, tOffset* 3));
	action->addAction(Action::YAW, Ogre::Degree(dOffset * 4), Ogre::Vector3(-tOffset*14, 0, tOffset* 2));
	action->addAction(Action::YAW, Ogre::Degree(dOffset * 4), Ogre::Vector3(-tOffset*15, 0, tOffset* 1));

	addAction(action);
}

void Animation::setActionLock(int actionPos) {
	if (_actions.size() <= actionPos)
		return;
	if (_inAction && _currentAction != _actions.at(actionPos))
		return;
	_currentAction = _actions.at(actionPos);
	_inAction = true;
}
