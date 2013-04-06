
#.c
#arm-linux-gnueabi-gcc -g -Os -fno-common -msoft-float -I/home/sudolee/open/u-boot/include -fno-builtin -ffreestanding -nostdinc -pipe -marm -mno-thumb-interwork -mabi=aapcs-linux -march=armv4 -Wall -Wstrict-prototypes -fno-stack-protector -Wno-format-nonliteral -Wno-format-security -fstack-usage -o timer.o timer.c -c

#.s
#	arm-linux-gnueabi-gcc -g -Os -fno-common -msoft-float -I/home/sudolee/open/u-boot/include -fno-builtin -ffreestanding -nostdinc -pipe -marm -mno-thumb-interwork -mabi=aapcs-linux -march=armv4 -o start.o start.S -c

OPTIONS = -Os -fno-common -msoft-float -fno-builtin -ffreestanding -nostdinc -pipe -marm -mno-thumb-interwork -mabi=aapcs-linux -march=armv4

all:
	arm-linux-gnueabi-gcc ${OPTIONS} leds-speaker.s -c
	arm-linux-gnueabi-ld -e _start leds-speaker.o -o leds-speaker

clean:
	rm -f leds-speaker.o leds-speaker
