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
#include <stdint.h>
#include <math.h>
#include <values.h>
#include "heightfield.h"

// Length of the vision square
#define VIEW_LEN_N 5
#define VIEW_LEN (1 << VIEW_LEN_N)
// Length of the submap square
#define SUBMAP_LEN_N 3
#define SUBMAP_LEN (1 << SUBMAP_LEN_N)

#define MASK_W(a) ((a) & (MAP_LEN-1))

/* The map is the central structure of the game.
 * It's a simple square heightfield of unsigned bytes
 * which side length is a power of 2.
 *
 * There are 11 of these maps.
 * The most important one is the global map, of side length MAP_LEN (128).
 * The other ones are the submaps, of length SUBMAP_LEN (16), representing the height of
 * each world map tile.
 * We have 10 of them, 9 is for water (animated), while 0
 * is completely flat for the airfields and roads. Each other tile of
 * the global map that is neither water nor airfield is
 * attributed one of the other 1 to 8 submaps according to its
 * height, with some random.
 *
 * So all in all we need very few memory for the world map (less than 20Kb).
 */

struct tile *map;   // The global map: a square of MAP_LEN*MAP_LEN tiles.

/* The submaps (same structure as the global map, only smaller: SUBMAP_LEN*SUBMAP_LEN).
 * These (positive) heights are added to the main map heights, so that submaped tiles
 * are always slightly higher. Thus, transition between submaped close tiles and tiles
 * farther away show no gap.
 */
static uchar *submap[10];

/* Each vector of a submap heighfield have a specific lightning (to be added to the
 * lightning of the tile). It's faster than to compute a proper lightning for every
 * polygon!
 */
static signed char *submap_rel_light[ARRAY_LEN(submap)];

extern inline int submap_get(int k);

/* Ground colors are chosen merely based on the height of the map.
 * So this array gives the color of any altitude.
 * It's also used to draw the ingame map.
 */
struct pixel zcol[256];

/*
 * Map Generation
 */

// Generate the heighfield for a portion of the map (recursive)
static void random_submap(
    int x, int y,   // location of the starting corner of the square to randomize
    int s,          // length of the square to randomize
    size_t z_off,   // offset between any two consecutive z bytes along x
    uchar *m,       // first z
    int smap        // length ot the total map
) {
    int ss = s >> 1;
    if (! ss) return;

    double sf = Accident * (double)s/smap;
    // offset of first corner of the square
    size_t const sx = z_off * x;
    size_t const sy = z_off * y * smap;
    // offset of the last corner of the square (wrapped)
    size_t const ey = y + s == smap ? 0 : z_off * (y + s);
    size_t const ex = x + s == smap ? 0 : z_off * (x + s);
    // offset of the center of the square
    size_t const mx = sx + z_off * ss;
    size_t const my = sy + z_off * ss * smap;

    if (m[mx + sy] == 0)
        m[mx + sy] = (((int)m[sx + sy] + m[ex + sy])>>1) + (drand48()-.5)*sf;
    if (m[ex + my] == 0)
        m[ex + my] = (((int)m[ex + sy] + m[ex + ey])>>1) + (drand48()-.5)*sf;
    if (m[mx + ey] == 0)
        m[mx + ey] = (((int)m[sx + ey] + m[ex + ey])>>1) + (drand48()-.5)*sf;
    if (m[sx + my] == 0)
        m[sx + my] = (((int)m[sx + sy] + m[sx + ey])>>1) + (drand48()-.5)*sf;
    if (m[mx + my] == 0)
        m[mx + my] = (((int)m[mx + sy] + m[ex + my] + m[mx + ey] + m[sx + my])>>2) + (drand48()-.5)*sf;
    // Recurse into each quad
    random_submap(     x,      y, ss, z_off, m, smap);
    random_submap(x + ss,      y, ss, z_off, m, smap);
    random_submap(     x, y + ss, ss, z_off, m, smap);
    random_submap(x + ss, y + ss, ss, z_off, m, smap);
}

static void smooth_map(uchar *m, int smap, int s, size_t z_off){
    for (int i = 0; i < s; i++) {
        size_t yo = 0, yn;
        for (int y = 0; y < smap; y++, yo = yn) {
            size_t xo = 0, xn;
            yn = y == smap-1 ? 0 : yo + z_off*smap;
            for (int x = 0; x < smap; x++, xo = xn) {
                xn = x == smap-1 ? 0 : xo + z_off;
                m[xo+yo] = (m[xo+yo] + m[xn+yo] + m[xo+yn]) / 3;
            }
        }
    }
}

static void make_map(uchar *m, int smooth_factor, int mapzmax, int map_length, size_t z_off) {
    m[0] = 255; // ?
    random_submap(0, 0, map_length, z_off, m, map_length);

    /* Scale all heights (remember height are unsigned bytes)
     * so that we have all values from 0 to mapzmax */
    int zmin = MAXINT, zmax = -MAXINT;
    size_t const map_size = map_length * map_length;
    for (size_t i = 0; i < map_size; i += z_off) {
        int const z = m[i];
        if (z > zmax) zmax = z;
        if (z < zmin) zmin = z;
    }
    double const ratio = (double)mapzmax/(zmax-zmin+1);
    for (size_t i = 0; i < map_size; i += z_off) {
        m[i] = (m[i]-zmin)*ratio;
    }

    // Smooth all maps at least once
    smooth_map(m, map_length, smooth_factor, z_off);
}

// dig a wave in the map (in other words, a river)
// looks alien
static void dig(int dy) {
    float a,ai,xx,yy;
    int x,y;
    a=0; ai=.02; xx=0; yy=MAP_LEN/3;
    do {
        x=xx;
        y=yy;
        map[x+(y+dy)*MAP_LEN].z=1;
        x=xx-sin(a);
        y=yy+cos(a);
        map[x+(y+dy)*MAP_LEN].z=1;
        xx+=cos(a);
        yy+=sin(a);
        a+=ai;
        if ((ai>0 && a>M_PI/3) || (ai<0 && a<-M_PI/3)) ai=-ai;
    } while (x<MAP_LEN-2 && y>2 && y<MAP_LEN-2);
}

