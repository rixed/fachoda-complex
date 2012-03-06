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
#include "heightfield.h"
#include "robot.h"
#include "video_sdl.h"

char *tankname="Rug-Warrior";

static int bosse(int a)
{
    if (! a) return MAXINT;
    return abs((int)map[a].z-map[a+1].z)+abs((int)map[a].z-map[a+MAP_LEN].z)+abs((int)map[a].z-map[a+1+MAP_LEN].z);
}

static int bossep(struct vector *p)
{
    return bosse(akpos(p));
}

void randomhm(struct matrix *m)
{
    float a=drand48()*M_PI*2;
    copym(m,&mat_id);
    m->x.x=cos(a);
    m->x.y=sin(a);
    m->y.x=-m->x.y;
    m->y.y=m->x.x;
}

static void posem(struct matrix *m, struct vector *p)
{   // tourne légèrement la matrice pour la "poser" sur le sol
    m->x.z=z_ground(p->x+m->x.x,p->y+m->x.y, true)-z_ground(p->x,p->y, true);
    renorme(&m->x);
    m->y.x=-m->x.y;
    m->y.y=m->x.x;
    m->y.z=z_ground(p->x+m->y.x,p->y+m->y.y, true)-z_ground(p->x,p->y, true);
    renorme(&m->y);
    prodvect(&m->x,&m->y,&m->z);
}

static int randomvroute(struct vector *v)
{
    int i;
    do i=2+drand48()*(routeidx-4); while(route[i+1].ak==-1 || route[i].ak==-1);
    v->x=route[i].i.x;
    v->y=route[i].i.y;
    v->z=route[i].i.z;
    return i;
}

static void addbabase(int c)
{
    int x,y;
    int x1[4] = { MAP_LEN/5., 3.*MAP_LEN/5., MAP_LEN/5., 3.*MAP_LEN/5. };
    int y1[4] = { MAP_LEN/5., MAP_LEN/5., 3.*MAP_LEN/5., 3.*MAP_LEN/5. };
    int a[3] = { 0, 0, 0 };
    struct matrix m;
    struct vector p,pp;
    for (y=y1[c]; y<y1[c]+3.*MAP_LEN/10.; y+=5)
        for (x=x1[c]; x<x1[c]+3.*MAP_LEN/10.; x+=5) {
            int aa=x+(y<<LOG_MAP_LEN);
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
            a[x] = x1[x] + (y1[x]<LOG_MAP_LEN);
        }
        int yb=a[x]>>LOG_MAP_LEN;
        int xb=a[x]-(yb<<LOG_MAP_LEN);
        p.x=(xb-(MAP_LEN>>1))*TILE_LEN+TILE_LEN/2;
        p.y=(yb-(MAP_LEN>>1))*TILE_LEN+TILE_LEN/2;
        p.z=0;
        // Force the heighmap to be flat around airfields
        map[a[x]-1].submap=0;
        map[a[x]].submap=0;
        map[a[x]+1].submap=0;
        map[a[x]+2].submap=0;
        map[a[x]+3].submap=1;
        map[a[x]-1].submap=1;
        map[a[x]-MAP_LEN].submap=0;
        map[a[x]+MAP_LEN].submap=0;
        map[a[x]+MAP_LEN+1].submap=0;
        map[a[x]-MAP_LEN+1].submap=0;
        map[a[x]+MAP_LEN-1].submap=0;
        map[a[x]-MAP_LEN-1].submap=0;
        map[a[x]+MAP_LEN+2].submap=1;
        map[a[x]-MAP_LEN+2].submap=1;
        map[a[x]-1].z=map[a[x]].z;
        map[a[x]+1].z=map[a[x]].z;
        map[a[x]+2].z=map[a[x]].z;
        map[a[x]-MAP_LEN].z=map[a[x]].z;
        map[a[x]-MAP_LEN-1].z=map[a[x]].z;
        map[a[x]-MAP_LEN+1].z=map[a[x]].z;
        map[a[x]-MAP_LEN+2].z=map[a[x]].z;
        map[a[x]+MAP_LEN].z=map[a[x]].z;
        map[a[x]+MAP_LEN-1].z=map[a[x]].z;
        map[a[x]+MAP_LEN+1].z=map[a[x]].z;
        map[a[x]+MAP_LEN+2].z=map[a[x]].z;
    }
    for (x=0; x<3; x++) {
        int yb=a[x]>>LOG_MAP_LEN;
        int xb=a[x]-(yb<<LOG_MAP_LEN);
        p.x=(xb-(MAP_LEN>>1))*TILE_LEN+TILE_LEN/2;
        p.y=(yb-(MAP_LEN>>1))*TILE_LEN+TILE_LEN/2;
        p.z=0;
        airfield_obj[0][x][c]=addnobjet(NB_PLANES, &p, &mat_id, 1);  // la piste

        copyv(&pp,&p);
        pp.x+=(drand48()-.5)*TILE_LEN*.4;
        pp.y+=(drand48()+.5)*TILE_LEN*.2;
        pp.z=85;
        randomhm(&m);
        addnobjet(NB_PLANES+1, &pp, &m, 1); // une tour de controle
        for (y=0; y<3; y++) {
            pp.x+=(drand48()-.5)*TILE_LEN*.2;
            pp.y+=(drand48()-.5)*TILE_LEN*.2;
            pp.z=15;
            randomhm(&m);
            addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES+drand48()*3, &pp, &m, 1); // des tank
        }
        airfield_obj[1][x][c]=nb_obj;
        for (y=0; y<NB_PLANES; y++) {
            struct matrix mp = {
                { cos(.3), 0, sin(.3) },
                { 0,1,0 },
                { -sin(.3),0,cos(.3) }
            };
            copyv(&pp,&p);
            pp.x-=600;
            pp.y+=(y-NB_PLANES/2)*200;
            pp.z=20;
            addnobjet(y, &pp, y!=2?&mp:&mat_id, 1);
        }
    //  printf("Add Babase #%d, C#%d, at %f,%f\n",x,c,p.x,p.y);
    }
}

