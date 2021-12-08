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

enum modes {
	keyboard_mode,
	training_mode,
	memory_mode,
};

enum note {

	C4,
	Db4,
	D4,
	Eb4,
	E4,
	F4,
	Gb4,
	G4,
	Ab4,
	A4,
	Bb4,
	B4,
	C5,
	Db5,
	D5,
	Eb5,
	E5,
	F5,
	Gb5,
	G5,

	note_count,
};

static const char* note_spelling[] = {
	"C4",
	"Db4",
	"D4",
	"Eb4",
	"E4",
	"F4",
	"Gb4",
	"G4",
	"Ab4",
	"A4",
	"Bb4",
	"B4",
	"C5",
	"Db5",
	"D5",
	"Eb5",
	"E5",
	"F5",
	"Gb5",
	"G5",
};

static const double frequency[] = {
	261.6256, // C4
	277.1826, // Db4
	293.6648, // D4
	311.1270, // Eb4
	329.6276, // E4
	349.2282, // F4
	369.9944, // Gb4
	391.9954, // G4
	415.3047, // Ab4
	440.0000, // A4
	466.1638, // Bb4
	493.8833, // B4
	523.2511, // C5
	554.3653, // Db5
	587.3295, // D5
	622.2540, // Eb5
	659.2551, // E5
	698.4565, // F5
	739.9888, // Gb5
	783.9909, // G5
};

static const int keybind[]= {
	SDL_SCANCODE_Z, // C4
	SDL_SCANCODE_X, // Db4
	SDL_SCANCODE_C, // D4
	SDL_SCANCODE_V, // Eb4
	SDL_SCANCODE_B, // E4
	SDL_SCANCODE_N, // F4
	SDL_SCANCODE_M, // Gb4 
	SDL_SCANCODE_A, // G4
	SDL_SCANCODE_S, // Ab4
	SDL_SCANCODE_D, // A4
	SDL_SCANCODE_F, // Bb4
	SDL_SCANCODE_G, // B4
	SDL_SCANCODE_H, // C5
	SDL_SCANCODE_J, // Db5
	SDL_SCANCODE_K, // D5
	SDL_SCANCODE_L, // Eb5
	SDL_SCANCODE_Q, // E5
	SDL_SCANCODE_W, // F5
	SDL_SCANCODE_E, // Gb5
	SDL_SCANCODE_R, // G5
};

static int active[note_count] = {0};
static double time_value[note_count] = {0};

static void callback(void* __attribute__((unused)) userdata, Uint8* stream, int len) {

	void* raw_buffer = stream;
	int16_t* buffer = (int16_t*)(raw_buffer);
	for (int i = 0; i < len / 2; i++) {

		// super-impose all sine waves together:
		double sum = 0.0;
		for (int n = 0; n < note_count; n++) 
			sum += sin(time_value[n]);
		
		// write value to buffer:
		buffer[i] = (int16_t)(3200.0 * (sum)); 
		// amplitude should be the largest K such that K * max_simuultaneously_active_note_count < 32,767.    max is 10 for now.

		// move time_value for each note forward based on frequency:
		for (int n = 0; n < note_count; n++) {
			time_value[n] += (double)active[n] * frequency[n] * tau / 44100.0;
			if (time_value[n] >= tau) time_value[n] -= tau;
		}
	}
}

static const char* interval_spelling(int diff) {
	if (diff == 0) return "Unison";

	if (diff == 1) return "Ascending Minor Second";
	if (diff == -1) return "Descending Minor Second";

	if (diff == 2) return "Ascending Major Second";
	if (diff == -2) return "Descending Major Second";

	if (diff == 3) return "Ascending Minor Third";
	if (diff == -3) return "Descending Minor Third";

	if (diff == 4) return "Ascending Major Third";
	if (diff == -4) return "Descending Major Third";

	if (diff == 5) return "Ascending Perfect Fourth";
	if (diff == -5) return "Descending Perfect Fourth";

	if (diff == 6) return "Ascending Tri-Tone";
	if (diff == -6) return "Descending Tri-Tone";

	if (diff == 7) return "Ascending Perfect Fifth";
	if (diff == -7) return "Descending Perfect Fifth";

	if (diff == 8) return "Ascending Minor Sixth";
	if (diff == -8) return "Descending Minor Sixth";

	if (diff == 9) return "Ascending Major Sixth";
	if (diff == -9) return "Descending Major Sixth";

	if (diff == 10) return "Ascending Minor Seventh";
	if (diff == -10) return "Descending Minor Seventh";

	if (diff == 11) return "Ascending Major Seventh";
	if (diff == -11) return "Descending Major Seventh";
		
	if (diff == 11) return "Ascending Perfect Octave";
	if (diff == -11) return "Descending Perfect Octave";

	return "Undefined Interval";
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

	bool quit = false, tab = false, escape = false;

	int mode = training_mode;

	int note1 = 0, note2 = 0;
	
	while (not quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);

			if (event.type == SDL_QUIT) quit = true;

			else if (event.type == SDL_KEYDOWN) {

				if (tab and key[SDL_SCANCODE_Q]) quit = true;

				if (tab and key[SDL_SCANCODE_H]) {
					printf("global: \n"
						"\ttab-Q : quit\n"
						"\tescape-1 : keyboard mode\n"
						"\tescape-2 : relative pitch interval training mode\n"
						"\tescape-3 : memory mode - unimplemented.\n"
						"\ntraining mode:\n"
						"\tj : reveal current interval textual answer\n"
						"\te : play current interval\n"
						"\tr : generate a new ascending-only interval\n"
						"\tf : generate random two notes\n"
						"\td : generate a new descending-only interval\n"
						"\n"
					);
				}

				if (escape and key[SDL_SCANCODE_1]) mode = keyboard_mode;
				if (escape and key[SDL_SCANCODE_2]) mode = training_mode;
				if (escape and key[SDL_SCANCODE_3]) mode = memory_mode;
			
				if (mode == training_mode and key[SDL_SCANCODE_R]) {
					note1 = (C4 + rand() % 12) % note_count;
					note2 = (note1 + rand() % 12) % note_count;
				}
				if (mode == training_mode and key[SDL_SCANCODE_F]) {
					note1 = rand() % note_count;
					note2 = rand() % note_count;
				}
				if (mode == training_mode and key[SDL_SCANCODE_D]) {
					note1 = (C4 + rand() % 12) % note_count;
					note2 = (note1 + rand() % 12) % note_count;
					int temp = note2; note2 = note1; note1 = temp;
				}
				
				if (mode == training_mode and key[SDL_SCANCODE_E]) {
					active[note1] = 1;
					SDL_Delay(1000);
					active[note1] = 0;

					active[note2] = 1;
					SDL_Delay(1000);
					active[note2] = 0;
				}
				
				if (mode == training_mode and key[SDL_SCANCODE_J]) { 
					printf("%s   :  note1 = %s, note2 = %s    [diff = %d]\n", 
						interval_spelling(note2 - note1), note_spelling[note1], note_spelling[note2], note2 - note1 );
				}
			}
		}

		const Uint8* key = SDL_GetKeyboardState(0);
		tab = !!key[SDL_SCANCODE_TAB];
		escape = !!key[SDL_SCANCODE_ESCAPE];

		if (mode == keyboard_mode) {
			for (int n = 0; n < note_count; n++) active[n] = !!key[keybind[n]];
		}
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
}

