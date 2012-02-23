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
#include "file.h"

short int sxtbtile, sytbtile;
int xsoute,ysoute,xthrust,ythrust,rthrust,xspeed,yspeed,rspeed,xassi,yassi,rassi,xinclin,yinclin,hinclin,dxinclin,xgear,ygear,rgear,xflap,yflap,rflap,xvert,yvert,rvert,xalti,yalti,ralti,xbous,ybous,hbous,dxbous,rbous,xfrein,yfrein,rfrein,xauto,yauto,rauto;
struct pixel32 *tbtile, *tbback, *tbback1, *tbback2;
uchar *tbz;
int *tbwidth;
int *boussole;
void rectangle(int *v, int rx, int ry, int c) {
    while (ry>0) {
        memset32(v,c,rx);
        v+=256;
        ry--;
    }
}
void disque(int *v, int r, int c) {
    int balance=-r, xoff=0, yoff=r;
    do {
        rectangle(v-xoff+(yoff<<8), xoff+xoff, 1, c);
        rectangle(v-xoff+(-yoff<<8), xoff+xoff, 1, c);
        rectangle(v-yoff+(xoff<<8), yoff+yoff, 1, c);
        rectangle(v-yoff+(-xoff<<8), yoff+yoff, 1, c);
        if ((balance += xoff + xoff + 1) >= 0) {
            yoff --;
            balance -= yoff + yoff;
        }
    } while (++xoff <= yoff);
}
void rectangletb(struct pixel32 *v, int rx, int ry, int c) {
    while (ry>0) {
        memset32((int*)v,c,rx);
        v+=SXTB;
        ry--;
    }
}
void disquetb(struct pixel32 *v, int r, int c) {
    int balance=-r, xoff=0, yoff=r;
    do {
        rectangletb(v-xoff+SXTB*(yoff), xoff+xoff, 1, c);
        rectangletb(v-xoff+SXTB*(-yoff), xoff+xoff, 1, c);
        rectangletb(v-yoff+SXTB*(xoff), yoff+yoff, 1, c);
        rectangletb(v-yoff+SXTB*(-xoff), yoff+yoff, 1, c);
        if ((balance += xoff + xoff + 1) >= 0) {
            yoff --;
            balance -= yoff + yoff;
        }
    } while (++xoff <= yoff);
}
void cercletb(int x, int y, int r, int c) {
    int balance=-r, xoff=0, yoff=r;
    do {
        *((int*)tbback+x+xoff+(y+yoff)*SXTB)=c;
        *((int*)tbback+x-xoff+(y+yoff)*SXTB)=c;
        *((int*)tbback+x-xoff+(y-yoff)*SXTB)=c;
        *((int*)tbback+x+xoff+(y-yoff)*SXTB)=c;
        *((int*)tbback+x+yoff+(y-xoff)*SXTB)=c;
        *((int*)tbback+x-yoff+(y-xoff)*SXTB)=c;
        *((int*)tbback+x-yoff+(y+xoff)*SXTB)=c;
        *((int*)tbback+x+yoff+(y+xoff)*SXTB)=c;
        if ((balance += xoff + xoff + 1) >= 0) {
            yoff --;
            balance -= yoff + yoff;
        }
    } while (++xoff <= yoff);
}
void gradutb(int x, int y, double a, int r1, int r2, int c) {
    struct vect2d p1,p2;
    p1.x=x+r1*cos(a);
    p1.y=y-r1*sin(a);
    p2.x=x+r2*cos(a);
    p2.y=y-r2*sin(a);
    drawlinetb(&p1,&p2,c);
}
void rectangleZ(int x, int y, int rx, int ry, int c) {
    int xx,yy;
    rectangletb(tbback+x+y*SXTB,rx,ry,c);
    for (yy=0; yy<ry; yy++) {
        double p=(double)yy*2./(double)ry-1.;
        double pp=1.-p*p*p*p;
        for (xx=0; xx<rx; xx++) {
            tbz[x+xx+SXTB*(y+yy)]=255*sin(xx*M_PI/rx)*pp;
        }
    }
}
void disqueZ(int x, int y, int r, int c) {
    int xx,yy;
    double r2=r*.85;
    disquetb(tbback+x+SXTB*y,r,c);
    for (yy=-r; yy<+r; yy++) {
        for(xx=-r; xx<r; xx++) {
            double rr=sqrt(xx*xx+yy*yy);
            if (rr<=r && rr>r2) {
                tbz[x+xx+SXTB*(y+yy)]=40*sin(((double)rr-r2)*M_PI/(r-r2));
            } else if (rr<=r2) {
                tbz[x+xx+SXTB*(y+yy)]=255*cos((double)rr*.5*M_PI/r2);
            }
        }
    }
}
void loadtbtile(char *fn) {
    FILE *f;
    int x,y;
    struct pixel32 *vid;
    float delta, a;
    if ((f=file_open(fn, DATADIR, "r"))==NULL) {
        exit(-1);
    }
    fseek(f,12,SEEK_SET);
    fread(&sxtbtile,2,1,f);
    fread(&sytbtile,2,1,f);
    fseek(f,-sxtbtile*sytbtile*sizeof(struct pixel),SEEK_END);
    tbtile = malloc(sxtbtile*sytbtile*sizeof(*tbtile));
    for (y=0; y<sytbtile; y++) for (x=0; x<sxtbtile; x++) {
            struct pixel p;
            fread(&p, sizeof(p), 1, f);
            tbtile[x+y*sxtbtile].r=p.r;
            tbtile[x+y*sxtbtile].g=p.g;
            tbtile[x+y*sxtbtile].b=p.b;
    }
    fclose(f);
    tbz=(uchar*)malloc(SXTB*SYTB*sizeof(uchar));
    tbback = malloc(SXTB*SYTB*sizeof(*tbback));
    vid=tbback;
    for (y=0; y<SYTB; y++) {
        for (x=0; x<SXTB-sxtbtile; x+=sxtbtile) MMXCopy((int*)vid+x,(int*)tbtile+(y%sytbtile)*sxtbtile,sxtbtile);
        MMXCopy((int*)vid+x,(int*)tbtile+(y%sytbtile)*sxtbtile,SXTB-x);
        vid+=SXTB;
    }
    free(tbtile);
    for (y=0; y<SYTB; y++) {
        for (x=0; x<SXTB; x++) {
            tbz[x+y*SXTB]=0;
        }
    }
    delta=((SYTB<<2)-320)/13;

    rvert=(SYTB-3*delta)/4;
    xvert=delta+rvert;
    yvert=SYTB>>2;
    disqueZ(xvert,yvert,rvert,0);
    gradutb(xvert,yvert,M_PI,0,rvert-1,0x707070);
    gradutb(xvert,yvert,M_PI-4*M_PI/5,0,rvert-1,0x303080);
    gradutb(xvert,yvert,M_PI+4*M_PI/5,0,rvert-1,0x803030);
    for (x=0; x<5; x++) {
        a=x*M_PI/5;
        gradutb(xvert,yvert,M_PI-a,rvert-5,rvert-1,0x3030FF);
        gradutb(xvert,yvert,M_PI+a,rvert-5,rvert-1,0xFF3030);
    }

    xassi=xvert;
    yassi=3*yvert;
    rassi=rvert;
    disqueZ(xassi,yassi,rassi,0);
    gradutb(xassi,yassi,0,rassi-5,rassi-1,0xD0D0D0);
    gradutb(xassi,yassi,M_PI,rassi-5,rassi-1,0xD0D0D0);
    gradutb(xassi,yassi,-M_PI/3,rassi-8,rassi-1,0xD0D0D0);
    gradutb(xassi,yassi,-2*M_PI/3,rassi-8,rassi-1,0xD0D0D0);
    gradutb(xassi,yassi,-M_PI/6,rassi-6,rassi-1,0xD0D0D0);
    gradutb(xassi,yassi,-5*M_PI/6,rassi-6,rassi-1,0xD0D0D0);

    xspeed=xvert+rvert*2+delta;
    yspeed=yvert;
    rspeed=rvert;
    disqueZ(xspeed,yspeed,rspeed,0);
    cercletb(xspeed,yspeed,rspeed-2,0xD0D0D0);
    for (x=0; x<15; x++) {
        a=M_PI/2-x*.4;
        gradutb(xspeed,yspeed,a,rspeed-4,rspeed-1,0xD0D0D0);
    }
    a=0;
    do {
        gradutb(xspeed,yspeed,M_PI/2-a,rspeed-6,rspeed-4,a<1.5?0xD0D0D0:(a<4.5?0x20E020:0xE02020));
        a+=.01;
    } while (a<M_PI*1.9);

    xthrust=xspeed;
    ythrust=yassi;
    rthrust=rvert;
    disqueZ(xthrust,ythrust,rthrust,0);
    for (a=0; a<M_PI; a+=.001) gradutb(xthrust,ythrust,a,rthrust*(.9-.07*floor(a*1.2)),rthrust*.9,0x505050);
    cercletb(xthrust,ythrust,rthrust-2,0xD0D0D0);
    for (x=0; x<6; x++) {
        double a=x*M_PI/5;
        gradutb(xthrust,ythrust,a,rthrust-4,rthrust-1,0xD0D0D0);
    }
    for (a=-M_PI; a<0; a+=.001) gradutb(xthrust,ythrust,a,rthrust*.55,rthrust*.8,a<-2?0x2020A0:(a<-.7?0x303030:0xA02020));

    xinclin=xthrust+rthrust+delta;
    yinclin=delta*3;
    hinclin=SYTB-6*delta;
    dxinclin=hinclin*.1;
    rectangleZ(xinclin,yinclin,dxinclin,hinclin,0x03030);

    ralti=rvert;
    xalti=xinclin+dxinclin+delta+ralti;
    yalti=yvert;
    disqueZ(xalti,yalti,ralti,0);
    cercletb(xalti,yalti,ralti-2,0xD0D0D0);
    for (x=0; x<10; x++) {
        a=-M_PI/2+x*M_PI/5;
        gradutb(xalti,yalti,a,ralti-4,ralti-1,0xD0D0D0);
        if (!(x&1)) pcharady(x+16,(int*)tbback+(int)(xalti+.6*ralti*cos(a))+((int)(yalti-4+.6*ralti*sin(a)))*SXTB,0xE0E0E0,SXTB);
    }
    cercletb(xalti,yalti,1,0xA0A0A0);

    rbous=rvert;
    xbous=xalti;
    ybous=yassi;
    disqueZ(xbous,ybous,rbous,0);
    pcharady('E',(int*)tbback+xbous-3+rbous/2+(ybous-4)*SXTB,0xE0E0E0,SXTB);
    pcharady('N',(int*)tbback+xbous-3+(ybous-rbous/2-4)*SXTB,0xF03030,SXTB);
    pcharady('O',(int*)tbback+xbous-3-rbous/2+(ybous-4)*SXTB,0xE0E0E0,SXTB);
    pcharady('S',(int*)tbback+xbous-3+(ybous+rbous/2-4)*SXTB,0x3030F0,SXTB);
    for (a=0; a<2*M_PI; a+=M_PI/2) {
        gradutb(xbous,ybous,a,0,rbous/3,0xD0D0D0);
        gradutb(xbous,ybous,a,2*rbous/3,.8*rbous,0xD0D0D0);
    }
    xsoute=xalti+ralti+delta;
    ysoute=delta*4;
    rectangleZ(xsoute-1,ysoute-1,32,22,0x404010);

    rgear=(32-delta)/4;
    xgear=xsoute+rgear;
    ygear=ysoute+22+delta+rgear;
    disqueZ(xgear,ygear,rgear,0);
    xflap=xgear+rgear*2+delta;
    yflap=ygear;
    rflap=rgear;
    disqueZ(xflap,yflap,rflap,0);
    xfrein=xgear;
    yfrein=ygear+rgear*2+delta;
    rfrein=rgear;
    disqueZ(xfrein,yfrein,rfrein,0);
    xauto=xflap;
    yauto=yfrein;
    rauto=rgear;
    disqueZ(xauto,yauto,rauto,0);

}
void drawtbback() {
    int y;
    for (y=0; y<SYTB; y++) MMXCopy(mapping+((MARGE+y)<<8)+MARGE,(int*)tbback+SXTB*y,SXTB);
}

