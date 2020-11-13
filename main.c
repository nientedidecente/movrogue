#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/* Helper macros */

/* Consts/enums */

#define WIDTH 80
#define HEIGHT 24
#define LAST_LINE_STR "25"
#define MAP_SIZE (WIDTH*HEIGHT)

#define NOT_WALKABLE 0x0
#define FLOOR 0x1
#define CORRIDOR 0x2

#define NOT_WALKABLE_CHAR '-'
#define FLOOR_CHAR '.'
#define CORRIDOR_CHAR '/'
#define STAIRS_CHAR '%'
#define PLAYER_CHAR 'P'
#define AMULET_CHAR '*'
#define ENEMY_CHAR 'x'

#define STATE_PLAY 0
#define STATE_WIN 1
#define STATE_LOSS 2

/* Typedefs/structs */

typedef unsigned char Floor;
typedef char bool;
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

#define FLOORS 8
#define LAST_FLOOR (FLOORS - 1)
#define AMULET_FLOOR (LAST_FLOOR / 2)
#define ENEMIES 1

/* Globals */

Map current_map;
LiveEntity player;
LiveEntity enemies[ENEMIES];
StillEntity stairs;
LiveEntity *opponent;
/* NOTE: we are limited to 255 floors */
Floor cur_floor;
Floor old_floor;

const char * top_message;
const char * bottom_message;

struct {
	StillEntity stairs[FLOORS];
	LiveEntity enemies[FLOORS][ENEMIES];
} memory;

/* Look-up tables and similar */

const char messages[][64] = {
#define WIN_MESSAGE 0
	"You have got the amulet! - GAME OVER",
#define LOSE_MESSAGE 1
	"You died! - GAME OVER"
};
const char symbol_lut[] = {
	NOT_WALKABLE_CHAR,
	FLOOR_CHAR,
	CORRIDOR_CHAR,
};

/* NOTE: this function will disappear as soon as we start to generate a map */
char state_lut(const char c) {
	switch(c) {
	case NOT_WALKABLE_CHAR: return NOT_WALKABLE;
	case        FLOOR_CHAR: return FLOOR;
	case     CORRIDOR_CHAR: return CORRIDOR;
	               default: return NOT_WALKABLE; /* Should never happen */
	}
}

/* Functions and macro-as-functions */

#define clear() printf("\033[H\033[J")
#define print_to_coordinates(pos, c) do { \
    printf("\033[%d;%dH%c", (pos).y+1, (pos).x+1, c); \
    printf("\033[" LAST_LINE_STR ";1H"); \
    fflush(stdout); \
} while(0)
#define map_at(map, pos) ((map)[(pos).x + WIDTH * (pos).y])

void update_current_map() {
	int e;
	print_to_coordinates(player.old_pos, symbol_lut[map_at(current_map, player.old_pos)]);
	/* Has to be done after enemies are cleared to avoid overwriting */
	for (e = 0; e < ENEMIES; e++) {
		print_to_coordinates(enemies[e].pos, ENEMY_CHAR);
	}
    print_to_coordinates(stairs.pos, (cur_floor == AMULET_FLOOR) ? AMULET_CHAR : STAIRS_CHAR);
	print_to_coordinates(player.pos, PLAYER_CHAR);
	printf(
		"|Floor -%03d%38s| HP: %3d/%-3d  | ATK: %-3d     |\n",
		((cur_floor > AMULET_FLOOR) ? LAST_FLOOR - cur_floor : cur_floor + 1),
		top_message,
		player.hp, player.max_hp,
		player.attack
		);
	printf("+------------------------------------------------+--------------+--------------|\n");
	printf(
		"|%48s| Level: %-3d   | XP: %4d/%-4d|\n",
		bottom_message,
		player.level,
		player.xp, player.next_level_xp);
	printf("+------------------------------------------------+--------------+--------------|\n");
}

