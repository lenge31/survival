/*
Copyright 2020 <wanzp@thundersoft.com>
*/

#include "common.h"

#define D_INVALID	(-0xFFF)
#define D_DEATH		(0x100)
#define D_DANGER_C	(0x35)
#define D_DANGER	(0x30)
#define D_GHOST		(0x20)
#define D_COMPETE	(0x10)
#define D_NONE		(0x0)
#define D_FIRE		(-0x3)
#define D_RISK		(-0x10)
#define D_WALL		(-0x20)

static int index_i = 0, score_i = 0;

static struct {
	struct coord c_l;
	char dir[4];		/* left/right/up/down */
	int value[4];		/* left/right/up/down */
	char action[4][3];	/* left/right/up/down */
} current_value_history;

static void store_value(struct coord *l, char d, int value, char *action)
{
	int ii = 0;

	if (current_value_history.c_l.x != l->x || current_value_history.c_l.y != l->y) {
		current_value_history.c_l = *l;

		for (ii = 0; ii < 4; ii++) {
			current_value_history.value[ii] = D_INVALID;
			current_value_history.action[ii][0] = STOP;
			current_value_history.action[ii][1] = LAUNCH_NO;
			current_value_history.action[ii][2] = '\0';
		}
	}

	if (d == LEFT) {
		ii = 0;
	} else if (d == RIGHT) {
		ii = 1;
	} else if (d == UP) {
		ii = 2;
	} else {
		ii = 3;
	}

	current_value_history.dir[ii] = d;
	current_value_history.value[ii] = value;
	current_value_history.action[ii][0] = action[0];
	current_value_history.action[ii][1] = action[1];
}

static int get_value(char d, char *action)
{
	int ii = 0;

	if (d == LEFT) {
		ii = 0;
	} else if (d == RIGHT) {
		ii= 1;
	} else if (d == UP) {
		ii = 2;
	} else {
		ii = 3;
	}

	if (action != NULL) {
		action[0] = current_value_history.action[ii][0];
		action[1] = current_value_history.action[ii][1];
	}

	return current_value_history.value[ii];
}

static char turn_left(char d)
{
	if (d == LEFT) return DOWN;
	else if (d == RIGHT) return UP;
	else if (d == UP) return LEFT;
	else if (d == DOWN) return RIGHT;

	return d;
}

static char turn_right(char d)
{
	if (d == LEFT) return UP;
	else if (d == RIGHT) return DOWN;
	else if (d == UP) return RIGHT;
	else if (d == DOWN) return LEFT;

	return d;
}

static char turn_back(char d)
{
	if (d == LEFT) return RIGHT;
	else if (d == RIGHT) return LEFT;
	else if (d == UP) return DOWN;
	else if (d == DOWN) return UP;

	return d;
}

static char get_char(struct map_obj *m_obj, struct coord *l)
{
	if (l->x < 0 || l->x >= m_obj->map_size_h ||
			l->y < 0 || l->y >= m_obj->map_size_v)
		return WALL;

	return *(m_obj->map + (m_obj->map_size_h * l->y + l->x));
}

static int get_char_value(char c)
{
	return c - '0';
}

static char left_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord n_l;
	int x_delta, y_delta;

	n_l = *l;

	x_delta = y_delta = 0;
	if (d == LEFT) y_delta = 1;
	else if (d == RIGHT) y_delta = -1;
	else if (d == UP) x_delta = -1;
	else if (d == DOWN) x_delta = 1;

	n_l.x += x_delta;
	n_l.y += y_delta;

	return get_char(m_obj, &n_l);
}
static char right_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord n_l;
	int x_delta, y_delta;

	n_l = *l;

	x_delta = y_delta = 0;
	if (d == LEFT) y_delta = -1;
	else if (d == RIGHT) y_delta = 1;
	else if (d == UP) x_delta = 1;
	else if (d == DOWN) x_delta = -1;

	n_l.x += x_delta;
	n_l.y += y_delta;

	return get_char(m_obj, &n_l);
}
static char forward_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord n_l;
	int x_delta, y_delta;

	n_l = *l;

	x_delta = y_delta = 0;
	if (d == LEFT) x_delta = -1;
	else if (d == RIGHT) x_delta = 1;
	else if (d == UP) y_delta = -1;
	else if (d == DOWN) y_delta = 1;

	n_l.x += x_delta;
	n_l.y += y_delta;

	return get_char(m_obj, &n_l);
}