void initmap(void) {
    int x,y,i;
    struct pixel colterrain[4] = {
        { 150,150,20 }, // glouglou
        { 111,180,215 },    // désert
        { 40, 140, 70 },    // prairie
        { 210,210,210 } // roc
    };
    struct {
        uchar z1, z2;
    } zcolterrain[4] = { {0,3}, {40,50}, {80,151}, {200,255} };//{ {0,52}, {100,151}, {200,255} };
    // ZCOL
    for (y=0; y<4; y++) {
        for (x=zcolterrain[y].z1; x<=zcolterrain[y].z2; x++) {
            zcol[x].r=colterrain[y].r;
            zcol[x].g=colterrain[y].g;
            zcol[x].b=colterrain[y].b;
        }
        if (y<3) for (;x<zcolterrain[y+1].z1; x++) {
            zcol[x].r=colterrain[y].r+(colterrain[y+1].r-colterrain[y].r)/(zcolterrain[y+1].z1-zcolterrain[y].z2)*(x-zcolterrain[y].z2);
            zcol[x].g=colterrain[y].g+(colterrain[y+1].g-colterrain[y].g)/(zcolterrain[y+1].z1-zcolterrain[y].z2)*(x-zcolterrain[y].z2);
            zcol[x].b=colterrain[y].b+(colterrain[y+1].b-colterrain[y].b)/(zcolterrain[y+1].z1-zcolterrain[y].z2)*(x-zcolterrain[y].z2);
        }
    }
    if (Dark) for (x=0; x<256; x++) {
        zcol[x].r-=zcol[x].r>>1;
        zcol[x].g-=zcol[x].g>>2;
        zcol[x].b-=zcol[x].b>>2;
    }

    // Global map
    map = calloc(MAP_LEN*MAP_LEN,sizeof(*map));   // starts from 0
    if (Fleuve) dig(0); // digging in 0?
    make_map(&map[0].z, 0, 255, MAP_LEN, sizeof(*map));
    for (i=0;i<Smooth; i++) {   // Instead of using make_map smooth_factor, we smooth (and dig) several times.
        smooth_map(&map[0].z, MAP_LEN, 1, sizeof(*map));
        if (Fleuve) dig(0);
    }
    for (y=0; y<MAP_LEN; y++) {
        for (x=0; x<MAP_LEN; x++) {
            map[x+y*MAP_LEN].has_road = 0;
            map[x+y*MAP_LEN].first_obj = -1;
            // init submap according to z
            int const z = map[x+y*MAP_LEN].z;
            if (z > 200) {  // submaps 4 and 5 are for high lands
                map[x+y*MAP_LEN].submap = 4+drand48()*2;
            } else if (z < 100) { // submaps 6, 7, 8 are for middle lands
                if (z > 15) map[x+y*MAP_LEN].submap = 6+drand48()*3;
                else map[x+y*MAP_LEN].submap = 9;
            } else { // submaps 1, 2, 3 are for low lands
                map[x+y*MAP_LEN].submap = 1+drand48()*3;
            }
        }
    }

    // Build submaps
    for (unsigned i = 0; i < ARRAY_LEN(submap); i++) {
        submap[i] = calloc(SUBMAP_LEN*SUBMAP_LEN, sizeof(*submap[i]));
    }
    make_map(submap[0], 10,  5, SUBMAP_LEN, 1); // flat
    make_map(submap[1], 2, 100, SUBMAP_LEN, 1); // mostly flat
    make_map(submap[2], 2, 190, SUBMAP_LEN, 1); // mostly flat
    make_map(submap[3], 2, 230, SUBMAP_LEN, 1); // mostly flat
    make_map(submap[4], 1, 250, SUBMAP_LEN, 1); // mountain tops
    make_map(submap[5], 1, 250, SUBMAP_LEN, 1); // mountain tops
    make_map(submap[6], 3, 253, SUBMAP_LEN, 1); // middle lands
    make_map(submap[7], 3, 253, SUBMAP_LEN, 1); // middle lands
    make_map(submap[8], 2, 253, SUBMAP_LEN, 1); // middle lands
    make_map(submap[9], 0,   2, SUBMAP_LEN, 1); // reserved for water: we will move this heighfield
    // compure relative lighting.
    for (unsigned i = 0; i < ARRAY_LEN(submap); i++) {
        submap_rel_light[i]=malloc(SUBMAP_LEN*SUBMAP_LEN*sizeof(char));
        for (y=0; y<SUBMAP_LEN; y++) {
            for (x=0; x<SUBMAP_LEN; x++) {
                double in;
                int z=submap[i][x+y*SUBMAP_LEN];
                if (x) in=((z-submap[i][x-1+y*SUBMAP_LEN])/200.+1.)*.5;
                else in=((z-submap[i][SUBMAP_LEN-1+y*SUBMAP_LEN])/200.+1.)*.5;
                if (in<0) in=0;
                else if (in>1) in=1;
                submap_rel_light[i][x+y*SUBMAP_LEN]=90*(in-.5);
            }
        }
    }
}

void animate_water(float dt_sec)
{
    static float Fphix=0, Fphiy=0;
    for (int y=0; y<SUBMAP_LEN; y++) {
        float sy=sin(y*2*M_PI/SUBMAP_LEN+Fphiy);
        for (int x=0; x<SUBMAP_LEN; x++) {
            submap[9][x+(y<<SUBMAP_LEN_N)]=128+60*(sin(x*4*M_PI/SUBMAP_LEN+Fphix)+sy);
            submap_rel_light[9][x+(y<<SUBMAP_LEN_N)]=8*(cos(x*4*M_PI/SUBMAP_LEN+Fphix)+sy);
        }
    }
    Fphiy += .003 * dt_sec;
    Fphix += .011 * dt_sec;
}

/*
 * Map Rendering
 */

// Set to true for flat shadded landscape
bool flat_shadding = false;

