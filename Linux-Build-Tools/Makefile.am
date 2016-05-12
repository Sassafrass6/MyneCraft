noinst_HEADERS = Application.h MultiPlatformHelper.h OISManager.h SceneHelper.h CoreConfig.h SoundManager.h GameState.h GameManager.h GameObject.h CubeManager.h PlayerObject.h Cube.h Perlin.h Simulator.h BulletContactCallback.h CollisionContext.h OgreMotionState.h NetManager.h StaticObject.h Biome.h BiomeManager.h Chunk.h Player.h Animation.h Action.h BlockInfo.h Rand.h

bin_PROGRAMS = MyneCraft
MyneCraft_CPPFLAGS = -I$(top_srcdir) -std=c++11 -O2
MyneCraft_SOURCES = Application.cpp main.cpp OISManager.cpp SoundManager.cpp GameManager.cpp Simulator.cpp GameObject.cpp CubeManager.cpp PlayerObject.cpp Cube.cpp Perlin.cpp OgreMotionState.cpp CollisionContext.cpp BulletContactCallback.cpp NetManager.cpp StaticObject.cpp Biome.cpp BiomeManager.cpp Chunk.cpp Player.cpp Animation.cpp Action.cpp Rand.cpp
MyneCraft_CXXFLAGS = $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS) $(CEGUI_CFLAGS)
MyneCraft_LDADD = $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS) $(CEGUI_LIBS) $(CEGUI_OGRE_LIBS)
MyneCraft_LDFLAGS = -lOgreOverlay -lboost_system -lSDL -lSDL_mixer -lSDL_net 

EXTRA_DIST = buildit makeit
AUTOMAKE_OPTIONS = foreign