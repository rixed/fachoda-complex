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
#include <stdbool.h>
#include <math.h>
#include <values.h>
#include <assert.h>
#include "video_sdl.h"
#include "heightfield.h"

struct vect2dlum *pts2d;
struct matrix *oL;
void initrender() {
    pts2d = malloc(3000*sizeof(*pts2d));   // nbpts max par objets
#define MAXNO 5000
    oL = malloc(MAXNO*sizeof(*oL));   // nb objet max dans un ak
}
void plot(int x, int y, int r)
{
    if(x<win_center_x && x>=-win_center_x && y<win_center_y && y>=-win_center_y) {
        ((int*)videobuffer)[x+win_center_x+win_width*(y+win_center_y)]=r;
    }
}

void mixplot(int x, int y, int r, int g, int b){
    int c;
    int rr,gg,bb;
    if(x<win_center_x && x>=-win_center_x && y<win_center_y && y>=-win_center_y) {
        c=((int*)videobuffer)[x+win_center_x+win_width*(y+win_center_y)];
        rr=(c>>16)&0xFF;
        gg=(c>>8)&0xFF;
        bb=c&0xFF;
        rr+=r;
        gg+=g;
        bb+=b;
        rr>>=1;
        gg>>=1;
        bb>>=1;
        ((int*)videobuffer)[x+win_center_x+win_width*(y+win_center_y)]=(rr<<16)+(gg<<8)+bb;
    }
}
void plotmouse(int x,int y){
    plot(x-5,y,0xA0A0A0);
    plot(x+5,y,0xA0A0A0);
    plot(x,y-5,0xA0A0A0);
    plot(x,y+5,0xA0A0A0);
    plot(x-4,y,0xB0B0B0);
    plot(x+4,y,0xB0B0B0);
    plot(x,y-4,0xB0B0B0);
    plot(x,y+4,0xB0B0B0);
    plot(x-3,y,0xC0C0C0);
    plot(x+3,y,0xC0C0C0);
    plot(x,y-3,0xC0C0C0);
    plot(x,y+3,0xC0C0C0);
    plot(x-2,y,0xD0D0D0);
    plot(x+2,y,0xD0D0D0);
    plot(x,y-2,0xD0D0D0);
    plot(x,y+2,0xD0D0D0);
    plot(x-1,y,0xE0E0E0);
    plot(x+1,y,0xE0E0E0);
    plot(x,y-1,0xE0E0E0);
    plot(x,y+1,0xE0E0E0);
}
void plotboule(int x,int y) {
    plot(x,y-1,0xA0A020);
    plot(x+1,y-1,0x909020);
    plot(x-1,y,0xA0A020);
    plot(x,y,0xF0F020);
    plot(x+1,y,0xB0B020);
    plot(x+2,y,0x808020);
    plot(x-1,y+1,0x909020);
    plot(x,y+1,0xB0B020);
    plot(x+1,y+1,0xA0A020);
    plot(x+2,y+1,0x909020);
    plot(x,y+2,0x808020);
    plot(x+1,y+2,0x606020);
}
void plotcursor(int x,int y) {
    static float a=0, ar=0;
    float r=8*sin(ar);
    float c=r*cos(a);
    float s=r*sin(a);
    plotboule(x+c-win_center_x,y+s-win_center_y);
    plotboule(x+s-win_center_x,y-c-win_center_y);
    plotboule(x-c-win_center_x,y-s-win_center_y);
    plotboule(x-s-win_center_x,y+c-win_center_y);
    a+=.31; ar+=.2;
}
void cercle(int x, int y, int radius, int c) {
    int balance=-radius, xoff=0, yoff=radius;
    do {
        plot(x+xoff, y+yoff, c);
        plot(x-xoff, y+yoff, c);
        plot(x-xoff, y-yoff, c);
        plot(x+xoff, y-yoff, c);
        plot(x+yoff, y+xoff, c);
        plot(x-yoff, y+xoff, c);
        plot(x-yoff, y-xoff, c);
        plot(x+yoff, y-xoff, c);

        if ((balance += xoff + xoff + 1) >= 0) {
            yoff --;
            balance -= yoff + yoff;
        }

    } while (++xoff <= yoff);
}

extern inline int color_of_pixel(struct pixel c);
extern inline struct pixel32 pixel32_of_pixel(struct pixel c);

