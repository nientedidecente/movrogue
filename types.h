#ifndef TYPES_H_
#define TYPES_H_

#include "defines.h"

/* Typedefs/structs */

typedef unsigned char Floor;
typedef char Map[MAP_SIZE];
typedef struct {
	char x;
	char y;
} Point;

typedef struct {
	Point pos;
	Point old_pos;
	char max_hp; /* unused but for status bar */
	char attack; /* unused but for status bar */
	char level; /* unused but for status bar */
	char xp; /* unused but for status bar */
	char next_level_xp; /* unused but for status bar */
	char hp;
} LiveEntity;

typedef struct {
	Point pos;
} StillEntity;

#endif