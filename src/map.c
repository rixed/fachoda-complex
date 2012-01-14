#include <math.h>
#include "3d.h"
uchar *map;	// les Z
uchar *mapmap;	// les No de sousmap (ca prend que 3 bits, bits 5 pour visibilité)
uchar *(sousmap[10]);	// les Z de les sousmaps
short int *fomap;	// No du premier objet de la map
uchar Direct;
pixel zcol[256];
char *(sousmapint[10]);	// les intens relatives des points de la sousmap
uchar *flottemap;
void carremap(int x, int y, int s, uchar *m, int smap) {
	int ss=s>>1;
	double sf=Accident*(double)s/smap;
	if (ss) {
		int xx, yy;
		if((xx=x+s)==smap) xx=0;
		if((yy=y+s)==smap) yy=0;
		if (m[x+ss+y*smap]==0)
			m[x+ss+y*smap]=(((int)m[x+y*smap]+m[xx+y*smap])>>1)+(randK()-.5)*sf;
		if (m[xx+(y+ss)*smap]==0)
			m[xx+(y+ss)*smap]=(((int)m[xx+y*smap]+m[xx+yy*smap])>>1)+(randK()-.5)*sf;
		if (m[x+ss+yy*smap]==0)
			m[x+ss+yy*smap]=(((int)m[x+yy*smap]+m[xx+yy*smap])>>1)+(randK()-.5)*sf;
		if (m[x+(y+ss)*smap]==0)
			m[x+(y+ss)*smap]=(((int)m[x+y*smap]+m[x+yy*smap])>>1)+(randK()-.5)*sf;
		if (m[x+ss+(y+ss)*smap]==0)
			m[x+ss+(y+ss)*smap]=(((int)m[(x+ss)+y*smap]+m[xx+(y+ss)*smap]+m[(x+ss)+yy*smap]+m[x+(y+ss)*smap])>>2)+(randK()-.5)*sf;
		carremap(x,y,ss,m,smap);
		carremap(x+ss,y,ss,m,smap);
		carremap(x,y+ss,ss,m,smap);
		carremap(x+ss,y+ss,ss,m,smap);
	}
}
void smooth(uchar **m,int smap,int s){
	int i,x,y;
	for (i=0; i<s; i++)
		for (y=0; y<smap; y++)
			for (x=0; x<smap; x++)
				(*m)[x+y*smap]=((*m)[x+y*smap]+(*m)[(x==smap-1?0:x+1)+y*smap]+(*m)[x+(y==smap-1?0:y+1)*smap])/3;
}
void genemap(uchar **m, int s, int mapzmax, int smap) {
	int i;
	int zmin=MAXINT,zmax=-MAXINT;
	(*m)[0]=255;
	carremap(0,0,smap,*m,smap);
	for (i=0; i<smap*smap; i++) {
		int z=(*m)[i];
		if (z>zmax) zmax=z;
		if (z<zmin) zmin=z;
	}
	for (i=0; i<smap*smap; i++) (*m)[i]=((*m)[i]-zmin)*(double)mapzmax/(zmax-zmin+1);
	smooth(m,smap,s);
}
void creuse(int dy) {
	float a,ai,xx,yy;
	int x,y;
	a=0; ai=.02; xx=0; yy=WMAP/3;
	do {
		x=xx;
		y=yy;
		map[x+(y+dy)*WMAP]=1;
		x=xx-sin(a);
		y=yy+cos(a);
		map[x+(y+dy)*WMAP]=1;
		xx+=cos(a);
		yy+=sin(a);
		a+=ai;
		if ((ai>0 && a>M_PI/3) || (ai<0 && a<-M_PI/3)) ai=-ai;
	} while (x<WMAP-2 && y>2 && y<WMAP-2);
}

