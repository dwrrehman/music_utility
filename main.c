
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#define tau (2.0 * M_PI)


static double my_time = 0;
static double freq = 440;

static double my_time2 = 0;
static double freq2 = 220;



static void callback(void* userdata, Uint8* stream, int len) {
	int16_t* snd = (int16_t*)(stream);
	for (int i = 0; i < len / 2; i++) {
		snd[i] = (int16_t)(5000 * (sin(my_time) + sin(my_time2)));
		
		my_time += freq * tau / 44100.0;
		if (my_time >= tau) my_time -= tau;

		my_time2 += freq2 * tau / 44100.0;
		if (my_time2 >= tau) my_time2 -= tau;
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

	unsigned int id = 0;
	if (not (id = SDL_OpenAudioDevice(NULL, 0, &spec, &aspec, SDL_AUDIO_ALLOW_ANY_CHANGE))) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError()); exit(-1);
	}

	SDL_PauseAudioDevice(id, 0);

	
	SDL_Window *window = SDL_CreateWindow("block game", 0, 0, 0, 0, 0);

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

		if (key[SDL_SCANCODE_A]) { freq = 440; freq2 = 261.6256; }
	          
		if (key[SDL_SCANCODE_W]) { freq = 440; freq2 = 220; }

		if (key[SDL_SCANCODE_0]) { freq = 0; freq2 = 0; }

		if (key[SDL_SCANCODE_E]) { freq = 0; }
		if (key[SDL_SCANCODE_F]) { freq2 = 0; }

          //if needed, you can do cool stuff here, like change frequency for different notes: 
          //https://en.wikipedia.org/wiki/Piano_key_frequencies

	}
}






