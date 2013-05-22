#include "type.h"
#include "io.h"
#include "uart.h"

#ifdef DEBUG_LEDS
#include "debug.h"
#endif /* DEBUG_LEDS */

#define DEFAULT_SERIAL (0)

void *memcpy(void *dest, const void *src, size_t count)
{
	char *d = dest;
	const char *s = src;

	/* Not strcpy(), so, don't check '\0'. */
	while(count--)
		*d++ = *s++;
	return dest;
}

static char *long2hexstr(char *buf, unsigned long n, int len)
{
	const char digits[] = "0123456789abcdef";
	int mod;
	/*
	 * Before use digits[](in .rodata), asm need copy rodata into stack.
	 *   So, memcpy() is requested.
	 */

	buf[--len] = '\0';
	while(n > 0 && len > 0) {
		mod = n & 0xF;
		n >>= 4;
		buf[--len] = digits[mod];
	}

	if(len > 1) {
		buf[--len] = 'x';
		buf[--len] = '0';
	}
	return buf + len;
}

void putslong(unsigned long n)
{
	char buf[sizeof(void *)*2 + 3]; /* '0','x','\0' consume 3bytes */
  
	puts(long2hexstr(buf, n, sizeof(void *)*2 + 3));
}

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

void puts(const char *str)
{
	struct uart_res *port0 = get_port_entry(0);
	while(*str)
		putc(port0, *str++);
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
inline struct uart_res *get_port_entry(int port_no)
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

void uart_setup(void)
{
	struct uart_res *port0 = get_port_entry(0);

	init_uart0(port0);
}
