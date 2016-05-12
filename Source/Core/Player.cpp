 #include "Player.h"

Player::Player(Ogre::Camera* camera, GameObject* body, Ogre::SceneManager* sm, GameManager* gm) : _body(body), _playerCam(camera), _sceneManager(sm), _gm(gm), diamondCount(0) {
	 _body->getNode()->setVisible(false);
	_body->getBody()->setAngularFactor(btVector3(0, 0, 0));


	for(int i = 0; i < averageSize; i++) {
		camAvg[i] = Ogre::Vector3::ZERO;
	}
	Ogre::Entity* item;
	Ogre::SceneNode* node;
	Ogre::SceneNode* rotNode;

	Ogre::String nme = body->getName();


	item = _sceneManager->createEntity("Pickaxe"+nme, "Mynecraft-Pickaxe.mesh");
	item->setCastShadows(true);
	node = _sceneManager->getRootSceneNode()->createChildSceneNode("Pickaxe"+nme);
	node->attachObject(item);
	node->setDirection(Ogre::Vector3(0, 1 ,0));
	node->setScale(3, 3, 3);
	node->setVisible(false);

	_animation.createPickaxeAnimation(node);

	inventory.push_back(node);
	inventoryEntities.push_back(item);

	item = _sceneManager->createEntity("Sword"+nme, "Mynecraft-Sword.mesh");
	item->setCastShadows(true);
	node = _sceneManager->getRootSceneNode()->createChildSceneNode("Sword"+nme);
	rotNode = node->createChildSceneNode("SwordNode"+nme);
	rotNode->attachObject(item);
	rotNode->setPosition(Ogre::Vector3(-2, 6, 10));
	rotNode->roll(Ogre::Degree(90));
	rotNode->pitch(Ogre::Degree(-90));
	node->setScale(1, 1, 1);
	node->setVisible(false);

	_animation.createSwordAnimation(node);
	inventory.push_back(node);
	inventoryEntities.push_back(item);
	rotNodes.push_back(rotNode);

	item = CubeManager::getSingleton()->getNewEntity(CubeManager::TORCH);
	item->setCastShadows(true);
	node = _sceneManager->getRootSceneNode()->createChildSceneNode("Light"+nme);
	rotNode = node->createChildSceneNode("LightNode"+nme);
	rotNode->attachObject(item);
	rotNode->setPosition(Ogre::Vector3(0, 0, 3));
	rotNode->roll(Ogre::Degree(-60));
	rotNode->setDirection(Ogre::Vector3(0, -1 , 0));
	node->setScale(2, 10, 2);

	Ogre::Light* light = _sceneManager->createLight("torch"+nme);
	light->setDiffuseColour(1, .6, .05);
	light->setType(Ogre::Light::LT_POINT);
	float range = 5000.0f;
	light->setAttenuation(range, 1.0f, .5f/range, 5.0f/(range*range));
	node->attachObject(light);
	node->setVisible(false);

	_animation.createSwordAnimation(node);

	inventory.push_back(node);
	inventoryEntities.push_back(item);
	inventoryEntities.push_back(light);
	rotNodes.push_back(rotNode);

	item = CubeManager::getSingleton()->getNewEntity(CubeManager::DIAMOND);
	item->setCastShadows(true);
	node = _sceneManager->getRootSceneNode()->createChildSceneNode("DiamondCube"+nme);
	rotNode = node->createChildSceneNode("DiamondCubeNode"+nme);
	rotNode->attachObject(item);
	rotNode->setPosition(Ogre::Vector3(0, 0, 2));
	rotNode->roll(Ogre::Degree(90));
	rotNode->pitch(Ogre::Degree(-90));
	node->setScale(9, 9, 9);
	node->setVisible(false);

	_animation.createBlockAnimation(node);

	inventory.push_back(node);
	inventoryEntities.push_back(item);
	rotNodes.push_back(rotNode);

	item = CubeManager::getSingleton()->getNewEntity(CubeManager::GRASS);
	item->setCastShadows(true);
	node = _sceneManager->getRootSceneNode()->createChildSceneNode("GrassCube"+nme);
	rotNode = node->createChildSceneNode("GrassCubeNode"+nme);
	rotNode->attachObject(item);
	rotNode->setPosition(Ogre::Vector3(0, 0, 2));
	rotNode->roll(Ogre::Degree(90));
	rotNode->pitch(Ogre::Degree(-90));
	node->setScale(9, 9, 9);
	node->setVisible(false);

	_animation.createBlockAnimation(node);

	inventory.push_back(node);
	inventoryEntities.push_back(item);
	rotNodes.push_back(rotNode);

	item = CubeManager::getSingleton()->getNewEntity(CubeManager::ROCK);
	item->setCastShadows(true);
	node = _sceneManager->getRootSceneNode()->createChildSceneNode("RockCube"+nme);
	rotNode = node->createChildSceneNode("RockCubeNode"+nme);
	rotNode->attachObject(item);
	rotNode->setPosition(Ogre::Vector3(0, 0, 2));
	rotNode->roll(Ogre::Degree(90));
	rotNode->pitch(Ogre::Degree(-90));
	node->setScale(9, 9, 9);
	node->setVisible(false);

	_animation.createBlockAnimation(node);

	inventory.push_back(node);
	inventoryEntities.push_back(item);
	rotNodes.push_back(rotNode);

	item = CubeManager::getSingleton()->getNewEntity(CubeManager::SNOW);
	item->setCastShadows(true);
	node = _sceneManager->getRootSceneNode()->createChildSceneNode("SnowCube"+nme);
	rotNode = node->createChildSceneNode("SnowCubeNode"+nme);
	rotNode->attachObject(item);
	rotNode->setPosition(Ogre::Vector3(0, 0, 2));
	rotNode->roll(Ogre::Degree(90));
	rotNode->pitch(Ogre::Degree(-90));
	node->setScale(9, 9, 9);
	node->setVisible(false);

	_animation.createBlockAnimation(node);

	inventory.push_back(node);
	inventoryEntities.push_back(item);
	rotNodes.push_back(rotNode);

	item = CubeManager::getSingleton()->getNewEntity(CubeManager::SAND);
	item->setCastShadows(true);
	node = _sceneManager->getRootSceneNode()->createChildSceneNode("SandCube"+nme);
	rotNode = node->createChildSceneNode("SandCubeNode"+nme);
	rotNode->attachObject(item);
	rotNode->setPosition(Ogre::Vector3(0, 0, 2));
	rotNode->roll(Ogre::Degree(90));
	rotNode->pitch(Ogre::Degree(-90));
	node->setScale(9, 9, 9);
	node->setVisible(false);

	_animation.createBlockAnimation(node);

	inventory.push_back(node);
	inventoryEntities.push_back(item);
	rotNodes.push_back(rotNode);

	item = CubeManager::getSingleton()->getNewEntity(CubeManager::DIRT);
	item->setCastShadows(true);
	node = _sceneManager->getRootSceneNode()->createChildSceneNode("DirtCube"+nme);
	rotNode = node->createChildSceneNode("DirtCubeNode"+nme);
	rotNode->attachObject(item);
	rotNode->setPosition(Ogre::Vector3(0, 0, 2));
	rotNode->roll(Ogre::Degree(90));
	rotNode->pitch(Ogre::Degree(-90));
	node->setScale(9, 9, 9);
	node->setVisible(false);

	_animation.createBlockAnimation(node);

	inventory.push_back(node);
	inventoryEntities.push_back(item);
	rotNodes.push_back(rotNode);

	item = CubeManager::getSingleton()->getNewEntity(CubeManager::PLANK);
	item->setCastShadows(true);
	node = _sceneManager->getRootSceneNode()->createChildSceneNode("PlankCube"+nme);
	rotNode = node->createChildSceneNode("PlankCubeNode"+nme);
	rotNode->attachObject(item);
	rotNode->setPosition(Ogre::Vector3(0, 0, 2));
	rotNode->roll(Ogre::Degree(90));
	rotNode->pitch(Ogre::Degree(-90));
	node->setScale(9, 9, 9);
	node->setVisible(false);

	_animation.createBlockAnimation(node);

	inventory.push_back(node);
	inventoryEntities.push_back(item);
	rotNodes.push_back(rotNode);

	item = CubeManager::getSingleton()->getNewEntity(CubeManager::GLASS);
	item->setCastShadows(true);
	node = _sceneManager->getRootSceneNode()->createChildSceneNode("GlassCube"+nme);
	rotNode = node->createChildSceneNode("GlassCubeNode"+nme);
	rotNode->attachObject(item);
	rotNode->setPosition(Ogre::Vector3(0, 0, 2));
	rotNode->roll(Ogre::Degree(90));
	rotNode->pitch(Ogre::Degree(-90));
	node->setScale(9, 9, 9);
	node->setVisible(false);

	_animation.createBlockAnimation(node);

	inventory.push_back(node);
	inventoryEntities.push_back(item);
	rotNodes.push_back(rotNode);


	equippedItem = -1;

	std::cout << "Finished creating player" << std::endl;
}

