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
#include "map.h"
#include "robot.h"

char *tankname="Rug-Warrior";
int bosse(int a) {
    if (a) return abs((int)map[a].z-map[a+1].z)+abs((int)map[a].z-map[a+WMAP].z)+abs((int)map[a].z-map[a+1+WMAP].z);
    else return MAXINT;
}
void randomhm(matrix *m) {
    float a=drand48()*M_PI*2;
    copym(m,&mat_id);
    m->x.x=cos(a);
    m->x.y=sin(a);
    m->y.x=-m->x.y;
    m->y.y=m->x.x;
}
void posem(matrix *m, vector *p) {  // tourne légèrement la matrice pour la "poser" sur le sol
    m->x.z=z_ground(p->x+m->x.x,p->y+m->x.y, true)-z_ground(p->x,p->y, true);
    renorme(&m->x);
    m->y.x=-m->x.y;
    m->y.y=m->x.x;
    m->y.z=z_ground(p->x+m->y.x,p->y+m->y.y, true)-z_ground(p->x,p->y, true);
    renorme(&m->y);
    prodvect(&m->x,&m->y,&m->z);
}
int randomvroute(vector *v) {
    int i;
    do i=2+drand48()*(routeidx-4); while(route[i+1].ak==-1 || route[i].ak==-1);
    v->x=route[i].i.x;
    v->y=route[i].i.y;
    v->z=route[i].i.z;
    return i;
}
int bossep(vector *p) { return bosse(akpos(p)); }
void addbabase(int c) {
    int x,y;
    int x1[4] = { WMAP/5., 3.*WMAP/5., WMAP/5., 3.*WMAP/5. };
    int y1[4] = { WMAP/5., WMAP/5., 3.*WMAP/5., 3.*WMAP/5. };
    int a[3] = { 0, 0, 0 };
    matrix m;
    vector p,pp;
    for (y=y1[c]; y<y1[c]+3.*WMAP/10.; y+=5)
        for (x=x1[c]; x<x1[c]+3.*WMAP/10.; x+=5) {
            int aa=x+(y<<NWMAP);
            if (map[aa].z>50 && map[aa].z<120) {
                if (!a[0] || bosse(aa)<bosse(a[0])) {
                    a[2]=a[1]; a[1]=a[0]; a[0]=aa;
                } else if (!a[1] || bosse(aa)<bosse(a[1])) {
                    a[2]=a[1]; a[1]=aa;
                } else if (!a[2] || bosse(aa)<bosse(a[2])) a[2]=aa;
            }
        }
    for (x=0; x<3; x++) {
        if (a[x] == 0) {    // use default location if nothing better was found
            a[x] = x1[x] + (y1[x]<NWMAP);
        }
        int yb=a[x]>>NWMAP;
        int xb=a[x]-(yb<<NWMAP);
        p.x=(xb-(WMAP>>1))*ECHELLE+ECHELLE/2;
        p.y=(yb-(WMAP>>1))*ECHELLE+ECHELLE/2;
        p.z=0;
        // Force the heighmap to be flat around airfields
        map[a[x]-1].submap=0;
        map[a[x]].submap=0;
        map[a[x]+1].submap=0;
        map[a[x]+2].submap=0;
        map[a[x]+3].submap=1;
        map[a[x]-1].submap=1;
        map[a[x]-WMAP].submap=0;
        map[a[x]+WMAP].submap=0;
        map[a[x]+WMAP+1].submap=0;
        map[a[x]-WMAP+1].submap=0;
        map[a[x]+WMAP-1].submap=0;
        map[a[x]-WMAP-1].submap=0;
        map[a[x]+WMAP+2].submap=1;
        map[a[x]-WMAP+2].submap=1;
        map[a[x]-1].z=map[a[x]].z;
        map[a[x]+1].z=map[a[x]].z;
        map[a[x]+2].z=map[a[x]].z;
        map[a[x]-WMAP].z=map[a[x]].z;
        map[a[x]-WMAP-1].z=map[a[x]].z;
        map[a[x]-WMAP+1].z=map[a[x]].z;
        map[a[x]-WMAP+2].z=map[a[x]].z;
        map[a[x]+WMAP].z=map[a[x]].z;
        map[a[x]+WMAP-1].z=map[a[x]].z;
        map[a[x]+WMAP+1].z=map[a[x]].z;
        map[a[x]+WMAP+2].z=map[a[x]].z;
    }
    for (x=0; x<3; x++) {
        int yb=a[x]>>NWMAP;
        int xb=a[x]-(yb<<NWMAP);
        p.x=(xb-(WMAP>>1))*ECHELLE+ECHELLE/2;
        p.y=(yb-(WMAP>>1))*ECHELLE+ECHELLE/2;
        p.z=0;
        babaseo[0][x][c]=addnobjet(NBNAVIONS, &p, &mat_id, 1);  // la piste

        copyv(&pp,&p);
        pp.x+=(drand48()-.5)*ECHELLE*.4;
        pp.y+=(drand48()+.5)*ECHELLE*.2;
        pp.z=85;
        randomhm(&m);
        addnobjet(NBNAVIONS+1, &pp, &m, 1); // une tour de controle
        for (y=0; y<3; y++) {
            pp.x+=(drand48()-.5)*ECHELLE*.2;
            pp.y+=(drand48()-.5)*ECHELLE*.2;
            pp.z=15;
            randomhm(&m);
            addnobjet(NBNAVIONS+NBBASES+NBMAISONS+drand48()*3, &pp, &m, 1); // des vehic
        }
        babaseo[1][x][c]=nbobj;
        for (y=0; y<NBNAVIONS; y++) {
            matrix mp = {
                { cos(.3), 0, sin(.3) },
                { 0,1,0 },
                { -sin(.3),0,cos(.3) }
            };
            copyv(&pp,&p);
            pp.x-=600;
            pp.y+=(y-NBNAVIONS/2)*200;
            pp.z=20;
            addnobjet(y, &pp, y!=2?&mp:&mat_id, 1);
        }
    //  printf("Add Babase #%d, C#%d, at %f,%f\n",x,c,p.x,p.y);
    }
}
void randomvferme(vector *p) {
    int c,i,ok;
    do {
        ok=1;
        randomv(p);
        mulv(p,ECHELLE*(WMAP-WMAP/4));
        p->z=z_ground(p->x,p->y, true);
        for (c=0; c<4; c++) for (i=0; i<3; i++) {
            vector pp;
            copyv(&pp,&obj[babaseo[0][i][c]].pos);
            subv(&pp,p);
            if (norme(&pp)<ECHELLE*10) ok=0;
        }
    } while (map[akpos(p)].z<80 || map[akpos(p)].z>150 || !ok);
}
int collisionpoint(vector *pp, int k, int mo) {
    vector u;
    subv3(pp,&obj[k].pos,&u);
    return norme(&u)<=mod[mo].rayoncollision+mod[obj[k].model].rayoncollision;
}
void affjauge(float j) {
    static float jauge=0;
    float nj=jauge+j;
    static int x=10;
    int nx,y,xx;
    nx=10+(int)(nj*(SX-20.));
    if (nx>x) {
        for (y=_DY-(SY>>3); y<_DY+(SY>>3); y++)
            for (xx=x; xx<nx; xx++)
                //*(int*)&videobuffer[y*SX+xx]=0x3060A0;
                MMXAddSatC((int*)&videobuffer[y*SX+xx],0x001080);
        buffer2video();
        x=nx;
    }
    jauge=nj;
}