/* This is an awful hack to skip drawing of roads when the ground is seen from below
 * (for instance, roads on the other side of a hill). Problems:
 * - this flag is set by the poly_gouraud function, while it should be set by the draw_ground
 *   method itself, which should choose internally whether to draw road or not (based on this
 *   and distance, for instance)
 * - this currently works because roads are only possible on flat submap (thus the last poly_gouraud
 *   will have the correct information)
 */
static int some_poly_were_visible;

// Clip segment p1-p2 by the frustum zmin plan (p1->z is below FRUSTUM_ZMIN)
#define FRUSTUM_ZMIN (32<<8)
static void do_clip(struct vecic *p1, struct vecic *p2, struct vecic *pr) {
    int const dz1 = FRUSTUM_ZMIN - p1->v.z;
    int const dz2 = p2->v.z - p1->v.z;
    pr->v.x = p1->v.x + ((dz1 * (((p2->v.x-p1->v.x)<<8)/dz2))>>8);
    pr->v.y = p1->v.y + ((dz1 * (((p2->v.y-p1->v.y)<<8)/dz2))>>8);
    pr->v.z = FRUSTUM_ZMIN;
    pr->c.r = p1->c.r + ((dz1 * ((((int)p2->c.r-p1->c.r)<<8)/dz2))>>8);
    pr->c.g = p1->c.g + ((dz1 * ((((int)p2->c.g-p1->c.g)<<8)/dz2))>>8);
    pr->c.b = p1->c.b + ((dz1 * ((((int)p2->c.b-p1->c.b)<<8)/dz2))>>8);
}

static void poly(struct vecic *p1, struct vecic *p2, struct vecic *p3) {
    struct vect2dc l1,l2,l3;
    proji(&l1.v, &p1->v);   // FIXME: should not project the same pt several times!
    l1.c = p1->c;
    proji(&l2.v, &p2->v);
    l2.c = p2->c;
    proji(&l3.v, &p3->v);
    l3.c = p3->c;
    struct pixel mix = {
        .r = (l1.c.r + l2.c.r + l3.c.r) / 3,
        .g = (l1.c.g + l2.c.g + l3.c.g) / 3,
        .b = (l1.c.b + l2.c.b + l3.c.b) / 3,
    };
    if (flat_shadding) {
        some_poly_were_visible = polyflat(&l1.v, &l2.v, &l3.v, mix);
    } else {
        some_poly_were_visible = poly_gouraud(&l1, &l2, &l3);
    }
}

void polyclip(struct vecic *p1, struct vecic *p2, struct vecic *p3) {
    int i;
    struct vecic pp1, pp2;
    i  =  p1->v.z < FRUSTUM_ZMIN;
    i += (p2->v.z < FRUSTUM_ZMIN)<<1;
    i += (p3->v.z < FRUSTUM_ZMIN)<<2;
    switch (i) {
    case 0:
        poly(p1,p2,p3);
        break;
    case 1:
        do_clip(p1,p2,&pp1);
        do_clip(p1,p3,&pp2);
        poly(&pp1,p2,p3);
        poly(&pp1,p3,&pp2);
        break;
    case 2:
        do_clip(p2,p3,&pp1);
        do_clip(p2,p1,&pp2);
        poly(&pp1,p3,p1);
        poly(&pp1,p1,&pp2);
        break;
    case 3:
        do_clip(p1,p3,&pp1);
        do_clip(p2,p3,&pp2);
        poly(&pp1,&pp2,p3);
        break;
    case 4:
        do_clip(p3,p1,&pp1);
        do_clip(p3,p2,&pp2);
        poly(&pp1,p1,p2);
        poly(&pp1,p2,&pp2);
        break;
    case 5:
        do_clip(p1,p2,&pp1);
        do_clip(p3,p2,&pp2);
        poly(&pp2,&pp1,p2);
        break;
    case 6:
        do_clip(p2,p1,&pp1);
        do_clip(p3,p1,&pp2);
        poly(&pp1,&pp2,p1);
        break;
    case 7:
        // don't draw anything is all points are below zmin
        break;
    }
}

/* We will draw the world using the painter method.
 * Notice that the painter method is only local: the painter
 * does not necessarily sort by distance all what he has to draw and proceed to
 * draw things in that order. He proceed that way to draw a portion of the painting,
 * then can switch to another portion of its drawing which distance is unrelated
 * to the portion he just left. So, he sort only locally.
 * We do the same here: we do not sort all the world's tiles but merely select a
 * way to traverse the world map so that we avoid overdrawing an object with another
 * one that's farther away (notice there are no zbuffer).
 * It's mostly enough to traverse the map along its rows and columns, starting
 * from the farther corner and proceeding toward the camera (obj[0]).
 * So here we consider the direction the camera is looking at and choose
 * the traversal parameters accordingly.
 * (why only "mostly enough"? Because the objects within the tiles are allowed
 * to cross the tiles boundaries - we deal with this with the defered_tiles list).
 */
/*                                Y
 *                  case 2       ^        case 3
 *                       ^       |       ^
 *                        \      |      /
 *                         \     |     /
 *                          \    |    /
 *          case 6 <-___     \   |   /     ___-> case 7
 *                      \___  \  |  /  ___/
 *                          \__\ | /__/
 *                              \|/
 *                   -----------------------------> X
 *                            __/|\__
 *                        ___/ / | \ \___
 *                    ___/    /  |  \    \___
 *          case 4 <-/       /   |   \       \-> case 5
 *                          /    |    \
 *                         /           \
 *                        /             \
 *                       v               v
 *                     case 0          case 1
 *
 * map[0] is down there
 */
/* Also, we want to draw     But what we have in tiles[] is
 * each tile like this:      this (l is either 0 or 1):
 *
 *  NW-------NE               [dx-1][!l]---[dx][!l]
 *   | \     |                     |          |
 *   |   \   |                     |          |
 *   |     \ |                     |          |
 *  SW-------SE                [dx-1][l]----[dx][l] <- current one
 *
 * Notice that we don't know the orientation of this tile.
 * In case 1, for instance, current point will be NW.
 */
/* To finish, note that map[x,y] gives the tile which south-west corner
 * lies at position x,y.
 * So the first tile to draw is this one only if we are facing south-west! */

