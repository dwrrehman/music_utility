// a mpe midi keyboard/sythesizer program written in c that uses my sine wave generation
// that uses sdl to play pure tones for music synthesis. 
// written on 202408165.113331 by dwrr.

// example sine wave gneration in 
// sdl2 in c.
// dwrr
// written on 1202407276.155019
// 202407276.212254: a music sine synthesizer

#include <SDL.h>
#include <CoreMIDI/MIDIServices.h>
#include <CoreFoundation/CFRunLoop.h>

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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>

typedef uint64_t nat;
typedef uint8_t byte;

static MIDIPortRef gOutPort = 0;
static MIDIEndpointRef gDest = 0;

enum midi_packet_type { null_packet, note_off, note_on, poly_pressure, control_change, program_change, channel_pressure, pitch_bend};

enum note_state { sustained, rising, falling };
static const int sample_count = 4096;
static const int sample_rate = 44100;
static const float f_sample_rate = 2.0f * (float) sample_rate;
static const float tau = (float) (2.0 * M_PI);
static const float start_vol = 0.00005f;
static const float cut_off_vol = 0.00001f;
static const float max_vol = 0.05f;
static const float start_speed = 0.003f;
static const float decay_speed = 0.00004f;

enum note_names { 
	C0, Db0, D0, Eb0, E0, F0, Gb0, G0, Ab0, A0, Bb0, B0,
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
static _Atomic nat state[note_count];

static const int melody[] = {
	D4, A4, F4, D4, A4, F4, D4, A4, F4, D4, A4, F4, 
	D4, Bb4, G4, D4, Bb4, G4, D4, Bb4, G4, D4, Bb4, G4, 
	Db4, G4, E4, Db4, G4, E4, Db4, Bb4, G4, E4, A4, G4, 
	F4, D4, F4, A4, F4, A4, D5, A4, D5, F5, D5, F5, 
	B5, F5, E5, D5, C5, B4, A4, Ab4, Gb4, E4, D5, B4, 
	C5, C5, 
};
static const int melody_count = sizeof melody / sizeof *melody;

static int melody_index = melody_count;
static int counter = 0;


#define debug 1


#include <stdatomic.h>

static void oscillator_callback(
	void* userdata, 
	Uint8* stream, int len
) {
	float* output = (float*) stream;
	
	for (int o = 0; o < sample_count * 2; o++) {

		if (melody_index < melody_count) { 
			if (counter == 0) { 
					atomic_store_explicit(state + melody[melody_index], rising, memory_order_relaxed); 
					counter++; 
			}
			else if (counter ==  15000) { 
				if (melody_index) { 
					atomic_store_explicit(state + melody[melody_index - 1], falling, memory_order_relaxed); 
				} 
				counter++; 
			}
			else if (counter >= 30000) { melody_index++; counter = 0; }
			else counter++;

		} else if (melody_index == melody_count) {
			     if (counter == 0) { counter++; }
			else if (counter ==  15000) { 
				if (melody_index) { 
					atomic_store_explicit(state + melody[melody_index - 1], falling, memory_order_relaxed); 
				} 
				counter++; 
			}
			else if (counter >= 30000) { melody_index++; counter = 0; }
			else counter++;
		}

		output[o] = 0.0;
		for (int n = 0; n < note_count; n++) {
			//if (notes[n].volume > 0.0f) {
				output[o] += sinf(notes[n].step) * notes[n].volume;
				notes[n].step = fmodf(notes[n].step + tau / (f_sample_rate / (freq[n])), tau);
			//}

			const byte s = atomic_load_explicit(state + n, memory_order_relaxed); //memory_order_relaxed);

			if (s == falling) {

				if (notes[n].volume > cut_off_vol) 
					notes[n].volume *= (1 - decay_speed);
				else { 
					//state[n] = sustained; 
					atomic_store_explicit(state + n, sustained, memory_order_relaxed); //memory_order_relaxed);
					notes[n].volume = 0.0f; 
				}

			} else if (s == rising) {

				if (notes[n].volume < max_vol) { 
					if (notes[n].volume < start_vol) 
						notes[n].volume += start_vol; 
					notes[n].volume *= (1 + start_speed); 
				} else { 
					//state[n] = sustained; 
					atomic_store_explicit(state + n, sustained, memory_order_relaxed); //memory_order_relaxed);
					notes[n].volume = max_vol; 
				}
			}
		}
	}
}





static void midi_packet_handler(const MIDIPacketList* pktlist, void* refCon, void* connRefCon) {

	const MIDIPacket* packet = pktlist->packet;

	size_t type = 0, index = 0, bend = 0, channel = 0, note = 0;

	for (size_t j = 0; j < pktlist->numPackets; j++) {
		for (size_t i = 0; i < packet->length; i++) {


			const byte b = packet->data[i];

			if ((b >> 7) == 1) {

				channel = (size_t) (b & 0xF);

				if (debug) printf("\n");
				index = 0;

				if ((b >> 4) == 0x8) {
					if (debug) printf("(\033[33;1mNOTE_OFF\033[m)(channel=%lu) ", channel);
					type = note_off;

				} else if ((b >> 4) == 0x9) {
					if (debug) printf("(\033[32;1mNOTE_ON\033[m)(channel=%lu) ", channel);
					type = note_on;

				} else if ((b >> 4) == 0xA) {
					if (debug) printf("(\033[36;1mPOLY_PRESSURE\033[m)(channel=%lu) ", channel);
					type = poly_pressure;

				} else if ((b >> 4) == 0xB) {
					if (debug) printf("(\033[31;1mCONTROL_CHANGE\033[m)(channel=%lu) ", channel);
					type = control_change;

				} else if ((b >> 4) == 0xC) {
					if (debug) printf("(\033[31;1mPROGRAM_CHANGE\033[m)(channel=%lu) ", channel);
					type = program_change;

				} else if ((b >> 4) == 0xD) {
					if (debug) printf("(\033[35;1mCHANNEL_PRESSURE\033[m)(channel=%lu) ", channel);
					type = channel_pressure;

				} else if ((b >> 4) == 0xE) {
					if (debug) printf("(\033[34;1mPITCH_BEND\033[m)(channel=%lu) ", channel);
					type = pitch_bend;
					bend = 0;

				} else {
					printf("STATUS:%02X ", b);
				}

			} else {
				if (type == note_off) {

					if (index == 0) { 
						if (debug) printf("note#:%d ", b); 
						index = 1; note = b;

					} else if (index == 1) { 
						if (debug) printf("velocity:%d ", b); 
						index = 0;
						if (note < 127) {
							atomic_store_explicit(state + note, falling, memory_order_relaxed);
						} else {
							printf("ERROR: note = %lu\n", note);
						}
						if (debug) puts("");

					} else abort();

				} else if (type == note_on) {
					if (index == 0) { 
						if (debug) printf("note#:%d ", b); 
						index = 1; note = b;

					} else if (index == 1) { 
						if (debug) printf("velocity:%d ", b); 
						index = 0; 
						if (note < 127) {
							atomic_store_explicit(state + note, rising, memory_order_relaxed);
						} else {
							printf("ERROR: note = %lu\n", note);
						}
						if (debug) puts(""); 

					} else abort();

				} else if (type == control_change) {
					if (index == 0) { 
						if (debug) printf("controller:%d ", b); 
						index = 1; 
					}
					else if (index == 1) { 
						if (debug) printf("value:%d ", b); 
						index = 0; 
					}
					else abort();

				} else if (type == pitch_bend) {
					if (index == 0) { bend |= (size_t) b; index = 1; }
					else if (index == 1) { 
						bend |= (size_t) b << 7LU; 
						if (debug) printf(". BEND:%04hX ", (signed short) (bend - 8192)); 
						index = 0; }
					else abort();

				} else if (type == channel_pressure) {
					if (index == 0) { 
						if (debug) printf("pressure:%d ", b); 
						index = 0; 
					}
					else abort();

				} else if (type == poly_pressure) {
					if (index == 0) { 
						if (debug) printf("note#:%d ", b); 
						index = 1; 
					}
					else if (index == 1) { 
						if (debug) printf("pressure:%d ", b); 
						index = 0; 
					}
					else abort();

				} else { 
					printf("DATA:%02X ", b);
				}
			}
		}
		packet = MIDIPacketNext(packet);
	}
}

int main(void) { 

	for (int n = 0; n < note_count; n++) {
		atomic_init(state + n, 0);
	}	

	//if (argc >= 2) {
		// first argument, if present, is the MIDI channel number to echo to (1-16)
		//sscanf(argv[1], "%ld", &channel);
		//if (channel < 1) channel = 1;
		//else if (channel > 16) channel = 16;
		//channel--; // convert to 0-15
	//}

	MIDIClientRef client = 0;
	MIDIClientCreate(CFSTR("MIDI Echo"), 0, 0, &client);

	MIDIPortRef inPort = 0;
	MIDIInputPortCreate(client, CFSTR("Input port"), midi_packet_handler, 0, &inPort);
	MIDIOutputPortCreate(client, CFSTR("Output port"), &gOutPort);

	CFStringRef pname, pmanuf, pmodel;
	char name[64], manuf[64], model[64];

	const size_t device_count = MIDIGetNumberOfDevices();
	for (size_t i = 0; i < device_count; i++) {
		MIDIDeviceRef dev = MIDIGetDevice(i);

		MIDIObjectGetStringProperty(dev, kMIDIPropertyName, &pname);
		MIDIObjectGetStringProperty(dev, kMIDIPropertyManufacturer, &pmanuf);
		MIDIObjectGetStringProperty(dev, kMIDIPropertyModel, &pmodel);

		CFStringGetCString(pname, name, sizeof(name), 0);
		CFStringGetCString(pmanuf, manuf, sizeof(manuf), 0);
		CFStringGetCString(pmodel, model, sizeof(model), 0);
		CFRelease(pname);
		CFRelease(pmanuf);
		CFRelease(pmodel);

		printf("name=%s, manuf=%s, model=%s\n", name, manuf, model);
	}

	const size_t source_count = MIDIGetNumberOfSources();
	printf("%lu sources\n", source_count);
	for (size_t i = 0; i < source_count; i++) {
		MIDIEndpointRef src = MIDIGetSource(i);
		MIDIPortConnectSource(inPort, src, NULL);
	}

	const size_t destination_count = MIDIGetNumberOfDestinations();
	if (destination_count > 0)
		gDest = MIDIGetDestination(0);

	if (gDest) {
		MIDIObjectGetStringProperty(gDest, kMIDIPropertyName, &pname);
		CFStringGetCString(pname, name, sizeof(name), 0);
		CFRelease(pname);
		printf("Echoing to channel %lu of %s\n", 42LU, name);
	} else {
		printf("No MIDI destinations present\n");
	}

	struct sigaction action = {0};
	sigaction(SIGINT, NULL, &action);
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		printf("SDL: %s\n", 
		SDL_GetError());
		exit(0);
	}
	sigaction(SIGINT, &action, NULL);

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
 	CFRunLoopRun();
}




























































	/*struct termios terminal = {0};
	tcgetattr(0, &terminal);
	struct termios terminal_copy = terminal; 
	terminal_copy.c_cc[VMIN] = 0; 
	terminal_copy.c_cc[VTIME] = 0;
	terminal_copy.c_lflag &= ~((size_t) (ICANON | ECHO));
	tcsetattr(0, TCSAFLUSH, &terminal_copy);*/




