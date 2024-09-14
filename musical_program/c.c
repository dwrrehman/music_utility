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

#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>


typedef uint64_t nat;

enum note_state { sustained, rising, falling };
static const int sample_count = 4096;
static const int sample_rate = 44100;
static const float f_sample_rate = 2.0f * (float) sample_rate;
static const float tau = (float) (2.0 * M_PI);
static const float start_vol = 0.00005f;
static const float cut_off_vol = 0.00001f;
static const float max_vol = 0.06f;
static const float start_speed = 0.001f;
static const float decay_speed = 0.00002f;

enum note_names { 
	C0, Db0, D0, Eb0, E0, F0, Gb0, G0, Ab0, A0, Bb0, B0_,
	C1, Db1, D1, Eb1, E1, F1, Gb1, G1, Ab1, A1, Bb1, B1,
	C2, Db2, D2, Eb2, E2, F2, Gb2, G2, Ab2, A2, Bb2, B2,
	C3, Db3, D3, Eb3, E3, F3, Gb3, G3, Ab3, A3, Bb3, B3,
	C4, Db4, D4, Eb4, E4, F4, Gb4, G4, Ab4, A4, Bb4, B4,
	C5, Db5, D5, Eb5, E5, F5, Gb5, G5, Ab5, A5, Bb5, B5,
	C6, Db6, D6, Eb6, E6, F6, Gb6, G6, Ab6, A6, Bb6, B6,
	C7, Db7, D7, Eb7, E7, F7, Gb7, G7, Ab7, A7, Bb7, B7,
	C8, Db8, D8, Eb8, E8, F8, Gb8, G8, Ab8, A8, Bb8, B8,
	C9, Db9, D9, Eb9, E9, F9, Gb9, G9, Ab9, A9, Bb9, B9,
note_count
};
static const float freq[note_count] = {
	// harmonic series:
	//110, 220, 330, 440,  550, 660, 770, 880,  990, 1100, 1210, 1320, 
	//13 * 110, 14 * 110, 15 * 110, 16 * 110,  17 * 110, 18 * 110, 19 * 110, 20 * 110,  21 * 110, 22 * 110, 23 * 110, 24 * 110, 

	// equal temperment:

	16.35f, 17.32f, 18.35f, 19.45f, 20.60f, 21.83f, 23.12f, 24.50f, 25.96f, 27.50f, 29.14f, 30.87f,	
	32.70f, 34.65f, 36.71f, 38.89f, 41.20f, 43.65f, 46.25f, 49.00f, 51.91f, 55.00f, 58.27f, 61.74f,
	65.41f, 69.30f, 73.42f, 77.78f, 82.41f, 87.31f, 92.50f, 98.00f, 103.8f, 110.0f, 116.5f, 123.5f, 
	130.8f, 138.6f, 146.8f, 155.6f, 164.8f, 174.6f, 185.0f, 196.0f, 207.7f, 220.0f, 233.1f, 246.9f, 
	261.626f, 277.183f, 293.665f, 311.184f, 329.628f, 349.228f, 369.994f, 391.995f, 415.305f, 440.000f, 466.164f, 493.883f,
	523.3f, 554.4f, 587.3f, 622.3f, 659.3f, 698.5f, 740.0f, 784.0f, 830.6f, 880.0f, 932.3f, 987.8f,
	1047.0f, 1109.0f, 1175.0f, 1245.0f, 1319.0f, 1397.0f, 1480.0f, 1568.0f, 1661.0f, 1760.0f, 1865.0f, 1976.0f, 
	2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951, 
	4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902, 
	8372.019f, 8869.845f, 9397.273f, 9956.064f, 10548.083f, 11175.305f, 11839.823f, 12543.855f, 13289.752f, 14080.0f, 14080.0f, 14080.0f, 
};

struct note {
	float step;
	float volume;
};

static struct note notes[note_count] = {0};
static _Atomic nat state[note_count] = {0};

static struct termios terminal = {0};

static void oscillator_callback(
	void* userdata, 
	Uint8* stream, int len
) {
	float* output = (float*) stream;
	
	for (int o = 0; o < sample_count * 2; o++) {

		output[o] = 0.0;
		for (int n = 0; n < note_count; n++) {
			//if (notes[n].volume > 0.0f) {
				output[o] += sinf(notes[n].step) * notes[n].volume;
				notes[n].step = fmodf(notes[n].step + tau / (f_sample_rate / (2 * freq[n])), tau);
			//}

			if (state[n] == falling) {

				if (notes[n].volume > cut_off_vol) 
					notes[n].volume *= (1 - decay_speed);
				else { 
					state[n] = sustained; 
					notes[n].volume = 0.0f; 
				}

			} else if (state[n] == rising) {

				if (notes[n].volume < max_vol) { 
					if (notes[n].volume < start_vol) 
						notes[n].volume += start_vol; 
					notes[n].volume *= (1 + start_speed); 
				} else { 
					state[n] = sustained; 
					notes[n].volume = max_vol; 
				}
			}
		}
	}
}