Player::~Player() {
	for (int i = 0; i < rotNodes.size(); ++i) {
		rotNodes.at(i)->detachAllObjects();
	}
	for (int i = 0; i < inventory.size(); ++i) {
		inventory.at(i)->detachAllObjects();
	}
	for (int i = 0; i < inventoryEntities.size(); ++i) {
		_sceneManager->destroyMovableObject(inventoryEntities.at(i));
	}
	for (int i = 0; i < rotNodes.size(); ++i) {
		_sceneManager->destroySceneNode(rotNodes.at(i));
	}
	for (int i = 0; i < inventory.size(); ++i) {
		_sceneManager->destroySceneNode(inventory.at(i));
	}
	inventory.clear();
	inventoryEntities.clear();
	rotNodes.clear();
	delete _body;
	std::cout << "Deleted Player" << std::endl;
}

void Player::setWeapon ( int i ) {
	if (_animation._inAction )
	 if ( equippedItem != DIAMOND_CUBE || diamondCount > 0 )
		return;
	if ( equippedItem >= 0 && equippedItem < inventory.size()) {
		inventory[equippedItem]->setVisible(false);
	}
	if ( i < 0 || i >= inventory.size() )
		equippedItem = -1;
	else if ( diamondCount == 0 && i == DIAMOND_CUBE )
		equippedItem = -1;
	else {
		equippedItem = i;
		inventory[equippedItem]->setVisible(true);
	}
}