/*
	const char key_rising[note_count]  = "1234567890-=ashtgyneoi";
	const char key_falling[note_count] = "qdrwbjfup;[]zxmcvkl,./";

	while (1) {
		char c = 0;
		read(0, &c, 1);

		if (c == 'Q') break;

		for (int i = 0; i < note_count; i++) {
			if (c == key_rising[i])  atomic_store_explicit(state + i, rising, memory_order_relaxed); //memory_order_relaxed);
			if (c == key_falling[i]) atomic_store_explicit(state + i, falling, memory_order_relaxed); //memory_order_relaxed);
		}

		usleep(10000);
	}
	SDL_PauseAudio(1);
	exit(0);

*/






































//size_t n = (note - 0);
							//state[n] = falling;
							//printf("atomic_store_explicit(state + note, falling)\n");

















	//if (gOutPort != NULL && gDest != NULL) {
//if (packet->data[i] >= 0x80 && packet->data[i] < 0xF0) // if status message:
			//packet->data[i] = (packet->data[i] & 0xF0) | gChannel; // set channel...? no. 
//MIDISend(gOutPort, gDest, pktlist);
	//}







/*



copyb insert ./build
copya do ,./run




enum note_state { sustained, rising, falling };
static const int sample_count = 4096;
static const int sample_rate = 44100;
static const float f_sample_rate = 2.0f * (float) sample_rate;
static const float tau = (float) (2.0 * M_PI);
static const float start_vol = 0.00005f;
static const float cut_off_vol = 0.00001f;
static const float max_vol = 0.05f;
static const float start_speed = 0.003f;
static const float decay_speed = 0.00002f;

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

struct note {
	float step;
	float volume;
};

static struct note notes[note_count] = {0};
static uint8_t state[note_count] = {0};

static struct termios terminal = {0};

static const int melody[] = {
	D4, A4, F4, D4, A4, F4, D4, A4, F4, D4, A4, F4, 
	D4, Bb4, G4, D4, Bb4, G4, D4, Bb4, G4, D4, Bb4, G4, 
	Db4, G4, E4, Db4, G4, E4, Db4, Bb4, G4, E4, A4, G4, 
	F4, D4, F4, A4, F4, A4, D5, A4, D5, F5, D5, F5, 
	B5, F5, E5, D5, C5, B4, A4, Ab4, Gb4, E4, D5, B4, 
	C5, C5, 
};
const int melody_count = sizeof melody / sizeof *melody;

static int melody_index = 0; // melody_count;
static int counter = 0;

static void oscillator_callback(
	void* userdata, 
	Uint8* stream, int len
) {
	float* output = (float*) stream;
	
	for (int o = 0; o < sample_count * 2; o++) {

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
	terminal_copy.c_lflag &= ~((size_t) (ICANON | ECHO));
	tcsetattr(0, TCSAFLUSH, &terminal_copy);

	SDL_PauseAudio(0);

	const char key_rising[note_count]  = "1234567890-=ashtgyneoi";
	const char key_falling[note_count] = "qdrwbjfup;[]zxmcvkl,./";

	while (1) {
		char c = 0;
		read(0, &c, 1);

		if (c == 'Q') break;

		for (int i = 0; i < note_count; i++) {
			if (c == key_rising[i])  state[i] = rising;
			if (c == key_falling[i]) state[i] = falling;
		}

		usleep(10000);
	}




	//SDL_PauseAudio(1);



	//exit(0);
}



*/












