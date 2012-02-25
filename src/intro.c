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
#include <jpeglib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "keycodesdef.h"
#include "proto.h"
#include "sound.h"
#include "gtime.h"

extern void deltatime(void);
extern void plotfumee(int,int,int);
static struct {
    char nbkases;
    struct {
        char *label;
        char nxtround;
    } kase[9];
} Round []= {
    {   // cercle 0
        3,
        {
            {"Options",16},
            {"Quit",10},
            {"Go !",-1}
        }
    }, {    // cercle 1 game type
        8,
        {
            {"Strategy",4},
            {"What Plane",5},
            {"Number of Drones",-4},
            {"Number of Tanks",-5},
            {"Nation",6},
            {"Day or Night ?",7},
            {"When Dead...",15},
            {"Back",16}
        }
    }, {    // cercle 2 techniks
        5,
        {
            {"Controls",8},
            {"KbControl Sensitivity",-7},
            {"KbControl Amortisment",-8},
            {"Language",9},
            {"Back",16}
        }
    }, {    // cercle 3 Cheat modes
        4,
        {
            {"Difficulty",11},
            {"Super-Sight Cheat",12},
            {"More Cheats",13},
            {"Back",16}
        }
    } , {   // cercle 4 Initial Position
        2,
        {
            {"Campain",-14},
            {"Kill'em All",-15}
        }
    }, {    // cercle 5 Plane
        6,
        {
            {"Snophill",-37},
            {"Dewoitine",-16},
            {"Bogoplane",-17},
            {"Corsair",-18},
            {"Spitflame",-34},
            {"Moshito",-19}
        }
    }, {    // cercle 6 Camp
        4,
        {
            {"Sierra Freedom",-20},
            {"D.T.V.U.",-21},
            {"Almouchie",-22},
            {"Low-Wanana",-23}
        }
    }, {    // cercle 7 Time
        2,
        {
            {"Day",-24},
            {"Night",-25}
        }
    }, {    // cercle 8 Controles
        2,
        {
            {"Mouse",-26},
            {"Keyboard",-27}
        }
    }, {    // cercle 9 Language
        2,
        {
            {"English",-28},
            {"Francais",-29}
        }
    }, {    // cercle 10 Quit
        2,
        {
            {"Yeah, sure!",-3},
            {"Well, maybe not...",0}
        }
    }, {    // cercle 11 Difficulty
        2,
        {
            {"easy_mode",-11},
            {"Normal",-12}
        }
    }, {    // cercle 12 Visibility
        2,
        {
            {"Spot all enemies",-13},
            {"Spot only friends",-30}
        }
    }, {    // cercle 13 MetaGruge
        2,
        {
            {"Alti Cheat",-31},
            {"No, too lame",-32}
        }
    }, {    // cercle 14 Pas dans le menu, just un boutton QUIT
        1,
        {
            {"OK",16}
        }
    },  {   // cercle 15, Mortal/Immoral?
        2,
        {
            {"Replace a drone",-33},
            {"Rest In Peace",-6}
        }
    }, {    // cercle 16 Options
        6,
        {
            {"Game Type", 1},
            {"Technics", 2},
            {"Redefine Keys", -2},
            {"Cheat", 3},
            {"Map", 17},
            {"Back", 0}
        }
    },  {   // cercle 17 Map
        3,
        {
            {"Hilly", -35},
            {"Smoothness", -36},
            {"Back", 16}
        }
    }
};

void drawseg(int x1, int x2, int y, int c) {
    if (x1<0) x1=0;
    if (x2>win_width-1) x2=win_width-1;
    if (y>=0 && y<win_height && x1<win_width && x2>=0)
        memset32((int*)videobuffer+x1+y*win_width,c,x2-x1+1);
}
void disqueintro(int x, int y, int r, int c) {
    int balance=-r, xoff=0, yoff=r, newyoff=1;
    if (r<=0 || x-r>=win_width || x+r<0 || y-r>=win_height || y+r<0 || r>win_width) return;
    do {
        if (newyoff) {
            drawseg(x-xoff,x+xoff,y+yoff,c);
            drawseg(x-xoff,x+xoff,y-yoff,c);
        }
        if (xoff!=yoff) {
            drawseg(x-yoff,x+yoff,y+xoff,c);
            if (xoff) drawseg(x-yoff,x+yoff,y-xoff,c);
        }
        if ((balance += xoff + xoff + 1) >= 0) {
            yoff --;
            balance -= yoff + yoff;
            newyoff=1;
        } else newyoff=0;
    } while (++xoff <= yoff);
}
#define RAYONBOUTTON 40
void button(int x, int y, char *label,char highlight) {
    disqueintro(x,y,RAYONBOUTTON,highlight?0x359FE0:0x258FD0);
    cercle(x-win_center_x,y-win_center_y,RAYONBOUTTON-3,0xD0D0D0);
    cercle(x-win_center_x,y-win_center_y,RAYONBOUTTON,0x103050);
    TextClipX1=x-RAYONBOUTTON+6;
    TextClipX2=x+RAYONBOUTTON-6;
    pstrlent(label,y-7,0xFFF0F0);
    TextClipX1=TextClipX2=0;
}

