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
void timer_handler();
void button_handler ();
void framebuffer_init();
void framebuffer_cleanup();
void init_screen();
int is_Nth_bit_set(unsigned char b, int n);
void move_brick(int direction);
void paint_brick();
void create_asteroid();
void move_astroids();
int astroid_exists(int x_pos);
int add_astroid(int x_pos);
int astroid_hit_brick(int astroid);
astroid_hit_ground(int astroid);
void remove_astroid(int astroid);

void write_to_timer(uint16_t msecs);

int fd_dg, fd_fb, fd_timer;
uint16_t *map_fb;
size_t nofPixels;

/* Game variables */

#define HEIGHT 240
#define WIDTH 320
#define index(x, y) (x)+(y*WIDTH) 
#define BRICK_HEIGHT HEIGHT/16
#define BRICK_WIDTH WIDTH/6
#define BRICK_Y 210
#define BRICK_SPEED 2
#define ASTROID_INTERVAL 100
#define ASTROID_WIDTH 10
#define ASTROID_HEIGHT 10
#define TIMER_INTERVAL 5
#define MAX_NOF_ASTROIDS 10
#define ASTROID_SPEED 1

#define BACKGROUND_COLOUR 128
#define BRICK_COLOUR -50
#define ASTROID_COLOUR -124

typedef struct {
	int y;
	int x;
} Position;

const int START_POSITION = index(WIDTH/2 - (BRICK_WIDTH/2), BRICK_Y);
Position astroid_positions[MAX_NOF_ASTROIDS];
int current_position;
int next_astroid;


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
	signal(SIGIO, &signal_handler);
	fcntl(fd_dg, F_SETOWN, getpid());
	oflags_dg = fcntl(fd_dg, F_GETFL);
	fcntl(fd_dg, F_SETFL, oflags_dg | FASYNC);

	fd_timer = open("/dev/driver-timer", O_RDWR);
	if(fd_dg == -1) {
		perror("Could not open driver-gamepad: \n");
		exit(EXIT_FAILURE);
	}	
	fcntl(fd_timer, F_SETOWN, getpid());
	oflags_timer = fcntl(fd_timer, F_GETFL);
	fcntl(fd_timer, F_SETFL, oflags_timer | FASYNC);

	framebuffer_init();
	init_screen();
	write_to_timer(TIMER_INTERVAL);
	while(1);
	framebuffer_cleanup();
	close(fd_dg);
	

	exit(EXIT_SUCCESS);
}

void signal_handler () {
	char timer_zero;
	read(fd_timer, &timer_zero, 1);
	if (timer_zero) {
		timer_handler();
	}
	else {
		//button_handler();
	}
}

void timer_handler () {
	button_handler();
	move_astroids();
	if (next_astroid == 0) {
		create_asteroid();
		next_astroid = ASTROID_INTERVAL;
	}
	else next_astroid--;
	write_to_timer(TIMER_INTERVAL);
}

void button_handler () {
	char button_status;	
	read(fd_dg, &button_status, 1);
	if(!is_Nth_bit_set(button_status, 0)) {
		move_brick(-1);
	}
	else if (!is_Nth_bit_set(button_status, 2)) {
		move_brick(1);
	}
}

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
			map_fb[index(j,i)+1] = BACKGROUND_COLOUR;
		}
	}
	for (i = 0; i < BRICK_HEIGHT; i++) {
		for (j = 0; j < BRICK_WIDTH; j++) {
			map_fb[START_POSITION+index(j,i)] = BRICK_COLOUR;
			map_fb[START_POSITION+index(j,i)+1] = BRICK_COLOUR;
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

void move_brick(int direction) {
	int i,j;
	for (i = 0; i < BRICK_HEIGHT; i++) {
		for (j = 0; j < BRICK_WIDTH; j++) {
			map_fb[current_position+index(j,i)] = BACKGROUND_COLOUR;
			map_fb[current_position+index(j,i)+1] = BACKGROUND_COLOUR;
		}
	}
	current_position += direction*BRICK_SPEED;
	paint_brick();
}

void paint_brick() {
	int i,j;
	for (i = 0; i < BRICK_HEIGHT; i++) {
		for (j = 0; j < BRICK_WIDTH; j++) {
			map_fb[current_position+index(j,i)] = BRICK_COLOUR;
			map_fb[current_position+index(j,i)+1] = BRICK_COLOUR;
		}
	}
	struct fb_copyarea rect;
	rect.dx = (current_position%WIDTH)-5;
	rect.dy = BRICK_Y;
	rect.width = BRICK_WIDTH+11;
	rect.height = BRICK_HEIGHT+1;
	ioctl(fd_fb,0x4680 ,&rect);
}



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
			if (astroid_hit_brick(n)) remove_astroid(n);
			else if (astroid_hit_ground(n)) init_screen();
		}
	}
}

int astroid_hit_brick(int astroid) {
	int x, y;
	x = astroid_positions[astroid].x;
	y = astroid_positions[astroid].y;
	return (y + ASTROID_HEIGHT >= BRICK_Y) && (x >= current_position%WIDTH && x <= (current_position%WIDTH)+BRICK_WIDTH);

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
	rect.width = ASTROID_WIDTH+1;
	rect.height = ASTROID_HEIGHT+1;
	ioctl(fd_fb,0x4680 ,&rect);
	astroid_positions[astroid].x = -1;
}

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