static char backward_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord n_l;
	int x_delta, y_delta;

	n_l = *l;

	x_delta = y_delta = 0;
	if (d == LEFT) x_delta = 1;
	else if (d == RIGHT) x_delta = -1;
	else if (d == UP) y_delta = 1;
	else if (d == DOWN) y_delta = -1;

	n_l.x += x_delta;
	n_l.y += y_delta;

	return get_char(m_obj, &n_l);
}

static struct coord *get_next_location(struct coord *l, char dir)
{
	if (dir == LEFT) {
		l->x -= 1;
	} else if (dir == RIGHT) {
		l->x += 1;
	} else if (dir == UP) {
		l->y -= 1;
	} else if (dir == DOWN) {
		l->y += 1;
	}

	return l;
}

/*
		x
	>
*/
static char forward_left_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord x_l;

	x_l = *l;
	get_next_location(&x_l, d);
	get_next_location(&x_l, turn_left(d));

	return get_char(m_obj, &x_l);
}

/*
	>	i	x
*/
static char forward_forward_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord x_l;

	x_l = *l;
	get_next_location(&x_l, d);
	get_next_location(&x_l, d);

	return get_char(m_obj, &x_l);
}

/*
	>
		x
*/
static char forward_right_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord x_l;

	x_l = *l;
	get_next_location(&x_l, d);
	get_next_location(&x_l, turn_right(d));

	return get_char(m_obj, &x_l);
}

/*
		xx
		i
	>	i
*/
static char forward_left_left_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord x_l;

	x_l = *l;
	get_next_location(&x_l, d);
	get_next_location(&x_l, turn_left(d));
	get_next_location(&x_l, turn_left(d));

	return get_char(m_obj, &x_l);
}

/*
			xx
	>	i	i
*/
static char forward_forward_left_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord x_l;

	x_l = *l;
	get_next_location(&x_l, d);
	get_next_location(&x_l, d);
	get_next_location(&x_l, turn_left(d));

	return get_char(m_obj, &x_l);
}

/*
	>	i	i	xx
*/
static char forward_forward_forward_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord x_l;

	x_l = *l;
	get_next_location(&x_l, d);
	get_next_location(&x_l, d);
	get_next_location(&x_l, d);

	return get_char(m_obj, &x_l);
}

/*
	>	i
		i
		xx
*/
static char forward_right_right_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord x_l;

	x_l = *l;
	get_next_location(&x_l, d);
	get_next_location(&x_l, turn_right(d));
	get_next_location(&x_l, turn_right(d));

	return get_char(m_obj, &x_l);
}

/*
	>	i	i
			xx
*/
static char forward_forward_right_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord x_l;

	x_l = *l;
	get_next_location(&x_l, d);
	get_next_location(&x_l, d);
	get_next_location(&x_l, turn_right(d));

	return get_char(m_obj, &x_l);
}

/*
	>	i	i	i	xxx
*/
static char forward_forward_forward_forward_char(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord x_l;

	x_l = *l;
	get_next_location(&x_l, d);
	get_next_location(&x_l, d);
	get_next_location(&x_l, d);
	get_next_location(&x_l, d);

	return get_char(m_obj, &x_l);
}

static int is_player(char c)
{
	if (c == LEFT || c == RIGHT ||
			c == UP || c == DOWN)
		return 1;

	return 0;
}

/* d: dir, c: cannonball */
static int is_cannonball_attack(char d, char c)
{
	if (d == LEFT && c == CANNONBALL_R) return 1;
	else if (d == RIGHT && c == CANNONBALL_L) return 1;
	else if (d == UP && c == CANNONBALL_D) return 1;
	else if (d == DOWN && c == CANNONBALL_U) return 1;

	return 0;
}

