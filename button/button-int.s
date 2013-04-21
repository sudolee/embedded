
.section ".text.buttonint"

.globl _start

_start:
	b reset
	.rept 5
	b .
	.endr
	b arm_irq
#	ldr pc, =arm_irq
	b .

/*	1, store lr, spsr_irq
	2, store v1-v8 which is in use
	3, check interrupt
	4, handle irq
		- check whether EXINT8_23 occured ?
		- check which button pending in EINTPEND ?
	5, int ack
	6, return where we from
*/
arm_irq:
	msr cpsr, #(0x3<<6 | 0x12)		@ disable fiq+irq
	stmdb sp!, {r0-r5}				@
	sub r4, lr, #4
	mrs r5, spsr
	stmdb sp!, {r4,r5}

	bl irq_handle

	ldmia sp!, {r5}
	msr spsr, r5
	ldmia sp!, {pc, r0-r5}^			@ return where we from, and restore cpsr<- spsr_irq

irq_handle:

reset:
	# set the cpu to SVC32 mode, disable fiq and irq
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd3
	msr cpsr, r0

	# disable watchdog
	ldr r1, =0x53000000		@WTCON
	mov r0, #0x0
	str r0, [r1]

	# configure GPB5~8 for leds
	ldr r1, =0x56000010		@GPBCON
	ldr r0, [r1]
    bic r0, r0, #0x3fc00	@ clear [17:10]
    bic r0, r0, #0x00003	@ clear [1:0]
	orr r0, r0, #(1<<16 | 1<<14 | 1<<12 | 1<<10)
	str r0, [r1]

	# configure GPG, relate pins as int mode
	ldr r1, =0x56000060		@GPGCON
	ldr r0, [r1]
	ldr r2, =(0x3<<0 | 0x3<<6 | 0x3<<10 | 0x3<<12 | 0x3<<14 | 0x3<<22)	@ clear relate bits
	bic r0, r0, r2
	ldr r2, =(0x2<<0 | 0x2<<6 | 0x2<<10 | 0x2<<12 | 0x2<<14 | 0x2<<22)
	orr r0, r0, r2
	str r0, [r1]

	# configure INT attribute, falling edge trigger int(s).
	ldr r1, =0x5600008C		@EXTINT1
	ldr r0, [r1]
	ldr r2, =(0xF<<0 | 0xF<<12 | 0xF<<20 | 0xF<<24 | 0xF<<28)	@ clear relate bits
	bic r0, r0, r2
	ldr r2, =(0xA<<0 | 0xA<<12 | 0xA<<20 | 0xA<<24 | 0xA<<28)
	orr r0, r0, r2
	str r0, [r1]

	ldr r1, =0x56000090		@EXTINT2
	ldr r0, [r1]
	bic r0, r0, #(0xF<<12)
	orr r0, r0, #(0xA<<12)
	str r0, [r1]

	# mask EXTINT(s) in use
	ldr r1, =0x560000A4		@EINTMASK
	ldr r0, [r1]
	ldr r2, =(0x1<<8 | 0x1<<11 | 0x1<<13 | 0x1<<14 | 0x1<<15 | 0x1<<19)
	bic r0, r0, r2
	str r0, [r1]

	# configure intmsk register, only set EINT8_23 we focus on.
	ldr r1, =0x4A000008		@INTMSK
	ldr r0, [r1]
	bic r0, r0, #(0x1<<5)
	str r0, [r1]

	# init stack pointer
	ldr sp, =stack_top

main:
	b .
	nop
	nop

# void lights(int which);
lights:
	and r0, r0, #0x1F		@ which led(s) will be turn on/off

	ldr r3, =0x56000014		@GPBDAT
	ldr r2, [r3]
	orr r2, #0x1E0			@ mask all leds
	tst r0, #(1<<4)
	biceq r2, r2, r0, lsl #5
	str r2, [r3]
	bl delay				@ TODO: remove

	mov pc, lr

delay:
    ldr r0, =0x493DF
2:
    subs r0, r0, #1         @ Note: cpsr_f changed, bits[31:24]
    bne 2b
    mov pc, lr              @ return

.ltorg
.align 2		@ align to 4 bytes
stack_base:
	.skip 128
stack_top:

.end

