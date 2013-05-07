CC = arm-linux-gnueabi-gcc
LD = arm-linux-gnueabi-ld
OBJCOPY = arm-linux-gnueabi-objcopy
OBJDUMP = arm-linux-gnueabi-objdump

OPTIONS = -Os -fno-common -fno-builtin -ffreestanding -nostdinc -pipe -marm \
		  -mcpu=arm920t -mno-thumb-interwork -mlittle-endian -mabi=atpcs -msoft-float \
		  -Wall -DDEBUG_LEDS

STACK_TOP := 0xFFF

all:
	${CC} ${OPTIONS} start.S -c -DSTACK_TOP=${STACK_TOP}
	${CC} ${OPTIONS} uart.c -c
	${CC} ${OPTIONS} clk.c -c
#	${CC} ${OPTIONS} print.c -c
	${CC} ${OPTIONS} nand.c -c
#	${LD} -Tbuild.lds start.o clk.o uart.o print.o -o Nand.elf
	${LD} -Tbuild.lds start.o clk.o uart.o nand.o -o Nand.elf
	${OBJCOPY} -S -O binary Nand.elf Nand.bin
	${OBJDUMP} -D Nand.elf > n.S

clean:
	rm -f *.o *.elf *.bin n.S