struct orient_param {
    int start_x_dir;
    int start_y_dir;
    enum corner { SW, NW, SE, NE } f0, f1, f2, f3;  // f0 is the farthest, f0-f1 is the inner loop, f0-f2 the outer loop
    int inner_dx, inner_dy;
    int outer_dx, outer_dy;
    int dl1, dx1,
        dl2, dx2,
        dl3, dx3,
        dl4, dx4;
    int dmx, dmy;   // when we are at x,y, the tile is map[x+dmx,y+dmy] (ie. where is SW?)
};

static void render_map(
    int corner_x, int corner_y,
    struct orient_param const *orient,
    unsigned length_n,
    unsigned nb_params, int (*param)[4], // each param has 4 values (order sw, nw, se, ne)
    unsigned nb_add_params, // number of parameters set by get_z
    void (*get_z)(int x, int y, int *current),
    void (*render_tile)(
        int x, int y, struct orient_param const *orient,
        int *sw, int *nw, int *se, int *ne)
) {
    int const length = 1 << length_n;

    // Starting tile (further away)
    int const start_x = corner_x + (orient->start_x_dir << length_n);
    int const start_y = corner_y + (orient->start_y_dir << length_n);

    int tiles[length+1][2][nb_params+nb_add_params];
    int oincr1_p[nb_params];    // to goes from value f0 to value f2
    int oincr2_p[nb_params];    // to goes from value f1 to value f3
    for (unsigned p = 0; p < nb_params; p++) {
        oincr1_p[p] = (param[p][orient->f2] - param[p][orient->f0]) >> length_n;
        oincr2_p[p] = (param[p][orient->f3] - param[p][orient->f1]) >> length_n;
    }

    // Outer loop, along the 2nd farther edge of the view square
    /* FIXME: Both the inner and outer loops could also be exited whenever the z goes too far behind.
     *        We can't do that since when the camera is pointed at the sky the whole landscape
     *        is <Z_MIN, yet we want to draw the objects (if not the landscape itself).
     *        There is certainly room for optimization, though (like draw close tiles objects
     *        at the end in all cases). Ideally, we'd like no know when the whole space perpendicular
     *        to a tile is completely behind us.
     */
    for (
        int ox = start_x, oy = start_y, od = 0, l = 0;
        od <= length;
        ox += orient->outer_dx,
        oy += orient->outer_dy,
        od ++,
        l ^= 1
    ) {
        int iparam[nb_params];
        for (unsigned p = 0; p < nb_params; p++) {
            iparam[p] = param[p][orient->f0];
        }

        int iincr_p[nb_params]; // to goes from value f0 to value f1
        for (unsigned p = 0; p < nb_params; p++) {
            iincr_p[p] = (param[p][orient->f1] - param[p][orient->f0]) >> length_n;
        }

        // Inner loop, along the farther edge of the view square
        for (
            int ix = ox, iy = oy, id = 0;
            id <= length;
            ix += orient->inner_dx,
            iy += orient->inner_dy,
            id ++
        ) {
            // Store tile altitude and current parameters in tiles[id][l]
            for (unsigned p = 0; p < nb_params; p++) {
                tiles[id][l][p] = iparam[p];
            }
            get_z(ix, iy, tiles[id][l]);

            if (od > 0 && id > 0) {
                render_tile(ix, iy, orient,
                    tiles[id-orient->dx1][l^orient->dl1],   // SW
                    tiles[id-orient->dx2][l^orient->dl2],   // NW
                    tiles[id-orient->dx3][l^orient->dl3],   // SE
                    tiles[id-orient->dx4][l^orient->dl4]);  // NE
            }

            for (unsigned p = 0; p < nb_params; p++) {
                iparam[p] += iincr_p[p];
            }
        }
        for (unsigned p = 0; p < nb_params; p++) {
            param[p][orient->f0] += oincr1_p[p];
            param[p][orient->f1] += oincr2_p[p];
        }
    }
}

// These are inited by draw_ground_and_objects() and used also by it's callbacks
static struct veci mx, my, mz; // World map base relative to camera, in 24:8 fixed prec.
static int camera_x, camera_y;  // location of the camera in map[]
static int defered_tiles[9], nb_defered_tiles;  // some tiles which objects we want to draw last

// For rendering submap tiles
static int submap_x, submap_y;    // location of our submap

extern inline int add_sat(int a, int b, int max);

static void get_submap_z(int x, int y, int *params)
{
    int m_x = submap_x, m_y = submap_y;
    if (x == SUBMAP_LEN) {
        x = 0;
        m_x = MASK_W(m_x + 1);
    }
    if (y == SUBMAP_LEN) {
        y = 0;
        m_y = MASK_W(m_y + 1);
    }

    int const smap = submap_get(m_x + (m_y << LOG_MAP_LEN));
    int const sm = x + (y << SUBMAP_LEN_N);
    int const z = submap[smap][sm];
    params[6] = params[0] + ((mz.x * z) >> 10);
    params[7] = params[1] + ((mz.y * z) >> 10);
    params[8] = params[2] + ((mz.z * z) >> 10);

    int const rel_light = submap_rel_light[smap][sm] << 8;
    params[9]  = add_sat(params[3], rel_light, 0xFFFF);
    params[10] = add_sat(params[4], rel_light, 0xFFFF);
    params[11] = add_sat(params[5], rel_light, 0xFFFF);
}

// For rendering map tiles

static void get_map_z(int x, int y, int *params)
{
    int const z      = map[MASK_W(x) + (MASK_W(y)<<LOG_MAP_LEN)].z;
    int const z_next = map[MASK_W(x-1) + (MASK_W(y)<<LOG_MAP_LEN)].z;

    int intens = ((z-z_next))+32+64;
    if (intens<64) intens=64;
    else if (intens>127) intens=127;

    // params: x, y, z (of the grid) + x y z of the ground + r g b
    params[3] = params[0] + ((mz.x * (z<<2)) >> 8);
    params[4] = params[1] + ((mz.y * (z<<2)) >> 8);
    params[5] = params[2] + ((mz.z * (z<<2)) >> 8);
    params[6] = (zcol[z].r * intens)<<1;
    params[7] = (zcol[z].g * intens)<<1;
    params[8] = (zcol[z].b * intens)<<1;
}