int lx,ly,lz,lumdec=6;
void rectangleL(int x,int y, int rx,int ry) {
    int xx,yy;
    if (lz<=0) return;
    for (yy=y; yy<(y+ry); yy++) {
        for (xx=x; xx<x+rx; xx++) {
            int j = ((100+((lx*(tbz[xx+yy*SXTB+1]-tbz[xx+yy*SXTB])+ly*(tbz[xx+yy*SXTB+SXTB]-tbz[xx+yy*SXTB]))>>lumdec))*lz)>>lumdec;
            MMXAddSat((int*)mapping+MARGE+((MARGE+yy)<<8)+xx,j);
        }
    }
}
void disqueL(int x, int y, int r) {
    int balance=-r, xoff=0, yoff=r, newyoff=1;
    do {
        if (newyoff) {
            rectangleL(x-xoff,y+yoff, xoff+xoff, 1);
            rectangleL(x-xoff,y-yoff, xoff+xoff, 1);
        }
        if (xoff!=yoff) {
            rectangleL(x-yoff,y+xoff, yoff+yoff, 1);
            if (xoff) rectangleL(x-yoff,y-xoff, yoff+yoff, 1);
        }
        if ((balance += xoff + xoff + 1) >= 0) {
            yoff --;
            balance -= yoff + yoff;
            newyoff=1;
        } else newyoff=0;
    } while (++xoff <= yoff);
}
void drawtbcadrans(int b) {
    struct vect2d p1,p2;
    double a, ai, aj, ak;
    int i;
    // Cargo
    pnuma(bot[b].bullets,MARGE+xsoute+1+20,MARGE+ysoute+1,0x403010,0);
    pnuma(bot[b].nbomb,MARGE+xsoute+1+20,MARGE+ysoute+11, 0x403010,0);
    pnuma(bot[b].bullets,MARGE+xsoute+20,MARGE+ysoute,b==bmanu && arme==0?0xFFFFFF:0xFFFF10,0);
    pnuma(bot[b].nbomb,MARGE+xsoute+20,MARGE+ysoute+10,b==bmanu && arme==1?0xFFFFFF:0xFFFF10,0);
    lumdec=7;
    rectangleL(xsoute-1,ysoute-1,32,22);    // regrouper à la fin et faire un seul MMXSAVEFPU
    // Thrust
    p1.x=p2.x=xthrust;
    p1.y=p2.y=ythrust;
    a=(M_PI-.2)*bot[b].thrust+.1;
    p2.x+=rthrust*cos(a);
    p2.y+=-rthrust*sin(a);
    drawline2(&p1,&p2,0x40E060);
    // Fiul
    p2.x=xthrust;
    p2.y=ythrust;
    a=(M_PI-.2)*bot[b].fiul/plane_desc[bot[b].navion].fiulmax+.1;
    p2.x+=rthrust*cos(a);
    p2.y+=rthrust*sin(a);
    drawline2(&p1,&p2,0x707070);
    lumdec=6;
    disqueL(xthrust,ythrust,rthrust);
    // Airspeed indicator
    p1.x=p2.x=xspeed;
    p1.y=p2.y=yspeed;
    a = M_PI*.5 - .015*bot[b].vitlin;
    if (a > M_PI*.5) a = M_PI*.5;
    else if (a < -1.5*M_PI + .01) a = -1.5*M_PI + .01;
    p2.x += rspeed*cos(a);
    p2.y -= rspeed*sin(a);
    drawline2(&p1, &p2, 0xF02070);
    disqueL(xspeed, yspeed, rspeed);
    // Altimeter
    ai=obj[bot[b].vion].pos.z/10;
    aj=ai/10;
    ak=aj/10;
    aj-=floor(ak)*10;
    ai-=floor(aj)*10+floor(ak)*100;
    p1.x=xalti;
    p1.y=yalti;
    p2.x=p1.x+(ralti-1)*cos(M_PI*.5-ai*M_PI/5);
    p2.y=p1.y-(ralti-1)*sin(M_PI*.5-ai*M_PI/5);
    drawline2(&p1,&p2,0xD0D0D0);
    p2.x=p1.x+ralti*.7*cos(M_PI*.5-aj*M_PI/5);
    p2.y=p1.y-ralti*.7*sin(M_PI*.5-aj*M_PI/5);
    drawline2(&p1,&p2,0x5050F0);
    p2.x=p1.x+ralti*.5*cos(M_PI*.5-ak*M_PI/5);
    p2.y=p1.y-ralti*.5*sin(M_PI*.5-ak*M_PI/5);
    drawline2(&p1,&p2,0xE0E040);
    disqueL(xalti,yalti,ralti);
    // Attitude indicator
    p2.y=-rassi*.8*obj[bot[b].vion].rot.y.z;
    p2.x=sqrt(rassi*rassi*.64-p2.y*p2.y);
    if (obj[bot[b].vion].rot.z.z<0) p2.x=-p2.x;
    p1.x=xassi-p2.x;
    p1.y=yassi-p2.y;
    p2.x+=xassi;
    p2.y+=yassi;
    drawline2(&p1,&p2,0xC0C0C0);
    p1.x=xassi;
    p1.y=yassi;
    a=p2.x;
    p2.x=(p2.y-yassi)*.5+xassi;
    p2.y=-(a-xassi)*.5+yassi;
    drawline2(&p1,&p2,0xC0C0C0);
    disqueL(xassi,yassi,rassi);
    // Vertical speed
    p1.x=xvert;
    p1.y=yvert;
    a=.025*bot[b].vionvit.z;
    if (a>3) a=3;
    else if (a<-3) a=-3;
    p2.x=p1.x-rvert*cos(a);
    p2.y=p1.y-rvert*sin(a);
    drawline2(&p1,&p2,0xF0C0C0);
    disqueL(xvert,yvert,rvert);
    // Inclination relative to airspeed
    a = scalaire(&bot[b].vionvit, &obj[bot[b].vion].rot.z);
#   define amax 50
    i=(hinclin>>1)*(1.+a/amax);
    if (i<0) i=0;
    for (; i<hinclin; i++)
        memset32((int*)mapping+MARGE+xinclin+((MARGE+yinclin+i)<<8),0xB04242,dxinclin);
    lumdec=7;
    rectangleL(xinclin,yinclin,dxinclin,hinclin);
    // Gear
    if (bot[b].but.gear) {
        disque(mapping+xgear+MARGE+((MARGE+ygear)<<8),rgear*.8,0xCC2020);
    }
    disqueL(xgear,ygear,rgear);
    // Flap
    if (bot[b].but.flap) {
        disque(mapping+xflap+MARGE+((MARGE+yflap)<<8),rflap*.8,0xCC2020);
    }
    disqueL(xflap,yflap,rflap);
    // Brakes
    if (bot[b].but.frein) {
        disque(mapping+xfrein+MARGE+((MARGE+yfrein)<<8),rfrein*.8,0xCC2020);
    }
    disqueL(xfrein,yfrein,rfrein);
    // Autopilot
    if (autopilot) {
        disque(mapping+xauto+MARGE+((MARGE+yauto)<<8),rauto*.8,0x20CC20);
    }
    disqueL(xauto,yauto,rauto);
    // Compas
    p1.x=xbous; p1.y=ybous;
    p2.x=rbous*.6*cos(bot[visubot].cap)+p1.x;
    p2.y=-rbous*.6*sin(bot[visubot].cap)+p1.y;
    drawline2(&p1,&p2,0xF0F000);
    lumdec=6;
    disqueL(xbous,ybous,rbous);
}