void generate_new_map() {
	int i = 0;
	char map[MAP_SIZE] =
		"--------------------------------------------------------------------------------"
		"--------------------------------------------------------------------------------"
		"--------------------------------------------------------------------------------"
		"--------------------------------------------------------------------------------"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"----.................//////////////////////////////////////.................----"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"----.................--------------------------------------.................----"
		"--------------------------------------------------------------------------------"
		"--------------------------------------------------------------------------------"
		"--------------------------------------------------------------------------------"
		"--------------------------------------------------------------------------------";
	clear();
	while (i < MAP_SIZE) {
		current_map[i] = state_lut(map[i]);
		putchar(symbol_lut[current_map[i++]]);
		if(i % WIDTH == 0) putchar('\n');
	}
	memcpy(enemies, memory.enemies[cur_floor], sizeof(enemies) * ENEMIES);
	stairs.pos = memory.stairs[cur_floor].pos;
	player.pos.x = 5;
	player.pos.y = 5;
	update_current_map();
}

#define is_alive(entity) ((entity).hp > 0)

#define same_pos(pos1, pos2) ((pos1).x == (pos2).x && (pos1).y == (pos2).y)

#define move(input_ch) do {\
	old_floor = cur_floor;\
	player.old_pos = player.pos;\
	switch((input_ch)) {\
	case 'w': player.pos.y--; break;\
	case 's': player.pos.y++; break;\
	case 'a': player.pos.x--; break;\
	case 'd': player.pos.x++; break;\
	case 'v': cur_floor += same_pos(player.pos, stairs.pos); break;\
	}\
	if(NOT_WALKABLE == map_at(current_map, player.pos)\
	|| player.pos.x < 0\
	|| player.pos.y < 0\
	|| player.pos.x >= WIDTH\
	|| player.pos.y >= HEIGHT) {\
		player.pos = player.old_pos;\
	}\
} while(0)

#define FSM_STATES() \
	FSM_STATE_MACRO(START) \
	FSM_STATE_MACRO(NEW_FLOOR) \
	FSM_STATE_MACRO(ON_FLOOR) \
	FSM_STATE_MACRO(BATTLE) \
	FSM_STATE_MACRO(WIN) \
	FSM_STATE_MACRO(LOSE)

/************************************************/
/***** PROTECTED FSM SECTION, DO NOT MODIFY *****/
#define FSM_FUN_NAME(X) fsm_state_fun_ ## X
#define FSM_STATE_NAME(X) FSM_STATE_ ## X
#define FSM_LOOP() do {\
	State fsm_state = FSM_STATE_NAME(START);\
	while((fsm_state = fsm_state_table[fsm_state]()) != FSM_STATE_NAME(END));\
} while(0)

#define FSM_STATE_MACRO(X) FSM_STATE_NAME(X),
typedef enum {
	FSM_STATE_END = -1,
	FSM_STATES()
	FSM_STATE_CNT
} State;
#undef FSM_STATE_MACRO

#define FSM_STATE_MACRO(X) State FSM_FUN_NAME(X) (void);
FSM_STATES()
#undef FSM_STATE_MACRO

#define FSM_STATE_MACRO(X) FSM_FUN_NAME(X),
State(*fsm_state_table[FSM_STATE_CNT])(void) = {
	FSM_STATES()
};
#undef FSM_STATE_MACRO
/********* END OF PROTECTED FSM SECTION *********/
/************************************************/