void initmap() {
	int x,y,i;
	pixel colterrain[4] = {
		{ 150,150,20 },	// glouglou
		{ 111,180,215 },	// désert
		{ 40, 140, 70 },	// prairie
		{ 210,210,210 }	// roc
	};
	struct {
		uchar z1, z2;
	} zcolterrain[4] = { {0,3}, {40,50}, {80,151}, {200,255} };//{ {0,52}, {100,151}, {200,255} };
	// ZCOL
	for (y=0; y<4; y++) {
		for (x=zcolterrain[y].z1; x<=zcolterrain[y].z2; x++) {
			zcol[x].r=colterrain[y].r;
			zcol[x].g=colterrain[y].g;
			zcol[x].b=colterrain[y].b;
		}
		if (y<3) for (;x<zcolterrain[y+1].z1; x++) {
			zcol[x].r=colterrain[y].r+(colterrain[y+1].r-colterrain[y].r)/(zcolterrain[y+1].z1-zcolterrain[y].z2)*(x-zcolterrain[y].z2);
			zcol[x].g=colterrain[y].g+(colterrain[y+1].g-colterrain[y].g)/(zcolterrain[y+1].z1-zcolterrain[y].z2)*(x-zcolterrain[y].z2);
			zcol[x].b=colterrain[y].b+(colterrain[y+1].b-colterrain[y].b)/(zcolterrain[y+1].z1-zcolterrain[y].z2)*(x-zcolterrain[y].z2);
		}
	}
	if (Dark) for (x=0; x<256; x++) {
		zcol[x].r-=zcol[x].r>>1;
		zcol[x].g-=zcol[x].g>>2;
		zcol[x].b-=zcol[x].b>>2;
	}
	// MAP GENERALE
	map=(uchar*)calloc(WMAP*WMAP,sizeof(uchar));
	if (Fleuve) creuse(0);
	genemap(&map,0,255,WMAP);
	for (i=0;i<Smooth; i++) {	//7
		smooth(&map,WMAP,1);
		if (Fleuve) creuse(0);
	}
	mapmap=(uchar*)malloc(WMAP*WMAP*sizeof(uchar));
	fomap=(short int*)malloc(WMAP*WMAP*sizeof(short int));
	for (y=0; y<WMAP; y++) {
		for (x=0; x<WMAP; x++) {
			int z=map[x+y*WMAP];
			fomap[x+y*WMAP]=-1;
			if (z>200) {
				mapmap[x+y*WMAP]=4+randK()*2;
			} else if (z<100) {
				if (z>15) mapmap[x+y*WMAP]=6+randK()*3;
				else mapmap[x+y*WMAP]=9;
			} else {
				mapmap[x+y*WMAP]=1+randK()*3;
			}
		}
	}
	// SOUSMAPS
	for (i=0; i<10; i++) sousmap[i]=(uchar*)calloc(SMAP2*SMAP2,sizeof(uchar));
	genemap(&sousmap[0],10,5,SMAP2);
	genemap(&sousmap[1],2,100,SMAP2);
	genemap(&sousmap[2],2,190,SMAP2);
	genemap(&sousmap[3],2,230,SMAP2);
	genemap(&sousmap[4],1,250,SMAP2);
	genemap(&sousmap[5],1,250,SMAP2);
	genemap(&sousmap[6],3,253,SMAP2);
	genemap(&sousmap[7],3,253,SMAP2);
	genemap(&sousmap[8],2,253,SMAP2);
	genemap(&sousmap[9],0,2,SMAP2);	// SPECIAL FLOTTE
	for (i=0; i<10; i++) {
		sousmapint[i]=(char*)malloc(SMAP2*SMAP2*sizeof(char));
		for (y=0; y<SMAP2; y++) {
			for (x=0; x<SMAP2; x++) {
				double in;
				int z=sousmap[i][x+y*SMAP2];
				if (x) in=((z-sousmap[i][x-1+y*SMAP2])/200.+1.)*.5;
				else in=((z-sousmap[i][SMAP2-1+y*SMAP2])/200.+1.)*.5;
				if (in<0) in=0;
				else if (in>1) in=1;
				sousmapint[i][x+y*SMAP2]=90*(in-.5);
			}
		}
	}
}
float Fphix=0,Fphiy=0,Fphix2=0;
void bougeflotte() {
	int x,y;
	float sy;
	for (y=0; y<SMAP2; y++) {
		sy=sin(y*2*M_PI/SMAP2+Fphiy);
		for (x=0; x<SMAP2; x++) {
			sousmap[9][x+(y<<NMAP2)]=128+60*(sin(x*4*M_PI/SMAP2+Fphix)+sy);
			sousmapint[9][x+(y<<NMAP2)]=8*(cos(x*4*M_PI/SMAP2+Fphix)+sy);
		}
	}
	Fphiy+=.03*AccelFactor;
	Fphix+=.11*AccelFactor;
}

#define H (32<<8)

void coupe (vecic *p1, vecic *p2, vecic *pr) {
	pr->x=p1->x+( ( (H-p1->z) * (((p2->x-p1->x)<<8)/(p2->z-p1->z)) )>>8 );
	pr->y=p1->y+( ( (H-p1->z) * (((p2->y-p1->y)<<8)/(p2->z-p1->z)) )>>8 );
	pr->z=H;
}

void poly (veci *p1, veci *p2, veci *p3) {
	int coulpoly;
	vect2d l1,l2,l3;
	l1.x=_DX+(p1->x*_DX)/p1->z;
	l1.y=_DY+(p1->y*_DX)/p1->z;
	l2.x=_DX+(p2->x*_DX)/p2->z;
	l2.y=_DY+(p2->y*_DX)/p2->z;
	l3.x=_DX+(p3->x*_DX)/p3->z;
	l3.y=_DY+(p3->y*_DX)/p3->z;
	polyflat(&l1,&l2,&l3,&coulpoly);
}

