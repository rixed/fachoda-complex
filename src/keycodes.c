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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <SDL/SDL.h>
#include "file.h"
#include "proto.h"

struct kc gkeys[NBKEYS] = {
    { SDLK_ESCAPE, "Quit" },
    { SDLK_y, "Yes" },
    { SDLK_n, "No" },

    { SDLK_EQUALS, "Motor +5%" },
    { SDLK_MINUS, "Motor -5%" },

    { SDLK_F5, "External views" },
    { SDLK_F6, "Travelling view" },
    { SDLK_F4, "Internal views" },
    { SDLK_F7, "Zoom out" },
    { SDLK_F8, "Zoom in" },
    { SDLK_F2, "View next plane" },
    { SDLK_F3, "View previous plane" },
    { SDLK_F1, "View your plane or closest ennemy" },
    { SDLK_m, "View next bomb" },
    { SDLK_UP, "Rise your head" },
    { SDLK_DOWN, "Lower your head" },
    { SDLK_LEFT, "Turn left your head" },
    { SDLK_RIGHT, "Turn right your head" },
    { SDLK_HOME, "Look ahead" },
    { SDLK_END, "Look backward" },
    { SDLK_DELETE, "Look at left" },
    { SDLK_PAGEDOWN, "Look at right" },
    { SDLK_PAGEUP, "Look up" },
    { SDLK_INSERT, "Look at the instrument panel" },

    { SDLK_g, "Gear" },
    { SDLK_f, "Flaps" },
    { SDLK_b, "Brakes" },
    { SDLK_p, "Autopilot" },
    { SDLK_F10, "Buy a plane" },
    { SDLK_KP8, "Nose down" },
    { SDLK_KP2, "Nose up" },
    { SDLK_KP4, "Roll left" },
    { SDLK_KP6, "Roll right" },
    { SDLK_KP6, "Center stick" },
    { SDLK_SPACE, "Fire" },
    { SDLK_RCTRL, "Change weapon" },

    { SDLK_PAUSE, "Pause" },
    { SDLK_TAB, "See Highscores" },
    { SDLK_x, "Accelerated mode" },
    { SDLK_n, "Set navpoint to home base" },
    { SDLK_F9, "Map mode" },
    { SDLK_F12, "Suicide" },
    { SDLK_c, "Flag the map at plane's position" },

    { SDLK_h, "Emergency UP! (...?)" },
    { SDLK_j, "Gun this plane (...?)" }
};

static FILE *keyfile_open(char const *perms)
{
    return file_open_try(".fachoda-keys", getenv("HOME"), perms);
}

void keys_save(void)
{
    FILE *f = keyfile_open("w+");
    if (! f) return;

    for (unsigned i = 0; i < ARRAY_LEN(gkeys); i++) {
        ssize_t ret = fwrite(&gkeys[i].kc, sizeof(gkeys[i].kc), 1, f);
        if (ret < 1) {
            fprintf(stderr, "Cannot write key\n");
        }
    }

    fclose(f);
}

void keys_load(void)
{
    FILE *f = keyfile_open("r");
    if (! f) return;

    for (unsigned i = 0; i < ARRAY_LEN(gkeys); i++) {
        ssize_t ret = fread(&gkeys[i].kc, sizeof(gkeys[i].kc), 1, f);
        if (ret < 1) {
            fprintf(stderr, "Cannot read key\n");
        }
    }

    fclose(f);
}