bool polyflat(struct vect2d *p1, struct vect2d *p2, struct vect2d *p3, struct pixel coul) {
    struct vect2d *tmp;
    int xi, yi, lx, i, j, jlim, yfin;
    int q1, q2, q3, ql, qx, qx2 = 0, ql2 = 0;
    struct pixel32 *vid;

    if (p2->y<p1->y) { tmp=p1; p1=p2; p2=tmp; }
    if (p3->y<p1->y) { tmp=p1; p1=p3; p3=tmp; }
    if (p3->y<p2->y) { tmp=p2; p2=p3; p3=tmp; }
    if (p1->y==p2->y && p1->x>p2->x) { tmp=p1; p1=p2; p2=tmp; }
    if (p3->y<0 || p1->y>=win_height) return false;

//  if (p3->y==p2->y) p3->y++;
//  if (p1->y==p2->y) p1->y--;

    yi=p1->y;

    if (p3->y==p1->y) {
        if (p1->x>p3->x) { tmp=p1; p1=p3; p3=tmp; }
        if (p2->x<p3->x) { tmp=p2; p2=p3; p3=tmp; }
        xi=p1->x<<vf;
        lx = (p2->x - p1->x +1)<<vf;
        yfin = yi+1;
        goto debtrace;
    }
    lx = 1<<vf;
    xi=p1->x<<vf;

    q1=((p3->x-p1->x)<<vf)/(p3->y-p1->y);   // et le cas p3y=p1y ?? maintenant il faut le traiter à part !
    if (p1->y!=p2->y) {
        q2=((p2->x-p1->x)<<vf)/(p2->y-p1->y);
    } else {    // on a forcément p1x<p2x
        q2 = MAXINT;
        lx = (p2->x-p1->x+1)<<vf;
    }
    if (p3->y!=p2->y) {
        q3=((p3->x-p2->x)<<vf)/(p3->y-p2->y);
    } else {
        q3=MAXINT;  // ou -MAXINT, mais comme on s'en sert pas..
    }
    if (q1<=q2) {
        ql = (q2-q1)|1;     // le taux d'accroissement de la taille du segment (en évitant 0);
        ql2= q3-q1;
        qx=qx2=q1;
    } else {
        ql = (q1-q2)|1;
        ql2= q1-q3;
        qx=q2; qx2=q3;
    }
    // clipper les y<0 ! sinon ca fait des pauses !
    yfin=p2->y;
    if (p2->y<0) {
        xi+=(p2->y-p1->y)*qx;
        yi=p2->y;
        lx+=(p2->y-p1->y)*ql;
        yfin=p3->y; ql=ql2; qx=qx2;
    }
    if (yi<0) {
        xi-=yi*qx;
        lx-=yi*ql;
        yi=0;
    }
debtrace:
    vid=videobuffer+(yi)*win_width;

    for (i=0; i<2; i++, yfin=p3->y, ql=ql2, qx=qx2){
        while (yi<yfin && yi<win_height) {
            jlim = (lx+xi)>>vf;
            j = xi>>vf;
            if (j < 0) j = 0;
            if (jlim > win_width) jlim = win_width;
            if (j < jlim) {
                memset32((int*)(vid+j), color_of_pixel(coul), jlim-j);
            }
            xi += qx;
            yi++;
            lx += ql;
            vid += win_width;
        }
    }
    return true;
}
void drawline(struct vect2d const *restrict p1, struct vect2d const *restrict p2, int col) {
    int s, x,y,xi, dy;
    struct vect2d const *tmp;
    int q;
    if (p1->x > p2->x) { tmp=p1; p1=p2; p2=tmp; }
    if ((dy = p2->y - p1->y) > 0) {
        s = 1;
        q = ((p2->x - p1->x)<<vf)/(1+dy);
    } else {
        dy = -dy;
        s = -1;
        q = ((p2->x - p1->x)<<vf)/(1+dy);
    }
    // FIXME: clipping
    if (p2->x - p1->x > win_width || dy > win_height) return;
    x = p1->x<<vf;
    for (y = p1->y; dy >= 0; dy--, y += s) {
        for (xi = x>>vf; xi < 1+((x+q)>>vf); xi++) {
            plot(xi - win_center_x, y - win_center_y, col);
        }
        x += q;
    }
}