void polyclip(vecic *p1, vecic *p2, vecic *p3) {
	int i;
	vecic pp1,pp2;
	i=p1->z<H;
	i+=(p2->z<H)<<1;
	i+=(p3->z<H)<<2;
	switch (i) {
	case 0:
		poly(p1,p2,p3);
		break;
	case 1:
		coupe(p1,p2,&pp1);
		coupe(p1,p3,&pp2);
		poly(&pp1,p2,p3);
		poly(&pp1,p3,&pp2);
		break;
	case 2:
		coupe(p2,p3,&pp1);
		coupe(p2,p1,&pp2);
		poly(&pp1,p3,p1);
		poly(&pp1,p1,&pp2);
		break;
	case 3:
		coupe(p1,p3,&pp1);
		coupe(p2,p3,&pp2);
		poly(&pp1,&pp2,p3);
		break;
	case 4:
		coupe(p3,p1,&pp1);
		coupe(p3,p2,&pp2);
		poly(&pp1,p1,p2);
		poly(&pp1,p2,&pp2);
		break;
	case 5:
		coupe(p1,p2,&pp1);
		coupe(p3,p2,&pp2);
		poly(&pp2,&pp1,p2);
		break;
	case 6:
		coupe(p2,p1,&pp1);
		coupe(p3,p1,&pp2);
		poly(&pp1,&pp2,p1);
		break;
	}
}
int sxx,syx,sxy,syy,coli,x,y,x2,y2,direct;
int dirx, diry;
veci mx,my,mz;
#define MASK2(a) ((a)&(SMAP2-1))
#define MASK(a) ((a)&(SMAP-1))
#define MASKW(a) ((a)&(WMAP-1))
uchar AddSatB(int a, int b) {
	int c=a+b;
	if (c<10) c=10;
	else if (c>245) c=245;
	return c;
}
static int calcasm(int a, int b, int c)
{
	return (((long long)b*c)>>13)+a;
}
void remap(veci coin, int z1, pixel i1, int z2, pixel i2, int z3, pixel i3, int z4, pixel i4, int m) {
	vecic ptsi[(SMAP2+1)*2];
	int xx,yx,xy,yy, dx,dy, ay;
	veci coinp;
	veci nmx,nmy;
	int zi, zj, zz, dzz;
	int dzi = ((z2-z3)<<8)>>NMAP2;
	int dzj = ((z1-z4)<<8)>>NMAP2;
	int ri, rj, rr, drr;
	int dri = ((i2.r-i3.r)<<8)>>NMAP2;
	int drj = ((i1.r-i4.r)<<8)>>NMAP2;
	int gi, gj, gg, dgg;
	int dgi = ((i2.g-i3.g)<<8)>>NMAP2;
	int dgj = ((i1.g-i4.g)<<8)>>NMAP2;
	int bi, bj, bb, dbb;
	int dbi = ((i2.b-i3.b)<<8)>>NMAP2;
	int dbj = ((i1.b-i4.b)<<8)>>NMAP2;
	nmx.x=mx.x>>NMAP2;
	nmy.x=my.x>>NMAP2;
	nmx.y=mx.y>>NMAP2;
	nmy.y=my.y>>NMAP2;
	nmx.z=mx.z>>NMAP2;
	nmy.z=my.z>>NMAP2;
	for (xy=x2, yy=y2, dy=0, ay=0, zi=z3<<8, zj=z4<<8, ri=i3.r<<8, rj=i4.r<<8, gi=i3.g<<8, gj=i4.g<<8, bi=i3.b<<8, bj=i4.b<<8; dy<=SMAP2; dy++, xy+=sxy, yy+=syy, zi+=dzi, zj+=dzj, ri+=dri, rj+=drj, gi+=dgi, gj+=dgj, bi+=dbi, bj+=dbj, ay^=SMAP2+1) {
		coinp.x=coin.x;
		coinp.y=coin.y;
		coinp.z=coin.z;
		dzz = (zj-zi)>>NMAP2;
		drr = (rj-ri)>>NMAP2;
		dgg = (gj-gi)>>NMAP2;
		dbb = (bj-bi)>>NMAP2;
		for (xx=xy, yx=yy, dx=0, zz=zi, rr=ri, gg=gi, bb=bi; dx<=SMAP2; dx++, xx+=sxx, yx+=syx, zz+=dzz, rr+=drr, gg+=dgg, bb+=dbb) {
			int mm=m;
			int a=dx+ay, b=MASK2(xx)+(MASK2(yx)<<NMAP2);
			int z;
			int nmap;
			if (dx==SMAP2) mm+=sxx+(syx<<NWMAP);
			if (dy==SMAP2) mm+=sxy+(syy<<NWMAP);
			nmap=mapmapm(mm);
			z=(sousmap[nmap][b]<<10)+(zz>>8);
			ptsi[a].c.r=AddSatB(sousmapint[nmap][b],rr>>8);
			ptsi[a].c.g=AddSatB(sousmapint[nmap][b],gg>>8);
			ptsi[a].c.b=AddSatB(sousmapint[nmap][b],bb>>8);
			ptsi[a].x=calcasm(coinp.x,z,mz.x);
			ptsi[a].y=calcasm(coinp.y,z,mz.y);
			ptsi[a].z=calcasm(coinp.z,z,mz.z);
			if (dx && dy) {
				polyclip(&ptsi[a-1+direct],coli?&ptsi[dx+(ay^(SMAP2+1))-1]:&ptsi[dx+(ay^(SMAP2+1))],&ptsi[a-direct]);
				polyclip(coli?&ptsi[a]:&ptsi[a-1],&ptsi[dx+(ay^(SMAP2+1))-1+direct],&ptsi[dx+(ay^(SMAP2+1))-direct]);
			}
			switch (dirx) {
			case 1:
				addvi(&coinp,&nmx);
				break;
			case 2:
				subvi(&coinp,&nmx);
				break;
			case 3:
				addvi(&coinp,&nmy);
				break;
			case 4:
				subvi(&coinp,&nmy);
				break;
			}
		}
		switch (diry) {
		case 1:
			addvi(&coin,&nmx);
			break;
		case 2:
			subvi(&coin,&nmx);
			break;
		case 3:
			addvi(&coin,&nmy);
			break;
		case 4:
			subvi(&coin,&nmy);
			break;
		}
	}
}
void rendusol() {
	vecic ptsi[(SMAP+1)*2];
	int pz[(SMAP+1)*2];
	int i, ay, oldz;
	int xx,yx,xy,yy,xk,yk, dx,dy;
	int dmx=0,dmy=0;
	int lastcare[9], lcidx=0;
	veci coin, coinp;
	vector c;
	i=(obj[0].rot.z.x>0);
	i+=(obj[0].rot.z.y>0)<<1;
	i+=(fabs(obj[0].rot.z.x)>fabs(obj[0].rot.z.y))<<2;
	switch(i) {
	case 0:
		x=0; y=0; sxx=1; syx=0; sxy=0; syy=1; coli=0; dirx=1; diry=3; dmx=-1; dmy=-1; direct=1; break;
	case 1:
		x=SMAP; y=0; sxx=-1; syx=0; sxy=0; syy=1; coli=1; dirx=2; diry=3; dmx=0; dmy=-1; direct=0; break;
	case 2:
		x=0; y=SMAP; sxx=1; syx=0; sxy=0; syy=-1; coli=1; dirx=1; diry=4; dmx=-1; dmy=0; direct=0; break;
	case 3:
		x=SMAP; y=SMAP; sxx=-1; syx=0; sxy=0; syy=-1; coli=0; dirx=2; diry=4; dmx=dmy=0; direct=1; break;
	case 4:
		x=0; y=0; sxx=0; syx=1; sxy=1; syy=0; coli=0; dirx=3; diry=1; dmx=-1; dmy=-1; direct=0; break;
	case 5:
		x=SMAP; y=0; sxx=0; syx=1; sxy=-1; syy=0; coli=1; dirx=3; diry=2; dmx=0; dmy=-1; direct=1; break;
	case 6:
		x=0; y=SMAP; sxx=0; syx=-1; sxy=1; syy=0; coli=1; dirx=4; diry=1; dmx=-1; dmy=0; direct=1; break;
	case 7:
		x=SMAP; y=SMAP; sxx=0; syx=-1; sxy=-1; syy=0; coli=0; dirx=4; diry=2; dmx=0; dmy=0; direct=0; break;
	}
	if (x) x2=SMAP2; else x2=0;
	if (y) y2=SMAP2; else y2=0;
	x+=-(SMAP>>1)+(int)(obj[0].pos.x/ECHELLE);
	y+=-(SMAP>>1)+(int)(obj[0].pos.y/ECHELLE);
	c.x=x*ECHELLE;
	c.y=y*ECHELLE;
	c.z=0;
	x+=WMAP>>1;
	y+=WMAP>>1;
	subv(&c,&obj[0].pos);
	mulmtv(&obj[0].rot,&c,&c);
	coin.x=c.x*256;
	coin.y=c.y*256;
	coin.z=c.z*256;
	mx.x=obj[0].rot.x.x*ECHELLE*256;
	my.x=obj[0].rot.x.y*ECHELLE*256;
	mx.y=obj[0].rot.y.x*ECHELLE*256;
	my.y=obj[0].rot.y.y*ECHELLE*256;
	mx.z=obj[0].rot.z.x*ECHELLE*256;
	my.z=obj[0].rot.z.y*ECHELLE*256;
	mz.x=obj[0].rot.x.z*8192.;
	mz.y=obj[0].rot.y.z*8192.;
	mz.z=obj[0].rot.z.z*8192.;
	xk=(int)floor(obj[0].pos.x/ECHELLE)+(WMAP>>1)+1;
	yk=(int)floor(obj[0].pos.y/ECHELLE)+(WMAP>>1)+1;
	MMXSaveFPU();
	for (xy=x, yy=y, dy=0, ay=0; dy<=SMAP; dy++, ay^=SMAP+1, xy+=sxy, yy+=syy) {
		coinp.x=coin.x;
		coinp.y=coin.y;
		coinp.z=coin.z;
		for (xx=xy, yx=yy, dx=0; dx<=SMAP; dx++, xx+=sxx, yx+=syx) {
			int a=dx+ay, b=MASKW(xx)+(MASKW(yx)<<NWMAP), bb=MASKW(xx+dmx)+(MASKW(yx+dmy)<<NWMAP);
			int ob=MASKW(xx-1)+(MASKW(yx)<<NWMAP), intens;
			uchar rm=0;
			int z;
			z=map[b];
			pz[a]=z<<14;
			ptsi[a].x=calcasm(coinp.x,z<<14,mz.x);
			ptsi[a].y=calcasm(coinp.y,z<<14,mz.y);
			ptsi[a].z=calcasm(coinp.z,z<<14,mz.z);
			intens=((z-map[ob]))+32+64;
			if (intens<64) intens=64;
			else if (intens>127) intens=127;
			ptsi[a].c.r=(zcol[z].r*intens)>>7;
			ptsi[a].c.g=(zcol[z].g*intens)>>7;
			ptsi[a].c.b=(zcol[z].b*intens)>>7;
#define KVISU 4
#define KREMAP 3
			if (xk-xx>-KREMAP && xk-xx<KREMAP && yk-yx>-KREMAP && yk-yx<KREMAP) {
				if (dx && dy) {
					veci c;
					c.x=coinp.x;
					c.y=coinp.y;
					c.z=coinp.z;
					switch (dirx) {
					case 1:
						subvi(&c,&mx);
						break;
					case 2:
						addvi(&c,&mx);
						break;
					case 3:
						subvi(&c,&my);
						break;
					case 4:
						addvi(&c,&my);
						break;
					}
					switch (diry) {
					case 1:
						subvi(&c,&mx);
						break;
					case 2:
						addvi(&c,&mx);
						break;
					case 3:
						subvi(&c,&my);
						break;
					case 4:
						addvi(&c,&my);
						break;
					}
					Direct=0;
					remap(c,pz[a],ptsi[a].c,pz[a-1],ptsi[a-1].c,pz[dx+(ay^(SMAP+1))-1],ptsi[dx+(ay^(SMAP+1))-1].c,pz[dx+(ay^(SMAP+1))],ptsi[dx+(ay^(SMAP+1))].c,bb);
					MMXRestoreFPU();
					if (Direct && (mapmap[bb]&0x80)) {	// est-ce vraient utile de remapper quand c'est plat ?
						drawroute(bb);
					}
				} else MMXRestoreFPU();
				rm=1;
			} else rm=0;
			if (!rm) {
				if (dx && dy) {
					Direct=0;
					polyclip(&ptsi[a-1+direct],coli?&ptsi[dx+(ay^(SMAP+1))-1]:&ptsi[dx+(ay^(SMAP+1))],&ptsi[a-direct]);
					polyclip(coli?&ptsi[a]:&ptsi[a-1],&ptsi[dx+(ay^(SMAP+1))-1+direct],&ptsi[dx+(ay^(SMAP+1))-direct]);
					MMXRestoreFPU();
					if (Direct && (mapmap[bb]&0x80)) {
						drawroute(bb);
					}
				} else MMXRestoreFPU();
			} else MMXRestoreFPU();
			//MMXRestoreFPU();
			if (xk-xx>-KVISU && xk-xx<KVISU && yk-yx>-KVISU && yk-yx<KVISU) {
				if (fomap[bb]!=-1) {
					if (xk-xx<-1 || xk-xx>1 || yk-yx<-1 || yk-yx>1) renderer(bb,3);
					else {
						renderer(bb,0);
						lastcare[lcidx++]=bb;
					}
				}
			} else renderer(bb,1);
			MMXSaveFPU();
			switch (dirx) {
			case 1:
				addvi(&coinp,&mx);
				break;
			case 2:
				subvi(&coinp,&mx);
				break;
			case 3:
				addvi(&coinp,&my);
				break;
			case 4:
				subvi(&coinp,&my);
				break;
			}
			if (coinp.z<-((int)ECHELLE<<10)) break;
			oldz=z;
		}
		switch (diry) {
		case 1:
			addvi(&coin,&mx);
			break;
		case 2:
			subvi(&coin,&mx);
			break;
		case 3:
			addvi(&coin,&my);
			break;
		case 4:
			subvi(&coin,&my);
			break;
		}
		if (coin.z<-((int)ECHELLE<<10)) break;
	}
	MMXRestoreFPU();
	for (i=0; i<lcidx; i++) renderer(lastcare[i],2);
}
#define Gourovf 8	// NE PAS CHANGER !
#define Gourovfm (1>>Gourovf)
int Gouroxi, Gouroyi, Gourolx, Gouroql, Gouroqx, Gouroqr, Gouroqg, Gouroqb, Gourocoulr, Gourocoulg, Gourocoulb, *Gourovid, Gourody, Gouroir, Gouroig, Gouroib;
/*
void Gouro() {
	int cr, cg, cb;
	MMXGouro();
	if (Gouroyi<0) {
		if (Gouroyi<-Gourody) {
			Gouroyi+=Gourody;
			Gouroxi+=Gourody*Gouroqx;
			Gourolx+=Gourody*Gouroql;
			Gourocoulr+=Gourody*Gouroqr;
			Gourocoulg+=Gourody*Gouroqg;
			Gourocoulb+=Gourody*Gouroqb;
			return;
		} else {
			Gouroxi-=Gouroyi*Gouroqx;
			Gourolx-=Gouroyi*Gouroql;
			Gourocoulr-=Gouroyi*Gouroqr;
			Gourocoulg-=Gouroyi*Gouroqg;
			Gourocoulb-=Gouroyi*Gouroqb;
			Gourody+=Gouroyi;
			Gouroyi=0;
		}
	}
	Gourovid=(int*)videobuffer+Gouroyi*SX;
	while (Gourody>0 && Gouroyi<SY) {
		i=Gouroxi>>Gourovf; ilim=i-(Gourolx>>Gourovf);
		if (ilim<0) ilim=0;
		cr=Gourocoulr;
		cg=Gourocoulg;
		cb=Gourocoulb;
		if (i>=SX) {
			cr+=(i-SX+1)*Gouroir;
			cg+=(i-SX+1)*Gouroig;
			cb+=(i-SX+1)*Gouroib;
			i=SX-1;
		}
		if (i>=ilim) {
			do {
				Gourovid[i]=((cb>>Gourovf)&0xFF)+(((cg>>Gourovf)&0xFF)<<8)+(((cr>>Gourovf)&0xFF)<<16);
				i--;
				cr+=Gouroir; cb+=Gouroib; cg+=Gouroig;
			} while (i>=ilim);
		}
		Gouroxi+=Gouroqx;
		Gourolx+=Gouroql;
		Gourocoulr+=Gouroqr;
		Gourocoulg+=Gouroqg;
		Gourocoulb+=Gouroqb;
		Gourovid+=SX;
		Gourody--;
		Gouroyi++;
	}
}*/
void MMXGouro(void) {}
void MMXGouroPreca(int ib, int ig, int ir) {}

