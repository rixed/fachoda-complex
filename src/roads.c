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
#include "heightfield.h"
#include "robot.h"

#define NBMAXROUTE 5000 /* <=65534 ! */
struct road *route;
int routeidx=0;
int NbElmLim;
int EndRus=-1, EndMotorways=-1, EndRoads=-1;
int largroute[3]={100,70,50};
int actutype=0;
#define NHASH 11    // 2048 max items in map2route hashtable
#define NBREPHASH 4 // max nbr of items per hash bucket
short (*map2route)[NBREPHASH];  // table de hash

int akpos(struct vector *p)
{
    int x,y;
    x=p->x+((WMAP<<NECHELLE)>>1);
    y=p->y+((WMAP<<NECHELLE)>>1);
    x>>=NECHELLE;
    y>>=NECHELLE;
    return x+(y<<NWMAP);
}

static void akref(int ak,struct vector *r)
{
    int x=(ak&(WMAP-1))-(WMAP>>1);
    int y=(ak>>NWMAP)-(WMAP>>1);
    r->x=(x<<NECHELLE);
    r->y=(y<<NECHELLE);
    r->z=z_ground(r->x,r->y, true);
}

void hashroute() {
    int i,j, nk, missed=0;
    int nbe[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  // nbr de cases utilisées pour chaque ligne de la table de hash
    // (une ligne = déteeminée par les 2 bits venant du numéro de ak)
    short hi;
    for (i=0; i<routeidx; i++) {
        if (route[i].ak==-1) continue;
        hi=(route[i].ak&(3<<NWMAP))>>(NWMAP-2);
        hi|=(route[i].ak&3);
        if (!(map[route[i].ak].has_road)) { // première route dans cette kase
            // on peut donc fixer arbitrairement le code nk
            nk=nbe[hi]++;
            if (nk>=(1<<7)) {
                printf("HI=%d\n",hi);
                printf("ERROR: Hash Table too narrow\n");
                exit(-1);
            }
            hi|=nk<<4;
            map2route[hi][0]=(short)i;
            map[route[i].ak].has_road = 1;
            map[route[i].ak].submap = nk;
        } else {    // il y a déjà une route dans cette kase
            // donc on ne peut pas fixer nk comme ca nous arrange
            // donc il faut rajouter cet elemnt de route à la meme
            // position dans le hash
            nk = map[route[i].ak].submap;
            hi|=nk<<4;
            for (j=0; map2route[hi][j]!=-1 && j<4; j++);
            if (j==0) printf("ERROR CONSTRUCTING HASH TABLE !\n");  // il y a deja un elmt ou pas ?
            if (j<4) map2route[hi][j]=(short)i;
            else missed++;
        }
    }
    for (i=j=0; i<16; i++) j+=nbe[i];
//  printf("Hash table fullness : %.0f %%\nMissed elements : %.0f %%\n",j*100./(float)(1<<NHASH),missed*100./j);
}

void initroute() {
    int i,j;
    route=(struct road*)malloc(NBMAXROUTE*sizeof(struct road));
    map2route=malloc((1<<NHASH)*NBREPHASH*sizeof(short));
    for (i=0; i<(1<<NHASH); i++)
        for (j=0; j<NBREPHASH; j++)
            map2route[i][j]=-1;

}
void endinitroute() {
    route=(struct road*)realloc(route,routeidx*sizeof(struct road));
}
float oldcap,curcap,bestcap;
float note(struct vector *a, struct vector *f, struct vector *i) {
    float n,dist,zs;
    struct vector v;
    if ((zs=z_ground(a->x,a->y, false))<3000 || zs>11500) n=MAXFLOAT;
    else {
        copyv(&v,a);
        subv(&v,i);
        if (v.y!=0 || v.x!=0) {
            curcap=cap(v.x,v.y);
            copyv(&v,f);
            subv(&v,i);
            dist=norme(&v);
            n=curcap-cap(v.x,v.y);
            if (n<-M_PI) n+=2*M_PI;
            else if (n>M_PI) n-=2*M_PI;
            n=fabs(n);
            n+=(dist>ECHELLE*10?.0015:.0025)*fabs(zs-z_ground(i->x,i->y, false));
            n+=0.003*fabs(zs-z_ground(f->x,f->y, false))/(dist+100);
            if (oldcap!=MAXFLOAT) {
                float dc=curcap-oldcap;
                if (dc<-M_PI) dc+=2*M_PI;
                else if (dc>M_PI) dc-=2*M_PI;
                dc=fabs(dc);
                n+=1.1*dc;
            }
        } else n=MAXFLOAT;
    }
    return n;
}
void nxtrt(struct vector i, struct vector *f, int lastd) {
    // routeidx pointe sur une route dont l'origine est mise, mais
    // pointant sur rien
    int d, bestd=0, intens;
    float bestnote=MAXFLOAT, notecur, dist, s;
    struct pixel coul;
    struct vector r,besta,u;
    float tmp;
    copyv(&r,f);
    subv(&r,&i);
    dist=norme(&r);
    if (NbElmLim<0 || dist<ECHELLE || routeidx>=NBMAXROUTE-1) {
        routeidx++;
        route[routeidx].ak=-1;  // mark fin de route
        routeidx++;
        return;
    }
    NbElmLim--;
    akref(route[routeidx].ak,&r);
//  printf("nxtrt: ak=%d  akref=%f %f  i=%f %f\n",route[routeidx].ak,r.x,r.y,route[routeidx].i.x,route[routeidx].i.y);
//  printf("nxtrt : from %f %f - dist=%f\n",i.x,i.y,dist);
    for (d=0; d<4; d++) {
        // d=coté parcouru : 0=sud, 1=ouest, 2=nord, 3=ouest
        if (d==(lastd^2)) continue;
        for (s=ECHELLE/5; s<ECHELLE*4/5; s+=ECHELLE/10) {
            float ll=d>1?ECHELLE:0;
            struct vector a;
            a.x=(d&1?ll:s)+r.x;
            a.y=(d&1?s:ll)+r.y;
            a.z=z_ground(a.x,a.y, false);
            if ((notecur=note(&a,f,&i))<bestnote) {
                bestnote=notecur;
                bestd=d;
                copyv(&besta,&a);
                bestcap=curcap;
            }
        }
    }
    if (bestnote>1000) {
        route[routeidx].ak=-1;
        routeidx++;
        return;
    }
    oldcap=bestcap;
//  printf(" found this way : dir=%d, leading to %f %f (note=%f)\n",bestd, besta.x,besta.y,bestnote);
    routeidx++;
    copyv(&route[routeidx].i,&besta);
    subv3(&route[routeidx-1].i,&route[routeidx].i,&u);
    tmp=u.x; u.x=u.y; u.y=-tmp; u.z=0;
    renorme(&u);
    mulv(&u,largroute[actutype]);
    addv3(&route[routeidx].i,&u,&route[routeidx].i2);
    route[routeidx].ak=route[routeidx-1].ak;
    *((int*)&route[routeidx-1].e.c)=0x4A6964;   // couleur par défaut
    switch (bestd) {
    case 0:
        route[routeidx].ak-=WMAP;
        d=route[routeidx].ak<WMAP;
        break;
    case 1:
        route[routeidx].ak-=2;
    case 3:
        route[routeidx].ak+=1;
        d=(route[routeidx].ak&WMAP)!=(route[routeidx-1].ak&WMAP);
        break;
    case 2:
        route[routeidx].ak+=WMAP;
        d=route[routeidx].ak>=WMAP<<(NWMAP-1);
        break;
    }
    if (d) {
//      printf("Interrputing road\n");
        route[routeidx].ak=-1;
        routeidx++;
        return;
    }
    // colore la route
    intens=((map[route[routeidx].ak].z-map[(route[routeidx].ak-1)&((WMAP<<NWMAP)-1)].z))+32+64;
    if (intens<80) intens=80;
    else if (intens>117) intens=117;
    if (EndMotorways==-1) {coul.r=120; coul.g=150; coul.b=130; }
    else if (EndRoads==-1) {coul.r=90; coul.g=130; coul.b=110; }
    else { coul.r=100; coul.g=120; coul.b=70; }
    route[routeidx-1].e.c.r=(coul.r*intens)>>7;
    route[routeidx-1].e.c.g=(coul.g*intens)>>7;
    route[routeidx-1].e.c.b=(coul.b*intens)>>7;
    if (route[routeidx-1].e.c.r<20) route[routeidx-1].e.c.r=20; // pour éviter les swaps
    if (route[routeidx-1].e.c.g<20) route[routeidx-1].e.c.g=20;
    if (route[routeidx-1].e.c.b<20) route[routeidx-1].e.c.b=20;
    // avance le chantier
    akref(route[routeidx].ak,&r);
    nxtrt(besta,f,bestd);
}

void traceroute(struct vector *i, struct vector *f) {
    if (routeidx>=NBMAXROUTE-1) return;
    route[routeidx].ak=akpos(i);
    copyv(&route[routeidx].i,i);
    copyv(&route[routeidx].i2,i);
    oldcap=MAXFLOAT;
    nxtrt(*i,f,-1);
}

void prospectroute(struct vector *i, struct vector *f) {
    int deb=routeidx, bestfin=MAXINT;
    int j,k;
    int nbelmlim;   // nb d'element au dela duquel ca vaut pas le coup
    struct vector p1,p2, bestp1, bestp2, v;
    copyv(&p1,i);
    copyv(&p2,f);
    copyv(&v,f);
    subv(&v,i);
    nbelmlim=8.*norme(&v)/ECHELLE;
    if (EndMotorways==-1) actutype=0;
    else if (EndRoads==-1) actutype=1;
    else actutype=2;
    for (j=0; j<10; j++) {
        if (j) {
            randomv(&v);
            mulv(&v,ECHELLE*(EndRoads==-1?2:0.00001));
            addv(&p1,&v);
            p1.z=z_ground(p1.x,p1.y, false);
            randomv(&v);
            mulv(&v,ECHELLE*(EndRoads==-1?2:3));
            addv(&p2,&v);
            p2.z=z_ground(p2.x,p2.y, false);
        }
        NbElmLim=nbelmlim;
        traceroute(&p1,&p2);
        k=routeidx-2;
        if (k>=0) {
//          akref(route[k].ak,&v);
            v = route[k].i;
            subv(&v, f);
            if (
                routeidx < bestfin &&
                norme(&v) < 2.*ECHELLE
            ) {
                bestp1 = p1;
                bestp2 = p2;
                bestfin = routeidx;
            }
        }
        routeidx=deb;
    }
    if (bestfin-routeidx<nbelmlim) {
        NbElmLim=nbelmlim;
        traceroute(&bestp1,&bestp2);
    }
}

static void drawroadline(int x1, int y1, int x2, int y2, int l, struct pixel c1, struct pixel c2)
{
    // trace la route (r,r+1) en ligne, vect2dc étant calculé
    int s,ss,ssi,ssf,medline, x,y,xi, dy, cr,cg,cb, qdizor;
    int q, qcr,qcb,qcg;
    if (x1>x2) {
        int tmp = x1; x1 = x2; x2 = tmp;
        tmp = y1; y1 = y2; y2 = tmp;
        struct pixel tmpp = c1; c1 = c2; c2 = tmpp;
    }
    if (x2<0 || x1>=SX) return;
    // clip...
    if (x2-x1) {
        if (x1<0) {
            y1+=-(x1*(y2-y1))/(x2-x1);
            c1.r+=((c1.r-c2.r)*x1)/(x2-x1);
            c1.g+=((c1.g-c2.g)*x1)/(x2-x1);
            c1.b+=((c1.b-c2.b)*x1)/(x2-x1);
            x1=0;
        }
        if (x2>=SX) {
            y2-=(y2-y1)*(x2-SX)/(x2-x1);
            c2.r-=(x2-SX)*(c2.r-c1.r)/(x2-x1);
            c2.g-=(x2-SX)*(c2.g-c1.g)/(x2-x1);
            c2.b-=(x2-SX)*(c2.b-c1.b)/(x2-x1);
            x2=SX-1;
        }
    }
    if (y2-y1) {
        if (y1<0) {
            x1+=(y1*(x1-x2))/(y2-y1);
            c1.r+=((c1.r-c2.r)*y1)/(y2-y1);
            c1.g+=((c1.g-c2.g)*y1)/(y2-y1);
            c1.b+=((c1.b-c2.b)*y1)/(y2-y1);
            y1=0;
        }
        if (y2>=SY) {
            x2-=(x2-x1)*(y2-SY)/(y2-y1);
            c2.r-=(y2-SY)*(c2.r-c1.r)/(y2-y1);
            c2.g-=(y2-SY)*(c2.g-c1.g)/(y2-y1);
            c2.b-=(y2-SY)*(c2.b-c1.b)/(y2-y1);
            y2=SY-1;
        }
    }
    if ((dy=y2-y1)>0) {
        if (y2<0 || y1>=SY) return;
        s=1;
    } else {
        if (y1<0 || y2>=SY) return;
        dy=-dy;
        s=-1;
    }
    q = ((x2-x1)<<vf)/++dy;
    ss=x2-x1>=dy;
    if (ss) qdizor=x2-x1+1;
    else qdizor=dy+1;
    qcr= ((c2.r-c1.r)<<vf)/qdizor;
    qcg= ((c2.g-c1.g)<<vf)/qdizor;
    qcb= ((c2.b-c1.b)<<vf)/qdizor;
    x = x1<<vf;
    cr = c1.r<<vf;
    cg = c1.g<<vf;
    cb = c1.b<<vf;
    if (l>1) {
        ssi=-(l>>1);
        ssf=ssi+l;
        medline=l>5;
        if (ss) for (y=y1; dy>=0; dy--, y+=s) {
            for (xi=x>>vf; xi<1+((x+q)>>vf); xi++) {
                for (ss=ssi; ss<ssf; ss++) plot(xi-_DX,y-_DY+ss,(!ss && medline)?0x909030:(((cr>>vf)<<16)&0xFF0000)+(((cg>>vf)<<8)&0xFF00)+((cb>>vf)&0xFF));
                cr+=qcr;
                cg+=qcg;
                cb+=qcb;
            }
            x+=q;
        } else for (y=y1; dy>=0; dy--, y+=s) {
            for (ss=ssi; ss<ssf; ss++) plot((x>>vf)-_DX+ss,y-_DY,(!ss && medline)?0x909030:(((cr>>vf)<<16)&0xFF0000)+(((cg>>vf)<<8)&0xFF00)+((cb>>vf)&0xFF));
            cr+=qcr;
            cg+=qcg;
            cb+=qcb;
            x+=q;
        }
    } else {
        if (ss) for (y=y1; dy>=0; dy--, y+=s) {
            for (xi=x>>vf; xi<1+((x+q)>>vf); xi++) {
                plot(xi-_DX,y-_DY,(((cr>>vf)<<16)&0xFF0000)+(((cg>>vf)<<8)&0xFF00)+((cb>>vf)&0xFF));
                cr+=qcr;
                cg+=qcg;
                cb+=qcb;
            }
            x+=q;
        } else for (y=y1; dy>=0; dy--, y+=s) {
            plot((x>>vf)-_DX,y-_DY,(((cr>>vf)<<16)&0xFF0000)+(((cg>>vf)<<8)&0xFF00)+((cb>>vf)&0xFF));
            cr+=qcr;
            cg+=qcg;
            cb+=qcb;
            x+=q;
        }
    }
}

static void couperoute(struct vect2dc *e, struct vector *v1, struct vector *v2)
{
#   define H (32<<3)
    struct vector p;
    p.x=((v2->x-v1->x)*(H-v1->z))/(v2->z-v1->z)+v1->x;
    p.y=((v2->y-v1->y)*(H-v1->z))/(v2->z-v1->z)+v1->y;
    p.z=H;
    proj(&e->v,&p);
}

void drawroute(int k)
{
    int nk, hi, r;
    int j,i, typ, larg;
    struct vector pt3d[4], v,u;
    struct vecic pt[4];
    assert(map[k].has_road);
    nk = map[k].submap;
    hi=(k&(3<<NWMAP))>>(NWMAP-2);
    hi|=k&3;
    hi|=nk<<4;
    for (j=0; j<4; j++) {
        r=map2route[hi][j];
        if (r==-1) break;
        if (route[r].ak==-1) {
            printf("HASH TABLE ERROR: FOUND A ROAD TERMINATION ELEMENT AT route[%d]\n",r);
            printf("Kase=%d ; HI=%d ; NK=%d ; j=%d\n",k,hi,nk,j);
            exit(-1);
        }
        if (route[r].ak!=k) {
            printf("HASH TABLE ERROR: ROAD KASE DO NOT MATCH MAP KASE\n");
            printf("Kase=%d ; HI=%d ; NK=%d ; j=%d\n",k,hi,nk,j);
            exit(-1);
        }
        if (r<EndMotorways) typ=0;
        else if (r<EndRoads) typ=1;
        else typ=2;
        if (route[r+1].ak!=-1) {
            // calculer les coords 2D des deux extrémitées de la route
            for (i=0; i<2; i++) {
                subv3(&route[r+i].i,&obj[0].pos,&v);
                mulmtv(&obj[0].rot,&v,&pt3d[i]);
            }
            larg=(int)((largroute[typ]*focale)/norme(&v));
            if (larg<1) {
                if (pt3d[0].z>H) {
                    proj(&route[r].e.v,&pt3d[0]);
                    if (pt3d[1].z>H) {
                        proj(&route[r+1].e.v,&pt3d[1]);
                        drawroadline(route[r].e.v.x,route[r].e.v.y,route[r+1].e.v.x,route[r+1].e.v.y,larg,route[r].e.c,route[r+1].e.c);
                    } else {
                        couperoute(&route[r+1].e,&pt3d[1],&pt3d[0]);
                        drawroadline(route[r].e.v.x,route[r].e.v.y,route[r+1].e.v.x,route[r+1].e.v.y,larg,route[r].e.c,route[r+1].e.c);
                    }
                } else if (pt3d[1].z>H) {
                    proj(&route[r+1].e.v,&pt3d[1]);
                    couperoute(&route[r].e,&pt3d[0],&pt3d[1]);
                    drawroadline(route[r].e.v.x,route[r].e.v.y,route[r+1].e.v.x,route[r+1].e.v.y,larg,route[r].e.c,route[r+1].e.c);
                }
            } else {    // route proche
                subv3(&route[r].i2,&obj[0].pos,&u);
                mulmtv(&obj[0].rot,&u,&pt3d[2]);
                subv3(&route[r+1].i2,&obj[0].pos,&u);
                mulmtv(&obj[0].rot,&u,&pt3d[3]);
                pt[0].v.x=pt3d[0].x*256;
                pt[0].v.y=pt3d[0].y*256;
                pt[0].v.z=pt3d[0].z*256;
                pt[1].v.x=pt3d[1].x*256;
                pt[1].v.y=pt3d[1].y*256;
                pt[1].v.z=pt3d[1].z*256;
                pt[2].v.x=pt3d[2].x*256;
                pt[2].v.y=pt3d[2].y*256;
                pt[2].v.z=pt3d[2].z*256;
                pt[3].v.x=pt3d[3].x*256;
                pt[3].v.y=pt3d[3].y*256;
                pt[3].v.z=pt3d[3].z*256;
                pt[0].c=pt[2].c=route[r].e.c;
                pt[1].c=pt[3].c=route[r+1].e.c;
                polyclip(&pt[1],&pt[0],&pt[2]);
                polyclip(&pt[3],&pt[1],&pt[2]);
            }
        }
    }
    if (!j) {
        printf("HASH TABLE ERROR: MAPMAP FLAGED BUT NO ROAD ELEMENT PRESENT\n");
        exit(-1);
    }
}
