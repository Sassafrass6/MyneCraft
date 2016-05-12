#include "btBulletDynamicsCommon.h"
#include <OgreEntity.h>
#include <OgreRoot.h>

class GameState {
public:
	Ogre::Quaternion _thePaddle;
	Ogre::Quaternion _otherPaddle;
	Ogre::Vector3 _thePaddlePos;
	Ogre::Vector3 _otherPaddlePos;
	Ogre::Vector3 _ballPos;
	btVector3 _velocity;
	int _gameScore;
	int _opponentScore;

	GameState(const Ogre::Quaternion& qt1, const Ogre::Quaternion& qt2, const Ogre::Vector3& pPos, const Ogre::Vector3& oPos, const Ogre::Vector3& pos, const btVector3& vel, int gScore, int oScore) : _thePaddle(qt1), _otherPaddle(qt2), _thePaddlePos(pPos), _otherPaddlePos(oPos), _ballPos(pos), _velocity(vel), _gameScore(gScore), _opponentScore(oScore) {}
};