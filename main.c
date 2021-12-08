// a C program to help me develop my relative pitch, and function as a piano/keyboard.

// find more tone frequencies here:  
//
//	https://en.wikipedia.org/wiki/Piano_key_frequencies
//

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

static const double tau = 2.0 * M_PI;


enum note {

	C4,
	D4,
	E4,
	F4,
	G4,

	A4,
	B4,
	C5,
	D5,
	E5,

	note_count,
};

static const double frequency[] = {
	261.6256, // C4
	293.6648, // D4
	329.6276, // E4
	349.2282, // F4
	391.9954, // G4
	440.0000, // A4
	493.8833, // B4
	523.2511, // C5
	587.3295, // D5
	659.2551, // E5
};

static const int keybind[]= {
	SDL_SCANCODE_A,
	SDL_SCANCODE_S,
	SDL_SCANCODE_D,
	SDL_SCANCODE_F,
	SDL_SCANCODE_G,
	SDL_SCANCODE_H,
	SDL_SCANCODE_J,
	SDL_SCANCODE_K,
	SDL_SCANCODE_L,
	SDL_SCANCODE_SEMICOLON,
};

static bool active[note_count] = {0};
static double time_value[note_count] = {0};

static void callback(void* __attribute__((unused)) userdata, Uint8* stream, int len) {
	int16_t* snd = (int16_t*)(stream);
	for (int i = 0; i < len / 2; i++) {

		// super-impose all sine waves together:
		double sum = 0.0;
		for (int n = 0; n < note_count; n++) 
			sum += sin(time_value[n]);
	
		// write value to buffer:
		snd[i] = (int16_t)(3200 * (sum)); // amplitude should be the largest n such that n * note_count < 32,767.
		
		// move time_value for each note forward based on frequency:
		for (int n = 0; n < note_count; n++) {
			time_value[n] += (double)active[n] * frequency[n] * tau / 44100.0;
			if (time_value[n] >= tau) time_value[n] -= tau;
		}
	}
}

int main() {

	srand((unsigned)time(NULL));

	if (SDL_Init(SDL_INIT_EVERYTHING)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));

	SDL_AudioSpec spec, aspec; 
	SDL_zero(spec);
	spec.freq = 44100; 
	spec.format = AUDIO_S16SYS;
	spec.channels = 1;
	spec.samples = 4096;
	spec.callback = callback;
	spec.userdata = NULL;

	SDL_Window *window = SDL_CreateWindow("music utility", 0, 0, 0, 0, 0);
	const unsigned int id = SDL_OpenAudioDevice(NULL, 0, &spec, &aspec, SDL_AUDIO_ALLOW_ANY_CHANGE);
	SDL_PauseAudioDevice(id, 0);

	bool quit = false;

	while (not quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);

			if (event.type == SDL_QUIT) quit = true;

			else if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_ESCAPE]) quit = true;
				if (key[SDL_SCANCODE_Q]) quit = true;
			}
		}

		const Uint8* key = SDL_GetKeyboardState(0);

		for (int n = 0; n < note_count; n++) 
			active[n] = !!key[keybind[n]];
		// if (key[SDL_SCANCODE_A]) { freq = 440; freq2 = 261.6256; }
		// if (key[SDL_SCANCODE_W]) { freq = 440; freq2 = 220; }
		// if (key[SDL_SCANCODE_0]) { freq = 0; freq2 = 0; }
		// if (key[SDL_SCANCODE_E]) { freq = 0; }
		// if (key[SDL_SCANCODE_F]) { freq2 = 0; }
		
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
}

