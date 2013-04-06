
OPTIONS=-Os -fno-common -fno-builtin -ffreestanding -nostdinc -pipe -marm -march=armv4 -mcpu=arm920t -mno-thumb-interwork -mlittle-endian -mabi=atpcs -msoft-float

all:
	arm-linux-gnueabi-gcc ${OPTIONS} leds-speaker.s -c
	arm-linux-gnueabi-ld -e _start leds-speaker.o -o leds-speaker.elf
	arm-linux-gnueabi-objcopy -S -O binary leds-speaker.elf leds-speaker.bin

clean:
	rm -f leds-speaker.o leds-speaker.elf leds-speaker.bin