int Player::getWeapon() {
	return equippedItem;
}

void Player::nextWeapon(int delta) {
	if (_animation._inAction)
		return;

	int index = equippedItem + delta;
	index = index % (Player::NUM_WEP+1);
	if (index < 0)
		index += Player::NUM_WEP+1;

	if ( equippedItem >= 0 && equippedItem < inventory.size())
		inventory[equippedItem]->setVisible(false);

	if ( diamondCount == 0 && index == DIAMOND_CUBE )
		equippedItem = index + delta;
	else
		equippedItem = index;

	if ( equippedItem >= inventory.size() || equippedItem < 0 )
		equippedItem = -1;
	else
		inventory[equippedItem]->setVisible(true);
}

Ogre::SceneNode* Player::getWeaponNode() {
	if ( equippedItem != -1 )
		return inventory[equippedItem];
	else
		return nullptr;
}

void Player::update(OISManager* ois) {
	static int speed = 1000;
	static int camAvgIndex = 0;

	Ogre::Vector3 movePos = _playerCam->getDirection();
	movePos = Ogre::Vector3(movePos.x, 0, movePos.z);
	movePos.normalise();
	Ogre::Vector3 strafePos = movePos.crossProduct(Ogre::Vector3(0, 1, 0));
	float currentY = _body->getBody()->getLinearVelocity().y();

	OIS::KeyCode lastkey;
	bool moved = false;

	float buoyant = 10000.0f;
	
	if(_body->water) {
		float drag = _body->getBody()->getLinearVelocity().length();

		float dragVec = -(_body->getBody()->getLinearVelocity().y());

		dragVec *= drag/200.0f;

		if ( dragVec > 10000.0 )
			dragVec = 10000.0;
		_body->applyForce(0, dragVec + buoyant, 0);

		currentY = _body->getBody()->getLinearVelocity().y();
	}

	if(_body->getNode()->getPosition().y/CHUNK_SCALE_FULL >= WATER_LEVEL-1) {
		_body->water = false;
	}

	if (ois->isKeyDown(OIS::KC_LSHIFT)) {
		speed = 2500;
	}
	else {
		speed = 1000;
	}

	if (ois->isKeyDown(OIS::KC_W)) {
		_body->setVelocity(movePos.x*speed, currentY, movePos.z*speed);
		moved = true;
	}
	if (ois->isKeyDown(OIS::KC_S)) {
		_body->setVelocity(movePos.x*-speed, currentY, movePos.z*-speed);
		moved = true;
	}
	if (ois->isKeyDown(OIS::KC_D)) {
		_body->setVelocity(strafePos.x*speed, currentY, strafePos.z*speed);
		moved = true;
	}
	if (ois->isKeyDown(OIS::KC_A)) {
		_body->setVelocity(strafePos.x*-speed, currentY, strafePos.z*-speed);
		moved = true;
	}
	if (ois->isKeyDown(OIS::KC_SPACE) && _body->water) {
		_body->applyForce(0, buoyant, 0);
		currentY = _body->getBody()->getLinearVelocity().y();
		moved = true;
		_body->canJump = true;
	}
	else if (ois->isKeyDown(OIS::KC_SPACE) && _body->canJump) {
		int power = 2500;
		_body->setVelocity(_body->getBody()->getLinearVelocity().x(), power, _body->getBody()->getLinearVelocity().z());
		currentY = _body->getBody()->getLinearVelocity().y();
		moved = true;
		_body->canJump = false;
	}

	if (!moved) {
		_body->setVelocity(0, currentY, 0);
	}

	camAvg[camAvgIndex % averageSize] = _body->getNode()->getPosition() + Ogre::Vector3(0, 170, 0);
	camAvgIndex++;

	Ogre::Vector3 total(0,0,0);
	int n = 0;

	for(int i = 0; i < averageSize; i++) {
		if(camAvg[i] != Ogre::Vector3::ZERO) {
			total += camAvg[i];
			n++;
		}
	}

	if(n > 0)
		total /= n;

	_playerCam->setPosition(total);


	/* Is the player out of diamonds? */
	if ( equippedItem == DIAMOND_CUBE && diamondCount == 0 )
		setWeapon(GRASS_CUBE);

	Ogre::SceneNode* node = getWeaponNode();
	if ( node != nullptr ) {
		Ogre::Vector3 unit = _playerCam->getDirection().normalisedCopy();
		Ogre::Vector3 u = unit.crossProduct(Ogre::Vector3::UNIT_Y).normalisedCopy();
		Ogre::Vector3 v = unit.crossProduct(u).normalisedCopy();

		Ogre::Quaternion qt(unit, u, v);

		node->setOrientation(qt);

		Ogre::Vector3 offset = unit.crossProduct(qt.zAxis()).normalisedCopy();
		node->setPosition(total + unit*50 - offset*20);
		node->roll(Ogre::Degree(-10));
	}

	_animation.playAnimation();
}

