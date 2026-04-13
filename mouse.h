#include "types.h"
#include "defs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "x86.h"
#include "traps.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"

// interface
#define MSSTATP		0x64 // status register
#define MSDATAP		0x60 // data register

// states
#define MSFREE		0x01 // ready status
#define MSBUSY		0x02 // busy status

// commands
#define MSENABLE	0xA8 // enable command
#define MSREPORTING	0xF4 // data reporting
#define MSCMD_WRITE	0xD4 // write command from  
#define MS_READ_CMD_B	0x20 // read command byte
#define MS_WRITE_CMD_B  0x60 // write command byte

#define MOUSE_BUF_SIZE 128

static struct {
  struct spinlock lock;
  uchar buf[MOUSE_BUF_SIZE];
  uint head;
  uint tail;
} mouse;

// mouse driver callbacks
int mouseread(struct file*, char*, int);
int mousewrite(struct file* f, char* buf, int n);
int mouseioctl(struct file*, int, int);

// mouse driver helper functions
void mousewait_send();
void mousewait_recv();
void mousecmd(uchar);
