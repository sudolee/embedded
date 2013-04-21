
CC = arm-linux-gnueabi-gcc
LD = arm-linux-gnueabi-ld
OBJCOPY = arm-linux-gnueabi-objcopy
OBJDUMP = arm-linux-gnueabi-objdump

OPTIONS = -Os -fno-common -fno-builtin -ffreestanding -nostdinc -pipe -marm \
		  -march=armv4 -mcpu=arm920t -mno-thumb-interwork -mlittle-endian -mabi=atpcs -msoft-float

all:
	${CC} ${OPTIONS} music-box.s -c
	${LD} -e _start -Ttext 0x0 music-box.o -o music-box.elf
	${OBJCOPY} -S -O binary music-box.elf music-box.bin
	${OBJDUMP} -D music-box.elf > mb.s

leds-buzzer:
	${CC} ${OPTIONS} leds-buzzer.s -c
	${LD} -e _start -Ttext 0x0 leds-buzzer.o -o leds-buzzer.elf
	${OBJCOPY} -S -O binary leds-buzzer.elf leds-buzzer.bin
	${OBJDUMP} -D leds-buzzer.elf > lb.s

clean:
	rm -f leds-buzzer.o leds-buzzer.elf leds-buzzer.bin lb.s
	rm -f music-box.o music-box.elf music-box.bin mb.s
