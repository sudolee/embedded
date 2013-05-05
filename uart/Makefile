#CC = arm-linux-gnueabi-gcc -DDEBUG=1 -Wall
CC = arm-linux-gnueabi-gcc -Wall
LD = arm-linux-gnueabi-ld
OBJCOPY = arm-linux-gnueabi-objcopy
OBJDUMP = arm-linux-gnueabi-objdump

OPTIONS = -Os -fno-common -fno-builtin -ffreestanding -nostdinc -pipe -marm \
		  -mcpu=arm920t -mno-thumb-interwork -mlittle-endian -mabi=atpcs -msoft-float

all:
	${CC} ${OPTIONS} start.s -c
	${CC} ${OPTIONS} umain.c -c
	${CC} ${OPTIONS} clk.c -c
	${CC} ${OPTIONS} print.c -c
	${LD} -Tbuild.lds start.o clk.o umain.o print.o -o Uart.elf
	${OBJCOPY} -S -O binary Uart.elf Uart.bin
	${OBJDUMP} -D Uart.elf > u.s

clean:
	rm -f *.o *.elf *.bin u.s