static void handler(int unused) {
	puts("stopping program.");
	exit(1);
}

int main(void) {

	struct sigaction action = {.sa_handler = &handler };
	sigaction(SIGINT, &action, NULL);

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

	SDL_PauseAudio(0);
	usleep(1000000);

	const nat fifth = (G4 - C4);
	const nat fourth = (F4 - C4);
	const nat major_third = (E4 - C4);
	const nat major_second = (D4 - C4);

	const nat max_range = 24;
	const nat timesteps = 1000;
	nat voices[2] = {0};


	const nat base = C3;



	//printf("fourth = %llu\n", fourth);
	//printf("fifth = %llu\n", fifth);
	//printf("turning on note C4 + %llu\n", (voices[0] + fifth) % max_range);

	for (nat timestep = 0; timestep < timesteps; timestep += 1) {
		if (timestep % 5 == 0) {
			state[base + ((voices[0] + fifth) % max_range)] = rising;
			state[base + ((voices[0]) % max_range)] = falling;
			voices[0] += fifth;
		}
		if (timestep % 3 == 0) {
			state[base + ((voices[1] + fourth) % max_range)] = rising;
			state[base + ((voices[1]) % max_range)] = falling;
			voices[1] += fourth;
		}
		if (timestep % 11 == 0) {
			state[base + ((voices[1] + major_third) % max_range)] = rising;
			state[base + ((voices[1]) % max_range)] = falling;
			voices[1] += major_third;
		}
		if (timestep % 7 == 0) {
			state[base + ((voices[1] + major_second) % max_range)] = rising;
			state[base + ((voices[1]) % max_range)] = falling;
			voices[1] += major_second;
		}
		usleep(400000);
	}

	for (nat i = 0; i < note_count; i++) state[i] = falling;
	usleep(3000000);

	SDL_PauseAudio(1);
}








































/*
		if (melody_index < melody_count) {
			     if (counter == 0) { state[melody[melody_index]] = rising; counter++; }
			else if (counter ==  15000) { if (melody_index) state[melody[melody_index - 1]] = falling; counter++; }
			else if (counter >=  30000) { melody_index++; counter = 0; }
			else counter++;

		} else if (melody_index == melody_count) {
			     if (counter == 0) { counter++; }
			else if (counter ==  15000) { if (melody_index) state[melody[melody_index - 1]] = falling; counter++; }
			else if (counter >=  30000) { melody_index++; counter = 0; }
			else counter++;
		}








static const int melody[] = {
	D4, A4, F4, D4, A4, F4, D4, A4, F4, D4, A4, F4, 
	D4, Bb4, G4, D4, Bb4, G4, D4, Bb4, G4, D4, Bb4, G4, 
	Db4, G4, E4, Db4, G4, E4, Db4, Bb4, G4, E4, A4, G4, 
	F4, D4, F4, A4, F4, A4, D5, A4, D5, F5, D5, F5, 
	B5, F5, E5, D5, C5, B4, A4, Ab4, Gb4, E4, D5, B4, 
	C5, C5, 
};
const int melody_count = sizeof melody / sizeof *melody;

static int melody_index = melody_count;
static int counter = 0;








	//const char key_rising[16]  = "1234567890-=ashtgyneoi";
	//const char key_falling[16] = "qdrwbjfup;[]zxmcvkl,./";


tcgetattr(0, &terminal);
	struct termios terminal_copy = terminal; 
	terminal_copy.c_cc[VMIN] = 0; 
	terminal_copy.c_cc[VTIME] = 0;
	terminal_copy.c_lflag &= ~((size_t) (ICANON | ECHO));
	tcsetattr(0, TCSAFLUSH, &terminal_copy);


		char c = 0; read(0, &c, 1);
		if (c == 'Q') break;
		for (int i = 0; i < 16; i++) {
			if (c == key_rising[i])  state[C4 + i] = rising;
			if (c == key_falling[i]) state[C4 + i] = falling;
		}
*/



















































