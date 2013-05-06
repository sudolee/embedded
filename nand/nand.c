#include "type.h"
#include "io.h"
#include "uart.h"

struct nand_res {
};

void nand_main(void)
{
	while(1)
		puts(get_port_entry(0), "hello world....\n");
}