static void draw_tile(int *sw, int *nw, int *se, int *ne, int xo, int ro)
{
    // Draw a mere tile, without submap
    struct vecic p1 = {
        .v = { .x=sw[xo], .y=sw[xo+1], .z=sw[xo+2] },
        .c = { .r=sw[ro]>>8, .g=sw[ro+1]>>8, .b=sw[ro+2]>>8 }
    };
    struct vecic p2 = {
        .v = { .x=nw[xo], .y=nw[xo+1], .z=nw[xo+2] },
        .c = { .r=nw[ro]>>8, .g=nw[ro+1]>>8, .b=nw[ro+2]>>8 }
    };
    struct vecic p3 = {
        .v = { .x=se[xo], .y=se[xo+1], .z=se[xo+2] },
        .c = { .r=se[ro]>>8, .g=se[ro+1]>>8, .b=se[ro+2]>>8 }
    };
    struct vecic p4 = {
        .v = { .x=ne[xo], .y=ne[xo+1], .z=ne[xo+2] },
        .c = { .r=ne[ro]>>8, .g=ne[ro+1]>>8, .b=ne[ro+2]>>8 }
    };
    polyclip(&p1, &p2, &p3);
    polyclip(&p4, &p3, &p2);
}

static void render_submap_tile(int x, int y, struct orient_param const *orient, int *sw, int *nw, int *se, int *ne)
{
    (void)x; (void)y; (void)orient;
    draw_tile(sw, nw, se, ne, 6, 9);
}

static void render_map_tile(int x, int y, struct orient_param const *orient, int *sw, int *nw, int *se, int *ne)
{
    submap_x = x + orient->dmx;
    submap_y = y + orient->dmy;
    int const m = MASK_W(submap_x) + (MASK_W(submap_y)<<LOG_MAP_LEN);
    int const dx = abs(submap_x - camera_x);
    int const dy = abs(submap_y - camera_y);

    // First draw the landscape

    some_poly_were_visible = 0; // FIXME: please find me an elegant substitute

#   define DIST_REMAP 3
    if (dx < DIST_REMAP && dy < DIST_REMAP) {
        // Draw a more detailed view of the ground, using submaps
        // We need to interpolate colors and altitudes
        int params[6][4];
        for (unsigned p = 0; p < 6; p++) {
            params[p][SW] = sw[p+3];
            params[p][NW] = nw[p+3];
            params[p][SE] = se[p+3];
            params[p][NE] = ne[p+3];
        }
        render_map(
            0, 0, orient, SUBMAP_LEN_N,
            6, params, 6,   // to the original x,y,z,r,g,b, add new x,y,z,r,g,b of the ground
            get_submap_z,
            render_submap_tile
        );
    } else {
        draw_tile(sw, nw, se, ne, 3, 6);
    }

    // Draw roads
    if (some_poly_were_visible && map[m].has_road) {
        drawroute(m);
    }

    // Then draw the objects

#   define DIST_VISU 5
    if (dx < 2 && dy < 2) {
        if (map[m].first_obj != -1) {
            renderer(m, GROUND);
            defered_tiles[nb_defered_tiles++] = m;
        }
    } else if (dx < DIST_VISU && dy < DIST_VISU) {
        if (map[m].first_obj != -1) {
            renderer(m, ALL);
        }
    } else {
        renderer(m, CLOUDS);
    }
}

// Return the coordinates of map[x,y] relative to camera
static void cam_to_tile(int *v, int x, int y)
{
    struct vector v_ = { (x-MAP_LEN/2)<<LOG_TILE_LEN, (y-MAP_LEN/2)<<LOG_TILE_LEN, 0. };
    subv(&v_, &obj[0].pos);
    mulmtv(&obj[0].rot, &v_, &v_);
    v[0] = v_.x*256.;
    v[4] = v_.y*256.;
    v[8] = v_.z*256.;
}

void draw_ground_and_objects(void)
{
    // Compute our orientation
    unsigned o =
        (obj[0].rot.z.x > 0) |
        ((obj[0].rot.z.y > 0) <<1) |
        ((fabs(obj[0].rot.z.x) > fabs(obj[0].rot.z.y)) <<2);

    // The parameters driving the traversal of the map depend on this orientation
    static struct orient_param const orient_param[8] = {
        { 0,0, SW,SE,NW,NE, +1,0, 0,+1, 1,1, 0,1, 1,0, 0,0, -1,-1 },
        { 1,0, SE,SW,NE,NW, -1,0, 0,+1, 1,0, 0,0, 1,1, 0,1,  0,-1 },
        { 0,1, NW,NE,SW,SE, +1,0, 0,-1, 0,1, 1,1, 0,0, 1,0, -1,0 },
        { 1,1, NE,NW,SE,SW, -1,0, 0,-1, 0,0, 1,0, 0,1, 1,1, 0,0 },
        { 0,0, SW,NW,SE,NE, 0,+1, +1,0, 1,1, 1,0, 0,1, 0,0, -1,-1 },
        { 1,0, SE,NE,SW,NW, 0,+1, -1,0, 0,1, 0,0, 1,1, 1,0, 0,-1 },
        { 0,1, NW,SW,NE,SE, 0,-1, +1,0, 1,0, 1,1, 0,0, 0,1, -1,0 },
        { 1,1, NE,SE,NW,SW, 0,-1, -1,0, 0,0, 0,1, 1,0, 1,1, 0,0 },
    };
    struct orient_param const *orient = orient_param+o;

    mx.x = obj[0].rot.x.x * (TILE_LEN<<8);
    mx.y = obj[0].rot.y.x * (TILE_LEN<<8);
    mx.z = obj[0].rot.z.x * (TILE_LEN<<8);
    my.x = obj[0].rot.x.y * (TILE_LEN<<8);
    my.y = obj[0].rot.y.y * (TILE_LEN<<8);
    my.z = obj[0].rot.z.y * (TILE_LEN<<8);
    mz.x = obj[0].rot.x.z * (TILE_LEN<<8);
    mz.y = obj[0].rot.y.z * (TILE_LEN<<8);
    mz.z = obj[0].rot.z.z * (TILE_LEN<<8);

    // On what tile are we ? (beware that obj[0].pos might be < 0)
    camera_x = (int)((obj[0].pos.x + (MAP_LEN<<(LOG_TILE_LEN-1)))/ TILE_LEN);
    camera_y = (int)((obj[0].pos.y + (MAP_LEN<<(LOG_TILE_LEN-1)))/ TILE_LEN);

    // Position of the lowest corner of the square to render
    int const corner_x = camera_x - (VIEW_LEN>>1);
    int const corner_y = camera_y - (VIEW_LEN>>1);

    // The corners location, relative to camera, in 24:8 fixed prec
    int corners[3][4];
    cam_to_tile(&corners[0][SW], corner_x, corner_y);
    cam_to_tile(&corners[0][NW], corner_x, corner_y+VIEW_LEN);
    cam_to_tile(&corners[0][SE], corner_x+VIEW_LEN, corner_y);
    cam_to_tile(&corners[0][NE], corner_x+VIEW_LEN, corner_y+VIEW_LEN);

    nb_defered_tiles = 0;

    render_map(
        corner_x, corner_y, orient,
        VIEW_LEN_N,
        3, corners, 6,
        get_map_z, render_map_tile);

    /* Now that the picture is almost done, render the objects that were so close
     * to the camera that we refused to draw them when encountered (see note above
     * about the "mostly enough")
     */
    for (int i = 0; i < nb_defered_tiles; i++) {
        renderer(defered_tiles[i], SKY);
    }
}

