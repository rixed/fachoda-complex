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
#include "heightfield.h"
#include "sound.h"
#include "gtime.h"
#include "robot.h"

#define SHOT_PERIOD (300 * ONE_MILLISECOND)

//#define PRINT_DEBUG

float snd_thrust;   // thrust of the motor that can be heard

static void obj_rotate(int v, float d, float p, float g)
{
    float ar, ap, ag, c1, c2, c3, s1, s2, s3;
    struct matrix r;
    ar=d*2.; ap=p*.5; ag=g*.5;
    c1=cos(ar); s1=sin(ar); c2=cos(ap); s2=sin(ap); c3=cos(ag); s3=sin(ag);
/*  r.x.x=c2;       r.y.x=0;        r.z.x=-s2;
    r.x.y=-s1*s2;   r.y.y=c1;   r.z.y=-s1*c2;
    r.x.z=c1*s2;    r.y.z=s1;   r.z.z=c1*c2;
    */
    r.x.x=c2*c3;                r.y.x=-c2*s3;               r.z.x=-s2;
    r.x.y=c1*s3-s1*s2*c3;   r.y.y=s1*s2*s3+c1*c3;   r.z.y=-s1*c2;
    r.x.z=c1*s2*c3+s1*s3;   r.y.z=s1*c3-s3*c1*s2;   r.z.z=c1*c2;
    mulm(&obj[v].rot,&r);
    // renormalize/orthogonalize ?
}

static void obj_rotate_x(int o, float a)
{
    struct matrix m;
    copym(&m,&mat_id);
    m.y.y=cos(a);
    m.y.z=sin(a);
    m.z.y=-sin(a);
    m.z.z=cos(a);
    mulm(&obj[o].rot,&m);
}

static void obj_rotate_y(int o, float a)
{
    struct matrix m;
    copym(&m,&mat_id);
    m.x.x=cos(a);
    m.x.z=-sin(a);
    m.z.x=sin(a);
    m.z.z=cos(a);
    mulm(&obj[o].rot,&m);
}

void obj_check_pos(int i)
{
    int xk,yk,ak;
    xk=(int)floor(obj[i].pos.x/TILE_LEN)+(MAP_LEN>>1);
    yk=(int)floor(obj[i].pos.y/TILE_LEN)+(MAP_LEN>>1);
    if (xk<0) {obj[i].pos.x=-(MAP_LEN<<(LOG_TILE_LEN-1))+10; xk=0;}
    else if (xk>=MAP_LEN) {obj[i].pos.x=(MAP_LEN<<(LOG_TILE_LEN-1))-10; xk=MAP_LEN-1;}
    if (yk<10) {obj[i].pos.y=-((MAP_LEN/2-10)<<LOG_TILE_LEN)+10; yk=10;}
    else if (yk>=MAP_LEN-10) {obj[i].pos.y=((MAP_LEN/2-10)<<LOG_TILE_LEN)-10; yk=MAP_LEN-1-10;}
    if (!mod[obj[i].model].fix || !mod[obj[i].model].anchored) {   // immobile ?
        ak=xk+(yk<<LOG_MAP_LEN);
        if (ak!=obj[i].ak) {
            if (obj[i].next!=-1) obj[obj[i].next].prec=obj[i].prec;
            if (obj[i].prec!=-1) obj[obj[i].prec].next=obj[i].next;
            else map[obj[i].ak].first_obj=obj[i].next;
            obj[i].next=map[ak].first_obj;
            if (map[ak].first_obj != -1) obj[map[ak].first_obj].prec=i;
            obj[i].prec=-1;
            map[ak].first_obj=i;
            obj[i].ak=ak;
        }
    }
}

