#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <values.h>
#include <stdinc.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86dga.h>
#define vf 8
#define vfm (1<<vf)
typedef struct {
	uchar b,g,r;
} pixel;
typedef struct {
	uchar b,g,r,u;
} pixel32;
typedef struct {
	float x,y,z;
} vector;
typedef struct {
	int p[3];
	pixel color;
	vector norm;
} face;
typedef struct {
	int x,y,z;
} veci;
typedef struct {
	int x,y,z;
	pixel c;
} vecic;
typedef struct {
	int x, y;
} vect2d;
typedef struct {
	int x, y;
	int xl, yl;
} vect2dlum;
typedef struct {
	int x, y;
	pixel c;
} vect2dc;
typedef struct {
	int x, y;
	uchar mx,my;
} vect2dm;
typedef struct {
	vector x,y,z;
} matrix;
matrix Light={ {1,0,0},{0,1,0},{0,0,1}};
matrix mat_id={{1,0,0},{0,1,0},{0,0,1}};
vector vac_diag={1,1,1}, vec_zero={0,0,0}, vec_g={0,0,-1};
// VIDEO
int bank, size, width, BufVidOffset, yview=0, xmouse, ymouse, bmouse;
pixel32 *videobuffer, *bckbuffer;
pixel *video;
GC gc;
Display *disp;
Window win,root;
XEvent  ev;
XImage img = { 0,0, 0, ZPixmap,NULL, LSBFirst, 32, LSBFirst, 32, 24, 0, 24, 0,0,0 };
// 3DSTUDIO
vector *pts, *norm;
face *fac;
vector campos;
matrix camrot;
int nbfaces, nbpts;
double focale=200;
vect2dlum *pts2d;
uchar *preca;
// options changeables à la ligne de com :
int _DX,_DY,SX=960,SY=300;
void initXwin(){
	img.width=width=SX; img.height=SY;
	disp = XOpenDisplay("");
	gc = DefaultGC(disp,DefaultScreen(disp));
	root = DefaultRootWindow(disp);
	win=XCreateSimpleWindow(disp, root, 0,0, SX,SY, 0,0,15);
	XSelectInput(disp,win,ExposureMask|KeyPressMask|ButtonPressMask|ButtonReleaseMask);
	XInitImage(&img);
	XPutImage(disp, win, gc, &img, 0,0, 0,0, SX,SY);
	XMapWindow(disp,win);
}
// ROUTINES DE MATH
 inline void addv(vector *r, vector *a) { r->x+=a->x; r->y+=a->y; r->z+=a->z; }
 inline void addvi(veci *r, veci *a) { r->x+=a->x; r->y+=a->y; r->z+=a->z; }
 inline void subv(vector *r, vector *a) { r->x-=a->x; r->y-=a->y; r->z-=a->z; }
 inline void subvi(veci *r, veci *a) { r->x-=a->x; r->y-=a->y; r->z-=a->z; }
 inline void mulv(vector *r, float a) { r->x*=a; r->y*=a; r->z*=a; }
 inline void copyv(vector *r, vector *a) { r->x=a->x; r->y=a->y; r->z=a->z; }
 inline void copym(matrix *r, matrix *a) { memcpy(r,a,sizeof(matrix)); }
 inline void mulm(matrix *r, matrix *a) {
	matrix b;
	copym(&b,r);
	r->x.x = b.x.x*a->x.x+b.y.x*a->x.y+b.z.x*a->x.z;
	r->y.x = b.x.x*a->y.x+b.y.x*a->y.y+b.z.x*a->y.z;
	r->z.x = b.x.x*a->z.x+b.y.x*a->z.y+b.z.x*a->z.z;
	r->x.y = b.x.y*a->x.x+b.y.y*a->x.y+b.z.y*a->x.z;
	r->y.y = b.x.y*a->y.x+b.y.y*a->y.y+b.z.y*a->y.z;
	r->z.y = b.x.y*a->z.x+b.y.y*a->z.y+b.z.y*a->z.z;
	r->x.z = b.x.z*a->x.x+b.y.z*a->x.y+b.z.z*a->x.z;
	r->y.z = b.x.z*a->y.x+b.y.z*a->y.y+b.z.z*a->y.z;
	r->z.z = b.x.z*a->z.x+b.y.z*a->z.y+b.z.z*a->z.z;
}
 inline void mulm3(matrix *r, matrix *c, matrix *a) {
	matrix b;
	b.x.x = c->x.x*a->x.x+c->y.x*a->x.y+c->z.x*a->x.z;
	b.y.x = c->x.x*a->y.x+c->y.x*a->y.y+c->z.x*a->y.z;
	b.z.x = c->x.x*a->z.x+c->y.x*a->z.y+c->z.x*a->z.z;
	b.x.y = c->x.y*a->x.x+c->y.y*a->x.y+c->z.y*a->x.z;
	b.y.y = c->x.y*a->y.x+c->y.y*a->y.y+c->z.y*a->y.z;
	b.z.y = c->x.y*a->z.x+c->y.y*a->z.y+c->z.y*a->z.z;
	b.x.z = c->x.z*a->x.x+c->y.z*a->x.y+c->z.z*a->x.z;
	b.y.z = c->x.z*a->y.x+c->y.z*a->y.y+c->z.z*a->y.z;
	b.z.z = c->x.z*a->z.x+c->y.z*a->z.y+c->z.z*a->z.z;
	copym(r,&b);
}
 inline void mulmt3(matrix *r, matrix *c, matrix *a) {	// c est transposée
	matrix b;
	b.x.x = c->x.x*a->x.x + c->x.y*a->x.y + c->x.z*a->x.z;
	b.y.x = c->x.x*a->y.x + c->x.y*a->y.y + c->x.z*a->y.z;
	b.z.x = c->x.x*a->z.x + c->x.y*a->z.y + c->x.z*a->z.z;
	b.x.y = c->y.x*a->x.x + c->y.y*a->x.y + c->y.z*a->x.z;
	b.y.y = c->y.x*a->y.x + c->y.y*a->y.y + c->y.z*a->y.z;
	b.z.y = c->y.x*a->z.x + c->y.y*a->z.y + c->y.z*a->z.z;
	b.x.z = c->z.x*a->x.x + c->z.y*a->x.y + c->z.z*a->x.z;
	b.y.z = c->z.x*a->y.x + c->z.y*a->y.y + c->z.z*a->y.z;
	b.z.z = c->z.x*a->z.x + c->z.y*a->z.y + c->z.z*a->z.z;
	copym(r,&b);
}
 inline float norme(vector *u){ return(sqrt(u->x*u->x+u->y*u->y+u->z*u->z)); }
 inline float norme2(vector *u){ return(u->x*u->x+u->y*u->y+u->z*u->z); }
 inline float scalaire(vector *u, vector *v){ return(u->x*v->x+u->y*v->y+u->z*v->z); }
 inline float renorme(vector *a) {
	float d = norme(a);
	if (d!=0) {a->x/=d; a->y/=d; a->z/=d; }
	return(d);
}
 inline void prodvect(vector *a, vector *b, vector *c) {
	c->x = a->y*b->z-a->z*b->y;
	c->y = a->z*b->x-a->x*b->z;
	c->z = a->x*b->y-a->y*b->x;
}
 inline void orthov(vector *a, vector *b) {
	float s=scalaire(a,b);
	a->x -= s*b->x;
	a->y -= s*b->y;
	a->z -= s*b->z;
}
 inline float orthov3(vector *a, vector *b, vector *r) {
	float s=scalaire(a,b);
	r->x = a->x-s*b->x;
	r->y = a->y-s*b->y;
	r->z = a->z-s*b->z;
	return(s);
}
 inline void mulmv(matrix *n, vector *v, vector *r) {
	vector t;
	copyv(&t,v);
	r->x = n->x.x*t.x+n->y.x*t.y+n->z.x*t.z;
	r->y = n->x.y*t.x+n->y.y*t.y+n->z.y*t.z;
	r->z = n->x.z*t.x+n->y.z*t.y+n->z.z*t.z;
}
 inline void mulmtv(matrix *n, vector *v, vector *r) {
	vector t;
	copyv(&t,v);
	r->x = n->x.x*t.x+n->x.y*t.y+n->x.z*t.z;
	r->y = n->y.x*t.x+n->y.y*t.y+n->y.z*t.z;
	r->z = n->z.x*t.x+n->z.y*t.y+n->z.z*t.z;
}
 inline void neg(vector *v) { v->x=-v->x; v->y=-v->y; v->z=-v->z; }
 inline void proj(vect2d *e, vector *p) {
	e->x=_DX+p->x*focale/p->z;
	e->y=_DY+p->y*focale/p->z;
}
 inline void projl(vect2dlum *e, vector *p) {
	e->x=_DX+p->x*focale/p->z;
	e->y=_DY+p->y*focale/p->z;
}
 inline float proj1(float p, float z) { return(p*focale/z); }
 inline void subv3(vector *a, vector *b, vector *r) {	// il faut r!=a,b
	r->x = a->x-b->x;
	r->y = a->y-b->y;
	r->z = a->z-b->z;
}
 inline void randomv(vector *v) {
	v->x=drand48()-.5;
	v->y=drand48()-.5;
	v->z=drand48()-.5;
}
 inline void randomm(matrix *m) {
	randomv(&m->x);
	renorme(&m->x);
	m->y.x=-m->x.y;
	m->y.y=+m->x.x;
	m->y.z=-m->x.z;
	orthov(&m->y,&m->x);
	renorme(&m->y);
	prodvect(&m->x,&m->y,&m->z);
}
// ROUTINE DE TXT
unsigned char num[10][8]= {
	{ 0x3E,0x41,0x41,0x41,0x41,0x41,0x41,0x3E },		// 0
	{ 0x08,0x18,0x38,0x18,0x18,0x18,0x18,0x3C },		// 1
	{ 0x3E,0x63,0x43,0x06,0x0C,0x18,0x39,0x7F },		// 2
	{ 0x3E,0x63,0x43,0x07,0x03,0x43,0x63,0x3E },		// 3
	{ 0x30,0x30,0x30,0x36,0x3F,0x06,0x06,0x0F },
	{ 0x7F,0x61,0x60,0x7E,0x63,0x03,0x47,0x3E },		// 5
	{ 0x3E,0x63,0x60,0x6C,0x73,0x63,0x36,0x1C },
	{ 0x7F,0x43,0x03,0x0E,0x38,0x70,0x60,0x60 },		// 7
	{ 0x3E,0x63,0x63,0x3E,0x63,0x63,0x63,0x3E },
	{ 0x3E,0x63,0x63,0x33,0x1F,0x03,0x06,0x0C }		// 9
};
void pnumchar(int n, int x, int y, int c) {
	int i, l;
	for (l=0; l<8; l++, y++) {
		for (i=128; i>=1; i>>=1, x++) if (num[n][l]&i) ((int*)videobuffer)[x+SX*y]=c;
		x-=8;
	}
}
void pnum(int n, int x, int y, int c, char just) {
	char sig=2*(n>=0)-1;
	int m=n;
	if (just==1) { // justifié à gauche
		if (sig==1) x-=8;
		while (m!=0) { x+=8; m/=10; };
	}
	while (n!=0) {
		pnumchar(sig*(n%10),x,y,c);
		x-=8;
		n/=10;
	};
	if (sig==-1) for (m=x+2; m<x+6; m++) ((int*)videobuffer)[m+SX*(y+4)]=c;
}
// ROUTINES DE GFX
void plot(int x, int y, int r){
	if(x<_DX && x>=-_DX && y<_DY && y>=-_DY) {
		((int*)videobuffer)[x+_DX+SX*(y+_DY)]=r;
	}
}
void polyflat(vect2dlum *p1, vect2dlum *p2, vect2dlum *p3, int coul) {
	vect2dlum *tmp;
	int xi, yi, lx, i, j, jlim, yfin;
	int q1, q2, q3, ql, qx, qx2, ql2;
	pixel32 *vid;
	
	if (p2->y<p1->y) { tmp=p1; p1=p2; p2=tmp; }
	if (p3->y<p1->y) { tmp=p1; p1=p3; p3=tmp; }
	if (p3->y<p2->y) { tmp=p2; p2=p3; p3=tmp; }
	
	if (p3->y<0 || p1->y>SY) return;
	
	if (p1->y==p2->y) p1->y--;	// de l'avantage d'une grosse rézo...
	if (p3->y==p2->y) p3->y++;
	
	xi=p1->x<<vf; yi=p1->y;
	q1=((p3->x-p1->x)<<vf)/(p3->y-p1->y);
	q2=((p2->x-p1->x)<<vf)/(p2->y-p1->y);
	q3=((p3->x-p2->x)<<vf)/(p3->y-p2->y);
	lx = 1<<vf;
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
		for (xi=x>>vf; xi<1+((x+q)>>vf); xi++) plot(xi-_DX,y-_DY,col);
		x+=q;
	}
}
void polyphong(vect2dlum *p1, vect2dlum *p2, vect2dlum *p3, int coul) {
	vect2dlum *tmp;
	int xi, yi, lx, dx, dy, i, j, jlim, yfin;
	int q1, q2, q3, ql, p1x, p1y, p2x, p2y, qx, qx2, ql2, px,py,px2,py2;
	int a, aa, k, x, y, atmp, l;
	pixel32 *vid;
	
	if (p2->y<p1->y) { tmp=p1; p1=p2; p2=tmp; }
	if (p3->y<p1->y) { tmp=p1; p1=p3; p3=tmp; }
	if (p3->y<p2->y) { tmp=p2; p2=p3; p3=tmp; }
	
	if (p3->y<0 || p1->y>SY) return;
	
	if (p1->y==p2->y) p1->y--;	// de l'avantage d'une grosse rézo...
	if (p3->y==p2->y) p3->y++;
	
	q1=((p3->x-p1->x)<<vf)/(p3->y-p1->y);
	q2=((p2->x-p1->x)<<vf)/(p2->y-p1->y);
	q3=((p3->x-p2->x)<<vf)/(p3->y-p2->y);
	
	xi=p1->x<<vf; yi=p1->y;
	x=p1->xl<<vf; y=p1->yl<<vf;
	lx = 1<<vf;
	
	p1x=((p3->xl-p1->xl)<<vf)/(p3->y-p1->y);	// vecteurs quotients dans la "texture"
	p1y=((p3->yl-p1->yl)<<vf)/(p3->y-p1->y);
	p2x=((p2->xl-p1->xl)<<vf)/(p2->y-p1->y);
	p2y=((p2->yl-p1->yl)<<vf)/(p2->y-p1->y);

	if (q1<=q2) {
		ql = (q2-q1)|1;		// le taux d'accroissement de la taille du segment (en évitant 0);
		ql2= q3-q1;
		qx=qx2=q1;
		dx=((p2x-p1x)<<vf)/ql;
		dy=((p2y-p1y)<<vf)/ql;
		px=px2=p1x; py=py2=p1y;
	} else {
		int p3x,p3y;
		ql = (q1-q2)|1;
		ql2= q1-q3;
		qx=q2; qx2=q3;	
		dx=((p1x-p2x)<<vf)/ql;
		dy=((p1y-p2y)<<vf)/ql;
		p3x=((p3->xl-p2->xl)<<vf)/(p3->y-p2->y);
		p3y=((p3->yl-p2->yl)<<vf)/(p3->y-p2->y);
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
				MMXPhongLum((int*)vid+j,jlim-j,l,atmp,k,coul);
			}
			xi+=qx; yi++;
			lx+=ql;
			x+=px; y+=py;
			vid+=SX;
		}
	}
	MMXRestoreFPU();
}
int DMARK=0, DYMARK=0;
char fncol[200];
void save(char *fn) {
	int f;
	FILE *out;
	sprintf(fncol,"%s.col",fn);
	if ((out=fopen(fncol,"w+"))==NULL) {
		perror("fopen save");
		exit(-1);
	}
	for (f=0; f<nbfaces; f++) fwrite(&fac[f].color,sizeof(pixel), 1, out);
	fclose(out);
	for (f=0; f<SY-DYMARK; f++) MMXCopy((int*)bckbuffer+f*SX,(int*)bckbuffer+(f+DYMARK)*SX,SX);
}
int main(int nbarg, char **arg) {
	int i,j,k,n,a, quitte=0, faca=-1, obja=1;
	double loinvisu=10, visuteta=0,visuphi=0;
	double xm, ym, s;
	vector u,v,p; matrix m;
	FILE *file;
	vector c,t,pts3d,posc,ot,opos;
	double rayonapparent=0, r;
	int blanc=0xFFFFFF;
	matrix co, L, orot;
	vect2d e;
	int root_return, child_return, root_x_return, root_y_return;
	_DX=SX>>1; _DY=SY>>1;
	videobuffer=(pixel32*)malloc(SX*SY*sizeof(pixel32));
	bckbuffer=(pixel32*)malloc(SX*SY*sizeof(pixel32));
	MMXMemSetInt((int*)bckbuffer,0x808080,SX*SY);
	BufVidOffset=SX*sizeof(pixel32);
	img.data=malloc(SX*SY*sizeof(pixel));	//(char*)videobuffer;
	initXwin();
	preca = calloc(8*(256+2+256),1);	// des qwords
	for (r=0, a=0; a<256+2; a++, r+=1) {
		preca[8*a]=preca[8*a+2]=preca[8*a+4]=preca[8*a+6]=255.*exp(-r/50.);
		preca[8*a+1]=preca[8*a+3]=preca[8*a+5]=preca[8*a+7]=0;
	}
	pts2d=(vect2dlum*)malloc(1000*sizeof(vect2dlum));
	// VISION
	do {
		// Charger un fichier si faca est à -1
		if (faca==-1) {
			if ((file=fopen(arg[obja],"r"))==NULL) {
				printf("Sa mère à lui ! : %s\n",arg[obja]);
				perror("fopen");
				exit(-1);
			}
			fread(&p,sizeof(vector),1,file);
			fread(&nbpts,sizeof(int),1,file);
			fread(&nbfaces,sizeof(int),1,file);
			pts=(vector*)malloc(nbpts*sizeof(vector));
			norm=(vector*)malloc(nbpts*sizeof(vector));
			fac=(face*)malloc(nbfaces*sizeof(face));
			fread(pts,sizeof(vector),nbpts,file);
			for (i=0; i<nbfaces; i++) {
				fread(&fac[i].p[0],sizeof(int),3,file);
				subv3(&pts[fac[i].p[1]],&pts[fac[i].p[0]],&u);
				subv3(&pts[fac[i].p[2]],&pts[fac[i].p[0]],&v);
				prodvect(&v,&u,&fac[i].norm);
				renorme(&fac[i].norm);
			}
			for (i=0; i<nbpts; i++) {
				copyv(&u,&vec_zero);
				for (j=0; j<nbfaces; j++)
					if (fac[j].p[0]==i || fac[j].p[1]==i || fac[j].p[2]==i)
						addv(&u, &fac[j].norm);
				renorme(&u);
				copyv(&norm[i],&u);
			}
			fclose(file);
			sprintf(fncol,"%s.col",arg[obja]);
			if ((file=fopen(fncol,"r"))==NULL) {
				printf("réation de %s\n",fncol);
				file=fopen(fncol,"w+");
			} else for (i=0; i<nbfaces; i++) fread(&fac[i].color,sizeof(pixel), 1, file);
			fclose(file);
			faca=0;
			copym(&orot,&mat_id);
			copyv(&opos,&vec_zero);
			DMARK=SX/nbfaces;
			if (DMARK>SY/10) DMARK=SY/10;
			DYMARK=3*DMARK;
			if (DYMARK<20) DYMARK=20;
		}
		// POSITIONNER LA CAMERA DEVANT FACA
		copyv(&campos,&pts[fac[faca].p[0]]);
		addv(&campos,&pts[fac[faca].p[1]]);
		addv(&campos,&pts[fac[faca].p[2]]);
		mulv(&campos,.333333);
		copyv(&p,&fac[faca].norm);
		mulv(&p,loinvisu);
		addv(&campos,&p);
		copyv(&camrot.z,&p);
		renorme(&camrot.z);
		neg(&camrot.z);
		camrot.x.x=cos(visuteta);
		camrot.x.y=sin(visuteta);
		camrot.x.z=0;
		orthov(&camrot.x,&camrot.z);
		renorme(&camrot.x);
		prodvect(&camrot.z,&camrot.x,&camrot.y);
		// POSITIONNER LA LIGHTSOURCE
		copym(&Light,&camrot);
		mulv(&Light.x,(float)(xmouse-_DX)/_DX);
		mulv(&Light.y,(float)(ymouse-_DY)/_DY);
		addv(&Light.z,&Light.x);
		addv(&Light.z,&Light.y);
		renorme(&Light.z);
		orthov(&Light.x,&Light.z);
		renorme(&Light.x);
		prodvect(&Light.z,&Light.x,&Light.y);
		// L'OBJET GIGOTTE
		orot.z.x=.1*cos(visuphi);
		orot.z.y=.1*sin(visuphi);
		orot.z.z=.995;
		orthov(&orot.x,&orot.z);
		renorme(&orot.x);
		prodvect(&orot.z,&orot.x,&orot.y);
		visuphi+=.04;
		// TRACER LE FOND
		MMXCopy((int*)videobuffer,(int*)bckbuffer,SX*SY);
		blanc=0xFFFFFF^(*(int*)&fac[faca].color);
		// AFFICHER L'OBJET
		// on calcule alors la pos de la cam dans le repère de l'objet, ie ObjT*(campos-objpos)
		copyv(&ot,&opos);
		subv(&ot,&campos);
		mulmtv(&camrot,&ot,&posc);
		mulmtv(&orot,&ot,&c);
		neg(&c);
		// on calcule aussi la position de tous les points de l'objet dans le repere de la camera, ie CamT*Obj*u
		mulmt3(&co,&camrot,&orot);
		// et la position de la lumière dans le repère de l'objet, ie ObjT*Lx et ObjT*Ly
		mulmt3(&L, &orot, &Light);
#define DISTLUM 300.
		mulv(&L.x,DISTLUM);
		mulv(&L.y,DISTLUM);
		for (i=0; i<nbpts; i++) {
			mulmv(&co, &pts[i], &pts3d);
			addv(&pts3d,&posc);
			if (pts3d.z >0) projl(&pts2d[i],&pts3d);
			else pts2d[i].x = MAXINT;
			// on calcule aussi les projs des
			// norms dans le plan lumineux infiniment éloigné
			if (scalaire(&norm[i],&L.z)<0) {
				pts2d[i].xl = scalaire(&norm[i],&L.x);
				pts2d[i].yl = scalaire(&norm[i],&L.y);
			} else pts2d[i].xl = MAXINT;
		}
		for (i=0; i<nbfaces; i++) {
			// test de visibilité entre cam et normale
			copyv(&t,&pts[fac[i].p[0]]);
			subv(&t,&c);
			if (scalaire(&t,&fac[i].norm)<=0) {
				if (pts2d[fac[i].p[0]].x != MAXINT &&
					 pts2d[fac[i].p[1]].x != MAXINT &&
					 pts2d[fac[i].p[2]].x != MAXINT) {
					if (pts2d[fac[i].p[0]].xl!=MAXINT && pts2d[fac[i].p[1]].xl!=MAXINT && pts2d[fac[i].p[2]].xl!=MAXINT) polyphong(&pts2d[fac[i].p[0]],&pts2d[fac[i].p[1]],&pts2d[fac[i].p[2]],*(int*)&fac[i].color);
					else polyflat(&pts2d[fac[i].p[0]],&pts2d[fac[i].p[1]],&pts2d[fac[i].p[2]],*(int*)&fac[i].color);	
				}
			}
		}
		if (pts2d[fac[faca].p[0]].x != MAXINT &&
			 pts2d[fac[faca].p[1]].x != MAXINT &&
			 pts2d[fac[faca].p[2]].x != MAXINT) {
			drawline(&pts2d[fac[faca].p[0]],&pts2d[fac[faca].p[1]],blanc);
			drawline(&pts2d[fac[faca].p[0]],&pts2d[fac[faca].p[2]],blanc);
			drawline(&pts2d[fac[faca].p[1]],&pts2d[fac[faca].p[2]],blanc);
		}
		// LA PALETTE
		for (i=0; i<nbfaces; i++) {
			if (i==faca) {
				for (j=0; j<DYMARK/4; j++) MMXMemSetInt((int*)videobuffer+SX*(SY-DYMARK+j)+i*DMARK,blanc,DMARK);
				for (; j<DYMARK; j++) MMXMemSetInt((int*)videobuffer+SX*(SY-DYMARK+j)+i*DMARK,*(int*)&fac[i].color,DMARK);
			} else for (j=0; j<DYMARK; j++) MMXMemSetInt((int*)videobuffer+SX*(SY-DYMARK+j)+i*DMARK,*(int*)&fac[i].color,DMARK);
		}
		pnum(fac[faca].color.r,faca*DMARK,SY-DYMARK-30,0xFF8080,1);
		pnum(fac[faca].color.g,faca*DMARK,SY-DYMARK-20,0x80FF80,1);
		pnum(fac[faca].color.b,faca*DMARK,SY-DYMARK-10,0x8080FF,1);
		// TEST LA SOURIS
		XQueryPointer(disp,win,&root_return,&child_return,&root_x_return,&root_y_return,&xmouse,&ymouse,&bmouse);
		if (xmouse<0) xmouse=0;
		if (xmouse>=SX) xmouse=SX-1;
		if (ymouse<0) ymouse=0;
		if (ymouse>=SY) ymouse=SY-1;
		if (bmouse&Button3Mask) {
			for (i=faca; i<nbfaces; i++) {
				fac[i].color.r=videobuffer[xmouse+ymouse*SX].r;
				fac[i].color.g=videobuffer[xmouse+ymouse*SX].g;
				fac[i].color.b=videobuffer[xmouse+ymouse*SX].b;
			}
		}
		if (bmouse&Button1Mask) {
			fac[faca].color.r=videobuffer[xmouse+ymouse*SX].r;
			fac[faca].color.g=videobuffer[xmouse+ymouse*SX].g;
			fac[faca].color.b=videobuffer[xmouse+ymouse*SX].b;
		}
		// TEST LES TOUCHES
		if (XPending(disp)) {
			XNextEvent(disp, &ev);
			if (ev.type==KeyPress) {
				int sympercod;
				KeySym *k=XGetKeyboardMapping(disp,ev.xkey.keycode,1,&sympercod);
				if (*k==XK_Escape) { save(arg[obja]); quitte=1; }
				else if (*k==XK_a) loinvisu+=1;
				else if (*k==XK_z) loinvisu-=1;
				else if (*k==XK_q) visuteta-=1;
				else if (*k==XK_s) visuteta+=1;
				else if (*k==XK_e) fac[faca].color.r++;
				else if (*k==XK_d) fac[faca].color.r--;
				else if (*k==XK_r) fac[faca].color.r+=10;
				else if (*k==XK_f) fac[faca].color.r-=10;
				else if (*k==XK_t) fac[faca].color.g++;
				else if (*k==XK_g) fac[faca].color.g--;
				else if (*k==XK_y) fac[faca].color.g+=10;
				else if (*k==XK_h) fac[faca].color.g-=10;
				else if (*k==XK_u) fac[faca].color.b++;
				else if (*k==XK_j) fac[faca].color.b--;
				else if (*k==XK_i) fac[faca].color.b+=10;
				else if (*k==XK_k) fac[faca].color.b-=10;
				else if (*k==XK_x) MMXMemSetInt((int*)bckbuffer,0x808080,SX*SY);
				else if (*k==XK_Right) {if (++faca>=nbfaces) faca=0;}
				else if (*k==XK_Left) {if (--faca<0) faca=nbfaces-1;}
				else if (*k==XK_KP_Add) {
					save(arg[obja]);
					if (++obja>=nbarg) obja=1;
					faca=-1;
				}
				else if (*k==XK_KP_Subtract) {
					save(arg[obja]);
					if (--obja<1) obja=nbarg-1;
					faca=-1;
				}
			}
		}
		// AFFICHAGE
		MMXCopyToScreen((int*)img.data,(int*)videobuffer,SX,SY,SX);
		XPutImage(disp, win, gc, &img, 0,0, 0,0, SX,SY);
	} while (!quitte);
	// FIN
	exit(0);
}