static int xb[10],yb[10], kzc;
static int agit=0;
static void draw_page(int r, float rayon, float phase) {
    int b;
    int SS = MAX(win_center_x,win_center_y);
    update_listener(&vec_zero, &vec_zero, &mat_id);
    affpresent(drand48()*(agit>>8),drand48()*(agit>>8));
    if (agit>256) agit=(agit*9)/10;
    for (b=0; b<Round[r].nbkases; b++) {    // compute buttons position
        float ang = phase + b*M_PI*2./Round[r].nbkases;
        xb[b] = win_center_x+SS*rayon*cos(ang);
        yb[b] = win_center_y+SS*rayon*sin(ang);
    }
    for (b=0; b<Round[r].nbkases; b++) {    // draw buttons shaddow
        int dx = xmouse-xb[b];
        int dy = ymouse-yb[b];
        plotfumee(xb[b]-dx*.2, yb[b]-dy*.2, RAYONBOUTTON*(.6+sqrt(dx*dx+dy*dy)*.5/SS));
    }
    for (b=0; b<Round[r].nbkases; b++) {    // draw buttons
        button(xb[b], yb[b], Round[r].kase[b].label, kzc==b);
    }
}
int kazeclick(int x, int y, int r) {
    int b;
    for (b=Round[r].nbkases-1; b>=0; b--) {
        if ((x-xb[b])*(x-xb[b])+(y-yb[b])*(y-yb[b])<RAYONBOUTTON*RAYONBOUTTON) return b;
    }
    return -1;
}
int jauge(int vi, int max) {
    int va=vi;
    int jx, y;
    float phaz=0;
    do {
        float const dt_sec = gtime_next_sec();
        jx = (va*(win_width-20))/max;
        kzc = kazeclick(xmouse,ymouse,14);
        draw_page(14, .45 + .2*sin(phaz*.61), .5*M_PI + .2*sin(phaz));
        phaz += 2.1*dt_sec;
        for (y=win_height/3-(win_height>>3); y<win_height/3+(win_height>>3); y++)
            memset32((int*)videobuffer+y*win_width+10,0x3060A0,jx);
        pbignum(va,win_center_x,win_height/3-SizeBigCharY/2,2,1,0);
        plotcursor(xmouse,ymouse);
        buffer2video();
        xproceed();
        if (kread(0) || kread(1)) {
            if (kzc==0) {
                playsound(VOICE_MOTOR, SAMPLE_BIPINTRO, 1+(drand48()-.5)*.05, &voices_in_my_head, true, false);
                agit=50*256;
                return va;
            }
            else if (xmouse>=10 && xmouse<win_width-10) va=((xmouse-10)*max)/(win_width-20);
        }
    } while (1);
}
/*void readstring(char *m) {
    float phaz=0;
    int curpos=strlen(m), i;
    char prompt[]="Type in name :                                             ";
    do {
        kzc=kazeclick(xmouse,ymouse,14);
        draw_page(14,.45+.2*sin(phaz*.61),.5*M_PI+0.2*sin(phaz));
        phaz+=.11;
        for (i=0; i+15<strlen(prompt) && i<strlen(m)+1; i++)
        prompt[i+15]=i==curpos?108+16:m[i];
        prompt[i+15]='\0';
        pstr(prompt,win_height/3,0xFFF020);
        plotcursor(xmouse,ymouse);
        buffer2video();
        //wait_sync();  !! INDISPENSABLE
        xproceed();
        if (kread(0) || kread(1)) {
            if (kzc==0) return;
        }

    } while (1);
}*/