void draw_rectangle(struct vect2d const *restrict min, struct vect2d const *restrict max, int col)
{
    struct vect2d const p1 = { .x = min->x, .y = max->y };
    struct vect2d const p2 = { .x = max->x, .y = min->y };
    drawline(min, &p1, col);
    drawline(min, &p2, col);
    drawline(max, &p1, col);
    drawline(max, &p2, col);
}

void drawline2(struct vect2d *p1, struct vect2d *p2, int col) {
    int s, x,y,xi, dy;
    struct vect2d *tmp;
    int q;
    if (p1->x>p2->x) { tmp=p1; p1=p2; p2=tmp; }
    if ((dy=(p2->y-p1->y))>0) {
        s=1;
        q = ((p2->x-p1->x)<<vf)/(1+dy);
    } else {
        dy=-dy;
        s=-1;
        q = ((p2->x-p1->x)<<vf)/(1+dy);
    }
    x = (p1->x)<<vf;
    for (y=p1->y; dy>=0; dy--, y+=s) {
        for (xi=x>>vf; xi<1+((x+q)>>vf); xi++) {
            mapping[xi+MAP_MARGIN+((y+MAP_MARGIN)<<8)]=col+0x0F0F0F;
            mapping[xi+1+MAP_MARGIN+((y+MAP_MARGIN)<<8)]=col;
            mapping[xi+1+MAP_MARGIN+((y+1+MAP_MARGIN)<<8)]=col;
            mapping[xi+MAP_MARGIN+((y+1+MAP_MARGIN)<<8)]=col+0x0F0F0F;
        }
        x+=q;
    }
}
void drawlinetb(struct vect2d *p1, struct vect2d *p2, int col) {
    int s, x,y,xi, dy;
    struct vect2d *tmp;
    int q;
    if (p1->x>p2->x) { tmp=p1; p1=p2; p2=tmp; }
    if ((dy=(p2->y-p1->y))>0) {
        s=1;
        q = ((p2->x-p1->x)<<vf)/(1+dy);
    } else {
        dy=-dy;
        s=-1;
        q = ((p2->x-p1->x)<<vf)/(1+dy);
    }
    x = (p1->x)<<vf;
    for (y=p1->y; dy>=0; dy--, y+=s) {
        for (xi=x>>vf; xi<1+((x+q)>>vf); xi++) *((int*)tbback+xi+MAP_MARGIN+(y+MAP_MARGIN)*pannel_height)=col;
        x+=q;
    }
}
void calcposrigide(int o) {
    mulmv(&obj[obj[o].objref].rot,&mod[obj[o].model].offset,&obj[o].pos);
    addv(&obj[o].pos,&obj[obj[o].objref].pos);
    copym(&obj[o].rot,&obj[obj[o].objref].rot);
    obj_check_pos(o);
}
void calcposarti(int o, struct matrix *m) {
    mulmv(&obj[obj[o].objref].rot,&mod[obj[o].model].offset,&obj[o].pos);
    addv(&obj[o].pos,&obj[obj[o].objref].pos);
    mulm3(&obj[o].rot,&obj[obj[o].objref].rot,m);
    obj_check_pos(o);
}
void calcposaind(int i) {
    int xk,yk,ak;
    xk=(int)floor(obj[i].pos.x/TILE_LEN)+(MAP_LEN>>1);
    yk=(int)floor(obj[i].pos.y/TILE_LEN)+(MAP_LEN>>1);
    if (xk<0 || xk>=MAP_LEN || yk<0 || yk>=MAP_LEN) {
        printf("HS!\n"); exit(-1);}
    ak=xk+(yk<<LOG_MAP_LEN);
    if (ak!=obj[i].ak) {
        if (obj[i].next!=-1) obj[obj[i].next].prec=obj[i].prec;
        if (obj[i].prec!=-1) obj[obj[i].prec].next=obj[i].next;
        else map[obj[i].ak].first_obj=obj[i].next;
        obj[i].next=map[ak].first_obj;
        if (map[ak].first_obj!=-1) obj[map[ak].first_obj].prec=i;
        obj[i].prec=-1;
        map[ak].first_obj=i;
        obj[i].ak=ak;
    }
}
void calcposa(void)
{
    // calcule les pos et les rot absolues des objets du monde réel
    int xk, yk, ak;
    for (int i=0; i<nb_obj; i++) {
        if (!mod[obj[i].model].fix || !mod[obj[i].model].anchored) {
            xk=(int)floor(obj[i].pos.x/TILE_LEN)+(MAP_LEN>>1);
            yk=(int)floor(obj[i].pos.y/TILE_LEN)+(MAP_LEN>>1);
            assert(xk>=0 && xk<MAP_LEN && yk>=0 && yk<MAP_LEN);
            ak=xk+(yk<<LOG_MAP_LEN);
            if (ak!=obj[i].ak) {
                if (obj[i].next!=-1) obj[obj[i].next].prec=obj[i].prec;
                if (obj[i].prec!=-1) obj[obj[i].prec].next=obj[i].next;
                else map[obj[i].ak].first_obj=obj[i].next;
                obj[i].next=map[ak].first_obj;
                if (map[ak].first_obj!=-1) obj[map[ak].first_obj].prec=i;
                obj[i].prec=-1;
                map[ak].first_obj=i;
                obj[i].ak=ak;
            }
        }
    }
}

