
.section ".text.ledbuzzer"

.globl _start
_start:
	b reset
	.rept 7
	b .
	.endr

	.balignl 32,0xdeadbeef

reset:
	/* set the cpu to SVC32 mode */
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd3
	msr cpsr_c, r0

	/* disable watchdog */
	ldr r1, =0x53000000 @WTCON
	mov r0, #0x0
	str r0, [r1]

#ifdef PWM
	/* configure-init PWM&TIMER0 */
	ldr r1, =0x51000004 @ TCFG1
	mov r0, #0x3
	str r0, [r1]		@ 1/16 divide

	ldr r8, =0x5100000C @ TCNTB0
	ldr r7, =65535
	str r7, [r8]

	ldr r6, =0x51000008 @ TCON
	ldr r5, [r6]
	orr r5, r5, #0xE	@ auto load, update manual and Timer0 inverter on
	str r5, [r6]

	bic r5, r5, #0x2
	orr r5, r5, #0x1
	str r5, [r6]		@ start timer0, and clear manual update
#endif

	/* configure GPB0, GPB5~8 as output */
	ldr r1, =0x56000010 @GPBCON
	ldr r0, [r1]
    bic r0, r0, #0x3fc00 @ clear [17:10]
    bic r0, r0, #0x3 @ clear [1:0]
	/* 1<<16 | 1<<14 | 1<<12 | 1<<10 | 1<<0 */
	orr r0, r0, #0x15400
#ifdef PWM
	orr r0, r0, #0x2	@ config GPB0 as Timer0 function
#else
	orr r0, r0, #0x1	@ config GPB0 as output
#endif
	str r0, [r1]

	/* GPB0, GPB5~8 pull up enable */
	ldr r1, =0x56000018 @GPBUP
	ldr r0, [r1]
	/* 1<<8 | 1<<7 | 1<<6 | 1<<5 | 1<<0 */
	bic r0, r0, #0x1E0
	bic r0, r0, #0x1
	str r0, [r1]

/*
 * Buzzer connected to GPB0, high level enable.
 * nLED1~4 connected to GPB5~8, low level enable.
 */
	ldr r1, =0x56000014 @GPBDAT
    mov r2, #0
	mov r3, #0x20			@ 0010 0000
Play:
	ldr r0, [r1]
	orr r0, #0x1E0			@ disable leds
	bic r0, r0, #1			@ disable buzzer
	bic r0, r0, r3, lsl r2
	cmp r2, #3
	moveq r2, #0
	moveq r3, #0x20
#ifdef PWM
	bleq timer_update
#else
	orreq r0, r0, #1
#endif
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

#ifdef PWM
timer_update:
	subs r7, r7, #0xF		@ TCNTB0
	ldrle r7, =65535		@ reset if <=
	str r7, [r8]			@ reset TCNTB0

	orr r5, r5, #0x2		@ update manual
	str r5, [r6]

	bic r5, r5, #0x2
	str r5, [r6]			@ clear manual

	mov pc, lr
#endif

.end
