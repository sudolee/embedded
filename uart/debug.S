config_leds:
	/* configure GPB5~8 for leds */
	ldr r1, =0x56000010		@GPBCON
	ldr r0, [r1]
    bic r0, r0, #0x3fc00	@ clear [17:10]
	orr r0, r0, #(1<<16 | 1<<14 | 1<<12 | 1<<10)
	str r0, [r1]

	/* turn off all leds */
	ldr r1, =0x56000014		@GPBDAT
	ldr r0, [r1]
	orr r0, #0x1E0			@ mask all leds
	str r0, [r1]
	mov pc, lr

/*
 * void lights(int no)
 */
.globl lights
lights:
	stmdb sp!,{r4-r5,lr}
	and r0, r0, #0xF		@ which led(s) will be turn on

	ldr r5, =0x56000014		@ GPBDAT
	ldr r4, [r5]
	orr r4, #0x1E0			@ mask all leds
	bic r4, r4, r0, lsl #5
	str r4, [r5]

	bl delay
	orr r4, #0x1E0			@ turn off all leds
	str r4, [r5]

	ldmia sp!, {r4-r5,pc}

delay:
    ldr r0, =0x493DF
0:
    subs r0, r0, #1
    bne 0b
    mov pc, lr
