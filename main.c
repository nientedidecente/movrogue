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
#define PLAYER_CHAR 'P'

/* Typedefs/structs */

typedef char Map[MAP_SIZE];
typedef struct {
	char x;
	char y;
} Position;

/* Globals */

Map current_map;
Position player_pos;
Position old_player_pos;

/* Look-up tables and similar */

const char symbol_lut[] = {
	NOT_WALKABLE_CHAR,
	FLOOR_CHAR,
	CORRIDOR_CHAR
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
#define print_to_coordinates(x, y, c) do { \
    printf("\033[%d;%dH%c", y+1, x+1, c); \
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

void update_map() {
	print_to_coordinates(old_player_pos.x, old_player_pos.y, symbol_lut[map_at(current_map, old_player_pos)]);
	print_to_coordinates(player_pos.x, player_pos.y, PLAYER_CHAR);
}

#define move(input_ch) do {\
	old_player_pos = player_pos;\
	switch((input_ch)) {\
	case 'w': player_pos.y--; break;\
	case 's': player_pos.y++; break;\
	case 'a': player_pos.x--; break;\
	case 'd': player_pos.x++; break;\
	}\
	if(NOT_WALKABLE == map_at(current_map, player_pos)\
	|| player_pos.x < 0\
	|| player_pos.y < 0\
	|| player_pos.x > WIDTH\
	|| player_pos.y > HEIGHT) {\
		player_pos = old_player_pos;\
	}\
} while(0)

int main() {
	char input;
	/* Terminal stuff*/
	static struct termios oldt, newt;
	/* Write the attributes of stdin(STDIN_FILENO) to oldt */
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	/* Disables "wait for '\n' or EOF" mode */
	newt.c_lflag &= ~(ICANON);
	tcsetattr( STDIN_FILENO, TCSANOW, &newt);

	/* Game preamble */
	player_pos.x = 5;
	player_pos.y = 5;
	gen_map();
	print_map();

	/* Game loop */
	do {
		update_map();
		input = getchar();
		move(input);
	} while(input != 'X'); /* Stop when 'X' is entered, TO BE REMOVED */
	
	/* Restore old attributes */
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
	return 0;
}
