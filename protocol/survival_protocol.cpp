/*
Copyright 2020 <wanzp@thundersoft.com>
*/

#include "common.h"
#include "survival_protocol.h"

static int ADDR_IN_LEN = sizeof(struct sockaddr_in);

#define PROTO_LOG_RECV_DATA		0x1
#define PROTO_LOG_SEND_DATA		0x2
#define PROTO_LOG_DATA_MAX		0x4
static int proto_log_flag = PROTO_LOG_RECV_DATA | PROTO_LOG_SEND_DATA | PROTO_LOG_DATA_MAX;

static const char *key[] = {
	"15ba4a49c8234c93af0b76ec7074e61c",

	"3efd4e39d0b946f59a7dd987343b9d1e",
	"678d2fb05ea14367bd7f425ddc2b5a8a",
	"2e4c0738f97546ea81a441291ff3de8d",
	"8deef46940dc4ede8a8694b02e63ce05",
	"9e462bc5bf0b4fc89d44acb21786672e",
	"43bf8c8290a84a94842d18e9d7c8f9c8",
	"12496b829e7c486fbb1a47cbb345e004"

};

int server_obj_init(struct server_obj *s_obj)
{
	if (s_obj == NULL) return -EINVAL;

	memset(s_obj, '\0', sizeof(struct server_obj));

	for (unsigned int i = 0; i < sizeof(key)/sizeof(key[0]); i++) {
		snprintf(s_obj->key[i], sizeof(s_obj->key[0]), "%s%s%s", SEND_PREFIX, key[i], SEND_SUFFIX);
	}
	strncpy(s_obj->start_prefix, RECV_PREFIX START_PREFIX " ", sizeof(s_obj->start_prefix));
	strncpy(s_obj->end_prefix, RECV_PREFIX END_PREFIX, sizeof(s_obj->end_prefix));
	strncpy(s_obj->quit_prefix, SEND_PREFIX QUIT_PREFIX SEND_SUFFIX, sizeof(s_obj->quit_prefix));
	strncpy(s_obj->heartbeat, SEND_PREFIX HEARTBEAT SEND_SUFFIX, sizeof(s_obj->heartbeat));
	s_obj->heartbeat_period = HEARTBEAT_PERIOD;
	strncpy(s_obj->ack, RECV_PREFIX ACK RECV_SUFFIX, sizeof(s_obj->ack));
	strncpy(s_obj->ready, SEND_PREFIX READY SEND_SUFFIX, sizeof(s_obj->ready));

	strncpy(s_obj->map_prefix, RECV_PREFIX MAP_PREFIX " ", sizeof(s_obj->map_prefix));
	s_obj->map_size_h = -1;
	s_obj->map_size_v = -1;
	strncpy(s_obj->location_prefix, RECV_PREFIX LOCATION_PREFIX " ", sizeof(s_obj->location_prefix));
	strncpy(s_obj->score_prefix, RECV_PREFIX SCORE_PREFIX " ", sizeof(s_obj->score_prefix));

	s_obj->player_me = -1;
	s_obj->player_count = 0;

	s_obj->current_action[0] = RIGHT;
	s_obj->current_action[1] = LAUNCH_NO;

	return 0;
}

static int s_send(struct server_obj *s_obj, char *data)
{
	int ret = 0;

	memset(s_obj->last_send_data, '\0', sizeof(s_obj->last_send_data));

	snprintf(s_obj->last_send_data, sizeof(s_obj->last_send_data), "%s", data);
	ret = write(s_obj->sfd, s_obj->last_send_data, strlen(s_obj->last_send_data));
	if (ret >= 0) {
		if ((proto_log_flag & PROTO_LOG_SEND_DATA) && (proto_log_flag & PROTO_LOG_DATA_MAX)) {
			log_i("send data >>%.*s>>\n", STRING_LENGTH_MAX, s_obj->last_send_data);
		} else if (proto_log_flag & PROTO_LOG_SEND_DATA)
			log_i("send data >>%s>>\n", s_obj->last_send_data);
	}

	return ret;
}

#define POLL_TIMEOUT_MS		500
static int s_recv(struct server_obj *s_obj)
{
	int ret = 0;
	struct pollfd poll_fds[1];

	memset(s_obj->last_recv_data, '\0', sizeof(s_obj->last_recv_data));

	poll_fds[0].revents = 0;
	poll_fds[0].events = POLLIN;
	poll_fds[0].fd = s_obj->sfd;

	ret = poll(poll_fds, 1, POLL_TIMEOUT_MS);
	if (ret < 0) errno_return("poll");

	if (ret == 0 || (poll_fds[0].events & POLLIN) == 0) return 0;

	ret = read(s_obj->sfd, s_obj->last_recv_data, sizeof(s_obj->last_recv_data));
	if (ret >= 0) {
		if ((proto_log_flag & PROTO_LOG_RECV_DATA) && (proto_log_flag & PROTO_LOG_DATA_MAX )) {
			log_i("recv data <<%.*s<<\n", STRING_LENGTH_MAX, s_obj->last_recv_data);
		} else if (proto_log_flag & PROTO_LOG_RECV_DATA)
			log_i("recv data <<%s<<\n", s_obj->last_recv_data);
	}

	return ret;
}

