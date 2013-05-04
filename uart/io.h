#ifndef _IO_H_
#define _IO_H_

#define mb() __asm__ __volatile__("":::"memory")

#define readb(addr) (*(volatile u8  *)(addr))
#define readw(addr) (*(volatile u16 *)(addr))
#define readl(addr) (*(volatile u32 *)(addr))
#define writeb(addr, val) (*(volatile u8  *)(addr) = (val))
#define writew(addr, val) (*(volatile u16 *)(addr) = (val))
#define writel(addr, val) (*(volatile u32 *)(addr) = (val))

#endif /* _IO_H_ */
