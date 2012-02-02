// -*- c-basic-offset: 4; c-backslash-column: 79; indent-tabs-mode: nil -*-
// vim:sw=4 ts=4 sts=4 expandtab
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "map.h"
#include "sound.h"
#include "gtime.h"

#define SHOT_PERIOD (200 * ONE_MILLISECOND)

//#define PRINT_DEBUG

float soundthrust;
int IsFlying;

void controlepos(int i) {
    int xk,yk,ak;
    xk=(int)floor(obj[i].pos.x/ECHELLE)+(WMAP>>1);
    yk=(int)floor(obj[i].pos.y/ECHELLE)+(WMAP>>1);
    if (xk<0) {obj[i].pos.x=-(WMAP<<(NECHELLE-1))+10; xk=0;}
    else if (xk>=WMAP) {obj[i].pos.x=(WMAP<<(NECHELLE-1))-10; xk=WMAP-1;}
    if (yk<10) {obj[i].pos.y=-((WMAP/2-10)<<NECHELLE)+10; yk=10;}
    else if (yk>=WMAP-10) {obj[i].pos.y=((WMAP/2-10)<<NECHELLE)-10; yk=WMAP-1-10;}
    if (mod[obj[i].model].fixe!=-1) {   // immobile ?
        ak=xk+(yk<<NWMAP);
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
void control_plane(int b, float dt_sec) {
    int i, j;
    int o1 = bot[b].vion;
    int o2 = o1+nobjet[bot[b].navion].nbpieces;
    vector u, v; matrix m;
    double rt;

    // FIXME: use: mulmtv(&obj[bot[b].vion].rot, &bot[b].vionvit, &v);
    double vx = scalaire(&bot[b].vionvit, &obj[bot[b].vion].rot.x);
    double vy = scalaire(&bot[b].vionvit, &obj[bot[b].vion].rot.y);
    double vz = scalaire(&bot[b].vionvit, &obj[bot[b].vion].rot.z);
#   ifdef PRINT_DEBUG
    if (b == visubot) printf("vionvit=%"PRIVECTOR"\n", PVECTOR(bot[b].vionvit));
    if (b == visubot) printf("dt =%f\n", dt_sec);
#   endif
#   ifdef VEC_DEBUG
    if (b == visubot) {
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
    if (viondesc[bot[b].navion].nbcharngearx==0 && viondesc[bot[b].navion].nbcharngeary==0) bot[b].but.gear=1;

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
    float const alt_factor = 1 - r*r*r*r;

    // Acceleration
    vector a = {
        0., 0.,
#       ifndef NGRAVITY
        -G
#       else
        0.
#       endif
    };
#   ifdef PRINT_DEBUG
    if (b == visubot) printf("startA -> %"PRIVECTOR"\n", PVECTOR(a));
#   endif
#   ifdef VEC_DEBUG
    if (b == visubot) {
        debug_vector[DBG_VEC_GRAVITY][0] = obj[bot[b].vion].pos;
        debug_vector[DBG_VEC_GRAVITY][1] = a;
        addv(debug_vector[DBG_VEC_GRAVITY]+1, debug_vector[DBG_VEC_GRAVITY]+0);
    }
#   endif

#   ifndef NTHRUST
#   define THRUST_ACC (1. * G)  // at full thrust, with motorpower=1, fail to compensate gravity
    {   // Thrust
        double k = THRUST_ACC * bot[b].thrust * alt_factor * (1-bot[b].motorloss/128.) * viondesc[bot[b].navion].motorpower;
        v = obj[bot[b].vion].rot.x;
        mulv(&v, k);
        addv(&a, &v);
#       ifdef PRINT_DEBUG
        if (b == visubot) printf("thrust -> %"PRIVECTOR"\n", PVECTOR(a));
#       endif
#       ifdef VEC_DEBUG
        if (b == visubot) {
            debug_vector[DBG_VEC_THRUST][0] = obj[bot[b].vion].pos;
            debug_vector[DBG_VEC_THRUST][1] = v;
            addv(debug_vector[DBG_VEC_THRUST]+1, debug_vector[DBG_VEC_THRUST]+0);
        }
#       endif
    }
#   endif

#   ifndef NDRAG
    {   // Drag
        double k = viondesc[bot[b].navion].drag + .02 * bot[b].nbomb;
        if (!bot[b].but.gearup) k += .07;
        if (bot[b].but.flap) k += .03;
        k *= alt_factor;
        // linear up to around 150 and proportional to v*v afterward (so that we can't stop abruptly)
#       define LINEAR_DRAG_MAXSPEED 150
#       define LDM LINEAR_DRAG_MAXSPEED
#       define LIN_FACTOR .6
#       define SQ_FACTOR .003
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
        if (b == visubot) printf("drag   -> %"PRIVECTOR"\n", PVECTOR(a));
#       endif
#       ifdef VEC_DEBUG
        if (b == visubot) {
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
    {   // miracle des airs, les ailes portent...
#       define MIN_SPEED_FOR_LIFT 90
        float kx = vx < MIN_SPEED_FOR_LIFT ?
            0. : MIN(.0001*(vx-MIN_SPEED_FOR_LIFT)*(vx-MIN_SPEED_FOR_LIFT), 1.2*exp(-.001*vx));
        // kx max is aprox 1., when vx is around 200
        // TODO: add lift with a-o-a?
        float lift = viondesc[bot[b].navion].lift;
        if (bot[b].but.flap) lift *= 1.2;
        if (zs < 2. * ONE_METER) lift *= 1.1;   // more lift when close to the ground
        lift *= alt_factor; // less lift with altitude
        u = obj[bot[b].vion].rot.z;
        mulv(&u, (G * 1.) * lift * kx * (1-bot[b].aeroloss/128.));
        addv(&a, &u);
#       ifdef PRINT_DEBUG
        if (b == visubot) printf("lift   -> %"PRIVECTOR"\n", PVECTOR(a));
#       endif
#       ifdef VEC_DEBUG
        if (b == visubot) {
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
        vector const *wheel_pos = &obj[bot[b].vion+viondesc[bot[b].navion].roue[i]].pos;
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
                v.x = (bot[b].but.frein && vx < 150. ? .08:.00005) * vx;
                v.y = .05 * vy;
                v.z = 0;
                mulmv(&obj[bot[b].vion].rot, &v, &u);
                subv(&a, &u);
            }
#           ifdef PRINT_DEBUG
            if (b == visubot) printf("gdrag%d -> %"PRIVECTOR"\n", i, PVECTOR(a));
#           endif
#           endif
            if (zr < rt) rt = zr;

            // A wheel hit the ground, make some noise/smoke
#           define VZ_MIN_FOR_SOUND (-60.)
#           define VZ_MIN_FOR_SMOKE (-90.)
            if (vz < VZ_MIN_FOR_SOUND) {
                float t = drand48()-.5;
                if (b == visubot) {
                    if (!bot[b].but.gearup) {
                        playsound(VOICEGEAR, SCREETCH, 1+t*.08, wheel_pos, false);
                    } else {
                        playsound(VOICEGEAR, TOLE, 1+t*.08, wheel_pos, false);
                    }
                }
            }
            if (vz < VZ_MIN_FOR_SMOKE) {
                int fum;
                for (fum=0; rayonfumee[fum] && fum<NBMAXFUMEE; fum++);
                if (fum<NBMAXFUMEE) {
                    rayonfumee[fum]=1;
                    typefumee[fum]=1;   // type poussière jaune
                    obj[firstfumee+fum].pos = *wheel_pos;
                    controlepos(firstfumee+fum);
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

        if (Easy || b>=NbHosts) kx*=1.2;

        // les ailes aiment bien etre de front (girouette)
        double const prof = .001*vz + bot[b].yctl * kx;
        double const gouv = .001*vy +
            // rear wheel in contact with the ground
            (touchdown_mask & 4 ? -vx*.06*bot[b].xctl : 0.);

        double const deriv =
            (bot[b].xctl * kx)/(1. + .05*bot[b].nbomb) /*+
            .00001 * obj[bot[b].vion].rot.y.z*/;

        tournevion(bot[b].vion, deriv*dt_sec, prof*dt_sec, gouv*dt_sec);
    }
#   endif

    if (touchdown_mask) {
#       ifdef PRINT_DEBUG
        if (b == visubot) printf("hit ground, vz=%f\n", vz);
#       endif

        // So we hit the ground. With what speed?
        bool const easy = Easy || b>=NbHosts;
        float const vz_min = easy ? -100. : -80.;
        float const vz_min_rough = easy ? -80. : -50.;
        if (
            (vz < vz_min) ||
            (vz < vz_min_rough && (bot[b].but.gearup || submap_get(obj[bot[b].vion].ak)!=0))
        ) {
            explose(bot[b].vion,bot[b].vion);
            return;
        } else {
            // nice landing !
            if (b==bmanu && IsFlying && fabs(obj[bot[b].vion].rot.x.x)>.8) {
                subv3(&obj[bot[b].babase].pos,&obj[bot[b].vion].pos, &v);
                if (norme2(&v) < ECHELLE*ECHELLE*1.5) {
                    bot[b].gold += 300;
                    playsound(VOICEEXTER, BRAVO, 1, &voices_in_my_head, true);
                }
            }
        }
        if (b==bmanu && IsFlying) {
            IsFlying = 0;
        }

/*      a.z -= rt / (dt_sec * dt_sec);
        if (b == visubot) printf("ground -> %"PRIVECTOR"\n", PVECTOR(a));*/
        obj[bot[b].vion].pos.z -= rt;   // at t+dt, be out of the ground
        bot[b].vionvit.z = 0.;
        a.z = 0.;
#       ifdef PRINT_DEBUG
        if (b == visubot) printf("ground -> %"PRIVECTOR"\n", PVECTOR(a));
#       endif

        float const fix_orient = rt * .1 * dt_sec;
        if (touchdown_mask&3  && !(touchdown_mask&4)) { // either right or left wheel but not front/rear -> noise down/up
            if (viondesc[bot[b].navion].avant) basculeY(bot[b].vion, -fix_orient);
            else basculeY(bot[b].vion, fix_orient);
        } else if (!(touchdown_mask&3) && touchdown_mask&4) {   // front/read but neither left nor right -> noise up/down
            if (viondesc[bot[b].navion].avant) basculeY(bot[b].vion, fix_orient);
            else basculeY(bot[b].vion, -fix_orient);
        }
        if (touchdown_mask&1 && !(touchdown_mask&2)) {  // right but not left
            basculeX(bot[b].vion, -fix_orient);
        } else if (touchdown_mask&2 && !(touchdown_mask&1)) {   // left but not right
            basculeX(bot[b].vion, fix_orient);
        }
        //if (touchdown_mask&3) basculeZ(bot[b].vion, -bot[b].xctl* dt_sec);    // petite gruge pour diriger en roulant

        // Reload
        if (touchdown_mask && vx < .5 * ONE_METER) {
            v = obj[bot[b].babase].pos;
            subv(&v, &obj[bot[b].vion].pos);
            if (norme2(&v) < ECHELLE*ECHELLE*.1) {
                int prix, amo;
                amo = bot[b].gold;
                if (amo+bot[b].bullets > viondesc[bot[b].navion].bulletsmax) {
                    amo = viondesc[bot[b].navion].bulletsmax - bot[b].bullets;
                }
                if (amo) {
                    bot[b].gold -= amo;
                    bot[b].bullets += amo;
                }
                amo = bot[b].gold*1000;
                if (amo+bot[b].fiul > viondesc[bot[b].navion].fiulmax) {
                    amo = viondesc[bot[b].navion].fiulmax - bot[b].fiul;
                }
                if (amo >= 1000) {
                    bot[b].gold -= amo/1000;
                    bot[b].fiul += amo;
                }
                for (i=o1; i<o2; i++) {
                    int mo=obj[i].model;
                    if (obj[i].objref==bot[b].babase && mod[mo].type==BOMB) {
                        prix=200;
                        if (prix>bot[b].gold) break;
                        bot[b].gold-=prix;
                        obj[i].objref=bot[b].vion;
                        obj[i].type=mod[obj[i].model].type;
                        copym(&obj[i].rot,&mat_id);
                        copyv(&obj[i].pos,&mod[mo].offset);
                        bot[b].fiul=viondesc[bot[b].navion].fiulmax;
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
                if (bot[b].but.commerce) {
                    i=bot[b].vion;
                    do {
                        int v;
                        bot[b].gold+=viondesc[bot[b].navion].prix;
                        do {
                            i=obj[i].next;
                            if (i==-1) i=map[obj[bot[b].vion].ak].first_obj;
                        } while (obj[i].type!=AVION || mod[obj[i].model].fixe!=0);
                        bot[b].navion=mod[obj[i].model].nobjet;
                        bot[b].gold-=viondesc[bot[b].navion].prix;
                        for (j=v=0; v<NBBOT; v++) {
                            if (v!=b && bot[v].vion==i) { j=1; break; }
                        }
                    } while (j || bot[b].gold<0);
                    if (i!=bot[b].vion) {
                        bot[b].vion=i;
                        armstate(b);
                        playsound(VOICEEXTER, TARATATA, 1+(bot[b].navion-1)*.1, &voices_in_my_head, true);
                    }
                }
            }
        }
    } else /* !touchdown */ if (b==bmanu && bot[b].zs > 100 && !IsFlying) {
        IsFlying = 1;
    }

    // Done computing acceleration, now move plane
    mulv(&a, dt_sec);
    addv(&bot[b].vionvit, &a);
#   ifdef PRINT_DEBUG
    if (b == visubot) printf("* dt   -> %"PRIVECTOR"\n", PVECTOR(a));
    if (b == visubot) printf("velocity= %"PRIVECTOR"\n", PVECTOR(bot[b].vionvit));
#   endif
    v = bot[b].vionvit;
    mulv(&v, dt_sec);
    addv(&obj[bot[b].vion].pos, &v);
    controlepos(bot[b].vion);

    // And the attached parts follow
    // Moyeux d'hélice
    for (i=1; i<viondesc[bot[b].navion].nbmoyeux+1; i++) {
        m.x.x=1; m.x.y=0; m.x.z=0;
        m.y.x=0; m.y.y=cos(bot[b].anghel); m.y.z=sin(bot[b].anghel);
        m.z.x=0; m.z.y=-sin(bot[b].anghel); m.z.z=cos(bot[b].anghel);
        calcposarti(bot[b].vion+i,&m);
    }
#   define PLANE_PROPELLER_ROTATION_SPEED (2. * M_PI * 10.) // per seconds
    bot[b].anghel += PLANE_PROPELLER_ROTATION_SPEED * bot[b].thrust * dt_sec;

    // Engine sound
    // TODO: have a second motor voice for another bot but the visubot
    if (b == visubot && bot[b].thrust != soundthrust) {
        soundthrust = bot[b].thrust;
        float const pitch = 1. + 0.1 * (soundthrust - .8);
        attachsound(VOICEMOTOR, soundthrust <= 0.18 ? TAXI : MOTOR,
                pitch, &obj[bot[b].vion].pos, false);   // FIXME: the actual pos of the engine
    }

    // charnières des essieux
    if (bot[b].but.gear) {
        if (bot[b].but.gearup) {
            bot[b].but.gearup=0;
            if (b == visubot) playsound(VOICEGEAR, GEAR_DN, 1, &obj[bot[b].vion].pos, false);   // FIXME: the pos of the gear
            for (j=0; j<(viondesc[bot[b].navion].retract3roues?3:2); j++) obj[bot[b].vion+viondesc[bot[b].navion].roue[j]].aff=1;
        }
#       define GEAR_ROTATION_SPEED (.5 * M_PI / 1.5) // deployed in 1.5 seconds
        bot[b].anggear -= GEAR_ROTATION_SPEED * dt_sec;
        if (bot[b].anggear<0) bot[b].anggear=0;
    } else if (!bot[b].but.gearup) {
        if (b == visubot && bot[b].anggear<.1) playsound(VOICEGEAR, GEAR_UP, 1., &obj[bot[b].vion].pos, false); // FIXME: the pos of the gear
        bot[b].anggear += GEAR_ROTATION_SPEED * dt_sec;
        if (bot[b].anggear>1.5) {
            bot[b].anggear=1.5; bot[b].but.gearup=1;
            for (j=0; j<(viondesc[bot[b].navion].retract3roues?3:2); j++) obj[bot[b].vion+viondesc[bot[b].navion].roue[j]].aff=0;
        }
    }
    for (j=i; j<viondesc[bot[b].navion].nbcharngearx+i; j++) {
        m.y.y=cos(bot[b].anggear);
        m.y.z=((j-i)&1?-1:1)*sin(bot[b].anggear);
        m.z.y=((j-i)&1?1:-1)*sin(bot[b].anggear);
        m.z.z=cos(bot[b].anggear);
        calcposarti(bot[b].vion+j,&m);
    }
    m.y.x=0; m.y.y=1; m.y.z=0;
    m.z.y=0;
    for (i=j; i<j+viondesc[bot[b].navion].nbcharngeary; i++) {
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
    for (i+=3; i<nobjet[mod[obj[bot[b].vion].model].nobjet].nbpieces; i++) {
        if (obj[bot[b].vion+i].objref!=-1) calcposrigide(bot[b].vion+i);
    }
    // tirs ?
    // the shot frequency is given by the number of canons
    gtime const min_shot_period = SHOT_PERIOD / viondesc[bot[b].navion].nbcanon;
    if (bot[b].but.canon && nbtir<NBMAXTIR && bot[b].bullets>0 && gtime_age(bot[b].last_shot) > min_shot_period) {
        if (++bot[b].alterc>=4) bot[b].alterc=0;
        if (bot[b].alterc<viondesc[bot[b].navion].nbcanon) {
            copyv(&v, &obj[bot[b].vion].rot.x);
            mulv(&v, 44);
            vector const *canon = &obj[ bot[b].vion + viondesc[bot[b].navion].firstcanon + bot[b].alterc ].pos;
            addv(&v, canon);
            if (b == visubot) playsound(VOICESHOT, SHOT, 1+(drand48()-.5)*.08, &v, false);
            else drand48();
            gunner[nbobj-debtir]=b;
            vieshot[nbobj-debtir]=80;
            addobjet(0, &v, &obj[bot[b].vion].rot, -1, 0);
            nbtir++;
            bot[b].bullets--;
            bot[b].last_shot = gtime_last();
        }
    }
    if (bot[b].but.bomb) {
        for (i=bot[b].vion; i<bot[b].vion+nobjet[bot[b].navion].nbpieces && (obj[i].type!=BOMB || obj[i].objref!=bot[b].vion); i++);
        if (i<bot[b].vion+nobjet[bot[b].navion].nbpieces) {
            if (b == visubot) playsound(VOICEGEAR, BIPBIP2, 1.1, &obj[i].pos, false);
            obj[i].objref=-1;
            for (j=0; j<bombidx && bombe[j].o!=-1; j++);
            if (j>=bombidx) bombidx=j+1;
            copyv(&bombe[j].vit,&bot[b].vionvit);
            bombe[j].o=i;
            bombe[j].b=b;
            bot[b].nbomb--;
        }
    }
    if (bot[b].but.repere && bot[b].camp==bot[bmanu].camp) {
        copyv(&repere[repidx],&obj[bot[b].vion].pos);
        if (++repidx>NBREPMAX) repidx=0;
    }
    bot[b].but.bomb=bot[b].but.canon=bot[b].but.commerce=bot[b].but.repere=0;
}

void control_vehic(int v, float dt_sec) {
    vector p,u;
    int o=vehic[v].o1, i;
    matrix m;
    double c,s;
    if (vehic[v].camp==-1) return;
    c=cos(vehic[v].ang0);
    s=sin(vehic[v].ang0);
    m.x.x=c; m.x.y=s; m.x.z=0;
    m.y.x=-s; m.y.y=c; m.y.z=0;
    m.z.x=0; m.z.y=0; m.z.z=1;
    copym(&obj[o].rot,&m);
    if (vehic[v].moteur) {
#       define TANK_SPEED (2. * ONE_METER) // per seconds
        copyv(&p, &obj[o].rot.x);
        mulv(&p, TANK_SPEED * dt_sec);
        addv(&obj[o].pos, &p);
        obj[o].pos.z=z_ground(obj[o].pos.x,obj[o].pos.y, true);
        controlepos(o);
    }
    c=cos(vehic[v].ang1);
    s=sin(vehic[v].ang1);
    m.x.x=c; m.x.y=s; m.x.z=0;
    m.y.x=-s; m.y.y=c; m.y.z=0;
    calcposarti(o+1,&m);
    c=cos(vehic[v].ang2);
    s=sin(vehic[v].ang2);
    m.x.x=c; m.x.y=0; m.x.z=s;
    m.y.x=0; m.y.y=1; m.y.z=0;
    m.z.x=-s;m.z.y=0; m.z.z=c;
    calcposarti(o+2,&m);
    for (i=o+3; i<vehic[v].o2; i++) calcposrigide(i);
    if (vehic[v].tir && nbtir<NBMAXTIR) {
        // TODO: a gun object with a last_shot date?
        copyv(&p,&obj[o+3+vehic[v].ocanon].rot.x);
        mulv(&p,90);
        copyv(&u,&obj[o+3+vehic[v].ocanon].pos);
        addv(&p,&u);
        gunner[nbobj-debtir]=v|(1<<NTANKMARK);
        vieshot[nbobj-debtir]=70;
        addobjet(0, &p, &obj[o+3+vehic[v].ocanon].rot, -1, 0);
        nbtir++;
    }
}

void tiradonf(int z, vector *c, int i) {
    vector p;
    matrix m;
    float s;
    gtime const min_shot_period = SHOT_PERIOD / 6; // 6 cannons per Zeppelin
    if (nbtir < NBMAXTIR && gtime_age(zep[z].last_shot) > min_shot_period) {
        zep[z].last_shot = gtime_last();
        copyv(&p,c);
        renorme(&p);
        s=scalaire(&p,&obj[zep[z].o].rot.y);
        if (i<3) s=-s;
        if (s<.5) return;
        copyv(&m.x,&p);
        m.y.x=m.y.y=m.y.z=1;
        orthov(&m.y,&m.x);
        renorme(&m.y);
        prodvect(&m.x,&m.y,&m.z);
        mulv(&p,40);
        addv(&p,&obj[zep[z].o+5+i].pos);
        gunner[nbobj-debtir]=-1;    // passe inapercu (ie pas pris pour cible en retours)
        vieshot[nbobj-debtir]=90;
        addobjet(0, &p, &m, -1, 0);
        nbtir++;
    }
}

void control_zep(int z, float dt_sec) { // fait office de routine robot sur les commandes aussi
    int i;
    vector v;
    matrix m;
    float dir;
    float cx,cy,cz,sx,sy,sz, zs, angcap=0, angprof=0;
    static float phazx=0, phazy=0, phazz=0, balot=0;
    balot += .04/NBZEPS;    // since this function will be called for each zep. how lame!
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
            zep[z].nav.x=(WMAP<<NECHELLE)*drand48()*(SpaceInvaders?.1:.7);
            zep[z].nav.y=(WMAP<<NECHELLE)*drand48()*(SpaceInvaders?.1:.7);
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
    controlepos(zep[z].o);

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
            if (norme2(&v)<ECHELLE*ECHELLE*0.5) zep[z].cib[i]=nc;
        }
    }

    // Shots
    for (i=5; i<5+6; i++) {
        if (zep[z].cib[i-5]!=-1) {
            subv3(&obj[bot[zep[z].cib[i-5]].vion].pos,&obj[zep[z].o+i].pos,&v);
            if (norme2(&v)>ECHELLE*ECHELLE*0.5) zep[z].cib[i-5]=-1;
            else if ((i+imgcount)&1) tiradonf(z,&v,i-5);        //dans gunner, laisser -1 ?
        }
    }

    // Everything else
    for (i=5; i<nobjet[mod[obj[zep[z].o].model].nobjet].nbpieces; i++) calcposrigide(zep[z].o+i);
}

