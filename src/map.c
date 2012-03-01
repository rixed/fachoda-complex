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
#include <values.h>
#include "heightfield.h"

int zoom = 200, map_x = 0, map_y = 0;

// the player can set some marks on the map (barely useful in single player mode, though)
int next_mark_set = 0;
struct vector mark[NB_MARKS];

static void plotchar(int x, int y, int c, uchar m)
{
    pcharlent(m+16,x-3,y-SizeCharY/2,c);
}

static void draw_fg(void)
{
    static unsigned my_imgcount = 0;
    struct vect2dlum p1,p2;
    int o,i;
    int c[4]={ 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00 };
    my_imgcount ++;
    for (i=0; i<3; i++) for (o=0; o<4; o++) {
        if (enable_view_enemy || o==bot[controled_bot].camp) plotchar(win_center_x+(obj[airfield_obj[0][i][o]].pos.x-TILE_LEN*map_x)*zoom/(TILE_LEN*MAP_LEN/2),win_center_y-(obj[airfield_obj[0][i][o]].pos.y-TILE_LEN*map_y)*zoom/(TILE_LEN*MAP_LEN/2),c[o],10);
    }
    for (o=0; o<NB_VILLAGES; o++) {
        int x,y;
        plotchar(x=win_center_x+(village[o].p.x-TILE_LEN*map_x)*zoom/(TILE_LEN*MAP_LEN/2),y=win_center_y-(village[o].p.y-TILE_LEN*map_y)*zoom/(TILE_LEN*MAP_LEN/2),0xFFFFFF,11);
        pwordlent(village[o].nom,x-3*strlen(village[o].nom),y+9,0xFFFFFF);
    }
    for (o=0; o<NBTANKBOTS; o++) {
        if (tank[o].camp!=-1 && (enable_view_enemy||tank[o].camp==bot[controled_bot].camp)) plotchar(win_center_x+(obj[tank[o].o1].pos.x-TILE_LEN*map_x)*zoom/(TILE_LEN*MAP_LEN/2),win_center_y-(obj[tank[o].o1].pos.y-TILE_LEN*map_y)*zoom/(TILE_LEN*MAP_LEN/2),c[(int)tank[o].camp],14);
    }
    for (o=0; o<NBBOT; o++) {
        if (bot[o].camp!=-1 && (enable_view_enemy||bot[o].camp==bot[controled_bot].camp)) plotchar(win_center_x+(obj[bot[o].vion].pos.x-TILE_LEN*map_x)*zoom/(TILE_LEN*MAP_LEN/2),win_center_y-(obj[bot[o].vion].pos.y-TILE_LEN*map_y)*zoom/(TILE_LEN*MAP_LEN/2),o==viewed_bot && my_imgcount&1?0xFFFFFF:c[(int)bot[o].camp],12);
    }
    for (o=0; o<NB_MARKS; o++) {
        if (mark[o].x!=MAXFLOAT) plotchar(win_center_x+(mark[o].x-TILE_LEN*map_x)*zoom/(TILE_LEN*MAP_LEN/2),win_center_y-(mark[o].y-TILE_LEN*map_y)*zoom/(TILE_LEN*MAP_LEN/2),0xBFEF9F,9);
    }
    for (o=0; o<MAX_REWARDS; o++) {
        if (reward[o].amount && reward[o].camp==bot[viewed_bot].camp)
            plotchar(win_center_x+(obj[reward[o].no].pos.x-TILE_LEN*map_x)*zoom/(TILE_LEN*MAP_LEN/2),win_center_y-(obj[reward[o].no].pos.y-TILE_LEN*map_y)*zoom/(TILE_LEN*MAP_LEN/2),0xFF00FF,13);
    }
    plotchar(win_center_x+(bot[viewed_bot].u.x-TILE_LEN*map_x)*zoom/(TILE_LEN*MAP_LEN/2),win_center_y-(bot[viewed_bot].u.y-TILE_LEN*map_y)*zoom/(TILE_LEN*MAP_LEN/2),0xFFFF,'x'-16);
    pchar('O',10,win_height-SizeCharY*2-10,0xF0F0F0);
    pchar('E',22,win_height-SizeCharY*2-10,0xF0F0F0);
    pchar('N',16,win_height-SizeCharY*3-10,0xF0F0F0);
    pchar('S',16,win_height-SizeCharY-10,0xF0F0F0);
    pchar(15+16,16,win_height-SizeCharY*2-10,0xA0A0A0);
    p1.v.x=19; p1.v.y=win_height-SizeCharY*2+SizeCharY/2-10;
    p2.v.x=10*cos(bot[viewed_bot].cap)+p1.v.x;
    p2.v.y=-10*sin(bot[viewed_bot].cap)+p1.v.y;
    drawline(&p1.v, &p2.v, c[(int)bot[viewed_bot].camp]);
}