void polygouro(vect2dc *p1, vect2dc *p2, vect2dc *p3) {
	vect2dc *tmp, *pmax, *pmin;
	int q1, q2, q3=0, qxx, ql2;
	int qr1, qr2, qr3=0, qg1, qg2, qg3=0, qb1, qb2, qb3=0, qrr, qgg, qbb;
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
	Gouroyi=p1->y;
	Direct=1;
	if (p1->y!=p2->y) {
		Gouroxi=p1->x<<Gourovf;
		Gourocoulb=p1->c.b<<Gourovf;
		Gourocoulg=p1->c.g<<Gourovf;
		Gourocoulr=p1->c.r<<Gourovf;
		q1=((p2->x-p1->x)<<Gourovf)/(p2->y-p1->y);
		qr1=(((int)(p2->c.r-p1->c.r))<<Gourovf)/(p2->y-p1->y);
		qg1=(((int)(p2->c.g-p1->c.g))<<Gourovf)/(p2->y-p1->y);
		qb1=(((int)(p2->c.b-p1->c.b))<<Gourovf)/(p2->y-p1->y);
		q2=((p3->x-p1->x)<<Gourovf)/(p3->y-p1->y);
		qr2=(((int)(p3->c.r-p1->c.r))<<Gourovf)/(p3->y-p1->y);
		qg2=(((int)(p3->c.g-p1->c.g))<<Gourovf)/(p3->y-p1->y);
		qb2=(((int)(p3->c.b-p1->c.b))<<Gourovf)/(p3->y-p1->y);
		if (p3->y-p2->y) {
			q3=((p3->x-p2->x)<<Gourovf)/(p3->y-p2->y);
			qr3=(((int)(p3->c.r-p2->c.r))<<Gourovf)/(p3->y-p2->y);
			qg3=(((int)(p3->c.g-p2->c.g))<<Gourovf)/(p3->y-p2->y);
			qb3=(((int)(p3->c.b-p2->c.b))<<Gourovf)/(p3->y-p2->y);
		}
		Gourolx = Gourovfm;
		if (q1<=q2) {
			if(!(Gouroql=q2-q1)) Gouroql=1;		// le taux d'accroissement de la taille du segment (en évitant 0);
			if(!(ql2=q2-q3)) ql2=-1;
			Gouroqx=q2; qxx=q2;
			Gouroqr=qr2; Gouroqg=qg2; Gouroqb=qb2; qrr=qr2; qgg=qg2; qbb=qb2;
			if (p2->y-p1->y>p3->y-p2->y) {
#define QLPREC (Gourovfm/4)
				if (Gouroql>QLPREC) {
					Gouroir=((qr1-qr2)<<Gourovf)/Gouroql;
					Gouroig=((qg1-qg2)<<Gourovf)/Gouroql;
					Gouroib=((qb1-qb2)<<Gourovf)/Gouroql;
				} else Gouroir=Gouroig=Gouroib=0;
			} else {
				if (ql2<-QLPREC) {
					Gouroir=((qr3-qr2)<<Gourovf)/ql2;
					Gouroig=((qg3-qg2)<<Gourovf)/ql2;
					Gouroib=((qb3-qb2)<<Gourovf)/ql2;
				} else Gouroir=Gouroig=Gouroib=0;
			}
		} else {	// q1>q2
			if (!(Gouroql=q1-q2)) Gouroql=1;
			if (!(ql2=q3-q2)) ql2=-1;
			Gouroqx=q1; qxx=q3;
			Gouroqr=qr1; Gouroqg=qg1; Gouroqb=qb1; qrr=qr3; qgg=qg3; qbb=qb3;
			if (p2->y-p1->y>p3->y-p2->y) {
				if (Gouroql>QLPREC) {
					Gouroir=((qr2-qr1)<<Gourovf)/Gouroql;
					Gouroig=((qg2-qg1)<<Gourovf)/Gouroql;
					Gouroib=((qb2-qb1)<<Gourovf)/Gouroql;
				} else Gouroir=Gouroig=Gouroib=0;
			} else {
				if (ql2<-QLPREC) {
					Gouroir=((qr2-qr3)<<Gourovf)/ql2;
					Gouroig=((qg2-qg3)<<Gourovf)/ql2;
					Gouroib=((qb2-qb3)<<Gourovf)/ql2;
				} else Gouroir=Gouroig=Gouroib=0;
			}
		}
		MMXGouroPreca(Gouroib,Gouroig,Gouroir);
		Gourody=p2->y-p1->y;
		MMXGouro();
		Gouroqx=qxx; Gouroql=ql2;
		Gouroqr=qrr; Gouroqg=qgg; Gouroqb=qbb;
		Gourody=p3->y-p2->y+1;
		MMXGouro();
	} else {	// base plate
		if (p3->y>p2->y) {	// triangle qd meme
			q2=((p3->x-p1->x)<<Gourovf)/(p3->y-p1->y);
			qr2=(((int)(p3->c.r-p1->c.r))<<Gourovf)/(p3->y-p1->y);
			qg2=(((int)(p3->c.g-p1->c.g))<<Gourovf)/(p3->y-p1->y);
			qb2=(((int)(p3->c.b-p1->c.b))<<Gourovf)/(p3->y-p1->y);
			q3=((p3->x-p2->x)<<Gourovf)/(p3->y-p2->y);
			qr3=(((int)(p3->c.r-p2->c.r))<<Gourovf)/(p3->y-p2->y);
			qg3=(((int)(p3->c.g-p2->c.g))<<Gourovf)/(p3->y-p2->y);
			qb3=(((int)(p3->c.b-p2->c.b))<<Gourovf)/(p3->y-p2->y);
			if (p2->x>=p1->x) {
				Gouroxi=p2->x<<Gourovf;
				Gourocoulb=p2->c.b<<Gourovf;
				Gourocoulg=p2->c.g<<Gourovf;
				Gourocoulr=p2->c.r<<Gourovf;
				Gourolx = (p2->x-p1->x)<<Gourovf;
				if(!(Gouroql=q3-q2)) Gouroql=-1;
				Gouroqx=q3;
				Gouroqr=qr3; Gouroqg=qg3; Gouroqb=qb3;
				if (Gouroql<-QLPREC) {
					Gouroir=((qr2-qr3)<<Gourovf)/Gouroql;
					Gouroig=((qg2-qg3)<<Gourovf)/Gouroql;
					Gouroib=((qb2-qb3)<<Gourovf)/Gouroql;
				} else Gouroir=Gouroig=Gouroib=0;
			} else {
				Gouroxi=p1->x<<Gourovf;
				Gourocoulb=p1->c.b<<Gourovf;
				Gourocoulg=p1->c.g<<Gourovf;
				Gourocoulr=p1->c.r<<Gourovf;
				Gourolx = (p1->x-p2->x)<<Gourovf;
				if(!(Gouroql=q2-q3)) Gouroql=-1;
				Gouroqx=q2;
				Gouroqr=qr2; Gouroqg=qg2; Gouroqb=qb2;
				if (Gouroql<-QLPREC) {
					Gouroir=((qr3-qr2)<<Gourovf)/Gouroql;
					Gouroig=((qg3-qg2)<<Gourovf)/Gouroql;
					Gouroib=((qb3-qb2)<<Gourovf)/Gouroql;
				} else Gouroir=Gouroib=Gouroig=0;
			}
			MMXGouroPreca(Gouroib,Gouroig,Gouroir);
			Gourody=p3->y-p1->y+1;
			MMXGouro();
		} else {	// trait plat
			Gouroxi=pmax->x<<Gourovf;
			if(!(Gourolx=(pmax->x-pmin->x)<<Gourovf)) Gourolx=Gourovfm;
			Gourocoulb=pmax->c.b<<Gourovf;
			Gourocoulg=pmax->c.g<<Gourovf;
			Gourocoulr=pmax->c.r<<Gourovf;
			if (Gourolx>QLPREC) {
				Gouroir=(pmin->c.r-pmax->c.r)<<Gourovf/Gourolx;
				Gouroig=(pmin->c.g-pmax->c.g)<<Gourovf/Gourolx;
				Gouroib=(pmin->c.b-pmax->c.b)<<Gourovf/Gourolx;
			} else Gouroir=Gouroig=Gouroib=0;
			MMXGouroPreca(Gouroib,Gouroig,Gouroir);
			Gourody=1;
			MMXGouro();
		}
	}
//	MMXRestoreFPU();
}

