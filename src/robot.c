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
#include <values.h>
#include <assert.h>
#include "proto.h"
#include "robot.h"

bot_s *bot;
vehic_s *vehic;
voiture_s *voiture;
zep_s *zep;

char const *aerobatic_2_str(enum aerobatic aerobatic)
{
    switch (aerobatic) {
        case MANEUVER:   return "maneuver";
        case TURN:       return "turn";
        case RECOVER:    return "recover";
        case CLIMB_VERT: return "high climb";
        case TAIL:       return "follow 6s";
        case CLIMB:      return "climb";
    }
    assert(!"Invalid aerobatic");
    return "INVALID";
}

char const *maneuver_2_str(enum maneuver maneuver)
{
    switch (maneuver) {
        case PARKING:     return "parking";
        case TAXI:        return "taxi";
        case LINE_UP:     return "line up";
        case TAKE_OFF:    return "take off";
        case NAVIG:       return "navigation";
        case DIVE_N_BOMB: return "dive and bomb";
        case NOSE_UP:     return "nose up";
        case ILS_1:       return "ILS(1)";
        case ILS_2:       return "ILS(2)";
        case ILS_3:       return "ILS(3)";
        case EVADE:       return "evade";
        case HEDGEHOP:    return "hedgehop";
        case BOMBING:     return "bombing";
    }
    assert(!"Invalid maneuver");
    return "INVALID";
}

static double tirz(double dz, double d)
{
    double z=0, l=0;
    double dz2 = dz*dz;
    while (l<d && dz2<1) {
        z+=100.*dz;
        dz+=vec_g.z*.005;
        dz2+=(vec_g.z*.005*vec_g.z*.005)+vec_g.z*.01;
        l+=100.*sqrt(1-dz2);
    }
    return z;
}

/*
static double tircoli(vector v, vector u)
{
    // V est unitaire. Les tirs ont une vitesse CST
    vector g, l;
    double dd=MAXDOUBLE,d;
    copyv(&g,&vec_g);
    mulv(&g,.005);
    do {
        d=dd;
        copyv(&l,&v);
        mulv(&l,100);
        subv(&u,&l);
        addv(&v,&g);
        renorme(&v);
        dd=norme2(&u);
    } while (dd<d);
    return d;
}
*/

static double bombz2(vector v, vector u)
{
    vector g;
    double dd=MAXDOUBLE,d;
    copyv(&g,&vec_g);
    mulv(&g,G_FACTOR);
    do {
        d=dd;
        addv(&v,&g);
        mulv(&v,.999);
        subv(&u,&v);
        dd=norme2(&u);
    } while (dd<d);
    return d;
}

// Look for a flying target for tank v
static int vehic_new_flying_target(int v)
{
    vector p;
    int cib = drand48()*NBBOT;
    if (bot[cib].camp != -1 && bot[cib].camp != vehic[v].camp) {
        subv3(&obj[bot[cib].vion].pos, &obj[vehic[v].o1].pos, &p);
        if (norme2(&p) < 5000000) return bot[cib].vion;
    }

    // no target found
    return -1;
}

// Look for another ground target
static int vehic_new_ground_target(int v)
{
    for (int j = 0; j < 10; j++) {
        double r = drand48();
        if (r < .3) {
            // go for a village
            int a = NBVILLAGES*drand48();
            for (int i = 0; i < 10; i++) {
                int cib = village[a].o1 + (village[a].o2 - village[a].o1)*drand48();
                if (obj[cib].type == CIBGRAT) return cib;
            }
        } else {
            // go for another tank
            for (int i = 0; i < 10; i++) {
                int v2 = NBTANKBOTS*drand48();
                if (obj[vehic[v2].o1].type == VEHIC && vehic[v2].camp != vehic[v].camp && vehic[v2].camp != -1) {
                    return vehic[v2].o1;
                }
            }
        }
    }

    // no target found
    return -1;
}

