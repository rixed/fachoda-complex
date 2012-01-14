#include <string.h>

extern float randK();
static inline void addv(vector *r, vector *a) { r->x+=a->x; r->y+=a->y; r->z+=a->z; }
static inline void addvi(veci *r, veci *a) { r->x+=a->x; r->y+=a->y; r->z+=a->z; }
static inline void subv(vector *r, vector *a) { r->x-=a->x; r->y-=a->y; r->z-=a->z; }
static inline void subvi(veci *r, veci *a) { r->x-=a->x; r->y-=a->y; r->z-=a->z; }
static inline void mulv(vector *r, float a) { r->x*=a; r->y*=a; r->z*=a; }
static inline void copyv(vector *r, vector *a) { r->x=a->x; r->y=a->y; r->z=a->z; }
static inline void copym(matrix *r, matrix *a) { memcpy(r,a,sizeof(matrix)); }
static inline void mulm(matrix *r, matrix *a) {
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
static inline void mulm3(matrix *r, matrix *c, matrix *a) {
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
static inline void mulmt3(matrix *r, matrix *c, matrix *a) {	// c est transposée
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
static inline float norme(vector *u){ return(sqrt(u->x*u->x+u->y*u->y+u->z*u->z)); }
static inline float norme2(vector *u){ return(u->x*u->x+u->y*u->y+u->z*u->z); }
static inline float scalaire(vector *u, vector *v){ return(u->x*v->x+u->y*v->y+u->z*v->z); }
static inline float renorme(vector *a) {
	float d = norme(a);
	if (d!=0) {a->x/=d; a->y/=d; a->z/=d; }
	return(d);
}
static inline void prodvect(vector *a, vector *b, vector *c) {
	c->x = a->y*b->z-a->z*b->y;
	c->y = a->z*b->x-a->x*b->z;
	c->z = a->x*b->y-a->y*b->x;
}
static inline void orthov(vector *a, vector *b) {
	float s=scalaire(a,b);
	a->x -= s*b->x;
	a->y -= s*b->y;
	a->z -= s*b->z;
}
static inline float orthov3(vector *a, vector *b, vector *r) {
	float s=scalaire(a,b);
	r->x = a->x-s*b->x;
	r->y = a->y-s*b->y;
	r->z = a->z-s*b->z;
	return(s);
}
static inline void mulmv(matrix *n, vector *v, vector *r) {
	vector t;
	copyv(&t,v);
	r->x = n->x.x*t.x+n->y.x*t.y+n->z.x*t.z;
	r->y = n->x.y*t.x+n->y.y*t.y+n->z.y*t.z;
	r->z = n->x.z*t.x+n->y.z*t.y+n->z.z*t.z;
}
static inline void mulmtv(matrix *n, vector *v, vector *r) {
	vector t;
	copyv(&t,v);
	r->x = n->x.x*t.x+n->x.y*t.y+n->x.z*t.z;
	r->y = n->y.x*t.x+n->y.y*t.y+n->y.z*t.z;
	r->z = n->z.x*t.x+n->z.y*t.y+n->z.z*t.z;
}
static inline void neg(vector *v) { v->x=-v->x; v->y=-v->y; v->z=-v->z; }
static inline void proj(vect2d *e, vector *p) {
	e->x=_DX+p->x*focale/p->z;
	e->y=_DY+p->y*focale/p->z;
}
static inline float proj1(float p, float z) { return(p*focale/z); }
static inline void subv3(vector *a, vector *b, vector *r) {	// il faut r!=a,b
	r->x = a->x-b->x;
	r->y = a->y-b->y;
	r->z = a->z-b->z;
}
static inline void addv3(vector *a, vector *b, vector *r) {	// il faut r!=a,b
	r->x = a->x+b->x;
	r->y = a->y+b->y;
	r->z = a->z+b->z;
}
static inline void randomv(vector *v) {
	v->x=randK()-.5;
	v->y=randK()-.5;
	v->z=randK()-.5;
}
static inline void randomm(matrix *m) {
	randomv(&m->x);
	renorme(&m->x);
	m->y.x=-m->x.y;
	m->y.y=+m->x.x;
	m->y.z=-m->x.z;
	orthov(&m->y,&m->x);
	renorme(&m->y);
	prodvect(&m->x,&m->y,&m->z);
}
