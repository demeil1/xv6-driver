#include "types.h"
#include "user.h"
#include "types.h"
#include "user.h"

int main(void) {
	int fd = open("mouse", 0);
	if(fd < 0) {
		printf(1, "failed to open mouse\n");
		exit();
	}

	uchar buf[3];
	int x_total = 0;
	int y_total = 0;

	printf(1, "mouse test started\n");

	while(1) {
		if(read(fd, buf, 3) != 3) {
			continue;
		}

		int dx = (char)buf[1];
		int dy = (char)buf[2];

		x_total += dx;
		y_total += dy;

		printf(1, "Btns: %d | dx: %d dy: %d | X: %d Y: %d\n", 
				buf[0] & 0x07, dx, dy, x_total, y_total);
	}

	exit();
}
