#pragma once

#include <vector>

#include "Action.h"

class Animation {
public:
	Animation();
	~Animation();
	void addAction(Action* action);
	void setActionLock(int actionPos);
	void createPickaxeAnimation(Ogre::SceneNode* sceneNode);
	void createSwordAnimation(Ogre::SceneNode* sceneNode);
	void createTorchAnimation(Ogre::SceneNode* sceneNode);
	void createBlockAnimation(Ogre::SceneNode* sceneNode);
	void playAnimation();
	bool _inAction;
	Action* _currentAction = nullptr;

protected:
	std::vector<Action*> _actions;
};