float zsol(float x, float y) {	// renvoit les coords du sol à cette pos
	int zi,zj,mzi,mzj;
	int xi, xx=x*16.+(((WMAP<<NECHELLE)>>1)<<4), medx, minx;
	int yi, yy=y*16.+(((WMAP<<NECHELLE)>>1)<<4), medy, miny;
	int iix,iiy,ii,i;
	int mz1,mz2,mz3,mz4;
	xi=xx>>(NECHELLE+4);
	yi=yy>>(NECHELLE+4);
	i=xi+(yi<<NWMAP);
#define z1 ((int)map[i]<<(14-NECHELLE+5))
#define z2 ((int)map[i+WMAP]<<(14-NECHELLE+5))
#define z3 ((int)map[i+WMAP+1]<<(14-NECHELLE+5))
#define z4 ((int)map[i+1]<<(14-NECHELLE+5))
	medx=xx&((ECHELLE<<4)-1);
	medy=yy&((ECHELLE<<4)-1);
	zi=((medy*(z2-z1))>>(NECHELLE+4))+z1;	//z est sur 8 bits, delta z peut etre sur 3 seulement ?
	zj=((medy*(z3-z4))>>(NECHELLE+4))+z4;
	iix=(medx<<NMAP2)>>(NECHELLE+4);
	iiy=(medy<<NMAP2)>>(NECHELLE+4);
	ii=iix+(iiy<<NMAP2);
	mz1=((int)sousmap[mapmapm(i)][ii]<<(10+5-NECHELLE+NMAP2));
	if (iiy!=SMAP2-1) {
		mz2=((int)sousmap[mapmapm(i)][ii+SMAP2]<<(10+5-NECHELLE+NMAP2));
		if (iix!=SMAP2-1) {
			mz3=((int)sousmap[mapmapm(i)][ii+SMAP2+1]<<(10+5-NECHELLE+NMAP2));
			mz4=((int)sousmap[mapmapm(i)][ii+1]<<(10+5-NECHELLE+NMAP2));
		} else {
			mz3=((int)sousmap[mapmapm(i+1)][(iiy+1)<<NMAP2]<<(10+5-NECHELLE+NMAP2));
			mz4=((int)sousmap[mapmapm(i+1)][iiy<<NMAP2]<<(10+5-NECHELLE+NMAP2));
		}
	} else {
		mz2=((int)sousmap[mapmapm(i+WMAP)][iix]<<(10+5-NECHELLE+NMAP2));
		if (iix!=SMAP2-1) {
			mz3=((int)sousmap[mapmapm(i+WMAP)][iix+1]<<(10+5-NECHELLE+NMAP2));
			mz4=((int)sousmap[mapmapm(i)][ii+1]<<(10+5-NECHELLE+NMAP2));
		} else {
			mz3=((int)sousmap[mapmapm(i+WMAP+1)][0]<<(10+5-NECHELLE+NMAP2));
			mz4=((int)sousmap[mapmapm(i+1)][iiy<<NMAP2]<<(10+5-NECHELLE+NMAP2));
		}
	}
	minx=medx&((1<<(NECHELLE+4-NMAP2))-1);
	miny=medy&((1<<(NECHELLE+4-NMAP2))-1);
	mzi=((miny*(mz2-mz1))>>(NECHELLE+4-NMAP2))+mz1;
	mzj=((miny*(mz3-mz4))>>(NECHELLE+4-NMAP2))+mz4;
	return ((((medx*(zj-zi))>>4)+(zi<<NECHELLE)) + ((minx*(mzj-mzi))>>4)+(mzi<<(NECHELLE-NMAP2)))/8192.;
}

