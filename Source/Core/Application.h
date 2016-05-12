#pragma once

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreWindowEventUtilities.h>
#include <OgreRenderTargetListener.h> 

#include <OISMouse.h>
#include <OISKeyboard.h>
#include <OISInputManager.h>

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/Renderer.h>

#include <unordered_map>
#include <unordered_set>
#include <list>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <utility>

#include "Perlin.h"
#include "NetManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "OISManager.h"
#include "Simulator.h"
#include "PlayerObject.h"
#include "StaticObject.h"
#include "Chunk.h"
#include "CubeManager.h"
#include "BiomeManager.h"
#include "Player.h"
#include "Cube.h"
#include "BlockInfo.h"
#include "Rand.h"
#include "HashCombine.h"

class Application : public Ogre::FrameListener, public Ogre::WindowEventListener, public Ogre::RenderTargetListener
{
public:
	Application();
	virtual ~Application();

	virtual void init();

	enum State{ HOME, SINGLE, SERVER, CLIENT, WIN, HOWTO };
	State gameState = HOME;

	Ogre::Root * mRoot;
	Ogre::String mResourcesCfg;
	Ogre::String mPluginsCfg;
	Ogre::RenderWindow * mRenderWindow;
	Ogre::SceneManager * mSceneManager;
	Ogre::Camera* playerCam;
	Ogre::Timer* t1;
	BiomeManager* biomeManager;
	Player* player = NULL;
	GameObject* playerObj = NULL;
	Player* otherPlayer = NULL;
	GameObject* otherPlayerObj = NULL;
	Cube* highlight = NULL;
	Ogre::Light* sun;
	long long frame = 0;

	NetManager* netManager = nullptr;

	GameManager* gameManager;
	OISManager* _oisManager;
	Simulator* _simulator;

    CEGUI::OgreRenderer* mRenderer;
	
	CEGUI::Window* quitButton;
	CEGUI::Window* diamondCount;
	CEGUI::Window* winTextBox;
    CEGUI::Window* singlePlayerButton;

	std::vector<Ogre::Camera*> cameras;
	std::list<GameState> states;

	Ogre::StaticGeometry* sg;

	Perlin* perlin;
	
	std::unordered_map<std::pair<int,int>, Chunk*> prevChunks;
	std::unordered_map<std::pair<int, int>, Chunk*> modifiedChunks;
	std::unordered_set<Chunk*> chunksToBuild;

	Chunk* currentChunk = nullptr;

	int width;
	int height;
	int seed;
	float lastTime = 0.0f;

	std::string magicHeader = "#MCSAVEFILE";

	double fps = 120.0;
#ifdef _WIN32
	#ifdef _DEBUG
		int fieldOfView = 5000;
	#else
		int fieldOfView = 15000;
	#endif
#endif
#ifdef __linux__
	int fieldOfView = 20000;
#endif

	bool begin = false;
	bool mRunning = true;

	virtual bool frameRenderingQueued(const Ogre::FrameEvent &evt) override;
	void createRootEntity(std::string name, std::string mesh, int x, int y, int z);
	Ogre::Entity* createRootEntity(std::string mesh, int x, int y, int z);
	void createChildEntity(std::string name, std::string mesh, Ogre::SceneNode* sceneNode, int x, int y, int z);

	PlayerObject* createPlayerObject(Ogre::String nme, GameObject::objectType tp, Ogre::String meshName, int x, int y, int z, Ogre::Vector3 scale, Ogre::Degree pitch, Ogre::Degree yaw, Ogre::Degree roll, Ogre::SceneManager* scnMgr, GameManager* ssm, Ogre::Real mss, Ogre::Real rest, Ogre::Real frict, bool kinematic, Simulator* mySim);
	Cube* createCube(Ogre::String nme, GameObject::objectType tp, Ogre::String meshName, int x, int y, int z, Ogre::Vector3 scale, Ogre::Degree pitch, Ogre::Degree yaw, Ogre::Degree roll, Ogre::SceneManager* scnMgr, GameManager* ssm, Ogre::Real mss, Ogre::Real rest, Ogre::Real frict, bool kinematic, Simulator* mySim);
	bool update(const Ogre::FrameEvent &evt);
	bool handleGUI(const Ogre::FrameEvent &evt);
	bool updateServer(const Ogre::FrameEvent &evt);
	bool updateClient(const Ogre::FrameEvent &evt);

	void setupWorld();
	void saveWorld();
	void setupWindowRendererSystem(void);
	void setupResources(void);
	void setupOIS(void);
	void setupCEGUI(void);
	void setupCameras(void);
	void setupGM(void);
	void setupLighting(void);
	void createGame(void);
	void createObjects(void);

	bool Quit(const CEGUI::EventArgs&);
	bool StartSinglePlayer(const CEGUI::EventArgs&);

	bool error();
	std::unordered_map<std::string, char*> dataParser(char*);
	void hideGui();
	void showGui();
	void setState(State state);

	Chunk* getChunk(std::unordered_map<std::pair<int, int>, Chunk*>& chunks,int, int);
	void recomputeColliders(std::unordered_map<std::pair<int, int>, Chunk*>& chunks, int, int);
	void moveDayTime(float time);
	void loadSeed();
	void buildChunks(int);
};