static void randomvferme(struct vector *p)
{
    int c,i,ok;
    do {
        ok=1;
        randomv(p);
        mulv(p,TILE_LEN*(MAP_LEN-MAP_LEN/4));
        p->z=z_ground(p->x,p->y, true);
        for (c=0; c<4; c++) for (i=0; i<3; i++) {
            struct vector pp;
            copyv(&pp,&obj[airfield_obj[0][i][c]].pos);
            subv(&pp,p);
            if (norme(&pp)<TILE_LEN*10) ok=0;
        }
    } while (map[akpos(p)].z<80 || map[akpos(p)].z>150 || !ok);
}

static int collisionpoint(struct vector *pp, int k, int mo)
{
    struct vector u;
    subv3(pp,&obj[k].pos,&u);
    return norme(&u)<=mod[mo].rayoncollision+mod[obj[k].model].rayoncollision;
}

void affjauge(float j)
{
    static float jauge=0;
    float nj=jauge+j;
    static int x=10;
    int nx,y,xx;
    nx=10+(int)(nj*(win_width-20.));
    if (nx>x) {
        for (y=win_center_y-(win_height>>3); y<win_center_y+(win_height>>3); y++)
            for (xx=x; xx<nx; xx++)
                //*(int*)&videobuffer[y*win_width+xx]=0x3060A0;
                MMXAddSatC((int*)&videobuffer[y*win_width+xx],0x001080);
        buffer2video();
        x=nx;
    }
    jauge=nj;
}

