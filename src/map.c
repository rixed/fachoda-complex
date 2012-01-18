#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <values.h>
#include <assert.h>
#include "map.h"

/* The map is the central structure of the game.
 * It's a simple square heightfield of unsigned bytes
 * which side length is a power of 2.
 *
 * There are 11 of these maps.
 * The most important one is the global map, of side length WMAP (128).
 * The other ones are the submaps, of length SMAP2 (16), representing the height of
 * each world map tile.
 * We have 10 of them, 9 is for water (animated), while 0
 * is completely flat for the airfields and roads. Each other tile of
 * the global map that is neither water nor airfield is
 * attributed one of the other 1 to 8 submaps according to its
 * height, with some random.
 *
 * So all in all we need very few memory for the world map (less than 20Kb).
 */

struct tile *map;	// The global map: a square of WMAP*WMAP tiles.

static uchar *submap[10];	// The submaps (same structure as the global map, only smaller: SMAP2*SMAP2)

/* Each vector of a submap heighfield have a specific lightning (to be added to the
 * lightning of the tile). It's faster than to compute a proper lightning for every
 * polygon!
 */
static char *submap_rel_light[ARRAY_LEN(submap)];

int submap_get(int k)
{
	return map[k].has_road ? 0 : map[k].submap;
}

/* This is an awfull hack to skip drawing of roads when the ground is seen from below
 * (for instance, roads on the other side of a hill). Problems:
 * - this flag is set by the polygouro function, while it should be set by the draw_ground
 *   method itself, which should choose internaly whether to draw road or not (based on this
 *   and distance, for instance)
 * - this currently works because roads are only possible on flat submap (thus the last polygouro
 *   will have the correct information)
 */
static int last_poly_is_visible;

/* Ground colors are chosen merely based on the height of the map.
 * So this array gives the color of any altitude.
 * It's also used to draw the ingame map.
 */
pixel zcol[256];

/*
 * Map Generation
 */

// Generate the heighfield for a portion of the map (recursive)
static void random_submap(
	int x, int y,	// location of the starting corner of the square to randomize
	int s,			// length of the square to randomize
	size_t z_off,	// offset between any two consecutive z bytes along x
	uchar *m,		// first z
	int smap        // length ot the total map
) {
	int ss=s>>1;
	if (! ss) return;

	double sf=Accident*(double)s/smap;
	// offset of first corner of the square
	size_t const sx = z_off * x;
	size_t const sy = z_off * y * smap;
	// offset of the last corner of the square (wrapped)
	size_t const ey = y + s == smap ? 0 : z_off * (y + s);
	size_t const ex = x + s == smap ? 0 : z_off * (x + s);
	// offset of the center of the square
	size_t const mx = sx + z_off * ss;
	size_t const my = sy + z_off * ss * smap;

	if (m[mx + sy] == 0)
		m[mx + sy] = (((int)m[sx + sy] + m[ex + sy])>>1) + (drand48()-.5)*sf;
	if (m[ex + my] == 0)
		m[ex + my] = (((int)m[ex + sy] + m[ex + ey])>>1) + (drand48()-.5)*sf;
	if (m[mx + ey] == 0)
		m[mx + ey] = (((int)m[sx + ey] + m[ex + ey])>>1) + (drand48()-.5)*sf;
	if (m[sx + my] == 0)
		m[sx + my] = (((int)m[sx + sy] + m[sx + ey])>>1) + (drand48()-.5)*sf;
	if (m[mx + my] == 0)
		m[mx + my] = (((int)m[mx + sy] + m[ex + my] + m[mx + ey] + m[sx + my])>>2) + (drand48()-.5)*sf;
	// Recurse into each quad
	random_submap(     x,      y, ss, z_off, m, smap);
	random_submap(x + ss,      y, ss, z_off, m, smap);
	random_submap(     x, y + ss, ss, z_off, m, smap);
	random_submap(x + ss, y + ss, ss, z_off, m, smap);
}

static void smooth_map(uchar *m, int smap, int s, size_t z_off){
	for (int i=0; i<s; i++) {
		size_t yo = 0, yn;
		for (int y=0; y<smap; y++, yo=yn) {
			size_t xo = 0, xn;
			yn = y==smap-1 ? 0 : yo + z_off*smap;
			for (int x=0; x<smap; x++, xo=xn) {
				xn = x==smap-1 ? 0 : xo + z_off;
				m[xo+yo] = (m[xo+yo] + m[xn+yo] + m[xo+yn]) / 3;
			}
		}
	}
}

static void make_map(uchar *m, int smooth_factor, int mapzmax, int map_length, size_t z_off) {
	m[0]=255;
	random_submap(0, 0, map_length, z_off, m, map_length);

	/* Scale all heights (remember height are unsigned bytes)
	 * so that we have all values from 0 to mapzmax */
	int zmin=MAXINT, zmax=-MAXINT;
	size_t const map_size = map_length * map_length;
	for (size_t i = 0; i < map_size; i += z_off) {
		int const z = m[i];
		if (z > zmax) zmax = z;
		if (z < zmin) zmin = z;
	}
	double const ratio = (double)mapzmax/(zmax-zmin+1);
	for (size_t i = 0; i < map_size; i += z_off) {
		m[i] = (m[i]-zmin)*ratio;
	}

	// Smooth all maps at least once
	smooth_map(m, map_length, smooth_factor, z_off);
}

