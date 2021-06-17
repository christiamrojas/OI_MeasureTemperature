;----------------------------------------------------
; Rtc
;----------------------------------------------------
;.dseg
;Rtc_Flag:	.byte 1

.cseg
;----------------------------------------------------
Isr_Rtc:
	push	r16
	ldi		r16,0x00
	sts		rtc_intctrl,r16			; Disable RTC interruptions
	ldi		r16,0x01
	;sts		rtc_flag,r16			; Set user flag
	pop		r16
	reti

;----------------------------------------------------
Rtc_Reset:
	in		r16, cpu_sreg
	push	r16
	cli	
	rcall	RTC_Busy
	ldi		r16,0x00
	sts		rtc_cnt,r16
	sts		rtc_cnt+1,r16			; Reset counter
	ldi		r16,0x02
	sts		rtc_intflags,r16		; Cleart interruptions flags
	ldi		r16,0x0c
	sts		rtc_intctrl,r16			; High level interrupt
	;ldi		r16,0x00
	;sts		rtc_flag,r16			; Clear user flag
	pop		r16
	out		cpu_sreg,r16
	ret

;----------------------------------------------------
.cseg
Rtc_Config:
	in		r16,cpu_sreg
	push	r16
	cli	

	ldi		r16,0x00
	sts		rtc_intctrl,r16			; Disable compare and overflow interrupt

	ldi		r16,(0x02<1)|0x01	
;	ldi		r16,0x01	
	sts		clk_rtcctrl,r16			; Enable clock for RTC | Selecting 1024 Hz from 32.768 kHz internal oscillator
	
	rcall	Rtc_Busy
	ldi		r16,0x00				
	sts		rtc_ctrl,r16			; RTC stopped

	rcall	Rtc_Busy
	ldi		r16,0x00
	sts		rtc_cnt,r16
	sts		rtc_cnt+1,r16			; Reset counter

	rcall	Rtc_Busy
	sts		rtc_comp,xl
	sts		rtc_comp+1,xh			; Set compare value from X
	
	rcall	rtc_busy
	ldi		r16,0x07
	sts		rtc_ctrl,r16			; Prescaler= 1024

	ldi		r16,0x02
	sts		rtc_intflags,r16		; Clear compare flag
	ldi		r16,0x0c	
	sts		rtc_intctrl,r16			; High level interrupt Compare

	pop		r16
	out		cpu_sreg,r16
	ret

;----------------------------------------------------
Rtc_Busy:
	push	r16
Rtc_Busy_Loop:
	lds		r16,rtc_status
	sbrc	r16,0
	rjmp	Rtc_Busy_Loop
	pop		r16
	ret