int zfac;

static void phplot(int x, int y, int r) {
    int ix=0,iy=zfac*r;
    int balance=-r, xoff=0, yoff=r, newyoff=1;
    if (r<=0 || y<0 || y>=win_height) return;
    do {
        int lum = ((iy&0xff00)<<8) | (iy&0xff00);
        if (x+xoff>=0 && x+xoff<win_width) MMXAddSatC((int*)videobuffer+x+xoff+y*win_width, lum);
        if (xoff && x-xoff>=0 && x-xoff<win_width) MMXAddSatC((int*)videobuffer+x-xoff+y*win_width, lum);
        if (newyoff && xoff!=yoff) {
            if (x+yoff>=0 && x+yoff<win_width) MMXAddSatC((int*)videobuffer+x+yoff+y*win_width, lum);
            if (x-yoff>=0 && x-yoff<win_width) MMXAddSatC((int*)videobuffer+x-yoff+y*win_width, lum);
        }
        if ((balance += xoff + xoff + 1) >= 0) {
            yoff--;
            balance -= yoff + yoff;
            newyoff=1;
            iy-=zfac;
        } else newyoff=0;
        ix+=zfac;
    } while (++xoff <= yoff);
}
void plotphare(int x, int y, int r) {
    int balance=-r, xoff=0, yoff=r, newyoff=1;
    if (r==0 || x-r>=win_width || x+r<0 || y-r>=win_height || y+r<0 || r>win_width) return;
    zfac=(190<<8)/r;
    do {
        if (newyoff) {
            phplot(x,y+yoff, xoff);
            phplot(x,y-yoff, xoff);
        }
        if (xoff!=yoff) {
            phplot(x,y+xoff, yoff);
            if (xoff) phplot(x,y-xoff, yoff);
        }
        if ((balance += xoff + xoff) >= 0) {
            yoff --;
            balance -= yoff + yoff;
            newyoff=1;
        } else newyoff=0;
        xoff++;
    } while (xoff <= yoff);
}

static void nuplot(int x, int y, int r) {
    int ix=0,iy=zfac*r;
    int balance=-r, xoff=0, yoff=r, newyoff=1;
    if (r<=0 || y<0 || y>=win_height) return;
    do {
        if (x+xoff>=0 && x+xoff<win_width) MMXAddSat((int*)videobuffer+x+xoff+y*win_width,iy>>8);
        if (xoff && x-xoff>=0 && x-xoff<win_width) MMXAddSat((int*)videobuffer+x-xoff+y*win_width,iy>>8);
        if (newyoff && xoff!=yoff) {
            if (x+yoff>=0 && x+yoff<win_width) MMXAddSat((int*)videobuffer+x+yoff+y*win_width,ix>>8);
            if (x-yoff>=0 && x-yoff<win_width) MMXAddSat((int*)videobuffer+x-yoff+y*win_width,ix>>8);
        }
        if ((balance += xoff + xoff) >= 0) {
            --yoff;
            balance -= yoff + yoff;
            newyoff=1;
            iy-=zfac;
        } else newyoff=0;
        xoff++;
        ix+=zfac;
    } while (xoff <= yoff);
}