void physics_plane(int b, float dt_sec)
{
    int i, j;
    int o1 = bot[b].vion;
    int o2 = o1+n_object[bot[b].navion].nbpieces;
    struct vector u, v; struct matrix m;
    double rt;

    // FIXME: use: mulmtv(&obj[bot[b].vion].rot, &bot[b].vionvit, &v);
    double vx = scalaire(&bot[b].vionvit, &obj[bot[b].vion].rot.x);
    double vy = scalaire(&bot[b].vionvit, &obj[bot[b].vion].rot.y);
    double vz = scalaire(&bot[b].vionvit, &obj[bot[b].vion].rot.z);
#   ifdef PRINT_DEBUG
    if (b == viewed_bot) printf("vionvit=%"PRIVECTOR"\n", PVECTOR(bot[b].vionvit));
    if (b == viewed_bot) printf("dt =%f\n", dt_sec);
#   endif
#   ifdef VEC_DEBUG
    if (b == viewed_bot) {
        debug_vector[DBG_VEC_SPEED][0] = obj[bot[b].vion].pos;
        debug_vector[DBG_VEC_SPEED][1] = bot[b].vionvit;
        addv(debug_vector[DBG_VEC_SPEED]+1, debug_vector[DBG_VEC_SPEED]+0);
    }
#   endif

    if (bot[b].camp==-1) return;
    bot[b].cap = cap(obj[o1].rot.x.x,obj[o1].rot.x.y);
    bot[b].vitlin = vx;
    bot[b].xctl += bot[b].bloodloss*.02*(drand48()-.5);
    if (b < NbHosts) bot[b].xctl += bot[b].aeroloss*(b&1?.01:-.01);
    bot[b].yctl += bot[b].bloodloss*.02*(drand48()-.5);
    if (b < NbHosts) bot[b].yctl -= bot[b].aeroloss*.005;

    // controles
    if (bot[b].thrust<0) bot[b].thrust=0;
    if (bot[b].thrust>1) bot[b].thrust=1;
    if (bot[b].xctl<-1) bot[b].xctl=-1;
    if (bot[b].xctl>1) bot[b].xctl=1;
    if (bot[b].yctl<-1) bot[b].yctl=-1;
    if (bot[b].yctl>1) bot[b].yctl=1;
    if (plane_desc[bot[b].navion].nbcharngearx==0 && plane_desc[bot[b].navion].nbcharngeary==0) bot[b].but.gear=1;

    // Fiul
#   define FIUL_CONSUMPTION_SPEED .01   // 0.01 unit of fiul per second at full thrust
    bot[b].fiul -= (FIUL_CONSUMPTION_SPEED * bot[b].thrust + bot[b].fiulloss) * dt_sec;
    if (bot[b].fiul < 0) {
        bot[b].fiul = 0;
        bot[b].thrust = 0;
    }

//# define NGRAVITY
//# define NTHRUST
//# define NDRAG
//# define NTORSION
//# define NLIFT
//# define NGROUND_DRAG

    // lift, thrust and drag fades with altitude
#   define MAX_ALTITUDE (200 * ONE_METER)
    float r = obj[bot[b].vion].pos.z/MAX_ALTITUDE;
    if (r > 1.) r = 1.;
    r *= r; r *= r; r *= r;
    float const alt_factor = 1 - r;

    // Acceleration
    struct vector a = {
        0., 0.,
#       ifndef NGRAVITY
        -G
#       else
        0.
#       endif
    };
#   ifdef PRINT_DEBUG
    if (b == viewed_bot) printf("startA -> %"PRIVECTOR"\n", PVECTOR(a));
#   endif
#   ifdef VEC_DEBUG
    if (b == viewed_bot) {
        debug_vector[DBG_VEC_GRAVITY][0] = obj[bot[b].vion].pos;
        debug_vector[DBG_VEC_GRAVITY][1] = a;
        addv(debug_vector[DBG_VEC_GRAVITY]+1, debug_vector[DBG_VEC_GRAVITY]+0);
    }
#   endif

#   ifndef NTHRUST
#   define THRUST_ACC (.7 * G)  // at full thrust, with motorpower=1, fail to compensate gravity
    {   // Thrust
        double k = THRUST_ACC * bot[b].thrust * alt_factor * (1-bot[b].motorloss/128.) * plane_desc[bot[b].navion].motorpower;
        v = obj[bot[b].vion].rot.x;
        mulv(&v, k);
        addv(&a, &v);
#       ifdef PRINT_DEBUG
        if (b == viewed_bot) printf("thrust -> %"PRIVECTOR"\n", PVECTOR(a));
#       endif
#       ifdef VEC_DEBUG
        if (b == viewed_bot) {
            debug_vector[DBG_VEC_THRUST][0] = obj[bot[b].vion].pos;
            debug_vector[DBG_VEC_THRUST][1] = v;
            addv(debug_vector[DBG_VEC_THRUST]+1, debug_vector[DBG_VEC_THRUST]+0);
        }
#       endif
    }
#   endif

#   ifndef NDRAG
    {   // Drag
        double k = plane_desc[bot[b].navion].drag + .02 * bot[b].nbomb;
        if (!bot[b].but.gearup) k += .07;
        if (bot[b].but.flap) k += .03;
        k *= alt_factor;
        // linear up to around 150 and proportional to v*v afterward (so that we can't stop abruptly)
#       define LINEAR_DRAG_MAXSPEED 200
#       define LDM LINEAR_DRAG_MAXSPEED
#       define LIN_FACTOR .3
#       define SQ_FACTOR .006
#       define DRAG(what, factor) \
            fabs(what) < LDM ? \
                (factor)*LIN_FACTOR*(what) : \
                (what) > 0. ? \
                    (factor)*(LIN_FACTOR*(what) + SQ_FACTOR*((what)-LDM)*((what)-LDM)) : \
                    (factor)*(LIN_FACTOR*(what) - SQ_FACTOR*(-(what)-LDM)*(-(what)-LDM))
        v.x = DRAG(vx, k);
        v.y = DRAG(vy, k * 3.);
        v.z = DRAG(vz, k * 7.);
        mulmv(&obj[bot[b].vion].rot, &v, &u);
        subv(&a, &u);
#       ifdef PRINT_DEBUG
        if (b == viewed_bot) printf("drag   -> %"PRIVECTOR"\n", PVECTOR(a));
#       endif
#       ifdef VEC_DEBUG
        if (b == viewed_bot) {
            debug_vector[DBG_VEC_DRAG][0] = obj[bot[b].vion].pos;
            mulv(&u, -1.);
            debug_vector[DBG_VEC_DRAG][1] = u;
            addv(debug_vector[DBG_VEC_DRAG]+1, debug_vector[DBG_VEC_DRAG]+0);
        }
#       endif
    }
#   endif

    double zs = obj[bot[b].vion].pos.z - bot[b].zs; // ground altitude
#   ifndef NLIFT
    {
        float kx = vx < MIN_SPEED_FOR_LIFT ?
            0. : MIN(.00005*(vx-MIN_SPEED_FOR_LIFT)*(vx-MIN_SPEED_FOR_LIFT), 1.2*exp(-.001*vx));
        // kx max is aprox 1., when vx is around 250 (BEST_LIFT_SPEED!)
        // TODO: add lift with a-o-a?
        float lift = plane_desc[bot[b].navion].lift;
        if (bot[b].but.flap) lift *= 1.2;
        if (zs < 5. * ONE_METER) lift *= 1.1;   // more lift when close to the ground
        lift *= alt_factor; // less lift with altitude
        u = obj[bot[b].vion].rot.z;
        mulv(&u, (G * 1.) * lift * kx * (1-bot[b].aeroloss/128.));
        addv(&a, &u);
#       ifdef PRINT_DEBUG
        if (b == viewed_bot) printf("lift   -> %"PRIVECTOR"\n", PVECTOR(a));
#       endif
#       ifdef VEC_DEBUG
        if (b == viewed_bot) {
            debug_vector[DBG_VEC_LIFT][0] = obj[bot[b].vion].pos;
            debug_vector[DBG_VEC_LIFT][1] = u;
            addv(debug_vector[DBG_VEC_LIFT]+1, debug_vector[DBG_VEC_LIFT]+0);
        }
#       endif
    }
#   endif

    // Contact wheels / ground
    // loop over right, left and rear (or front) wheels
    unsigned touchdown_mask = 0;
    for (rt=0, i=0; i<3; i++) {
        // zr : altitude of this wheel, relative to the ground, at t + dt
        struct vector const *wheel_pos = &obj[bot[b].vion+plane_desc[bot[b].navion].roue[i]].pos;
        float zr = (wheel_pos->z - zs) + bot[b].vionvit.z * dt_sec;
        if (zr < 0) {
            touchdown_mask |= 1<<i;
#           ifndef NGROUND_DRAG
            // slow down plane due to contact with ground
            if (bot[b].but.gearup) { // all directions the same
                v = bot[b].vionvit;
                mulv(&v, .4);
                subv(&a, &v);
            } else {
                v.x = (bot[b].but.frein && vx < 1. * ONE_METER ? .08:.00005) * vx;
                v.y = .05 * vy;
                v.z = 0;
                mulmv(&obj[bot[b].vion].rot, &v, &u);
                subv(&a, &u);
            }
#           ifdef PRINT_DEBUG
            if (b == viewed_bot) printf("gdrag%d -> %"PRIVECTOR"\n", i, PVECTOR(a));
#           endif
#           endif
            if (zr < rt) rt = zr;

            // A wheel hit the ground, make some noise/smoke
#           define VZ_MIN_FOR_SOUND (-60.)
#           define VZ_MIN_FOR_SMOKE (-90.)
            if (vz < VZ_MIN_FOR_SOUND) {
                float t = drand48()-.5;
                if (b == viewed_bot) {
                    if (!bot[b].but.gearup) {
                        playsound(VOICE_GEAR, SAMPLE_SCREETCH, 1+t*.08, wheel_pos, false, false);
                    } else {
                        playsound(VOICE_GEAR, SAMPLE_TOLE, 1+t*.08, wheel_pos, false, false);
                    }
                }
            }
            if (vz < VZ_MIN_FOR_SMOKE) {
                int fum;
                for (fum=0; smoke_radius[fum] > 0. && fum < MAX_SMOKES; fum++);
                if (fum<MAX_SMOKES) {
                    smoke_radius[fum] = 1.;
                    smoke_type[fum] = 1;   // type poussière jaune
                    obj[smoke_start+fum].pos = *wheel_pos;
                    obj_check_pos(smoke_start+fum);
                }
            }
        }
    }

#   ifndef NTORSION
    {   // des effets de moment angulaire
        // 1 lacet pour les ailes
        // Commands are the most effective when vx=200. (then kx=1)
        float kx;
        double const kx1 = .0001*(vx-40)*(vx-40);
        double const kx2 = 1. - .00003*(vx-200)*(vx-200);
        double const kx3 = 1. -.001*vx;
        if (vx < 40) kx=0;
        else if (vx < 200) kx = MIN(kx1, kx2);
        else kx = MAX(kx2, kx3);
        if (kx < 0) kx = 0;

        if (easy_mode || b>=NbHosts) kx*=1.2;

        // les ailes aiment bien etre de front (girouette)
        double const prof = .002*vz + bot[b].yctl * kx;
        double const gouv = .001*vy +
            // rear wheel in contact with the ground
            (touchdown_mask & 4 ? -vx*.1*bot[b].xctl : 0.);

        double const deriv =
            (bot[b].xctl * kx)/(1. + .05*bot[b].nbomb) /*+
            .00001 * obj[bot[b].vion].rot.y.z*/;

        obj_rotate(bot[b].vion, deriv*dt_sec, prof*dt_sec, gouv*dt_sec);
    }
#   endif

    if (touchdown_mask) {
#       ifdef PRINT_DEBUG
        if (b == viewed_bot) printf("hit ground, vz=%f\n", vz);
#       endif

        // So we hit the ground. With what speed?
        bool const easy = easy_mode || b>=NbHosts;
        float const vz_min = easy ? -150. : -100.;
        float const vz_min_rough = easy ? -100. : -70.;
        if (
            (vz < vz_min) ||
            (vz < vz_min_rough && (bot[b].but.gearup || submap_get(obj[bot[b].vion].ak)!=0))
        ) {
            explose(bot[b].vion,bot[b].vion);
            return;
        } else {
            // nice landing !
            if (b==controled_bot && bot[b].is_flying && fabs(obj[bot[b].vion].rot.x.x)>.8) {
                subv3(&obj[bot[b].babase].pos,&obj[bot[b].vion].pos, &v);
                if (norme2(&v) < TILE_LEN*TILE_LEN*1.5) {
                    bot[b].gold += 300;
                    playsound(VOICE_EXTER, SAMPLE_BRAVO, 1, &voices_in_my_head, true, false);
                }
            }
        }
        bot[b].is_flying = false;

        obj[bot[b].vion].pos.z -= rt;   // At t+dt, be out of the ground
        a.z = - bot[b].vionvit.z;
#       ifdef PRINT_DEBUG
        if (b == viewed_bot) printf("ground -> %"PRIVECTOR"\n", PVECTOR(a));
#       endif

        float const fix_orient = rt * 1. * dt_sec;
        if (touchdown_mask&3  && !(touchdown_mask&4)) { // either right or left wheel but not front/rear -> noise down/up
            if (plane_desc[bot[b].navion].avant) obj_rotate_y(bot[b].vion, -2.*fix_orient);
            else obj_rotate_y(bot[b].vion, 2.*fix_orient);
        } else if (!(touchdown_mask&3) && touchdown_mask&4) {   // front/read but neither left nor right -> noise up/down
            if (plane_desc[bot[b].navion].avant) obj_rotate_y(bot[b].vion, fix_orient);
            else obj_rotate_y(bot[b].vion, -fix_orient);
        }
        if (touchdown_mask&1 && !(touchdown_mask&2)) {  // right but not left
            obj_rotate_x(bot[b].vion, -fix_orient);
        } else if (touchdown_mask&2 && !(touchdown_mask&1)) {   // left but not right
            obj_rotate_x(bot[b].vion, fix_orient);
        }

        // Reload
        if (touchdown_mask && vx < .05 * ONE_METER) {
            v = obj[bot[b].babase].pos;
            subv(&v, &obj[bot[b].vion].pos);
            if (norme2(&v) < TILE_LEN*TILE_LEN*.1) {
                int prix, amo;
                amo = bot[b].gold;
                if (amo+bot[b].bullets > plane_desc[bot[b].navion].bulletsmax) {
                    amo = plane_desc[bot[b].navion].bulletsmax - bot[b].bullets;
                }
                if (amo) {
                    bot[b].gold -= amo;
                    bot[b].bullets += amo;
                }
                amo = bot[b].gold*1000;
                if (amo+bot[b].fiul > plane_desc[bot[b].navion].fiulmax) {
                    amo = plane_desc[bot[b].navion].fiulmax - bot[b].fiul;
                }
                if (amo >= 1000) {
                    bot[b].gold -= amo/1000;
                    bot[b].fiul += amo;
                }
                for (i=o1; i<o2; i++) {
                    int mo=obj[i].model;
                    if (obj[i].objref==bot[b].babase && mod[mo].type==TYPE_BOMB) {
                        prix=200;
                        if (prix>bot[b].gold) break;
                        bot[b].gold-=prix;
                        obj[i].objref=bot[b].vion;
                        obj[i].type=mod[obj[i].model].type;
                        copym(&obj[i].rot,&mat_id);
                        copyv(&obj[i].pos,&mod[mo].offset);
                        bot[b].fiul=plane_desc[bot[b].navion].fiulmax;
                        armstate(b);
                    }
                }
                if (bot[b].fiulloss) {
                    bot[b].gold-=bot[b].fiulloss;
                    bot[b].fiulloss=0;
                }
                if (bot[b].motorloss) {
                    bot[b].gold-=bot[b].motorloss*10;
                    bot[b].motorloss=0;
                }
                if (bot[b].aeroloss) {
                    bot[b].gold-=bot[b].aeroloss*10;
                    bot[b].aeroloss=0;
                }
                if (bot[b].bloodloss) {
                    bot[b].gold-=bot[b].bloodloss;
                    bot[b].bloodloss=0;
                }
                if (bot[b].burning) {
                    bot[b].gold-=bot[b].burning*10;
                    bot[b].burning=0;
                }
                if (bot[b].but.business) {
                    i = bot[b].vion;
                    do {
                        int v;
                        bot[b].gold += plane_desc[bot[b].navion].prix;
                        do {
                            i=obj[i].next;
                            if (i==-1) i=map[obj[bot[b].vion].ak].first_obj;
                        } while (obj[i].type!=TYPE_PLANE || !mod[obj[i].model].fix || mod[obj[i].model].anchored);
                        bot[b].gold -= plane_desc[bot[b].navion].prix;
                        for (j=v=0; v<NBBOT; v++) {
                            if (v!=b && bot[v].vion==i) { j=1; break; }
                        }
                    } while (j || bot[b].gold<0);
                    if (i != bot[b].vion) {
                        bot[b].navion = mod[obj[i].model].n_object;
                        bot[b].vion = i;
                        armstate(b);
                        playsound(VOICE_EXTER, SAMPLE_TARATATA, 1+(bot[b].navion-1)*.1, &voices_in_my_head, true, false);
                    }
                }
            }
        }
    } else /* !touchdown */ if (bot[b].zs > 30) {
        bot[b].is_flying = true;
    }

    // Done computing acceleration, now move plane
    mulv(&a, dt_sec);
    addv(&bot[b].vionvit, &a);
#   ifdef PRINT_DEBUG
    if (b == viewed_bot) printf("* dt   -> %"PRIVECTOR"\n", PVECTOR(a));
    if (b == viewed_bot) printf("velocity= %"PRIVECTOR"\n", PVECTOR(bot[b].vionvit));
#   endif
    v = bot[b].vionvit;
    mulv(&v, dt_sec);
    addv(&obj[bot[b].vion].pos, &v);
    obj_check_pos(bot[b].vion);

    // And the attached parts follow
    // Moyeux d'hélice
    for (i=1; i<plane_desc[bot[b].navion].nbmoyeux+1; i++) {
        m.x.x=1; m.x.y=0; m.x.z=0;
        m.y.x=0; m.y.y=cos(bot[b].anghel); m.y.z=sin(bot[b].anghel);
        m.z.x=0; m.z.y=-sin(bot[b].anghel); m.z.z=cos(bot[b].anghel);
        calcposarti(bot[b].vion+i,&m);
    }
#   define PLANE_PROPELLER_ROTATION_SPEED (2. * M_PI * 10.) // per seconds
    bot[b].anghel += PLANE_PROPELLER_ROTATION_SPEED * bot[b].thrust * dt_sec;

    // Engine sound
    // TODO: have a second motor voice for another bot but the viewed_bot
    if (b == viewed_bot && bot[b].thrust != snd_thrust) {
        snd_thrust = bot[b].thrust;
#       define LOW_2_HI_REGIME 0.18
        if (snd_thrust > LOW_2_HI_REGIME) {
            float const pitch = 1. + 0.1 * (snd_thrust - .8);
            playsound(VOICE_MOTOR, SAMPLE_MOTOR, pitch, &obj[bot[b].vion].pos, false, true);   // FIXME: the actual pos of the engine
        } else {
            float const pitch = 1. + (snd_thrust - LOW_2_HI_REGIME/2.);
            playsound(VOICE_MOTOR, SAMPLE_LOW_SPEED, pitch, &obj[bot[b].vion].pos, false, true);
        }
    }

    // charnières des essieux
    if (bot[b].but.gear) {
        if (bot[b].but.gearup) {
            bot[b].but.gearup=0;
            if (b == viewed_bot) playsound(VOICE_GEAR, SAMPLE_GEAR_DN, 1, &obj[bot[b].vion].pos, false, false);   // FIXME: the pos of the gear
            for (j=0; j<(plane_desc[bot[b].navion].retract3roues?3:2); j++) obj[bot[b].vion+plane_desc[bot[b].navion].roue[j]].aff=1;
        }
#       define GEAR_ROTATION_SPEED (.5 * M_PI / 1.5) // deployed in 1.5 seconds
        bot[b].anggear -= GEAR_ROTATION_SPEED * dt_sec;
        if (bot[b].anggear<0) bot[b].anggear=0;
    } else if (!bot[b].but.gearup) {
        if (b == viewed_bot && bot[b].anggear<.1) playsound(VOICE_GEAR, SAMPLE_GEAR_UP, 1., &obj[bot[b].vion].pos, false, false); // FIXME: the pos of the gear
        bot[b].anggear += GEAR_ROTATION_SPEED * dt_sec;
        if (bot[b].anggear>1.5) {
            bot[b].anggear=1.5; bot[b].but.gearup=1;
            for (j=0; j<(plane_desc[bot[b].navion].retract3roues?3:2); j++) obj[bot[b].vion+plane_desc[bot[b].navion].roue[j]].aff=0;
        }
    }
    for (j=i; j<plane_desc[bot[b].navion].nbcharngearx+i; j++) {
        m.y.y=cos(bot[b].anggear);
        m.y.z=((j-i)&1?-1:1)*sin(bot[b].anggear);
        m.z.y=((j-i)&1?1:-1)*sin(bot[b].anggear);
        m.z.z=cos(bot[b].anggear);
        calcposarti(bot[b].vion+j,&m);
    }
    m.y.x=0; m.y.y=1; m.y.z=0;
    m.z.y=0;
    for (i=j; i<j+plane_desc[bot[b].navion].nbcharngeary; i++) {
        m.x.x=cos(bot[b].anggear); m.x.z=((i-j)&1?1:-1)*sin(bot[b].anggear);
        m.z.x=((i-j)&1?-1:1)*sin(bot[b].anggear); m.z.z=cos(bot[b].anggear);
        calcposarti(bot[b].vion+i,&m);
    }
    // charnière de profondeur
    m.x.x=cos(bot[b].yctl);
    m.x.z=-sin(bot[b].yctl);
    m.z.x=sin(bot[b].yctl);
    m.z.z=cos(bot[b].yctl);
    calcposarti(bot[b].vion+i,&m);
    // charnières de rouli
    m.x.x=cos(bot[b].xctl);
    m.x.z=-sin(bot[b].xctl);
    m.z.x=sin(bot[b].xctl);
    m.z.z=cos(bot[b].xctl);
    calcposarti(bot[b].vion+i+1,&m);
    m.x.z=-m.x.z;
    m.z.x=-m.z.x;
    calcposarti(bot[b].vion+i+2,&m);
    for (i+=3; i<n_object[mod[obj[bot[b].vion].model].n_object].nbpieces; i++) {
        if (obj[bot[b].vion+i].objref!=-1) calcposrigide(bot[b].vion+i);
    }

    // shots, which frequency is given by the number of canons
    gtime const min_shot_period = SHOT_PERIOD / plane_desc[bot[b].navion].nbcanon;
    if (bot[b].but.canon && nb_shot<MAX_SHOTS && bot[b].bullets>0 && gtime_age(bot[b].last_shot) > min_shot_period) {
        if (++bot[b].alterc>=4) bot[b].alterc=0;
        if (bot[b].alterc<plane_desc[bot[b].navion].nbcanon) {
            copyv(&v, &obj[bot[b].vion].rot.x);
            mulv(&v, 44);
            struct vector const *canon = &obj[ bot[b].vion + plane_desc[bot[b].navion].firstcanon + bot[b].alterc ].pos;
            addv(&v, canon);
            if (b == viewed_bot) playsound(VOICE_SHOT, SAMPLE_SHOT, 1+(drand48()-.5)*.08, &v, false, false);
            else drand48();
            gunner[nb_obj-shot_start]=b;
            shot_ttl[nb_obj-shot_start] = 5.;   // TTL of the shot in seconds
            object_add(0, &v, &obj[bot[b].vion].rot, -1, 0);
            nb_shot++;
            bot[b].bullets--;
            bot[b].last_shot = gtime_last();
        }
    }

    if (bot[b].but.bomb) {
        for (i=bot[b].vion; i<bot[b].vion+n_object[bot[b].navion].nbpieces && (obj[i].type!=TYPE_BOMB || obj[i].objref!=bot[b].vion); i++);
        if (i<bot[b].vion+n_object[bot[b].navion].nbpieces) {
            if (b == viewed_bot) playsound(VOICE_GEAR, SAMPLE_BIPBIP2, 1.1, &obj[i].pos, false, false);
            obj[i].objref=-1;
            for (j=0; j<bombidx && bomb[j].o!=-1; j++);
            if (j>=bombidx) bombidx=j+1;
            copyv(&bomb[j].vit,&bot[b].vionvit);
            bomb[j].o=i;
            bomb[j].b=b;
            bot[b].nbomb--;
        }
    }

    if (bot[b].but.mark && bot[b].camp==bot[controled_bot].camp) {
        mark[next_mark_set] = obj[bot[b].vion].pos;
        if (++next_mark_set>NB_MARKS) next_mark_set=0;
    }

    bot[b].but.bomb = bot[b].but.canon = bot[b].but.business = bot[b].but.mark = 0;
}

