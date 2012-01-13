#include <stdlib.h>
#include <math.h>
#include "3d.h"

int *mapping;
uchar *preca;

void initmapping() {
	mapping=(int*)malloc(256*256*sizeof(int));
}

void polymap(vect2dm *p1, vect2dm *p2, vect2dm *p3) {
	vect2dm *tmp, *pmax, *pmin;
	int xi, yi, lx, i, ilim;
	int q1, q2, q3=0, ql, qx, qxx, ql2;
	int ix, iy;
	int qmx1, qmx2, qmx3=0, qmy1, qmy2, qmy3=0, qmxx, qmyy, qmx, qmy;
	int *vid;
	int xm, ym;
	void trapeze (int dy) {
		int xxm, yym;
		if (yi<0) {
			if (yi<-dy) {
				yi+=dy;
				xi+=dy*qx;
				lx+=dy*ql;
				xm+=dy*qmx;
				ym+=dy*qmy;
				return;
			} else {
				xi-=yi*qx;
				lx-=yi*ql;
				xm-=yi*qmx;
				ym-=yi*qmy;
				dy+=yi;
				yi=0;
			}
		}
		vid=(int*)videobuffer+yi*SX;
		while (dy>0 && yi<SY) {
			i=xi>>vf; ilim=i+(lx>>vf);
			if (ilim>=SX) ilim=SX-1;
			xxm=xm;
			yym=ym;
			if (i<0) {
				xxm-=i*ix;
				yym-=i*iy;
				i=0;
			}
			if (i<ilim) {
				do {
					vid[i]=*((int*)mapping+((xxm>>vf)&0xFF)+((yym>>(vf-8))&0xFF00));
					i++;
					xxm+=ix; yym+=iy;
				} while (i<=ilim);
			}
			xi+=qx;
			lx+=ql;
			xm+=qmx;
			ym+=qmy;
			vid+=SX;
			dy--;
			yi++;
		}
	};
	if (p2->y<p1->y) { tmp=p1; p1=p2; p2=tmp; }
	if (p3->y<p1->y) { tmp=p1; p1=p3; p3=tmp; }
	if (p3->y<p2->y) { tmp=p2; p2=p3; p3=tmp; }
	if (p3->y<0 || p1->y>SY) return;
	pmin=pmax=p1;
	if (p2->x>pmax->x) pmax=p2;
	if (p3->x>pmax->x) pmax=p3;
	if (pmax->x<0) return;
	if (p2->x<pmin->x) pmin=p2;
	if (p3->x<pmin->x) pmin=p3;
	if (pmin->x>SX) return;
	yi=p1->y;
//	MMXSaveFPU();
	if (p1->y!=p2->y) {
		xi=p1->x<<vf;
		xm=p1->mx<<vf;
		ym=p1->my<<vf;
		q1=((p2->x-p1->x)<<vf)/(p2->y-p1->y);
		qmx1=(((int)(p2->mx-p1->mx))<<vf)/(p2->y-p1->y);
		qmy1=(((int)(p2->my-p1->my))<<vf)/(p2->y-p1->y);
		q2=((p3->x-p1->x)<<vf)/(p3->y-p1->y);
		qmx2=(((int)(p3->mx-p1->mx))<<vf)/(p3->y-p1->y);
		qmy2=(((int)(p3->my-p1->my))<<vf)/(p3->y-p1->y);
		if (p3->y-p2->y) {
			q3=((p3->x-p2->x)<<vf)/(p3->y-p2->y);
			qmx3=(((int)(p3->mx-p2->mx))<<vf)/(p3->y-p2->y);
			qmy3=(((int)(p3->my-p2->my))<<vf)/(p3->y-p2->y);
		}
		lx = vfm;
		if (q1<=q2) {
			if(!(ql=q2-q1)) ql=1;		// le taux d'accroissement de la taille du segment (en évitant 0);
			if(!(ql2=q2-q3)) ql2=-1;
			qx=q1; qxx=q3;
			qmx=qmx1; qmy=qmy1; qmxx=qmx3; qmyy=qmy3;
			if (p2->y-p1->y>p3->y-p2->y) {
#define QLPREC (vfm/4)
				if (ql>QLPREC) {
					ix=((qmx2-qmx1)<<vf)/ql;
					iy=((qmy2-qmy1)<<vf)/ql;
				} else ix=iy=0;
			} else {
				if (ql2<-QLPREC) {
					ix=((qmx2-qmx3)<<vf)/ql2;
					iy=((qmy2-qmy3)<<vf)/ql2;
				} else ix=iy=0;
			}
		} else {
			if (!(ql=q1-q2)) ql=1;
			if (!(ql2=q3-q2)) ql2=-1;
			qx=qxx=q2;
			qmx=qmxx=qmx2; qmy=qmyy=qmy2;
			if (p2->y-p1->y>p3->y-p2->y) {
				if (ql>QLPREC) {
					ix=((qmx1-qmx2)<<vf)/ql;
					iy=((qmy1-qmy2)<<vf)/ql;
				} else ix=iy=0;
			} else {
				if (ql2<-QLPREC) {
					ix=((qmx3-qmx2)<<vf)/ql2;
					iy=((qmy3-qmy2)<<vf)/ql2;
				} else ix=iy=0;
			}
		}
	//	MMXGouroPreca(ib,ig,ir);
		trapeze(p2->y-p1->y);
		qx=qxx; ql=ql2;
		qmx=qmxx; qmy=qmyy;
		trapeze(p3->y-p2->y+1);
	} else {
		if (p3->y>p2->y) {
			q2=((p3->x-p1->x)<<vf)/(p3->y-p1->y);
			qmx2=(((int)(p3->mx-p1->mx))<<vf)/(p3->y-p1->y);
			qmy2=(((int)(p3->my-p1->my))<<vf)/(p3->y-p1->y);
			q3=((p3->x-p2->x)<<vf)/(p3->y-p2->y);
			qmx3=(((int)(p3->mx-p2->mx))<<vf)/(p3->y-p2->y);
			qmy3=(((int)(p3->my-p2->my))<<vf)/(p3->y-p2->y);
			if (p2->x>=p1->x) {
				xi=p1->x<<vf;
				xm=p1->mx<<vf;
				ym=p1->my<<vf;
				lx = (p2->x-p1->x)<<vf;
				if(!(ql=q3-q2)) ql=-1;
				qx=q2;
				qmx=qmx2; qmy=qmy2;
				if (ql<-QLPREC) {
					ix=((qmx3-qmx2)<<vf)/ql;
					iy=((qmy3-qmy2)<<vf)/ql;
				} else ix=iy=0;
			} else {
				xi=p2->x<<vf;
				xm=p2->mx<<vf;
				ym=p2->my<<vf;
				lx = (p1->x-p2->x)<<vf;
				if(!(ql=q2-q3)) ql=-1;
				qx=q3;
				qmx=qmx3; qmy=qmy3;
				if (ql<-QLPREC) {
					ix=((qmx2-qmx3)<<vf)/ql;
					iy=((qmy2-qmy3)<<vf)/ql;
				} else ix=iy=0;
			}
		//	MMXGouroPreca(ib,ig,ir);
			trapeze(p3->y-p1->y+1);
		} else {
			xi=pmin->x<<vf;
			if(!(lx=(pmax->x-pmin->x)<<vf)) lx=vfm;
			xm=pmin->mx<<vf;
			ym=pmin->my<<vf;
			if (lx>QLPREC) {
				ix=(pmax->mx-pmin->mx)<<vf/lx;
				iy=(pmax->my-pmin->my)<<vf/lx;
			} else ix=iy=0;
		//	MMXGouroPreca(ib,ig,ir);
			trapeze(1);
		}
	}
//	MMXRestoreFPU();
}

