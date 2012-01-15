#ifndef MAP_H_120115
#define MAP_H_120115

extern uchar *map;
extern uchar *submap_of_map;
extern short int *objs_of_tile;
extern pixel zcol[256];

/* Given a tile index k, return its submap number */
int submap_get(int k);

#endif