static void bpoint(struct vect2dc *p, int x, int y)
{
    int z, intens;
    z=map[x+(y<<LOG_MAP_LEN)].z;
    intens=((z-(x?map[x-1+(y<<LOG_MAP_LEN)].z:0)))+32+64;
    if (intens<64) intens=64;
    else if (intens>127) intens=127;
    p->c.r=(zcol[z].r*intens)>>7;
    p->c.g=(zcol[z].g*intens)>>7;
    p->c.b=(zcol[z].b*intens)>>7;
    p->v.y=win_center_y+zoom-zoom*2*(y-map_y)/MAP_LEN;
    p->v.x=win_center_x-zoom+zoom*2*(x-map_x)/MAP_LEN;
}

static void draw_roads()
{
    int i,typ=0;
    int coulr[3][2]= {
        {0xBFBF4F,0x909060},
        {0xA0A0A0,0x808080},
        {0x503010,0x503010}
    };
    struct vect2dlum p1,p2;
    for (i=0; i<routeidx-1; i++) {
        if (i>EndMotorways) typ=1;
        if (i>EndRoads) typ=2;
        if (route[i].ak!=-1) {
            p1.v.x=win_center_x+(route[i].i.x-TILE_LEN*map_x)*zoom/(TILE_LEN*MAP_LEN/2);
            p1.v.y=win_center_y-(route[i].i.y-TILE_LEN*map_y)*zoom/(TILE_LEN*MAP_LEN/2);
            if (route[i+1].ak!=-1) {
                p2.v.x=win_center_x+(route[i+1].i.x-TILE_LEN*map_x)*zoom/(TILE_LEN*MAP_LEN/2);
                p2.v.y=win_center_y-(route[i+1].i.y-TILE_LEN*map_y)*zoom/(TILE_LEN*MAP_LEN/2);
                drawline(&p1.v, &p2.v, coulr[typ][i&1]);
            }
        }
    }
}

static void draw_bg(void)
{
    int x,y;
    struct vect2dc p1,p2,p3,p4;
    for (y=0; y<MAP_LEN-1; y++) {
        for (x=0; x<MAP_LEN-1; x++) {
            bpoint(&p1,x,y);
            bpoint(&p2,x+1,y);
            bpoint(&p3,x,y+1);
            bpoint(&p4,x+1,y+1);
            poly_gouraud(&p1,&p2,&p3);
            poly_gouraud(&p3,&p2,&p4);
        }
    }
    bpoint(&p1,0,0);
    bpoint(&p2,MAP_LEN-1,MAP_LEN-1);
    for (y=0; y<win_height; y++) {
        if (p1.v.x>0) memset32((int*)videobuffer+y*win_width,0,p1.v.x);
        if (p2.v.x<win_width) memset32((int*)videobuffer+y*win_width+p2.v.x-1,0,win_width-p2.v.x+1);
        if (y<p2.v.y) memset32((int*)videobuffer+y*win_width+MAX(0,p1.v.x),0,MIN(p2.v.x,win_width)-MAX(0,p1.v.x));
        if (y>p1.v.y) memset32((int*)videobuffer+y*win_width+MAX(0,p1.v.x),0,MIN(p2.v.x,win_width)-MAX(0,p1.v.x));
    }
}

void map_draw(void)
{
    draw_bg();
    draw_roads();
    draw_fg();
}

