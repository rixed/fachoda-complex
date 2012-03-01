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
#include <math.h>
#include "proto.h"

extern inline void addv(struct vector *r, struct vector const *a);
extern inline void addvi(struct veci *r, struct veci const *a);
extern inline void subv(struct vector *r, struct vector const *a);
extern inline void subvi(struct veci *r, struct veci const *a);
extern inline void negvi(struct veci *r);
extern inline void mulv(struct vector *r, float a);
extern inline void copyv(struct vector *r, struct vector const *a);
extern inline void copym(struct matrix *r, struct matrix const *a);
extern inline void mulm(struct matrix *r, struct matrix const *a);
extern inline void mulm3(struct matrix *r, struct matrix const *c, struct matrix const *a);
extern inline void mulmt3(struct matrix *r, struct matrix const *c, struct matrix const *a);
float norme(struct vector *u)
{
    return sqrt(u->x*u->x+u->y*u->y+u->z*u->z);
}
extern inline float norme2(struct vector const *u);
extern inline float scalaire(struct vector const *u, struct vector const *v);
extern inline float renorme(struct vector *a);
extern inline int normei_approx(struct veci const *v);
extern inline void prodvect(struct vector const *a, struct vector const *b, struct vector *c);
extern inline void orthov(struct vector *a, struct vector *b);
extern inline float orthov3(struct vector *a, struct vector *b, struct vector *r);
extern inline void mulmv(struct matrix *n, struct vector *v, struct vector *r);
extern inline void mulmtv(struct matrix *n, struct vector *v, struct vector *r);
extern inline void neg(struct vector *v);
extern inline void proj(struct vect2d *e, struct vector const *p);
extern inline void proji(struct vect2d *e, struct veci const *p);
extern inline float proj1(float p, float z);
extern inline void subv3(struct vector const *restrict a, struct vector const *restrict b, struct vector *restrict r);
extern inline void addv3(struct vector *a, struct vector *b, struct vector *restrict r);
extern inline void cap_dist(struct vector *a, float dist);
extern inline void randomm(struct matrix *m);

void randomv(struct vector *v) {
    v->x=drand48()-.5;
    v->y=drand48()-.5;
    v->z=drand48()-.5;
}
