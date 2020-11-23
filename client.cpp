/*
Copyright 2020 <wanzp@thundersoft.com>
*/

#include <signal.h>

#include "common.h"
#include "survival_protocol.h"

int log_fd = 1;
static struct server_obj s_obj_g;
static const char *client_version = "v3.1";

/*
extern int policy_do_wzp(struct map_obj *, char *);
int (*policy_do)(struct map_obj *, char *) = policy_do_wzp;
*/
extern int policy_do_v3(struct map_obj *, char *);
int (*policy_do)(struct map_obj *, char *) = policy_do_v3;

static int policy_get(struct server_obj *s_obj)
{
	int ret = 0;
	static char action[] = {STOP, LAUNCH_NO, '\0'};
	struct map_obj *m_obj;

	m_obj = get_map_obj(s_obj);
	if (m_obj == NULL) return -EAGAIN;

	ret = policy_do(m_obj, action);

	if (ret >= 0) set_action(s_obj, action);

	return ret;
}

#define IP_PARAM_PREFIX		"--ip"
#define PORT_PARAM_PREFIX	"--port"
#define LOGFILE_PARAM_PREFIX	"--logfile"
#define SHOWMAP_PARAM_PREFIX	"--showmap"
#define KEYINDEX_PARAM_PREFIX	"--keyindex"

int client_start(int argc, char *argv[])
{
	int ret = 0;
	unsigned short port = 0;
	char log_file[256];

	log_i("client version: %s.\n", client_version);

	signal(SIGPIPE, SIG_IGN);

	ret = server_obj_init(&s_obj_g);
	if (ret < 0) errno_return("server_obj_init");

	memset(log_file, '\0', sizeof(log_file));
	for (int i = 0; i < argc-1; i++) {
		if (strncmp(argv[i], IP_PARAM_PREFIX, strlen(IP_PARAM_PREFIX)) == 0) {
			s_obj_g.addr.sin_family = AF_INET;
			s_obj_g.addr.sin_addr.s_addr = inet_addr(argv[++i]);
			log_i("ip:\t\t%s.\n", inet_ntoa(s_obj_g.addr.sin_addr));
		}

		if (strncmp(argv[i], PORT_PARAM_PREFIX, strlen(PORT_PARAM_PREFIX)) == 0) {
			sscanf(argv[++i], "%hu", &port);
			log_i("port:\t%hu.\n", port);
			s_obj_g.addr.sin_port = bswap_16(port);
		}

		if (strncmp(argv[i], LOGFILE_PARAM_PREFIX, strlen(LOGFILE_PARAM_PREFIX)) == 0) {
			sscanf(argv[++i], "%s", log_file);
		}

		if (strncmp(argv[i], SHOWMAP_PARAM_PREFIX, strlen(SHOWMAP_PARAM_PREFIX)) == 0) {
			s_obj_g.is_show_map = 1;
		}

		if (strncmp(argv[i], KEYINDEX_PARAM_PREFIX, strlen(KEYINDEX_PARAM_PREFIX)) == 0) {
			sscanf(argv[++i], "%d", &s_obj_g.key_index);
		}
	}
	if (strncmp(argv[argc-1], SHOWMAP_PARAM_PREFIX, strlen(SHOWMAP_PARAM_PREFIX)) == 0) {
		s_obj_g.is_show_map = 1;
	}

	if (strlen(log_file) > 0) {
		ret = open(log_file, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
		if (ret <= 0) {
			errno_info("open log_file");
		} else log_fd = ret;
	}

	if (s_obj_g.addr.sin_addr.s_addr == 0 || s_obj_g.addr.sin_port == 0) {
		log_i("ip/port params missing.\n");
		return -EINVAL;
	}

	ret = server_connect(&s_obj_g);
	if (ret <= 0) {
		server_close(&s_obj_g);
		errno_return("server_connect");
	}

	while (1) {
		m_sleep(s_obj_g.heartbeat_period);
		
		ret = server_interactive(&s_obj_g);

		if (s_obj_g.is_map_update) policy_get(&s_obj_g);

		if (s_obj_g.status == S_QUIT || ret < 0) break;
	}

	server_close(&s_obj_g);

	return ret;
}