void polyphong(vect2dlum *p1, vect2dlum *p2, vect2dlum *p3, int coul) {
	vect2dlum *tmp;
	int xi, yi, lx, dx, dy, i, j, jlim, yfin;
	int q1, q2, q3, ql, p1x, p1y, p2x, p2y, qx, qx2, ql2, px,py,px2,py2;
	int a, aa, k, x, y, atmp, l;
	pixel32 *vid;
	
	int coul_r = coul & 0xff;
	int coul_g = (coul >> 8) & 0xff;
	int coul_b = (coul >> 16) & 0xff;

	if (p2->y<p1->y) { tmp=p1; p1=p2; p2=tmp; }
	if (p3->y<p1->y) { tmp=p1; p1=p3; p3=tmp; }
	if (p3->y<p2->y) { tmp=p2; p2=p3; p3=tmp; }
	if (p1->y==p2->y && p1->x>p2->x) { tmp=p1; p1=p2; p2=tmp; }
	if (p3->y<0 || p1->y>SY) return;
	
//	if (p1->y==p2->y) p1->y--;	// de l'avantage d'une grosse rézo...
//	if (p3->y==p2->y) p3->y++;
	
	yi=p1->y; y=p1->yl<<vf;
	
	if (p3->y==p1->y) {
		if (p1->x>p3->x) { tmp=p1; p1=p3; p3=tmp; }
		if (p2->x<p3->x) { tmp=p2; p2=p3; p3=tmp; }
		xi = p1->x<<vf;
		x=p1->xl<<vf;
		lx = (p2->x - p1->x +1);
		yfin = yi+1;
		dx = ((p2->xl-p1->xl)<<vf)/lx;
		dy = ((p2->yl-p1->yl)<<vf)/lx;
		lx <<= vf;
		a=(dx*dx+dy*dy)>>vf;
		aa=a+a;
		MMXPhongInit(aa,50);
		goto debtrace;
	}
	
	xi=p1->x<<vf;
	x=p1->xl<<vf;
	lx = 1<<vf;
	
	q1=((p3->x-p1->x)<<vf)/(p3->y-p1->y);
	p1x=((p3->xl-p1->xl)<<vf)/(p3->y-p1->y);	// vecteurs quotients dans la "texture"
	p1y=((p3->yl-p1->yl)<<vf)/(p3->y-p1->y);
	
	if (p1->y!=p2->y) {
		q2=((p2->x-p1->x)<<vf)/(p2->y-p1->y);
		p2x=((p2->xl-p1->xl)<<vf)/(p2->y-p1->y);
		p2y=((p2->yl-p1->yl)<<vf)/(p2->y-p1->y);
	} else {
		q2 = p2x = p2y = MAXINT;
		lx = (p2->x-p1->x+1)<<vf;
	}
	if (p3->y!=p2->y) {
		q3=((p3->x-p2->x)<<vf)/(p3->y-p2->y);
	} else {
		q3 = MAXINT;
	}
	
	if (q1<=q2) {
		int p3x,p3y;
		ql = (q2-q1)|1;		// le taux d'accroissement de la taille du segment (en évitant 0);
		ql2= (q3-q1)|1;
		qx=qx2=q1;
		if (q2==MAXINT && p3->y!=p2->y) {
			p3x=((p3->xl-p2->xl)<<vf)/(p3->y-p2->y);
			p3y=((p3->yl-p2->yl)<<vf)/(p3->y-p2->y);
			dx=((p3x-p1x)<<vf)/ql2;
			dy=((p3y-p1y)<<vf)/ql2;
		} else {
			dx=((p2x-p1x)<<vf)/ql;
			dy=((p2y-p1y)<<vf)/ql;
		}
		px=px2=p1x; py=py2=p1y;
	} else {
		int p3x,p3y;
		ql = (q1-q2)|1;
		ql2= q1-q3;
		qx=q2; qx2=q3;	
		dx=((p1x-p2x)<<vf)/ql;
		dy=((p1y-p2y)<<vf)/ql;
		if (p3->y!=p2->y) {
			p3x=((p3->xl-p2->xl)<<vf)/(p3->y-p2->y);
			p3y=((p3->yl-p2->yl)<<vf)/(p3->y-p2->y);
		} else {
			p3x = p3y = 0;
		}
		px=p2x; px2=p3x; py=p2y; py2=p3y;
	}
	
	a=(dx*dx+dy*dy)>>vf;
	aa=a+a;
	MMXPhongInit(aa,50);
	// clipper les y<0 ! sinon ca fait des pauses !
	yfin=p2->y;
	if (p2->y<0) {
		xi+=(p2->y-p1->y)*qx;
		yi=p2->y;
		lx+=(p2->y-p1->y)*ql;
		x+=(p2->y-p1->y)*px; y+=(p2->y-p1->y)*py;
		yfin=p3->y; ql=ql2; qx=qx2; px=px2; py=py2;
	}
	if (yi<0) {
		xi-=yi*qx;
		lx-=yi*ql;
		x-=yi*px; y-=yi*py;
		yi=0;
	}
debtrace:
	vid=videobuffer+(yi)*SX;
	
	for (i=0; i<2; i++, yfin=p3->y, ql=ql2, qx=qx2, px=px2, py=py2){
		while (yi<yfin && yi<SY) {
			k=(dx*(x>>vf)+dy*(y>>vf))<<1;
			l=x*(x>>vf)+y*(y>>vf);
			atmp=a;
			jlim=(lx+xi)>>vf;
			j=xi>>vf;
			if (j<0) {
				l += -j*(k+atmp+(((-j-1)*aa)>>1));
				atmp += -j*aa;
				j=0;
			}
			if (jlim>SX-1) jlim=SX-1;
			if (j<jlim) {
				for (; j!=jlim; j++) {
					int cc;
					cc=l<(1<<24)?preca[l>>16]:0;
					vid[j].r=(int)coul_r+(int)cc<256?coul_r+cc:255;
					vid[j].g=(int)coul_g+(int)cc<256?coul_g+cc:255;
					vid[j].b=(int)coul_b+(int)cc<256?coul_b+cc:255;
					l+=k+atmp;
					atmp+=aa;
				}
			}
			xi+=qx; yi++;
			lx+=ql;
			x+=px; y+=py;
			vid+=SX;
		}
	}
	MMXRestoreFPU();
}
