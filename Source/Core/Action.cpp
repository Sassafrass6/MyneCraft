#include "Action.h"

Action::Action(Ogre::SceneNode* sceneNode) {
	_sceneNode = sceneNode;
	_currentActionIndex = 0;

}

void Action::addAction(ACTION_TYPE type, Ogre::Degree degree, Ogre::Vector3 translation) {
	_actions.push_back(type);
	_degrees.push_back(degree);
	_translations.push_back(translation);
}

/**
*	Does the action and returns if the action is done or not
*/
bool Action::playAction() {
	ACTION_TYPE action = _actions.at(_currentActionIndex);
	Ogre::Degree degree = _degrees.at(_currentActionIndex);
	Ogre::Vector3 translation = _translations.at(_currentActionIndex);

	if (action == PITCH) {
		_sceneNode->pitch(degree);
	}
	else if (action == YAW) {
		_sceneNode->yaw(degree);
	}
	else if (action == ROLL) {
		_sceneNode->roll(degree);
	}
	else {
		assert(!"Block was null");
	}

	_sceneNode->translate(translation, Ogre::Node::TransformSpace::TS_LOCAL);
	
	++_currentActionIndex;

	if (_currentActionIndex == _actions.size()) {
		_currentActionIndex = 0;
		return false;
	}

	return true;
}