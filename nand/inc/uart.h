#ifndef _UART_H_
#define _UART_H_
/* Export for other modules */

#include "type.h"

struct uart_res {
	u32 ulcon0;
	u32 ucon0;
	u32 ufcon0;
	u32 umcon0;
	u32 utrstat0;
	u32 uerstat0;
	u32 ufstat0;
	u32 umstat0;
/*
	u32 utxh0;
	u32 urxh0;
*/
#ifndef BIG_ENDIAN
	u8 txdata;
	u8 pad0[3];
	u8 rxdata;
	u8 pad1[3];
#else
	u8 pad0[3];
	u8 txdata;
	u8 pad1[3];
	u8 rxdata;
#endif
	u32 ubrdiv0;
};

void uart_setup(void);
struct uart_res *get_port_entry(int port_no);
void puts(const char *str);
void putslong(unsigned long n);
long serial_printf(const char *fmt, ...);

#endif /* _UART_H_ */