void robotvehic(int v)
{
    vector p, u;
    double xx, yy, n;

    if (vehic[v].camp == -1) return;
    vehic[v].tir = 0;

    // Try to aquire a flying target
    // Notice that any object can become a DECO when crashed or destroyed
    if (vehic[v].cibv != -1 && obj[vehic[v].cibv].type == DECO) {
        vehic[v].cibv = -1;
    }
    if (vehic[v].cibv == -1) {
        vehic[v].cibv = vehic_new_flying_target(v);
    }

    if (vehic[v].cibt != -1 && obj[vehic[v].cibt].type == DECO) {
        vehic[v].cibt = -1;
    }
    if (vehic[v].cibt == -1) {
        vehic[v].cibt = vehic_new_ground_target(v);
    }

    // Choose to aim at the flying target if we have one, then the ground target
    int cib = vehic[v].cibv;
    int vol = 1;
    if (cib == -1) {
        cib = vehic[v].cibt;
        vol = 0;
    }

    if (cib == -1) {
        // No target? Life's not worth living.
        return;
    }

    subv3(&obj[cib].pos, &obj[vehic[v].o1].pos, &p);
    n = renorme(&p);
    if (n < 4000) {
        // Target is close enough to deal with it. Only change turret position.
        if (n < 600) vehic[v].moteur = 0;
        yy = scalaire(&p, &obj[vehic[v].o1+1].rot.y);
        xx = scalaire(&p, &obj[vehic[v].o1+1].rot.x);
        if (xx < 0) {
            vehic[v].ang1 += .4;
        } else {
            vehic[v].ang1 += .4 * yy;
            if (yy > -.2 && yy < .2) {
                subv3(&obj[cib].pos, &obj[vehic[v].o1+3+vehic[v].ocanon].pos, &u);
                double tz = u.z - tirz(obj[vehic[v].o1+2].rot.x.z, sqrt(u.x*u.x + u.y*u.y));
                if (tz > 0.) {
                    vehic[v].ang2 += tz < 100. ? .001*tz : .1;
                } else if (tz<0) {
                    vehic[v].ang2 += tz >-100. ? .001*tz :-.1;
                }
                if (tz > -100. && tz < 100. && n < 2500.) {
                    vehic[v].tir = 1;
                }
            }
        }
    } else {
        // Target is not close enough to fire at it, get closer.
        if (vol) vehic[v].cibv = -1;
        vehic[v].moteur = 1;
        yy = scalaire(&p, &obj[vehic[v].o1].rot.y);
        xx = scalaire(&p, &obj[vehic[v].o1].rot.x);
        if (xx < 0.) vehic[v].ang0 += .01;
        else if (yy > 0.) vehic[v].ang0 += .01;
        else vehic[v].ang0 -= .01;
    }
}

void armstate(int b)
{
    int i;
    bot[b].nbomb=0;
    for (i=bot[b].vion; i<bot[b].vion+nobjet[bot[b].navion].nbpieces; i++) {
        if (obj[i].objref==bot[b].vion && obj[i].type==BOMB) bot[b].nbomb++;
    }
}

