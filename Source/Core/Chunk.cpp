#include "Chunk.h"
#include "MultiPlatformHelper.h"

Chunk::Chunk(int xStart, int yStart, Ogre::SceneManager* mSceneManager, BiomeManager* biomeMgr, Perlin* perlin, Simulator* sim, bool generate) : _biomeMgr(biomeMgr), _xStart(xStart), _yStart(yStart), _mSceneManager(mSceneManager), _simulator(sim) {
	if (air == nullptr) air = new StaticObject(nullptr, CubeManager::AIR, Ogre::Vector3(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE), Ogre::Vector3::ZERO, sim, this);

	_name = getChunkName(xStart, yStart);

	Ogre::Vector3 scale(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);
	_scale = scale;

	_xEnd = xStart + CHUNK_SIZE;
	_yEnd = yStart + CHUNK_SIZE;

	_sg = mSceneManager->createStaticGeometry(_name);

	_sg->setOrigin(Ogre::Vector3(xStart*CHUNK_SCALE_FULL + CHUNK_SCALE*CHUNK_SIZE, 0, yStart*CHUNK_SCALE_FULL + CHUNK_SCALE*CHUNK_SIZE));
	_sg->setCastShadows(false);

	Biome* curBiome = biomeMgr->inBiome(_xStart, _yStart);

	float worldScale = 2.0;

	// Precompute perlin values
	if (generate) { 
		for (int i = 0; i < CHUNK_SIZE + 2; i++) {
			for (int j = 0; j < CHUNK_SIZE + 2; j++) {
				int chunkx = xStart + i - 1;
				int chunky = yStart + j - 1;

				float fi = (float)chunkx / (float)CHUNK_SCALE_FULL;
				float fj = (float)chunky / (float)CHUNK_SCALE_FULL;

				float steepnessY = (perlin->getPerlin((fi + 10000.0f) / worldScale, (fj + 10000.0f) / worldScale) * 150);
				float steepnessZ = (perlin->getPerlin((fi + 1000.0f) / worldScale, (fj + 1000.0f) / worldScale) * 150);
				float steepnessCaves = (perlin->getPerlin((fi + 100.0f) / worldScale, (fj + 100.0f) / worldScale) * 99);
				int caveHeight = (perlin->getPerlin((fi + 512.0f) / worldScale, (fj + 512.0f) / worldScale) * 15) + 9;

				// Sand biome is less steep
				/* We must make sure not to change steepness for adjacent chunk blocks because it could be in a different biome
				 and it would compute its height for its biome which won't be congruent with our values here. Thus the interpolation will think everything is okay
				 and not interpolate correctly. */

				if (curBiome->getType() == CubeManager::SAND && curBiome->inRadius(chunkx, chunky)) {
					float steepnessDivider = curBiome->getSteepnessDivider(chunkx, chunky);
					if (steepnessDivider < 1) steepnessDivider = 1;
					steepnessY /= steepnessDivider;
				}

				float y1 = (perlin->getPerlin(fi / worldScale, fj / worldScale)*steepnessY);
				float y2 = 1.0f / 2.0f * (perlin->getPerlin(2 * fi / worldScale, 2 * fj / worldScale)*steepnessY);
				float y3 = 1.0f / 4.0f * (perlin->getPerlin(4 * fi / worldScale, 4 * fj / worldScale)*steepnessY);
				float y4 = 1.0f / 8.0f * (perlin->getPerlin(8 * fi / worldScale, 8 * fj / worldScale)*steepnessY);
				float y5 = 1.0f / 16.0f * (perlin->getPerlin(16 * fi / worldScale, 16 * fj / worldScale)*steepnessY);

				float z1 = (perlin->getPerlin((fi + 99), (fj + 99))*steepnessZ);

				float pvaly = y1 + y2 + y3 + y4 + y5;
				float pvalz = z1;

				pvaly *= 2.5;
				pvalz *= 2.5;

				int y = (int)pvaly;
				int z = (int)pvalz;

				heights[i][j] = y;
				topHeights[i][j] = z - 11;
				bottomHeights[i][j] = topHeights[i][j] - caveHeight;
				caveHeights[i][j] = caveHeight;
				caves[i][j] = steepnessCaves;
			}
		}
		// Does this chunk generate new terrain?
		for (int i = 0; i < CHUNK_SIZE; ++i) {
			for (int j = 0; j < CHUNK_SIZE; ++j) {

				int chunkx = xStart + i - 1;
				int chunky = yStart + j - 1;
				int y = heights[i + 1][j + 1];

				float fi = (float)chunkx / (float)CHUNK_SCALE_FULL;
				float fj = (float)chunky / (float)CHUNK_SCALE_FULL;

				Ogre::Vector3 pos(chunkx*scale.x * 2, y*scale.y * 2, chunky*scale.z * 2);
				key index = getKey(pos);

				/* If the surface terrain was created, water or a tree can be placed at that location. */
				bool surfaceTerrainCreated = createTerrainColumn(i, j, pos);

				if (surfaceTerrainCreated) {
					StaticObject* so = _staticObjects[index];
					/* Create tree returns true if a tree was created in this position. */
					if (!createTree(so->_pos, so->_cubeType)) {
						int numAir = computeMaxNeighbor(i + 1, j + 1, heights);

						for (int i = 0; i < numAir; i++) {
							key airIndex = getKey(so->_pos + Ogre::Vector3(0, CHUNK_SCALE_FULL*(i + 1), 0));
							_staticObjects[airIndex] = air;
						}
					}
					buildWaterBlock(y, pos);
				}
				else if (y <= waterLevel)
					buildWaterBlock(y, pos);

				createCloud(pos);

				_sg->setRegionDimensions(Ogre::Vector3(2000, 300, 2000));
			}
		}
	}
}

