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
#include "proto.h"
#define SunMapX 64
#define SunMapY 32
#define SunImgL 40
#define SunNbRays 9

static struct pixel32 SunMap[SunMapX * (SunMapY+1)];
static double SunPicPh[20], SunPicOmega[20];
static int SunImgAdy[SunImgL*SunImgL];

static struct {
    float x, y, z, xy;
} stars[80];

void initsol(void) {
    double r,teta;
    for (int y=-SunImgL/2; y<SunImgL/2; y++)
        for (int x=-SunImgL/2; x<SunImgL/2; x++) {
            r = sqrt(x*x+y*y);
            teta = x ? (atan(-y/(double)x) + (x<0 ? M_PI : 0)) : (y>0 ? -M_PI/2 : M_PI/2);
            while (teta<0 || teta>2*M_PI) { // A REFAIRE AVEC FMOD !
                if (teta<0) teta+=2*M_PI;
                if (teta>2*M_PI) teta -= 2*M_PI;
            }
            SunImgAdy[(y+SunImgL/2)*SunImgL+x+SunImgL/2] =
                r < SunImgL/2 ?
                    SunMapX*(int)(SunMapY*r/(SunImgL*.5)) + SunMapX*teta/(2*M_PI) :
                    -1; // mask color
        }
    for (unsigned i=0; i<20; i++) {
        SunPicPh[i] = drand48()*2*M_PI;
        SunPicOmega[i] = drand48()*0.4 + 0.3;
    }

    // The stars
    for (unsigned i = 0; i < ARRAY_LEN(stars); i++){
        double teta, phi;
        do teta = drand48()*M_PI*.4; while (drand48()>sin(teta));
        phi = drand48()*M_PI*2;
        stars[i].x = 1e4 * sin(teta) * cos(phi);
        stars[i].y = 1e4 * sin(teta) * sin(phi);
        stars[i].z = 1e4 * cos(teta);
        stars[i].xy = stars[i].x * stars[i].y;
    }
}

void animsoleil(void) {
    for (unsigned i = 0; i < SunNbRays; i++) {
        SunPicPh[i] += 0.2 * drand48();
    }
}

void affsoleil(struct vector *L) {
    struct vector soleil;
    struct vector ac;
    if (night_mode) for (unsigned i = 0; i < ARRAY_LEN(stars); i++) {
        // Compute star's position relative to camera (CamT*Start)
        float gh = obj[0].rot.z.x * obj[0].rot.z.y;
        float ab = obj[0].rot.x.x * obj[0].rot.x.y;
        float de = obj[0].rot.y.x * obj[0].rot.y.y;
        ac.z =
            (obj[0].rot.z.x + stars[i].y) *
            (obj[0].rot.z.y + stars[i].x)
            - gh - stars[i].xy
            + obj[0].rot.z.z * stars[i].z;
        if (ac.z > 0) {
            ac.x =
                (obj[0].rot.x.x + stars[i].y) *
                (obj[0].rot.x.y + stars[i].x)
                - ab - stars[i].xy
                + obj[0].rot.x.z * stars[i].z;
            int x = proj1(ac.x, ac.z);
            if (x >= -win_center_x && x < win_center_x-1) {
                ac.y =
                    (obj[0].rot.y.x + stars[i].y) *
                    (obj[0].rot.y.y + stars[i].x)
                    - de - stars[i].xy
                    + obj[0].rot.y.z * stars[i].z;
                int y = proj1(ac.y, ac.z);
                if (y >= -win_center_y && y < win_center_y-1) {
                    plot(x,   y-1, 0xA0A0A0);
                    plot(x+1, y-1, 0x909090);
                    plot(x-1, y,   0xA0A0A0);
                    plot(x,   y,   0xF0F0F0);
                    plot(x+1, y,   0xB0B0B0);
                    plot(x+2, y,   0x808080);
                    plot(x-1, y+1, 0x909090);
                    plot(x,   y+1, 0xB0B0B0);
                    plot(x+1, y+1, 0xA0A0A0);
                    plot(x+2, y+1, 0x909090);
                    plot(x,   y+2, 0x808080);
                    plot(x+1, y+2, 0x606060);
                }
            }
        }
    }
    copyv(&soleil,L);
    neg(&soleil);
    mulv(&soleil,1e10);
    // quelle position pour le soleil ?
    ac.z = scalaire(&obj[0].rot.z,&soleil);
    if (ac.z > 0) {
        ac.x = scalaire(&obj[0].rot.x,&soleil);
        int xse = win_center_x+(ac.x*z_near)/ac.z-SunImgL/2;
        if (xse>=-SunImgL && xse<win_width) {
            ac.y = scalaire(&obj[0].rot.y,&soleil);
            int yse = win_center_y+(ac.y*z_near)/ac.z-SunImgL/2;
            if (yse>=-SunImgL && yse<win_height) {
                // AFICHAGE DU SOLEIL
                if (! night_mode) for (int i=0; i < SunNbRays; i++) {
                    for (int x=(i*SunMapX)/SunNbRays; x<((i+1)*SunMapX)/SunNbRays; x++) {
                        int y;
                        for (y=0; (y<SunMapY*.3+SunMapY*.3*(sin(SunPicPh[i])+1.5)*sin(x*M_PI*SunNbRays/SunMapX)*sin(x*M_PI*SunNbRays/SunMapX)) && y<SunMapY; y++) {
                            SunMap[y*SunMapX+x].r = (255-3*y);
                            SunMap[y*SunMapX+x].g = (255-5*y);
                            SunMap[y*SunMapX+x].b = ((200-10*y)>0?(200-10*y):0);
                        }
                        for (; y <= SunMapY; y++) {
                            SunMap[y*SunMapX+x].r = 0xA0;
                            SunMap[y*SunMapX+x].g = 0xA0;
                            SunMap[y*SunMapX+x].b = 0xC0;
                        }
                    }
                } else /* night_mode */ for (int i = 0; i < SunNbRays; i++) {
                    for (int x=(i*SunMapX)/SunNbRays; x<((i+1)*SunMapX)/SunNbRays; x++) {
                        int y;
                        for (y=0; (y<SunMapY*.5+SunMapY*.1*(sin(SunPicPh[i])+1.5)*sin(x*M_PI*SunNbRays/SunMapX)*sin(x*M_PI*SunNbRays/SunMapX)) && y<SunMapY; y++) {
                            SunMap[y*SunMapX+x].r = y<SunMapY*.5?255-3*y:170-3*y;
                            SunMap[y*SunMapX+x].g = y<SunMapY*.5?255-3*y:170-3*y;
                            SunMap[y*SunMapX+x].b = y<SunMapY*.5?255-y:170-3*y;
                        }
                        for (; y <= SunMapY; y++) {
                            SunMap[y*SunMapX+x].r = 0x20;
                            SunMap[y*SunMapX+x].g = 0x20;
                            SunMap[y*SunMapX+x].b = 0x40;
                        }
                    }
                }
                for (int y=0; y<SunImgL; y++) {
                    if (y+yse>=0 && y+yse<win_height) {
                        for (int x=0; x<SunImgL; x++) {
                            if (x+xse>=0 && x+xse<win_width) {
                                int a = SunImgAdy[y*SunImgL+x];
                                if (a != -1) {
                                    videobuffer[(y+yse)*win_width+x+xse] = SunMap[a];
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