void physics_tank(int v, float dt_sec)
{
    struct vector p,u;
    int o=tank[v].o1, i;
    struct matrix m;
    double c,s;
    if (tank[v].camp==-1) return;
    c=cos(tank[v].ang0);
    s=sin(tank[v].ang0);
    m.x.x=c; m.x.y=s; m.x.z=0;
    m.y.x=-s; m.y.y=c; m.y.z=0;
    m.z.x=0; m.z.y=0; m.z.z=1;
    copym(&obj[o].rot,&m);
    if (tank[v].moteur) {
#       define TANK_SPEED (1.4 * ONE_METER) // per seconds
        copyv(&p, &obj[o].rot.x);
        mulv(&p, TANK_SPEED * dt_sec);
        addv(&obj[o].pos, &p);
        obj[o].pos.z = z_ground(obj[o].pos.x,obj[o].pos.y, true);
        obj_check_pos(o);
    }
    c=cos(tank[v].ang1);
    s=sin(tank[v].ang1);
    m.x.x=c; m.x.y=s; m.x.z=0;
    m.y.x=-s; m.y.y=c; m.y.z=0;
    calcposarti(o+1,&m);
    c=cos(tank[v].ang2);
    s=sin(tank[v].ang2);
    m.x.x=c; m.x.y=0; m.x.z=s;
    m.y.x=0; m.y.y=1; m.y.z=0;
    m.z.x=-s;m.z.y=0; m.z.z=c;
    calcposarti(o+2,&m);
    for (i=o+3; i<tank[v].o2; i++) calcposrigide(i);

    gtime const min_shot_period = SHOT_PERIOD / 4;
    if (tank[v].tir && nb_shot < MAX_SHOTS && gtime_age(tank[v].last_shot) > min_shot_period) {
        tank[v].last_shot = gtime_last();
        p = obj[o+3+tank[v].ocanon].rot.x;
        mulv(&p, 90.);
        u = obj[o+3+tank[v].ocanon].pos;
        addv(&p, &u);
        gunner[nb_obj-shot_start] = v|(1<<NTANKMARK);
        shot_ttl[nb_obj-shot_start] = 4.;
        object_add(0, &p, &obj[o+3+tank[v].ocanon].rot, -1, 0);
        if (++tank[v].ocanon >= 4) tank[v].ocanon = 0;
        nb_shot ++;
    }
}