Chunk::~Chunk() {
	for(auto& var : _staticObjects) {
		if (var.second == nullptr || var.second == air) continue;
		delete var.second;
		var.second = nullptr;
	}
	_staticObjects.clear();
	_mSceneManager->destroyStaticGeometry(_sg);
}

bool Chunk::pointInChunk(float x, float y) {
	int roundX = (int) x;
	int roundY = (int) y;

	return (roundX >= _xStart*_scale.x*2 && roundX <= _xEnd*_scale.x*2) && (roundY >= _yStart*_scale.y*2 && roundY <= _yEnd*_scale.y*2);
}

void Chunk::addChunksToSimulator(Ogre::Vector3& playerPos) {
	for (auto& var : _staticObjects) {
		if(var.second != nullptr && var.second != air)
			if(var.second->_pos.distance(playerPos) <= 550) {
				var.second->addToSimulator();
			}
	}
}

void Chunk::addBlockToStaticGeometry(CubeManager::CubeType cubeType, Ogre::Vector3 position, key index) {
	StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(cubeType), cubeType, _scale, position, _simulator, this);
	_staticObjects[index] = so;
	_sg->addEntity(so->_geom, position, so->_orientation, so->_scale);
}

void Chunk::interpolateBlock(int i, int j, int hs[CHUNK_SIZE+2][CHUNK_SIZE+2], Ogre::Vector3& pos) {
	int interpolate = computeMinNeighbor(i+1, j+1, hs);

	if(interpolate > 0) {
		CubeManager::CubeType interpolateType = CubeManager::DIRT;
		for(int i = 0; i < interpolate; i++) {
			if (i > 0) interpolateType = CubeManager::STONE;
			Ogre::Vector3 newPos = pos - Ogre::Vector3(0, CHUNK_SCALE_FULL*(i+1), 0);
			key newIndex = getKey(newPos);
			if ( !_staticObjects[newIndex] ) {
				StaticObject* sos = new StaticObject(CubeManager::getSingleton()->getEntity(interpolateType), interpolateType, _scale, newPos, _simulator, this);
				_staticObjects[newIndex] = sos;
				_sg->addEntity(sos->_geom, sos->_pos, sos->_orientation, sos->_scale);
			}
		}	
	}
}

void Chunk::removeBlock(const std::vector<Chunk*>& chunks, StaticObject* obj) {
	if (obj == nullptr) assert(!"Block was null");
	if (obj == air) return;
	
	key index = getKey(obj->_pos);

	if (_staticObjects.count(index) == 0) return;

	_sg->reset();
	_staticObjects[index] = air;

	key tkey = index;
	if (lights[tkey])
		lights[tkey]->setVisible(false);

	for (auto& so : _staticObjects) {
		if (so.second == air || so.second == nullptr || (so.second->_cubeType == CubeManager::WATER && so.second->_pos.y / CHUNK_SCALE_FULL < WATER_LEVEL - 1)) continue;
		_sg->addEntity(so.second->_geom, so.second->_pos, so.second->_orientation, so.second->_scale);
	}

	// Only create neighbors for non-tree blocks
	if (!_biomeMgr->isTreeType(obj->_cubeType)) {
		// top
		CubeManager::CubeType newType = getGeneratedType(obj->_cubeType, (int) obj->_pos.y);

		Ogre::Vector3 topPos = obj->_pos + Ogre::Vector3(0, CHUNK_SCALE_FULL, 0);
		key topIndex = getKey(topPos);
		StaticObject* topObj = getObjFromChunks(chunks, topIndex);
		if (topObj == nullptr) {
			Ogre::Vector3 scale(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(newType), newType, scale, topPos, _simulator, this);
			if(so == nullptr) assert(!"StaticObject is null");
			_staticObjects[topIndex] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
		}

		// bottom
		Ogre::Vector3 bottomPos = obj->_pos + Ogre::Vector3(0, -CHUNK_SCALE_FULL, 0);
		key bottomIndex = getKey(bottomPos);
		StaticObject* bottomObj = getObjFromChunks(chunks, bottomIndex);
		if (bottomObj == nullptr) {
			Ogre::Vector3 scale(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(newType), newType, scale, bottomPos, _simulator, this);
			_staticObjects[bottomIndex] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
		}

		// left
		Ogre::Vector3 leftPos = obj->_pos + Ogre::Vector3(-CHUNK_SCALE_FULL, 0, 0);
		key leftIndex = getKey(leftPos);
		StaticObject* leftObj = getObjFromChunks(chunks, leftIndex);
		if (leftObj == nullptr) {
			Ogre::Vector3 scale(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(newType), newType, scale, leftPos, _simulator, this);
			_staticObjects[leftIndex] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
		}

		// right
		Ogre::Vector3 rightPos = obj->_pos + Ogre::Vector3(CHUNK_SCALE_FULL, 0, 0);
		key rightIndex = getKey(rightPos);
		StaticObject* rightObj = getObjFromChunks(chunks, rightIndex);
		if (rightObj == nullptr) {
			Ogre::Vector3 scale(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(newType), newType, scale, rightPos, _simulator, this);
			_staticObjects[rightIndex] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
		}

		// back
		Ogre::Vector3 backPos = obj->_pos + Ogre::Vector3(0, 0, -CHUNK_SCALE_FULL);
		key backIndex = getKey(backPos);
		StaticObject* backObj = getObjFromChunks(chunks, backIndex);
		if (backObj == nullptr) {
			Ogre::Vector3 scale(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(newType), newType, scale, backPos, _simulator, this);
			_staticObjects[backIndex] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
		}

		// front
		Ogre::Vector3 frontPos = obj->_pos + Ogre::Vector3(0, 0, CHUNK_SCALE_FULL);
		key frontIndex = getKey(frontPos);
		StaticObject* frontObj = getObjFromChunks(chunks, frontIndex);
		if (frontObj == nullptr) {
			Ogre::Vector3 scale(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(newType), newType, scale, frontPos, _simulator, this);
			_staticObjects[frontIndex] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
		}
	}

	_sg->build();
}

