#ifndef TYPES_H_
#define TYPES_H_

#include "defines.h"

/* Typedefs/structs */

typedef unsigned char Floor;

typedef char Map[MAP_SIZE];
#define map_at(map, pos) ((map)[(pos).x + WIDTH * (pos).y])

typedef struct {
	char x;
	char y;
} Point;

#endif