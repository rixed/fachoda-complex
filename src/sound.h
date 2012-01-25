#ifndef SOUND_H_120123
#define SOUND_H_120123

#include <stdbool.h>
#include "proto.h"

int opensound(bool with_sound);
void exitsound(void);

int loadsample(sample_e samp, char const *filename, bool loop, float gain);

// Set listener position (will also update all attached sound sources)
void update_listener(vector const *pos, vector const *velocity, matrix const *rot);

// Play a sound (relative -> position is relative to the listener)
void playsound(enum voice, sample_e, float freq, vector const *pos, bool relative);

// same as above, but will update the sound source position when update_listener is called
void attachsound(enum voice, sample_e, float freq, vector const *pos, bool relative);

// A predefined position located in the head of the listener (relative pos of course)
vector voices_in_my_head;

#endif