void Chunk::addBlock(const std::vector<Chunk*>& chunks, StaticObject* obj, const btVector3& hitnormal, CubeManager::CubeType newType) {
	if (obj == nullptr) assert(!"Block was null");
	if (obj == air) return;

	// Sometimes the vector comes in with very small values close to zero, just truncate values to 0 or 1
	int x = round(hitnormal.x());
	int y = round(hitnormal.y());
	int z = round(hitnormal.z());
	
	Ogre::Vector3 scale;
	if (newType == CubeManager::TORCH)
		scale = Ogre::Vector3(CHUNK_SCALE/5, 4*CHUNK_SCALE/5, CHUNK_SCALE/5);
	else
		scale = Ogre::Vector3(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);

	btVector3 norm = btVector3(x, y, z);

	key index = getKey(obj->_pos);

	if (_staticObjects.count(index) == 0) return;

	_sg->reset();

	for (auto& so : _staticObjects) {
		if (so.second == air || so.second == nullptr || (so.second->_cubeType == CubeManager::WATER && so.second->_pos.y/CHUNK_SCALE_FULL < WATER_LEVEL - 1) ) continue;
		_sg->addEntity(so.second->_geom, so.second->_pos, so.second->_orientation, so.second->_scale);
	}
	
	Ogre::Vector3 pos;

	// Top
	if (norm == btVector3(0, 1, 0)) {
		pos = obj->_pos + Ogre::Vector3(0, CHUNK_SCALE_FULL, 0);
		key topIndex = getKey(pos);
		StaticObject* topObj = getObjFromChunks(chunks, topIndex);

		if (topObj == air) {
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(newType), newType, scale, pos, _simulator, this);
			if (so == nullptr) assert(!"StaticObject is null");
			_staticObjects[topIndex] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
			createAirNeighbors(chunks, so);
		}
	}

	// Bottom
	if (norm == btVector3(0, -1, 0)) {
		pos = obj->_pos + Ogre::Vector3(0, -CHUNK_SCALE_FULL, 0);
		key index = getKey(pos);
		StaticObject* obj = getObjFromChunks(chunks, index);

		if (obj == air) {
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(newType), newType, scale, pos, _simulator, this);
			if (so == nullptr) assert(!"StaticObject is null");
			_staticObjects[index] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
			createAirNeighbors(chunks, so);
		}
	}

	// Left
	if (norm == btVector3(-1, 0, 0)) {
		pos = obj->_pos + Ogre::Vector3(-CHUNK_SCALE_FULL, 0, 0);
		key index = getKey(pos);
		StaticObject* obj = getObjFromChunks(chunks, index);

		if (obj == air) {
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(newType), newType, scale, pos, _simulator, this);
			if (so == nullptr) assert(!"StaticObject is null");
			_staticObjects[index] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
			createAirNeighbors(chunks, so);
		}
	}

	// Right
	if (norm == btVector3(1, 0, 0)) {
		pos = obj->_pos + Ogre::Vector3(CHUNK_SCALE_FULL, 0, 0);
		key index = getKey(pos);
		StaticObject* obj = getObjFromChunks(chunks, index);

		if (obj == air) {
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(newType), newType, scale, pos, _simulator, this);
			if (so == nullptr) assert(!"StaticObject is null");
			_staticObjects[index] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
			createAirNeighbors(chunks, so);
		}
	}

	// Front
	if (norm == btVector3(0, 0, 1)) {
		pos = obj->_pos + Ogre::Vector3(0, 0, CHUNK_SCALE_FULL);
		key index = getKey(pos);
		StaticObject* obj = getObjFromChunks(chunks, index);

		if (obj == air) {
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(newType), newType, scale, pos, _simulator, this);
			if (so == nullptr) assert(!"StaticObject is null");
			_staticObjects[index] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
			createAirNeighbors(chunks, so);
		}
	}

	// Back
	if (norm == btVector3(0, 0, -1)) {
		pos = obj->_pos + Ogre::Vector3(0, 0, -CHUNK_SCALE_FULL);
		key index = getKey(pos);
		StaticObject* obj = getObjFromChunks(chunks, index);

		if (obj == air) {
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(newType), newType, scale, pos, _simulator, this);
			if (so == nullptr) assert(!"StaticObject is null");
			_staticObjects[index] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
			createAirNeighbors(chunks, so);
		}
	}

	if (newType == CubeManager::TORCH) {
		key tkey = getKey(pos);
		if (!lights[tkey]) {
			float range = 5000.0f;
			Ogre::Light* light = _mSceneManager->createLight(getKeyName(tkey));
			light->setDiffuseColour(0.9, 0.4, 0.2);
			light->setType(Ogre::Light::LT_POINT);
			light->setAttenuation(range, 1.0f, .5f/range, 5.0f/(range*range));
			light->setPosition(pos);
			light->setVisible(true);
			lights[tkey] = light;
		}
	}
	_sg->build();
}