static int is_cannonball(char c)
{
	if (c == CANNONBALL_L || c == CANNONBALL_R || c == CANNONBALL_U || c == CANNONBALL_D)
		return 1;

	return 0;
}

static int get_player_score(struct map_obj *m_obj, struct coord *l)
{
	for (int i=0; i<m_obj->player_count; i++) {
		if (m_obj->location[i].x == l->x && m_obj->location[i].y == l->y) return m_obj->score[i];
	}

	return -1;
}

/*
case:
1. forward;
2. veer.
*/

/*
forward:
		xxx

i	i	xx	xxx	

i	>	x	xx	xxx	xxxx

i	i	xx	xxx	

		xxx
*/
static void forward_value(struct map_obj *m_obj)
{
	int value = D_WALL;
	char action[2];
	struct coord *c_l, x_l;
	char c_d, x_c;
	int xxx_ghost_count = 0, score = 0;

	c_l = &m_obj->location[index_i];
	c_d = get_char(m_obj, c_l);
	action[0] = STOP;
	action[1] = LAUNCH_NO;

	/* forward */
	x_c = forward_char(m_obj, c_l, c_d);
	if (x_c == GHOST || is_player(x_c) || is_cannonball_attack(c_d, x_c)) {
		log_i("F stop&fire %c.\n", x_c);
		action[1] = LAUNCH_YES;
		store_value(c_l, c_d, D_DEATH, action);
		return;
	} else	if (x_c == WALL) {
		store_value(c_l, c_d, D_WALL, action);
		return;
	} else if (is_cannonball(x_c)) {
		log_i("F risk %c.\n", x_c);
		action[0] = c_d;
		store_value(c_l, c_d, D_RISK, action);
		return;
	} else if (x_c >= BLANK && x_c <= WATERMELON) {
		value = get_char_value(x_c);
	}
	log_i("forward %c.\n", x_c);

	/* forward-left */
	x_c = forward_left_char(m_obj, c_l, c_d);
	if (is_player(x_c)) {
		log_i("F-L stop&fire %c.\n", x_c);
		store_value(c_l, c_d, D_FIRE, action);
		return;
	}
	if (is_cannonball_attack(turn_left(c_d), x_c)) {
		log_i("F-L stop %c.\n", x_c);
		store_value(c_l, c_d, D_WALL, action);
		return;
	}

	/* forward-right*/
	x_c = forward_right_char(m_obj, c_l, c_d);
	if (is_player(x_c)) {
		log_i("F-R stop&fire %c.\n", x_c);
		action[1] = LAUNCH_YES;
		store_value(c_l, c_d, D_FIRE, action);
		return;
	}
	if (is_cannonball_attack(turn_right(c_d), x_c)) {
		log_i("F-R stop %c.\n", x_c);
		store_value(c_l, c_d, D_WALL, action);
		return;
	}

	/* forward-forward*/
	x_c = forward_forward_char(m_obj, c_l, c_d);
	if (is_player(x_c)) {
		x_l = *c_l;
		get_next_location(&x_l, c_d);
		get_next_location(&x_l, c_d);
		score = get_player_score(m_obj, &x_l);
		log_i("F-F compete %c(%d).\n", x_c, score);
		action[1] = LAUNCH_YES;
		value = D_RISK;
		if (score_i > score + 5) value = D_COMPETE;
		store_value(c_l, c_d, value, action);
		return;
	} else if (is_cannonball_attack(c_d, x_c)) {
		log_i("F-F fire %c.\n", x_c);
		action[0] = c_d;
		action[1] = LAUNCH_YES;
		value += D_FIRE;
	} else if (x_c >= BLANK && x_c <= WATERMELON) {
		value += get_char_value(x_c);
	}

	action[0] = c_d;
	/* forward-left-left */
	if (forward_left_char(m_obj, c_l, c_d) != WALL) {
		if (is_player(x_c=forward_left_left_char(m_obj, c_l, c_d))) {
			log_i("F-L-L risk %c.\n", x_c);
			value += D_RISK;
			store_value(c_l, c_d, value, action);
			return;
		}
	}
	if (forward_left_left_char(m_obj, c_l, c_d) == GHOST) {
		if (++xxx_ghost_count >= 2) {
			log_i("F-L-L risk ghost.\n");
			value += D_RISK;
			store_value(c_l, c_d, value, action);
			return;
		}
	}

	/* forward-forward-left */
	if (!(forward_left_char(m_obj, c_l, c_d) == WALL && forward_forward_char(m_obj, c_l, c_d) == WALL)) {
		if (is_player(x_c=forward_forward_left_char(m_obj, c_l, c_d))) {
			log_i("F-F-L risk %c.\n", x_c);
			value += D_RISK;
			store_value(c_l, c_d, value, action);
			return;
		}
	}
	if (forward_forward_left_char(m_obj, c_l, c_d) == GHOST) {
		if (++xxx_ghost_count >= 2) {
			log_i("F-F-L risk ghost.\n");
			value += D_RISK;
			store_value(c_l, c_d, value, action);
			return;
		}
	}

	/* forward-right-right*/
	if (forward_right_char(m_obj, c_l, c_d) != WALL) {
		if (is_player(x_c=forward_right_right_char(m_obj, c_l, c_d))) {
			log_i("F-R-R risk %c.\n", x_c);
			value += D_RISK;
			store_value(c_l, c_d, value, action);
			return;
		}
	}
	if (forward_right_right_char(m_obj, c_l, c_d) == GHOST) {
		if (++xxx_ghost_count >= 2) {
			log_i("F-R-R risk ghost.\n");
			value += D_RISK;
			store_value(c_l, c_d, value, action);
			return;
		}
	}

	/* forward-forward-right*/
	if (!(forward_right_char(m_obj, c_l, c_d) == WALL && forward_forward_char(m_obj, c_l, c_d) == WALL)) {
		if (is_player(x_c=forward_forward_right_char(m_obj, c_l, c_d))) {
			log_i("F-F-R risk %c.\n", x_c);
			value += D_RISK;
			store_value(c_l, c_d, value, action);
			return;
		}
	}
	if (forward_forward_right_char(m_obj, c_l, c_d) == GHOST) {
		if (++xxx_ghost_count >= 2) {
			log_i("F-F-R risk ghost.\n");
			value += D_RISK;
			store_value(c_l, c_d, value, action);
			return;
		}
	}

	/* forward-forward-forward*/
	if (forward_forward_char(m_obj, c_l, c_d) != WALL) {
		if (is_player(x_c=forward_forward_forward_char(m_obj, c_l, c_d))) {
			log_i("F-F-F stop&fire %c.\n", x_c);
			action[0] = STOP;
			action[1] = LAUNCH_YES;
			value += D_FIRE;
			store_value(c_l, c_d, value, action);
			return;
		}

		/* forward-forward-forward-forward*/
		if (forward_forward_forward_char(m_obj, c_l, c_d) != WALL) {
			if (is_player(forward_forward_forward_forward_char(m_obj, c_l, c_d))) {
				log_i("F-F-F-F fire.\n");
				action[1] = LAUNCH_YES;
				value += D_FIRE;
			}
		}
	}
	if (forward_forward_forward_char(m_obj, c_l, c_d) == GHOST) {
		if (++xxx_ghost_count >= 2) {
			log_i("F-F-F risk ghost.\n");
			value += D_RISK;
			store_value(c_l, c_d, value, action);
			return;
		}
	}

	store_value(c_l, c_d, value, action);
	return;
}

