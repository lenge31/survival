/*
Copyright 2020 <wanzp@thundersoft.com>
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <byteswap.h>
#include <time.h>
#include <fcntl.h>

#define BLANK		'0'
#define APPLE		'1'
#define ORANGE		'2'
#define ANANAS		'3'
#define STRAWBERRY	'4'
#define WATERMELON	'5'
#define WALL		'9'
#define LEFT		'a'
#define RIGHT		'd'
#define UP		'w'
#define DOWN		's'
#define STOP		' '
#define CANNONBALL_L	'<'
#define CANNONBALL_R	'>'
#define CANNONBALL_U	'^'
#define CANNONBALL_D	'v'
#define GHOST		'G'
#define LAUNCH_YES	'v'
#define LAUNCH_NO	' '

struct coord {
	int x;
	int y;
};
#define MAP_X_MAX	100
#define MAP_Y_MAX	100
#define PLAYER_COUNT_MAX	100
struct map_obj {
	const char *map;
	/* map size */
	int map_size_h;
	int map_size_v;

	int player_me;
	int player_count;
	struct coord location[PLAYER_COUNT_MAX];
	int score[PLAYER_COUNT_MAX];
};

extern int log_fd;
#define log_i(fmt, ...)\
{\
        dprintf(log_fd, "<i>" fmt, ##__VA_ARGS__);\
}
#define log_w(fmt, ...)\
{\
        dprintf(log_fd, "<w>" fmt, ##__VA_ARGS__);\
}
#define log_e(fmt, ...)\
{\
        dprintf(log_fd, "<e>" fmt, ##__VA_ARGS__);\
}
#define log_pure(fmt, ...)\
{\
        dprintf(log_fd, fmt, ##__VA_ARGS__);\
}

#define errno_return(what)\
{\
        log_e("%s failed, errno{%d:%s}.\n", what, errno, strerror(errno));\
        return -errno;\
}

#define errno_info(what) \
{\
        log_e("%s failed, errno{%d:%s}.\n", what, errno, strerror(errno));\
        ret = -errno;\
}

inline static void m_sleep(int ms)
{
	int ret = 0;
	struct timespec req, rem;

	req.tv_sec = ms / 1000;
	req.tv_nsec = (ms % 1000) * 1000000;
	rem.tv_sec = 0;
	rem.tv_nsec = 0;

	ret = nanosleep(&req, &rem);
	if (ret == -1) {
		if (errno == EINTR) {
			log_i("sleep was interrupted, go on.\n");
			req.tv_sec = rem.tv_sec;
			req.tv_nsec = rem.tv_nsec;
			rem.tv_sec = 0;
			rem.tv_nsec = 0;
			nanosleep(&req, &rem);
		} else {
			errno_info("nanosleep");
		}
	}
}

/* client entry */
int client_start(int argc, char *argv[]);

#endif /* _COMMON_H_ */