void plotnuage(int x, int y, int r) {
    int balance=-r, xoff=0, yoff=r, newyoff=1;
    if (r==0 || x-r>=win_width || x+r<0 || y-r>=win_height || y+r<0 || r>win_width) return;
    zfac=(90<<8)/r;
    do {
        if (newyoff) {
            nuplot(x,y+yoff, xoff);
            nuplot(x,y-yoff, xoff);
        }
        if (xoff!=yoff) {
            nuplot(x,y+xoff, yoff);
            if (xoff) nuplot(x,y-xoff, yoff);
        }
        if ((balance += xoff + xoff + 1) >= 0) {
            yoff --;
            balance -= yoff + yoff;
            newyoff=1;
        } else newyoff=0;
    } while (++xoff <= yoff);
}

static void fuplot(int x, int y, int r) {
    int ix=0,iy=zfac*r;
    int balance=-r, xoff=0, yoff=r, newyoff=1;
    if (r<=0 || y<0 || y>=win_height) return;
    do {
        if (x+xoff>=0 && x+xoff<win_width) MMXSubSat((int*)videobuffer+x+xoff+y*win_width,iy>>8);
        if (xoff && x-xoff>=0 && x-xoff<win_width) MMXSubSat((int*)videobuffer+x-xoff+y*win_width,iy>>8);
        if (newyoff && xoff!=yoff) {
            if (x+yoff>=0 && x+yoff<win_width) MMXSubSat((int*)videobuffer+x+yoff+y*win_width,ix>>8);
            if (x-yoff>=0 && x-yoff<win_width) MMXSubSat((int*)videobuffer+x-yoff+y*win_width,ix>>8);
        }
        if ((balance += xoff + xoff + 1) >= 0) {
            yoff--;
            balance -= yoff + yoff;
            newyoff=1;
            iy-=zfac;
        } else newyoff=0;
        ix+=zfac;
    } while (++xoff <= yoff);
}
void plotfumee(int x, int y, int r) {
    int balance=-r, xoff=0, yoff=r, newyoff=1;
    if (r==0 || x-r>=win_width || x+r<0 || y-r>=win_height || y+r<0 || r>win_width) return;
    zfac=(40<<8)/r;
    do {
        if (newyoff) {
            fuplot(x,y+yoff, xoff);
            fuplot(x,y-yoff, xoff);
        }
        if (xoff!=yoff) {
            fuplot(x,y+xoff, yoff);
            if (xoff) fuplot(x,y-xoff, yoff);
        }
        if ((balance += xoff + xoff + 1) >= 0) {
            yoff --;
            balance -= yoff + yoff;
            newyoff=1;
        } else newyoff=0;
    } while (++xoff <= yoff);
}

static void darken(uchar *b)
{
    *b = *b - ((*b>>2) & 0x3F);
}