void initworld(void)
{
    int i,j,k;
    struct vector p;
    struct matrix m;

    bomb = malloc(NBBOT*4*sizeof(*bomb));    // FIXME: most bots have 2 or 4 bombs, but spitflame has 7!
    tank = malloc(NBTANKBOTS*sizeof(*tank));
    initmap();
    // caméra
    obj[0].objref=-1;
    obj[0].type=TYPE_CAMERA;
    obj[0].distance=-1;
    obj[0].prec=-1; obj[0].next=-1;
    map[0].first_obj = 0;
    obj[0].ak=0;
    nb_obj=1;
    // babases
    printf("Adding airfields...\n");
    for (j=0; j<4; j++) addbabase(j);
    // zeppelins
    if ((zep = malloc(sizeof(*zep)*NBZEP))==NULL) {
        perror("malloc zep"); exit(-1);
    }
    for (i=0; i<NBZEP; i++) {
        p.x=(drand48()-.5)*(MAP_LEN<<LOG_TILE_LEN)*0.8;
        p.y=(drand48()-.5)*(MAP_LEN<<LOG_TILE_LEN)*0.8;
        p.z=4000+z_ground(p.x,p.y, false);
        zep[i].o=addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES+NB_TANKS+NB_DECOS,&p, &mat_id, 0);
        zep[i].nav = p;
        zep[i].angx=zep[i].angy=zep[i].angz=zep[i].anghel=zep[i].vit=0;
        for (j=0; j<6; j++) zep[i].cib[j]=-1;
        zep[i].last_shot = 0;
    }
    // installations au sol
    // des villages de pauvres inocents pour souffrir
    for (i=0; i<NB_VILLAGES; i++) {
        int nbm=drand48()*20+5;
        int ok,k;
        struct vector pp;
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
                if (fabs(village[j].p.x-p.x)+fabs(village[j].p.y-p.y)<TILE_LEN*5) break;
        } while (j<i);
        // Make heighmap more flat around villages
        map[a=akpos(&p)].submap=0;
        map[a-1].submap=0;
        map[a+1].submap=0;
        map[a+MAP_LEN].submap=0;
        map[a-MAP_LEN].submap=0;
        map[a+MAP_LEN+1].submap=0;
        map[a-MAP_LEN+1].submap=0;
        map[a+MAP_LEN-1].submap=0;
        map[a-MAP_LEN-1].submap=0;
        p.z=z_ground(p.x,p.y, true);
        copyv(&village[i].p,&p);
        village[i].o1=nb_obj;
        randomv(&pp);   // une église
        mulv(&pp,100);
        addv(&pp,&p);
        randomhm(&m);
        posem(&m,&pp);
        pp.z=40;
        addnobjet(NB_PLANES+NB_AIRFIELDS+1, &pp, &mat_id, 1);
        for (j=0; j<nbm; j++) { // des maisons
            struct matrix m;
            randomhm(&m);
            do {
                ok=1;
                randomv(&pp);
                mulv(&pp,TILE_LEN/2);
                addv(&pp,&p);
                posem(&m,&pp);
                pp.z=48;
                for (k=village[i].o1; k<nb_obj; k++) if (collisionpoint(&pp,k,NB_PLANES+NB_AIRFIELDS+0)) {ok=0; break;}
            } while (!ok);
            addnobjet(NB_PLANES+NB_AIRFIELDS+0, &pp, &m, 1);
        }
        village[i].o2=nb_obj;
        village[i].nom=village_name[i];
        for (j=0; j<nbm>>1; j++) {  // des reverberes
            struct matrix m;
            randomhm(&m);
            do {
                ok=1;
                randomv(&pp);
                mulv(&pp,TILE_LEN/2);
                addv(&pp,&p);
                posem(&m,&pp);
                pp.z=34;
                for (k=village[i].o1; k<nb_obj; k++) if (collisionpoint(&pp,k,NB_PLANES+NB_AIRFIELDS+5)) {ok=0; break;}
            } while (!ok);
            addnobjet(NB_PLANES+NB_AIRFIELDS+5, &pp, &m, 1);
        }
    }
    // routes
    initroute();
    printf("Adding motorways...\n");
    for (i=0; i<NB_VILLAGES-1; i++) for (j=i+1; j<NB_VILLAGES; j++) {
        int k;
        float cp;
        struct vector v;
        copyv(&v,&village[i].p);
        subv(&v,&village[j].p);
        if (norme(&v)<TILE_LEN*5) continue;
        if (v.x==0 && v.y==0) continue;
        cp=cap(v.x,v.y);
        for (k=i+1; k<j; k++) {
            float dc;
            copyv(&v,&village[i].p);
            subv(&v,&village[k].p);
            if (norme(&v)<TILE_LEN*5) continue;
            if (v.x==0 && v.y==0) continue;
            dc=cap(v.x,v.y)-cp;
            if (dc<-M_PI) dc+=2*M_PI;
            else if (dc>M_PI) dc-=2*M_PI;
            if (fabs(dc)<M_PI/4) break;
        }
        if (k==j) {
            prospectroute(&village[i].p,&village[j].p);
        }
        affjauge(.75/(1.5*((NB_VILLAGES+1)*NB_VILLAGES)));
    }
    EndMotorways=routeidx;
    printf("Adding roads around cities...\n");
    for (i=0; i<NB_VILLAGES; i++) {
        int nbr=drand48()*5+5;  // prop à la taille de la ville
        int r;
        for (r=0; r<nbr; r++) {
            struct vector dest;
            randomv(&dest);
            mulv(&dest,TILE_LEN*(5+10*drand48()));   // prop à la taille de la ville
            addv(&dest,&village[i].p);
            dest.z=z_ground(dest.x,dest.y, false);
            prospectroute(&village[i].p,&dest);
            affjauge(.75/(3.*7.5*NB_VILLAGES));
        }
    }
    EndRoads=routeidx;
    printf("Adding footpaths...\n");
    for (i=0; i<150; i++) {
        int ri=EndMotorways+drand48()*(routeidx-EndMotorways-1);
        struct vector dest,v;
        if (route[ri].ak!=-1 && route[ri+1].ak!=-1) {
            copyv(&v,&route[ri+1].i);
            subv(&v,&route[ri].i);
            dest.x=v.y;
            dest.y=-v.x;
            dest.z=0;
            if (drand48()>.5) mulv(&dest,-1);
            mulv(&dest,(2+drand48()*3));
            addv(&dest,&route[ri].i);
            if (fabs(dest.x)<((MAP_LEN-5)<<(LOG_TILE_LEN-1)) && fabs(dest.y)<((MAP_LEN-5)<<(LOG_TILE_LEN-1))) {
                dest.z=z_ground(dest.x,dest.y, false);
                prospectroute(&route[ri].i,&dest);
            }
        }
        affjauge(.75/(3.*150.));
    }