/*
veer:
i	x	i
i	>	i
*/
static void veer_value(struct map_obj *m_obj, char x_d, char x_c)
{
	int value = D_WALL;
	char action[2];
	struct coord *c_l;

	c_l = &m_obj->location[index_i];
	action[0] = x_d;
	action[1] = LAUNCH_NO;

	/* forward */
	if (x_c == GHOST) {
		log_i("turn %c kill %c.\n", x_d, x_c);
		action[1] = LAUNCH_YES;
		value = D_GHOST;
	} else if (is_player(x_c)) {
		log_i("turn %c kill %c.\n", x_d, x_c);
		action[1] = LAUNCH_YES;
		value = D_DANGER;
	} else if (is_cannonball_attack(x_d, x_c)) {
		log_i("turn (%c) clean %c.\n", x_d, x_c);
		action[1] = LAUNCH_YES;
		value = D_DANGER_C;
	} else if (x_c >= BLANK && x_c <= WATERMELON) {
		if (get_value(x_d, NULL) != D_INVALID) value = get_value(x_d, NULL);
		else value = get_char_value(x_c);
	} else if (x_c == WALL) value = D_WALL;
	else if (is_cannonball(x_c)) value = D_NONE;

	log_i("[%c-%c]: %d.\n",action[0], action[1], value);

	store_value(c_l, x_d, value, action);

	return;
}