static int ack_check(struct server_obj *s_obj)
{
	int ret = 0;

	ret = s_recv(s_obj);
	if (ret < 0) errno_return("ack_check s_recv");
	if (strcasestr(s_obj->last_recv_data, s_obj->ack) == NULL) {
		log_e("ack check failed, return {%s}.\n", s_obj->last_recv_data);
		return -(errno = EPROTO);
	}

	return ret;
}

static int key_verify(struct server_obj *s_obj)
{
	int ret = 0, i = s_obj->key_index;

	while (s_obj->key[i][0] != '\0') {
		ret = s_send(s_obj, s_obj->key[i++]);
		if (ret < 0) errno_return("key_verify s_send");

		m_sleep(500);
		ret = ack_check(s_obj);

		if (ret >= 0) {
			log_i("key verify success.\n");
			return ret;
		}
	}

	return ret;
}

int server_connect(struct server_obj *s_obj)
{
	int ret = 0;

	s_obj->sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (s_obj->sfd < 0) errno_return("socket");

	ret = connect(s_obj->sfd, (struct sockaddr *)&s_obj->addr, ADDR_IN_LEN);
	if (ret < 0) errno_return("connect");

	ret = key_verify(s_obj);
	if (ret < 0) errno_return("key_verify");

	log_i("connect server success.\n");
	s_obj->status = S_CONNECTED;

	return ret;
}

int server_close(struct server_obj *s_obj)
{
	log_i("close socket.\n");

	if (s_obj->sfd > 0) close(s_obj->sfd);

	return 0;
}

static int is_start(struct server_obj *s_obj)
{
	int ret = 0;
	int offset = 0;
	char *p_str = NULL;

	p_str = strcasestr(s_obj->last_recv_data, s_obj->start_prefix);
	if (p_str == NULL) return 0;

	offset = strlen(s_obj->start_prefix);
	ret = sscanf(p_str + offset, "%d %d " RECV_SUFFIX, &s_obj->player_me, &s_obj->map_size_h);
	s_obj->map_size_v = s_obj->map_size_h;
	log_i("index: %d, map size: %d x %d.\n", s_obj->player_me, s_obj->map_size_h, s_obj->map_size_v);

	if (s_obj->map_size_h < 0 || s_obj->player_me < 0) {
		log_e("START format wrong, return %s.\n", s_obj->last_recv_data);
		return -(errno = EPROTO);
	}

	ret = s_send(s_obj, s_obj->ready);
	if (ret < 0) errno_return("is_start s_send");

	log_i("game start.\n");
	s_obj->status = S_GAMING;
	s_obj->map_update_count = 0;

	return 0;
}

static int heart_beat(struct server_obj *s_obj)
{
	int ret = 0;

	ret = s_send(s_obj, s_obj->heartbeat);
	if (ret < 0) errno_return("heart_beat s_send");

	m_sleep(100);

	ret = s_recv(s_obj);
	if (ret < 0) errno_return("heart_beat s_recv");

	ret = is_start(s_obj);
	if (ret >= 0) return ret;

	if (strncmp(s_obj->last_recv_data, s_obj->ack, strlen(s_obj->ack)) != 0) {
		log_w("heartbeat failed, return {%s}.\n", s_obj->last_recv_data);
	} {
		log_i("heartbeat success.\n");
	}

	return ret;
}

static int is_end(struct server_obj *s_obj)
{
	int ret = 0;

	if (strcasestr(s_obj->last_recv_data, s_obj->end_prefix) == NULL) {
		return 0;
	}

	ret = s_send(s_obj, s_obj->quit_prefix);
	if (ret < 0) errno_return("is_end s_send");

	log_i("game over.\n");
	s_obj->status = S_QUIT;

	return 0;
}

static int send_action(struct server_obj *s_obj)
{
	int ret = 0;
	char temp_buf[STRING_LENGTH_MAX * 3];

	/* send action */
	memset(temp_buf, '\0', sizeof(temp_buf));
	snprintf(temp_buf, sizeof(temp_buf), "%s%s%s%s", SEND_PREFIX, s_obj->token, s_obj->current_action, SEND_SUFFIX);
	ret = s_send(s_obj, temp_buf);
	if (ret < 0) errno_return("action s_send");

	m_sleep(100);
	ack_check(s_obj);

	return ret;
}

int set_action(struct server_obj *s_obj, char *action)
{
	if (strlen(action) == 2) {
		log_i("update action to {%s}.\n", action);
		strncpy(s_obj->current_action, action, sizeof(s_obj->current_action));
		send_action(s_obj);
	} else {
		return -EINVAL;
	}

	return 0;
}

extern int terminal_show_map(struct server_obj *s_obj);

int map_log(struct server_obj *s_obj) {
	int offset = 0;

	log_i("map:vvvvvvvv\n");
	for (int i = 0; i < s_obj->map_size_v; i++) {
		offset = s_obj->map_size_h * i;
		log_pure("%.*s\n", s_obj->map_size_h, s_obj->map + offset);
	}
	log_i("map:^^^^^^^^\n");

	return 0;
}

