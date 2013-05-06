#include "type.h"
#include "io.h"

struct clk_res {
	u32 locktime;
	u32 mpllcon;
	u32 upllcon;
	u32 clkcon;
	u32 clkslow;
	u32 clkdivn;
	u32 camdivn;
};

void clk_setup(void)
{
	int i;
	struct clk_res *clk = (struct clk_res *)0x4c000000;

	/* longest lock time */
	writel(&clk->locktime, 0xFFFFFFFF);

	writel(&clk->upllcon, 0x38<<12 | 0x2<<4 | 0x2<<0);
	for(i = 7; i > 0; i--);

	/* Fclk = 405.00MHz; mdiv=127, pdiv=2, sdiv=1 */
	writel(&clk->mpllcon, (0x7F<<12 | 2<<4 | 1<<0));

	/* Pclk:Hclk:Pclk = 6:3:1 */
	writel(&clk->clkdivn, (0<<3 | 3<<1 | 1<<0));
}
