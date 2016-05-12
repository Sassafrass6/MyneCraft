#include "SoundManager.h"

SoundManager::SoundManager(void) {

	/* Load Sound Resources */
	bool success = true;
	muted = false;
	mutedM = false;
	mutedS = false;

	// Make sure the sounds are initialized to null or your check is useless! (garbage data can be read as !NULL).
	for (int i = 0; i < NUM_SOUNDS; i++) {
		gameSounds[i] = NULL;
	}

	/* Initialize all SDL subsystems */
	if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 ) {
		printf( "SDL not initialized! SDL Error: %s\n", Mix_GetError() );
		success = false;
	}

 	/* Initialize SDL_mixer */
	if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) {
		printf( "SDL_mixer not initialized! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	/* Load music & sound effects */

#ifdef __linux__
	std::string relative = "../Assets/";
#endif
#ifdef _WIN32
	std::string relative = "../../../Mynecraft/Assignment-2/Assets/";
#endif

	music = Mix_LoadMUS((relative + "MyndlessCrafting.wav").c_str());
	gameSounds[GLASS_BREAK] = Mix_LoadWAV((relative + "glass-break.wav").c_str());
	gameSounds[GRASS_BREAK] = Mix_LoadWAV((relative + "grass-break.wav").c_str());
	gameSounds[SAND_BREAK] = Mix_LoadWAV((relative + "sand-break.wav").c_str());
	gameSounds[SNOW_BREAK] = Mix_LoadWAV((relative + "snow-break.wav").c_str());
	gameSounds[STONE_BREAK] = Mix_LoadWAV((relative + "stone-break.wav").c_str());
	gameSounds[WOOD_BREAK] = Mix_LoadWAV((relative + "wood-break.wav").c_str());
	gameSounds[POP] = Mix_LoadWAV((relative + "pop.wav").c_str());
	gameSounds[STEP] = Mix_LoadWAV((relative + "step.wav").c_str());

	Mix_VolumeMusic(35);
	
}

/* Sound Functions */

SoundManager::~SoundManager(void) {
}

void SoundManager::startMusic(void) {
	Mix_PlayMusic( music, -1 );
}
/* Play a sound based on the soundID. (Sound IDs are identified in the header) */
void SoundManager::playSound(int soundID) {
	if ( gameSounds[soundID] != NULL && !mutedS ) {
		Mix_PlayChannel( -1, gameSounds[soundID], 0 );
	}
}

// Mute all sounds
void SoundManager::mute(void) {
	muted = !muted;
	mutedM = muted;
	mutedS = muted;

	if ( mutedM != mutedS ) {
		muted = mutedM = mutedS = true;
	}

	muteMusic(mutedM);
}

/* As of now sounds and music
cannot be muted independently. */

// Mute music only
void SoundManager::muteMusic(bool mute) {
	 if ( mute ) {
	 	Mix_PauseMusic();
	 } 
	 else { 
	 	Mix_ResumeMusic();
	 }
}

// Mute sounds only
void SoundManager::muteSounds(bool mute) {
	mutedS = mute;
}

/* Free resources */
void SoundManager::destroy(void) {
	 //Free the music 
	Mix_FreeMusic( music );

	/* Free Sounds */
	for ( int i = 0 ; i < NUM_SOUNDS ; i++ ) {
		if ( gameSounds[i] != NULL )
			Mix_FreeChunk( gameSounds[i] );
		gameSounds[i] = NULL;
	}
	//Quit SDL subsystems
	Mix_Quit();
	SDL_Quit();
}