/*  {
        struct vector u,v;
        u.x=-10*TILE_LEN+2345; u.y=10*TILE_LEN+1234; u.z=z_ground(u.x,u.y, true);
        v.x=10*TILE_LEN-2345; v.y=10*TILE_LEN+1234; v.z=z_ground(v.x,v.y, true);
        traceroute(&u,&v);
    }*/
    endinitroute();
    hashroute();
    printf("Adding villages...\n");
    // des fermes et des usines
    for (i=0; i<(NB_VILLAGES*10); i++) {
        struct vector pp;
        int ri;
        struct matrix m;
        do ri=EndRoads+drand48()*(routeidx-EndRoads-1);
        while (route[ri].ak!=-1 && route[ri+1].ak!=-1);
        copyv(&p,&route[ri].i);
        randomhm(&m);
        randomv(&pp);
        mulv(&pp,.5*TILE_LEN);
        addv(&pp,&p);
        posem(&m,&pp);
        if (i&1) {
            pp.z=10;
            addnobjet(NB_PLANES+NB_AIRFIELDS+2, &pp, &m, 1);
        } else {
            pp.z=20;
            addnobjet(NB_PLANES+NB_AIRFIELDS+3, &pp, &m, 1);
        }
    }
    printf("Adding farms...\n");
    // des maisons au bord des routes
    for (i=0; i<200; i++) {
        struct vector pp;
        int ri;
        struct matrix m;
        do ri=EndRoads+drand48()*(routeidx-EndRoads-1);
        while (route[ri].ak!=-1 && route[ri+1].ak!=-1);
        copyv(&p,&route[ri].i);
        randomhm(&m);
        randomv(&pp);
        mulv(&pp,.3*TILE_LEN);
        addv(&pp,&p);
        posem(&m,&pp);
        pp.z=48;
        addnobjet(NB_PLANES+NB_AIRFIELDS+0, &pp, &m, 1);
    }
    printf("Adding mills...\n");
    // des moulins
    mill_start=nb_obj;
    for (i=0; i<NB_VILLAGES*2; i++) {
        struct vector pp;
        int ri;
        struct matrix m;
        do ri=EndRoads+drand48()*(routeidx-EndRoads-1);
        while (route[ri].ak!=-1 && route[ri+1].ak!=-1);
        copyv(&p,&route[ri].i);
        randomhm(&m);
        randomv(&pp);
        mulv(&pp,3*TILE_LEN);
        addv(&pp,&p);
        pp.z=30;
        posem(&m,&pp);
        addnobjet(NB_PLANES+NB_AIRFIELDS+4, &pp, &m, 1);
    }
    mill_stop=nb_obj;
    printf("Adding cows...\n");
    // des troupeaux de charolaises
    for (i=0; i<NB_VILLAGES*2; i++) {
        int nbn=drand48()*5+2;
        copyv(&p,&village[i%NB_VILLAGES].p);
        for (k=0; k<2; k++) {
            struct vector pt;
            randomv(&pt);
            mulv(&pt,TILE_LEN*6);
            addv(&pt,&p);
            pt.z=0;
            for (j=0; j<nbn; j++) {
                struct vector pp;
                struct matrix m;
                randomhm(&m);
                randomv(&pp);
                mulv(&pp,TILE_LEN*.5*pow(nbn,.1));
                pp.z=12;
                addv(&pp,&pt);
                posem(&m,&pp);
                addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES+NB_TANKS+5, &pp, &m, 1);
            }
        }
    }
    printf("Adding vehicules...\n");
    // des véhicules en décors
    car = malloc((NB_CARS+1)*sizeof(*car));
    for (i=0; i<NB_CARS/4; i++) {
        car[i].r=randomvroute(&p);
        car[i].sens=1;
        p.z=15;
        car[i].o=addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES+1, &p, &mat_id, 1);
        car[i].dist=-1;
        car[i].vit=80+240*drand48();
    }
    for (; i<NB_CARS/2; i++) {
        car[i].r=randomvroute(&p);
        car[i].sens=1;
        p.z=15;
        car[i].o=addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES+4, &p, &mat_id, 1);
        car[i].dist=-1;
        car[i].vit=80+240*drand48();
    }
    for (; i<NB_CARS*8/10; i++) {
        car[i].r=randomvroute(&p);
        car[i].sens=1;
        p.z=15;
        car[i].o=addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES+2, &p, &mat_id, 1);
        car[i].dist=-1;
        car[i].vit=160+160*drand48();
    }
    for (; i<NB_CARS; i++) {
        car[i].r=randomvroute(&p);
        car[i].sens=1;
        p.z=30;
        car[i].o=addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES+3, &p, &mat_id, 1);
        car[i].dist=-1;
        car[i].vit=80+80*drand48();
    }
    car[i].o=nb_obj;
    printf("Adding tractors...\n");
    // des tracteurs dans les champs
    for (i=0; i<50; i++) {
        struct matrix m;
        randomhm(&m);
        randomvferme(&p);
        posem(&m,&p);
        p.z=30;
        addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES+3, &p, &m, 1);
    }
    printf("Adding trees...\n");
    // et des arbres
    for (i=0; i<150; i++) {
        struct matrix m;
        randomhm(&m);
        randomvferme(&p);
        posem(&m,&p);
        p.z=29;
        addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES+NB_TANKS+3, &p, &m, 1);
    }