void Chunk::createAirNeighbors(const std::vector<Chunk*>& chunks, StaticObject* obj) {
	// top
	Ogre::Vector3 topPos = obj->_pos + Ogre::Vector3(0, CHUNK_SCALE_FULL, 0);
	key topIndex = getKey(topPos);
	StaticObject* topObj = getObjFromChunks(chunks, topIndex);
	if (topObj == nullptr) {
		_staticObjects[topIndex] = air;
	}

	// bottom
	Ogre::Vector3 bottomPos = obj->_pos + Ogre::Vector3(0, -CHUNK_SCALE_FULL, 0);
	key bottomIndex = getKey(bottomPos);
	StaticObject* bottomObj = getObjFromChunks(chunks, bottomIndex);
	if (bottomObj == nullptr) {
		_staticObjects[bottomIndex] = air;
	}

	// left
	Ogre::Vector3 leftPos = obj->_pos + Ogre::Vector3(-CHUNK_SCALE_FULL, 0, 0);
	key leftIndex = getKey(leftPos);
	StaticObject* leftObj = getObjFromChunks(chunks, leftIndex);
	if (leftObj == nullptr) {
		_staticObjects[leftIndex] = air;
	}

	// right
	Ogre::Vector3 rightPos = obj->_pos + Ogre::Vector3(CHUNK_SCALE_FULL, 0, 0);
	key rightIndex = getKey(rightPos);
	StaticObject* rightObj = getObjFromChunks(chunks, rightIndex);
	if (rightObj == nullptr) {
		_staticObjects[rightIndex] = air;
	}

	// back
	Ogre::Vector3 backPos = obj->_pos + Ogre::Vector3(0, 0, -CHUNK_SCALE_FULL);
	key backIndex = getKey(backPos);
	StaticObject* backObj = getObjFromChunks(chunks, backIndex);
	if (backObj == nullptr) {
		_staticObjects[backIndex] = air;
	}

	// front
	Ogre::Vector3 frontPos = obj->_pos + Ogre::Vector3(0, 0, CHUNK_SCALE_FULL);
	key frontIndex = getKey(frontPos);
	StaticObject* frontObj = getObjFromChunks(chunks, frontIndex);
	if (frontObj == nullptr) {
		_staticObjects[frontIndex] = air;
	}
}

StaticObject* Chunk::getObjFromChunks(const std::vector<Chunk*>& chunks, key index) {
	for (auto& var : chunks) {
		if (var->_staticObjects[index]) return var->_staticObjects[index];
	}

	return nullptr;
}

StaticObject* Chunk::getBlock(int x, int y, int z) {

	key index = getKey(x,y,z);

	StaticObject* theObj = _staticObjects[index];

	if(theObj) return theObj;
	else return nullptr;
}

// This is just temporary and needs to be improved
// Playing around with probabilities
CubeManager::CubeType Chunk::getGeneratedType(CubeManager::CubeType objType, int height) {
	static int offset = 18 * CHUNK_SCALE_FULL;
	switch (objType) {
		case CubeManager::GRASS :
		case CubeManager::DIRT :
			if (Rand::rand()%500 < (height + offset))
				return CubeManager::DIRT;
			else 
				return CubeManager::ROCK;
			break;

		case CubeManager::SAND :
			if (Rand::rand()%500 < (height + offset))
				return CubeManager::SAND;
			else
				return CubeManager::ROCK;
			break;

		case CubeManager::SNOW :
			return CubeManager::DIRT;
			break;
		case CubeManager::DIAMOND :
		case CubeManager::ROCK :
			return CubeManager::ROCK;
			break;
		default :
			return objType;
	}
}

