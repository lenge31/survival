/*
Copyright 2020 <wanzp@thundersoft.com>
*/

#include "common.h"
#include "survival_protocol.h"

#define SCREEN_CLEAN		"\033[2J"
#define COLOR_DEFAULT		"\033[0m"
#define COLOR_PLAYER_ME		"\033[1;5;46m"	/* blue */
#define COLOR_PLAYER_OTHER	"\033[45m"	/* violet */
#define COLOR_GHOST		"\033[41m"	/* red */
#define COLOR_CANNONBALL	"\033[41m"	/* red */
#define COLOR_WALL		"\033[43m"	/* yellow */

int terminal_show_map(struct server_obj *s_obj)
{
	int offset = 0, pause = 0, next_offset;
	char c = '\0';
	int index = 0;

	if (s_obj->is_show_map == 0) return 0;

	printf(SCREEN_CLEAN);
	index = s_obj->player_me;
	printf("<%d>[%d] score:%d, location:(%d,%d)\n\n",
		s_obj->map_update_count, index,
		s_obj->score[index],
		s_obj->location[index]%s_obj->map_size_h,
		s_obj->location[index]/s_obj->map_size_h);

	for (int i = 0; i < s_obj->map_size_v; i++) {
		offset = s_obj->map_size_h * i;
		next_offset = s_obj->map_size_h * (i+1);

		while (offset < next_offset) {
			pause = offset;
			while(s_obj->map[pause] >= BLANK && s_obj->map[pause] <= WATERMELON)
				pause++;

			if (pause >= next_offset) {
				printf("%.*s", next_offset - offset, s_obj->map + offset);
				offset = next_offset;
				continue;
			}

			if (pause - offset > 0) {
				printf("%.*s", pause - offset, s_obj->map + offset);
				offset = pause;
			}

			/* show A special char */
			c = s_obj->map[pause];
			if (c == UP || c == RIGHT || c == LEFT || c == DOWN) {
				if (pause == s_obj->location[index])
					printf(COLOR_PLAYER_ME "%c" COLOR_DEFAULT, c);
				else
					printf(COLOR_PLAYER_OTHER "%c" COLOR_DEFAULT, c);
			} else if (c == GHOST) {
				printf(COLOR_GHOST "%c" COLOR_DEFAULT, c);
			} else if (c == CANNONBALL_L || c== CANNONBALL_U || c == CANNONBALL_R || c == CANNONBALL_D) {
				printf(COLOR_CANNONBALL "%c" COLOR_DEFAULT, c);
			} else if (c == WALL) {
				printf(COLOR_WALL "%c" COLOR_DEFAULT, c);
			} else {
				printf("%c", c);
			}
			/* printf("\noffset=%d, pause=%d, next_offset=%d, c=%c\n", offset, pause, next_offset, c); */
			offset++;
		}
		printf("\n");
	}

	return 0;
}
