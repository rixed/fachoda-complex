#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "map.h"

void drawroadline(int x1, int y1, int x2, int y2, int l, pixel c1, pixel c2) {
	// trace la route (r,r+1) en ligne, vect2dc étant calculé
	int s,ss,ssi,ssf,medline, x,y,xi, dy, cr,cg,cb, qdizor;
	int tmp;
	int q, qcr,qcb,qcg;
	if (x1>x2) {
		tmp=x1; x1=x2; x2=tmp;
		tmp=y1; y1=y2; y2=tmp;
		tmp=*(int*)&c1; *(int*)&c1=*(int*)&c2; *(int*)&c2=tmp;
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

void couperoute(vect2dc *e, vector *v1,vector *v2) {
#define H (32<<3)
	vector p;
	p.x=((v2->x-v1->x)*(H-v1->z))/(v2->z-v1->z)+v1->x;
	p.y=((v2->y-v1->y)*(H-v1->z))/(v2->z-v1->z)+v1->y;
	p.z=H;
	proj(&e->v,&p);
}

void drawroute(int k) {
	int nk, hi, r;
	int j,i, typ, larg;
	vector pt3d[4], v,u;
	vecic pt[4];
//	if (!(submap_of_map[k]&0x80)) return;
	nk=submap_of_map[k]&0x7F;
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
			} else {	// route proche
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
				MMXSaveFPU();
				polyclip(&pt[1],&pt[0],&pt[2]);
				polyclip(&pt[3],&pt[1],&pt[2]);
				MMXRestoreFPU();
			}
		}
	}
	if (!j) {
		printf("HASH TABLE ERROR: MAPMAP FLAGED BUT NO ROAD ELEMENT PRESENT\n");
		exit(-1);
	}
}
