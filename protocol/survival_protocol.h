/*
Copyright 2020 <wanzp@thundersoft.com>
*/

#ifndef _SURVIVAL_PROTOCOL_H_
#define _SURVIVAL_PROTOCOL_H_

#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <arpa/inet.h>

int server_obj_init(struct server_obj *s_obj);
int server_connect(struct server_obj *s_obj);
int server_close(struct server_obj *s_obj);
int server_interactive(struct server_obj *s_obj);
struct map_obj *get_map_obj(struct server_obj *s_obj);
int set_action(struct server_obj *s_obj, char *action);

struct server_obj {
	struct sockaddr_in addr;
	int sfd;
#define PACKET_SIZE_MAX		(1024*1024)
	char last_recv_data[PACKET_SIZE_MAX];
	char last_send_data[PACKET_SIZE_MAX];

#define S_INIT		0
#define S_CONNECTED	1
#define S_GAMING	2
#define S_QUIT		3
	int status;

#define SEND_PREFIX	"("
#define SEND_SUFFIX	")"
#define RECV_PREFIX	"["
#define RECV_SUFFIX	"]"

#define KEY_LENGTH_MAX	128
#define KEY_COUNT_MAX	8
	char key[KEY_COUNT_MAX][KEY_LENGTH_MAX];
	int key_index;

#define STRING_LENGTH_MAX	64
#define START_PREFIX		"START"
	char start_prefix[STRING_LENGTH_MAX];

#define END_PREFIX		"GAMEOVER"
	char end_prefix[STRING_LENGTH_MAX];

#define QUIT_PREFIX		"QUIT"
	char quit_prefix[STRING_LENGTH_MAX];

#define HEARTBEAT_PERIOD	500
#define HEARTBEAT		"H"
	char heartbeat[STRING_LENGTH_MAX];
	/* ms */
	int heartbeat_period;
#define ACK	"OK"
	char ack[STRING_LENGTH_MAX];
#define READY	"READY"
	char ready[STRING_LENGTH_MAX];

#define MAP_PREFIX	"MAP"
	char map_prefix[STRING_LENGTH_MAX];
#define MAP_MAX_SIZE	((MAP_X_MAX) * (MAP_X_MAX))
	char map[MAP_MAX_SIZE];
	/* map actual size */
	int map_size_h;
	int map_size_v;
	int is_map_update;
	int map_update_count;
	int is_show_map;

	char token[STRING_LENGTH_MAX];

	int player_count;
	int player_me;

	/* map index */
	int location[PLAYER_COUNT_MAX];
#define LOCATION_PREFIX		"LOCATION"
	char location_prefix[STRING_LENGTH_MAX];

	int score[PLAYER_COUNT_MAX];
#define SCORE_PREFIX		"SCORE"
	char score_prefix[STRING_LENGTH_MAX];

	char current_action[STRING_LENGTH_MAX];
};

#endif /*_SURVIVAL_PROTOCOL_H_*/