void initworld() {
    int i,j,k; vector p; matrix m;
    bombe=(bombe_s*)malloc(NBBOT*4*sizeof(bombe_s));    // FIXME: most bots have 2 or 4 bombs, but spitflame has 7!
    vehic=(vehic_s*)malloc(NBTANKBOTS*sizeof(vehic_s));
    initmap();
    obj=(objet*)malloc(sizeof(objet)*50000);
    // caméra
    obj[0].objref=-1;
    obj[0].type=CAMERA;
    obj[0].distance=-1;
    obj[0].prec=-1; obj[0].next=-1;
    map[0].first_obj = 0;
    obj[0].ak=0;
    nbobj=1;
    // babases
    printf("Adding airfields...\n");
    for (j=0; j<4; j++) addbabase(j);
    // zeppelins
    if ((zep=malloc(sizeof(zep_s)*NBZEPS))==NULL) {
        perror("malloc zep"); exit(-1);
    }
    for (i=0; i<NBZEPS; i++) {
        p.x=(drand48()-.5)*(WMAP<<NECHELLE)*0.8;
        p.y=(drand48()-.5)*(WMAP<<NECHELLE)*0.8;
        p.z=4000+z_ground(p.x,p.y, false);
        zep[i].o=addnobjet(NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+NBDECOS,&p, &mat_id, 0);
        zep[i].nav = p;
        zep[i].angx=zep[i].angy=zep[i].angz=zep[i].anghel=zep[i].vit=0;
        for (j=0; j<6; j++) zep[i].cib[j]=-1;
        zep[i].last_shot = 0;
    }
    // installations au sol
    // des villages de pauvres inocents pour souffrir
    for (i=0; i<NBVILLAGES; i++) {
        int nbm=drand48()*20+5;
        int ok,k;
        vector pp;
        int a;
        do {
            int bos = MAXINT;
            for (j=0; j<5; j++) {   // choose the less mountainous location out of 5 tries
                int b;
                randomvferme(&pp);
                if (map[akpos(&pp)].z < 160 && (b=bossep(&pp))<bos) {
                    bos=b;
                    copyv(&p,&pp);
                }
            }
            for (j=0; j<i; j++)
                if (fabs(village[j].p.x-p.x)+fabs(village[j].p.y-p.y)<ECHELLE*5) break;
        } while (j<i);
        // Make heighmap more flat around villages
        map[a=akpos(&p)].submap=0;
        map[a-1].submap=0;
        map[a+1].submap=0;
        map[a+WMAP].submap=0;
        map[a-WMAP].submap=0;
        map[a+WMAP+1].submap=0;
        map[a-WMAP+1].submap=0;
        map[a+WMAP-1].submap=0;
        map[a-WMAP-1].submap=0;
        p.z=z_ground(p.x,p.y, true);
        copyv(&village[i].p,&p);
        village[i].o1=nbobj;
        randomv(&pp);   // une église
        mulv(&pp,100);
        addv(&pp,&p);
        randomhm(&m);
        posem(&m,&pp);
        pp.z=40;
        addnobjet(NBNAVIONS+NBBASES+1, &pp, &mat_id, 1);
        for (j=0; j<nbm; j++) { // des maisons
            matrix m;
            randomhm(&m);
            do {
                ok=1;
                randomv(&pp);
                mulv(&pp,ECHELLE/2);
                addv(&pp,&p);
                posem(&m,&pp);
                pp.z=48;
                for (k=village[i].o1; k<nbobj; k++) if (collisionpoint(&pp,k,NBNAVIONS+NBBASES+0)) {ok=0; break;}
            } while (!ok);
            addnobjet(NBNAVIONS+NBBASES+0, &pp, &m, 1);
        }
        village[i].o2=nbobj;
        village[i].nom=nomvillage[i];
        for (j=0; j<nbm>>1; j++) {  // des reverberes
            matrix m;
            randomhm(&m);
            do {
                ok=1;
                randomv(&pp);
                mulv(&pp,ECHELLE/2);
                addv(&pp,&p);
                posem(&m,&pp);
                pp.z=34;
                for (k=village[i].o1; k<nbobj; k++) if (collisionpoint(&pp,k,NBNAVIONS+NBBASES+5)) {ok=0; break;}
            } while (!ok);
            addnobjet(NBNAVIONS+NBBASES+5, &pp, &m, 1);
        }
    }
    // routes
    initroute();
    printf("Adding motorways...\n");
    for (i=0; i<NBVILLAGES-1; i++) for (j=i+1; j<NBVILLAGES; j++) {
        int k;
        float cp;
        vector v;
        copyv(&v,&village[i].p);
        subv(&v,&village[j].p);
        if (norme(&v)<ECHELLE*5) continue;
        if (v.x==0 && v.y==0) continue;
        cp=cap(v.x,v.y);
        for (k=i+1; k<j; k++) {
            float dc;
            copyv(&v,&village[i].p);
            subv(&v,&village[k].p);
            if (norme(&v)<ECHELLE*5) continue;
            if (v.x==0 && v.y==0) continue;
            dc=cap(v.x,v.y)-cp;
            if (dc<-M_PI) dc+=2*M_PI;
            else if (dc>M_PI) dc-=2*M_PI;
            if (fabs(dc)<M_PI/4) break;
        }
        if (k==j) {
            prospectroute(&village[i].p,&village[j].p);
        }
        affjauge(.75/(1.5*((NBVILLAGES+1)*NBVILLAGES)));
    }
    EndMotorways=routeidx;
    printf("Adding roads around cities...\n");
    for (i=0; i<NBVILLAGES; i++) {
        int nbr=drand48()*5+5;  // prop à la taille de la ville
        int r;
        for (r=0; r<nbr; r++) {
            vector dest;
            randomv(&dest);
            mulv(&dest,ECHELLE*(5+10*drand48()));   // prop à la taille de la ville
            addv(&dest,&village[i].p);
            dest.z=z_ground(dest.x,dest.y, false);
            prospectroute(&village[i].p,&dest);
            affjauge(.75/(3.*7.5*NBVILLAGES));
        }
    }
    EndRoads=routeidx;
    printf("Adding footpaths...\n");
    for (i=0; i<150; i++) {
        int ri=EndMotorways+drand48()*(routeidx-EndMotorways-1);
        vector dest,v;
        if (route[ri].ak!=-1 && route[ri+1].ak!=-1) {
            copyv(&v,&route[ri+1].i);
            subv(&v,&route[ri].i);
            dest.x=v.y;
            dest.y=-v.x;
            dest.z=0;
            if (drand48()>.5) mulv(&dest,-1);
            mulv(&dest,(2+drand48()*3));
            addv(&dest,&route[ri].i);
            if (fabs(dest.x)<((WMAP-5)<<(NECHELLE-1)) && fabs(dest.y)<((WMAP-5)<<(NECHELLE-1))) {
                dest.z=z_ground(dest.x,dest.y, false);
                prospectroute(&route[ri].i,&dest);
            }
        }
        affjauge(.75/(3.*150.));
    }
/*  {
        vector u,v;
        u.x=-10*ECHELLE+2345; u.y=10*ECHELLE+1234; u.z=z_ground(u.x,u.y, true);
        v.x=10*ECHELLE-2345; v.y=10*ECHELLE+1234; v.z=z_ground(v.x,v.y, true);
        traceroute(&u,&v);
    }*/
    endinitroute();
    hashroute();
    printf("Adding villages...\n");
    // des fermes et des usines
    for (i=0; i<(NBVILLAGES*10); i++) {
        vector pp;
        int ri;
        matrix m;
        do ri=EndRoads+drand48()*(routeidx-EndRoads-1);
        while (route[ri].ak!=-1 && route[ri+1].ak!=-1);
        copyv(&p,&route[ri].i);
        randomhm(&m);
        randomv(&pp);
        mulv(&pp,.5*ECHELLE);
        addv(&pp,&p);
        posem(&m,&pp);
        if (i&1) {
            pp.z=10;
            addnobjet(NBNAVIONS+NBBASES+2, &pp, &m, 1);
        } else {
            pp.z=20;
            addnobjet(NBNAVIONS+NBBASES+3, &pp, &m, 1);
        }
    }
    printf("Adding farms...\n");
    // des maisons au bord des routes
    for (i=0; i<200; i++) {
        vector pp;
        int ri;
        matrix m;
        do ri=EndRoads+drand48()*(routeidx-EndRoads-1);
        while (route[ri].ak!=-1 && route[ri+1].ak!=-1);
        copyv(&p,&route[ri].i);
        randomhm(&m);
        randomv(&pp);
        mulv(&pp,.3*ECHELLE);
        addv(&pp,&p);
        posem(&m,&pp);
        pp.z=48;
        addnobjet(NBNAVIONS+NBBASES+0, &pp, &m, 1);
    }
    printf("Adding mills...\n");
    // des moulins
    DebMoulins=nbobj;
    for (i=0; i<NBVILLAGES*2; i++) {
        vector pp;
        int ri;
        matrix m;
        do ri=EndRoads+drand48()*(routeidx-EndRoads-1);
        while (route[ri].ak!=-1 && route[ri+1].ak!=-1);
        copyv(&p,&route[ri].i);
        randomhm(&m);
        randomv(&pp);
        mulv(&pp,3*ECHELLE);
        addv(&pp,&p);
        pp.z=30;
        posem(&m,&pp);
        addnobjet(NBNAVIONS+NBBASES+4, &pp, &m, 1);
    }
    FinMoulins=nbobj;
    printf("Adding cows...\n");
    // des troupeaux de charolaises
    for (i=0; i<NBVILLAGES*2; i++) {
        int nbn=drand48()*5+2;
        copyv(&p,&village[i%NBVILLAGES].p);
        for (k=0; k<2; k++) {
            vector pt;
            randomv(&pt);
            mulv(&pt,ECHELLE*6);
            addv(&pt,&p);
            pt.z=0;
            for (j=0; j<nbn; j++) {
                vector pp;
                matrix m;
                randomhm(&m);
                randomv(&pp);
                mulv(&pp,ECHELLE*.5*pow(nbn,.1));
                pp.z=12;
                addv(&pp,&pt);
                posem(&m,&pp);
                addnobjet(NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+5, &pp, &m, 1);
            }
        }
    }
    printf("Adding vehicules...\n");
    // des véhicules en décors
    voiture=(voiture_s*)malloc((NBVOITURES+1)*sizeof(voiture_s));
    for (i=0; i<NBVOITURES/4; i++) {
        voiture[i].r=randomvroute(&p);
        voiture[i].sens=1;
        p.z=15;
        voiture[i].o=addnobjet(NBNAVIONS+NBBASES+NBMAISONS+1, &p, &mat_id, 1);
        voiture[i].dist=-1;
        voiture[i].vit=5+15*drand48();
    }
    for (; i<NBVOITURES/2; i++) {
        voiture[i].r=randomvroute(&p);
        voiture[i].sens=1;
        p.z=15;
        voiture[i].o=addnobjet(NBNAVIONS+NBBASES+NBMAISONS+4, &p, &mat_id, 1);
        voiture[i].dist=-1;
        voiture[i].vit=5+15*drand48();
    }
    for (; i<NBVOITURES*8/10; i++) {
        voiture[i].r=randomvroute(&p);
        voiture[i].sens=1;
        p.z=15;
        voiture[i].o=addnobjet(NBNAVIONS+NBBASES+NBMAISONS+2, &p, &mat_id, 1);
        voiture[i].dist=-1;
        voiture[i].vit=10+10*drand48();
    }
    for (; i<NBVOITURES; i++) {
        voiture[i].r=randomvroute(&p);
        voiture[i].sens=1;
        p.z=30;
        voiture[i].o=addnobjet(NBNAVIONS+NBBASES+NBMAISONS+3, &p, &mat_id, 1);
        voiture[i].dist=-1;
        voiture[i].vit=5+5*drand48();
    }
    voiture[i].o=nbobj;
    printf("Adding tractors...\n");
    // des tracteurs dans les champs
    for (i=0; i<50; i++) {
        matrix m;
        randomhm(&m);
        randomvferme(&p);
        posem(&m,&p);
        p.z=30;
        addnobjet(NBNAVIONS+NBBASES+NBMAISONS+3, &p, &m, 1);
    }
    printf("Adding trees...\n");
    // et des arbres
    for (i=0; i<150; i++) {
        matrix m;
        randomhm(&m);
        randomvferme(&p);
        posem(&m,&p);
        p.z=29;
        addnobjet(NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+3, &p, &m, 1);
    }
/*  for (i=0; i<1; i++) {
        matrix m;
        randomhm(&m);
        randomvferme(&p);
        p.z=20;
        addnobjet(NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+4, &p, &m, 1);
    }*/
    printf("Adding planes...\n");
    // des vionvions
    if ((bot=calloc(sizeof(*bot),NBBOT))==NULL) { perror("bot"); exit(-1); }
    bot[bmanu].camp=camp;
    bot[bmanu].navion=monvion-1;
    //if (NetCamp()==-1) {printf("Net Error\n"); exit(-1); }
    printf("Playing with %d planes & %d tanks\nPlayers :\n",NBBOT,NBTANKBOTS);
    for (i=0; i<NbHosts; i++) printf("%s, camp %d, in a %s\n",playbotname[i],bot[i].camp+1, viondesc[bot[i].navion].name);
    for (i=0; i<NBBOT; i++) {
        int c=i&3, b;
        if (i>=NbHosts) bot[i].camp=c;
        bot[i].babase=b=babaseo[0][(int)(drand48()*3)][(int)bot[i].camp];
        if (! SpaceInvaders) {
            copyv(&p,&obj[b].pos);
            p.y+=i>=NbHosts?10:250;
            p.x+=300*(i>=NbHosts?(i>>2):i);
            p.z=20+z_ground(p.x,p.y, true);
            copym(&m,&mat_id);
            m.x.x=0;
            m.x.y=-1;
            m.y.x=1;
            m.y.y=0;
        } else {
            randomhm(&m);
            p = vec_zero;
            if (bot[i].camp < 2) p.y += 1000; else p.y -= 1000;
            if (bot[i].camp & 1) p.x += 1000; else p.x -= 1000;
            if (i >= NbHosts) {
                p.x *= 2*((i-NbHosts));
                p.y *= 2*((i-NbHosts));
                p.z = 0;
            } else {
                p.x += (drand48()-.5)*400;
                p.y += (drand48()-.5)*400;
                p.z = 100*i;
            }
            p.z += 16000.;
        }
        if (i >= NbHosts) bot[i].navion = drand48()*NBNAVIONS;
        bot[i].vion = addnobjet(bot[i].navion,&p,&m, 0);
        bot[i].but.gear = !SpaceInvaders;
        bot[i].but.canon = 0;
        bot[i].but.bomb = 0;
        bot[i].but.gearup = 0;
        bot[i].but.frein = 0;
        bot[i].but.business = 0;
        bot[i].anghel = 0;
        bot[i].anggear = 0;
        bot[i].xctl = bot[i].yctl = 0;
        bot[i].thrust = SpaceInvaders ? 1.:0.;
        bot[i].maneuver = SpaceInvaders ? NAVIG : PARKING;
        bot[i].aerobatic = MANEUVER;
        bot[i].gunned = -1;
        bot[i].fiulloss = bot[i].bloodloss = bot[i].motorloss = bot[i].aeroloss = 0;
        bot[i].fiul = viondesc[bot[i].navion].fiulmax;
        bot[i].bullets = viondesc[bot[i].navion].bulletsmax;
        bot[i].cibv = bot[i].cibt = -1;
        bot[i].gold = i > NbHosts ? 30000:2000;
        bot[i].is_flying = SpaceInvaders;
        armstate(i);
        if (!SpaceInvaders) {
            bot[i].vionvit = vec_zero;
            bot[i].target_rel_alt = 100. * ONE_METER;
            bot[i].u.x = bot[i].u.y = 0.;
            bot[i].u.z = z_ground(bot[i].u.x, bot[i].u.y, true);
        } else {
            bot[i].vionvit = obj[bot[i].vion].rot.x;
            mulv(&bot[i].vionvit, 2. * ONE_METER);
            newnav(i);
        }
    }
    printf("Adding tanks...\n");
    // des tanks
    for (i=0; i<NBTANKBOTS; i++) {
        if (drand48()<.2) {
            randomvferme(&p);
            if (i&1) p.x*=.1; else p.y*=.1;
        } else {
            p.x+=(drand48()-.5)*100;
            p.y+=(drand48()-.5)*100;
        }
        p.z=z_ground(p.x,p.y, true);
        if (p.y>0) vehic[i].camp=2; else vehic[i].camp=0;
        if (p.x>0) vehic[i].camp++;
        vehic[i].cibv=-1;
        copyv(&vehic[i].p,&p);
        p.z=20;
        vehic[i].o1=addnobjet(NBNAVIONS+NBBASES+NBMAISONS, &p, &mat_id, 1);
        vehic[i].o2=nbobj;
        vehic[i].moteur=0;
        vehic[i].cibt=-1;
        vehic[i].ang0=vehic[i].ang1=vehic[i].ang2=0;
        vehic[i].ocanon=0;
        vehic[i].last_shot = 0;
        vehic[i].nom=tankname;
    }
    printf("Adding clouds...\n");
    // et des nuages
    for (i=0; i<70; i++) {
        int nbn=drand48()*15+10;
        randomv(&p);
        mulv(&p,ECHELLE*(WMAP-5));
        p.z=drand48()*5000+20000;
        for (j=0; j<nbn; j++) {
            vector pp;
            randomv(&pp);
            mulv(&pp,ECHELLE*pow(nbn,.3));
            pp.z/=2.;
            addv(&pp,&p);
            addnobjet(NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS, &pp, &mat_id, 0);
        }
    }
    printf("Adding smoke...\n");
    // et de la fumée
    firstfumee=nbobj;
    for (i=0; i<NBMAXFUMEE; i++) {
        addnobjet(NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+1,&vec_zero,&mat_id,0);
    }
    rayonfumee=(uchar*)calloc(NBMAXFUMEE,sizeof(uchar));
    typefumee=(uchar*)calloc(NBMAXFUMEE,sizeof(uchar));
}
