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

#define FLOORS 8
#define ENEMIES 1

/* Globals */

Map current_map;
Point player_pos;
Point old_player_pos;
Point amulet_pos;
Point enemies_pos[ENEMIES];
Point stairs_pos;
/* NOTE: we are limited to 255 floors */
Floor cur_floor;
Floor old_floor;
bool has_amulet;

struct {
	Point stairs_pos[FLOORS];
	Point amulet_pos[FLOORS];
	Point enemies_pos[FLOORS][ENEMIES];
} memory;

/* Look-up tables and similar */

const char symbol_lut[] = {
	NOT_WALKABLE_CHAR,
	FLOOR_CHAR,
	CORRIDOR_CHAR,
};

const char amulet_char_lut[] = {
	AMULET_CHAR,
	FLOOR_CHAR
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
	print_to_coordinates(old_player_pos, symbol_lut[map_at(current_map, old_player_pos)]);
	/* Has to be done after enemies are cleared to avoid overwriting */
	for (e = 0; e < ENEMIES; e++) {
		print_to_coordinates(enemies_pos[e], ENEMY_CHAR);
	}
	print_to_coordinates(amulet_pos, amulet_char_lut[has_amulet]);
	print_to_coordinates(stairs_pos, STAIRS_CHAR);
	print_to_coordinates(player_pos, PLAYER_CHAR);
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
	memcpy(enemies_pos, memory.enemies_pos[cur_floor], sizeof(enemies_pos) * ENEMIES);
	amulet_pos = memory.amulet_pos[cur_floor];
	stairs_pos = memory.stairs_pos[cur_floor];
	player_pos.x = 5;
	player_pos.y = 5;
	update_current_map();
}

#define same_pos(pos1, pos2) ((pos1).x == (pos2).x && (pos1).y == (pos2).y)

#define move(input_ch) do {\
	old_floor = cur_floor;\
	old_player_pos = player_pos;\
	switch((input_ch)) {\
	case 'w': player_pos.y--; break;\
	case 's': player_pos.y++; break;\
	case 'a': player_pos.x--; break;\
	case 'd': player_pos.x++; break;\
	case 'v': cur_floor += same_pos(player_pos, stairs_pos); break;\
	}\
	if(NOT_WALKABLE == map_at(current_map, player_pos)\
	|| player_pos.x < 0\
	|| player_pos.y < 0\
	|| player_pos.x > WIDTH\
	|| player_pos.y > HEIGHT) {\
		player_pos = old_player_pos;\
	}\
	if (cur_floor > FLOORS - 1) {\
		cur_floor = FLOORS - 1;\
	}\
	if (cur_floor < 0) {\
		cur_floor = 1;\
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
	memory.stairs_pos[0].x = 62;
	memory.stairs_pos[0].y = 11;
	memory.stairs_pos[1].x = 72;
	memory.stairs_pos[1].y = 15;
	memory.stairs_pos[2].x = 72;
	memory.stairs_pos[2].y = 15;
	memory.stairs_pos[3].x = 72;
	memory.stairs_pos[3].y = 15;
	memory.stairs_pos[4].x = 72;
	memory.stairs_pos[4].y = 15;
	memory.stairs_pos[5].x = 72;
	memory.stairs_pos[5].y = 15;
	memory.stairs_pos[6].x = 72;
	memory.stairs_pos[6].y = 15;
	/* Enemies */
	memory.enemies_pos[0][0].x = 59;
	memory.enemies_pos[0][0].y = 8;
	memory.enemies_pos[1][0].x = 60;
	memory.enemies_pos[1][0].y = 7;
	memory.enemies_pos[2][0].x = 61;
	memory.enemies_pos[2][0].y = 6;
	memory.enemies_pos[3][0].x = 59;
	memory.enemies_pos[3][0].y = 5;
	memory.enemies_pos[4][0].x = 59;
	memory.enemies_pos[4][0].y = 8;
	memory.enemies_pos[5][0].x = 60;
	memory.enemies_pos[5][0].y = 7;
	memory.enemies_pos[6][0].x = 61;
	memory.enemies_pos[6][0].y = 6;

	/* Ah, the things you do not to use ifs */
	for (i = 1; i < FLOORS; i++) {
		memory.amulet_pos[i].x = -2;
		memory.amulet_pos[i].y = -2;
	}
	memory.amulet_pos[(FLOORS - 1) / 2].x = 72;
	memory.amulet_pos[(FLOORS - 1) / 2].y = 18;
	cur_floor = 0;
	has_amulet = 0;
	return FSM_STATE_NAME(NEW_FLOOR);
}

State FSM_FUN_NAME(NEW_FLOOR)(void) {
	generate_new_map();
	printf("Level -%03d\n%s\n", ((cur_floor > (FLOORS - 1) / 2) ? (FLOORS - 1) - cur_floor : cur_floor + 1), has_amulet ? "You have the amulet!" : "Find the amulet!");
	return (cur_floor == (FLOORS - 1) && has_amulet) ? FSM_STATE_NAME(WIN) : FSM_STATE_NAME(ON_FLOOR);
}

State FSM_FUN_NAME(ON_FLOOR)(void) {
	int i;
	move(getchar());
	update_current_map();
	for (i = 0; i < ENEMIES; i++) {
		/* XXX: can we avoid this if? */
		if(same_pos(player_pos, enemies_pos[i])) {
			return FSM_STATE_NAME(BATTLE);
		}
	}
	has_amulet = same_pos(player_pos, amulet_pos) || has_amulet;
	printf("Level -%03d\n%s\n", ((cur_floor > (FLOORS - 1) / 2) ? (FLOORS - 1) - cur_floor : cur_floor + 1), has_amulet ? "You have the amulet!" : "Find the amulet!");
	return (cur_floor == old_floor) ? FSM_STATE_NAME(ON_FLOOR) : FSM_STATE_NAME(NEW_FLOOR);
}

State FSM_FUN_NAME(BATTLE)(void) {
	return FSM_STATE_NAME(LOSE);
}

State FSM_FUN_NAME(WIN)(void) {
	puts("\033[DYou have got the amulet! - GAME OVER\n");
	return FSM_STATE_NAME(END);
}

State FSM_FUN_NAME(LOSE)(void) {
	puts("\033[DYou died! - GAME OVER\n");
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
