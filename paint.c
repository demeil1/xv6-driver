
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define SCREEN_W 320
#define SCREEN_H 200
#define PALETTE_Y 180
#define PALETTE_SWATCH_W 20

// Standard VGA Colors mapped to Mode 13h
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

// Draws the color picker at the bottom of the screen
void draw_palette() {
    for (int i = 0; i < num_colors; i++) {
        for (int y = PALETTE_Y; y < SCREEN_H; y++) {
            for (int x = i * PALETTE_SWATCH_W; x < (i + 1) * PALETTE_SWATCH_W; x++) {
                vgaplot(x, y, colors[i]); // Requires system call
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

    // Clear screen
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
        if(read(fd, packet, 3) == 3) {
            
            // 1. ALIGNMENT CHECK: Bit 3 of byte 0 must be 1.
            // If not, we are out of sync. Skip one byte and try again.
            if (!(packet[0] & 0x08)) {
                uchar dummy;
                read(fd, &dummy, 1);
                continue;
            }

            // Erase visual cursor dot
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

            // 2. SIGNED CASTING & SENSITIVITY
            // We divide by 2 (or 3) to slow the movement down.
            // This prevents the brush from "racing" away from the system cursor.
            int dx = ((signed char)packet[1]) / 2; 
            int dy = ((signed char)packet[2]) / 2;

            x += dx;
            y -= dy; // Keep Y inverted

            // 3. BOUNDARY CLAMPING
            if (x < 0) x = 0;
            if (x >= SCREEN_W) x = SCREEN_W - 1;
            if (y < 0) y = 0;
            if (y >= SCREEN_H) y = SCREEN_H - 1;

            // 4. DRAWING
            if (left_btn) {
                if (y < PALETTE_Y) {
                    // Draw brush
                    vgaplot(x, y, current_color);
                    vgaplot(x+1, y, current_color);
                    vgaplot(x, y+1, current_color);
                    vgaplot(x+1, y+1, current_color);
                } else {
                    // Change color
                    int color_index = x / PALETTE_SWATCH_W;
                    if (color_index < num_colors) current_color = colors[color_index];
                }
            }
	

            // 5. DRAW VISUAL CURSOR
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
    // Clear the drawing area (everything above the palette)
 		   for(int i = 0; i < SCREEN_W; i++) {
       			 for(int j = 0; j < PALETTE_Y; j++) {
           			 vgaplot(i, j, BLACK);
       				 }
   			 }
		   break;
		}
    }
    }
    close(fd);
    vgamode(3);
    exit();
}
