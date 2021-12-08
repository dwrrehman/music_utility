
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








/*
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include <SDL2/SDL.h>

#define tau (2.0 * M_PI)


static void callback(void* userdata, uint8_t* stream, int len) {
	int16_t* buffer = (int16_t*)stream;
	for (long i = 0; i < len / 2; i++) {

		 const int16_t sample = 

			(int16_t) (5000 * (

				  sin(tau * 440.0000 * ((double)i / (double)44100)) 

				// + sin(tau * 329.6276 * ((double)i / (double)44100))

				d// + sin(tau * 261.6256 * ((double)i / (double)44100))

			));

		buffer[i] = sample;
		
		// time += freq * tau / 44100.0;
		// if(time >= tau) time -= tau;
	}
}



int main(void) {

	SDL_Init(SDL_INIT_AUDIO);

	SDL_AudioDeviceID device;
	SDL_AudioSpec audio_spec;
	SDL_zero(audio_spec);

	audio_spec.freq = 44100;
	audio_spec.format = AUDIO_S16SYS;
	audio_spec.channels = 1;
	audio_spec.samples = 4096;
	audio_spec.callback = callback;


	device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);


	bool a = 0, c = 0, e = 0;

	bool quit = false;

	

	// while (not quit) {

	// //     for (long i = 0; i < 44100 * 10; i++) {
	 
	// //         const int16_t sample = 

	// // 		(int16_t) (5000 * (

	// // 			  a * sin(2.0 * M_PI * 440.0000 * ((double)i / (double)44100)) 

	// // 			+ e * sin(2.0 * M_PI * 329.6276 * ((double)i / (double)44100))

	// // 			+ c * sin(2.0 * M_PI * 261.6256 * ((double)i / (double)44100))

	// // 		));
	// // 					// (sin(x) + sin(y) + sin(z)) * 5000;
	// //         SDL_QueueAudio(device, &sample, 2);
	// // 	SDL_PauseAudioDevice(device, 0);
	// //     }

	    


	// 	int ch = getchar();
	// 	if (ch == 'q') quit = true;
	// 	else if (ch == 'a') a = not a;
	// 	else if (ch == 'c') c = not c;
	// 	else if (ch == 'e') e = not e;
	// }


    SDL_Delay(10000);

    SDL_CloseAudioDevice(device);
    SDL_Quit();

    return 0;
}

*/





/*

#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include <SDL2/SDL.h>

#include "audio.h"

int main() {

	srand((unsigned)time(0));

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) 
		fprintf(stderr, "error: could not initialize SDL2: %s\n", SDL_GetError());
	
	initAudio();

	

	bool quit = false;

	while (not quit) {

		int c = getchar();
		if (c == 'q') quit = true;
		else if (c == 'p') playSound("notes/C.wav", SDL_MIX_MAXVOLUME / 2);
		playSound
	}


	endAudio();
	SDL_CloseAudio();
	SDL_Quit();
}

*/




