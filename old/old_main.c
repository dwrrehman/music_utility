

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




