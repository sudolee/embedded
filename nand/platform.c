#include "clk.h"
#include "uart.h"
#include "nand.h"

extern void mem_setup(void);
extern void loadbin(void);

void platform_init(void)
{
	clk_setup();
	mem_setup();
	uart_setup();
	nand_setup();
}

void kmain(void)
{
	loadbin();
}
