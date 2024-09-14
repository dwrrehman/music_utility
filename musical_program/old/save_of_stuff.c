// example sine wave gneration in 
// sdl2 in c.
// dwrr
// written on 1202407276.155019
// 202407276.212254: a music sine synthesizer
/*
copyb insert ./build
copya do ,./run




*/


#include <SDL.h>

#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <termios.h>

enum note_state { sustained, rising, falling };

static const int sample_count = 4096;
static const int sample_rate = 44100;

static const float f_sample_rate = 2.0f * (float) sample_rate;
static const float tau = (float) (2.0 * M_PI);

static const float start_vol = 0.00001f;
static const float cut_off_vol = 0.00001f;
static const float max_vol = 0.05f;
static const float start_speed = 0.0001f;
static const float decay_speed = 0.00005f;

static const int note_count = 3;
static const float freq[note_count] = {
	440.000f, 329.628f, 277.183f, 
};

struct note {
	float step;
	float volume;
};

static struct note notes[note_count] = {0};
static uint8_t state[note_count] = {0};
static struct termios terminal = {0};

static void oscillator_callback(
	void* userdata, 
	Uint8* stream, int len
) {
	float* output = (float*) stream;

	for (int _i = 0; _i < sample_count * 2; _i++) {
		for (int n = 0; n < note_count; n++) {

			//if (notes[n].volume <= 0.0f) continue;

			output[_i] += sinf(notes[n].step) * notes[n].volume;
			notes[n].step = fmodf(notes[n].step + 
					tau / (f_sample_rate / (freq[n])), tau);

			if (state[n] == rising) {

				if (notes[n].volume > cut_off_vol) 
					notes[n].volume *= (1 - decay_speed);
				else { 
					state[n] = 0; 
					notes[n].volume = 0.0f; 
				}

			} else if (state[n] == falling) {

				if (notes[n].volume < max_vol) { 
					if (notes[n].volume < start_vol) 
						notes[n].volume += start_vol; 
					notes[n].volume *= (1 + start_speed); 
				} else { 
					state[n] = 0; 
					notes[n].volume = max_vol; 
				}
			}
		}
	}
}

int main(void) {

	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		printf("SDL: %s\n", 
		SDL_GetError());
		exit(0);
	}

	SDL_AudioSpec spec = {
		.format = AUDIO_F32,
		.channels = 2,
		.freq = sample_rate,
		.samples = sample_count,
		.callback = oscillator_callback,
	};

	if (SDL_OpenAudio(&spec, NULL) < 0) {
		printf("%s\n", SDL_GetError());
		exit(1);
	}

	tcgetattr(0, &terminal);
	struct termios terminal_copy = terminal; 
	terminal_copy.c_cc[VMIN] = 0; 
	terminal_copy.c_cc[VTIME] = 0;
	terminal_copy.c_lflag &= ~((size_t) ICANON);
	tcsetattr(0, TCSAFLUSH, &terminal_copy);

	SDL_PauseAudio(0);

	while (1) {

		char c = 0;
		read(0, &c, 1);
		if (c == 'q') break;

		if (c == 't') state[0] = rising;
		if (c == 'n') state[0] = falling;

		if (c == 'h') state[1] = rising;
		if (c == 'e') state[1] = falling;

		if (c == 'r') state[2] = rising;
		if (c == 'u') state[2] = falling;

		usleep(10000);
	}
	SDL_PauseAudio(1);
	exit(0);
}
























//	struct sigaction action2 = {.sa_handler = interrupted}; 
//	sigaction(SIGINT, &action2, NULL);













/*


		if (turning_off0) {
			if (_0.volume > cut_off_vol) _0.volume *= (1 - decay_speed);
			else { turning_off0 = 0; _0.volume = 0.0f; }
		} else if (turning_on0) {
			if (_0.volume < max_vol) { if (_0.volume < start_vol) _0.volume += start_vol; _0.volume *= (1 + start_speed); }
			else { turning_on0 = 0; _0.volume = max_vol; }
		}

		if (turning_off1) {
			if (_1.volume > cut_off_vol) _1.volume *= (1 - decay_speed);
			else { turning_off1 = 0; _1.volume = 0.0f; }
		} else if (turning_on1) {
			if (_1.volume < max_vol) { if (_1.volume < start_vol) _1.volume += start_vol; _1.volume *= (1 + start_speed); }
			else { turning_on1 = 0; _1.volume = max_vol; }
		}

		if (turning_off2) {
			if (_2.volume > cut_off_vol) _2.volume *= (1 - decay_speed);
			else { turning_off2 = 0; _2.volume = 0.0f; }
		} else if (turning_on2) {
			if (_2.volume < max_vol) { if (_2.volume < start_vol) _2.volume += start_vol; _2.volume *= (1 + start_speed); }
			else { turning_on2 = 0; _2.volume = max_vol; }
		}


*/





//static const float  A4_OSC = 2 * (float) sample_rate / (440.000f);
//static const float  E4_OSC = 2 * (float) sample_rate / (329.628f);
//static const float Cs4_OSC = 2 * (float) sample_rate / (277.183f);