bool Chunk::createTerrainColumn(int i, int j, Ogre::Vector3& pos) {

	Biome* curBiome = _biomeMgr->inBiome(_xStart, _yStart);
	CubeManager::CubeType tempType;

	int chunkx = _xStart + i - 1;
	int chunky = _yStart + j - 1;

	int i1 = i+1;
	int j1 = j+1;

	int y = heights[i1][j1];
	int cave = caves[i1][j1];
	int heightTop = topHeights[i1][j1];
	int heightBottom = heightTop - caveHeights[i1][j1];

	/* Flags for cave drawing. */
	bool buildTerrain = false;
	bool drawCave = y > heightTop; /* Below surface */

	Ogre::Vector3 posCaveTop(chunkx*_scale.x*2, heightTop*_scale.y*2, chunky*_scale.z*2);
	Ogre::Vector3 posCaveBottom = posCaveTop - Ogre::Vector3(0, caveHeights[i1][j1]*CHUNK_SCALE_FULL, 0);

	key index = getKey(pos);
	key indexTop = getKey(posCaveTop);
	key indexBottom = getKey(posCaveBottom);

	/* Replace with a cave cube. */
	CubeManager::CubeType rndCube = getCaveCubeType();

	/* Within Cave Walls */
	if ( (cave > -2  && cave < 2) ) {
		/* Cave is below surface */
		if ( drawCave ) {
			if ( !_staticObjects[index] )
				buildTerrain = true;

			if ( !_staticObjects[indexTop] )
				buildCaveBlock(i, j, indexTop, posCaveTop, -1, rndCube, topHeights);

			for ( int kk = 0 ; kk < caveHeights[i1][j1] ; kk++ ) {
				Ogre::Vector3 caveAirPos = posCaveTop-Ogre::Vector3(0,CHUNK_SCALE_FULL*kk,0);
				key innerWall = getKey(caveAirPos);
				if ( !_staticObjects[innerWall] )
					_staticObjects[innerWall] = air;
			}

			if ( !_staticObjects[indexBottom] ) {
				buildCaveBlock(i, j, indexBottom, posCaveBottom, 1, rndCube, bottomHeights);
			}
		}
		/* Cave is partially underground */
		else if ( y > heightBottom ) {
			if ( y < waterLevel && y > topHeights[i1][j1] )
				buildTerrain = true;

			for ( int kk = -1 ; kk < caveHeights[i1][j1] ; kk++ ) {
				Ogre::Vector3 caveAirPos = posCaveTop-Ogre::Vector3(0,CHUNK_SCALE_FULL*kk,0);
				key innerWall = getKey(caveAirPos);
				_staticObjects[innerWall] = air;
			}
			if ( !_staticObjects[indexBottom] )
				buildCaveBlock(i, j, indexBottom, posCaveBottom, 1, rndCube, bottomHeights);
		}
		else {
			buildTerrain = true;
		}
	}
	/* Coordinates are at the boundary of a cave */
	else if ( (cave == -2 || cave == 2) ) {
		for ( int kk = -1 ; kk < caveHeights[i1][j1]+2 ; kk++ ) {
			Ogre::Vector3 caveWallPos = posCaveTop-Ogre::Vector3(0,CHUNK_SCALE_FULL*kk,0);
			key caveWallIndex = getKey(caveWallPos);
			if ( y > heightTop-kk ) {
				rndCube = getCaveCubeType();
				addBlockToStaticGeometry(rndCube, caveWallPos, getKey(caveWallPos));
			}
			else {
				Ogre::Vector3 caveAirPos = posCaveTop-Ogre::Vector3(0,CHUNK_SCALE_FULL*kk,0);
				key innerWall = getKey(caveAirPos);
				if ( !_staticObjects[innerWall] )
					_staticObjects[innerWall] = air;
			}
		}
		buildTerrain = true;
	}
	else {
		buildTerrain = true;
	}

	/* Cave top is close enough to surface to cause tearing. Fill in the tearing */
	if ( drawCave && y-3 < heightTop ) {
		for ( int kk = 1 ; kk < y-heightTop ; kk++ ) {
			Ogre::Vector3 posAboveCave = posCaveTop+Ogre::Vector3(0,CHUNK_SCALE_FULL*kk,0);
			addBlockToStaticGeometry(rndCube, posAboveCave, getKey(posAboveCave));
		}
	}

	/* Build the surface terrain if necessary */
	if ( buildTerrain ) {
		StaticObject* so;

		if (y >= snowLevel && curBiome != nullptr) {
			if (curBiome->getType() != CubeManager::SAND) {
				Ogre::Entity* type = curBiome->getCubeEntity(chunkx, chunky, y, tempType);
				so = new StaticObject(type, tempType, _scale, pos, _simulator, this);
			}
		}
		if (curBiome != nullptr) {
			Ogre::Entity* type = curBiome->getCubeEntity(chunkx, chunky, y, tempType);
			so = new StaticObject(type, tempType, _scale, pos, _simulator, this);
		}
		else {
			so = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::GRASS), CubeManager::GRASS, _scale, pos, _simulator, this);
		}

		_staticObjects[index] = so;
		_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
		interpolateBlock(i, j, heights, so->_pos);

		key airIndex = getKey(so->_pos + Ogre::Vector3(0, CHUNK_SCALE_FULL, 0));
		_staticObjects[airIndex] = air;
	}

	return buildTerrain;
}

void Chunk::buildWaterBlock(int height, Ogre::Vector3& pos) {

	if ( height <= waterLevel ) {

		Ogre::Vector3 waterLine(pos.x, waterLevel*_scale.y * 2 - CHUNK_SCALE, pos.z);
		Ogre::Vector3 waterLineTwo(pos.x, (waterLevel-1)*_scale.y * 2 - CHUNK_SCALE, pos.z);
		Ogre::Vector3 bottomLine(pos.x, (height+1)*_scale.y * 2, pos.z);
		key waterIndex = getKey(waterLine);
		key waterIndexTwo = getKey(waterLineTwo);
		key bottomIndex = getKey(bottomLine);

		if ( !_staticObjects[waterIndex] ) { 
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::WATER), CubeManager::WATER, _scale, waterLine, _simulator, this);
			_staticObjects[waterIndex] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
		}
		if ( !_staticObjects[waterIndexTwo] ) { 
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::WATER), CubeManager::WATER, _scale, waterLineTwo, _simulator, this);
			_staticObjects[waterIndexTwo] = so;
		}

		if ( (!_staticObjects[bottomIndex] || _staticObjects[bottomIndex] == air) && (height+1) < waterLevel) { 
			StaticObject* so = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::WATER), CubeManager::WATER, _scale, bottomLine, _simulator, this);
			_staticObjects[bottomIndex] = so;
		}

		// Build air on surface of water
		Ogre::Vector3 waterAir = Ogre::Vector3(pos.x, (waterLevel*_scale.y * 2 - CHUNK_SCALE) + CHUNK_SCALE_FULL, pos.z);
		key newAirIndex = getKey(waterAir);
		_staticObjects[newAirIndex] = air;

	}
}