void renderer(int ak, enum render_part fast) {
    int o, p, no;
    struct vector c,t,pts3d;
    double rayonapparent=0;
    struct matrix co;
    struct vect2d e;
    //obj[0]=light
    if (map[ak].first_obj==-1) return;
    // boucler sur tous les objets
    for (o=map[ak].first_obj; o!=-1; o=obj[o].next) {
        if ((fast==1 && obj[o].type!=TYPE_CLOUD) || fast==2) continue;
        // calcul la position de l'objet dans le repère de la caméra, ie CamT*(objpos-campos)
//      if (obj[o].model==0) continue;  // une roue effacée
//      copyv(&obj[o].t,&obj[o].pos);
        subv3(&obj[o].pos,&obj[0].pos,&obj[o].t);
        mulmtv(&obj[0].rot,&obj[o].t,&obj[o].posc);
        obj[o].distance = norme2(&obj[o].posc);
    }
    // reclasser les objets en R2
    if (fast==0 || fast==3) {   // 1-> pas la peine et 2-> deja fait.
        o=map[ak].first_obj;
        if (obj[o].next!=-1) do {   // le if est utile ssi il n'y a qu'un seul objet
            int n=obj[o].next, ii=o;
            //for (p=0; p!=-1; p=obj[p].next) printf("%d<",p);
            //printf("\n");
            while (ii!=-1 && obj[n].distance<obj[ii].distance) ii=obj[ii].prec;
            if (ii!=o) { // réinsère
                obj[o].next=obj[n].next;
                if (obj[o].next!=-1) obj[obj[o].next].prec=o;
                obj[n].prec=ii;
                if (ii==-1) {
                    obj[n].next=map[ak].first_obj;
                    map[ak].first_obj=n;
                    obj[obj[n].next].prec=n;
                } else {
                    obj[n].next=obj[ii].next;
                    obj[ii].next=n;
                    obj[obj[n].next].prec=n;
                }
            } else o=obj[o].next;
        } while (o!=-1 && obj[o].next!=-1);
    }
    // affichage des ombres
    o=map[ak].first_obj; no=0;
    if (fast!=1) do {
        float z;
        char aff=norme2(&obj[o].t)<TILE_LEN*TILE_LEN*4;
        if (obj[o].aff && (fast==3 || (fast==0 && (obj[o].type==TYPE_CAR || obj[o].type==TYPE_TANK || obj[o].type==TYPE_LIGHT || obj[o].type==TYPE_DECO || obj[o].type==TYPE_DEBRIS)) || (fast==2 && (obj[o].type==TYPE_PLANE || obj[o].type==TYPE_ZEPPELIN || obj[o].type==TYPE_INSTRUMENTS || obj[o].type==TYPE_BOMB)))) {
            mulmt3(&oL[no], &obj[o].rot, &light);
#define DISTLUM 300.
            mulv(&oL[no].x,DISTLUM);
            mulv(&oL[no].y,DISTLUM);
            if (aff && (z=z_ground(obj[o].pos.x,obj[o].pos.y, true))>obj[o].pos.z-500) {
                for (p=0; p<mod[obj[o].model].nbpts[1]; p++) {
                    mulmv(&obj[o].rot, &mod[obj[o].model].pts[1][p], &pts3d);
                    addv(&pts3d,&obj[o].pos);
                    pts3d.x+=pts3d.z-z;
                    pts3d.z=z;
                    subv(&pts3d,&obj[0].pos);
                    mulmtv(&obj[0].rot,&pts3d,&pts3d);
                    if (pts3d.z >0) proj(&pts2d[p].v,&pts3d);
                    else pts2d[p].v.x = MAXINT;
                }
                for (p=0; p<mod[obj[o].model].nbfaces[1]; p++) {
                    if (scalaire(&mod[obj[o].model].fac[1][p].norm,&oL[no].z)<=0 &&
                            pts2d[mod[obj[o].model].fac[1][p].p[0]].v.x != MAXINT &&
                            pts2d[mod[obj[o].model].fac[1][p].p[1]].v.x != MAXINT &&
                            pts2d[mod[obj[o].model].fac[1][p].p[2]].v.x != MAXINT)
                        polyflat(
                                &pts2d[mod[obj[o].model].fac[1][p].p[0]].v,
                                &pts2d[mod[obj[o].model].fac[1][p].p[1]].v,
                                &pts2d[mod[obj[o].model].fac[1][p].p[2]].v,
                                (struct pixel){ .r = 0, .g = 0, .b = 0});
                }
            }
        }
        o=obj[o].next; no++;
    } while (o!=-1);
    if (no>MAXNO) printf("ERROR ! NO>MAXNO AT ak=%d (no=%d)\n",ak,no);
    // affichage dans l'ordre du Z
    o=map[ak].first_obj; no=0;
    if (fast!=1) while (obj[o].next!=-1 /*&& (viewall || obj[obj[o].next].distance<TL2)*/) { o=obj[o].next; no++; }
    do {
        if (
            fast==3 ||
            (fast==1 && obj[o].type==TYPE_CLOUD) ||
            (fast==0 && (obj[o].type==TYPE_CAR || obj[o].type==TYPE_TANK || obj[o].type==TYPE_LIGHT || obj[o].type==TYPE_DECO || obj[o].type==TYPE_DEBRIS)) ||
            (fast==2 && (obj[o].type==TYPE_PLANE || obj[o].type==TYPE_ZEPPELIN || obj[o].type==TYPE_SMOKE || obj[o].type==TYPE_SHOT || obj[o].type==TYPE_BOMB || obj[o].type==TYPE_INSTRUMENTS || obj[o].type==TYPE_CLOUD))
        ) {
            if (obj[o].aff && obj[o].posc.z>-mod[obj[o].model].rayon) { // il faut déjà que l'objet soit un peu devant la caméra et que ce soit pas un objet à passer...
                int visu;
                if (obj[o].posc.z > 0) {
                    // on va projetter ce centre à l'écran
                    proj(&e,&obj[o].posc);
                    rayonapparent = proj1(mod[obj[o].model].rayon,obj[o].posc.z);
                    visu = e.x>-rayonapparent && e.x<win_width+rayonapparent && e.y>-rayonapparent && e.y<win_height+rayonapparent;
                } else {    // verifier la formule qd meme...
                    if (obj[o].type != TYPE_CLOUD && obj[o].type != TYPE_SMOKE && obj[o].type != TYPE_LIGHT) {
                        double r = mod[obj[o].model].rayon*sqrt(z_near*z_near+win_center_x*win_center_x)/win_center_x;
                        visu = obj[o].posc.z > z_near*fabs(obj[o].posc.x)/win_center_x - r;
                        r = mod[obj[o].model].rayon*sqrt(z_near*z_near+win_center_y*win_center_y)/win_center_y;
                        visu = visu && (obj[o].posc.z > z_near*fabs(obj[o].posc.y)/win_center_y - r);
                        rayonapparent = win_width;
                    } else visu=0;
                }
                // la sphère est-elle visible ?
                if (visu) {
                    if (obj[o].type == TYPE_CLOUD) {
                        if (night_mode) plotfumee(e.x,e.y,rayonapparent);
                        else plotnuage(e.x,e.y,rayonapparent);
                    }
                    else if (obj[o].type == TYPE_SMOKE) {
                        if (smoke_radius[o-smoke_start] > 0.) {
                            plotfumee(e.x, e.y, (int)(rayonapparent*smoke_radius[o-smoke_start]) >> 9);
                        }
                    } else {
                        if (rayonapparent>.3) {
                            if (rayonapparent<.5) plot(e.x-win_center_x,e.y-win_center_y,0x0);
                            else {
                                int mo = obj[o].distance<(TILE_LEN*TILE_LEN*.14) ? 0 : 1;
                                // on calcule alors la pos de la obj[0] dans le repère de l'objet, ie ObjT*(campos-objpos)
                                mulmtv(&obj[o].rot,&obj[o].t,&c);
                                neg(&c);
                                // on calcule aussi la position de tous les points de l'objet dans le repere de la camera, ie CamT*Obj*u
                                mulmt3(&co,&obj[0].rot,&obj[o].rot);
                                for (p=0; p<mod[obj[o].model].nbpts[mo]; p++) {
                                    mulmv(&co, &mod[obj[o].model].pts[mo][p], &pts3d);
                                    addv(&pts3d,&obj[o].posc);
                                    if (pts3d.z>0) proj(&pts2d[p].v,&pts3d);
                                    else pts2d[p].v.x = MAXINT;
                                    // on calcule aussi les projs des
                                    // norms dans le plan lumineux infiniment éloigné
                                    if (scalaire(&mod[obj[o].model].norm[mo][p],&oL[no].z)<0) {
                                        pts2d[p].xl = scalaire(&mod[obj[o].model].norm[mo][p],&oL[no].x);
                                        pts2d[p].yl = scalaire(&mod[obj[o].model].norm[mo][p],&oL[no].y);
                                    } else pts2d[p].xl = MAXINT;
                                }
                                if (obj[o].type==TYPE_SHOT) {
                                    if (pts2d[0].v.x!=MAXINT && pts2d[1].v.x!=MAXINT) drawline(&pts2d[0].v, &pts2d[1].v, 0xFFA0F0);
                                } else {
                                    for (p=0; p<mod[obj[o].model].nbfaces[mo]; p++) {
                                        // test de visibilité entre obj[0] et normale
                                        copyv(&t,&mod[obj[o].model].pts[mo][mod[obj[o].model].fac[mo][p].p[0]]);
                                        subv(&t,&c);
                                        if (scalaire(&t,&mod[obj[o].model].fac[mo][p].norm)<=0) {
                                            if (pts2d[mod[obj[o].model].fac[mo][p].p[0]].v.x != MAXINT &&
                                                    pts2d[mod[obj[o].model].fac[mo][p].p[1]].v.x != MAXINT &&
                                                    pts2d[mod[obj[o].model].fac[mo][p].p[2]].v.x != MAXINT) {
                                                if (obj[o].type==TYPE_INSTRUMENTS && p>=mod[obj[o].model].nbfaces[mo]-2) {
                                                    struct vect2dm pt[3];
                                                    int i;
                                                    for (i=0; i<3; i++) {
                                                        pt[i].v.x=pts2d[mod[obj[o].model].fac[mo][p].p[i]].v.x;
                                                        pt[i].v.y=pts2d[mod[obj[o].model].fac[mo][p].p[i]].v.y;
                                                    }
                                                    if (p-(mod[obj[o].model].nbfaces[mo]-2)) {
                                                        pt[2].mx=MAP_MARGIN;
                                                        pt[2].my=pannel_width+MAP_MARGIN;
                                                        pt[0].mx=pannel_height+MAP_MARGIN;
                                                        pt[0].my=MAP_MARGIN;
                                                        pt[1].mx=pannel_height+MAP_MARGIN;
                                                        pt[1].my=pannel_width+MAP_MARGIN;
                                                    } else {
                                                        pt[0].mx=MAP_MARGIN;
                                                        pt[0].my=pannel_width+MAP_MARGIN;
                                                        pt[1].mx=MAP_MARGIN;
                                                        pt[1].my=MAP_MARGIN;
                                                        pt[2].mx=pannel_height+MAP_MARGIN;
                                                        pt[2].my=MAP_MARGIN;
                                                    }
                                                    polymap(&pt[0],&pt[1],&pt[2]);
                                                } else {
                                                    struct pixel coul = mod[obj[o].model].fac[mo][p].color;
                                                    if (night_mode) {
                                                        if (obj[o].type != TYPE_INSTRUMENTS) {
                                                            darken(&coul.r);
                                                        }
                                                        darken(&coul.g);
                                                        darken(&coul.b);
                                                    }
                                                    if (pts2d[mod[obj[o].model].fac[mo][p].p[0]].xl!=MAXINT && pts2d[mod[obj[o].model].fac[mo][p].p[1]].xl!=MAXINT && pts2d[mod[obj[o].model].fac[mo][p].p[2]].xl!=MAXINT)
                                                        polyphong(
                                                                &pts2d[mod[obj[o].model].fac[mo][p].p[0]],
                                                                &pts2d[mod[obj[o].model].fac[mo][p].p[1]],
                                                                &pts2d[mod[obj[o].model].fac[mo][p].p[2]],
                                                                coul);
                                                    else
                                                        polyflat(
                                                                &pts2d[mod[obj[o].model].fac[mo][p].p[0]].v,
                                                                &pts2d[mod[obj[o].model].fac[mo][p].p[1]].v,
                                                                &pts2d[mod[obj[o].model].fac[mo][p].p[2]].v,
                                                                coul);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (night_mode && obj[o].type==TYPE_LIGHT) {   // HALO
                        plotphare(e.x,e.y,rayonapparent*4+1);
                    }
                }
            }
        }
        if (fast!=1) { o=obj[o].prec; no--; } else o=obj[o].next;
    } while (o!=-1);
}

static bool to_camera(struct vector const *v3d, struct vect2d *v2d)
{
    struct vector vc = *v3d;
    struct vector vc_c;
    subv(&vc, &obj[0].pos);
    mulmtv(&obj[0].rot, &vc, &vc_c);
    if (vc_c.z > 0.) {
        proj(v2d, &vc_c);
        return true;
    }
    return false;
}

#ifdef VEC_DEBUG
struct vector debug_vector[NB_DBG_VECS][2];
void draw_debug(void)
{
    static int debug_vector_color[] = {
        0xFF0000, 0x00FF00, 0x0000FF,
        0xC0C000, 0xC000C0, 0x00C0C0,
    };

    for (unsigned v = 0; v < ARRAY_LEN(debug_vector); v++) {
        struct vect2d pts2d[2];
        if (to_camera(debug_vector[v]+0, pts2d+0) && to_camera(debug_vector[v]+1, pts2d+1)) {
            drawline(pts2d+0, pts2d+1, debug_vector_color[v % ARRAY_LEN(debug_vector_color)]);
        }
    }
}
#endif

void draw_target(struct vector p, int c)
{
    struct vect2d p2d;
    if (! to_camera(&p, &p2d)) return;
    struct vect2d const min = { .x = p2d.x - 10, .y = p2d.y - 10 };
    struct vect2d const max = { .x = p2d.x + 10, .y = p2d.y + 10 };
    draw_rectangle(&min, &max, c);
}

void draw_mark(struct vector p, int c)
{
    // TODO: a cross on the floor
    draw_target(p, c);
}
