#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>

void input_handler ();
static int fd_dg;
char buffer;
int i = 1;
int main(int argc, char *argv[])
{
	printf("Hello World, I'm game!\n");
	
	int oflags;
	
	fd_dg = open("/dev/driver-gamepad", O_RDWR);
	if(fd_dg == -1) perror("Could not open driver-gamepad: \n");
	
	signal(SIGIO, &input_handler);
	fcntl(fd_dg, F_SETOWN, getpid());
	oflags = fcntl(fd_dg, F_GETFL);
	fcntl(fd_dg, F_SETFL, oflags | FASYNC);
	
	read(fd_dg, &buffer, 1);
	printf("Buffer value: %d\n", buffer);
	while(i){}	
	close(fd_dg);

	

	exit(EXIT_SUCCESS);
}

void input_handler () {
	printf("Input handler called in game.");
	char buffer;
	read(fd_dg, &buffer, 1);
	printf("Buffer value: %d\n", buffer);
	i = 0;
}

void write_framebuffer() {
	// int i;
	// int fd_fb;
	// uint16_t *map_fb;
	// ssize_t nof_b;
	// size_t nofPixels;

	// nofPixels = 320*240*2;
	// fd_fb = open("/dev/fb0", O_RDWR);
	// if (fd_fb == -1) {
	// 	perror("Error opening file: /dev/fb0");
	// 	exit(EXIT_FAILURE);
	// }
	// map_fb = mmap(NULL, nofPixels, PROT_WRITE, MAP_SHARED, fd_fb, 0);
	// if (map_fb == MAP_FAILED) {
	// 	close(fd_fb);
	// 	perror("Error making mmap for: /dev/fb0");
	// 	exit(EXIT_FAILURE);
	// }
	// for (i = 0; i < 40; i++) {
	// 	map_fb[i] = 'Z';
	
	// struct fb_copyarea rect;
	// rect.dx = 0;
	// rect.dy = 0;
	// rect.width = 5;
	// rect.height = 1;
	// ioctl(fd_fb,0x4680 ,&rect); 
	
	// }
	// if (munmap(map_fb, nofPixels) == -1) {
	// 	close(fd_fb);
	// 	perror("Failed to unmap map for: /dev/fb0");
	// 	exit(EXIT_FAILURE);
	// }
	// close(fd_fb);

}