void newnav(int b)
{
    if (SpaceInvaders) {
        bot[b].u.x = bot[b].u.y = 0.;
        bot[b].u.z = 16000.;
        bot[b].target_speed = 2. * ONE_METER;
        bot[b].maneuver = NAVIG;
        return;
    }
    if (bot[b].cibt != -1 && bot[b].nbomb && bot[b].fiul > viondesc[bot[b].navion].fiulmax/2) {
        if (bot[b].maneuver != NOSE_UP) {
            copyv(&bot[b].u,&obj[bot[b].cibt].pos);
            bot[b].target_speed = 3.5 * ONE_METER;
            bot[b].maneuver = NAVIG;
        } else {
            copyv(&bot[b].u,&obj[bot[b].vion].rot.x);
            mulv(&bot[b].u,25000);
            bot[b].u.z=1000;
            addv(&bot[b].u,&obj[bot[b].vion].pos);
            bot[b].target_speed = 5.0 * ONE_METER;   // on s'éloigne fissa!
            bot[b].maneuver = EVADE;
        }
        bot[b].u.z+=3000;
    } else {
        if (bot[b].maneuver != ILS_1 && bot[b].maneuver != ILS_2 && bot[b].maneuver != ILS_3) {
            copyv(&bot[b].u,&obj[bot[b].babase].rot.x);
            mulv(&bot[b].u,6000);
            addv(&bot[b].u,&obj[bot[b].babase].pos);
            bot[b].u.z=1000+obj[bot[b].vion].pos.z-bot[b].zs;
            bot[b].maneuver = ILS_1;
            bot[b].target_speed = 3. * ONE_METER;
            bot[b].cibt=-1;
        } else if (bot[b].maneuver == ILS_1) {
            copyv(&bot[b].u,&obj[bot[b].babase].rot.x);
            mulv(&bot[b].u,1500);
            addv(&bot[b].u,&obj[bot[b].babase].pos);
            bot[b].u.z=obj[bot[b].vion].pos.z-bot[b].zs+300;
            bot[b].maneuver = ILS_2;
            bot[b].target_speed = 1.6 * ONE_METER;
        } else if (bot[b].maneuver == ILS_2) {
            copyv(&bot[b].u,&obj[bot[b].babase].pos);
            bot[b].but.gear=1;
            bot[b].maneuver = ILS_3;
            bot[b].target_speed = .9 * ONE_METER;
        }
    }
}

static void newcib(int b)
{
    int i, j=0;
    double r;
    if (SpaceInvaders) j=10;
choiz:
    if (j++>10) bot[b].cibt=-1;
    else {
        r=drand48();
        if (r<.5) {
            // attaque un village
            bot[b].a=NBVILLAGES*drand48();
            i=0;
            do {
                bot[b].cibt=village[bot[b].a].o1+(village[bot[b].a].o2-village[bot[b].a].o1)*drand48();
                i++;
            } while(i<10 && obj[bot[b].cibt].type!=CIBGRAT);
            if (i==10) goto choiz;
        } else {
            // attaque un tank
            i=0;
            do {
                bot[b].cibt=(int)(NBTANKBOTS*drand48());
                i++;
            } while(i<10 && obj[bot[b].cibt].type!=VEHIC && vehic[bot[b].cibt].camp==bot[b].camp);
            if (i==10) goto choiz;
            else bot[b].cibt=vehic[bot[b].cibt].o1;
        }
    }
}

double cap(double x, double y)
{
    // Returns cap between 0 and 2*M_PI
    double const d2 = y*y + x*x;
    if (d2 == 0.) return 0;
    double a = acos(x / sqrt(d2));
    if (y<0) a = 2*M_PI - a;
    return a;
}

// This is the most important function and thus can change any settings (yctl, xctl, flaps...)
// So call it last !
// FIXME: use control.c to predict the vert speed from the slope and target the correct slope
static void adjust_slope(int b, float diff_alt)
{
    float const  n = .7 * atan(.001 * diff_alt);    // ranges from -1 to 1
    vector v;
    mulmtv(&obj[bot[b].vion].rot, &bot[b].vionvit, &v);
    renorme(&v);
    bot[b].yctl = n - obj[bot[b].vion].rot.x.z;
    if (n > 0 && bot[b].vitlin < BEST_LIFT_SPEED) {
        float vr = bot[b].vitlin/BEST_LIFT_SPEED;
        vr *= vr; vr *= vr; vr *= vr;
        float const ratio = 1. - vr;
        bot[b].yctl = ratio * -obj[bot[b].vion].rot.x.z + (1.-ratio) * bot[b].yctl;
        bot[b].thrust += .1;
        if (n > .1) {
            bot[b].but.flap = 1;
        }
    }

    if (n > 0.5) {
        bot[b].but.flap = 1;
        bot[b].thrust = 1.;
        bot[b].xctl = (bot[b].xctl-obj[bot[b].vion].rot.y.z)*.5;
    }
#   ifdef PRINT_DEBUG
    if (b == visubot) printf("z=%f, vitlin=%f, diff_alt=%f, n=%f, yctl=%f\n",
        obj[bot[b].vion].pos.z, diff_alt, bot[b].vitlin, n, bot[b].yctl);
#   endif
}

