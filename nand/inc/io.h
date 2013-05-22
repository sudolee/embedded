#ifndef _IO_H_
#define _IO_H_

#include "type.h"

#define mb() __asm__ __volatile__("":::"memory")

#define readb(addr) (*(volatile u8  *)(addr))
#define readw(addr) (*(volatile u16 *)(addr))
#define readl(addr) (*(volatile u32 *)(addr))
#define writeb(addr, val) (*(volatile u8  *)(addr) = (val))
#define writew(addr, val) (*(volatile u16 *)(addr) = (val))
#define writel(addr, val) (*(volatile u32 *)(addr) = (val))

static inline void insb(u32 addr, const void *data, int len)
{
	u8 *buf = (u8 *)data;
	while(len--)
		*buf++ = readb(addr);
}

static inline void insw(u32 addr, const void *data, int len)
{
	u16 *buf = (u16 *)data;
	while(len--)
		*buf++ = readw(addr);
}

static inline void insl(u32 addr, const void *data, int len)
{
	u32 *buf = (u32 *)data;
	while(len--)
		*buf++ = readl(addr);
}

static inline void outsb(u32 addr, const void *data, int len)
{
	u8 *buf = (u8 *)data;
	while(len--)
		writeb(addr, *buf++);
}

static inline void outsw(u32 addr, const void *data, int len)
{
	u16 *buf = (u16 *)data;
	while(len--)
		writew(addr, *buf++);
}

static inline void outsl(u32 addr, const void *data, int len)
{
	u32 *buf = (u32 *)data;
	while(len--)
		writel(addr, *buf++);
}

#endif /* _IO_H_ */
