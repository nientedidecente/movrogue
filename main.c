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
#define STAIRS_UP_CHAR '>'
#define STAIRS_DOWN_CHAR '<'
#define PLAYER_CHAR 'P'
#define AMULET_CHAR '*'

#define STATE_PLAY 0
#define STATE_WIN 1
#define STATE_LOSS 1

/* Typedefs/structs */

typedef char STATE;
typedef char Map[MAP_SIZE];
typedef struct {
	char x;
	char y;
} Position;

#define LEVELS 4

/* Globals */

Map current_map;
Position player_pos;
Position old_player_pos;
Position amulet_pos;
Position stairs_pos[LEVELS + 1];
STATE game_state = STATE_PLAY;

/* Look-up tables and similar */

const char game_over_string[][32] = {
	"", /* STATE_PLAY: Unused */
	"You have got the amulet!", /* STATE_WIN */
	"You died!", /* STATE_LOSS */
};

const char symbol_lut[] = {
	NOT_WALKABLE_CHAR,
	FLOOR_CHAR,
	CORRIDOR_CHAR,
	STAIRS_UP_CHAR,
	STAIRS_DOWN_CHAR
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

void gen_map() {
	int i;
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
	for (i = 0; i < MAP_SIZE; i++) {
		current_map[i] = state_lut(map[i]);
	}
}

#define clear() printf("\033[H\033[J")
#define print_to_coordinates(pos, c) do { \
    printf("\033[%d;%dH%c", (pos).y+1, (pos).x+1, c); \
    printf("\033[" LAST_LINE_STR ";1H"); \
    fflush(stdout); \
} while(0)
#define map_at(map, pos) ((map)[(pos).x + WIDTH * (pos).y])


void print_map() {
	int i = 0;
	clear();
	while(i < MAP_SIZE) {
		putchar(symbol_lut[current_map[i++]]);
		if(i % WIDTH == 0) putchar('\n');
	}
}

void update_map(int level) {
	print_to_coordinates(old_player_pos, symbol_lut[map_at(current_map, old_player_pos)]);
	print_to_coordinates(amulet_pos, AMULET_CHAR);
	print_to_coordinates(stairs_pos[level - 2], symbol_lut[map_at(current_map, stairs_pos[level - 2])]);
	print_to_coordinates(stairs_pos[level + 1], symbol_lut[map_at(current_map, stairs_pos[level + 1])]);
	print_to_coordinates(stairs_pos[level - 1], STAIRS_UP_CHAR);
	print_to_coordinates(stairs_pos[level], STAIRS_DOWN_CHAR);
	print_to_coordinates(player_pos, PLAYER_CHAR);
}

#define on_stairs_up(player_pos, stairs_pos, level) player_pos.x == stairs_pos[level - 1].x && player_pos.y == stairs_pos[level - 1].y
#define on_stairs_down(player_pos, stairs_pos, level) player_pos.x == stairs_pos[level].x && player_pos.y == stairs_pos[level].y

#define move(input_ch) do {\
	old_player_pos = player_pos;\
	switch((input_ch)) {\
	case 'w': player_pos.y--; break;\
	case 's': player_pos.y++; break;\
	case 'a': player_pos.x--; break;\
	case 'd': player_pos.x++; break;\
	case 'c': level -= on_stairs_up(player_pos, stairs_pos, level); break;\
	case 'v': level += on_stairs_down(player_pos, stairs_pos, level); break;\
	}\
	if(NOT_WALKABLE == map_at(current_map, player_pos)\
	|| player_pos.x < 0\
	|| player_pos.y < 0\
	|| player_pos.x > WIDTH\
	|| player_pos.y > HEIGHT) {\
		player_pos = old_player_pos;\
	}\
	if (level > LEVELS - 1) {\
		level = LEVELS - 1;\
	}\
	if (level <= 0) {\
		level = 1;\
	}\
} while(0)

#define same_pos(pos1, pos2) ((pos1).x == (pos2).x && (pos1).y == (pos2).y)

int main() {
	char input;
	/* Terminal stuff*/
	static struct termios oldt, newt;
	/* Current level */
	int level;
	/* Write the attributes of stdin(STDIN_FILENO) to oldt */
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	/* Disables "wait for '\n' or EOF" mode */
	newt.c_lflag &= ~(ICANON);
	tcsetattr( STDIN_FILENO, TCSANOW, &newt);

	/* Game preamble */
	player_pos.x = 5;
	player_pos.y = 5;
	amulet_pos.x = 72;
	amulet_pos.y = 18;
	stairs_pos[0].x = 61;
	stairs_pos[0].y = 10;
	stairs_pos[1].x = 62;
	stairs_pos[1].y = 11;
	stairs_pos[2].x = 72;
	stairs_pos[2].y = 15;
	// Lowest level has no stairs down
	stairs_pos[3].x = -1;
	stairs_pos[3].y = -1;
	level = LEVELS - 1;
	gen_map();
	print_map();
	update_map(level);

	/* Game loop */
	while(game_state == STATE_PLAY) {
		input = getchar();
		move(input);
		update_map(level);
		game_state = same_pos(player_pos, amulet_pos) ? STATE_WIN : STATE_PLAY;
		printf("\nLevel -%03d\n", level);
	}
	printf("\033[D%s - GAME OVER\n", game_over_string[game_state]);
	
	/* Restore old attributes */
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
	return 0;
}