State FSM_FUN_NAME(START)(void) {
	int i;

	/* Game preamble */
	memory.stairs[0].pos.x = 62;
	memory.stairs[0].pos.y = 11;
	memory.stairs[1].pos.x = 72;
	memory.stairs[1].pos.y = 15;
	memory.stairs[2].pos.x = 72;
	memory.stairs[2].pos.y = 15;
	memory.stairs[3].pos.x = 72;
	memory.stairs[3].pos.y = 15;
	memory.stairs[4].pos.x = 72;
	memory.stairs[4].pos.y = 15;
	memory.stairs[5].pos.x = 72;
	memory.stairs[5].pos.y = 15;
	memory.stairs[6].pos.x = 72;
	memory.stairs[6].pos.y = 15;
	/* Enemies */
	memory.enemies[0][0].pos.x = 59;
	memory.enemies[0][0].pos.y = 8;
	memory.enemies[0][0].hp    = 2;
	memory.enemies[1][0].pos.x = 60;
	memory.enemies[1][0].pos.y = 7;
	memory.enemies[1][0].hp    = 2;
	memory.enemies[2][0].pos.x = 61;
	memory.enemies[2][0].pos.y = 6;
	memory.enemies[2][0].hp    = 2;
	memory.enemies[3][0].pos.x = 59;
	memory.enemies[3][0].pos.y = 5;
	memory.enemies[3][0].hp    = 2;
	memory.enemies[4][0].pos.x = 59;
	memory.enemies[4][0].pos.y = 8;
	memory.enemies[4][0].hp    = 2;
	memory.enemies[5][0].pos.x = 60;
	memory.enemies[5][0].pos.y = 7;
	memory.enemies[5][0].hp    = 2;
	memory.enemies[6][0].pos.x = 61;
	memory.enemies[6][0].pos.y = 6;
	memory.enemies[6][0].hp    = 2;

	memory.stairs[AMULET_FLOOR].pos.x = 72;
	memory.stairs[AMULET_FLOOR].pos.y = 18;
	cur_floor = 0;
	player.hp = 10;
	player.max_hp = 10;
	player.attack = 10;
	player.level = 1;
	player.xp = 1;
	player.next_level_xp = 0;

	top_message = "Find the amulet!";
	bottom_message = "";
	return FSM_STATE_NAME(NEW_FLOOR);
}

State FSM_FUN_NAME(NEW_FLOOR)(void) {
	if(cur_floor > AMULET_FLOOR) top_message = "You have the amulet!";
	generate_new_map();
	return (cur_floor == LAST_FLOOR) ? FSM_STATE_NAME(WIN) : FSM_STATE_NAME(ON_FLOOR);
}

State FSM_FUN_NAME(ON_FLOOR)(void) {
	int i;
	move(getchar());
	for (i = 0; i < ENEMIES; i++) {
		/* XXX: can we avoid this if? */
		if(same_pos(player.pos, enemies[i].pos)) {
			opponent = &enemies[i];
			return FSM_STATE_NAME(BATTLE);
		}
	}
	update_current_map();
	return (cur_floor == old_floor) ? FSM_STATE_NAME(ON_FLOOR) : FSM_STATE_NAME(NEW_FLOOR);
}

State FSM_FUN_NAME(BATTLE)(void) {
	/* Player attacks opponent */
	opponent->hp--;

	/* If opponent is alive, opponent attacks player */
	if(is_alive(*opponent)) {
		player.hp--;
		player.pos = player.old_pos;
	} else {
		opponent->pos.x = opponent->pos.y = -2;
	}

	update_current_map();
	return is_alive(player) ? FSM_STATE_NAME(ON_FLOOR) : FSM_STATE_NAME(LOSE);
}

State FSM_FUN_NAME(WIN)(void) {
	bottom_message = "You have got the amulet! - GAME OVER";
	update_current_map();
	return FSM_STATE_NAME(END);
}

State FSM_FUN_NAME(LOSE)(void) {
	bottom_message = "You died! - GAME OVER";
	update_current_map();
	return FSM_STATE_NAME(END);
}

int main() {
	/* Terminal stuff*/
	static struct termios oldt, newt;
	/* Write the attributes of stdin(STDIN_FILENO) to oldt */
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	/* Disables "wait for '\n' or EOF" mode */
	newt.c_lflag &= ~(ICANON);
	tcsetattr( STDIN_FILENO, TCSANOW, &newt);

	FSM_LOOP();

	/* Restore old attributes */
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
	return 0;
}
