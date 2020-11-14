#ifndef DEFINES_H_
#define DEFINES_H_

/* Consts/enums */

#define WIDTH 80
#define HEIGHT 24
#define LAST_LINE_STR "25"
#define MAP_SIZE (WIDTH*HEIGHT)

#define NOT_WALKABLE 0x0
#define CORRIDOR 0x1
#define FLOOR 0x2

#define NOT_WALKABLE_CHAR '-'
#define FLOOR_CHAR '.'
#define ROOM_CHAR '0'
#define CORRIDOR_CHAR '/'
#define STAIRS_CHAR '%'
#define PLAYER_CHAR 'P'
#define AMULET_CHAR '*'
#define ENEMY_CHAR 'x'

#define STATE_PLAY 0
#define STATE_WIN 1
#define STATE_LOSS 2

#define FLOORS 8
#define LAST_FLOOR (FLOORS - 1)
#define AMULET_FLOOR (LAST_FLOOR / 2)
#define ENEMIES 1

#endif