bool Player::clickAction(StaticObject* hitObj, const btVector3& hitnormal, std::unordered_map<std::pair<int, int>, Chunk*>& chunks, std::unordered_map<std::pair<int, int>, Chunk*>& modifiedChunks) {
	if (equippedItem == PICKAXE) {
		pickaxeAction(hitObj, chunks, modifiedChunks);
		_animation.setActionLock(PICKAXE);
		playHitSound(hitObj);
		return true;
	}
	if (equippedItem == SWORD) {
		_animation.setActionLock(SWORD);
		return true;
	}
	if (equippedItem == TORCH_CUBE) {
		cubePlaceAction(hitObj, hitnormal, chunks, modifiedChunks, CubeManager::TORCH);
		_animation.setActionLock(TORCH_CUBE);
		_gm->playSound(GameManager::POP);
		return true;
	}
	if (equippedItem == DIAMOND_CUBE) {
		cubePlaceAction(hitObj, hitnormal, chunks, modifiedChunks, CubeManager::DIAMOND);
		_animation.setActionLock(DIAMOND_CUBE);
		_gm->playSound(GameManager::POP);
		diamondCount--;
		setWeapon(PICKAXE);
		return true;
	}
	if (equippedItem == GRASS_CUBE) {
		cubePlaceAction(hitObj, hitnormal, chunks, modifiedChunks, CubeManager::GRASS);
		_animation.setActionLock(GRASS_CUBE);
		_gm->playSound(GameManager::POP);
		return true;
	}
	if (equippedItem == ROCK_CUBE) {
		cubePlaceAction(hitObj, hitnormal, chunks, modifiedChunks, CubeManager::ROCK);
		_animation.setActionLock(ROCK_CUBE);
		_gm->playSound(GameManager::POP);
		return true;
	}
	if (equippedItem == SNOW_CUBE) {
		cubePlaceAction(hitObj, hitnormal, chunks, modifiedChunks, CubeManager::SNOW);
		_animation.setActionLock(SNOW_CUBE);
		_gm->playSound(GameManager::POP);
		return true;
	}
	if (equippedItem == SAND_CUBE) {
		cubePlaceAction(hitObj, hitnormal, chunks, modifiedChunks, CubeManager::SAND);
		_animation.setActionLock(SAND_CUBE);
		_gm->playSound(GameManager::POP);
		return true;
	}
	if (equippedItem == DIRT_CUBE) {
		cubePlaceAction(hitObj, hitnormal, chunks, modifiedChunks, CubeManager::DIRT);
		_animation.setActionLock(DIRT_CUBE);
		_gm->playSound(GameManager::POP);
		return true;
	}
	if (equippedItem == PLANK_CUBE) {
		cubePlaceAction(hitObj, hitnormal, chunks, modifiedChunks, CubeManager::PLANK);
		_animation.setActionLock(PLANK_CUBE);
		_gm->playSound(GameManager::POP);
		return true;
	}
	if (equippedItem == GLASS_CUBE) {
		cubePlaceAction(hitObj, hitnormal, chunks, modifiedChunks, CubeManager::GLASS);
		_animation.setActionLock(GLASS_CUBE);
		_gm->playSound(GameManager::POP);
		return true;
	}
	return false;
}