static int is_veer_danger(char c_d)
{
	int count = 0;

	if (get_value(turn_left(c_d), NULL) >= D_DANGER) count++;
	if (get_value(turn_right(c_d), NULL) >= D_DANGER) count++;
	if (get_value(turn_back(c_d), NULL) >= D_DANGER) count++;

	if (count > 1) {
		log_i("can't veer(%d).\n", count);
		return 1;
	}

	return 0;
}

/*
		xx	
	xx		xx
xx		>		xx
	xx		xx
		xx
*/
static char ghost_interval_break(struct map_obj *m_obj, struct coord *l, char d)
{
	struct coord n_l;
	char dd, l_b, l_l, l_f, r_b, r_r, r_f, b_b, f_f;
	int count;

	l_b = l_l = l_f = r_b = r_r = r_f = b_b = f_f = 0;
	count = 0;

	n_l = *l;
	dd = turn_left(d);
	get_next_location(&n_l, dd);
	if (backward_char(m_obj, &n_l,  d) == GHOST) { l_b = 1; count++; };
	if (left_char(m_obj, &n_l,  d) == GHOST) { l_l = 1; count++; };
	if (forward_char(m_obj, &n_l, d) == GHOST) { l_f = 1; count++; };

	n_l = *l;
	dd = turn_right(d);
	get_next_location(&n_l, dd);
	if (backward_char(m_obj, &n_l, d) == GHOST) { r_b = 1; count++; };
	if (right_char(m_obj, &n_l,  d) == GHOST) { r_r = 1; count++; };
	if (forward_char(m_obj, &n_l, d) == GHOST) { r_f = 1; count++; };

	n_l = *l;
	dd = turn_back(d);
	get_next_location(&n_l, dd);
	if (backward_char(m_obj, &n_l, d) == GHOST) { b_b = 1; count++; };

	n_l = *l;
	get_next_location(&n_l, d);
	if (forward_char(m_obj, &n_l, d) == GHOST) { f_f = 1; count++; };

	if (l_l) return turn_left(d);
	if (r_r) return turn_right(d);
	if (b_b) return turn_back(d);
	if (f_f) return STOP;

	if (count == 0) return 0;
	else {
		if (d == LEFT || d == RIGHT) {
			if (l_f || r_f) return STOP;
			if (l_b || r_b) return turn_back(d);
		} else {
			if (l_b || l_f) return turn_left(d);
			if (r_b || r_f) return turn_right(d);
		}
	}

	return 0;
}