static void adjust_throttle(int b, float speed)
{
    if (bot[b].vitlin < speed) {
        bot[b].thrust += .01;
    } else if (bot[b].thrust > .02) {
        bot[b].thrust -= .01;
    }
}

static void adjust_direction(int b, vector const *dir)
{
    // Set target vertical slope according to required navpoint
    double dc = cap(dir->x - obj[bot[b].vion].pos.x, dir->y - obj[bot[b].vion].pos.y) - bot[b].cap;
    // Get it between [-PI:PI]
    while (dc >  M_PI) dc -= 2.*M_PI;
    while (dc < -M_PI) dc += 2.*M_PI;
    float a = 0;
    if (dc > .5) a = -.9;
    else if (dc < -.5) a = .9;
    else a = -.9*dc/.5;
    // don't lean too much if we lack vertical speed
    if (bot[b].vionvit.z < 0.) a *= pow(.7, (-30./ONE_METER) * bot[b].vionvit.z);
    bot[b].xctl = a - obj[bot[b].vion].rot.y.z;
}

#define HIGH_ALT (20. * ONE_METER)
void robot_safe(int b)
{
    // First: if we are grounded, the safest thing to do is to stop !
    if (! bot[b].is_flying) {
        bot[b].but.frein = 1;
        bot[b].thrust = 0.;
        bot[b].but.gear = 1;    // just in case
        return;
    }

    // No gears
    bot[b].but.gear = 0;

    // No roll
    bot[b].xctl = -obj[bot[b].vion].rot.y.z;

    if (bot[b].zs >= HIGH_ALT) {
        // Flaps, etc
        bot[b].but.flap = 0;
        // Adjut thrust
        adjust_throttle(b, 2.5 * ONE_METER);
        // No vertical speed
#       ifdef PRINT_DEBUG
        if (b == visubot) printf("safe, high\n");
#       endif
        adjust_slope(b, 0.);
    } else {
        // If the ground is near, try to keep our altitude
        bot[b].but.flap = 1;
        adjust_throttle(b, 50. * ONE_METER);
#       ifdef PRINT_DEBUG
        if (b == visubot) printf("safe, low\n");
#       endif
        adjust_slope(b, HIGH_ALT - bot[b].zs);
    }
}

void robot_autopilot(int b)
{
    if (bot[b].zs < .9 * HIGH_ALT) {
        robot_safe(b);
        return;
    }

    // flaps off
    bot[b].but.flap = 0;
    bot[b].but.gear = 0;
    adjust_direction(b, &bot[b].u);
    adjust_throttle(b, bot[b].target_speed);
    adjust_slope(b, bot[b].target_alt - obj[bot[b].vion].pos.z);
}

