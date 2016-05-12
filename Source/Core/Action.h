#pragma once

#include <vector>

#include <Ogre.h>

class Action {
public:
	enum ACTION_TYPE{ PITCH, YAW, ROLL };
	Action(Ogre::SceneNode* sceneNode);
	void addAction(ACTION_TYPE type, Ogre::Degree degree, Ogre::Vector3 translation);
	bool playAction();
	
private:
	std::vector<ACTION_TYPE> _actions;
	std::vector<Ogre::Degree> _degrees;
	std::vector<Ogre::Vector3> _translations;
	Ogre::SceneNode* _sceneNode;
	int _currentActionIndex;
};