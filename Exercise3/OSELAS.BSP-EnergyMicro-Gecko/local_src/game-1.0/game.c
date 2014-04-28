#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

void signal_handler();
void astroid_handler();
void shield_handler ();
void framebuffer_init();
void framebuffer_cleanup();
void init_screen();
int is_Nth_bit_set(unsigned char b, int n);
void move_shield(int direction);
void paint_shield(); 
void create_asteroid();
void move_astroids();
int astroid_exists(int x_pos);
int add_astroid(int x_pos);
int astroid_hit_shield(int astroid);
astroid_hit_ground(int astroid);
void remove_astroid(int astroid);

void write_to_timer(uint16_t msecs);

int fd_dg, fd_fb, fd_timer, fd_sound;
uint16_t *map_fb; /* Frambuffer map */
size_t nofPixels;

/* Game variables */

#define HEIGHT 240
#define WIDTH 320
#define index(x, y) (x)+(y*WIDTH) 
#define SHIELD_HEIGHT HEIGHT/16
#define SHIELD_WIDTH WIDTH/6
#define SHIELD_Y 210
#define SHIELD_SPEED 2
#define ASTROID_INTERVAL 100
#define ASTROID_WIDTH 10
#define ASTROID_HEIGHT 10
#define TIMER_INTERVAL_MSECS 5
#define MAX_NOF_ASTROIDS 10
#define ASTROID_SPEED 1

#define BACKGROUND_COLOUR 128
#define SHIELD_COLOUR -50
#define ASTROID_COLOUR -124

typedef struct {
	int y;
	int x;
} Position;

const int START_POSITION = index(WIDTH/2 - (SHIELD_WIDTH/2), SHIELD_Y); /* Start pos of the shield */
Position astroid_positions[MAX_NOF_ASTROIDS];
int current_position; /* Current shield position */
int next_astroid; /* Nof intervals left before new astroid */


int main(int argc, char *argv[])
{
	printf("Hello World, I'm game!\n");
	srand(time(NULL));	
	int oflags_dg, oflags_timer;
	
	fd_dg = open("/dev/driver-gamepad", O_RDWR);
	if(fd_dg == -1) {
		perror("Could not open driver-gamepad: \n");
		exit(EXIT_FAILURE);
	}	
	

	fd_timer = open("/dev/driver-timer", O_RDWR);
	if(fd_dg == -1) {
		perror("Could not open driver-gamepad: \n");
		exit(EXIT_FAILURE);
	}	
	signal(SIGIO, &signal_handler);
	fcntl(fd_timer, F_SETOWN, getpid());
	oflags_timer = fcntl(fd_timer, F_GETFL);
	fcntl(fd_timer, F_SETFL, oflags_timer | FASYNC);

	framebuffer_init();
	init_screen();
	write_to_timer(TIMER_INTERVAL_MSECS);
	while(1) sleep(100);
	framebuffer_cleanup();
	close(fd_dg);
	close(fd_timer);
	

	exit(EXIT_SUCCESS);
}

/******************/
/* Event handlers */
/******************/
void signal_handler () {
	astroid_handler();
	shield_handler();
}

void astroid_handler () {
	move_astroids();
	if (next_astroid == 0) {
		create_asteroid();
		next_astroid = ASTROID_INTERVAL;
	}
	else next_astroid--;
	write_to_timer(TIMER_INTERVAL_MSECS);
}

void shield_handler () {
	char button_status;	
	read(fd_dg, &button_status, 1);
	if(!is_Nth_bit_set(button_status, 0)) {
		move_shield(-1);
	}
	else if (!is_Nth_bit_set(button_status, 2)) {
		move_shield(1);
	}
}

/*********************************/
/* Framebuffer and initalization */
/*********************************/

void framebuffer_init() {

	nofPixels = WIDTH*HEIGHT*2;
	fd_fb = open("/dev/fb0", O_RDWR);
	if (fd_fb == -1) {
		perror("Error opening file: /dev/fb0");
		exit(EXIT_FAILURE);
	}
	map_fb = mmap(NULL, nofPixels, PROT_WRITE, MAP_SHARED, fd_fb, 0);
	if (map_fb == MAP_FAILED) {
		close(fd_fb);
		perror("Error making mmap for: /dev/fb0");
		exit(EXIT_FAILURE);
	}
}

void init_screen() {
	int i, j;
	next_astroid = 0;
	memset(&astroid_positions, -1, sizeof(Position)*10);

	current_position = START_POSITION;
	for (i = 0; i < HEIGHT; i++) {
		for (j = 0; j < WIDTH; j++) {
			map_fb[index(j,i)] = BACKGROUND_COLOUR;
		}
	}
	for (i = 0; i < SHIELD_HEIGHT; i++) {
		for (j = 0; j < SHIELD_WIDTH; j++) {
			map_fb[START_POSITION+index(j,i)] = SHIELD_COLOUR;
		}
	}

	
	struct fb_copyarea rect;
	rect.dx = 0;
	rect.dy = 0;
	rect.width = WIDTH;
	rect.height = HEIGHT;
	ioctl(fd_fb,0x4680 ,&rect); 
}

void framebuffer_cleanup() {
	if (munmap(map_fb, nofPixels) == -1) {
		close(fd_fb);
		perror("Failed to unmap map for: /dev/fb0");
		exit(EXIT_FAILURE);
	}
	close(fd_fb);
}

/******************/
/*    Shield      */
/******************/

