
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define SCREEN_W 320
#define SCREEN_H 200
#define PALETTE_Y 180
#define PALETTE_SWATCH_W 20

// vga mode 13h colors
#define BLACK   0
#define BLUE    1
#define GREEN   2
#define CYAN    3
#define RED     4
#define MAGENTA 5
#define BROWN   20
#define WHITE   15

int colors[] = {BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, WHITE};
int num_colors = 8;

// color picker
void draw_palette() {
	for (int i = 0; i < num_colors; i++) {
		for (int y = PALETTE_Y; y < SCREEN_H; y++) {
			for (int x = i * PALETTE_SWATCH_W; x < (i + 1) * PALETTE_SWATCH_W; x++) {
				vgaplot(x, y, colors[i]);
			}
		}
	}
}

int main(void) {
	int fd;
	uchar packet[3];
	int x = SCREEN_W / 2;
	int y = SCREEN_H / 2;
	int oldx = x;
	int oldy = y;
	int current_color = WHITE;

	vgamode(13);

	// clear screen
	for(int i = 0; i < SCREEN_W; i++)
		for(int j = 0; j < SCREEN_H; j++)
			vgaplot(i, j, BLACK);

	draw_palette();

	fd = open("mouse", O_RDONLY);
	if(fd < 0){
		vgamode(3);
		printf(1, "paint: cannot open mouse device\n");
		exit();
	}

	while(1) {
		if(read(fd, packet, 3) != 3)
			continue;

		// check mouse sync
		if (!(packet[0] & 0x08)) {
			uchar dummy;
			read(fd, &dummy, 1);
			continue;
		}

		// erase cursor from CRT array
		if (oldy < PALETTE_Y) {
			vgaplot(oldx, oldy, BLACK);
			vgaplot(oldx+1,oldy,BLACK);
			vgaplot(oldx-1,oldy,BLACK);
			vgaplot(oldx,oldy+1,BLACK);
			vgaplot(oldx,oldy-1,BLACK);
		} else {
			draw_palette();
		}

		int left_btn = packet[0] & 0x01;
		int right_btn = packet[0] & 0x02;

		// slow down the brush so it doesn't move too fast
		int dx = ((signed char)packet[1]) / 2; 
		int dy = ((signed char)packet[2]) / 2;

		x += dx;
		y -= dy; // keep y inverted

		// boundary checks
		if (x < 0) x = 0;
		if (x >= SCREEN_W) x = SCREEN_W - 1;
		if (y < 0) y = 0;
		if (y >= SCREEN_H) y = SCREEN_H - 1;

		// draw
		if (left_btn) {
			if (y < PALETTE_Y) {
				// draw brush
				vgaplot(x, y, current_color);
				vgaplot(x+1, y, current_color);
				vgaplot(x, y+1, current_color);
				vgaplot(x+1, y+1, current_color);
			} else {
				// choose color
				int color_index = x / PALETTE_SWATCH_W;
				if (color_index < num_colors) current_color = colors[color_index];
			}
		}


		// draw cursor
		if(y < PALETTE_Y){
			vgaplot(x,y,current_color);

			vgaplot(x+1,y,current_color);
			vgaplot(x-1,y,current_color);
			vgaplot(x,y+1,current_color);
			vgaplot(x,y-1,current_color);
		}else{
			vgaplot(x,y,WHITE);
		}


		oldx = x;
		oldy = y;

		if (right_btn) {
			// clear drawing area
			for(int i = 0; i < SCREEN_W; i++) {
				for(int j = 0; j < PALETTE_Y; j++) {
					vgaplot(i, j, BLACK);
				}
			}
			break;
		}
	}
	close(fd);
	vgamode(3);
	exit();
}
