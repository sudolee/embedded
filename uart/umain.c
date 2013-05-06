#include "type.h"
#include "io.h"
#include "print.h"

#ifdef DEBUG_LEDS
#include "debug.h"
#endif /* DEBUG_LEDS */

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

#if 0
static char gutc(struct uart_res *port)
{
	/* Is rx fifo empty ? */
	while(!(readl(&port->ufstat0) & 0x3F))
		;
	return readb(&port->rxdata);
}
#endif

static void putc(struct uart_res *port, const char ch)
{
	if(ch == '\n')
		putc(port, '\r');
		
	/* Is tx fifo full ? */
	while(readl(&port->ufstat0) & (1<<14))
		;
	writeb(&port->txdata, ch);
}

static void puts(struct uart_res *port, const char *str)
{
	while(*str)
		putc(port, *str++);
}

#define GPHCON_REG 0x56000070
static void init_uart0(struct uart_res *port)
{
	u32 gphcon;

	/* config gph in uart mode */
	gphcon = (readl(GPHCON_REG) & ~(0xF<<4)) | 0xA0;
	writel(GPHCON_REG, gphcon);

	/* normal mode, odd parity, 1stop bit, 8-bits data */
	writel(&port->ulcon0, 0<<6 | 4<<3 | 0<<2 | 3<<0);
	/* ucon0: receive and send must be set as interrupt or polling mode. */
	writel(&port->ucon0, 1<<2 | 1<<0);
	/* enable FIFO */
	writel(&port->ufcon0, 1);
	/*
	 * ubrdiv = (int)(Pclk/(baud-rate*16)) - 1
	 * 		  = 405MHz/(115KHz*16) - 1 = 36
	 */
	writel(&port->ubrdiv0, 36);
}

/* Export for other modlues */
struct uart_res *get_port_entry(int port_no)
{
#if 0 /* So far, only serial port 0 in use. */
	if(port_no == 2)
		return (struct uart_res *)0x50008000;
	else if(port_no == 1)
		return (struct uart_res *)0x50004000;
	else
#endif
		return (struct uart_res *)0x50000000;
}

/* TODO: size seems too small */
#define MAX_PRINTBUF_SIZE 80
long serial_printf(int port_no, const char *format, ...)
{
	va_list args;
	char printbuffer[MAX_PRINTBUF_SIZE];
	long rv;
	struct uart_res *port = get_port_entry(port_no);

	va_start(args, format);
	rv = vsnprintf(printbuffer, sizeof(printbuffer), format, args);
	va_end(args);

    puts(port, printbuffer);

	return rv;
}

void umain(void)
{
	struct uart_res *port0 = get_port_entry(0);
#ifdef DEBUG_LEDS
	lights(2);
#endif /* DEBUG_LEDS */

	init_uart0(port0);

	while(1) {
		serial_printf(0, "%c,%c,%c,%c\n", '-', '+', '*', '/');
		serial_printf(0, "%s\n", __func__);
		serial_printf(0, "%#x\n", (u32)umain);
		serial_printf(0, "%#X\n", (u32)umain);
		serial_printf(0, "%#o\n", 1234);
		serial_printf(0, "%#+5.4x\n", 12345678);
		serial_printf(0, "%#+5.4s\n", "hello world...");
		serial_printf(0, "%#x\n", 0x1234567);
	}

#ifdef DEBUG_LEDS
	lights(3);
#endif /* DEBUG_LEDS */
}
