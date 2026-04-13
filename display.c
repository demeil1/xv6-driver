#include "types.h"
#include "defs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "vga.h"
#include "memlayout.h"

#define VIDBUF ((char*)(KERNBASE + 0xA0000))
#define VGA13W 320 
#define VGA13H 200
#define VGA13S (VGA13W * VGA13H)

static struct {
  struct spinlock lock;
} disp;

int displayioctl(struct file* f, int param, int value)
{
	if (value == 0x3) {
		vgaMode3();
		//cgaRestorePalette();
		return 0;
	} else if (value == 0x13) {
		//cgaRestorePalette();
		vgaMode13();
		return 0;
	}

	unsigned char pltnum, r, g, b;

	pltnum = value >> 24;
	
	r = (value << 8) >> 24;
	g = (value << 16) >> 24;
	b = (value << 24) >> 24;

	vgaSetPalette(pltnum, r, g, b);

	return 0;
}

int displaywrite(struct file* f, char* buf, int n)
{
	acquire(&disp.lock);
	for (uint32 i = 0; i < n; i++)
		VIDBUF[f->off + i] = buf[i];
	release(&disp.lock);

	f->off += n;
	if (f->off >= VGA13S)
		f->off = 0;

	return n;
}

void displayinit(void)
{
	initlock(&disp.lock, "display");

	devsw[DISPLAY].write = displaywrite;
	devsw[DISPLAY].read = 0x0;
	devsw[DISPLAY].ioctl = displayioctl;
}