static void zep_shot(int z, struct vector *c, int i)
{
    gtime const min_shot_period = SHOT_PERIOD / 6; // 6 cannons per Zeppelin
    if (nb_shot < MAX_SHOTS && gtime_age(zep[z].last_shot) > min_shot_period) {
        zep[z].last_shot = gtime_last();
        struct vector p = *c;
        renorme(&p);
        float s = scalaire(&p, &obj[zep[z].o].rot.y);
        if (i<3) s = -s;
        if (s<.5) return;
        // spread bullets slightly
        p.x += 0.01*(drand48()-.5);
        p.y += 0.01*(drand48()-.5);
        p.z += 0.01*(drand48()-.5);
        renorme(&p);
        struct matrix m;
        m.x = p;
        m.y.x=m.y.y=m.y.z=1;
        orthov(&m.y, &m.x);
        renorme(&m.y);
        prodvect(&m.x,&m.y,&m.z);
        mulv(&p, 40);
        addv(&p, &obj[zep[z].o+5+i].pos);
        gunner[nb_obj-shot_start]=-1;    // passe inapercu (ie pas pris pour cible en retours)
        shot_ttl[nb_obj-shot_start] = 4.;
        object_add(0, &p, &m, -1, 0);
        nb_shot++;
    }
}

void physics_zep(int z, float dt_sec)
{ // fait office de routine robot sur les commandes aussi
    int i;
    struct vector v;
    struct matrix m;
    float dir;
    float cx,cy,cz,sx,sy,sz, zs, angcap=0, angprof=0;
    static float phazx=0, phazy=0, phazz=0, balot=0;
    balot += .04/NBZEP;    // since this function will be called for each zep. how lame!
    if (obj[zep[z].o].pos.z>50000) return;
    zs=z_ground(obj[zep[z].o].pos.x,obj[zep[z].o].pos.y, true);
#   define ZEP_SPEED (1.5 * ONE_METER)  // per seconds
    if (zep[z].vit > ZEP_SPEED) {
        // Deflate
        zep[z].angy += 5. * sin(phazy+=drand48()) * dt_sec;
        zep[z].angx += 5. * sin(phazx+=drand48()) * dt_sec;
        zep[z].angz += 5. * sin(phazz+=drand48()) * dt_sec;
        obj[zep[z].o].pos.z += zep[z].vit * dt_sec;
    } else {
        // Commands
        v = zep[z].nav;
        subv(&v, &obj[zep[z].o].pos);
        if (norme2(&v) < 2000) {
            zep[z].nav.x=(MAP_LEN<<LOG_TILE_LEN)*drand48()*(killemall_mode?.1:.7);
            zep[z].nav.y=(MAP_LEN<<LOG_TILE_LEN)*drand48()*(killemall_mode?.1:.7);
            zep[z].nav.z=1000+drand48()*3000+z_ground(v.x,v.y, false);
        } else {
            dir=cap(v.x,v.y)-zep[z].angz;
            if (dir<-M_PI) dir+=2*M_PI;
            else if (dir>M_PI) dir-=2*M_PI;
            angcap=-dir*.9;
            angprof = 10*(v.z-obj[zep[z].o].pos.z)/(1.+MAX(fabs(v.y),fabs(v.x)));
            if (obj[zep[z].o].pos.z-zs<3000) angprof=1;
            if (angprof>1) angprof=1;
            else if (angprof<-1) angprof=-1;
            zep[z].vit = ZEP_SPEED;
        }
        // Move
        zep[z].angy -= angprof * .0001 * zep[z].vit * dt_sec;
        if (zep[z].angy > .4) zep[z].angy = .4;
        else if (zep[z].angy < -.4) zep[z].angy = -.4;
        zep[z].angz -= angcap * .00004 * zep[z].vit * dt_sec;
        zep[z].angx = 10. * sin(balot) * dt_sec;
    }
    cx=cos(zep[z].angx);
    sx=sin(zep[z].angx);
    cy=cos(zep[z].angy);
    sy=sin(zep[z].angy);
    cz=cos(zep[z].angz);
    sz=sin(zep[z].angz);
    obj[zep[z].o].rot.x.x=cy*cz;
    obj[zep[z].o].rot.x.y=sz*cy;
    obj[zep[z].o].rot.x.z=-sy;
    obj[zep[z].o].rot.y.x=cz*sx*sy-sz*cx;
    obj[zep[z].o].rot.y.y=sz*sx*sy+cz*cx;
    obj[zep[z].o].rot.y.z=sx*cy;
    obj[zep[z].o].rot.z.x=cz*cx*sy+sz*sx;
    obj[zep[z].o].rot.z.y=sz*cx*sy-cz*sx;
    obj[zep[z].o].rot.z.z=cx*cy;

    v = obj[zep[z].o].rot.x;
    mulv(&v, zep[z].vit * dt_sec);
    addv(&obj[zep[z].o].pos, &v);
    if (obj[zep[z].o].pos.z < zs) obj[zep[z].o].pos.z=zs;
    obj_check_pos(zep[z].o);

    // Propellers
    m.x.x=1; m.x.y=m.x.z=m.y.x=m.z.x=0;
    m.z.z=m.y.y=cos(zep[z].anghel);
    m.y.z=sin(zep[z].anghel);
    m.z.y=-m.y.z;
    calcposarti(zep[z].o+1,&m);
    m.y.z=-m.y.z; m.z.y=-m.z.y;
    calcposarti(zep[z].o+2,&m);
    zep[z].anghel += zep[z].vit * dt_sec;

    // Animated controls
    cz=cos(angcap);
    sz=sin(angcap);
    cy=cos(angprof);
    sy=sin(angprof);
    m.x.x=cz; m.x.y=sz; m.x.z=0;
    m.y.x=-sz; m.y.y=cz; m.y.z=0;
    m.z.x=0; m.z.y=0; m.z.z=1;
    calcposarti(zep[z].o+3,&m);
    m.x.x=cy; m.x.y=0; m.x.z=-sy;
    m.y.x=0; m.y.y=1; m.y.z=0;
    m.z.x=sy; m.z.y=0; m.z.z=cy;
    calcposarti(zep[z].o+4,&m);

    // Targets
    for (i=0; i<6; i++) {
        if (zep[z].cib[i]==-1) {
            int nc=NBBOT*drand48();
            copyv(&v,&obj[bot[nc].vion].pos);
            subv(&v,&obj[zep[z].o].pos);
            if (norme2(&v)<TILE_LEN*TILE_LEN*0.5) zep[z].cib[i]=nc;
        }
    }

    // Shots
    for (i=5; i<5+6; i++) {
        if (zep[z].cib[i-5]!=-1) {
            subv3(&obj[bot[zep[z].cib[i-5]].vion].pos,&obj[zep[z].o+i].pos,&v);
            if (norme2(&v)>TILE_LEN*TILE_LEN*0.5) {
                zep[z].cib[i-5]=-1;
            } else {
                zep_shot(z, &v, i-5);        //dans gunner, laisser -1 ?
            }
        }
    }

    // Everything else
    for (i=5; i<n_object[mod[obj[zep[z].o].model].n_object].nbpieces; i++) calcposrigide(zep[z].o+i);
}