float zsolraz(float x, float y) {	// renvoit les coords du sol à cette pos, sans tenir compte de la sousmap
	int zi,zj;
	int xi, xx=x*16.+(((WMAP<<NECHELLE)>>1)<<4), medx, minx;
	int yi, yy=y*16.+(((WMAP<<NECHELLE)>>1)<<4), medy, miny;
	int iix,iiy,i;
	xi=xx>>(NECHELLE+4);
	yi=yy>>(NECHELLE+4);
	i=xi+(yi<<NWMAP);
#define z1 ((int)map[i]<<(14-NECHELLE+5))
#define z2 ((int)map[i+WMAP]<<(14-NECHELLE+5))
#define z3 ((int)map[i+WMAP+1]<<(14-NECHELLE+5))
#define z4 ((int)map[i+1]<<(14-NECHELLE+5))
	medx=xx&((ECHELLE<<4)-1);
	medy=yy&((ECHELLE<<4)-1);
	zi=((medy*(z2-z1))>>(NECHELLE+4))+z1;
	zj=((medy*(z3-z4))>>(NECHELLE+4))+z4;
	iix=(medx<<NMAP2)>>(NECHELLE+4);
	iiy=(medy<<NMAP2)>>(NECHELLE+4);
	minx=medx&((1<<(NECHELLE+4-NMAP2))-1);
	miny=medy&((1<<(NECHELLE+4-NMAP2))-1);
	return ((((medx*(zj-zi))>>4)+(zi<<NECHELLE)) /*+ ((minx*(mzj-mzi))>>4)+(mzi<<(NECHELLE-NMAP2))*/)/8192.;
}
