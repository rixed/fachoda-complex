#ifndef MAP_H_120115
#define MAP_H_120115

#include "proto.h"

struct tile {
	unsigned char z;	// The altitude of the map
	unsigned char submap:7;	// The submap number (between 0 and 10)
	/* If has_road is set then the tile has (at least) one road crossing it,
	 * is flat (submap is 0) and submap field encodes the road hash-key */
	unsigned char has_road:1;
	short first_obj;	// Number of first object above this tile
};

extern struct tile *map;

extern pixel zcol[256];

/* Given a tile index k, return its submap number */
int submap_get(int k);

/* Animate submap 9 with a wave pattern */
void animate_water(void);

#endif
