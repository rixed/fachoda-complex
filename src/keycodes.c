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
    {69,"View previous plane"},
    {67,"View your plane or closest ennemy"},
    {76,"View next bomb"},
    {98,"Rise your head"},
    {104,"Lower your head"},
    {100,"Turn left your head"},
    {102,"Turn right your head"},
    {97,"Look ahead"},
    {103,"Look backward"},
    {107,"Look at left"},
    {105,"Look at right"},
    {99,"Look up"},
    {106,"Look at the instrument panel"},

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

static FILE *keyfile_open(char const *perms)
{
    char file_name[2048];
    char const *home = getenv("HOME");
    snprintf(file_name, sizeof(file_name), "%s/.fachoda-keys", home ? home:".");
    FILE *f = fopen(file_name, perms);
    if (! f) {
        fprintf(stderr, "Cannot open '%s' for %s: %s\n", file_name, perms, strerror(errno));
        return NULL;
    }
    return f;
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

