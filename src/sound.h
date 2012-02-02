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

int opensound(bool with_sound);
void exitsound(void);

int loadsample(sample_e samp, char const *filename, bool loop, float gain);

// Load a wave file. Supported format include mono/stereo 8 or 16 bits/sample
int load_wave(sample_e samp, char const *fn, bool loop, float gain);

// Set listener position (will also update all attached sound sources)
void update_listener(vector const *pos, vector const *velocity, matrix const *rot);

// Play a sound (relative -> position is relative to the listener)
void playsound(enum voice, sample_e, float freq, vector const *pos, bool relative);

// same as above, but will update the sound source position when update_listener is called
void attachsound(enum voice, sample_e, float freq, vector const *pos, bool relative);

// A predefined position located in the head of the listener (relative pos of course)
vector voices_in_my_head;

#endif