#define Gourovf 8   // NE PAS CHANGER !
#define Gourovfm (1>>Gourovf)
static int Gouroxi, Gouroyi, Gourolx, Gouroql, Gouroqx, Gouroqr, Gouroqg, Gouroqb, Gourocoulr, Gourocoulg, Gourocoulb, *Gourovid, Gourody, Gouroir, Gouroig, Gouroib;

void gouraud_section(void) {
    int cr, cg, cb;
    if (Gouroyi<0) {
        if (Gouroyi<-Gourody) {
            Gouroyi+=Gourody;
            Gouroxi+=Gourody*Gouroqx;
            Gourolx+=Gourody*Gouroql;
            Gourocoulr+=Gourody*Gouroqr;
            Gourocoulg+=Gourody*Gouroqg;
            Gourocoulb+=Gourody*Gouroqb;
            return;
        } else {
            Gouroxi-=Gouroyi*Gouroqx;
            Gourolx-=Gouroyi*Gouroql;
            Gourocoulr-=Gouroyi*Gouroqr;
            Gourocoulg-=Gouroyi*Gouroqg;
            Gourocoulb-=Gouroyi*Gouroqb;
            Gourody+=Gouroyi;
            Gouroyi=0;
        }
    }
    Gourovid=(int*)videobuffer+Gouroyi*SX;
    while (Gourody>0 && Gouroyi<SY) {
        int i=Gouroxi>>Gourovf;
        int ilim=i-(Gourolx>>Gourovf);
        if (ilim<0) ilim=0;
        cr=Gourocoulr;
        cg=Gourocoulg;
        cb=Gourocoulb;
        if (i>=SX) {
            cr+=(i-SX+1)*Gouroir;
            cg+=(i-SX+1)*Gouroig;
            cb+=(i-SX+1)*Gouroib;
            i=SX-1;
        }
        if (i>=ilim) {
            do {
                Gourovid[i]=((cb>>Gourovf)&0xFF)+(((cg>>Gourovf)&0xFF)<<8)+(((cr>>Gourovf)&0xFF)<<16);
                i--;
                cr+=Gouroir; cb+=Gouroib; cg+=Gouroig;
            } while (i>=ilim);
        }
        Gouroxi+=Gouroqx;
        Gourolx+=Gouroql;
        Gourocoulr+=Gouroqr;
        Gourocoulg+=Gouroqg;
        Gourocoulb+=Gouroqb;
        Gourovid+=SX;
        Gourody--;
        Gouroyi++;
    }
}

