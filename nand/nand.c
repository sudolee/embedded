#include "type.h"
#include "io.h"
#include "uart.h"

#ifdef DEBUG_LEDS
#include "debug.h"
#endif /* DEBUG_LEDS */

struct nand_res {
};

void nand_main(void)
{
#ifdef DEBUG_LEDS
	lights(5);
#endif
	while(1) {
		puts(get_port_entry(0), "Hello, the curel world...\n");
		putslong(0x12345678);
	}
}
