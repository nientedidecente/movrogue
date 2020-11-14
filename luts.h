#ifndef LUTS_H_
#define LUTS_H_

#include "defines.h"

/* Look-up tables and similar */

const char messages[][64] = {
#define WIN_MESSAGE 0
	"You have got the amulet! - GAME OVER",
#define LOSE_MESSAGE 1
	"You died! - GAME OVER"
};
const char symbol_lut[] = {
	NOT_WALKABLE_CHAR,
	CORRIDOR_CHAR,
	FLOOR_CHAR,
	FLOOR_CHAR,
	FLOOR_CHAR,
	FLOOR_CHAR,
	FLOOR_CHAR,
	FLOOR_CHAR,
	FLOOR_CHAR,
	FLOOR_CHAR,
	FLOOR_CHAR,
	FLOOR_CHAR,
};

/* NOTE: this function will disappear as soon as we start to generate a map */
char state_lut(const char c) {
	switch(c) {
	case NOT_WALKABLE_CHAR: return NOT_WALKABLE;
	case     CORRIDOR_CHAR: return CORRIDOR;
	default:
		if(c < ROOM_CHAR || c > ROOM_CHAR+9) return NOT_WALKABLE;
		else 								 return FLOOR + (c - ROOM_CHAR);
	}
}

#endif