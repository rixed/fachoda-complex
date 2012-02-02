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
#include <math.h>
#include "proto.h"

kc_s gkeys[NBKEYS] = {
    {9,"Quit"},
    {29,"Yes"},
    {57,"No"},

    {21,"Motor +5%"},
    {20,"Motor -5%"},

    {71,"External views"},
    {72,"Travelling view"},
    {70,"Internal views"},
    {73,"Zoom out"},
    {74,"Zoom in"},
    {68,"View next plane"},
    {69,"View prec plane"},
    {67,"View your plane or closest ennemy"},
    {76,"View next bomb"},
    {98,"Rise your head"},
    {104,"Lower your head"},
    {100,"Turn left your head"},
    {102,"Turn right your head"},
    {97,"Look towards you"},
    {103,"Look backward"},
    {107,"Look on your left"},
    {105,"Look on your right"},
    {99,"Look up"},
    {106,"Bend toward the instrument panel"},

    {42,"Gear"},
    {41,"Flaps"},
    {56,"Brakes"},
    {33,"Autopilot"},
    {95,"Buy a plane"},
    {80,"Nose down"},
    {88,"Nose up"},
    {83,"Roll left"},
    {85,"Roll right"},
    {84,"Center stick"},
    {65,"Fire"},
    {109,"Change weapon"},

    {110,"Pause"},
    {36,"See Highscores"},
    {53,"Accelerated mode"},
    {57,"Set navpoint to home base"},
    {75,"Map mode"},
    {96,"Suicide"},
    {39,"Flag the map at plane's position"},

    {43,"Emergency UP! (...?)"},
    {52,"Gun this plane (...?)"}
};

