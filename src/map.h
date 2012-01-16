#ifndef MAP_H_120115
#define MAP_H_120115

#include "proto.h"

extern uchar *map;
extern uchar *submap_of_map;
extern short int *objs_of_tile;
extern pixel zcol[256];

/* Given a tile index k, return its submap number */
int submap_get(int k);

/* Animate submap 9 with a wave pattern */
void animate_water(void);

#endif