/* Move to CubeManager */
CubeManager::CubeType Chunk::getCaveCubeType() {
	int rnd = Rand::rand()%10;
	if ( Rand::rand()%8000 == 1234 )
		return CubeManager::DIAMOND;
	else if ( Rand::rand()%10 < 3 )
		return CubeManager::DIRT;
	else
		return CubeManager::STONE;
}

void Chunk::buildCaveBlock(int i, int j, key index, Ogre::Vector3& pos, int offset, CubeManager::CubeType type, int hs[CHUNK_SIZE+2][CHUNK_SIZE+2] ) {
	addBlockToStaticGeometry(type, pos, index);
	interpolateBlock(i, j, hs, pos);
	key airIndex = getKey(pos + Ogre::Vector3(0, CHUNK_SCALE_FULL*offset, 0));
	_staticObjects[airIndex] = air;
}

bool Chunk::createTree(const Ogre::Vector3& pos, CubeManager::CubeType type) {

	static const int breadth = 5;
	static const int firHeight = 5;
	static const int firBreadth = 7;

	if ( type == CubeManager::GRASS && Rand::rand()%500 != 5 ) return false;
	if ( type == CubeManager::SNOW && Rand::rand()%3000 != 5 ) return false;
	if ( type == CubeManager::SAND && Rand::rand()%2000 != 5 ) return false;
	if(pos.y <= waterLevel*CHUNK_SCALE_FULL) return false;

	// Describes the shape of the leaves, think of each 5x5 grid as another layer
	bool leaves[breadth][breadth][breadth] = {	{ { 0, 0, 0, 0, 0 }, { 0, 1, 1, 1, 0 }, { 0, 1, 1, 1, 0 }, { 0, 1, 1, 1, 0 }, { 0, 0, 0, 0, 0 } },
												{ { 0, 1, 1, 1, 0 }, { 1, 0, 0, 0, 1 }, { 1, 0, 0, 0, 1 }, { 1, 0, 0, 0, 1 }, { 0, 1, 1, 1, 0 } },
												{ { 0, 1, 0, 1, 0 }, { 1, 0, 0, 0, 1 }, { 1, 0, 0, 0, 1 }, { 1, 0, 0, 0, 1 }, { 0, 1, 1, 1, 0 } },
												{ { 0, 1, 1, 1, 0 }, { 1, 0, 0, 0, 1 }, { 1, 0, 0, 0, 1 }, { 1, 0, 0, 0, 1 }, { 0, 1, 1, 1, 0 } },
												{ { 0, 0, 0, 0, 0 }, { 0, 1, 1, 1, 0 }, { 0, 1, 1, 1, 0 }, { 0, 1, 1, 1, 0 }, { 0, 0, 0, 0, 0 } } };

	bool firTree[firBreadth][firHeight][firBreadth] = {		{ { 0, 0, 1, 1, 1, 0, 0 }, { 0, 0, 0, 1, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 } },
															{ { 0, 1, 1, 1, 1, 1, 0 }, { 0, 0, 1, 1, 1, 0, 0 }, { 0, 0, 0, 1, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 } },
															{ { 1, 1, 1, 0, 1, 1, 1 }, { 0, 1, 1, 0, 1, 1, 0 }, { 0, 0, 1, 1, 1, 0, 0 }, { 0, 0, 0, 1, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 } },
															{ { 1, 1, 0, 0, 0, 1, 1 }, { 1, 1, 0, 0, 0, 1, 1 }, { 0, 1, 1, 0, 1, 1, 0 }, { 0, 0, 1, 0, 1, 0, 0 }, { 0, 0, 0, 1, 0, 0, 0 } },
															{ { 1, 1, 1, 0, 1, 1, 1 }, { 0, 1, 1, 0, 1, 1, 0 }, { 0, 0, 1, 1, 1, 0, 0 }, { 0, 0, 0, 1, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 } },
															{ { 0, 1, 1, 1, 1, 1, 0 }, { 0, 0, 1, 1, 1, 0, 0 }, { 0, 0, 0, 1, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 } },
															{ { 0, 0, 1, 1, 1, 0, 0 }, { 0, 0, 0, 1, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0 } } };
	
	Ogre::Vector3 scale(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);

	switch ( type ) {
		case CubeManager::GRASS:
			// Trunk
			for (int i = 1; i <= 3; i++) {
				StaticObject* treeLimb = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::WOOD), CubeManager::WOOD, scale, pos + Ogre::Vector3(0, i*CHUNK_SCALE_FULL, 0), _simulator, this);
				key index = getKey(treeLimb->_pos);
				_staticObjects[index] = treeLimb;
				_sg->addEntity(treeLimb->_geom, treeLimb->_pos, treeLimb->_orientation, treeLimb->_scale);
			}

			// Leaves
			for (int i = 0; i < breadth; i++) {
				for (int j = 0; j < breadth; j++) {
					for (int k = 0; k < breadth; k++) {
						if (leaves[i][j][k]) {
							StaticObject* treeLimb = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::LEAF), CubeManager::LEAF, scale, pos + Ogre::Vector3((i - 2)*CHUNK_SCALE_FULL, (j + breadth - 1)*CHUNK_SCALE_FULL, (k - 2)*CHUNK_SCALE_FULL), _simulator, this);
							key index = getKey(treeLimb->_pos);
							_staticObjects[index] = treeLimb;
							_sg->addEntity(treeLimb->_geom, treeLimb->_pos, treeLimb->_orientation, treeLimb->_scale);
						}
					}
				}
			}
			return true;
		case CubeManager::SNOW:
			//Fir Tree
			for (int i = 0; i < firBreadth; i++) {
				for (int j = 0; j < firHeight; j++) {
					for (int k = 0; k < firBreadth; k++) {
						if (firTree[i][j][k]) {
							StaticObject* firBlock = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::ICE), CubeManager::ICE, scale, pos + Ogre::Vector3((i - 3)*CHUNK_SCALE_FULL, (2*j+3)*CHUNK_SCALE_FULL, (k - 3)*CHUNK_SCALE_FULL), _simulator, this);
							key index = getKey(firBlock->_pos);
							_staticObjects[index] = firBlock;
							_sg->addEntity(firBlock->_geom, firBlock->_pos, firBlock->_orientation, firBlock->_scale);
						}
					}
				}
			}
			//Trunk
			for (int i = 0 ; i <= firHeight*2 ; i++ ) {
				StaticObject* firBlock = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::WOOD), CubeManager::WOOD, scale, pos + Ogre::Vector3(0, i*CHUNK_SCALE_FULL, 0), _simulator, this);
				key index = getKey(firBlock->_pos);
				_staticObjects[index] = firBlock;
				_sg->addEntity(firBlock->_geom, firBlock->_pos, firBlock->_orientation, firBlock->_scale);
			}
			return true;
		case CubeManager::SAND:
			//Cactus
			for (int i = 1; i <= 10; i++) {
				StaticObject* stalk = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::CACTUS), CubeManager::CACTUS, scale, pos + Ogre::Vector3(0, i*CHUNK_SCALE_FULL, 0), _simulator, this);
				key index = getKey(stalk->_pos);
				_staticObjects[index] = stalk;
				_sg->addEntity(stalk->_geom, stalk->_pos, stalk->_orientation, stalk->_scale);
			}
			//Arms
			if ( Rand::rand()%2 ) {
				StaticObject* armConnector1 = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::CACTUS), CubeManager::CACTUS, scale, pos + Ogre::Vector3(CHUNK_SCALE_FULL, 4*CHUNK_SCALE_FULL, 0), _simulator, this);

				key index = getKey(armConnector1->_pos);
				_staticObjects[index] = armConnector1;
				_sg->addEntity(armConnector1->_geom, armConnector1->_pos, armConnector1->_orientation, armConnector1->_scale);
			
				StaticObject* armConnector2 = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::CACTUS), CubeManager::CACTUS, scale, pos + Ogre::Vector3(-CHUNK_SCALE_FULL, 4*CHUNK_SCALE_FULL, 0), _simulator, this);
				key index2 = getKey(armConnector2->_pos);
				_staticObjects[index2] = armConnector2;
				_sg->addEntity(armConnector2->_geom, armConnector2->_pos, armConnector2->_orientation, armConnector2->_scale);
				
				for (int i = 4; i <= 8; i++) {
					StaticObject* armBlock = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::CACTUS), CubeManager::CACTUS, scale, pos + Ogre::Vector3(2*CHUNK_SCALE_FULL, i*CHUNK_SCALE_FULL, 0), _simulator, this);
					key index = getKey(armBlock->_pos);
					_staticObjects[index] = armBlock;
					_sg->addEntity(armBlock->_geom, armBlock->_pos, armBlock->_orientation, armBlock->_scale);
				}
				for (int i = 4; i <= 9; i++) {
					StaticObject* armBlock = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::CACTUS), CubeManager::CACTUS, scale, pos + Ogre::Vector3(-2*CHUNK_SCALE_FULL, i*CHUNK_SCALE_FULL, 0), _simulator, this);
					key index = getKey(armBlock->_pos);
					_staticObjects[index] = armBlock;
					_sg->addEntity(armBlock->_geom, armBlock->_pos, armBlock->_orientation, armBlock->_scale);
				}
			}
			else {
				StaticObject* armConnector1 = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::CACTUS), CubeManager::CACTUS, scale, pos + Ogre::Vector3(0, 4*CHUNK_SCALE_FULL, CHUNK_SCALE_FULL), _simulator, this);
				key index = getKey(armConnector1->_pos);
				_staticObjects[index] = armConnector1;
				_sg->addEntity(armConnector1->_geom, armConnector1->_pos, armConnector1->_orientation, armConnector1->_scale);
			
				StaticObject* armConnector2 = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::CACTUS), CubeManager::CACTUS, scale, pos + Ogre::Vector3(0, 4*CHUNK_SCALE_FULL, -CHUNK_SCALE_FULL), _simulator, this);
				key index2 = getKey(armConnector2->_pos);
				_staticObjects[index2] = armConnector2;
				_sg->addEntity(armConnector2->_geom, armConnector2->_pos, armConnector2->_orientation, armConnector2->_scale);
				
				for (int i = 4; i <= 8; i++) {
					StaticObject* armBlock = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::CACTUS), CubeManager::CACTUS, scale, pos + Ogre::Vector3(0, i*CHUNK_SCALE_FULL, 2*CHUNK_SCALE_FULL), _simulator, this);
					key index = getKey(armBlock->_pos);
					_staticObjects[index] = armBlock;
					_sg->addEntity(armBlock->_geom, armBlock->_pos, armBlock->_orientation, armBlock->_scale);
				}
				for (int i = 4; i <= 9; i++) {
					StaticObject* armBlock = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::CACTUS), CubeManager::CACTUS, scale, pos + Ogre::Vector3(0, i*CHUNK_SCALE_FULL, -2*CHUNK_SCALE_FULL), _simulator, this);
					key index = getKey(armBlock->_pos);
					_staticObjects[index] = armBlock;
					_sg->addEntity(armBlock->_geom, armBlock->_pos, armBlock->_orientation, armBlock->_scale);
				}
			}
			return true;
		default: return false;
	}
}

