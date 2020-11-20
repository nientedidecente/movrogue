#ifndef ENTITY_H_
#define ENTITY_H_

#include "types.h"

typedef struct {
	Point pos;
	Point old_pos;
	int max_hp; /* unused but for status bar */
	int attack; /* unused but for status bar */
	char level; /* unused but for status bar */
	int xp; /* unused but for status bar */
	int next_level_xp; /* unused but for status bar */
	int hp;
} LiveEntity;

typedef struct {
	Point pos;
} StillEntity;


#define is_alive(entity) ((entity).hp > 0)
#define same_pos(en1, en2) ((en1).pos.x == (en2).pos.x && (en1).pos.y == (en2).pos.y)
#define same_room(map, en1, en2) \
	(map_at((map), (en1).pos) >= FLOOR &&\
	 map_at((map), (en1).pos) == map_at((map), (en2).pos))

#endif