// dig a wave in the map (in other words, a river)
// looks alien
static void dig(int dy) {
	float a,ai,xx,yy;
	int x,y;
	a=0; ai=.02; xx=0; yy=WMAP/3;
	do {
		x=xx;
		y=yy;
		map[x+(y+dy)*WMAP].z=1;
		x=xx-sin(a);
		y=yy+cos(a);
		map[x+(y+dy)*WMAP].z=1;
		xx+=cos(a);
		yy+=sin(a);
		a+=ai;
		if ((ai>0 && a>M_PI/3) || (ai<0 && a<-M_PI/3)) ai=-ai;
	} while (x<WMAP-2 && y>2 && y<WMAP-2);
}

void initmap(void) {
	int x,y,i;
	pixel colterrain[4] = {
		{ 150,150,20 },	// glouglou
		{ 111,180,215 },	// d�sert
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

	// Global map
	map = calloc(WMAP*WMAP,sizeof(*map));	// starts from 0
	if (Fleuve) dig(0);	// digging in 0?
	make_map(&map[0].z, 0, 255, WMAP, sizeof(*map));
	for (i=0;i<Smooth; i++) {	// Instead of using make_map smooth_factor, we smooth (and dig) several times.
		smooth_map(&map[0].z, WMAP, 1, sizeof(*map));
		if (Fleuve) dig(0);
	}
	for (y=0; y<WMAP; y++) {
		for (x=0; x<WMAP; x++) {
			map[x+y*WMAP].has_road = 0;
			map[x+y*WMAP].first_obj = -1;
			// init submap according to z
			int const z = map[x+y*WMAP].z;
			if (z > 200) {	// submaps 4 and 5 are for high lands
				map[x+y*WMAP].submap = 4+drand48()*2;
			} else if (z < 100) { // submaps 6, 7, 8 are for middle lands
				if (z > 15) map[x+y*WMAP].submap = 6+drand48()*3;
				else map[x+y*WMAP].submap = 9;
			} else { // submaps 1, 2, 3 are for low lands
				map[x+y*WMAP].submap = 1+drand48()*3;
			}
		}
	}

	// Build submaps
	for (unsigned i = 0; i < ARRAY_LEN(submap); i++) {
		submap[i] = calloc(SMAP2*SMAP2, sizeof(*submap[i]));
	}
	make_map(submap[0], 10,  5, SMAP2, 1);	// flat
	make_map(submap[1], 2, 100, SMAP2, 1);	// mostly flat
	make_map(submap[2], 2, 190, SMAP2, 1);	// mostly flat
	make_map(submap[3], 2, 230, SMAP2, 1);	// mostly flat
	make_map(submap[4], 1, 250, SMAP2, 1);	// mountain tops
	make_map(submap[5], 1, 250, SMAP2, 1);	// mountain tops
	make_map(submap[6], 3, 253, SMAP2, 1);	// middle lands
	make_map(submap[7], 3, 253, SMAP2, 1);	// middle lands
	make_map(submap[8], 2, 253, SMAP2, 1);	// middle lands
	make_map(submap[9], 0,   2, SMAP2, 1);	// reserved for water: we will move this heighfield
	// compure relative lighting.
	for (unsigned i = 0; i < ARRAY_LEN(submap); i++) {
		submap_rel_light[i]=(char*)malloc(SMAP2*SMAP2*sizeof(char));
		for (y=0; y<SMAP2; y++) {
			for (x=0; x<SMAP2; x++) {
				double in;
				int z=submap[i][x+y*SMAP2];
				if (x) in=((z-submap[i][x-1+y*SMAP2])/200.+1.)*.5;
				else in=((z-submap[i][SMAP2-1+y*SMAP2])/200.+1.)*.5;
				if (in<0) in=0;
				else if (in>1) in=1;
				submap_rel_light[i][x+y*SMAP2]=90*(in-.5);
			}
		}
	}
}

void animate_water(void)
{
	static float Fphix=0, Fphiy=0;
	for (int y=0; y<SMAP2; y++) {
		float sy=sin(y*2*M_PI/SMAP2+Fphiy);
		for (int x=0; x<SMAP2; x++) {
			submap[9][x+(y<<NMAP2)]=128+60*(sin(x*4*M_PI/SMAP2+Fphix)+sy);
			submap_rel_light[9][x+(y<<NMAP2)]=8*(cos(x*4*M_PI/SMAP2+Fphix)+sy);
		}
	}
	Fphiy+=.03*AccelFactor;
	Fphix+=.11*AccelFactor;
}

/*
 * Map Rendering
 */

// Clip segment p1-p2 by the frustum zmin plan (p1->z is below FRUSTUM_ZMIN)
#define FRUSTUM_ZMIN (32<<8)
static void do_clip(vecic *p1, vecic *p2, vecic *pr) {
	int const dz1 = FRUSTUM_ZMIN - p1->v.z;
	int const dz2 = p2->v.z - p1->v.z;
	pr->v.x = p1->v.x + ((dz1 * (((p2->v.x-p1->v.x)<<8)/dz2))>>8);
	pr->v.y = p1->v.y + ((dz1 * (((p2->v.y-p1->v.y)<<8)/dz2))>>8);
	pr->v.z = FRUSTUM_ZMIN;
	pr->c.r = p1->c.r + ((dz1 * ((((int)p2->c.r-p1->c.r)<<8)/dz2))>>8);
	pr->c.g = p1->c.g + ((dz1 * ((((int)p2->c.g-p1->c.g)<<8)/dz2))>>8);
	pr->c.b = p1->c.b + ((dz1 * ((((int)p2->c.b-p1->c.b)<<8)/dz2))>>8);
}

static int color_of_pixel(pixel c) {
	return (c.r<<16) + (c.g<<8) + (c.b);
}
static void poly(vecic *p1, vecic *p2, vecic *p3) {
	vect2d l1,l2,l3;
	proji(&l1, &p1->v);	// FIXME: should not project the same pt several times!
	proji(&l2, &p2->v);
	proji(&l3, &p3->v);
	pixel mix = {
		.r = (p1->c.r + p2->c.r + p3->c.r) / 3,
		.g = (p1->c.g + p2->c.g + p3->c.g) / 3,
		.b = (p1->c.b + p2->c.b + p3->c.b) / 3,
	};
	polyflat(&l1, &l2, &l3, color_of_pixel(mix));
}

void polyclip(vecic *p1, vecic *p2, vecic *p3) {
	int i;
	vecic pp1, pp2;
	i  =  p1->v.z < FRUSTUM_ZMIN;
	i += (p2->v.z < FRUSTUM_ZMIN)<<1;
	i += (p3->v.z < FRUSTUM_ZMIN)<<2;
	switch (i) {
	case 0:
		poly(p1,p2,p3);
		break;
	case 1:
		do_clip(p1,p2,&pp1);
		do_clip(p1,p3,&pp2);
		poly(&pp1,p2,p3);
		poly(&pp1,p3,&pp2);
		break;
	case 2:
		do_clip(p2,p3,&pp1);
		do_clip(p2,p1,&pp2);
		poly(&pp1,p3,p1);
		poly(&pp1,p1,&pp2);
		break;
	case 3:
		do_clip(p1,p3,&pp1);
		do_clip(p2,p3,&pp2);
		poly(&pp1,&pp2,p3);
		break;
	case 4:
		do_clip(p3,p1,&pp1);
		do_clip(p3,p2,&pp2);
		poly(&pp1,p1,p2);
		poly(&pp1,p2,&pp2);
		break;
	case 5:
		do_clip(p1,p2,&pp1);
		do_clip(p3,p2,&pp2);
		poly(&pp2,&pp1,p2);
		break;
	case 6:
		do_clip(p2,p1,&pp1);
		do_clip(p3,p1,&pp2);
		poly(&pp1,&pp2,p1);
		break;
	case 7:
		// don't draw anything is all points are below zmin
		break;
	}
}

// These are inited by draw_ground_and_objects() and used also by draw_submap()
static int sxx,syx,sxy,syy,coli,x2,y2,direct;
static int dirx, diry;
static veci d_coinp;	// coinp increment in inner loop
static veci d_coin;	// coin increment in outer loop
static veci mz;	// see mx,my in draw_ground_and_objects()

#define MASK2(a) ((a)&(SMAP2-1))
#define MASK(a)  ((a)&(SMAP-1))
#define MASKW(a) ((a)&(WMAP-1))

// Saturated between 10 and 245
static uchar AddSatB(int a, int b) {
	int c=a+b;
	if (c<10) c=10;
	else if (c>245) c=245;
	return c;
}
static int calcasm(int a, int b, int c)
{
	return (((int64_t)b*c)>>13)+a;
}

/*
 *  p3--p4
 *  |   |
 *  p2--p1
 */
static void draw_submap(veci coin, int z1, pixel i1, int z2, pixel i2, int z3, pixel i3, int z4, pixel i4, int m) {
	vecic ptsi[(SMAP2+1)*2];
	int xx,yx,xy,yy, dx,dy, ay;
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
	veci n_d_coinp = {
		.x = d_coinp.x >> NMAP2,
		.y = d_coinp.y >> NMAP2,
		.z = d_coinp.z >> NMAP2,
	};
	veci n_d_coin = {
		.x = d_coin.x >> NMAP2,
		.y = d_coin.y >> NMAP2,
		.z = d_coin.z >> NMAP2,
	};
	for (
		xy=x2, yy=y2, dy=0, ay=0,
		zi=z3<<8, zj=z4<<8,
		ri=i3.r<<8, rj=i4.r<<8,
		gi=i3.g<<8, gj=i4.g<<8,
		bi=i3.b<<8, bj=i4.b<<8;
		dy<=SMAP2;
		dy++, xy+=sxy, yy+=syy,
		zi+=dzi, zj+=dzj,
		ri+=dri, rj+=drj,
		gi+=dgi, gj+=dgj,
		bi+=dbi, bj+=dbj,
		ay^=SMAP2+1,
		addvi(&coin, &n_d_coin)
	) {
		veci coinp = coin;
		dzz = (zj-zi)>>NMAP2;
		drr = (rj-ri)>>NMAP2;
		dgg = (gj-gi)>>NMAP2;
		dbb = (bj-bi)>>NMAP2;
		for (
			xx=xy, yx=yy, dx=0, zz=zi,
			rr=ri, gg=gi, bb=bi;
			dx<=SMAP2; 
			dx++, xx+=sxx, yx+=syx, zz+=dzz,
			rr+=drr, gg+=dgg, bb+=dbb,
			addvi(&coinp, &n_d_coinp)
		) {
			int mm=m;
			if (dx==SMAP2) mm+=sxx+(syx<<NWMAP);
			if (dy==SMAP2) mm+=sxy+(syy<<NWMAP);
			int const nmap = submap_get(mm);
			int b = MASK2(xx) + (MASK2(yx)<<NMAP2);
			int const z = (submap[nmap][b]<<10) + (zz>>8);
			int const a = dx + ay;
			ptsi[a].c.r=AddSatB(submap_rel_light[nmap][b],rr>>8);
			ptsi[a].c.g=AddSatB(submap_rel_light[nmap][b],gg>>8);
			ptsi[a].c.b=AddSatB(submap_rel_light[nmap][b],bb>>8);
			ptsi[a].v.x=calcasm(coinp.x,z,mz.x);
			ptsi[a].v.y=calcasm(coinp.y,z,mz.y);
			ptsi[a].v.z=calcasm(coinp.z,z,mz.z);
			if (dx && dy) {
				polyclip(&ptsi[a-1+direct],coli?&ptsi[dx+(ay^(SMAP2+1))-1]:&ptsi[dx+(ay^(SMAP2+1))],&ptsi[a-direct]);
				polyclip(coli?&ptsi[a]:&ptsi[a-1],&ptsi[dx+(ay^(SMAP2+1))-1+direct],&ptsi[dx+(ay^(SMAP2+1))-direct]);
			}
		}
	}
}

void draw_ground_and_objects(void)
{
	int x,y;
	int dmx=0,dmy=0;
	int lastcare[9], lcidx=0;	// some tiles we want to draw last

	/* We will draw the world using the painter method.
	 * Notice that the painter method is only local: the painter
	 * does not necessarily sort by distance all what he has to draw and proceed to
	 * draw things in that order. He proceed that way to draw a portion of the painting,
	 * then can switch to another portion of its drawing which distance is unrelated
	 * to the portion he just left. So, he sort only localy.
	 * We do the same here: we do not sort all the world's tiles but merely select a
	 * way to traverse the world map so that we avoid overdrawing an object with another
	 * one that's farther away (remember there are no zbuffer neither).
	 * It's mostly enough to traverse the map along its rows and columns, starting
	 * from the farther corner and proceeding toward the camera (obj[0]).
	 * So here we consider the direction the camera is looking at and choose
	 * the traversal parameters accordingly.
	 * (why only "mostly enough"? Because the objects within the tiles are allowed
	 * to cross the tiles boundaries - we deal with this with the abovementionned
	 * lastcare list).
	 */
	switch (
		(obj[0].rot.z.x > 0) |
		((obj[0].rot.z.y > 0) <<1) |
		((fabs(obj[0].rot.z.x) > fabs(obj[0].rot.z.y)) <<2)
	) {
	case 0:
		x=0;    y=0;    sxx=1;  syx=0;  sxy=0;  syy=1;
		coli=0; dirx=1; diry=3; dmx=-1; dmy=-1; direct=1;
		break;
	case 1:
		x=SMAP; y=0;    sxx=-1; syx=0;  sxy=0;  syy=1;
		coli=1; dirx=2; diry=3; dmx=0; dmy=-1; direct=0;
		break;
	case 2:
		x=0;    y=SMAP; sxx=1;  syx=0;  sxy=0;  syy=-1;
		coli=1; dirx=1; diry=4; dmx=-1; dmy=0; direct=0;
		break;
	case 3:
		x=SMAP; y=SMAP; sxx=-1; syx=0;  sxy=0;  syy=-1;
		coli=0; dirx=2; diry=4; dmx=0;  dmy=0; direct=1;
		break;
	case 4:
		x=0;    y=0;    sxx=0;  syx=1;  sxy=1;  syy=0;
		coli=0; dirx=3; diry=1; dmx=-1; dmy=-1; direct=0;
		break;
	case 5:
		x=SMAP; y=0;    sxx=0;  syx=1;  sxy=-1; syy=0;
		coli=1; dirx=3; diry=2; dmx=0; dmy=-1; direct=1;
		break;
	case 6:
		x=0;    y=SMAP; sxx=0;  syx=-1; sxy=1;  syy=0;
		coli=1; dirx=4; diry=1; dmx=-1; dmy=0; direct=1;
		break;
	case 7:
		x=SMAP; y=SMAP; sxx=0;  syx=-1; sxy=-1; syy=0;
		coli=0; dirx=4; diry=2; dmx=0; dmy=0; direct=0;
		break;
	}
	if (x) x2=SMAP2; else x2=0;
	if (y) y2=SMAP2; else y2=0;

	// This vector c goes from camera to the furthest corner of the vision square
	vector c = {
		.x = (x-(SMAP>>1))*ECHELLE - fmodf(obj[0].pos.x, (float)ECHELLE),
		.y = (y-(SMAP>>1))*ECHELLE - fmodf(obj[0].pos.y, (float)ECHELLE),
		.z = -obj[0].pos.z,
	};	
	mulmtv(&obj[0].rot, &c, &c);

	// The same, in 24:8 fixed prec
	veci coin = {
		.x = c.x * 256,
		.y = c.y * 256,
		.z = c.z * 256,
	};

	// Position x,y at the furthest corner of the vision square (of length SMAP),
	// relative to the south-west corner of the world map (of length WMAP)
	// (provided x goes from west to east and y from south to north).
	x += (int)(obj[0].pos.x / ECHELLE) - (SMAP>>1) + (WMAP>>1);
	y += (int)(obj[0].pos.y / ECHELLE) - (SMAP>>1) + (WMAP>>1);

	// mx,my,mz is the transposed of the camera, in 24:8 fixed prec
	// ie it's the world base relative to camera
	veci mx,my;
	mx.x = obj[0].rot.x.x*ECHELLE*256;
	my.x = obj[0].rot.x.y*ECHELLE*256;
	mx.y = obj[0].rot.y.x*ECHELLE*256;
	my.y = obj[0].rot.y.y*ECHELLE*256;
	mx.z = obj[0].rot.z.x*ECHELLE*256;
	my.z = obj[0].rot.z.y*ECHELLE*256;
	// apart that mz is in 19:13
	mz.x = obj[0].rot.x.z*8192.;
	mz.y = obj[0].rot.y.z*8192.;
	mz.z = obj[0].rot.z.z*8192.;

	switch (diry) {
		case 1:
			d_coin = mx;
			break;
		case 2:
			d_coin = mx;
			negvi(&d_coin);
			break;
		case 3:
			d_coin = my;
			break;
		case 4:
			d_coin = my;
			negvi(&d_coin);
			break;
	}

	switch (dirx) {
		case 1:
			d_coinp = mx;
			break;
		case 2:
			d_coinp = mx;
			negvi(&d_coinp);
			break;
		case 3:
			d_coinp = my;
			break;
		case 4:
			d_coinp = my;
			negvi(&d_coinp);
			break;
	}

	// xk,yk is the tile coordinate of the camera's tile
	int const xk = (int)floor(obj[0].pos.x/ECHELLE)+(WMAP>>1);
	int const yk = (int)floor(obj[0].pos.y/ECHELLE)+(WMAP>>1);
	
	vecic ptsi[(SMAP+1)*2];	// integer 3d position + color for a small tile ribbon
	int pz[(SMAP+1)*2];	// storing the height of this map position << 14, for convenience

#	define Z_MIN (-(4*ECHELLE<<8))
	// xy,yy will loop on all tile locations on the 2nd furthest edge of the vision square
	// (relative to world map origin)
	// dy count from 0 to SMAP (incl) (length of the vision square)
	// ay is 0, SMAP+1, 0, SMAP+1, 0, etc... for addressing ptsi and pz.
	// Also, coin will goes along this furthest side
	int xy, yy, dy, ay;
	for (
		xy = x, yy = y, dy = 0, ay = 0;
		/*coin.z > Z_MIN &&*/ dy <= SMAP;
		dy++, ay ^= SMAP+1, xy += sxy, yy += syy,
		addvi(&coin, &d_coin)
	) {
		veci coinp = coin;
		// xx,yx will loop on all tile locations of the furthest edge of the vision square
		// (the horizon), from (xy,yy) (ie the furthest location of the line) toward the camera
		// and eventually behind, SMAP+1 times.
		// dx count from 0 to SMAP.
		// coinp will goes along the line.
		// Both the inner and outer loops are also exited whenever the z goes too far behind (Z_MIN)
		// FIXME: no we can't do that since when the camera is pointed at the sky the whole landscape
		//        is <Z_MIN, yet we want to draw the objects (if not the landscape itself).
		//        There is certainly an optimization, though (like draw close tiles objects
		//        at the end in all cases).
		int xx, yx, dx;
		for (
			xx = xy, yx = yy, dx = 0;
			/*coinp.z > Z_MIN &&*/ dx <= SMAP;
			dx++, xx += sxx, yx += syx,
			addvi(&coinp, &d_coinp)
		) {
			int b = MASKW(xx) + (MASKW(yx)<<NWMAP);	// the current tile
			int bb = MASKW(xx+dmx) + (MASKW(yx+dmy)<<NWMAP);	// one tile further (??)
			int ob = MASKW(xx-1) + (MASKW(yx)<<NWMAP);	// the tile at left (for lighting)
			int const z = map[b].z;	// the altitude of current tile

			int const a = dx + ay;
			pz[a] = z<<14;
			// ptsi[a] position
			ptsi[a].v.x = calcasm(coinp.x, z<<14, mz.x);
			ptsi[a].v.y = calcasm(coinp.y, z<<14, mz.y);
			ptsi[a].v.z = calcasm(coinp.z, z<<14, mz.z);
			// Compute ptsi[a] color
			int intens = ((z-map[ob].z))+32+64;
			if (intens<64) intens=64;
			else if (intens>127) intens=127;
			ptsi[a].c.r=(zcol[z].r*intens)>>7;
			ptsi[a].c.g=(zcol[z].g*intens)>>7;
			ptsi[a].c.b=(zcol[z].b*intens)>>7;

#			define KREMAP 3
			int const xk_xx = xk - xx;
			int const yk_yx = yk - yx;

			// First draw the landscape
			
			// if dy == 0 then ptsi and pz does not have previous line.
			// if dx == 0 then ptsi and pz lacks the previous point.
			if (dx && dy) {
				if (// if we are close enough to the camera
					xk_xx > -KREMAP && xk_xx < KREMAP &&
					yk_yx > -KREMAP && yk_yx < KREMAP
				) {
					// then draw using submap
					// c is the location of the opposite corner (the one that's close to us)
					veci c = coinp;
					subvi(&c, &d_coinp);
					subvi(&c, &d_coin);
					last_poly_is_visible = 0;	// awful hack
					draw_submap(c,
						pz[a],                  ptsi[a].c,
						pz[a-1],                ptsi[a-1].c,
						pz[dx+(ay^(SMAP+1))-1], ptsi[dx+(ay^(SMAP+1))-1].c,
						pz[dx+(ay^(SMAP+1))],   ptsi[dx+(ay^(SMAP+1))].c,
						bb);
					if (last_poly_is_visible && map[bb].has_road) {
						drawroute(bb);
					}
				} else {
					// If the tile is far enough that we don't want to draw it's submap
					last_poly_is_visible = 0;
					polyclip(
						&ptsi[a - 1 + direct],
						coli ? &ptsi[dx+(ay^(SMAP+1))-1] : &ptsi[dx+(ay^(SMAP+1))],
						&ptsi[a-direct]);
					polyclip(
						coli ? &ptsi[a] : &ptsi[a-1],
						&ptsi[dx+(ay^(SMAP+1)) -1 + direct],
						&ptsi[dx+(ay^(SMAP+1)) - direct]);
					if (last_poly_is_visible && map[bb].has_road) {
						drawroute(bb);
					}
				}
			} // else dx || dy -> we are on our first row or column

			// Then draw the objects

#			define KVISU (KREMAP+1)
			if (
				xk_xx > -KVISU && xk_xx < KVISU &&
				yk_yx > -KVISU && yk_yx < KVISU
			) {
				if (map[bb].first_obj != -1) {
					if (xk_xx < -1 || xk_xx > 1 || yk_yx < -1 || yk_yx > 1) {
						renderer(bb,3);
					} else {
						renderer(bb,0);
						lastcare[lcidx++]=bb;
					}
				}
			} else {
				renderer(bb, 1);
			}
		} // end of xx,yx inner loop
	} // end of yx,yy outer loop

	/* Now that the picture is almost done, render the objects that were so close
	 * to the camera that we refused to draw them when encountered (see note above
	 * about the "mostly enough")
	 */
	for (int i=0; i < lcidx; i++) {
		renderer(lastcare[i], 2);
	}
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
void MMXGouroPreca(int ib, int ig, int ir) { (void)ib; (void)ig; (void)ir; }

void polygouro(vect2dc *p1, vect2dc *p2, vect2dc *p3) {
	vect2dc *tmp, *pmax, *pmin;
	int q1, q2, q3=0, qxx, ql2;
	int qr1, qr2, qr3=0, qg1, qg2, qg3=0, qb1, qb2, qb3=0, qrr, qgg, qbb;
	if (p2->v.y<p1->v.y) { tmp=p1; p1=p2; p2=tmp; }
	if (p3->v.y<p1->v.y) { tmp=p1; p1=p3; p3=tmp; }
	if (p3->v.y<p2->v.y) { tmp=p2; p2=p3; p3=tmp; }
	if (p3->v.y<0 || p1->v.y>SY) return;
	pmin=pmax=p1;
	if (p2->v.x>pmax->v.x) pmax=p2;
	if (p3->v.x>pmax->v.x) pmax=p3;
	if (pmax->v.x<0) return;
	if (p2->v.x<pmin->v.x) pmin=p2;
	if (p3->v.x<pmin->v.x) pmin=p3;
	if (pmin->v.x>SX) return;
	Gouroyi=p1->v.y;
	last_poly_is_visible=1;
	if (p1->v.y!=p2->v.y) {
		Gouroxi=p1->v.x<<Gourovf;
		Gourocoulb=p1->c.b<<Gourovf;
		Gourocoulg=p1->c.g<<Gourovf;
		Gourocoulr=p1->c.r<<Gourovf;
		q1=((p2->v.x-p1->v.x)<<Gourovf)/(p2->v.y-p1->v.y);
		qr1=(((int)(p2->c.r-p1->c.r))<<Gourovf)/(p2->v.y-p1->v.y);
		qg1=(((int)(p2->c.g-p1->c.g))<<Gourovf)/(p2->v.y-p1->v.y);
		qb1=(((int)(p2->c.b-p1->c.b))<<Gourovf)/(p2->v.y-p1->v.y);
		q2=((p3->v.x-p1->v.x)<<Gourovf)/(p3->v.y-p1->v.y);
		qr2=(((int)(p3->c.r-p1->c.r))<<Gourovf)/(p3->v.y-p1->v.y);
		qg2=(((int)(p3->c.g-p1->c.g))<<Gourovf)/(p3->v.y-p1->v.y);
		qb2=(((int)(p3->c.b-p1->c.b))<<Gourovf)/(p3->v.y-p1->v.y);
		if (p3->v.y-p2->v.y) {
			q3=((p3->v.x-p2->v.x)<<Gourovf)/(p3->v.y-p2->v.y);
			qr3=(((int)(p3->c.r-p2->c.r))<<Gourovf)/(p3->v.y-p2->v.y);
			qg3=(((int)(p3->c.g-p2->c.g))<<Gourovf)/(p3->v.y-p2->v.y);
			qb3=(((int)(p3->c.b-p2->c.b))<<Gourovf)/(p3->v.y-p2->v.y);
		}
		Gourolx = Gourovfm;
		if (q1<=q2) {
			if(!(Gouroql=q2-q1)) Gouroql=1;		// le taux d'accroissement de la taille du segment (en �vitant 0);
			if(!(ql2=q2-q3)) ql2=-1;
			Gouroqx=q2; qxx=q2;
			Gouroqr=qr2; Gouroqg=qg2; Gouroqb=qb2; qrr=qr2; qgg=qg2; qbb=qb2;
			if (p2->v.y-p1->v.y>p3->v.y-p2->v.y) {
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
			if (p2->v.y-p1->v.y>p3->v.y-p2->v.y) {
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
		Gourody=p2->v.y-p1->v.y;
		MMXGouro();
		Gouroqx=qxx; Gouroql=ql2;
		Gouroqr=qrr; Gouroqg=qgg; Gouroqb=qbb;
		Gourody=p3->v.y-p2->v.y+1;
		MMXGouro();
	} else {	// base plate
		if (p3->v.y>p2->v.y) {	// triangle qd meme
			q2=((p3->v.x-p1->v.x)<<Gourovf)/(p3->v.y-p1->v.y);
			qr2=(((int)(p3->c.r-p1->c.r))<<Gourovf)/(p3->v.y-p1->v.y);
			qg2=(((int)(p3->c.g-p1->c.g))<<Gourovf)/(p3->v.y-p1->v.y);
			qb2=(((int)(p3->c.b-p1->c.b))<<Gourovf)/(p3->v.y-p1->v.y);
			q3=((p3->v.x-p2->v.x)<<Gourovf)/(p3->v.y-p2->v.y);
			qr3=(((int)(p3->c.r-p2->c.r))<<Gourovf)/(p3->v.y-p2->v.y);
			qg3=(((int)(p3->c.g-p2->c.g))<<Gourovf)/(p3->v.y-p2->v.y);
			qb3=(((int)(p3->c.b-p2->c.b))<<Gourovf)/(p3->v.y-p2->v.y);
			if (p2->v.x>=p1->v.x) {
				Gouroxi=p2->v.x<<Gourovf;
				Gourocoulb=p2->c.b<<Gourovf;
				Gourocoulg=p2->c.g<<Gourovf;
				Gourocoulr=p2->c.r<<Gourovf;
				Gourolx = (p2->v.x-p1->v.x)<<Gourovf;
				if(!(Gouroql=q3-q2)) Gouroql=-1;
				Gouroqx=q3;
				Gouroqr=qr3; Gouroqg=qg3; Gouroqb=qb3;
				if (Gouroql<-QLPREC) {
					Gouroir=((qr2-qr3)<<Gourovf)/Gouroql;
					Gouroig=((qg2-qg3)<<Gourovf)/Gouroql;
					Gouroib=((qb2-qb3)<<Gourovf)/Gouroql;
				} else Gouroir=Gouroig=Gouroib=0;
			} else {
				Gouroxi=p1->v.x<<Gourovf;
				Gourocoulb=p1->c.b<<Gourovf;
				Gourocoulg=p1->c.g<<Gourovf;
				Gourocoulr=p1->c.r<<Gourovf;
				Gourolx = (p1->v.x-p2->v.x)<<Gourovf;
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
			Gourody=p3->v.y-p1->v.y+1;
			MMXGouro();
		} else {	// trait plat
			Gouroxi=pmax->v.x<<Gourovf;
			if(!(Gourolx=(pmax->v.x-pmin->v.x)<<Gourovf)) Gourolx=Gourovfm;
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


// return the ground altitude at this position
// FIXME: we should max out the precision extracted from the coordinates
float z_ground(float x, float y, bool with_submap)
{
	// This starts as above
	// xx, yy: 28:4 fixed prec coordinates
	int const xx = x*16. + (((WMAP<<NECHELLE)>>1)<<4);
	int const yy = y*16. + (((WMAP<<NECHELLE)>>1)<<4);
	// xi, yi: the tile we're in
	int const xi = xx>>(NECHELLE+4);
	int const yi = yy>>(NECHELLE+4);
	// medx, medy: location within the tile, in 0:16 fixed prec
	int const medx = xx & ((ECHELLE<<4)-1);
	int const medy = yy & ((ECHELLE<<4)-1);
	// z1, z2, z3, z4: the altitudes of the 4 corner of this tile, in 8:0 fixed prec
	int const i = xi + (yi<<NWMAP);
	int const z1 = (int)map[i].z;
	int const z2 = (int)map[i+WMAP].z;
	int const z3 = (int)map[i+WMAP+1].z;
	int const z4 = (int)map[i+1].z;
	// zi, zj: the altitudes in the left and right edges of the tile when y=medy, in 0:16*8:0=8:16 fixed prec
	int const zi = medy*(z2-z1) + (z1<<16);
	int const zj = medy*(z3-z4) + (z4<<16);
	// then the altitude in between these points when x=medx, in (0:16>>6)*(8:16>>6) = (8:32>>12) = 8:20, then further degraded into 8:16
	int const z_base = (((medx>>6)*((zj-zi)>>6))>>4) + zi;
	float const z_base_f = z_base / 1024.;	// we want altitude * 64.
	if (! with_submap) {
//		assert(z_base_f > 0 && z_base_f < 64.*256.);
		return z_base_f;
	}

	// iix, iiy: coord within the tile in 0:3 fixed prec, ie subtile we are in in submap si
	unsigned const si = submap_get(i);
	int const iix = (medx<<NMAP2)>>(NECHELLE+4);
	int const iiy = (medy<<NMAP2)>>(NECHELLE+4);
	int const ii = iix+(iiy<<NMAP2);
	// mz1 is the nord-west corner of the submap, the only one we can fetch for sure from si, in 8:0 fixed prec
	int const mz1 = submap[si][ii];
	int mz2, mz3, mz4;
	if (iiy != SMAP2-1) {	// not last row
		mz2 = submap[si][ii+SMAP2];
		if (iix != SMAP2-1) { // not last column
			mz3 = submap[si][ii+SMAP2+1];
			mz4 = submap[si][ii+1];
		} else {
			unsigned const si1 = submap_get(i+1);
			mz3 = submap[si1][(iiy+1)<<NMAP2];
			mz4 = submap[si1][iiy<<NMAP2];
		}
	} else {	// last row
		unsigned const siw = submap_get(i+WMAP);
		mz2 = submap[siw][iix];
		if (iix!=SMAP2-1) {
			mz3 = submap[siw][iix+1];
			mz4 = submap[si][ii+1];
		} else {
			unsigned const si1 = submap_get(i+1);
			unsigned const siw1 = submap_get(i+WMAP+1);
			mz3 = submap[siw1][0];
			mz4 = submap[si1][iiy<<NMAP2];
		}
	}
	// minx, miny: our location within the subtile, in 0:13
	int const minx = medx & (((ECHELLE<<4)>>NMAP2)-1);
	int const miny = medy & (((ECHELLE<<4)>>NMAP2)-1);
	// mzi, mzj: altitudes of the left and right edge of the submap, in 0:13*8:0 = 8:13 fixed prec
	int const mzi = miny*(mz2-mz1) + (mz1<<13);
	int const mzj = miny*(mz3-mz4) + (mz4<<13);
	// now the altitude in the middle of mzi..mzj, in (0:13>>4)*(8:13>>4) = (8:26>>8) = 8:18
	int const z = (minx>>4)*((mzj-mzi)>>4) + (mzi<<5);
	// add submap altitude (divided by 16) to ground level altitude
	float const ret = z_base_f + (z/(16.*4096.));	// return z * 64.
//	assert(ret > 0 && ret < 64.*256.);
	return ret;
}