bool Chunk::createCloud(const Ogre::Vector3& pos) {
	if(Rand::rand() % 400 != 10) return false;

	int imax = Rand::rand() % 25 + 5;
	int jmax = Rand::rand() % 25 + 5;

	Ogre::Vector3 scale(imax*CHUNK_SCALE, CHUNK_SCALE, jmax*CHUNK_SCALE);

	Ogre::Vector3 cloudPos(pos.x, 60*CHUNK_SCALE_FULL, pos.z);
	StaticObject* cloud = new StaticObject(CubeManager::getSingleton()->getEntity(CubeManager::CLOUD), CubeManager::CLOUD, scale, cloudPos, _simulator, this);
	key index = getKey(cloud->_pos);
	_staticObjects[index] = cloud;
	_sg->addEntity(cloud->_geom, cloud->_pos, cloud->_orientation, cloud->_scale);

	return true;
}

Chunk::key Chunk::getKey(int x, int y, int z) {
	return std::make_tuple(x, y, z);
}

Chunk::key Chunk::getKey(const Ogre::Vector3& pos) {
	return getKey((int)pos.x, (int)pos.y, (int)pos.z);
}

BlockInfo Chunk::getBlockInfo(key thekey, CubeManager::CubeType type) {
	return BlockInfo(std::get<0>(thekey), std::get<1>(thekey), std::get<2>(thekey), type);
}

