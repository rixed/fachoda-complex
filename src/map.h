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
#ifndef MAP_H_120115
#define MAP_H_120115

#include <stdbool.h>
#include "proto.h"

struct tile {
    unsigned char z;    // The altitude of the map
    unsigned char submap:7; // The submap number (between 0 and 10)
    /* If has_road is set then the tile has (at least) one road crossing it,
     * is flat (submap is 0) and submap field encodes the road hash-key */
    unsigned char has_road:1;
    int first_obj;  // Number of first object above this tile (or -1 if none)
};

extern struct tile *map;

extern pixel zcol[256];

/* Given a tile index k, return its submap number */
static inline int submap_get(int k)
{
    return map[k].has_road ? 0 : map[k].submap;
}

/* Animate submap 9 with a wave pattern */
void animate_water(float dt_sec);

/* Return the altitude of the ground at this location, with or without submap */
float z_ground(float, float, bool);

/* Landscape generation (with roads, villages and so on */
void initmap(void);

/* Draw the landscape and all objects above it */
void draw_ground_and_objects(void);

/* Draw a single Gouraud shaded triangle.
 * @return true if the triangle was actually visible. */
bool poly_gouraud(vect2dc *p1, vect2dc *p2, vect2dc *p3);

#endif
