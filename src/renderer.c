#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <values.h>
#include "map.h"

#define cam obj[0]
vect2dlum *pts2d;
matrix *oL;
void initrender() {
	double r;
	int a;
	preca = calloc(8*(256+2+256),1);
	for (r=0, a=0; a<256+2; a++, r+=1) {
		preca[8*a]=preca[8*a+2]=preca[8*a+4]=preca[8*a+6]=255.*exp(-r/50.);
		preca[8*a+1]=preca[8*a+3]=preca[8*a+5]=preca[8*a+7]=0;
	}
	pts2d=(vect2dlum*)malloc(3000*sizeof(vect2dlum));	// nbpts max par objets
#define MAXNO 5000
	oL=(matrix*)malloc(MAXNO*sizeof(matrix));	// nb objet max dans un ak
}
void plot(int x, int y, int r){
	if(x<_DX && x>=-_DX && y<_DY && y>=-_DY) {
		((int*)videobuffer)[x+_DX+SX*(y+_DY)]=r;
	}
}
void mixplot(int x, int y, int r, int g, int b){
	int c;
	int rr,gg,bb;
	if(x<_DX && x>=-_DX && y<_DY && y>=-_DY) {
		c=((int*)videobuffer)[x+_DX+SX*(y+_DY)];
		rr=(c>>16)&0xFF;
		gg=(c>>8)&0xFF;
		bb=c&0xFF;
		rr+=r;
		gg+=g;
		bb+=b;
		rr>>=1;
		gg>>=1;
		bb>>=1;
		((int*)videobuffer)[x+_DX+SX*(y+_DY)]=(rr<<16)+(gg<<8)+bb;
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
	plotboule(x+c-_DX,y+s-_DY);
	plotboule(x+s-_DX,y-c-_DY);
	plotboule(x-c-_DX,y-s-_DY);
	plotboule(x-s-_DX,y+c-_DY);
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
void polyflat(vect2d *p1, vect2d *p2, vect2d *p3, int coul) {
	vect2d *tmp;
	int xi, yi, lx, i, j, jlim, yfin;
	int q1, q2, q3, ql, qx, qx2, ql2;
	pixel32 *vid;
	
	if (p2->y<p1->y) { tmp=p1; p1=p2; p2=tmp; }
	if (p3->y<p1->y) { tmp=p1; p1=p3; p3=tmp; }
	if (p3->y<p2->y) { tmp=p2; p2=p3; p3=tmp; }
	if (p1->y==p2->y && p1->x>p2->x) { tmp=p1; p1=p2; p2=tmp; }
	if (p3->y<0 || p1->y>=SY) return;
	
//	if (p3->y==p2->y) p3->y++;
//	if (p1->y==p2->y) p1->y--;

	yi=p1->y;

	if (p3->y==p1->y) {
		if (p1->x>p3->x) { tmp=p1; p1=p3; p3=tmp; }
		if (p2->x<p3->x) { tmp=p2; p2=p3; p3=tmp; }
		xi=p1->x<<vf;
		lx = (p2->x - p1->x +1)<<vf;
		yfin = yi+1;
		MMXFlatInit();
		goto debtrace;
	}
	lx = 1<<vf;
	xi=p1->x<<vf;
	
	q1=((p3->x-p1->x)<<vf)/(p3->y-p1->y);	// et le cas p3y=p1y ?? maintenant il faut le traiter à part !
	if (p1->y!=p2->y) {
		q2=((p2->x-p1->x)<<vf)/(p2->y-p1->y);
	} else {	// on a forcément p1x<p2x
		q2 = MAXINT;
		lx = (p2->x-p1->x+1)<<vf;	
	}
	if (p3->y!=p2->y) {
		q3=((p3->x-p2->x)<<vf)/(p3->y-p2->y);
	} else {
		q3=MAXINT;	// ou -MAXINT, mais comme on s'en sert pas..
	}
	if (q1<=q2) {
		ql = (q2-q1)|1;		// le taux d'accroissement de la taille du segment (en évitant 0);
		ql2= q3-q1;
		qx=qx2=q1;
	} else {
		ql = (q1-q2)|1;
		ql2= q1-q3;
		qx=q2; qx2=q3;
	}
	MMXFlatInit();
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
	vid=videobuffer+(yi)*SX;
	
	for (i=0; i<2; i++, yfin=p3->y, ql=ql2, qx=qx2){
		while (yi<yfin && yi<SY) {
			jlim=(lx+xi)>>vf; j=xi>>vf;
			if (j<0) j=0;
			if (jlim>SX) jlim=SX;
			if (j<jlim) {
				MMXFlat((int*)vid+j,jlim-j,coul);
			}
			xi+=qx; yi++;
			lx+=ql;
			vid+=SX;
		}
	}
	MMXRestoreFPU();
}
void drawline(vect2dlum *p1, vect2dlum *p2, int col) {
	int s, x,y,xi, dy;
	vect2dlum *tmp;
	int q;
	if (p1->v.x>p2->v.x) { tmp=p1; p1=p2; p2=tmp; }
	if ((dy=(p2->v.y-p1->v.y))>0) {
		s=1;
		q = ((p2->v.x-p1->v.x)<<vf)/(1+dy);
	} else {
		dy=-dy;
		s=-1;
		q = ((p2->v.x-p1->v.x)<<vf)/(1+dy);
	}
	x = (p1->v.x)<<vf;
	for (y=p1->v.y; dy>=0; dy--, y+=s) {
		for (xi=x>>vf; xi<1+((x+q)>>vf); xi++) plot(xi-_DX,y-_DY,col);
		x+=q;
	}
}
void drawline2(vect2d *p1, vect2d *p2, int col) {
	int s, x,y,xi, dy;
	vect2d *tmp;
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
			mapping[xi+MARGE+((y+MARGE)<<8)]=col+0x0F0F0F;
			mapping[xi+1+MARGE+((y+MARGE)<<8)]=col;
			mapping[xi+1+MARGE+((y+1+MARGE)<<8)]=col;
			mapping[xi+MARGE+((y+1+MARGE)<<8)]=col+0x0F0F0F;
		}
		x+=q;
	}
}
void drawlinetb(vect2d *p1, vect2d *p2, int col) {
	int s, x,y,xi, dy;
	vect2d *tmp;
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
		for (xi=x>>vf; xi<1+((x+q)>>vf); xi++) *((int*)tbback+xi+MARGE+(y+MARGE)*SXTB)=col;
		x+=q;
	}
}
void calcposrigide(int o) {
	mulmv(&obj[obj[o].objref].rot,&mod[obj[o].model].offset,&obj[o].pos);
	addv(&obj[o].pos,&obj[obj[o].objref].pos);
	copym(&obj[o].rot,&obj[obj[o].objref].rot);
	controlepos(o);
}
void calcposarti(int o, matrix *m) {
	mulmv(&obj[obj[o].objref].rot,&mod[obj[o].model].offset,&obj[o].pos);
	addv(&obj[o].pos,&obj[obj[o].objref].pos);
	mulm3(&obj[o].rot,&obj[obj[o].objref].rot,m);
	controlepos(o);
}
void calcposaind(int i) {
	int xk,yk,ak;
/*	if (obj[i].objref!=-1 && obj[i].objref<i) {
		mulmv(&obj[obj[i].objref].rota,&obj[i].pos,&obj[i].posa);
		addv(&obj[i].posa,&obj[obj[i].objref].posa);	// que deux passes possibles !
		mulm3(&obj[i].rota,&obj[obj[i].objref].rota,&obj[i].rot);
	} else {
		copyv(&obj[i].posa,&obj[i].pos);
		copym(&obj[i].rota,&obj[i].rot);
	}*/
	xk=(int)floor(obj[i].pos.x/ECHELLE)+(WMAP>>1);
	yk=(int)floor(obj[i].pos.y/ECHELLE)+(WMAP>>1);
	if (xk<0 || xk>=WMAP || yk<0 || yk>=WMAP) {
		printf("HS!\n"); exit(-1);}
	ak=xk+(yk<<NWMAP);
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
void calcposa() {
	// calcule les pos et les rot absolues des objets du monde réel
	int i, xk, yk, ak;
	for (i=0; i<nbobj; i++) {
/*		if (obj[i].objref!=-1 && obj[i].objref<i) {
			mulmv(&obj[obj[i].objref].rota,&obj[i].pos,&obj[i].posa);
			addv(&obj[i].posa,&obj[obj[i].objref].posa);	// que deux passes possibles !
			if (mod[obj[i].model].fixe==1) copym(&obj[i].rota,&obj[obj[i].objref].rota);	// rigide?
			else mulm3(&obj[i].rota,&obj[obj[i].objref].rota,&obj[i].rot);	// articulé
		} else {
		//	memcpy(&obj[i].posa,&obj[i].pos,sizeof(vector)+sizeof(matrix));
		}*/
		if (mod[obj[i].model].fixe!=-1) {	// immobile ?
			xk=(int)floor(obj[i].pos.x/ECHELLE)+(WMAP>>1);
			yk=(int)floor(obj[i].pos.y/ECHELLE)+(WMAP>>1);
			if (xk<0 || xk>=WMAP || yk<0 || yk>=WMAP) {
				printf("HS!\n"); exit(-1);}
			ak=xk+(yk<<NWMAP);
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
void plotphare(int x, int y, int r) {
	(void)x; (void)y; (void)r;
#	if 0
	int balance=-r, xoff=0, yoff=r, newyoff=1;
	if (r==0 || x-r>=SX || x+r<0 || y-r>=SY || y+r<0 || r>SX) return;
	MMXSaveFPU();
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
		if ((balance += xoff++ + xoff) >= 0) {
			yoff --;
			balance -= yoff + yoff;
			newyoff=1;
		} else newyoff=0;
	} while (xoff <= yoff);
	MMXRestoreFPU();
#	endif
}

void nuplot(int x, int y, int r) {
	int ix=0,iy=zfac*r;
	int balance=-r, xoff=0, yoff=r, newyoff=1;
	if (r<=0 || y<0 || y>=SY) return;
	do {
		if (x+xoff>=0 && x+xoff<SX) MMXAddSat((int*)videobuffer+x+xoff+y*SX,iy>>8);
		if (xoff && x-xoff>=0 && x-xoff<SX) MMXAddSat((int*)videobuffer+x-xoff+y*SX,iy>>8);
		if (newyoff && xoff!=yoff) {
			if (x+yoff>=0 && x+yoff<SX) MMXAddSat((int*)videobuffer+x+yoff+y*SX,ix>>8);
			if (x-yoff>=0 && x-yoff<SX) MMXAddSat((int*)videobuffer+x-yoff+y*SX,ix>>8);
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
	if (r==0 || x-r>=SX || x+r<0 || y-r>=SY || y+r<0 || r>SX) return;
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

void fuplot(int x, int y, int r) {
	int ix=0,iy=zfac*r;
	int balance=-r, xoff=0, yoff=r, newyoff=1;
	if (r<=0 || y<0 || y>=SY) return;
	do {
		if (x+xoff>=0 && x+xoff<SX) MMXSubSat((int*)videobuffer+x+xoff+y*SX,iy>>8);
		if (xoff && x-xoff>=0 && x-xoff<SX) MMXSubSat((int*)videobuffer+x-xoff+y*SX,iy>>8);
		if (newyoff && xoff!=yoff) {
			if (x+yoff>=0 && x+yoff<SX) MMXSubSat((int*)videobuffer+x+yoff+y*SX,ix>>8);
			if (x-yoff>=0 && x-yoff<SX) MMXSubSat((int*)videobuffer+x-yoff+y*SX,ix>>8);
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
	if (r==0 || x-r>=SX || x+r<0 || y-r>=SY || y+r<0 || r>SX) return;
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
void renderer(int ak, int fast){	// fast=0->ombres+ objets au sol, =1->nuages; =2->objets volants + nuages; =3->tout
	int o, p, no, coul;
	vector c,t,pts3d;
	int mo;
	double rayonapparent=0;
	matrix co;
	vect2d e;
	//cam=light
	if (map[ak].first_obj==-1) return;
	// boucler sur tous les objets
	for (o=map[ak].first_obj; o!=-1; o=obj[o].next) {
		if ((fast==1 && obj[o].type!=NUAGE) || fast==2) continue;
		// calcul la position de l'objet dans le repère de la caméra, ie CamT*(objpos-campos)
//		if (obj[o].model==0) continue;	// une roue effacée
//		copyv(&obj[o].t,&obj[o].pos);
		subv3(&obj[o].pos,&cam.pos,&obj[o].t);
		mulmtv(&cam.rot,&obj[o].t,&obj[o].posc);
		obj[o].distance = norme2(&obj[o].posc);
	}
	// reclasser les objets en R2
	if (fast==0 || fast==3) {	// 1-> pas la peine et 2-> deja fait.
	o=map[ak].first_obj;
	if (obj[o].next!=-1)	do {	// le if est utile ssi il n'y a qu'un seul objet
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
		char aff=norme2(&obj[o].t)<ECHELLE*ECHELLE*4;
		if (obj[o].aff && (fast==3 || (fast==0 && (obj[o].type==CIBGRAT || obj[o].type==VEHIC || obj[o].type==PHARE || obj[o].type==DECO || obj[o].type==GRAV)) || (fast==2 && (obj[o].type==AVION || obj[o].type==ZEPPELIN || obj[o].type==TABBORD || obj[o].type==BOMB)))) {
			mulmt3(&oL[no], &obj[o].rot, &Light);
#define DISTLUM 300.
			mulv(&oL[no].x,DISTLUM);
			mulv(&oL[no].y,DISTLUM);
  			if (aff && (z=z_ground(obj[o].pos.x,obj[o].pos.y, true))>obj[o].pos.z-500) {
				for (p=0; p<mod[obj[o].model].nbpts[1]; p++) {
					mulmv(&obj[o].rot, &mod[obj[o].model].pts[1][p], &pts3d);
					addv(&pts3d,&obj[o].pos);
					pts3d.x+=pts3d.z-z;
					pts3d.z=z;
					subv(&pts3d,&cam.pos);
					mulmtv(&cam.rot,&pts3d,&pts3d);
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
							&pts2d[mod[obj[o].model].fac[1][p].p[2]].v, 0);
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
		if (fast==3 || (fast==1 && obj[o].type==NUAGE) || (fast==0 && (obj[o].type==CIBGRAT || obj[o].type==VEHIC || obj[o].type==PHARE || obj[o].type==DECO || obj[o].type==GRAV)) || (fast==2 && (obj[o].type==AVION || obj[o].type==ZEPPELIN || obj[o].type==FUMEE || obj[o].type==TIR || obj[o].type==BOMB || obj[o].type==TABBORD || obj[o].type==NUAGE))) {
		if (obj[o].aff && obj[o].posc.z>-mod[obj[o].model].rayon) {	// il faut déjà que l'objet soit un peu devant la caméra et que ce soit pas un objet à passer...
			int visu;
			if (obj[o].posc.z>0) {
				// on va projetter ce centre à l'écran
				proj(&e,&obj[o].posc);
				rayonapparent = proj1(mod[obj[o].model].rayon,obj[o].posc.z);
				visu = e.x>-rayonapparent && e.x<SX+rayonapparent && e.y>-rayonapparent && e.y<SY+rayonapparent;
			} else {	// verifier la formule qd meme...
				if (obj[o].type!=NUAGE && obj[o].type!=FUMEE) {
					double r = mod[obj[o].model].rayon*sqrt(focale*focale+_DX*_DX)/_DX;
					visu = obj[o].posc.z > focale*fabs(obj[o].posc.x)/_DX - r;
					r = mod[obj[o].model].rayon*sqrt(focale*focale+_DY*_DY)/_DY;
					visu = visu && (obj[o].posc.z > focale*fabs(obj[o].posc.y)/_DY - r);
					rayonapparent = SX;
				} else visu=0;
			}
			// la sphère est-elle visible ?
			if (visu) {
				if (obj[o].type==NUAGE) {
					if (Dark) plotfumee(e.x,e.y,rayonapparent);
					else plotnuage(e.x,e.y,rayonapparent);
				}
				else if (obj[o].type==FUMEE) {
					if (rayonfumee[o-firstfumee]) plotfumee(e.x,e.y,((int)rayonapparent*rayonfumee[o-firstfumee])>>9);
				} else {
					if (rayonapparent>.3) {
						if (rayonapparent<.5) plot(e.x-_DX,e.y-_DY,0x0);
						else {
						//	if (rayonapparent>=7) mo=0; else mo=1;
							if (obj[o].distance<(ECHELLE*ECHELLE*.14)) mo=0; else mo=1;
							// on calcule alors la pos de la cam dans le repère de l'objet, ie ObjT*(campos-objpos)
							mulmtv(&obj[o].rot,&obj[o].t,&c);
							neg(&c);
							// on calcule aussi la position de tous les points de l'objet dans le repere de la camera, ie CamT*Obj*u
							mulmt3(&co,&cam.rot,&obj[o].rot);
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
							if (obj[o].type==TIR) {
								if (pts2d[0].v.x!=MAXINT && pts2d[1].v.x!=MAXINT) drawline(&pts2d[0],&pts2d[1],0xFFA0F0);
							} else {
								for (p=0; p<mod[obj[o].model].nbfaces[mo]; p++) {
									// test de visibilité entre cam et normale
									copyv(&t,&mod[obj[o].model].pts[mo][mod[obj[o].model].fac[mo][p].p[0]]);
									subv(&t,&c);
									if (scalaire(&t,&mod[obj[o].model].fac[mo][p].norm)<=0) {
										if (pts2d[mod[obj[o].model].fac[mo][p].p[0]].v.x != MAXINT &&
											 pts2d[mod[obj[o].model].fac[mo][p].p[1]].v.x != MAXINT &&
											 pts2d[mod[obj[o].model].fac[mo][p].p[2]].v.x != MAXINT) {
											if (obj[o].type==TABBORD && p>=mod[obj[o].model].nbfaces[mo]-2) {
												vect2dm pt[3];
												int i;
												for (i=0; i<3; i++) {
													pt[i].v.x=pts2d[mod[obj[o].model].fac[mo][p].p[i]].v.x;
													pt[i].v.y=pts2d[mod[obj[o].model].fac[mo][p].p[i]].v.y;
												}
												if (p-(mod[obj[o].model].nbfaces[mo]-2)) {
													pt[2].mx=MARGE;
													pt[2].my=SYTB+MARGE;
													pt[0].mx=SXTB+MARGE;
													pt[0].my=MARGE;
													pt[1].mx=SXTB+MARGE;
													pt[1].my=SYTB+MARGE;
												} else {
													pt[0].mx=MARGE;
													pt[0].my=SYTB+MARGE;
													pt[1].mx=MARGE;
													pt[1].my=MARGE;
													pt[2].mx=SXTB+MARGE;
													pt[2].my=MARGE;
												}
												polymap(&pt[0],&pt[1],&pt[2]);
											} else {
												coul=*(int*)&mod[obj[o].model].fac[mo][p].color;
												if (Dark) {
													if (obj[o].type!=TABBORD) coul=coul-((coul>>2)&0x3F3F3F);
													else coul=coul-((coul>>2)&0x003F3F);
												}
												if (pts2d[mod[obj[o].model].fac[mo][p].p[0]].xl!=MAXINT && pts2d[mod[obj[o].model].fac[mo][p].p[1]].xl!=MAXINT && pts2d[mod[obj[o].model].fac[mo][p].p[2]].xl!=MAXINT)
													polyphong(&pts2d[mod[obj[o].model].fac[mo][p].p[0]],&pts2d[mod[obj[o].model].fac[mo][p].p[1]],&pts2d[mod[obj[o].model].fac[mo][p].p[2]],coul);
												else
													polyflat(
														&pts2d[mod[obj[o].model].fac[mo][p].p[0]].v,
														&pts2d[mod[obj[o].model].fac[mo][p].p[1]].v,
														&pts2d[mod[obj[o].model].fac[mo][p].p[2]].v, coul);
											}
										}
									}
								}
							}
						}
					}
				}
				if (Dark && obj[o].type==PHARE) {	// HALO
					plotphare(e.x,e.y,rayonapparent*4+1);
				}
			}
		}
		}
	if (fast!=1) { o=obj[o].prec; no--; } else o=obj[o].next;
	} while (o!=-1);
}
