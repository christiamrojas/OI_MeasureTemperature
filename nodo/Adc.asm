
;----------------------------------------------------
; Adc subroutines
;----------------------------------------------------
Adc_Read:		
	andi	r16,adc_ch_muxpos_gm			; Masking positive input	
	ori		r16,adc_ch_muxnegl_INTGND_gc	; Masking negative input
	sts		adca_ch0_muxctrl,r16			; Select input
	ldi		r16,adc_ch0if_bm
	sts		adca_ch0_intflags,r16			; Clear flag	
	ldi		r16,(ADC_CH_START_bm | ADC_CH_INPUTMODE_diffwgainl_gc)
	sts		adca_ch0_ctrl, r16				; Start conversion
Adc_Read_Loop:
	Task_Change
	lds		r16,adca_intflags
	sbrs	r16,0
	rjmp	Adc_Read_Loop					; Wait to finish conversion
	lds		r17,adca_ch0_res				
	lds		r16,adca_ch0_res+1				; Read value
	sbrc	r16,7
	clr		r16
	ret

;----------------------------------------------------
Adc_On:	
	ldi		r16,0x00
	sts		adca_ctrla,r16			; Turn off ADC
	ldi		r16,(ADC_CURRLIMIT_HIGH_gc|ADC_RESOLUTION_MT12BIT_gc|ADC_CONMODE_bm)
	sts		adca_ctrlb,r16			; High current limit | 12 bit right adjusted (Oversampling) | signed mode	
	sts		adca_refctrl,r17		; External reference from AREF (3V3)
	ldi		r16,ADC_PRESCALER_DIV16_gc
	sts		adca_prescaler,r16		; Prescaler = 2MHz/16 = 125 KHz
	ldi		r16,0
	sts		adca_sampctrl,r16		; Sampling time = (sampval + 1) x ClkAdc/2

	ldi		R16,ADC_CH_INPUTMODE_DIFFWGAINL_gc
	sts		adca_ch0_ctrl, R16		; Differential input | Gain=1	
	sts		adca_ch0_avgctrl,r18	; Average samples (512) |  right shift (0)

	ldi		r16,(ADC_FLUSH_bm | ADC_ENABLE_bm)
	sts		adca_ctrla,R16			; Flush and enable ADC	
	ret	

;----------------------------------------------------
Adc_Off:
	ldi		r16,0x00
	sts		adca_ctrla,r16
	ret