static char fruit_navigate(struct map_obj *m_obj)
{
	struct coord *c_l, x_l;
	char c_c;
	int value_l = 0, value_r = 0, value_u = 0, value_d = 0;

	c_l = &m_obj->location[index_i];

	if (get_value(LEFT, NULL) == D_NONE) {
		x_l = *c_l;
		while ((c_c = get_char(m_obj, get_next_location(&x_l, LEFT))) != WALL) {
			if (c_c >= BLANK && c_c <= WATERMELON) value_l += get_char_value(c_c);
		}
		log_i("fruit navigate left: %d.\n", value_l);
	}

	if (get_value(RIGHT, NULL) == D_NONE) {
		x_l = *c_l;
		while ((c_c = get_char(m_obj, get_next_location(&x_l, RIGHT))) != WALL) {
			if (c_c >= BLANK && c_c <= WATERMELON) value_r += get_char_value(c_c);
		}
		log_i("fruit navigate right: %d.\n", value_r);
	}

	if (get_value(UP, NULL) == D_NONE) {
		x_l = *c_l;
		while ((c_c = get_char(m_obj, get_next_location(&x_l, UP))) != WALL) {
			if (c_c >= BLANK && c_c <= WATERMELON) value_u += get_char_value(c_c);
		}
		log_i("fruit navigate up: %d.\n", value_u);
	}

	if (get_value(DOWN, NULL) == D_NONE) {
		x_l = *c_l;
		while ((c_c = get_char(m_obj, get_next_location(&x_l, DOWN))) != WALL) {
			if (c_c >= BLANK && c_c <= WATERMELON) value_d += get_char_value(c_c);
		}
		log_i("fruit navigate down: %d.\n", value_d);
	}

	c_c = LEFT;
	if (value_r > value_l) { value_l = value_r; c_c = RIGHT; }
	if (value_u > value_l) { value_l = value_u; c_c = UP; }
	if (value_d > value_l) { value_l = value_d; c_c = DOWN; }

	if (value_l > 0) return c_c;

	return 0;
}

int policy_do_v3(struct map_obj *m_obj, char *action)
{
	char action_tmp[3] = {STOP, LAUNCH_NO, '\0'};
	int value, value_tmp;
	struct coord *c_l;
	char c_d, g_d, n_d;

	index_i = m_obj->player_me;
	score_i = m_obj->score[index_i];
	c_l = &m_obj->location[index_i];
	c_d = get_char(m_obj, c_l);

	if (score_i < 0 || c_l->x < 0 || c_l->x >= m_obj->map_size_h ||
			c_l->y < 0 || c_l->y >= m_obj->map_size_v)
		return -EPROTO;

	if (c_d != LEFT && c_d != RIGHT && c_d != UP && c_d != DOWN)
		return -EPROTO;

	log_i("my info: %d(%d,%d) - %c.\n", score_i,
		m_obj->location[index_i].x, m_obj->location[index_i].y, c_d);

	forward_value(m_obj);
	veer_value(m_obj, turn_left(c_d), left_char(m_obj, c_l, c_d));
	veer_value(m_obj, turn_right(c_d), right_char(m_obj, c_l, c_d));
	veer_value(m_obj, turn_back(c_d), backward_char(m_obj, c_l, c_d));

	value = D_WALL;
	if (!is_veer_danger(c_d)) {
		value_tmp = get_value(turn_left(c_d), action_tmp);
		log_i("left: value=%d, action=%s.\n", value_tmp, action_tmp);
		if (value_tmp > value) {
			action[0] = action_tmp[0];
			action[1] = action_tmp[1];
			value = value_tmp;
		}
		value_tmp = get_value(turn_right(c_d), action_tmp);
		log_i("right: value=%d, action=%s.\n", value_tmp, action_tmp);
		if (value_tmp > value) {
			action[0] = action_tmp[0];
			action[1] = action_tmp[1];
			value = value_tmp;
		}
		value_tmp = get_value(turn_back(c_d), action_tmp);
		log_i("back: value=%d, action=%s.\n", value_tmp, action_tmp);
		if (value_tmp > value) {
			action[0] = action_tmp[0];
			action[1] = action_tmp[1];
			value = value_tmp;
		}

		g_d = ghost_interval_break(m_obj, c_l, c_d);
		if (g_d > 0 && value < D_GHOST) {
			log_i("fire ghost.\n");
			action[0] = g_d;
			action[1] = LAUNCH_YES;
			value = D_GHOST;
		}
	}
	value_tmp = get_value(c_d, action_tmp);
	log_i("foward: value=%d, action=%s.\n", value_tmp, action_tmp);
	if (value_tmp >= value || (value >= D_DANGER && action_tmp[0] != STOP && action[0] != turn_back(c_d))) {
			action[0] = action_tmp[0];
			action[1] = action_tmp[1];
			value = value_tmp;
	}

	if (value == D_NONE) {
		log_i("fruit navigate...\n");
		n_d = fruit_navigate(m_obj);
		if (n_d > 0) {
			action[0] = n_d;
		}
	}

	return 0;
}
