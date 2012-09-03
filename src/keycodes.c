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
    [kc_esc]            = { SDLK_ESCAPE, "key_quit" },
    [kc_yes]            = { SDLK_y, "key_yes" },
    [kc_no]             = { SDLK_n, "key_no" },
    [kc_motormore]      = { SDLK_EQUALS, "key_throttle_more" },
    [kc_motorless]      = { SDLK_MINUS, "key_throttle_less" },
    [kc_externview]     = { SDLK_F5, "key_view_external" },
    [kc_travelview]     = { SDLK_F6, "key_view_still" },
    [kc_internview]     = { SDLK_F4, "key_view_internal" },
    [kc_nextbot]        = { SDLK_F2, "key_view_next" },
    [kc_prevbot]        = { SDLK_F3, "key_view_previous" },
    [kc_mybot]          = { SDLK_F1, "key_view_self" },
    [kc_mapmode]        = { SDLK_F9, "key_view_map" },
    [kc_zoomout]        = { SDLK_F7, "key_zoom_out" },
    [kc_zoomin]         = { SDLK_F8, "key_zoom_in" },
    [kc_riseview]       = { SDLK_UP, "key_look_raise" },
    [kc_lowerview]      = { SDLK_DOWN, "key_look_lower" },
    [kc_leftenview]     = { SDLK_LEFT, "key_look_left" },
    [kc_rightenview]    = { SDLK_RIGHT, "key_look_right" },
    [kc_towardview]     = { SDLK_HOME, "key_look_ahead" },
    [kc_backview]       = { SDLK_END, "key_look_back" },
    [kc_leftview]       = { SDLK_DELETE, "key_look_at_left" },
    [kc_rightview]      = { SDLK_PAGEDOWN, "key_look_at_right" },
    [kc_upview]         = { SDLK_PAGEUP, "key_look_up" },
    [kc_movetowardview] = { SDLK_INSERT, "key_look_panel" },
    [kc_gear]           = { SDLK_g, "key_gears" },
    [kc_flaps]          = { SDLK_f, "key_flaps" },
    [kc_brakes]         = { SDLK_b, "key_brakes" },
    [kc_autopilot]      = { SDLK_p, "key_autopilot" },
    [kc_business]       = { SDLK_F10, "key_buy" },
    [kc_pause]          = { SDLK_PAUSE, "key_pause" },
    [kc_highscores]     = { SDLK_TAB, "key_scores" },
    [kc_accelmode]      = { SDLK_x, "key_acceleration" },
    [kc_basenav]        = { SDLK_n, "key_navpoint_to_base" },
    [kc_suicide]        = { SDLK_F12, "key_suicide" },
    [kc_markpos]        = { SDLK_c, "key_flag_map" },
    [kc_alti]           = { SDLK_h, "key_cheat_up" },
    [kc_gunned]         = { SDLK_j, "key_cheat_gunme" },
    [kc_down]           = { SDLK_KP8, "key_nose_down" },
    [kc_up]             = { SDLK_KP2, "key_nose_up" },
    [kc_left]           = { SDLK_KP4, "key_roll_left" },
    [kc_right]          = { SDLK_KP6, "key_roll_right" },
    [kc_center]         = { SDLK_KP5, "key_center_stick" },
    [kc_fire]           = { SDLK_SPACE, "key_shoot" },
    [kc_weapon]         = { SDLK_RCTRL, "key_alt_weapon" },
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

