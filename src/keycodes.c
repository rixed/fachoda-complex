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
#include <strings.h>
#include <errno.h>
#include <stdint.h>
#include <SDL/SDL.h>
#include "proto.h"
#include "config.h"
#include "keycodesdef.h"

struct kc gkeys[NBKEYS] = {
    { SDLK_ESCAPE, "key_quit" },
    { SDLK_y, "key_yes" },
    { SDLK_n, "key_no" },
    { SDLK_EQUALS, "key_throttle_more" },
    { SDLK_MINUS, "key_throttle_less" },
    { SDLK_F5, "key_view_external" },
    { SDLK_F6, "key_view_still" },
    { SDLK_F4, "key_view_internal" },
    { SDLK_F2, "key_view_next" },
    { SDLK_F3, "key_view_previous" },
    { SDLK_F1, "key_view_self" },
    { SDLK_F9, "key_view_map" },
    { SDLK_F7, "key_zoom_out" },
    { SDLK_F8, "key_zoom_in" },
    { SDLK_UP, "key_look_raise" },
    { SDLK_DOWN, "key_look_lower" },
    { SDLK_LEFT, "key_look_left" },
    { SDLK_RIGHT, "key_look_right" },
    { SDLK_HOME, "key_look_ahead" },
    { SDLK_END, "key_look_back" },
    { SDLK_DELETE, "key_look_at_left" },
    { SDLK_PAGEDOWN, "key_look_at_right" },
    { SDLK_PAGEUP, "key_look_up" },
    { SDLK_INSERT, "key_look_panel" },
    { SDLK_g, "key_gears" },
    { SDLK_f, "key_flaps" },
    { SDLK_b, "key_brakes" },
    { SDLK_p, "key_autopilot" },
    { SDLK_F10, "key_buy" },
    { SDLK_PAUSE, "key_pause" },
    { SDLK_TAB, "key_scores" },
    { SDLK_x, "key_acceleration" },
    { SDLK_n, "key_navpoint_to_base" },
    { SDLK_F12, "key_suicide" },
    { SDLK_c, "key_flag_map" },
    { SDLK_h, "key_cheat_up" },
    { SDLK_j, "key_cheat_gunme" },
    { SDLK_KP8, "key_nose_down" },
    { SDLK_KP2, "key_nose_up" },
    { SDLK_KP4, "key_roll_left" },
    { SDLK_KP6, "key_roll_right" },
    { SDLK_KP5, "key_center_stick" },
    { SDLK_SPACE, "key_shoot" },
    { SDLK_RCTRL, "key_alt_weapon" },
};

static SDLKey sdl_key_of_name(char const *name)
{
    for (SDLKey k = SDLK_FIRST; k < SDLK_LAST; k++) {
        if (0 == strcasecmp(name, SDL_GetKeyName(k))) return k;
    }

    fprintf(stderr, "Unknown key \"%s\"\n", name);
    return SDLK_UNKNOWN;
}

void keys_load(void)
{
    for (unsigned i = 0; i < ARRAY_LEN(gkeys); i++) {
        char const *keyname = config_get_string(gkeys[i].varname, NULL);
        if (keyname) {
            SDLKey kc = sdl_key_of_name(keyname);
            if (kc != SDLK_UNKNOWN) gkeys[i].kc = kc;
        }
    }
}