bool poly_gouraud(struct vect2dc *p1, struct vect2dc *p2, struct vect2dc *p3) {
    struct vect2dc *tmp, *p_maxx, *p_minx;
    int q1, q2, q3=0, qxx, ql2;
    int qr1, qr2, qr3=0, qg1, qg2, qg3=0, qb1, qb2, qb3=0, qrr, qgg, qbb;
    // order points in ascending Y
    if (p2->v.y<p1->v.y) { tmp=p1; p1=p2; p2=tmp; }
    if (p3->v.y<p1->v.y) { tmp=p1; p1=p3; p3=tmp; }
    if (p3->v.y<p2->v.y) { tmp=p2; p2=p3; p3=tmp; }
    if (p3->v.y<0 || p1->v.y>SY) return false;
    // bounds along X
    p_minx=p_maxx=p1;
    if (p2->v.x>p_maxx->v.x) p_maxx=p2;
    if (p3->v.x>p_maxx->v.x) p_maxx=p3;
    if (p_maxx->v.x<0) return false;
    if (p2->v.x<p_minx->v.x) p_minx=p2;
    if (p3->v.x<p_minx->v.x) p_minx=p3;
    if (p_minx->v.x>SX) return false;
    Gouroyi=p1->v.y;
    if (p1->v.y!=p2->v.y) {
        Gouroxi=p1->v.x<<Gourovf;
        Gourocoulb=p1->c.b<<Gourovf;
        Gourocoulg=p1->c.g<<Gourovf;
        Gourocoulr=p1->c.r<<Gourovf;
        q1=((p2->v.x-p1->v.x)<<Gourovf)/(p2->v.y-p1->v.y);
        qr1=(((int)(p2->c.r-p1->c.r))<<Gourovf)/(p2->v.y-p1->v.y);
        qg1=(((int)(p2->c.g-p1->c.g))<<Gourovf)/(p2->v.y-p1->v.y);
        qb1=(((int)(p2->c.b-p1->c.b))<<Gourovf)/(p2->v.y-p1->v.y);
        q2=((p3->v.x-p1->v.x)<<Gourovf)/(p3->v.y-p1->v.y);
        qr2=(((int)(p3->c.r-p1->c.r))<<Gourovf)/(p3->v.y-p1->v.y);
        qg2=(((int)(p3->c.g-p1->c.g))<<Gourovf)/(p3->v.y-p1->v.y);
        qb2=(((int)(p3->c.b-p1->c.b))<<Gourovf)/(p3->v.y-p1->v.y);
        if (p3->v.y != p2->v.y) {
            q3=((p3->v.x-p2->v.x)<<Gourovf)/(p3->v.y-p2->v.y);
            qr3=(((int)(p3->c.r-p2->c.r))<<Gourovf)/(p3->v.y-p2->v.y);
            qg3=(((int)(p3->c.g-p2->c.g))<<Gourovf)/(p3->v.y-p2->v.y);
            qb3=(((int)(p3->c.b-p2->c.b))<<Gourovf)/(p3->v.y-p2->v.y);
        }
        Gourolx = Gourovfm;
        if (q1<=q2) {
            if(0 == (Gouroql=q2-q1)) Gouroql=1;     // increasing ratio of the scanline length, avoiding 0
            if(0 == (ql2=q2-q3)) ql2=-1;
            Gouroqx=q2; qxx=q2;
            Gouroqr=qr2; Gouroqg=qg2; Gouroqb=qb2; qrr=qr2; qgg=qg2; qbb=qb2;
            if (p2->v.y-p1->v.y > p3->v.y-p2->v.y) {
#define QLPREC (Gourovfm/4)
                if (Gouroql>QLPREC) {
                    Gouroir=((qr1-qr2)<<Gourovf)/Gouroql;
                    Gouroig=((qg1-qg2)<<Gourovf)/Gouroql;
                    Gouroib=((qb1-qb2)<<Gourovf)/Gouroql;
                } else Gouroir=Gouroig=Gouroib=0;
            } else {
                if (ql2<-QLPREC) {
                    Gouroir=((qr3-qr2)<<Gourovf)/ql2;
                    Gouroig=((qg3-qg2)<<Gourovf)/ql2;
                    Gouroib=((qb3-qb2)<<Gourovf)/ql2;
                } else Gouroir=Gouroig=Gouroib=0;
            }
        } else {    // q1>q2
            if (0 == (Gouroql=q1-q2)) Gouroql=1;
            if (0 == (ql2=q3-q2)) ql2=-1;
            Gouroqx=q1; qxx=q3;
            Gouroqr=qr1; Gouroqg=qg1; Gouroqb=qb1; qrr=qr3; qgg=qg3; qbb=qb3;
            if (p2->v.y-p1->v.y > p3->v.y-p2->v.y) {
                if (Gouroql>QLPREC) {
                    Gouroir=((qr2-qr1)<<Gourovf)/Gouroql;
                    Gouroig=((qg2-qg1)<<Gourovf)/Gouroql;
                    Gouroib=((qb2-qb1)<<Gourovf)/Gouroql;
                } else Gouroir=Gouroig=Gouroib=0;
            } else {
                if (ql2<-QLPREC) {
                    Gouroir=((qr2-qr3)<<Gourovf)/ql2;
                    Gouroig=((qg2-qg3)<<Gourovf)/ql2;
                    Gouroib=((qb2-qb3)<<Gourovf)/ql2;
                } else Gouroir=Gouroig=Gouroib=0;
            }
        }
        Gourody=p2->v.y-p1->v.y;
        gouraud_section();
        Gouroqx=qxx; Gouroql=ql2;
        Gouroqr=qrr; Gouroqg=qgg; Gouroqb=qbb;
        Gourody=p3->v.y-p2->v.y+1;
        gouraud_section();
    } else {    // flat base
        if (p3->v.y > p2->v.y) {    // a triangle nonetheless
            q2=((p3->v.x-p1->v.x)<<Gourovf)/(p3->v.y-p1->v.y);
            qr2=(((int)(p3->c.r-p1->c.r))<<Gourovf)/(p3->v.y-p1->v.y);
            qg2=(((int)(p3->c.g-p1->c.g))<<Gourovf)/(p3->v.y-p1->v.y);
            qb2=(((int)(p3->c.b-p1->c.b))<<Gourovf)/(p3->v.y-p1->v.y);
            q3=((p3->v.x-p2->v.x)<<Gourovf)/(p3->v.y-p2->v.y);
            qr3=(((int)(p3->c.r-p2->c.r))<<Gourovf)/(p3->v.y-p2->v.y);
            qg3=(((int)(p3->c.g-p2->c.g))<<Gourovf)/(p3->v.y-p2->v.y);
            qb3=(((int)(p3->c.b-p2->c.b))<<Gourovf)/(p3->v.y-p2->v.y);
            if (p2->v.x >= p1->v.x) {
                Gouroxi=p2->v.x<<Gourovf;
                Gourocoulb=p2->c.b<<Gourovf;
                Gourocoulg=p2->c.g<<Gourovf;
                Gourocoulr=p2->c.r<<Gourovf;
                Gourolx = (p2->v.x-p1->v.x)<<Gourovf;
                if(!(Gouroql=q3-q2)) Gouroql=-1;
                Gouroqx=q3;
                Gouroqr=qr3; Gouroqg=qg3; Gouroqb=qb3;
                if (Gouroql<-QLPREC) {
                    Gouroir=((qr2-qr3)<<Gourovf)/Gouroql;
                    Gouroig=((qg2-qg3)<<Gourovf)/Gouroql;
                    Gouroib=((qb2-qb3)<<Gourovf)/Gouroql;
                } else Gouroir=Gouroig=Gouroib=0;
            } else {
                Gouroxi=p1->v.x<<Gourovf;
                Gourocoulb=p1->c.b<<Gourovf;
                Gourocoulg=p1->c.g<<Gourovf;
                Gourocoulr=p1->c.r<<Gourovf;
                Gourolx = (p1->v.x-p2->v.x)<<Gourovf;
                if(0 == (Gouroql=q2-q3)) Gouroql=-1;
                Gouroqx=q2;
                Gouroqr=qr2; Gouroqg=qg2; Gouroqb=qb2;
                if (Gouroql < -QLPREC) {
                    Gouroir=((qr3-qr2)<<Gourovf)/Gouroql;
                    Gouroig=((qg3-qg2)<<Gourovf)/Gouroql;
                    Gouroib=((qb3-qb2)<<Gourovf)/Gouroql;
                } else Gouroir=Gouroib=Gouroig=0;
            }
            Gourody=p3->v.y-p1->v.y+1;
            gouraud_section();
        } else {    // flat segment
            Gouroxi=p_maxx->v.x<<Gourovf;
            if(!(Gourolx=(p_maxx->v.x-p_minx->v.x)<<Gourovf)) Gourolx=Gourovfm;
            Gourocoulb=p_maxx->c.b<<Gourovf;
            Gourocoulg=p_maxx->c.g<<Gourovf;
            Gourocoulr=p_maxx->c.r<<Gourovf;
            if (Gourolx > QLPREC) {
                Gouroir=(p_minx->c.r-p_maxx->c.r)<<Gourovf/Gourolx;
                Gouroig=(p_minx->c.g-p_maxx->c.g)<<Gourovf/Gourolx;
                Gouroib=(p_minx->c.b-p_maxx->c.b)<<Gourovf/Gourolx;
            } else Gouroir=Gouroig=Gouroib=0;
            Gourody=1;
            gouraud_section();
        }
    }
    return true;
}


