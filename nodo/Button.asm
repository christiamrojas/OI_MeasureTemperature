;----------------------------------------------------
; Button routines
;----------------------------------------------------
.equ	Button_TimeoutPressed_ds	= 50

;----------------------------------------------------
.dseg
Button_Flag:					.byte 1
Button_Tmr_TimeoutPressed_ds:	.byte 1

;----------------------------------------------------
.cseg
Button_DetectLongPressed:
	ldi		r16,Button_TimeoutPressed_ds
	sts		Button_Tmr_TimeoutPressed_ds,r16
Button_DetectLongPressed_Loop:
	Task_Change
	lds		r16,porta_in
	andi	r16,0x10
	cpi		r16,0
	breq	Button_DetectLongPressed_Ko
	lds		r16,Button_Tmr_TimeoutPressed_ds
	cpi		r16,0
	brne	Button_DetectLongPressed_Loop
	sez
	rjmp	Button_DetectLongPressed_End
Button_DetectLongPressed_Ko:
	clz
Button_DetectLongPressed_End:
	ret

;----------------------------------------------------
Button_Config:
	in		r16,cpu_sreg
	push	r16
	cli		

	ldi		r16,0x10				; Button (PA4): Input-Pulldown
	sts		porta_dirclr,r16
	ldi		r16,port_opc_pullDown_gc
	sts		porta_pin4ctrl,r16

	ldi		r16,0x10				
	sts		porta_intflags,r16		; Clear pin interruption flag
	sts		porta_intmask,r16		; Set interrupt pint

	ldi		r16,0x00
	sts		porta_intctrl,r16		; Disable interrupt

	pop		r16
	out		cpu_sreg,r16
	ret
;----------------------------------------------------
Button_Reset:
	in		r16,cpu_sreg
	push	r16
	cli	
	ldi		r16,0x00
	sts		Button_flag,r16				; Clear user flag
	ldi		r16,0x10
	sts		porta_intflags,r16			; Clear interrupt
	ldi		r16,0x01
	sts		porta_intctrl,r16			; Low level interrupt		
	pop		r16
	out		cpu_sreg,r16
	ret

;----------------------------------------------------
ISR_Porta:	
	push	r16
	ldi		r16,0x00
	sts		porta_intctrl,r16			; Disable interrupt
	ldi		r16,0x01
	sts		Button_flag,r16				; Set user flag
	pop		r16
	reti

;----------------------------------------------------


/*;----------------------------------------------------


;----------------------------------------------------
Button_DetectSequence:
	rcall	Button_Scan								; Expect button to be pressed
	brne	Button_DetectSequence_Ko
	ldi		r16,Button_Timeout_ds
	sts		BUtton_Tmr_Timeout_ds,r16
Button_DetectSequence_WaitRelease1:					; Wait button to be released
	rcall	Button_Scan
	brne	Button_DetectSequence_WaitPress1
	lds		r16,BUtton_Tmr_Timeout_ds
	cpi		r16,0
	breq	Button_DetectSequence_Ko
	rjmp	Button_DetectSequence_WaitRelease1
Button_DetectSequence_WaitPress1:					; Wait button to be pressed
	ldi		r16,Button_Timeout_ds
	sts		Button_Tmr_Timeout_ds,r16
Button_DetectSequence_WaitPress1_Loop:
	rcall	Button_scan
	breq	Button_DetectSequence_WaitRelease2
	lds		r16,Button_Tmr_Timeout_ds
	cpi		r16,0
	breq	Button_DetectSequence_Ko
	rjmp	Button_DetectSequence_WaitPress1_Loop
Button_DetectSequence_WaitRelease2:					; Wait to be released
	ldi		r16,Button_Timeout_ds
	sts		Button_Tmr_Timeout_ds,r16
Button_DetectSequence_WaitRelease2_Loop:
	rcall	Button_scan
	brne	Button_DetectSequence_Ok
	lds		r16,BUtton_Tmr_Timeout_ds
	cpi		r16,0
	breq	Button_DetectSequence_Ko	
Button_DetectSequence_Ok:
	sez
	rjmp	Button_DetectSequence_End
Button_DetectSequence_Ko:
	clz
Button_DetectSequence_End:
	ret

;----------------------------------------------------
.dseg
Button_Ant:		.byte 1

.cseg
Button_Scan:
	ldi		r16,Button_Debounce_cs
	sts		Button_Tmr_Deb_cs,r16
Button_Scan_Loop:
	Task_Change
	lds		r16,porta_in
	andi	r16,0x10
	lds		r17,Button_Ant
	sts		BUtton_Ant,r16
	cp		r16,r17
	brne	Button_Scan
	lds		r16,Button_Tmr_Deb_cs
	cpi		r16,0
	brne	Button_Scan_Loop
	cpi		r17,0
	ret
	*/
