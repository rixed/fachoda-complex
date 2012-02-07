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
#include "map.h"

int zoom=200,xcarte=0,ycarte=0,repidx=0;
vector repere[NBREPMAX];
void plotchar(int x, int y, int c, uchar m) {
    pcharlent(m+16,x-3,y-SizeCharY/2,c);
}
void rendumap(void)
{
    static unsigned my_imgcount = 0;
    vect2dlum p1,p2;
    int o,i;
    int c[4]={ 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00 };
    my_imgcount ++;
    for (i=0; i<3; i++) for (o=0; o<4; o++) {
        if (ViewAll || o==bot[bmanu].camp) plotchar(_DX+(obj[babaseo[0][i][o]].pos.x-ECHELLE*xcarte)*zoom/(ECHELLE*WMAP/2),_DY-(obj[babaseo[0][i][o]].pos.y-ECHELLE*ycarte)*zoom/(ECHELLE*WMAP/2),c[o],10);
    }
    for (o=0; o<NBVILLAGES; o++) {
        int x,y;
        plotchar(x=_DX+(village[o].p.x-ECHELLE*xcarte)*zoom/(ECHELLE*WMAP/2),y=_DY-(village[o].p.y-ECHELLE*ycarte)*zoom/(ECHELLE*WMAP/2),0xFFFFFF,11);
        pwordlent(village[o].nom,x-3*strlen(village[o].nom),y+9,0xFFFFFF);
    }
    for (o=0; o<NBTANKBOTS; o++) {
        if (vehic[o].camp!=-1 && (ViewAll||vehic[o].camp==bot[bmanu].camp)) plotchar(_DX+(obj[vehic[o].o1].pos.x-ECHELLE*xcarte)*zoom/(ECHELLE*WMAP/2),_DY-(obj[vehic[o].o1].pos.y-ECHELLE*ycarte)*zoom/(ECHELLE*WMAP/2),c[(int)vehic[o].camp],14);
    }
    for (o=0; o<NBBOT; o++) {
        if (bot[o].camp!=-1 && (ViewAll||bot[o].camp==bot[bmanu].camp)) plotchar(_DX+(obj[bot[o].vion].pos.x-ECHELLE*xcarte)*zoom/(ECHELLE*WMAP/2),_DY-(obj[bot[o].vion].pos.y-ECHELLE*ycarte)*zoom/(ECHELLE*WMAP/2),o==visubot && my_imgcount&1?0xFFFFFF:c[(int)bot[o].camp],12);
    }
    for (o=0; o<NBREPMAX; o++) {
        if (repere[o].x!=MAXFLOAT) plotchar(_DX+(repere[o].x-ECHELLE*xcarte)*zoom/(ECHELLE*WMAP/2),_DY-(repere[o].y-ECHELLE*ycarte)*zoom/(ECHELLE*WMAP/2),0xBFEF9F,9);
    }
    for (o=0; o<NBPRIMES; o++) {
        if (prime[o].reward && prime[o].camp==bot[visubot].camp)
            plotchar(_DX+(obj[prime[o].no].pos.x-ECHELLE*xcarte)*zoom/(ECHELLE*WMAP/2),_DY-(obj[prime[o].no].pos.y-ECHELLE*ycarte)*zoom/(ECHELLE*WMAP/2),0xFF00FF,13);
    }
    plotchar(_DX+(bot[visubot].u.x-ECHELLE*xcarte)*zoom/(ECHELLE*WMAP/2),_DY-(bot[visubot].u.y-ECHELLE*ycarte)*zoom/(ECHELLE*WMAP/2),0xFFFF,'x'-16);
    pchar('O',10,SY-SizeCharY*2-10,0xF0F0F0);
    pchar('E',22,SY-SizeCharY*2-10,0xF0F0F0);
    pchar('N',16,SY-SizeCharY*3-10,0xF0F0F0);
    pchar('S',16,SY-SizeCharY-10,0xF0F0F0);
    pchar(15+16,16,SY-SizeCharY*2-10,0xA0A0A0);
    p1.v.x=19; p1.v.y=SY-SizeCharY*2+SizeCharY/2-10;
    p2.v.x=10*cos(bot[visubot].cap)+p1.v.x;
    p2.v.y=-10*sin(bot[visubot].cap)+p1.v.y;
    drawline(&p1.v, &p2.v, c[(int)bot[visubot].camp]);
}
void bpoint(vect2dc *p, int x, int y) {
    int z, intens;
    z=map[x+(y<<NWMAP)].z;
    intens=((z-(x?map[x-1+(y<<NWMAP)].z:0)))+32+64;
    if (intens<64) intens=64;
    else if (intens>127) intens=127;
    p->c.r=(zcol[z].r*intens)>>7;
    p->c.g=(zcol[z].g*intens)>>7;
    p->c.b=(zcol[z].b*intens)>>7;
    p->v.y=_DY+zoom-zoom*2*(y-ycarte)/WMAP;
    p->v.x=_DX-zoom+zoom*2*(x-xcarte)/WMAP;
}
void renduroute() {
    int i,typ=0;
    int coulr[3][2]= {
        {0xBFBF4F,0x909060},
        {0xA0A0A0,0x808080},
        {0x503010,0x503010}
    };
    vect2dlum p1,p2;
    for (i=0; i<routeidx-1; i++) {
        if (i>EndMotorways) typ=1;
        if (i>EndRoads) typ=2;
        if (route[i].ak!=-1) {
            p1.v.x=_DX+(route[i].i.x-ECHELLE*xcarte)*zoom/(ECHELLE*WMAP/2);
            p1.v.y=_DY-(route[i].i.y-ECHELLE*ycarte)*zoom/(ECHELLE*WMAP/2);
            if (route[i+1].ak!=-1) {
                p2.v.x=_DX+(route[i+1].i.x-ECHELLE*xcarte)*zoom/(ECHELLE*WMAP/2);
                p2.v.y=_DY-(route[i+1].i.y-ECHELLE*ycarte)*zoom/(ECHELLE*WMAP/2);
                drawline(&p1.v, &p2.v, coulr[typ][i&1]);
            }
        }
    }
}
void rendumapbg(void) {
    int x,y;
    vect2dc p1,p2,p3,p4;
    for (y=0; y<WMAP-1; y++) {
        for (x=0; x<WMAP-1; x++) {
            bpoint(&p1,x,y);
            bpoint(&p2,x+1,y);
            bpoint(&p3,x,y+1);
            bpoint(&p4,x+1,y+1);
            poly_gouraud(&p1,&p2,&p3);
            poly_gouraud(&p3,&p2,&p4);
        }
    }
    bpoint(&p1,0,0);
    bpoint(&p2,WMAP-1,WMAP-1);
    for (y=0; y<SY; y++) {
        if (p1.v.x>0) MMXMemSetInt((int*)videobuffer+y*SX,0,p1.v.x);
        if (p2.v.x<SX) MMXMemSetInt((int*)videobuffer+y*SX+p2.v.x-1,0,SX-p2.v.x+1);
        if (y<p2.v.y) MMXMemSetInt((int*)videobuffer+y*SX+MAX(0,p1.v.x),0,MIN(p2.v.x,SX)-MAX(0,p1.v.x));
        if (y>p1.v.y) MMXMemSetInt((int*)videobuffer+y*SX+MAX(0,p1.v.x),0,MIN(p2.v.x,SX)-MAX(0,p1.v.x));
    }
    renduroute();
}