void robot(int b)
{
    float vit,m,n,a,dist,disth;
    float distfrap2, dc;
    vector u,v,c;
    int o=bot[b].vion;
    if (bot[b].camp==-1) return;
//  printf("bot %d man %d ",b,bot[b].maneuver);
    vit=norme(&bot[b].vionvit);
#define zs bot[b].zs
    if (bot[b].gunned!=-1) {    // il reviendra a chaque fois en cas de voltige...
        if (!(bot[b].gunned&(1<<NTANKMARK))) {  // si c'est un bot qui l'a touché
            if (bot[b].cibv!=bot[bot[b].gunned].vion && bot[bot[b].gunned].camp!=bot[b].camp) {
                bot[b].aerobatic = TURN;
                bot[b].cibv=bot[bot[b].gunned].vion;
            }
        } else {
            bot[b].cibt = vehic[bot[b].gunned&((1<<NTANKMARK)-1)].o1;
            bot[b].maneuver = NAVIG;
            bot[b].aerobatic = MANEUVER;   // il va voir sa mere lui !
            bot[b].gunned = -1;
        }
    } else {
        if (bot[b].cibv==-1) {
            int cib=drand48()*NBBOT;
            if (bot[b].bullets>100 && bot[b].fiul>70000 && bot[cib].camp!=-1 && bot[cib].camp!=bot[b].camp) {
                subv3(&obj[bot[cib].vion].pos,&obj[bot[b].vion].pos,&u);
                if (norme2(&u)<ECHELLE*ECHELLE*10) {
                    bot[b].cibv=bot[cib].vion;
                    bot[b].aerobatic = TAIL;
                    bot[b].gunned=cib;
                }
            }
        }
    }
    if (bot[b].aerobatic && obj[bot[b].cibv].type==DECO) {
        bot[b].aerobatic = MANEUVER;
        bot[b].cibv=-1;
        bot[b].maneuver = NAVIG;
        bot[b].gunned=-1;
    }
    switch (bot[b].aerobatic) {
    case MANEUVER:
        switch (bot[b].maneuver) {
        case PARKING:   // acquire a target
            bot[b].u = obj[bot[b].babase].pos;
            bot[b].v = obj[bot[b].babase].rot.x;
            bot[b].but.gear = 1;
            newcib(b);
            if (bot[b].cibt != -1) bot[b].maneuver = TAXI;
            break;
        case TAXI:
            subv3(&bot[b].u, &obj[o].pos, &u);
            n = norme(&u);
            float const target_speed = n > 3. * ONE_METER ? .6 * ONE_METER : .3 * ONE_METER;
            if (vit < target_speed) {
                bot[b].thrust += .01;
                bot[b].but.frein = 0;
            } else if (vit > target_speed) {
                bot[b].thrust -= .01;
                bot[b].but.frein = 1;
            }
            if (n > .7 * ONE_METER) {
                bot[b].xctl = -2. * scalaire(&u, &obj[o].rot.y)/n;
                if (scalaire(&u, &obj[o].rot.x) < 0.) {
                    bot[b].xctl = bot[b].xctl > 0. ? 1. : -1.;
                }
            } else {
                bot[b].thrust = 0.;    // to trigger reload
                bot[b].but.frein = 1;
                if (vit < .01 * ONE_METER) bot[b].maneuver = LINE_UP;
            }
            break;
        case LINE_UP:
            bot[b].thrust = 0.1;
            bot[b].but.frein = 0;
            bot[b].xctl = -4*scalaire(&bot[b].v, &obj[o].rot.y);
            if (scalaire(&bot[b].v, &obj[o].rot.x) < 0.) {
                bot[b].xctl = bot[b].xctl > 0. ? 1. : -1.;
            }
            if (fabs(bot[b].xctl) < .02) bot[b].maneuver = TAKE_OFF;
            break;
        case TAKE_OFF:
            bot[b].thrust = 1.;
            bot[b].but.flap = 1;
            bot[b].xctl = 0.;
            if (vit > 1. * ONE_METER) {
                // Small trick: use rebound to gain lift
                bot[b].xctl = .01;
            }
            if (vit > 1.2 * ONE_METER) {
                bot[b].xctl = 0.;
                bot[b].yctl = -obj[o].rot.x.z;   // level the nose
            }
            if (vit > 1.8 * ONE_METER) {
                bot[b].xctl = -obj[o].rot.y.z;
                bot[b].yctl = .1 -obj[o].rot.x.z;
            }
            if (zs > ONE_METER) {
                bot[b].but.gear = 0;
                bot[b].yctl = .18 -obj[o].rot.x.z;
                armstate(b);
            }
            if (zs > 8. * ONE_METER) {
                bot[b].but.flap = 0;
                bot[b].yctl = .15 -obj[o].rot.x.z;
                newnav(b);
            }
            break;
        case NAVIG:
        case EVADE:
        case ILS_1:
        case ILS_2:
        case ILS_3:
        case HEDGEHOP:
        case BOMBING:
            bot[b].but.flap = !((bot[b].maneuver == NAVIG || bot[b].maneuver == ILS_1 || bot[b].maneuver == EVADE) && bot[b].vitlin > 1.3 * ONE_METER);
            if (bot[b].cibt != -1 && bot[b].maneuver != EVADE) {  // copy the location in case the target is moving
                bot[b].u.x = obj[bot[b].cibt].pos.x;
                bot[b].u.y = obj[bot[b].cibt].pos.y;
            }
            v = bot[b].u;        // distance to navpoint
            subv(&v, &obj[o].pos);
            n = renorme(&v);
            dc = cap(v.x, v.y) - bot[b].cap;
            while (dc >  M_PI) dc -= 2.*M_PI;
            while (dc < -M_PI) dc += 2.*M_PI;
            if (bot[b].maneuver == HEDGEHOP && n < 40. * ONE_METER) {
                bot[b].maneuver = BOMBING;
                bot[b].target_speed = 1.7 * ONE_METER;
                bot[b].u.z = obj[bot[b].cibt].pos.z + 6. * ONE_METER;
            }
            if (bot[b].maneuver == BOMBING) {
                vector c;
                if (obj[bot[b].cibt].type == DECO) {
                    newcib(b);
                    newnav(b);
                    break;
                } else {
                    c = obj[bot[b].cibt].pos;
                    subv(&c, &obj[bot[b].vion].pos);
                    if ((distfrap2 = bombz2(bot[b].vionvit,c)) < 7000) {    // diminuer n'augmente pas la precision !
                        bot[b].but.bomb = 1;
                        bot[b].maneuver = NOSE_UP;
                    }
#                   ifdef PRINT_DEBUG
                    if (b == visubot) printf("distfrap2=%f\n", distfrap2);
#                   endif
                    if (dc < -M_PI/2 || dc > M_PI/2) bot[b].maneuver = NOSE_UP;
                }
            }
            if (bot[b].maneuver == EVADE && n < 3000) {
                newnav(b);
                break;
            }
            if (bot[b].maneuver == NAVIG && bot[b].cibt != -1) {
                if (n < 40000) {
                    bot[b].u.z = obj[bot[b].cibt].pos.z + 1500;
                    bot[b].target_speed = 3.5 * ONE_METER;
                    bot[b].maneuver = HEDGEHOP;
                    break;
                } else if (n < 7000) {
                    bot[b].u.z = obj[bot[b].cibt].pos.z + 800;
                }
            } else if (bot[b].maneuver >= NAVIG && bot[b].maneuver < ILS_3 && n < 2000. && !SpaceInvaders) {
                newnav(b);
                break;
            } else if (bot[b].maneuver == ILS_3) {
                if (zs < 30.) {
                    bot[b].target_speed = 0.;
                }
                if (bot[b].vitlin < 15.) {
                    bot[b].but.frein = 1;
                }
                if (bot[b].vitlin < 1.) {
                    bot[b].maneuver = PARKING;
                    break;
                }
            }
        //  m=obj[o].rot.y.x*v.x+obj[o].rot.y.y*v.y;
        //  n=obj[o].rot.x.x*v.x+obj[o].rot.x.y*v.y;
        //  if (n>0 || m>.1) a=-.5*atan(10*m*(n<0?1000:1));
        //  else a=.9;
            // FIXME: most of this is already done in autopilot
            if (
                (bot[b].maneuver == NAVIG && zs < 15. * ONE_METER) ||
                (bot[b].maneuver == HEDGEHOP && zs < 4. * ONE_METER) ||
                (bot[b].maneuver == EVADE && zs < 5. * ONE_METER)
            ) {
                // Too low ! forget about navpoint and level the wings
                bot[b].xctl = -obj[bot[b].vion].rot.y.z;
                // small incidence
                bot[b].yctl = .2 - obj[bot[b].vion].rot.x.z;
                // full throttle
                bot[b].thrust = 1.;
                // flaps on
                bot[b].but.flap = 1;
            } else {
                if (dc > .5) a = -.9;
                else if (dc < -.5) a = .9;
                else a = -.9*dc/.5;
                // don't lean too much if we lack vertical speed
                if (bot[b].vionvit.z < 0.) a *= pow(.7, (-30./ONE_METER) * bot[b].vionvit.z);
                bot[b].xctl = a - obj[o].rot.y.z;

                // Adjust thrust
                if (bot[b].vitlin < bot[b].target_speed && bot[b].thrust) bot[b].thrust += .01;
                else if (bot[b].vitlin > bot[b].target_speed && bot[b].thrust > .2) bot[b].thrust -= .01;

                float target_alt = 90 * ONE_METER;
                if (bot[b].maneuver == BOMBING) target_alt = 4. * ONE_METER;
                else if (bot[b].maneuver == HEDGEHOP) target_alt = 9. * ONE_METER;
                else if (bot[b].maneuver == ILS_1) target_alt = 4. * ONE_METER;
                else if (bot[b].maneuver == ILS_2) target_alt = 2. * ONE_METER;
                float diff_alt = target_alt - zs;
                float n = .7 * atan(1e-3 * diff_alt);
                bot[b].yctl = 0.08*(n - bot[b].vionvit.z/bot[b].target_speed);
            }

            break;
        case DIVE_N_BOMB:
            // on vérifie que la cible n'est pas détruite
            if (obj[bot[b].cibt].type!=DECO) {
                double distfrap2;
                copyv(&v,&bot[b].u);
                subv(&v,&obj[o].pos);
                if ((distfrap2=bombz2(bot[b].vionvit,v))<1000) {
                    bot[b].but.bomb=1;
                    bot[b].maneuver = NOSE_UP;
                }
                renorme(&v);
                bot[b].thrust=0;
                m=obj[o].rot.y.x*v.x+obj[o].rot.y.y*v.y;
                n=obj[o].rot.x.x*v.x+obj[o].rot.x.y*v.y;
                if (n>0 || m>.1) a=-.5*atan(1*m*(n<0?1000:1)); else a=.5;
                bot[b].xctl=(a-obj[o].rot.y.z);
                bot[b].yctl=(v.z-obj[o].rot.x.z-.14);
                if (zs<200) bot[b].maneuver = NOSE_UP;
            } else {    // changer de cible
                newcib(b);
                newnav(b);
            }
            break;
        case NOSE_UP:
            bot[b].thrust=1;
            bot[b].but.flap=1;
            bot[b].xctl=(-obj[o].rot.y.z);
            bot[b].yctl=1;
            if (bot[b].vionvit.z>.5) newnav(b);
            break;
        }
        break;
    case TURN:
        if (obj[o].rot.y.z<0) bot[b].xctl=-1-obj[o].rot.y.z;
        else bot[b].xctl=1-obj[o].rot.y.z;
        if (obj[o].rot.z.z<0) bot[b].xctl=-bot[b].xctl;
        if (vit<13) bot[b].thrust+=.1;
        else if (vit>13) bot[b].thrust-=.01;
        bot[b].yctl=1-bot[b].xctl*bot[b].xctl;
        if (zs<400) bot[b].aerobatic = MANEUVER;
        if (scalaire(&obj[o].rot.x,&obj[bot[b].cibv].rot.x)<0) bot[b].aerobatic = RECOVER;
        break;
    case RECOVER:
        if (vit<18) bot[b].thrust+=.1;
        else if (vit>18) bot[b].thrust-=.01;
        bot[b].xctl=(-obj[o].rot.y.z);
        bot[b].yctl=(-bot[b].vionvit.z)*.13;
        if (obj[o].rot.z.z<0) bot[b].yctl=-bot[b].yctl;
        if (obj[o].rot.z.z>.8) bot[b].aerobatic = TAIL;
        break;
    case CLIMB_VERT:
        bot[b].thrust=1;
        bot[b].xctl=(-obj[o].rot.y.z);
        bot[b].yctl=(1-obj[o].rot.x.z)*.13;
        if (bot[b].vionvit.z<0 && obj[o].rot.x.z>.5) bot[b].aerobatic = RECOVER;
        if (zs<400) bot[b].aerobatic = MANEUVER;
        break;
    case TAIL:
        copyv(&v,&obj[bot[b].cibv].pos);
        subv(&v,&obj[o].pos);
        disth=sqrt(v.x*v.x+v.y*v.y);
        copyv(&u,&bot[bot[b].gunned].vionvit);
        if (disth<3000) mulv(&u,disth*.02);
        addv(&v,&u);
        a=scalaire(&v,&obj[o].rot.y);
        dist=renorme(&v);
        dc=cap(v.x,v.y)-bot[b].cap;
        m=mod[obj[bot[b].cibv].model].rayoncollision;
        if (dc<-M_PI) dc+=2*M_PI;
        else if (dc>M_PI) dc-=2*M_PI;
        distfrap2=0;
        if (dist<4000) {
            if (dc>-M_PI/7 && dc<M_PI/7) {
                mulv(&obj[o].rot.x,400);
                addv(&obj[o].rot.x,&v);
                renorme(&obj[o].rot.x);
                orthov(&obj[o].rot.y,&obj[o].rot.x);
                renorme(&obj[o].rot.y);
                prodvect(&obj[o].rot.x,&obj[o].rot.y,&obj[o].rot.z);
            }
            if (a>-6*m && a<6*m && dc>-M_PI/2 && dc<M_PI/2) {
                distfrap2=(-tirz(obj[o].rot.x.z,disth)+obj[bot[b].cibv].pos.z-obj[o].pos.z);
                if (distfrap2<4*m && distfrap2>-4*m) bot[b].but.canon=1;
            }
            copyv(&c,&bot[bot[b].gunned].vionvit);
            subv(&c,&bot[b].vionvit);
            a=scalaire(&c,&v);  // a = vitesse d'éloignement
#define NFEU 400.
#define ALPHA .004
#define H (ALPHA*NFEU)
        //  if (visubot==b) printf("dist=%f A=%f Avoulu=%f\n",dist,a,H-ALPHA*dist);
            if (a>H-ALPHA*dist || bot[b].vitlin<15) bot[b].thrust+=.1;
            else {
                if (bot[b].thrust>.06) bot[b].thrust-=.03;
            //  distfrap2=(bot[b].vitlin-18)*.02;
            }
        } else if (obj[bot[b].vion].rot.x.z>-.4) {
            bot[b].but.flap=0;
            bot[b].thrust=1;    // courrir après une cible (pas après le sol toutefois !)
        }
        if (dc>.3) a=-.9;
        else if (dc<-.3) a=.9;
        else a=-.9*dc/.3;
        bot[b].xctl=(a-obj[o].rot.y.z);
        n=(obj[bot[b].cibv].pos.z-obj[o].pos.z)*3+distfrap2*4/*30*/;    // DZ
        if (zs<6000) { n+=20000-5*zs; bot[b].thrust=1; }
        m=7*atan(1e-3*n);
        bot[b].yctl=fabs(obj[o].rot.y.z)*.5+(m-bot[b].vionvit.z)*.3;
        if (zs<3000) bot[b].aerobatic = CLIMB;
        break;
    case CLIMB:
        bot[b].but.flap=1;
        if (obj[o].rot.x.z>0) bot[b].thrust=1;
        else {
            if (bot[b].vitlin<19) bot[b].thrust+=.2;
            else if (bot[b].vitlin>23) bot[b].thrust-=.2;
        }
        bot[b].xctl=(-obj[o].rot.y.z);
        bot[b].yctl=(-obj[o].rot.x.z)+1.;
        if (bot[b].vionvit.z<0) bot[b].yctl=1;
        if (obj[o].rot.z.z<0) bot[b].yctl=-bot[b].yctl;
        if (zs>3100) bot[b].aerobatic = TAIL;
        break;
    }
    // Ensure controls are within bounds
    if (bot[b].thrust < 0.) bot[b].thrust = 0.;
    else if (bot[b].thrust > 1.) bot[b].thrust = 1.;
    if (bot[b].xctl < -1.) bot[b].xctl = -1;
    else if (bot[b].xctl > 1.) bot[b].xctl = 1;
    if (bot[b].yctl < -1.) bot[b].yctl = -1;
    else if (bot[b].yctl > 1.) bot[b].yctl = 1;
}