int present(void) {
    int curround=0, oldcurround, nextround, etap=2;
    float phaz=drand48()*2*M_PI, rayon=2, phazr=drand48()*2*M_PI;
    gtime_start();
    do {
        float const dt_sec = gtime_next_sec();
        if (etap==1) {
            // explosion
            rayon += 9.*dt_sec;
            if (rayon > 1.5) {
                if (nextround<0) {
                    switch (nextround) {
                    case -1: return -1;
                    case -2: return 1;
                    case -4: NBBOT=jauge(NBBOT,100); break;
                    case -5: NBTANKBOTS=jauge(NBTANKBOTS,500); break;
                    case -7: CtlSensitiv=.01*jauge(100*CtlSensitiv,100); break;
                    case -8: CtlAmortis=.01*jauge(100*CtlAmortis,100); break;
                    case -6: redefinekeys(); break;
            /*      case -9: readstring(hostname);
                    case -10: readstring(myname);*/
                    case -11: hilly_level=300+exp(.1*jauge(log(hilly_level-300.)/.1,100)); break;
                    case -12: smooth_level=2+jauge(smooth_level-2,20); break;
                    }
                } else curround=nextround;
                etap=2;
            }
        } else if (etap==2) {
            // implosion
            rayon -= dt_sec * 2.3;
            phaz  += dt_sec * (.9 + sin(phazr)*6.1);
            phazr += dt_sec * (sin(phaz)*6.3 + sin(phazr*3.)*1.1);
            if (rayon <= .4) {
                etap = 0;
                rayon = .4;
            }
        }
        kzc = kazeclick(xmouse, ymouse, curround);
        draw_page(curround, rayon + .018*sin(phazr), phaz);
        plotcursor(xmouse, ymouse);
        buffer2video();
        xproceed();
        phaz += dt_sec * (.09 + sin(phazr)*.61);
        phazr += dt_sec * (sin(phaz*.1)*.63 + sin(phazr*.13)*.11);

        if (etap==0 && (kreset(0) || kreset(1))) {
            if (kzc!=-1) {
                struct vector mousepos = { .x = (float)xmouse/win_width, .y = (float)ymouse/win_height, .z = 0. };
                playsound(VOICE_MOTOR, SAMPLE_BIPINTRO, 1+(drand48()-.5)*.05, &mousepos, true, false);
                agit=20*256;
                if (Round[curround].kase[kzc].nxtround>=0) {
                    oldcurround=curround;
                    nextround=Round[curround].kase[kzc].nxtround;
                    etap=1;
                } else {
                    switch (Round[curround].kase[kzc].nxtround) {
                    case -1: etap=1; nextround=-2; break;
                    case -2: etap=1; nextround=-6; break;
                    case -3: etap=1; nextround=-1; break;
                    case -4: etap=1; nextround=-4; break;
                    case -5: etap=1; nextround=-5; break;
                    case -6: enable_resurrection=0; break;
                    case -7: etap=1; nextround=-7; break;
                    case -8: etap=1; nextround=-8; break;
                /*  case -9: etap=1; nextround=-9; break;
                    case -10: etap=1; nextround=-10; break;*/
                    case -11: easy_mode=1; break;
                    case -12: easy_mode=0; break;
                    case -13: enable_view_enemy=1; break;
                    case -14: killemall_mode=0; break;
                    case -15: killemall_mode=1; break;
                    case -16: starting_plane=1; break;
                    case -17: starting_plane=4; break;
                    case -18: starting_plane=2; break;
                    case -19: starting_plane=3; break;
                    case -20: camp=0; break;
                    case -21: camp=1; break;
                    case -22: camp=2; break;
                    case -23: camp=3; break;
                    case -24: night_mode=0; break;
                    case -25: night_mode=1; break;
                    case -26: enable_mouse=1; break;
                    case -27: enable_mouse=0; break;
                    case -28: lang=1; break;
                    case -29: lang=0; break;
                    case -30: enable_view_enemy=0; break;
                    case -31: cheat_mode=1; break;
                    case -32: cheat_mode=0; break;
                    case -33: enable_resurrection=1; break;
                    case -34: starting_plane=5; break;
                    case -35: etap=1; nextround=-11; break;
                    case -36: etap=1; nextround=-12; break;
                    case -37: starting_plane=6; break;
                    }
                    if (etap==0) {
                        nextround=oldcurround;
                        etap=1;
                    }
                }
            }
        }
    } while (1);
}