/*



const int melody[] =   {C4,
		D4, A4, F4, D4, A4, F4, D4, A4, F4, D4, A4, F4, 
		D4, Bb4, G4, D4, Bb4, G4, D4, Bb4, G4, D4, Bb4, G4, 
		Db4, G4, E4, Db4, G4, E4, Db4, Bb4, G4, E4, A4, G4, 
		F4, D4, F4, A4, F4, A4, D5, A4, D5, F5, D5, F5, 
		B5, F5, E5, D5, C5, B4, A4, Ab4, Gb4, E4, D5, B4, 
		C5

	};

	//const int duration[] = { 1,   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

	const int melody_count = sizeof melody / sizeof *melody;

	for (int i = 0; i < melody_count - 1; i++) {

		state[melody[i + 1]] = rising;

		for (int d = 0; d < 1; d++) { // duration[i + 1]
			nanosleep((const struct timespec[]){{0, 2 * 40000000}}, NULL);
		}

		state[melody[i]] = falling;
		nanosleep((const struct timespec[]){{0, 2 * 150000000}}, NULL);
	}


	for (int i = 0; i < note_count; i++) state[i] = falling;
	nanosleep((const struct timespec[]){{2, 0}}, NULL);
	exit(0);




*/













































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









/*
	for (int i = 0; i < sample_count * 2; i++) {

		const float f0 = sinf(notes[0].step) * notes[0].volume;
		const float f1 = sinf(notes[1].step) * notes[1].volume;
		const float f2 = sinf(notes[2].step) * notes[2].volume;

		output[i] = f0 + f1 + f2;

		notes[0].step = fmodf(notes[0].step + (2 * M_PI) / (2 * (float) sample_rate / (freq[0])), 2 * M_PI);
		notes[1].step = fmodf(notes[1].step + (2 * M_PI) / (2 * (float) sample_rate / (freq[1])), 2 * M_PI);
		notes[2].step = fmodf(notes[2].step + (2 * M_PI) / (2 * (float) sample_rate / (freq[2])), 2 * M_PI);

		if (state[0] == falling) {
			if (notes[0].volume > cut_off_vol) notes[0].volume *= (1 - decay_speed);
			else { state[0] = sustained; notes[0].volume = 0.0f; }
		} else if (state[0] == rising) {
			if (notes[0].volume < max_vol) { if (notes[0].volume < start_vol) notes[0].volume += start_vol; notes[0].volume *= (1 + start_speed); }
			else { state[0] = 0; notes[0].volume = max_vol; }
		}

		if (state[1] == falling) {
			if (notes[1].volume > cut_off_vol) notes[1].volume *= (1 - decay_speed);
			else { state[1] = sustained; notes[1].volume = 0.0f; }
		} else if (state[1] == rising) {
			if (notes[1].volume < max_vol) { if (notes[1].volume < start_vol) notes[1].volume += start_vol; notes[1].volume *= (1 + start_speed); }
			else { state[1] = 0; notes[1].volume = max_vol; }
		}

		if (state[2] == falling) {
			if (notes[2].volume > cut_off_vol) notes[2].volume *= (1 - decay_speed);
			else { state[2] = sustained; notes[2].volume = 0.0f; }
		} else if (state[2] == rising) {
			if (notes[2].volume < max_vol) { if (notes[2].volume < start_vol) notes[2].volume += start_vol; notes[2].volume *= (1 + start_speed); }
			else { state[2] = 0; notes[2].volume = max_vol; }
		}
	}

*/








//277.183f, 329.628f, 440.000f, 




//const int duration[] = { 1,   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };



/*

//nanosleep((const struct timespec[]){{0, 2 * 150000000}}, NULL);
	//nanosleep((const struct timespec[]){{0, 2 * 40000000}}, NULL);
	//nanosleep((const struct timespec[]){{2, 0}}, NULL);
	//exit(0);



enum note_names { 
	C4, Db4, D4, Eb4, E4, F4, Gb4, G4, Ab4, A4, Bb4, B4,
	C5, Db5, D5, Eb5, E5, F5, Gb5, G5, Ab5, A5, Bb5, B5,
note_count
};
static const float freq[note_count] = {
	// harmonic series:
	//110, 220, 330, 440,  550, 660, 770, 880,  990, 1100, 1210, 1320, 
	//13 * 110, 14 * 110, 15 * 110, 16 * 110,  17 * 110, 18 * 110, 19 * 110, 20 * 110,  21 * 110, 22 * 110, 23 * 110, 24 * 110, 
	// equal temperment:
	261.626f, 277.183f, 293.665f, 311.184f, 329.628f, 349.228f, 369.994f, 391.995f, 415.305f, 440.000f, 466.164f, 493.883f,
	2 * 261.626f, 2 * 277.183f, 2 * 293.665f, 2 * 311.184f, 2 * 329.628f, 2 * 349.228f, 2 * 369.994f, 2 * 391.995f, 2 * 415.305f, 2 * 440.000f, 2 * 466.164f, 2 * 493.883f,
};



*/





