
.section ".text.ledspeaker"
.globl _start
_start:
	b reset
    b _undefined
    b _syscall
    b _prefetch_abort
    b _data_abort
    b _reserved
    b _irq
    b _fiq

	.balignl 32,0xdeadbeef

_undefined:
_syscall:
_prefetch_abort:
_data_abort:
_reserved:
_irq:
_fiq:
	b .

reset:
	/* set the cpu to SVC32 mode */
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd3
	msr cpsr, r0

	/* disable watchdog */
	ldr r1, =0x53000000 @WTCON
	mov r0, #0x0
	str r0, [r1]

	/* configure GPB0, GPB5~8 as output */
	ldr r1, =0x56000010 @GPBCON
	ldr r0, [r1]
    bic r0, r0, #0x3fc00 @ clear [17:10]
    bic r0, r0, #0x3 @ clear [1:0]
	/* 1<<16 | 1<<14 | 1<<12 | 1<<10 | 1<<0 */
	orr r0, r0, #0x15400
	orr r0, r0, #0x1
	str r0, [r1]

	/* GPB0, GPB5~8 pull up enable */
	ldr r1, =0x56000018 @GPBUP
	ldr r0, [r1]
	/* 1<<8 | 1<<7 | 1<<6 | 1<<5 | 1<<0 */
	bic r0, r0, #0x1E0
	bic r0, r0, #0x1
	str r0, [r1]

/*
 * Speaker connected to GPB0, high level enable.
 * nLED1~4 connected to GPB5~8, low level enable.
 */
	ldr r1, =0x56000014 @GPBDAT
    mov r2, #0
	mov r3, #0x20			@ 0010 0000
Play:
	ldr r0, [r1]
	orr r0, #0x1E0			@ disable leds
	bic r0, r0, #1			@ disable speaker
	bic r0, r0, r3, lsl r2
	cmp r2, #3
	moveq r2, #0
	orreq r0, r0, #1
	moveq r3, #0x20
	addne r2, r2, #1
	str r0, [r1]
	bl delay
	b Play

delay:
    mov r4, #0x19000
0:
    subs r4, r4, #1         @ Note: cpsr_f changed, bits[31:24]
    bne 0b
    mov pc, lr              @ return

.end
