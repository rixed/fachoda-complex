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
#include "proto.h"
bot_s *bot;
vehic_s *vehic;
voiture_s *voiture;
zep_s *zep;

double tirz(double dz, double d) {
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
double tircoli(vector v, vector u) {    // V est unitaire. Les tirs ont une vitesse CST
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
double bombz2(vector v, vector u) {
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
void newcibvehic(int v) {
    int i, j=0;
    double r;
choiz:
    if (j++>10) vehic[v].cibt=-1;
    else {
        r=drand48();
        if (r<.3) {
            // attaque un village
            int a=NBVILLAGES*drand48();
            i=0;
            do {
                vehic[v].cibt=village[a].o1+(village[a].o2-village[a].o1)*drand48();
                i++;
            } while(i<10 && obj[vehic[v].cibt].type!=CIBGRAT);
            if (i==10) goto choiz;
        } else {
            // attaque un tank
            i=0;
            do {
                vehic[v].cibt=(int)(NBTANKBOTS*drand48());
                i++;
            } while(i<10 && obj[vehic[v].cibt].type!=VEHIC && vehic[vehic[v].cibt].camp==vehic[v].camp);
            if (i==10) goto choiz;
            else vehic[v].cibt=vehic[vehic[v].cibt].o1;
        }
    }
}
void robotvehic(int v) {
    vector p,u;
    int cib,vol;
    double xx,yy,n;
    if (vehic[v].camp==-1) return;
    if (vehic[v].cibv==-1) {
        cib=drand48()*NBBOT;
        if (bot[cib].camp!=-1 && bot[cib].camp!=vehic[v].camp) {
            subv3(&obj[bot[cib].vion].pos,&obj[vehic[v].o1].pos,&p);
            if (norme2(&p)<5000000) vehic[v].cibv=bot[cib].vion;
        }
    }
    vehic[v].tir=0;
    if (vehic[v].cibt==-1 || obj[vehic[v].cibt].type==DECO) newcibvehic(v);
    if (vehic[v].cibv!=-1 && obj[vehic[v].cibv].type!=DECO) {
        cib=vehic[v].cibv;
        vol=1;
    } else {
        cib=vehic[v].cibt;
        vehic[v].cibv=-1;
        vol=0;
    }
    if (cib!=-1) {
        subv3(&obj[cib].pos,&obj[vehic[v].o1].pos,&p);
        n=renorme(&p);
        if (n<4000) {
            if (n<400) vehic[v].moteur=0;
            yy=scalaire(&p,&obj[vehic[v].o1+1].rot.y);
            xx=scalaire(&p,&obj[vehic[v].o1+1].rot.x);
            if (xx<0) vehic[v].ang1+=.4;
            else {
                vehic[v].ang1+=.4*yy;
                if (yy>-.2 && yy<.2) {
                    double tz;
                    if (++vehic[v].ocanon>=4) vehic[v].ocanon=0;
                    subv3(&obj[cib].pos,&obj[vehic[v].o1+3+vehic[v].ocanon].pos,&u);
                    if ((tz=u.z-tirz(obj[vehic[v].o1+2].rot.x.z,sqrt(u.x*u.x+u.y*u.y)))>0) vehic[v].ang2+=tz<100?.001*tz:.1;
                    else if (tz<0) vehic[v].ang2+=tz>-100?.001*tz:-.1;
                    if (tz>-100 && tz<100 && n<2500 && imgcount&1) vehic[v].tir=1;
                }
            }
        } else {
            if (vol) vehic[v].cibv=-1;
            vehic[v].moteur=1;
            yy=scalaire(&p,&obj[vehic[v].o1].rot.y);
            xx=scalaire(&p,&obj[vehic[v].o1].rot.x);
            if (xx<0) vehic[v].ang0+=.01;
            else if (yy>0) vehic[v].ang0+=.01;
            else vehic[v].ang0-=.01;
        }
    }
}

void armstate(int b) {
    int i;
    bot[b].nbomb=0;
    for (i=bot[b].vion; i<bot[b].vion+nobjet[bot[b].navion].nbpieces; i++) {
        if (obj[i].objref==bot[b].vion) {
            if (obj[i].type==BOMB) bot[b].nbomb++;
        }
    }
}
void newnav(int b) {
    if (SpaceInvaders) {
        bot[b].u.x=bot[b].u.y=0;
        bot[b].u.z=16000;
        bot[b].vc=20;
        bot[b].manoeuvre=4;
        return;
    }
    if (bot[b].cibt!=-1 && bot[b].nbomb && bot[b].fiul>viondesc[bot[b].navion].fiulmax/2) {
        if (bot[b].manoeuvre!=6) {
            copyv(&bot[b].u,&obj[bot[b].cibt].pos);
            bot[b].vc=35;
            bot[b].manoeuvre=4;
        } else {
            copyv(&bot[b].u,&obj[bot[b].vion].rot.x);
            mulv(&bot[b].u,25000);
            bot[b].u.z=1000;
            addv(&bot[b].u,&obj[bot[b].vion].pos);
            bot[b].vc=50;   // on s'éloigne fissa!
            bot[b].manoeuvre=10;
        }
        bot[b].u.z+=3000;
    } else {
        if (bot[b].manoeuvre!=7 && bot[b].manoeuvre!=8 && bot[b].manoeuvre!=9) {
            copyv(&bot[b].u,&obj[bot[b].babase].rot.x);
            mulv(&bot[b].u,6000);
            addv(&bot[b].u,&obj[bot[b].babase].pos);
            bot[b].u.z=1000+obj[bot[b].vion].pos.z-bot[b].zs;
            bot[b].manoeuvre=7;
            bot[b].vc=30;
            bot[b].cibt=-1;
        } else if (bot[b].manoeuvre==7) {
            copyv(&bot[b].u,&obj[bot[b].babase].rot.x);
            mulv(&bot[b].u,1500);
            addv(&bot[b].u,&obj[bot[b].babase].pos);
            bot[b].u.z=obj[bot[b].vion].pos.z-bot[b].zs+300;
            bot[b].manoeuvre=8;
            bot[b].vc=16;
        } else if (bot[b].manoeuvre==8) {
            copyv(&bot[b].u,&obj[bot[b].babase].pos);
            bot[b].but.gear=1;
            bot[b].manoeuvre=9;
            bot[b].vc=9;
        }
    }
}
void newcib(int b) {
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

double cap(double x, double y) {    // Returns cap between 0 and 2*M_PI
    double const d2 = y*y + x*x;
    if (d2 == 0.) return 0;
    double a = acos(x / sqrt(d2));
    if (y<0) a = 2*M_PI - a;
    return a;
}

void robot(int b){
    float vit,m,n,a,dist,disth;
    float distfrap2, dc;
    vector u,v,c;
    int o=bot[b].vion;
    if (bot[b].camp==-1) return;
//  printf("bot %d man %d ",b,bot[b].manoeuvre);
    vit=norme(&bot[b].vionvit);
#define zs bot[b].zs
    if (bot[b].gunned!=-1) {    // il reviendra a chaque fois en cas de voltige...
        if (!(bot[b].gunned&(1<<NTANKMARK))) {  // si c'est un bot qui l'a touché
            if (bot[b].cibv!=bot[bot[b].gunned].vion && bot[bot[b].gunned].camp!=bot[b].camp) {
                bot[b].voltige=1;   // virage serré
                bot[b].cibv=bot[bot[b].gunned].vion;
            }
        } else {
            bot[b].cibt=vehic[bot[b].gunned&((1<<NTANKMARK)-1)].o1;
            bot[b].manoeuvre=4; bot[b].voltige=0;   // il va voir sa mere lui !
            bot[b].gunned=-1;
        }
    } else {
        if (bot[b].cibv==-1) {
            int cib=drand48()*NBBOT;
            if (bot[b].bullets>100 && bot[b].fiul>70000 && bot[cib].camp!=-1 && bot[cib].camp!=bot[b].camp) {
                subv3(&obj[bot[cib].vion].pos,&obj[bot[b].vion].pos,&u);
                if (norme2(&u)<ECHELLE*ECHELLE*10) {
                    bot[b].cibv=bot[cib].vion;
                    bot[b].voltige=4;
                    bot[b].gunned=cib;
                }
            }
        }
    }
    if (bot[b].voltige && obj[bot[b].cibv].type==DECO) {
        bot[b].voltige=0;
        bot[b].cibv=-1;
        bot[b].manoeuvre=4;
        bot[b].gunned=-1;
    }
    switch (bot[b].voltige) {
    case 0:
        switch (bot[b].manoeuvre) {
        case 0: // parking
            copyv(&bot[b].u,&obj[bot[b].babase].pos);
            copyv(&bot[b].v,&obj[bot[b].babase].rot.x);
            bot[b].but.gear=1;
            newcib(b);
            if (bot[b].cibt!=-1) bot[b].manoeuvre=1;
            break;
        case 1: // taxi
            subv3(&bot[b].u,&obj[o].pos,&u);
            n=norme(&u);
            if (n>300) m=2; else m=1;
            if (vit<m) bot[b].thrust+=.01;
            else if (vit>m) bot[b].thrust-=.01;
            if (n>80) {
                bot[b].xctl=-2*scalaire(&u,&obj[o].rot.y)/n;
                if (scalaire(&u,&obj[o].rot.x)<0) {
                    if (bot[b].xctl>0) bot[b].xctl=1;
                    else bot[b].xctl=-1;
                }
            } else {
                bot[b].thrust=0;    // pour reloader les bombes
                bot[b].but.frein=1;
                if (vit<.1) bot[b].manoeuvre=2;
            }
//          printf("n %.0f xc %.3f\n",n,bot[b].xctl);
            break;
        case 2: // alignement
            bot[b].thrust=0.01;
            bot[b].but.frein=0;
            bot[b].xctl=-4*scalaire(&bot[b].v,&obj[o].rot.y);
            if (scalaire(&bot[b].v,&obj[o].rot.x)<0) {
                if (bot[b].xctl>0) bot[b].xctl=1;
                else bot[b].xctl=-1;
            }
            if (fabs(bot[b].xctl)<.02) bot[b].manoeuvre=3;
//          printf("xc %.3f\n",bot[b].xctl);
            break;
        case 3: // décollage
            bot[b].thrust=1;
            bot[b].but.flap=1;
            bot[b].xctl=0;
            if (vit>10) bot[b].xctl=.01;    // petite gruge
            if (vit>14) { bot[b].xctl=0; bot[b].yctl=-.15; }
            if (vit>15) { bot[b].yctl=.3; bot[b].xctl=-obj[o].rot.y.z; }
            if (zs>100) {
                bot[b].but.gear=0;
                armstate(b);
            }
            if (zs>1100) {
                bot[b].but.flap=0;
                newnav(b);
                bot[b].but.flap=0;
            }
//          printf("vit %.1f alt %.1f yc %.3f\n",vit,obj[o].pos.z,bot[b].yctl);
            break;
        case 4: // NAV
        case 10:    // escape from a mefait
        case 7: // ILS
        case 8: // ILS
        case 9:
        case 11:    // vol rasant et...
        case 12:    // ...et bombardement !
            if ((bot[b].manoeuvre==4 || bot[b].manoeuvre==7 || bot[b].manoeuvre==10) && bot[b].vitlin>19) bot[b].but.flap=0;
            else bot[b].but.flap=1;
            if (bot[b].cibt!=-1 && bot[b].manoeuvre!=10) {  // recopie les coords des fois que la cible bouge
                bot[b].u.x=obj[bot[b].cibt].pos.x;
                bot[b].u.y=obj[bot[b].cibt].pos.y;
            }
            copyv(&v,&bot[b].u);        // distance au navpoint
            subv(&v,&obj[o].pos);
            n=renorme(&v);
            dc=cap(v.x,v.y)-bot[b].cap;
            if (dc<-M_PI) dc+=2*M_PI;
            else if (dc>M_PI) dc-=2*M_PI;
            if (bot[b].manoeuvre==11 && n<4000) {
                bot[b].manoeuvre=12;
                bot[b].vc=18;
                bot[b].u.z=obj[bot[b].cibt].pos.z+1100;
                bot[b].df=MAXDOUBLE;
            }
            if (bot[b].manoeuvre==12) {
                vector c;
                if (obj[bot[b].cibt].type==DECO) {
                    newcib(b);
                    newnav(b);
                    break;
                } else {
                    copyv(&c,&obj[bot[b].cibt].pos);
                    subv(&c,&obj[bot[b].vion].pos);
                    if ((distfrap2=bombz2(bot[b].vionvit,c))<7000) {    // diminuer n'augmente pas la precision !
                        bot[b].but.bomb=1;
                        bot[b].manoeuvre=6;
                    }
                    if (distfrap2<bot[b].df) bot[b].df=distfrap2;
                    if (dc<-M_PI/2 || dc>M_PI/2) bot[b].manoeuvre=6;
                }
            }
            if (bot[b].manoeuvre==10 && n<3000) { newnav(b); break; }
            if (bot[b].manoeuvre==4 && bot[b].cibt!=-1) {
                if (n<40000) {
                    bot[b].u.z=obj[bot[b].cibt].pos.z+1500;
                    bot[b].vc=35;
                    bot[b].manoeuvre=11;
        //          if (visubot==b) printf("razant\n");
                    break;
                } else if (n<7000) {
                    bot[b].u.z=obj[bot[b].cibt].pos.z+800;
            //      if (visubot==b) printf("Basse altitude!\n");
                }
            } else if (bot[b].manoeuvre>=4 && bot[b].manoeuvre<9 && n<2000 && !SpaceInvaders){
                newnav(b);
                break;
            } else if (bot[b].manoeuvre==9) {
                if (zs<30) {
                    bot[b].vc=0;
                }
                if (bot[b].vitlin<15) {
                    bot[b].but.frein=1;
                }
                if (bot[b].vitlin<1) {
                //  printf("Bot#%d de retours à la babase\n",b);
                    bot[b].manoeuvre=0;
                    break;
                }
            }
        //  m=obj[o].rot.y.x*v.x+obj[o].rot.y.y*v.y;
        //  n=obj[o].rot.x.x*v.x+obj[o].rot.x.y*v.y;
        //  if (n>0 || m>.1) a=-.5*atan(10*m*(n<0?1000:1));
        //  else a=.9;
            if (dc>.5) a=-.6;
            else if (dc<-.5) a=.6;
            else a=-.6*dc/.5;
            bot[b].xctl=(a-obj[o].rot.y.z);
            if (bot[b].vitlin<bot[b].vc) bot[b].thrust+=.01;
            else if (bot[b].vitlin>bot[b].vc && bot[b].thrust>.02) bot[b].thrust-=.01;
            m=bot[b].u.z-obj[o].pos.z;  // DZ
            if (zs<6000 && (bot[b].manoeuvre==10 || bot[b].manoeuvre==4)) {
                if (zs>1000) m+=12000-2*zs; else { m+=20000-5*zs; bot[b].thrust=1; };
            } else if (zs<1000 && (bot[b].manoeuvre==11 || bot[b].manoeuvre==12)) m+=20000-20*zs;
            else if (zs<1200 && bot[b].manoeuvre==7) m+=24000-20*zs;
            else if (zs<500 && bot[b].manoeuvre==8) m+=10000-20*zs;
            if ((bot [b].manoeuvre==4 || bot[b].manoeuvre==7) && n>20000) m+=5000;
            m=7*atan(1e-3*m);       // rot.x.z souhaitable
            bot[b].yctl=(m-bot[b].vionvit.z)*.27;
            if (bot[b].vitlin<15 && zs>300) {
                if (bot[b].yctl>1) bot[b].yctl=1;
                bot[b].yctl/=(16-bot[b].vitlin);
            }
//          printf("dz %.0f Vzs %.1f yc %.3f\n",n,m,bot[b].yctl);
            break;
        case 5: // piqué & bomb
            // on vérifie que la cible n'est pas détruite
            if (obj[bot[b].cibt].type!=DECO) {
                double distfrap2;
                copyv(&v,&bot[b].u);
                subv(&v,&obj[o].pos);
                if ((distfrap2=bombz2(bot[b].vionvit,v))<1000) {
                    bot[b].but.bomb=1;
                    bot[b].manoeuvre=6;
                }
                renorme(&v);
                bot[b].thrust=0;
                m=obj[o].rot.y.x*v.x+obj[o].rot.y.y*v.y;
                n=obj[o].rot.x.x*v.x+obj[o].rot.x.y*v.y;
                if (n>0 || m>.1) a=-.5*atan(1*m*(n<0?1000:1)); else a=.5;
                bot[b].xctl=(a-obj[o].rot.y.z);
                bot[b].yctl=(v.z-obj[o].rot.x.z-.14);
                if (zs<200) bot[b].manoeuvre=6;
            } else {    // changer de cible
                newcib(b);
                newnav(b);
            }
            break;
        case 6: // redresse
            bot[b].thrust=1;
            bot[b].but.flap=1;
            bot[b].xctl=(-obj[o].rot.y.z);
            bot[b].yctl=1;
            if (bot[b].vionvit.z>.5) newnav(b);
            break;
        }
        break;
    case 1: // VIRAGE SERRE
        if (obj[o].rot.y.z<0) bot[b].xctl=-1-obj[o].rot.y.z;
        else bot[b].xctl=1-obj[o].rot.y.z;
        if (obj[o].rot.z.z<0) bot[b].xctl=-bot[b].xctl;
        if (vit<13) bot[b].thrust+=.1;
        else if (vit>13) bot[b].thrust-=.01;
        bot[b].yctl=1-bot[b].xctl*bot[b].xctl;
        if (zs<400) bot[b].voltige=0;
        if (scalaire(&obj[o].rot.x,&obj[bot[b].cibv].rot.x)<0) bot[b].voltige=2;
        break;
    case 2: // RETABLIRE
        if (vit<18) bot[b].thrust+=.1;
        else if (vit>18) bot[b].thrust-=.01;
        bot[b].xctl=(-obj[o].rot.y.z);
        bot[b].yctl=(-bot[b].vionvit.z)*.13;
        if (obj[o].rot.z.z<0) bot[b].yctl=-bot[b].yctl;
        if (obj[o].rot.z.z>.8) bot[b].voltige=4;
        break;
    case 3: // CHANDELLE
        bot[b].thrust=1;
        bot[b].xctl=(-obj[o].rot.y.z);
        bot[b].yctl=(1-obj[o].rot.x.z)*.13;
        if (bot[b].vionvit.z<0 && obj[o].rot.x.z>.5) bot[b].voltige=2;
        if (zs<400) bot[b].voltige=0;
        break;
    case 4: // CHERCHE 6 HEURES
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
            if (a>-6*m && a<6*m && dc>-M_PI/2 && dc<M_PI/2) {//dc>-.07 && dc<.07) {
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
        if (zs<3000) bot[b].voltige=5;
        break;
    case 5: // REPRENDRE DE L'ALTITUDE
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
        if (zs>3100) bot[b].voltige=4;
        break;
    }
}
