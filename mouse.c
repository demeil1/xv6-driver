#include "mouse.h"

void mouseinit(void)
{
	initlock(&mouse.lock, "mouse");

	// enable mouse
	mousewait_send();
	outb(MSSTATP, MSENABLE); 

	// enable controller interrupts
	mousewait_send();
	outb(MSSTATP, MS_READ_CMD_B);
	mousewait_recv();
	uchar status = inb(MSDATAP) | MSBUSY;
	mousewait_send();
	outb(MSSTATP, MS_WRITE_CMD_B);
	mousewait_send();
	outb(MSDATAP, status);

	// start data reporting 
	mousecmd(MSREPORTING);

	// link mouse driver callbacks
	devsw[MOUSE].write = mousewrite;
	devsw[MOUSE].read = mouseread;
	devsw[MOUSE].ioctl = mouseioctl;

	// route interrupt request for mouse 
	ioapicenable(IRQ_MOUSE, 0);
}

void mousewait_send() {
	while(inb(MSSTATP) & MSBUSY);
}

void mousewait_recv() { 
	while(!(inb(MSSTATP) & MSFREE));
}

void mousecmd(uchar cmd) {
	mousewait_send();
	outb(MSSTATP, MSCMD_WRITE);
	mousewait_send();
	outb(MSDATAP, cmd);
}

void mouseintr(void)
{
	acquire(&mouse.lock);
	while(inb(MSSTATP) & MSFREE) {
		uchar data = inb(MSDATAP);
		uint next = (mouse.head + 1) % MOUSE_BUF_SIZE;
		if(next != mouse.tail) {
			mouse.buf[mouse.head] = data;
			mouse.head = next;
		}
	}
	wakeup(&mouse);
	release(&mouse.lock);
}

int mouseread(struct file *f, char *dst, int n)
{
	uint cnt = 0;
	acquire(&mouse.lock);

	while(mouse.head == mouse.tail){
		if(proc->killed){
			release(&mouse.lock);
			return -1;
		}
		sleep(&mouse, &mouse.lock);
	}

	while(mouse.head != mouse.tail && cnt < n){
		if(copyout(proc->pgdir, (uint64)dst + cnt, &mouse.buf[mouse.tail], 1) < 0)
			break;

		mouse.tail = (mouse.tail + 1) % MOUSE_BUF_SIZE;
		cnt++;
	}

	release(&mouse.lock);
	return cnt;
}	


int mouseioctl(struct file* f, int param, int value) { 
	return 0; 
}

int mousewrite(struct file* f, char* buf, int n) {
	return n;
}