void Player::pickaxeAction(StaticObject* hitObj, std::unordered_map<std::pair<int, int>, Chunk*>& chunks, std::unordered_map<std::pair<int, int>, Chunk*>& modifiedChunks) {
	Chunk* chunk = hitObj->_chunk;

	if (chunk != nullptr) {
		int x = chunk->_xStart;
		int z = chunk->_yStart;

		if (hitObj != nullptr) {
			// Check neighboring chunks because this objects neighbors may be in seperate chunks
			std::vector<Chunk*> chunklist;
			getNeighborChunks(chunklist, x, z, chunks, chunk);

			chunk->removeBlock(chunklist, hitObj);
			chunk->modified = true;

			std::pair<int, int> name(x, z);
			modifiedChunks[name] = chunk;

			if ( hitObj->_cubeType == CubeManager::DIAMOND )
				diamondCount++;

		}
	}
}

void Player::cubePlaceAction(StaticObject* hitObj, const btVector3& hitnormal, std::unordered_map<std::pair<int, int>, Chunk*>& chunks, std::unordered_map<std::pair<int, int>, Chunk*>& modifiedChunks, CubeManager::CubeType type) {
	Chunk* chunk = hitObj->_chunk;
	if ( (hitObj->_pos - _body->getNode()->getPosition()).length() < 2.8*CHUNK_SCALE_FULL )
		return;
	if (chunk != nullptr) {
		int x = chunk->_xStart;
		int z = chunk->_yStart;

		if (hitObj != nullptr) {
			// Check neighboring chunks because this objects neighbors may be in seperate chunks
			std::vector<Chunk*> chunklist;
			getNeighborChunks(chunklist, x, z, chunks, chunk);

			chunk->addBlock(chunklist, hitObj, hitnormal, type);
			chunk->modified = true;

			std::pair<int, int> name(x, z);
			modifiedChunks[name] = chunk;
		}
	}
}

