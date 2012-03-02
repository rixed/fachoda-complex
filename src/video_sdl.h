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
#ifndef VIDEOSDL_H_120302
#define VIDEOSDL_H_120302

#include <stdbool.h>
#include <SDL/SDL.h>
#include "proto.h"

extern struct pixel32 *videobuffer;

void buffer2video(void);

SDLKey getscancode(void);

void initvideo(bool fullscreen);

bool kread(SDLKey);
bool kreset(SDLKey);
bool button_read(unsigned b);
bool button_reset(unsigned b);

void xproceed(void);

extern int xmouse, ymouse;

#endif
