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
#include <float.h>
#include <assert.h>
#include "proto.h"
#include "robot.h"
#include "heightfield.h"

//#define PRINT_DEBUG

struct bot *bot;
struct tank *tank;
struct car *car;
struct zeppelin *zep;

char const *aerobatic_2_str(enum aerobatic aerobatic)
{
    switch (aerobatic) {
        case MANEUVER:   return "maneuver";
        case TURN:       return "turn";
        case RECOVER:    return "recover";
        case TAIL:       return "follow 6s";
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

static double tirz(struct vector const *v0, double d)
{
    double const h_speed = SHOT_SPEED * sqrt(v0->x*v0->x + v0->y*v0->y);
    if (h_speed == 0) return 0;
    double const t = d / h_speed;   // the t when shot will have traversed dist d horizontaly
    return .05 /* from main.c */ * .5 * SQUARE(t) + SHOT_SPEED * v0->z * t;
}

// FIXME: same function to actually move the bombs!
double fall_min_dist2(int b)
{
    struct vector pos = obj[bot[b].vion].pos;
    struct vector velocity = bot[b].vionvit;
    double d = DBL_MAX, min_d;
    do {    // Note: using distance to target avoids computing zground at each step
        min_d = d;
        velocity.z -= G * MAX_DT_SEC;
        mulv(&velocity, pow(.9, MAX_DT_SEC));
        struct vector v = velocity;
        mulv(&v, MAX_DT_SEC);
        addv(&pos, &v);
        subv3(&obj[bot[b].cibt].pos, &pos, &v);
        d = norme2(&v);
    } while (d < min_d);
    bot[b].drop_mark = pos;
    return min_d;
}

// Look for a flying target for tank v
static int vehic_new_flying_target(int v)
{
    struct vector p;
    int cib = drand48()*NBBOT;
    if (bot[cib].camp != -1 && bot[cib].camp != tank[v].camp) {
        subv3(&obj[bot[cib].vion].pos, &obj[tank[v].o1].pos, &p);
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
            int a = NB_VILLAGES*drand48();
            for (int i = 0; i < 10; i++) {
                int cib = village[a].o1 + (village[a].o2 - village[a].o1)*drand48();
                if (obj[cib].type == TYPE_CAR) return cib;
            }
        } else {
            // go for another tank
            for (int i = 0; i < 10; i++) {
                int v2 = NBTANKBOTS*drand48();
                if (obj[tank[v2].o1].type == TYPE_TANK && tank[v2].camp != tank[v].camp && tank[v2].camp != -1) {
                    return tank[v2].o1;
                }
            }
        }
    }

    // no target found
    return -1;
}

void robotvehic(int v)
{
    struct vector p, u;
    double xx, yy, n;

    if (tank[v].camp == -1) return;
    tank[v].tir = 0;

    // Try to aquire a flying target
    // Notice that any object can become a TYPE_DECO when crashed or destroyed
    if (tank[v].cibv != -1 && obj[tank[v].cibv].type == TYPE_DECO) {
        tank[v].cibv = -1;
    }
    if (tank[v].cibv == -1) {
        tank[v].cibv = vehic_new_flying_target(v);
    }

    if (tank[v].cibt != -1 && obj[tank[v].cibt].type == TYPE_DECO) {
        tank[v].cibt = -1;
    }
    if (tank[v].cibt == -1) {
        tank[v].cibt = vehic_new_ground_target(v);
    }

    // Choose to aim at the flying target if we have one, then the ground target
    int cib = tank[v].cibv;
    int vol = 1;
    if (cib == -1) {
        cib = tank[v].cibt;
        vol = 0;
    }

    if (cib == -1) {
        // No target? Life's not worth living.
        return;
    }

    subv3(&obj[cib].pos, &obj[tank[v].o1].pos, &p);
    n = renorme(&p);
    if (n < 4000) {
        // Target is close enough to deal with it. Only change turret position.
        if (n < 600) tank[v].moteur = 0;
        yy = scalaire(&p, &obj[tank[v].o1+1].rot.y);
        xx = scalaire(&p, &obj[tank[v].o1+1].rot.x);
        if (xx < 0) {
            tank[v].ang1 += .4;
        } else {
            tank[v].ang1 += .4 * yy;
            if (yy > -.2 && yy < .2) {
                subv3(&obj[cib].pos, &obj[tank[v].o1+3+tank[v].ocanon].pos, &u);
                double tz = u.z - tirz(&obj[tank[v].o1+2].rot.x, sqrt(u.x*u.x + u.y*u.y));
                if (tz > 0.) {
                    tank[v].ang2 += tz < 100. ? .001*tz : .1;
                } else if (tz<0) {
                    tank[v].ang2 += tz >-100. ? .001*tz :-.1;
                }
                if (tz > -100. && tz < 100. && n < 2500.) {
                    tank[v].tir = 1;
                }
            }
        }
    } else {
        // Target is not close enough to fire at it, get closer.
        if (vol) tank[v].cibv = -1;
        tank[v].moteur = 1;
        yy = scalaire(&p, &obj[tank[v].o1].rot.y);
        xx = scalaire(&p, &obj[tank[v].o1].rot.x);
        if (xx < 0.) tank[v].ang0 += .01;
        else if (yy > 0.) tank[v].ang0 += .01;
        else tank[v].ang0 -= .01;
    }
}

void armstate(int b)
{
    int i;
    bot[b].nbomb=0;
    for (i=bot[b].vion; i<bot[b].vion+n_object[bot[b].navion].nbpieces; i++) {
        if (obj[i].objref==bot[b].vion && obj[i].type==TYPE_BOMB) bot[b].nbomb++;
    }
}

static void landing_approach(int b)
{
    bot[b].maneuver = ILS_1;
    bot[b].u = obj[bot[b].babase].rot.x;
    mulv(&bot[b].u, 130. * ONE_METER);
    addv(&bot[b].u, &obj[bot[b].babase].pos);
    bot[b].u.z = z_ground(bot[b].u.x, bot[b].u.y, false);
    bot[b].target_speed = 2.5 * ONE_METER;
    bot[b].target_rel_alt = 16. * ONE_METER;
    bot[b].cibt = -1;
}

void newnav(int b)
{
    if (killemall_mode) {
        bot[b].u.x = bot[b].u.y = 0.;
        bot[b].u.z = z_ground(bot[b].u.x, bot[b].u.y, true);
        bot[b].target_speed = 2. * ONE_METER;
        bot[b].target_rel_alt = 30. * ONE_METER;
        bot[b].maneuver = NAVIG;
        return;
    }
    if (bot[b].cibt != -1 && bot[b].nbomb && bot[b].fiul > plane_desc[bot[b].navion].fiulmax/2) {
        bot[b].u = obj[bot[b].cibt].pos;
        bot[b].target_speed = 3.5 * ONE_METER;
        bot[b].target_rel_alt = 100. * ONE_METER;
        bot[b].maneuver = NAVIG;
    } else if (bot[b].maneuver != ILS_1 && bot[b].maneuver != ILS_2 && bot[b].maneuver != ILS_3) {
        landing_approach(b);
    }
}

static void newcib(int b)
{
    int i, j=0;
    double r;
    if (killemall_mode) j=10;
choiz:
    if (j++>10) bot[b].cibt=-1;
    else {
        r=drand48();
        if (r<.5) {
            // attaque un village
            bot[b].a=NB_VILLAGES*drand48();
            i=0;
            do {
                bot[b].cibt=village[bot[b].a].o1+(village[bot[b].a].o2-village[bot[b].a].o1)*drand48();
                i++;
            } while(i<10 && obj[bot[b].cibt].type!=TYPE_CAR);
            if (i==10) goto choiz;
        } else {
            // attaque un tank
            i=0;
            do {
                bot[b].cibt=(int)(NBTANKBOTS*drand48());
                i++;
            } while(i<10 && obj[bot[b].cibt].type!=TYPE_TANK && tank[bot[b].cibt].camp==bot[b].camp);
            if (i==10) goto choiz;
            else bot[b].cibt=tank[bot[b].cibt].o1;
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
    struct vector speed = bot[b].vionvit;
    renorme(&speed);

    // First we choose the target slope (ie. rot.x.z), then we change actual one with yctl
    // (note: we should choose vertical speed instead)
    float slope = .7 * atan(.001 * diff_alt);    // ranges from -1 to 1
#   ifdef PRINT_DEBUG
    if (b==viewed_bot) printf("naive slope: %f ", slope);
#   endif

    // But we do not want to have a slope too distinct from plane actual direction
    float aoa = slope - speed.z;  // from -2 to 2
    // The more linear speed we have, the more we can turn
    if (bot[b].vitlin < 0) { // don't know how to handle this
        bot[b].thrust = 1.;
        return;
    }
    float const max_aoa = 0.01 + powf(bot[b].vitlin / (2. * BEST_LIFT_SPEED), 5.2);
    aoa = max_aoa*atan(5. * aoa);  // smoothly cap the angle of attack
    slope = aoa + speed.z;
#   ifdef PRINT_DEBUG
    if (b==viewed_bot) printf("capped slope: %f ", slope);
#   endif

    if (slope > 0 && bot[b].vitlin < BEST_LIFT_SPEED) {
        float vr = bot[b].vitlin/BEST_LIFT_SPEED;
        vr *= vr; vr *= vr; vr *= vr;
        slope = (1. - vr) * -speed.z + vr * slope;
        bot[b].thrust += .1;
        if (slope > .1) {
            bot[b].but.flap = 1;
        }
#       ifdef PRINT_DEBUG
        if (b==viewed_bot) printf("vitlin slope: %f ", slope);
#       endif
    }

    if (diff_alt > 0. && speed.z < 0.5 * ONE_METER) {
        bot[b].but.flap = 1;
        CLAMP(bot[b].xctl, 1.);
        bot[b].thrust = 1.;
    }

    // Choose yctl according to target slope
    bot[b].yctl = slope - speed.z;
    CLAMP(bot[b].yctl, 1.);

    // The more we roll, the more we need to pull the joystick
    float roll = obj[bot[b].vion].rot.y.z;
    roll = roll*roll; roll = roll*roll; roll = roll*roll;
    bot[b].yctl += 1. - exp(-32. * roll);
    CLAMP(bot[b].yctl, 1.);

    if (bot[b].thrust > 1.) bot[b].thrust = 1.;

#   ifdef PRINT_DEBUG
    if (b == viewed_bot) printf("vitlin=%f, diff_alt=%f, Sz=%f, slope=%f, yctl=%f, rot.x.z=%f\n",
        bot[b].vitlin, diff_alt, speed.z, slope, bot[b].yctl, obj[bot[b].vion].rot.x.z);
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

static double adjust_direction_rel(int b, struct vector const *dir)
{
    // First of all, do not allow the plane to get upside down
    if (obj[bot[b].vion].rot.z.z < 0) {
#       ifdef PRINT_DEBUG
        if (b == viewed_bot) printf("upside down!\n");
#       endif
        bot[b].xctl = obj[bot[b].vion].rot.y.z > 0 ? -1. : 1.;
        return 0.;
    }

    // Set target rolling angle according to required navpoint
    double dc = cap(dir->x, dir->y) - bot[b].cap;
    // Get it between [-PI:PI]
    while (dc >  M_PI) dc -= 2.*M_PI;
    while (dc < -M_PI) dc += 2.*M_PI;
    float a = 0;
    float const a_max = sqrtf(1.001 - SQUARE(obj[bot[b].vion].rot.x.z));
    if (dc > .5) a = -a_max;
    else if (dc < -.5) a = a_max;
    else a = a_max*dc/-.5;
    bot[b].xctl = a - obj[bot[b].vion].rot.y.z;
    CLAMP(bot[b].xctl, 1.);
#   ifdef PRINT_DEBUG
    if (b == viewed_bot) printf("a=%f rot.y.z=%f xctl=%f\n", a, obj[bot[b].vion].rot.y.z, bot[b].xctl);
#   endif
    return dc;
}

static double adjust_direction(int b, struct vector const *pos)
{
    struct vector rel;
    subv3(pos, &obj[bot[b].vion].pos, &rel);
    return adjust_direction_rel(b, &rel);
}

void robot_safe(int b, float min_alt)
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

    // allow only a few roll
    adjust_direction(b, &bot[b].u);
    if (obj[bot[b].vion].rot.z.z >= 0) {
        float r = fabsf(obj[bot[b].vion].rot.y.z);
        r = 1. - r;
        r *= r; r *= r;
        bot[b].xctl = (1. - r) * (-obj[bot[b].vion].rot.y.z) + r * bot[b].xctl;
        CLAMP(bot[b].xctl, 1.);
    }

    if (bot[b].zs >= min_alt) {
        // Flaps, etc
        bot[b].but.flap = 0;
        // Adjut thrust
        adjust_throttle(b, 2.5 * ONE_METER);
        // No vertical speed
#       ifdef PRINT_DEBUG
        if (b == viewed_bot) printf("safe, high\n");
#       endif
        adjust_slope(b, 0.);
    } else {
        // If the ground is near, try to keep our altitude
        bot[b].but.flap = 1;
        bot[b].thrust = 1.;
#       ifdef PRINT_DEBUG
        if (b == viewed_bot) printf("safe, low: %f < %f\n", bot[b].zs, min_alt);
#       endif
        adjust_slope(b, min_alt - bot[b].zs);
    }
}

// Returns the horizontal distance from bot b to its navpoint.
static float dist_from_navpoint(int b, struct vector *v)
{
    v->x = bot[b].u.x - obj[bot[b].vion].pos.x;
    v->y = bot[b].u.y - obj[bot[b].vion].pos.y;
    v->z = 0.;
    return norme(v);
}

// Fly to bot[b].u at relative altitude bot[b].target_rel_alt
void robot_autopilot(int b)
{
    float low_alt = SAFE_LOW_ALT;
    struct vector v;
    float d = dist_from_navpoint(b, &v);
    if (d < 80. * ONE_METER) {  // only allow low altitudes when close from destination
        low_alt = MIN(LOW_ALT, bot[b].target_rel_alt);
    }

#   ifdef PRINT_DEBUG
    if (b == viewed_bot) printf("d=%f, low_alt=%f, rel_alt=%f\n", d, low_alt, bot[b].target_rel_alt);
#   endif
    if (bot[b].zs < low_alt) {
        robot_safe(b, low_alt);
        return;
    }

    // flaps off
    bot[b].but.flap = 0;
    bot[b].but.gear = 0;
    adjust_direction(b, &bot[b].u);
    adjust_throttle(b, bot[b].target_speed);
    adjust_slope(b, bot[b].u.z + bot[b].target_rel_alt - obj[bot[b].vion].pos.z);
}

void robot(int b)
{
    float vit,dist,disth;
    struct vector u,v;
    int o=bot[b].vion;
    if (bot[b].camp==-1) return;
//  printf("bot %d man %d ",b,bot[b].maneuver);
    vit = norme(&bot[b].vionvit);
#define zs bot[b].zs
    if (bot[b].gunned!=-1) {    // il reviendra a chaque fois en cas de voltige...
        if (!(bot[b].gunned&(1<<NTANKMARK))) {  // si c'est un bot qui l'a touché
            if (bot[b].cibv!=bot[bot[b].gunned].vion && bot[bot[b].gunned].camp!=bot[b].camp) {
                bot[b].aerobatic = TURN;
                bot[b].cibv=bot[bot[b].gunned].vion;
            }
        } else {
            bot[b].cibt = tank[bot[b].gunned&((1<<NTANKMARK)-1)].o1;
            bot[b].maneuver = NAVIG;
            bot[b].aerobatic = MANEUVER;   // il va voir sa mere lui !
            bot[b].gunned = -1;
        }
    } else {
        if (bot[b].cibv == -1 && bot[b].bullets>100 && bot[b].fiul>70000) {
            int cib = drand48()*NBBOT;
            if (bot[cib].camp!=-1 && bot[cib].camp!=bot[b].camp) {
                subv3(&obj[bot[cib].vion].pos,&obj[bot[b].vion].pos,&u);
                if (norme2(&u) < TILE_LEN*TILE_LEN*5.) {
                    bot[b].cibv = bot[cib].vion;
                    bot[b].aerobatic = TAIL;
                    bot[b].gunned = cib;
                }
            }
        }
    }
    if (bot[b].aerobatic != MANEUVER && obj[bot[b].cibv].type == TYPE_DECO) {
        bot[b].aerobatic = MANEUVER;
        bot[b].cibv = -1;
        bot[b].maneuver = NAVIG;
        bot[b].gunned = -1;
    }
    if (bot[b].cibt != -1) {
        // Copy target's location in case it's moving
        bot[b].u = obj[bot[b].cibt].pos;
    }
    switch (bot[b].aerobatic) {
        case MANEUVER:;
            float d;
#           ifdef PRINT_DEBUG
            if (b == viewed_bot) printf("%s\n", maneuver_2_str(bot[b].maneuver));
#           endif
            switch (bot[b].maneuver) {
                case PARKING:
                    bot[b].u = obj[bot[b].babase].pos;
                    bot[b].v = obj[bot[b].babase].rot.x;
                    bot[b].but.gear = 1;
                    if (bot[b].vitlin < .01 * ONE_METER) bot[b].maneuver = TAXI;
                    break;
                case TAXI:
                    d = dist_from_navpoint(b, &u);
                    float const target_speed = d > 3. * ONE_METER ? .6 * ONE_METER : .3 * ONE_METER;
                    adjust_throttle(b, target_speed);
                    if (d > .7 * ONE_METER) {
                        if (fabs(obj[o].rot.y.z) < .1) {
                            bot[b].xctl = -2. * scalaire(&u, &obj[o].rot.y)/d;
                            if (scalaire(&u, &obj[o].rot.x) < 0.) {
                                bot[b].xctl = bot[b].xctl > 0. ? 1. : -1.;
                            }
                            CLAMP(bot[b].xctl, 1.);
                        } else {
                            bot[b].xctl = -obj[o].rot.y.z;
                        }
                    } else {
                        bot[b].thrust = 0.;    // to trigger reload
                        bot[b].but.frein = 1;
                        bot[b].xctl = 0.;
                        if (bot[b].vitlin < .01 * ONE_METER) {
                            newcib(b);
                            if (bot[b].cibt != -1) bot[b].maneuver = LINE_UP;
                        }
                    }
#                   ifdef PRINT_DEBUG
                    if (b == viewed_bot) printf("d=%f, u*x=%f, u=%"PRIVECTOR"\n", d, scalaire(&u, &obj[o].rot.x), PVECTOR(u));
#                   endif
                    break;
                case LINE_UP:
                    bot[b].thrust = 0.1;
                    bot[b].but.frein = 1;
                    bot[b].xctl = -4*scalaire(&bot[b].v, &obj[o].rot.y);
                    if (scalaire(&bot[b].v, &obj[o].rot.x) < 0.) {
                        bot[b].xctl = bot[b].xctl > 0. ? 1. : -1.;
                    }
                    CLAMP(bot[b].xctl, 1.);
                    if (fabs(bot[b].xctl) < .02) bot[b].maneuver = TAKE_OFF;
                    break;
                case TAKE_OFF:
                    bot[b].thrust = 1.;
                    bot[b].but.flap = 1;
                    bot[b].but.frein = 0;
                    bot[b].xctl = 0.;
                    if (vit > 1. * ONE_METER) {
                        // Small trick: use rebound to gain lift
                        bot[b].xctl = .01;
                    }
                    if (vit > 1.5 * ONE_METER) {
                        bot[b].xctl = -obj[o].rot.y.z;
                        bot[b].yctl = -obj[o].rot.x.z;    // level the nose
                        CLAMP(bot[b].yctl, 1.);
                        CLAMP(bot[b].xctl, 1.);
                    }
                    if (vit > 2.4 * ONE_METER) {
                        adjust_slope(b, ONE_METER);
                    }
                    if (bot[b].is_flying) {
                        armstate(b);
                        newnav(b);
                    }
                    break;
                case NAVIG:
                    robot_autopilot(b);
                    d = dist_from_navpoint(b, &u);
                    if (d < 400. * ONE_METER && bot[b].cibt != -1) {
                        bot[b].target_rel_alt = 17. * ONE_METER;
                        bot[b].target_speed = 3.5 * ONE_METER;
                        bot[b].maneuver = HEDGEHOP;
                    }
                    break;
                case HEDGEHOP:
                    robot_autopilot(b);
                    d = dist_from_navpoint(b, &u);
                    if (d < 40. * ONE_METER) {
                        bot[b].target_speed = 1.9 * ONE_METER;
                        bot[b].target_rel_alt = 9. * ONE_METER;
                        bot[b].cibt_drop_dist2 = DBL_MAX;
                        bot[b].maneuver = BOMBING;
                    }
                    break;
                case BOMBING:
                    robot_autopilot(b);
                    if (obj[bot[b].cibt].type == TYPE_DECO) {
                        newcib(b);
                        newnav(b);
                        break;
                    } else {
                        double df = fall_min_dist2(b);
                        if (
                                df < SQUARE(7. * ONE_METER) &&
                                df > bot[b].cibt_drop_dist2
                           ) {
                            bot[b].but.bomb = 1;
                            bot[b].maneuver = NOSE_UP;
                        } else {
                            bot[b].cibt_drop_dist2 = df;
                        }
                        struct vector c;
                        subv3(&obj[bot[b].cibt].pos, &obj[bot[b].vion].pos, &c);
                        float cx = scalaire(&c, &obj[bot[b].vion].rot.x);
                        if (cx < 0.) bot[b].maneuver = NOSE_UP;
                    }
#                   ifdef PRINT_DEBUG
                    if (b == viewed_bot) printf("vion.z=%f, cibt.z=%f\n", obj[bot[b].vion].pos.z, bot[b].u.z + bot[b].target_rel_alt);
#                   endif
                    break;
                case EVADE:
                    robot_autopilot(b);
                    d = dist_from_navpoint(b, &u);
                    if (d < 10. * ONE_METER) {
                        newcib(b);
                        newnav(b);
                    }
#                   ifdef PRINT_DEBUG
                    if (b == viewed_bot) printf("d=%f, target_alt=%f\n", d, bot[b].u.z + bot[b].target_rel_alt);
#                   endif
                    break;
                case ILS_1:
                    robot_autopilot(b);
                    d = dist_from_navpoint(b, &u);
                    if (d > 90. * ONE_METER) break;
                    if (scalaire(&obj[bot[b].vion].rot.x, &obj[bot[b].babase].rot.x) > -.7) break;
                    bot[b].u = obj[bot[b].babase].rot.x;
                    mulv(&bot[b].u, 50. * ONE_METER);
                    addv(&bot[b].u, &obj[bot[b].babase].pos);
                    bot[b].u.z = z_ground(bot[b].u.x, bot[b].u.y, false);
                    bot[b].target_speed = 2.0 * ONE_METER;
                    bot[b].target_rel_alt = 11. * ONE_METER;
                    bot[b].maneuver = ILS_2;
                    break;
                case ILS_2:
                    robot_autopilot(b);
                    d = dist_from_navpoint(b, &u);
                    if (d > 40. * ONE_METER) break;
                    if (scalaire(&obj[bot[b].vion].rot.x, &obj[bot[b].babase].rot.x) > -.6) {
                        landing_approach(b);    // abort
                    }
                    bot[b].u = obj[bot[b].babase].pos;
                    bot[b].maneuver = ILS_3;
                    bot[b].target_speed = 1.9 * ONE_METER;
                    bot[b].target_rel_alt = 0.;
                    break;
                case ILS_3:
                    robot_autopilot(b);
                    bot[b].but.gear = 1;    // whatever the autopilot does
                    bot[b].but.flap = 1;
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
                    break;
                case NOSE_UP:   // in case we were pointing down, start climbing again and re-nav
                    bot[b].thrust = 1.;
                    bot[b].but.flap = 1;
                    bot[b].xctl = -obj[o].rot.y.z;
                    CLAMP(bot[b].xctl, 1.);
                    bot[b].yctl = 1.;
                    if (bot[b].vionvit.z > .5) {
                        bot[b].u = obj[bot[b].vion].rot.x;
                        mulv(&bot[b].u, 100. * ONE_METER);
                        addv(&bot[b].u, &obj[bot[b].vion].pos);
                        bot[b].u.z = z_ground(bot[b].u.x, bot[b].u.y, false);
                        bot[b].target_speed = 5.0 * ONE_METER;   // run Forest, run!
                        bot[b].target_rel_alt = 2. * SAFE_LOW_ALT;
                        bot[b].maneuver = EVADE;
                    }
                    break;
            }
            break;
        case TURN:
            if (obj[o].rot.y.z < 0.) bot[b].xctl = -1. - obj[o].rot.y.z;
            else bot[b].xctl = 1. - obj[o].rot.y.z;
            if (obj[o].rot.z.z < 0.) bot[b].xctl = -bot[b].xctl;
            adjust_throttle(b, BEST_LIFT_SPEED);
            bot[b].yctl = 1. - bot[b].xctl * bot[b].xctl;
            if (zs < 4. * ONE_METER) bot[b].aerobatic = MANEUVER;
            if (scalaire(&obj[o].rot.x, &obj[bot[b].cibv].rot.x) < 0.) bot[b].aerobatic = RECOVER;
            break;
        case RECOVER:
            adjust_throttle(b, BEST_LIFT_SPEED);
            bot[b].xctl = -obj[o].rot.y.z;
            adjust_slope(b, 0.);
            if (obj[o].rot.z.z > .8) bot[b].aerobatic = TAIL;
            break;
        case TAIL:
            subv3(&obj[bot[b].cibv].pos, &obj[o].pos, &v);
            disth = sqrt(v.x*v.x + v.y*v.y);
            u = bot[bot[b].gunned].vionvit;
            // aim beyond opponent, proportional to distance
            if (bot[b].vitlin > 0) {
                mulv(&u, .0016*disth);
                addv(&v, &u);
            }
            double dc = adjust_direction_rel(b, &v);
            dist = renorme(&v);
            double distfrap2 = 0;
            float target_speed = BEST_LIFT_SPEED;
            float min_z = 30. * ONE_METER;  // will be lowered if odds look good
            if (fabs(dc) < M_PI/4) {    // opponent is in front of us
                min_z = 20. * ONE_METER;
                if (dist < 35. * ONE_METER) {   // and close. shot ?
                    min_z = 15. * ONE_METER;
                    if (fabsf(dc) < M_PI/7) {
                        min_z = 10. * ONE_METER;
                        mulv(&obj[o].rot.x, 400);
                        addv(&obj[o].rot.x, &v);
                        renorme(&obj[o].rot.x);
                        orthov(&obj[o].rot.y, &obj[o].rot.x);
                        renorme(&obj[o].rot.y);
                        prodvect(&obj[o].rot.x, &obj[o].rot.y, &obj[o].rot.z);
                    }
                    //float const a = scalaire(&v, &obj[o].rot.y);
                    float const m = mod[obj[bot[b].cibv].model].rayoncollision;
                    if (/*fabsf(a) < 6*m && fabsf(dc) < M_PI/2*/ fabsf(dc) < M_PI/8) {
                        min_z = 8. * ONE_METER;
                        float const dz_shot = tirz(&obj[o].rot.x, disth);
                        distfrap2 = obj[bot[b].cibv].pos.z - (obj[o].pos.z + dz_shot);
                        if (fabs(distfrap2) < 3. * m) bot[b].but.canon = 1;
#                       ifdef PRINT_DEBUG
                        if (b == viewed_bot) printf("dz_shot=%f bot.z=%f cib.z=%f\n", dz_shot, obj[o].pos.z, obj[bot[b].cibv].pos.z);
#                       endif
                    }
                }
                // try to copy opponent's speed
                struct vector rel_speed;
                subv3(&bot[bot[b].gunned].vionvit, &bot[b].vionvit, &rel_speed);
                float const away_speed = scalaire(&rel_speed, &v);
#               define BEST_TAIL_DIST (7. * ONE_METER)
                // while copying, we aim for a given shooting distance
                target_speed = dist > BEST_TAIL_DIST ?
                    bot[b].vitlin + away_speed + (0.6 * ONE_METER) :
                    bot[b].vitlin + away_speed - (0.4 * ONE_METER); // be conservative with speed
            }
#           ifdef PRINT_DEBUG
            if (b == viewed_bot) printf("close, dist=%f dc=%f, distfrap2=%f, target_speed=%f\n", dist, dc, distfrap2, target_speed);
#           endif
            // if we are too low deal with the ground first
            if (zs < min_z) {
                robot_safe(b, 5. * min_z);
                break;
            }
            adjust_throttle(b, target_speed);
            // adjust slope
            adjust_slope(b, obj[bot[b].cibv].pos.z - obj[o].pos.z);
            // try to lower distfrap2
            if (dist > 0) {
                bot[b].yctl += 100. * distfrap2/dist;
                CLAMP(bot[b].yctl, 1.);
            }
            break;
    }
    // Ensure controls are within bounds
    if (bot[b].thrust < 0.) bot[b].thrust = 0.;
    else if (bot[b].thrust > 1.) bot[b].thrust = 1.;
    CLAMP(bot[b].xctl, 1.);
    CLAMP(bot[b].yctl, 1.);
}
