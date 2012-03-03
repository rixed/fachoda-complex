// -*- c-basic-offset: 4; c-backslash-column: 79; indent-tabs-mode: nil -*-
// vim:sw=4 ts=4 sts=4 expandtab
/* Copyright 2012
 * This file is part of Fachoda.
 *
 * Fachoda is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fachoda is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fachoda.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SOUND_H_120123
#define SOUND_H_120123

#include <stdbool.h>
#include "proto.h"

int sound_init(bool with_sound);
void sound_fini(void);

enum snd_sample {
    SAMPLE_PRESENT, SAMPLE_BIPINTRO, SAMPLE_SHOT,
    SAMPLE_GEAR_DN, SAMPLE_GEAR_UP, SAMPLE_SCREETCH,
    SAMPLE_LOW_SPEED, SAMPLE_MOTOR, SAMPLE_HIT,
    SAMPLE_MESSAGE, SAMPLE_EXPLOZ, SAMPLE_BOMB_BLAST,
    SAMPLE_TOLE, SAMPLE_BIPBIP, SAMPLE_BIPBIP2, SAMPLE_BIPBIP3,
    SAMPLE_FEU, SAMPLE_TARATATA, SAMPLE_ALLELUIA,
    SAMPLE_ALERT, SAMPLE_DEATH, SAMPLE_PAIN, SAMPLE_BRAVO,
    NB_SAMPLES
};

enum snd_voice {
    VOICE_GEAR, VOICE_SHOT,
    VOICE_MOTOR, VOICE_EXTER,
    VOICE_EXTER2, VOICE_ALERT,
    NB_VOICES
};

// Set listener position (will also update all attached sound sources)
void update_listener(struct vector const *pos, struct vector const *velocity, struct matrix const *rot);

// Play a sound
// relative -> position is relative to the listener
// anchored -> position will be updated from frame to frame from given pos
void playsound(enum snd_voice, enum snd_sample, float freq, struct vector const *pos, bool relative, bool anchored);

// A predefined position located in the head of the listener (relative pos of course)
struct vector voices_in_my_head;

#endif
