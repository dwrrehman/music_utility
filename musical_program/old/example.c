// a simple shell that uses notes 
// to communicate the output to the user.

#include <SDL2/SDL.h>

void my_audio_callback(
	void *userdata, 
	Uint8 *stream, 
	int len
);

static Uint8 *audio_pos; 
static Uint32 audio_len; 

int main(int argc, const char** argv) {

	if (SDL_Init(SDL_INIT_AUDIO) < 0) exit(0);

	static Uint32 wav_length;
	static Uint8 *wav_buffer;
	static SDL_AudioSpec wav_spec;
	wav_spec.callback = my_audio_callback;
	wav_spec.userdata = NULL;
	audio_pos = wav_buffer;
	audio_len = wav_length;
	
	if (SDL_OpenAudio(&wav_spec, NULL) < 0) {
		fprintf(stderr, "audio: %s\n", 
			SDL_GetError()
		);
		exit(1);
	}
	
	SDL_PauseAudio(0);
	while ( audio_len > 0 ) {
		SDL_Delay(100); 
	}
	
	SDL_CloseAudio();
	SDL_FreeWAV(wav_buffer);

}

void my_audio_callback(
	void *userdata, 
	Uint8 *stream, 
	int len
) {
	if (not audio_len) return;
	if(audio_len < len) len = audio_len;

//SDL_memcpy(stream, audio_pos, len);
// simply copy from one buffer into the other
SDL_MixAudio(stream, audio_pos, 
len, SDL_MIX_MAXVOLUME);
// mix from one buffer into another
	
	audio_pos += len;
	audio_len -= len;
}