// return the ground altitude at this position
// FIXME: we should max out the precision extracted from the coordinates
float z_ground(float x, float y, bool with_submap)
{
    // This starts as above
    // xx, yy: 28:4 fixed prec coordinates
    int const xx = x*16. + (((MAP_LEN<<LOG_TILE_LEN)>>1)<<4);
    int const yy = y*16. + (((MAP_LEN<<LOG_TILE_LEN)>>1)<<4);
    // xi, yi: the tile we're in
    int const xi = xx>>(LOG_TILE_LEN+4);
    int const yi = yy>>(LOG_TILE_LEN+4);
    // medx, medy: location within the tile, in 0:16 fixed prec
    int const medx = xx & ((TILE_LEN<<4)-1);
    int const medy = yy & ((TILE_LEN<<4)-1);
    // z1, z2, z3, z4: the altitudes of the 4 corner of this tile, in 8:0 fixed prec
    int const i = xi + (yi<<LOG_MAP_LEN);
    int const z1 = (int)map[i].z;
    int const z2 = (int)map[i+MAP_LEN].z;
    int const z3 = (int)map[i+MAP_LEN+1].z;
    int const z4 = (int)map[i+1].z;
    // zi, zj: the altitudes in the left and right edges of the tile when y=medy, in 0:16*8:0=8:16 fixed prec
    int const zi = medy*(z2-z1) + (z1<<16);
    int const zj = medy*(z3-z4) + (z4<<16);
    // then the altitude in between these points when x=medx, in (0:16>>6)*(8:16>>6) = (8:32>>12) = 8:20, then further degraded into 8:16
    int const z_base = (((medx>>6)*((zj-zi)>>6))>>4) + zi;
    float const z_base_f = z_base / 1024.;  // we want altitude * 64.
    if (! with_submap) return z_base_f;

    // iix, iiy: coord within the tile in 0:3 fixed prec, ie subtile we are in in submap si
    unsigned const si = submap_get(i);
    int const iix = (medx<<SUBMAP_LEN_N)>>(LOG_TILE_LEN+4);
    int const iiy = (medy<<SUBMAP_LEN_N)>>(LOG_TILE_LEN+4);
    int const ii = iix+(iiy<<SUBMAP_LEN_N);
    // mz1 is the nord-west corner of the submap, the only one we can fetch for sure from si, in 8:0 fixed prec
    int const mz1 = submap[si][ii];
    int mz2, mz3, mz4;
    if (iiy != SUBMAP_LEN-1) {  // not last row
        mz2 = submap[si][ii+SUBMAP_LEN];
        if (iix != SUBMAP_LEN-1) { // not last column
            mz3 = submap[si][ii+SUBMAP_LEN+1];
            mz4 = submap[si][ii+1];
        } else {
            unsigned const si1 = submap_get(i+1);
            mz3 = submap[si1][(iiy+1)<<SUBMAP_LEN_N];
            mz4 = submap[si1][iiy<<SUBMAP_LEN_N];
        }
    } else {    // last row
        unsigned const siw = submap_get(i+MAP_LEN);
        mz2 = submap[siw][iix];
        if (iix!=SUBMAP_LEN-1) {
            mz3 = submap[siw][iix+1];
            mz4 = submap[si][ii+1];
        } else {
            unsigned const si1 = submap_get(i+1);
            unsigned const siw1 = submap_get(i+MAP_LEN+1);
            mz3 = submap[siw1][0];
            mz4 = submap[si1][iiy<<SUBMAP_LEN_N];
        }
    }
    // minx, miny: our location within the subtile, in 0:13
    int const minx = medx & (((TILE_LEN<<4)>>SUBMAP_LEN_N)-1);
    int const miny = medy & (((TILE_LEN<<4)>>SUBMAP_LEN_N)-1);
    // mzi, mzj: altitudes of the left and right edge of the submap, in 0:13*8:0 = 8:13 fixed prec
    int const mzi = miny*(mz2-mz1) + (mz1<<13);
    int const mzj = miny*(mz3-mz4) + (mz4<<13);
    // now the altitude in the middle of mzi..mzj, in (0:13>>4)*(8:13>>4) = (8:26>>8) = 8:18
    int const z = (minx>>4)*((mzj-mzi)>>4) + (mzi<<5);
    // add submap altitude (divided by 16) to ground level altitude
    float const ret = z_base_f + (z/(16.*4096.));   // return z * 64.
    return ret;
}

