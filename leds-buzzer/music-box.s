
.section ".text.musicbox"

.globl _start
_start:
	b reset
	.rept 7
	b .
	.endr

reset:
	# set the cpu to SVC32 mode, disable fiq and irq #
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd3
	msr cpsr, r0

	# disable watchdog #
	ldr r1, =0x53000000		@WTCON
	mov r0, #0x0
	str r0, [r1]

	# configure GPB0, GPB5~8 as output #
	ldr r1, =0x56000010		@GPBCON
	ldr r0, [r1]
    bic r0, r0, #0x3fc00	@ clear [17:10]
    bic r0, r0, #0x00003	@ clear [1:0]
	# 1<<16 | 1<<14 | 1<<12 | 1<<10 | 1<<0 #
	orr r0, r0, #0x15400
	orr r0, r0, #0x00002	@ config GPB0 as Timer0 function
	str r0, [r1]

	# GPB0, GPB5~8 pull up enable #
	ldr r1, =0x56000018 @GPBUP
	ldr r0, [r1]
	# 1<<8 | 1<<7 | 1<<6 | 1<<5 | 1<<0 #
	bic r0, r0, #0x1E0
	bic r0, r0, #0x001
	str r0, [r1]

	# set stack pointer
	ldr r12, =tiny_stack_top
#
# void main(void);
# @r11: value of next pitch
main:
	bl next_pitch
	mov r11, r0			@ save value of next pitch

	bl get_pwm_conf

	tst r0, #0x30		@ is low or high pitch ?
	beq tcfg0_updated	@ migh pitch, use default tcfg0 
	tst r0, #(1<<5)
	movne r0, #3		@ low pitch, set tcfg0 1/16
	tst r0, #(1<<6)
	movne r0, #1		@ high pitch, set tcfg0 1/4

tcfg0_updated:
	# now: r0=tcfg1, r1=tcfg0, r2=tcntb0
	bl pwm_update

	and r1, r11, #0xF00
	cmp r1, #0
	moveq r0, #4		@ this pitch keep 400ms, time=1beat
	beq 0f
	tst r11, #(1<<8)
	movne r0, #2		@ this pitch keep 200ms, time=beat/2
	tst r11, #(1<<9)
	movne r0, #1		@ this pitch keep 100ms, time=beat/4
	#TODO: 1/8 not used#
0:
	bl lights
	b main

#
# int next_pitch(void);
# return value of next pitch
next_pitch:
	ldr r2, =goddess_start
	ldr r1, =goddess_current_offset
	ldr r3, [r1]
	ldr r0, [r2, r3]		@ return r0
	add r3, r3, #4			@ update goddess_current_offset
	cmp r3, #(goddess_start - goddess_end)
	movgt r3, #0
	str r3, [r1]
	mov pc, lr
.ltorg
goddess_current_offset:
	.word 0

# void get_pwm_conf(int pitch_value)
get_pwm_conf:
	ldr r1, =doremi_start
	and r0, r0, #0xF
	add r3, r1, r0
	ldm r3, {r0-r2}
	mov pc, lr

#
# void pwm_update(int tcfg1, int tcfg0, int tcntb0, int tcmpb0);
# So far, tcmpb0 not used.
#
pwm_update:
	stmdb r12!, {r4,r5,lr}

	# tcfg0 configure
	and r1, r1, #0xFF
	ldr r4, =0x51000000 @ TCFG0 -> [7:0] for timer0/1 prescaler(0~255)
	ldr r5, [r4]
	bic r5, r5, #0xFF	@ clear [7:0]
	orr r1, r1, r5		@ set [7:0]
	str r1, [r4]

	# TCFG1 -> [3:0] 0000=1/2,0001=1/4,0010=1/8,0011=1/16,01xx=external tclk0
	and r0, r0, #0xF
	ldr r4, =0x51000004
	ldr r5, [r4]
	bic r5, r5, #0xF	@ clear [3:0]
	orr r0, r0, r5		@ set [3:0]
	str r0, [r4]		@ divide ?

	ldr r4, =0x5100000C @ TCNTB0
	str r2, [r4]

	mov r5, #0x0
	ldr r4, =0x51000010 @ TCMPB0
	str r5, [r4]

	ldr r4, =0x51000008 @ TCON
	ldr r5, [r4]
	orr r5, r5, #0xE	@ auto load + update manual + Timer0 inverter on
	str r5, [r4]

	bic r5, r5, #0x2	@ clear manual update
	orr r5, r5, #0x1	@ if not start, let it run
	str r5, [r4]

	ldmia r12!, {r4,r5,pc}

# void lights(int delay);
lights:
	stmdb r12!, {r4-r8,lr}

	ldr r5, =0x56000014 @GPBDAT
	ldr r1, =lights_counter
	ldm r1, {r6,r7}
#	mov r6, #0
#	mov r7, #0x20			@ 0010 0000
1:
	ldr r4, [r5]
	orr r4, #0x1E0			@ mask all leds
	bic r4, r4, r7, lsl r6
	cmp r6, #3
	moveq r6, #0
	moveq r7, #0x20
	addne r6, r6, #1
	str r4, [r5]
	bl delay
	subs r0, r0, #1
	beq 2f
	b 1b
2:
	stm r1, {r6,r7}
	ldmia r12!, {r4-r8,pc}
.ltorg
lights_counter:
	.word 0, 0x20			@ r6, r7

#
# Input clock is ?
# delay = beat / 4 = 0.4/4 s = 100ms
# delay = (4n + 3)/Hclk
# n = (0.1*12*10^6 - 3)/4 = 0x493DF
#
delay:
    ldr r0, =0x493DF
3:
    subs r0, r0, #1         @ Note: cpsr_f changed, bits[31:24]
    bne 3b
    mov pc, lr              @ return

#                     Bits Map
# +----------+-----+-----+-----+-----+-----------------------+
# | reserved | beat                  | level      | pitch    |
# +----------+-----+-----+-----+-----+-----------------------+
# | 31 ~ 12  | 11  | 10  |  9  |  8  | 7 ~ 4      | 3 ~ 0    |
# +----------+-----+-----+-----+-----+-----------------------+
# |          | 1/16| 1/8 | 1/4 | 1/2 | 0000: migh | 0 ~ 7    |
# +----------+-----+-----+-----+-----+ 0001: low  +----------+
#                                    | 0010: high |
#                                    +------------+
.ltorg
.align 2
goddess_start:
	.word 3, 3, 4, 5, 5, 4, 3, 2, 1, 1, 2, 3, 3, 3+(1<<8), 2+(1<<8), 2, 2
goddess_end:
	.long .

#
#  struct doremi {
#    int tcfg0;
#    int tcfg1;
#    int tcntb0;
#  }
doremi_start:			@ migh pitch
	.word 2, 0, 5108	@ do
	.word 2, 0, 4551	@ re
	.word 2, 0, 4295	@ mi
	.word 2, 0, 3827	@ fa
	.word 2, 0, 3409	@ sol
	.word 2, 0, 3037	@ la
	.word 2, 0, 2867	@ si
doremi_end:
	.long .

.bss
.align 2
tiny_stack:
	.skip 128
tiny_stack_top:
	.long .

.end
