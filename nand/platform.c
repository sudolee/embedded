#include "uart.h"
#include "nand.h"

extern void clk_setup(void);
extern void loadbin(void);

static void platform_init(void)
{
	clk_setup();
	uart_setup();
	nand_setup();
}

void platform_main(void)
{
	platform_init();
	loadbin();
}
