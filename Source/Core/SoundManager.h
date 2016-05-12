#pragma once

#ifdef _WIN32
#include <SDL.h>
#include <SDL_mixer.h>
#endif
#ifdef __linux__
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#endif

#include <string>


class SoundManager {
public:
	enum { SOUND, GLASS_BREAK, GRASS_BREAK, SAND_BREAK, SNOW_BREAK, STONE_BREAK, WOOD_BREAK, POP, STEP, HURT, NUM_SOUNDS } sounds;

	SoundManager(void);
	~SoundManager(void);

	void playSound(int);
	void startMusic(void);
	void mute(void);
	void muteMusic(bool mute);
	void muteSounds(bool mute);
	void destroy(void);

protected:
	// Sound Data
	bool muted; //overall muted
	bool mutedM; //music
	bool mutedS; //sounds
	Mix_Music *music = NULL;
	Mix_Chunk *gameSounds[NUM_SOUNDS];
};