/*  for (i=0; i<1; i++) {
        struct matrix m;
        randomhm(&m);
        randomvferme(&p);
        p.z=20;
        addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES+NB_TANKS+4, &p, &m, 1);
    }*/
    printf("Adding planes...\n");
    // des vionvions
    if ((bot=calloc(sizeof(*bot),NBBOT))==NULL) { perror("bot"); exit(-1); }
    bot[controled_bot].camp=camp;
    bot[controled_bot].navion=starting_plane-1;
    //if (NetCamp()==-1) {printf("Net Error\n"); exit(-1); }
    printf("Playing with %d planes & %d tanks\nPlayers :\n",NBBOT,NBTANKBOTS);
    for (i=0; i<NbHosts; i++) printf("%s, camp %d, in a %s\n",playbotname[i],bot[i].camp+1, plane_desc[bot[i].navion].name);
    for (i=0; i<NBBOT; i++) {
        int c=i&3, b;
        if (i>=NbHosts) bot[i].camp=c;
        bot[i].babase=b=airfield_obj[0][(int)(drand48()*3)][(int)bot[i].camp];
        if (! killemall_mode) {
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
        if (i >= NbHosts) bot[i].navion = drand48()*NB_PLANES;
        bot[i].vion = addnobjet(bot[i].navion,&p,&m, 0);
        bot[i].but.gear = !killemall_mode;
        bot[i].but.canon = 0;
        bot[i].but.bomb = 0;
        bot[i].but.gearup = 0;
        bot[i].but.brakes = 0;
        bot[i].but.business = 0;
        bot[i].anghel = 0;
        bot[i].anggear = 0;
        bot[i].xctl = bot[i].yctl = 0;
        bot[i].thrust = killemall_mode ? 1.:0.;
        bot[i].maneuver = killemall_mode ? NAVIG : PARKING;
        bot[i].aerobatic = MANEUVER;
        bot[i].gunned = -1;
        bot[i].fiulloss = bot[i].bloodloss = bot[i].motorloss = bot[i].aeroloss = 0;
        bot[i].fiul = plane_desc[bot[i].navion].fiulmax;
        bot[i].bullets = plane_desc[bot[i].navion].bulletsmax;
        bot[i].cibv = bot[i].cibt = -1;
        bot[i].gold = i > NbHosts ? 30000:2000;
        bot[i].is_flying = killemall_mode;
        armstate(i);
        if (!killemall_mode) {
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
        if (p.y>0) tank[i].camp=2; else tank[i].camp=0;
        if (p.x>0) tank[i].camp++;
        tank[i].cibv=-1;
        copyv(&tank[i].p,&p);
        p.z=20;
        tank[i].o1=addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES, &p, &mat_id, 1);
        tank[i].o2=nb_obj;
        tank[i].moteur=0;
        tank[i].cibt=-1;
        tank[i].ang0=tank[i].ang1=tank[i].ang2=0;
        tank[i].ocanon=0;
        tank[i].last_shot = 0;
        tank[i].nom=tankname;
    }
    printf("Adding clouds...\n");
    // et des nuages
    for (i=0; i<70; i++) {
        int nbn=drand48()*15+10;
        randomv(&p);
        mulv(&p,TILE_LEN*(MAP_LEN-5));
        p.z=drand48()*5000+20000;
        for (j=0; j<nbn; j++) {
            struct vector pp;
            randomv(&pp);
            mulv(&pp,TILE_LEN*pow(nbn,.3));
            pp.z/=2.;
            addv(&pp,&p);
            addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES+NB_TANKS, &pp, &mat_id, 0);
        }
    }
    printf("Adding smoke...\n");
    // et de la fumée
    smoke_start=nb_obj;
    for (i=0; i<MAX_SMOKES; i++) {
        addnobjet(NB_PLANES+NB_AIRFIELDS+NB_HOUSES+NB_TANKS+1,&vec_zero,&mat_id,0);
    }
}
