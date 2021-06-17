/*
 * TaskTimer.asm
 *
 *  Created: 14/04/2021 17:27:05
 *   Author: CHRISTIAM
 */ 

;----------------------------------------------------
; Macros
;----------------------------------------------------
 .macro dec8
	lds		r16,@0
	subi	r16,1
	brcs	dec8_zero
	sts		@0,r16
dec8_zero:
 .endm

 .macro dec16
	lds		xl,@0
	lds		xh,@0+1
	sbiw	xl,1
	brcs	dec16_zero
	sts		@0,xl
	sts		@0+1,xh
dec16_zero:
.endm

;----------------------------------------------------
; Variables
;----------------------------------------------------
 .dseg
 TaskTimer_Flags:	.byte 1
 Timer_cs:			.byte 1
 Timer_ds:			.byte 1
 Timer_s:			.byte 1

;----------------------------------------------------
; Task Timer
;----------------------------------------------------
 .cseg
 TaskTimer: 
	rcall	TaskTimer_Init
TaskTimer_Loop:
	Task_Change
	lds		r18,TaskTimer_Flags
	sbrs	r18,0
	rjmp	TaskTimer_Loop
	cbr		r18,0x01
;	Update ms soft timers	
	Dec8	Tmr_ms	
	
	sbrs	r18,1
	rjmp	TaskTimer_Update
	cbr		r18,0x02
;	Update cs soft timers	

	sbrs	r18,2
	rjmp	TaskTimer_Update
	cbr		r18,0x04
;	Update ds soft timers		
	dec8	Button_Tmr_TimeoutPressed_ds

	sbrs	r18,3
	rjmp	TaskTimer_Update
	cbr		r18,0x08
;	Update s soft timers
	dec8	Lora_Tmr_RxTimeout_s

TaskTimer_Update:
	sts		TaskTimer_Flags,r18
	rjmp	TaskTimer_Loop

;----------------------------------------------------
; ISR Timer 5D 
;----------------------------------------------------
ISR_TCd5_OVF:
	push	r16
	lds		r16,cpu_sreg
	push	r16
	push	r17
	push	r18

	ldi		r16,1
	sts		tcd5_intflags,r16		; Clear flag 
	
	lds		r17,TaskTimer_Flags
	ori		r17,1					; ms flag

	lds		r16,Timer_cs
	dec		r16
	sts		timer_cs,r16
	brne	ISR_TCd5_OVF_update
	ldi		r18,10
	sts		Timer_cs,r18
	ori		r17,0x02

	lds		r16,Timer_ds			; ds flag
	dec		r16
	sts		Timer_ds,r16
	brne	ISR_TCd5_OVF_update
	ldi		r18,10
	sts		Timer_ds,r18
	ori		r17,0x04

	lds		r16,Timer_s				; s flag
	dec		r16
	sts		Timer_s,r16
	brne	ISR_TCd5_OVF_update
	ldi		r18,10
	sts		Timer_s,r18
	ori		r17,0x08

ISR_TCd5_OVF_update:
	sts		TaskTimer_Flags,r17
	pop		r18
	pop		r17
	pop		r16
	sts		cpu_sreg,r16
	pop		r16
	reti

;----------------------------------------------------
TaskTimer_Init:
	ldi		r16,0x00
	sts		tcd5_ctrla,r16			;Turn off timer
	sts		tcd5_intctrla,r16
	sts		tcd5_intctrlb,r16		;Disable interruptions
	ldi		r16,0x00
	sts		tcd5_ctrlb,r16			;Mode: Normal
	sts		tcd5_ctrlc,r16			
	sts		tcd5_ctrld,r16			
	sts		tcd5_ctrle,r16			
	sts		tcd5_ctrlgclr,r16
	sts		tcd5_ctrlhclr,r16
	sts		tcd5_ctrlhset,r16
	sts		tcd5_cnt,r16
	sts		tcd5_cnt+1,r16

	ldi		r16,low(Main_frequency/1000-1)
	ldi		r17,high(Main_frequency/1000-1)
	sts		tcd5_per,r16
	sts		tcd5_per+1,r17

;	Clear local variables
	clr		r16
	sts		TaskTimer_Flags,r16
	ldi		r16,10
	sts		Timer_cs,r16
	sts		Timer_ds,r16
	sts		Timer_s,r16

	ldi		r16,0xf7
	sts		tcd5_intflags,r16		; Clear interruption flags
	ldi		r16,0x01
	sts		tcd5_intctrla,r16		; OVF active on low level
	ldi		r16,0x01				
	sts		tcd5_ctrla,r16			; Prescaler = 1
	
	ret