int Player::getDiamondCount() {
	return diamondCount;
}

void Player::setDiamondCount( int cnt ) {
	diamondCount = cnt;
}

void Player::getNeighborChunks(std::vector<Chunk*>& chunklist, int x, int z, std::unordered_map<std::pair<int, int>, Chunk*>& chunks, Chunk* chunk) {
	std::pair<int, int> left(x - CHUNK_SIZE, z);
	Chunk* leftChunk = chunks[left];

	std::pair<int, int> right(x + CHUNK_SIZE, z);
	Chunk* rightChunk = chunks[right];

	std::pair<int, int> bottom(x, z - CHUNK_SIZE);
	Chunk* bottomChunk = chunks[bottom];

	std::pair<int, int> top(x, z + CHUNK_SIZE);
	Chunk* topChunk = chunks[top];

	chunklist.push_back(leftChunk);
	chunklist.push_back(rightChunk);
	chunklist.push_back(bottomChunk);
	chunklist.push_back(topChunk);
	chunklist.push_back(chunk);
}

void Player::constrainSpeed() {
	static int maxSpeed = 10000;

	// Limit player speed
	btVector3 velocity = _body->getBody()->getLinearVelocity();
    btScalar lspeed = velocity.length();
    if(lspeed > maxSpeed) {
        velocity *= maxSpeed/lspeed;
        _body->setVelocity(velocity.x(), velocity.y(), velocity.z());
    }
}

std::string Player::getCoordinates() {
	Ogre::Quaternion orient(_body->getNode()->getOrientation());
	std::string ow = std::to_string(orient.w);
	std::string ox = std::to_string(orient.x);
	std::string oy = std::to_string(orient.y);
	std::string oz = std::to_string(orient.z);

	Ogre::Vector3 pos(_body->getNode()->getPosition());
	std::string px = std::to_string(pos.x);
	std::string py = std::to_string(pos.y);
	std::string pz = std::to_string(pos.z);

	std::string str = "PDW " + ow + "\nPDX " + ox + "\nPDY " + oy + "\nPDZ " + oz + "\nPPX " + px + "\nPPY " + py + "\nPPZ " + pz;
	
	return str;
}

void Player::playHitSound(StaticObject* hitobj) {
	CubeManager::CubeType type = hitobj->_cubeType;

	if (type == CubeManager::GLASS) {
		_gm->playSound(GameManager::GLASS_BREAK);
	}
	else if (type == CubeManager::GRASS) {
		_gm->playSound(GameManager::GRASS_BREAK);
	}
	else if (type == CubeManager::SAND) {
		_gm->playSound(GameManager::SAND_BREAK);
	}
	else if (type == CubeManager::SNOW || type == CubeManager::DIRT) {
		_gm->playSound(GameManager::SNOW_BREAK);
	}
	else if (type == CubeManager::STONE) {
		_gm->playSound(GameManager::STONE_BREAK);
	}
	else if (type == CubeManager::WOOD || type == CubeManager::PLANK) {
		_gm->playSound(GameManager::WOOD_BREAK);
	}
	else {
		_gm->playSound(GameManager::STONE_BREAK);
	}
}