void move_shield(int direction) {
	int i,j;
	/* Remove shield */
	for (i = 0; i < SHIELD_HEIGHT; i++) {
		for (j = 0; j < SHIELD_WIDTH; j++) {
			map_fb[current_position+index(j,i)] = BACKGROUND_COLOUR;
			map_fb[current_position+index(j,i)+1] = BACKGROUND_COLOUR;
		}
	}
	current_position += direction*SHIELD_SPEED;
	/* Repaint at new position */
	paint_shield();
}

void paint_shield() {
	int i,j;
	for (i = 0; i < SHIELD_HEIGHT; i++) {
		for (j = 0; j < SHIELD_WIDTH; j++) {
			map_fb[current_position+index(j,i)] = SHIELD_COLOUR;
			map_fb[current_position+index(j,i)+1] = SHIELD_COLOUR;
		}
	}
	struct fb_copyarea rect;
	rect.dx = (current_position%WIDTH)-5;
	rect.dy = SHIELD_Y;
	rect.width = SHIELD_WIDTH+11;
	rect.height = SHIELD_HEIGHT+1;
	ioctl(fd_fb,0x4680 ,&rect);
}

/******************/
/*    Astroids    */
/******************/

void create_asteroid() {
	int i, j;
	int position_astroid;
	do{
		position_astroid = rand() % (WIDTH - ASTROID_WIDTH);
	}
	while(astroid_exists(position_astroid));

	if (add_astroid(position_astroid) == 0) return;

	for (i = 0; i < ASTROID_HEIGHT; i++) {
		for (j = position_astroid; j < position_astroid + ASTROID_WIDTH; j++) {
			map_fb[index(j, i)] = ASTROID_COLOUR;
		}
	}

	struct fb_copyarea rect;
	rect.dx = position_astroid;
	rect.dy = 0;
	rect.width = ASTROID_WIDTH+1;
	rect.height = ASTROID_HEIGHT+1;
	ioctl(fd_fb,0x4680 ,&rect);
}

void move_astroids() {
	int i,j,n;
	for (n = 0; n < MAX_NOF_ASTROIDS; n++) {
		if (astroid_positions[n].x != -1) {
			int height = astroid_positions[n].y;
			int width = astroid_positions[n].x;
			for (i = height; i < height+ASTROID_SPEED; i++) {
				for (j = width; j < width + ASTROID_WIDTH; j++) {
					map_fb[index(j, i)] = BACKGROUND_COLOUR;
				}
			}
			height += ASTROID_HEIGHT;
			for (i = height; i < height+ASTROID_SPEED; i++) {
				for (j = width; j < width + ASTROID_WIDTH; j++) {
					map_fb[index(j, i)] = ASTROID_COLOUR;
				}
			}

			struct fb_copyarea rect;
			rect.dx = astroid_positions[n].x;
			rect.dy = astroid_positions[n].y;
			rect.width = ASTROID_WIDTH+1;
			rect.height = ASTROID_HEIGHT+ASTROID_SPEED+2;
			ioctl(fd_fb,0x4680 ,&rect);
			astroid_positions[n].y += ASTROID_SPEED;
			if (astroid_hit_shield(n)) {
				remove_astroid(n);
				paint_shield();
			} 
			else if (astroid_hit_ground(n)) init_screen();

		}
	}
}

int astroid_hit_shield(int astroid) {
	int x, y;
	x = astroid_positions[astroid].x;
	y = astroid_positions[astroid].y;
	return (y + ASTROID_HEIGHT >= SHIELD_Y) && (x >= current_position%WIDTH && x <= (current_position%WIDTH)+SHIELD_WIDTH);

}

int astroid_hit_ground(int astroid) {
	return astroid_positions[astroid].y + ASTROID_HEIGHT >= HEIGHT;
}

void remove_astroid(int astroid) {
	int i, j;
	int x, y;
	x = astroid_positions[astroid].x;
	y = astroid_positions[astroid].y;

	for (i = y; i < y + ASTROID_HEIGHT; i++) {
		for (j = x; j < x + ASTROID_WIDTH; j++) {
			map_fb[index(j, i)] = BACKGROUND_COLOUR;
		}
	}

	struct fb_copyarea rect;
	rect.dx = x;
	rect.dy = y;
	rect.width = ASTROID_WIDTH+2;
	rect.height = ASTROID_HEIGHT+1;
	ioctl(fd_fb,0x4680 ,&rect);
	astroid_positions[astroid].x = -1;
}

/******************/
/* Help functions */
/******************/

int is_Nth_bit_set(unsigned char b, int n) {
	return (1 << n) & b;
}

void write_to_timer(uint16_t msecs) {
	char timer_msecs[2];
	timer_msecs[0] = msecs & 0xff;
	timer_msecs[1] = (msecs>>8) & 0xff;
	write(fd_timer, &timer_msecs, 2);
}

int astroid_exists(int x_pos) {
	int i;
	for (i = 0; i < MAX_NOF_ASTROIDS; i++) {
		if (x_pos >= astroid_positions[i].x && x_pos <= astroid_positions[i].x + ASTROID_WIDTH) 
			return 1;
	}
	return 0;
}

int add_astroid(int x_pos) {
	int i;
	for (i = 0; i < MAX_NOF_ASTROIDS; i++) {
		if (astroid_positions[i].x == -1) {
			astroid_positions[i].x = x_pos;
			astroid_positions[i].y = 0;
			return 1;
		}
	}
	return 0;

}



