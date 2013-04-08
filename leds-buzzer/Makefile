
DEFINE += PWM

OPTIONS = -Os -fno-common -fno-builtin -ffreestanding -nostdinc -pipe -marm \
		  -march=armv4 -mcpu=arm920t -mno-thumb-interwork -mlittle-endian -mabi=atpcs -msoft-float

all:
	arm-linux-gnueabi-gcc ${OPTIONS} -D${DEFINE} leds-buzzer.s -c
	arm-linux-gnueabi-ld -e _start -Ttext 0x0 leds-buzzer.o -o leds-buzzer.elf
	arm-linux-gnueabi-objcopy -S -O binary leds-buzzer.elf leds-buzzer.bin
	arm-linux-gnueabi-objdump -D leds-buzzer.elf > asm.s

clean:
	rm -f leds-buzzer.o leds-buzzer.elf leds-buzzer.bin asm.s