#define RESTART_COUNT	60
static int map_location_score_parse(struct server_obj *s_obj)
{
	int ret = 0, offset = 0;
	static int restart_count = RESTART_COUNT;

	ret = s_recv(s_obj);
	if (ret < 0) errno_return("map_location_score_parse s_recv");

	if (ret == 0) {
		if (restart_count-- < 0) {
			log_e("try restart");
			s_obj->status = S_CONNECTED;
			restart_count = RESTART_COUNT;
		}
		return 0;
	}

	ret = is_end(s_obj);

	if (s_obj->status == S_GAMING) {
		if (strncmp(s_obj->last_recv_data, s_obj->map_prefix, strlen(s_obj->map_prefix)) == 0) {
			offset += strlen(s_obj->map_prefix);
			sscanf(s_obj->last_recv_data + offset, "%s ", s_obj->token);

			log_i("token is {%s}.\n", s_obj->token);

			offset += strlen(s_obj->token) + 1;
			strncpy(s_obj->map, s_obj->last_recv_data + offset, s_obj->map_size_h * s_obj->map_size_v);

			map_log(s_obj);

			offset += s_obj->map_size_h * s_obj->map_size_v + strlen(RECV_SUFFIX);

			s_obj->player_count = 0;
			offset += strlen(s_obj->location_prefix) + strlen(s_obj->token) + 1;
			while (strncmp(s_obj->last_recv_data + offset, RECV_SUFFIX, strlen(RECV_SUFFIX))) {
				ret = sscanf(s_obj->last_recv_data + offset, "%d", s_obj->location + s_obj->player_count++);
				if(ret == 0) s_obj->player_count--;

				offset++;
				while ((s_obj->last_recv_data[offset] >= '0' && s_obj->last_recv_data[offset] <= '9') ||
					s_obj->last_recv_data[offset] == '-')
					offset++;
			}
			offset++;

			log_i("location:");
			for (int i = 0; i < s_obj->player_count; i++) {
				if (i == s_obj->player_me) {
					log_pure(" [%d]", s_obj->location[i]);
				} else {
					log_pure(" %d", s_obj->location[i]);
				}
			}
			log_pure(".\n");

			s_obj->player_count = 0;
			offset += strlen(s_obj->score_prefix) + strlen(s_obj->token) + 1;
			while (strncmp(s_obj->last_recv_data + offset, RECV_SUFFIX, strlen(RECV_SUFFIX))) {
				ret = sscanf(s_obj->last_recv_data + offset, "%d", s_obj->score + s_obj->player_count++);
				if(ret == 0) s_obj->player_count--;

				offset++;
				while ((s_obj->last_recv_data[offset] >= '0' && s_obj->last_recv_data[offset] <= '9') ||
					s_obj->last_recv_data[offset] == '-')
					offset++;
			}

			log_i("score:");
			for (int i = 0; i < s_obj->player_count; i++) {
				if (i == s_obj->player_me) {
					log_pure(" [%d]", s_obj->score[i]);
				} else {
					log_pure(" %d", s_obj->score[i]);
				}
			}
			log_pure(".\n");

			s_obj->is_map_update = 1;
			s_obj->map_update_count += 1;
			restart_count = RESTART_COUNT;

			terminal_show_map(s_obj);

			log_i("<%d>[%d/%d]: L=%d, S=%d, last action={%s}.\n", s_obj->map_update_count,
					s_obj->player_me, s_obj->player_count, s_obj->location[s_obj->player_me],
					s_obj->score[s_obj->player_me], s_obj->current_action);
		}
	}

	return ret;
}

int server_interactive(struct server_obj *s_obj)
{
	int ret = 0;

	if (s_obj->status == S_CONNECTED) {
		log_i("heart beat...\n");

		ret = heart_beat(s_obj);
	} else if (s_obj->status == S_GAMING) {
		log_i("gaming...\n");

		ret = map_location_score_parse(s_obj);
	} else {
		ret = -(errno = EPROTO);
	}

	return ret;
}

struct map_obj *get_map_obj(struct server_obj *s_obj)
{
	static struct map_obj m_obj;

	/* check map whether generate */
	if (s_obj->map_size_h <= 0 || s_obj->map_size_v <= 0 ||
		strlen(s_obj->map) < (size_t)(s_obj->map_size_h * s_obj->map_size_v)) {
		return NULL;
	}

	m_obj.map = s_obj->map;
	m_obj.map_size_h = s_obj->map_size_h;
	m_obj.map_size_v = s_obj->map_size_v;
	m_obj.player_me= s_obj->player_me;
	m_obj.player_count = s_obj->player_count;

	for (int i = 0; i < s_obj->player_count; i++) {
		m_obj.location[i].x = s_obj->location[i] % s_obj->map_size_h;
		m_obj.location[i].y = s_obj->location[i] / s_obj->map_size_h;
		m_obj.score[i] = s_obj->score[i];
	}

	s_obj->is_map_update = 0;

	return &m_obj;
}
