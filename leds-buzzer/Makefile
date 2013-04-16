
OPTIONS = -Os -fno-common -fno-builtin -ffreestanding -nostdinc -pipe -marm \
		  -march=armv4 -mcpu=arm920t -mno-thumb-interwork -mlittle-endian -mabi=atpcs -msoft-float

all:
	arm-linux-gnueabi-gcc ${OPTIONS} music-box.s -c
	arm-linux-gnueabi-ld -e _start -Ttext 0x0 music-box.o -o music-box.elf
	arm-linux-gnueabi-objcopy -S -O binary music-box.elf music-box.bin
	arm-linux-gnueabi-objdump -D music-box.elf > mb.s

leds-buzzer:
	arm-linux-gnueabi-gcc ${OPTIONS} leds-buzzer.s -c
	arm-linux-gnueabi-ld -e _start -Ttext 0x0 leds-buzzer.o -o leds-buzzer.elf
	arm-linux-gnueabi-objcopy -S -O binary leds-buzzer.elf leds-buzzer.bin
	arm-linux-gnueabi-objdump -D leds-buzzer.elf > lb.s

clean:
	rm -f leds-buzzer.o leds-buzzer.elf leds-buzzer.bin lb.s
	rm -f music-box.o music-box.elf music-box.bin mb.s