std::string Chunk::getKeyName(key thekey) {
	char buf[64];
	sprintf(buf, "%d_%d_%d", std::get<0>(thekey), std::get<1>(thekey), std::get<2>(thekey));
	return std::string(buf);
}

void Chunk::rebuildFromSave(const std::vector<BlockInfo>& blocks) {
	for (auto& var : blocks) {
		StaticObject* so;
		Ogre::Vector3 pos(var.x, var.y, var.z);
		Ogre::Vector3 scale(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);
		key index = getKey(pos);

		if (var.type == CubeManager::AIR) {
			_staticObjects[index] = air;
		}
		else if (var.type == CubeManager::WATER && pos.y / CHUNK_SCALE_FULL < WATER_LEVEL - 1) {
			so = new StaticObject(CubeManager::getSingleton()->getEntity(var.type), var.type, scale, pos, _simulator, this);
			_staticObjects[index] = so;
		}
		else if (var.type == CubeManager::TORCH) {
			scale = Ogre::Vector3(CHUNK_SCALE / 5, 4 * CHUNK_SCALE / 5, CHUNK_SCALE / 5);

			key tkey = getKey(pos);
			if (!lights[tkey]) {
				float range = 5000.0f;
				Ogre::Light* light = _mSceneManager->createLight(getKeyName(tkey));
				light->setDiffuseColour(0.9, 0.4, 0.2);
				light->setType(Ogre::Light::LT_POINT);
				light->setAttenuation(range, 1.0f, .5f / range, 5.0f / (range*range));
				light->setPosition(pos);
				light->setVisible(true);
				lights[tkey] = light;
			}

			so = new StaticObject(CubeManager::getSingleton()->getEntity(var.type), var.type, scale, pos, _simulator, this);
			_staticObjects[index] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
		}
		else {
			so = new StaticObject(CubeManager::getSingleton()->getEntity(var.type), var.type, scale, pos, _simulator, this);
			_staticObjects[index] = so;
			_sg->addEntity(so->_geom, so->_pos, so->_orientation, so->_scale);
		}	
	}
	_sg->build();
}

// returns the number of blocks to create below us
int Chunk::computeMinNeighbor(int x, int y, int hs[CHUNK_SIZE+2][CHUNK_SIZE+2]) {
	int min = 99999999;
	int distance = 0;
	int height = hs[x][y];

	// up
	distance = hs[x][y + 1] - height;
	if(distance < min) min = distance;
	// down
	distance = hs[x][y - 1] - height;
	if(distance < min) min = distance;
	// left
	distance = hs[x - 1][y] - height;
	if(distance < min) min = distance;
	// right
	distance = hs[x + 1][y] - height;
	if(distance < min) min = distance;

	// A block is is lower than 1 block below us
	if(min < -1) {
		return abs(min + 1);
	}
	else {
		return 0;
	}
}

// returns the number of air blocks to create above us
int Chunk::computeMaxNeighbor(int x, int y, int hs[CHUNK_SIZE+2][CHUNK_SIZE+2]) {
	int max = -99999999;
	int distance = 0;
	int height = hs[x][y];

	// up
	distance = hs[x][y + 1] - height;
	if(distance > max) max = distance;
	// down
	distance = hs[x][y - 1] - height;
	if(distance > max) max = distance;
	// left
	distance = hs[x - 1][y] - height;
	if(distance > max) max = distance;
	// right
	distance = hs[x + 1][y] - height;
	if(distance > max) max = distance;

	// A block is is lower than 1 block below us
	if(max > 1) {
		return max;
	}
	else {
		return 1;
	}
}

void Chunk::build() {
	_sg->build();
}
