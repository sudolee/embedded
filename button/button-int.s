.section ".text.buttonint"

.macro clear_pendr reg
	ldr r4, =\reg
	ldr r5, [r4]
	str r5, [r4]
.endm

.globl _start

_start:
	b reset
	.rept 5
	b .
	.endr
	b arm_irq
#	ldr pc, =arm_irq
	b .

arm_irq:
/*
	1, disable irq and fiq
	2, store lr, spsr_irq
	3, push r0-r12
	4, call handle irq
	5, pull r0-r12 and return where we from
*/
	msr cpsr_c, #(0x3<<6 | 0x12)		@ disable fiq+irq
	sub lr, lr, #4
	stmdb sp!, {r0-r12,lr}
	mrs r4, spsr
	stmdb sp!, {r4}

	bl irq_handle

	/* clear pending registers */
	clear_pendr 0x560000A8			@ eintpend, this must be clear before srcpnd and intpnd
	clear_pendr 0x4A000000			@ srcpnd, this must be clear before intpnd
	clear_pendr 0x4A000010			@ intpnd

	ldmia sp!, {r4}
	msr spsr_cxsf, r4
	ldmia sp!, {r0-r12,pc}^			@ return where we from, and restore cpsr<- spsr_irq

irq_handle:
/*
	1, check whether EINT8_23 ?
	2, check which button pushed ?
	3, light leds
	4, delay some time
	5, clear srcpend and intpend
*/
	stmdb sp!, {r4-r5,lr}

	/* Eint8_23 occured ? */
	ldr r4, =0x4A000014				@ intoffset
	ldr r5, [r4]
	cmp r5, #0x5					@ 0x5 -> EINT8_23
	bne 0f

	/* Which button pushed ? */
	ldr r4, =0x560000A8				@ EINTPEND
	ldr r5, [r4]
	ldr r4, =(0x1<<8 | 0x1<<11 | 0x1<<13 | 0x1<<14 | 0x1<<15 | 0x1<<19)		@ EINT(8,11,13,14,15,19) marks for buttons
	ands r5, r5, r4
	beq 0f							@ not expect EINT

	tst r5, #(0x1<<8)
	movne r0, #1
	bne 1f
	tst r5, #(0x1<<11)
	movne r0, #2
	bne 1f
	tst r5, #(0x1<<13)
	movne r0, #3
	bne 1f
	tst r5, #(0x1<<14)
	movne r0, #4
	bne 1f
	tst r5, #(0x1<<15)
	movne r0, #5
	bne 1f
	tst r5, #(0x1<<19)
	movne r0, #6
1:
	bl lights
0:
	ldmia sp!, {r4-r5,pc}

reset:
	# set the cpu to SVC32 mode, disable fiq and irq
	msr cpsr_c, #(0x13 | 0xC0)

	# init SVC mode stack pointer
	ldr sp, =stack_top

	# init irq mod registers
	msr cpsr_c, #(0x12 | 0xC0)		@ switch into irq mode
	ldr sp, =irq_stack_top
	mov lr, #0
	msr cpsr_c, #(0x13 | 0xC0)		@ switch into SVC mode

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

	# turn off all leds
	ldr r1, =0x56000014		@GPBDAT
	ldr r0, [r1]
	orr r0, #0x1E0			@ mask all leds
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

	# enable irq
	mrs r0, cpsr
	bic r0, r0, #(0x1<<7)
	msr cpsr_c, r0

main:
	b .						@ nothing to do

# void lights(int which);
lights:
	stmdb sp!,{r4-r5,lr}
	and r0, r0, #0xF		@ which led(s) will be turn on/off

	ldr r5, =0x56000014		@ GPBDAT
	ldr r4, [r5]
	orr r4, #0x1E0			@ mask all leds
	bic r4, r4, r0, lsl #5
	str r4, [r5]
	bl delay				@ TODO: remove

	orr r4, #0x1E0			@ mask all leds
	str r4, [r5]

	ldmia sp!, {r4-r5,pc}

delay:
#    ldr r0, =0x493DF
	mov r0, #0x19000
2:
    subs r0, r0, #1         @ Note: cpsr_f changed, bits[31:24]
    bne 2b
    mov pc, lr              @ return

.ltorg
.align 2		@ align to 4 bytes
stack_base:
	.skip 64
stack_top:

irq_stack_base:
	.skip 128
irq_stack_top:

.end
