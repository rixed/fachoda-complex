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
#ifndef KEYS_H_120209
#define KEYS_H_120209

enum {
    kc_esc,
    kc_yes,
    kc_no,
    kc_motormore,
    kc_motorless,
    kc_externview,
    kc_travelview,
    kc_internview,
    kc_nextbot,
    kc_prevbot,
    kc_mybot,
    kc_mapmode,
    kc_zoomout,
    kc_zoomin,
    kc_riseview,
    kc_lowerview,
    kc_leftenview,
    kc_rightenview,
    kc_towardview,
    kc_backview,
    kc_leftview,
    kc_rightview,
    kc_upview,
    kc_movetowardview,
    kc_gear,
    kc_flaps,
    kc_brakes,
    kc_autopilot,
    kc_business,
    kc_pause,
    kc_highscores,
    kc_accelmode,
    kc_basenav,
    kc_suicide,
    kc_markpos,
    kc_alti,
    kc_gunned,
    kc_down,
    kc_up,
    kc_left,
    kc_right,
    kc_center,
    kc_fire,
    kc_weapon,
} kc_e;

void keys_load(void);

#endif
