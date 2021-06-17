;----------------------------------------------------
; Sensors reading
;----------------------------------------------------	

.equ	InternalTemperatureMax = 55*16

.dseg
Serial_Number:			.byte 1							
Thermocouple_Table:		.byte 8
InternalTemp_Table:		.byte 8
Battery_Level:			.byte 1
Spread_Factor:			.byte 1

.cseg
Sensors_read:
	rcall	SerialNumber_Read
	rcall	Thermocouples_Read
	rcall	InternalTemperature_Check
	rcall	Battery_Read
	ret

;----------------------------------------------------
; Thermocouples reading
;----------------------------------------------------
.cseg
Thermocouples_Read: 	
	rcall	Thermocouples_Read_Enable				; Enable power supply and I/O pins for acquisition
	ldi		r16,220									; Thermocouple conversion power-up time (Min: 200 ms)
	rcall	DelayMs									
	
;	Discard first reading
	ldi		r16,0x10								; Reading thermocouple 1	
	sts		portd_outclr,r16
	rcall	Thermocouple_read						
	ldi		r16,0x10
	sts		portd_outset,r16

	ldi		r16,0x20								; Reading thermocouple 2	
	sts		portd_outclr,r16
	rcall	Thermocouple_read						
	ldi		r16,0x20
	sts		portd_outset,r16

	ldi		r16,0x40								; Reading thermocouple 3
	sts		portd_outclr,r16
	rcall	Thermocouple_read						
	ldi		r16,0x40
	sts		portd_outset,r16

	ldi		r16,0x02								; Reading thermocouple 4	
	sts		portr_outclr,r16
	rcall	Thermocouple_read						
	ldi		r16,0x02
	sts		portr_outset,r16

	ldi		r16,100									; Thermocouple conversion (Min: 100 ms)
	rcall	DelayMs									

;	Valid reading
	ldi		r16,0x10								; Reading temperature of thermocouple 1	
	sts		portd_outclr,r16
	rcall	Thermocouple_read						
	ldi		r16,0x10
	sts		portd_outset,r16
	sts		Thermocouple_Table,yh
	sts		Thermocouple_Table+1,yl
	sts		InternalTemp_Table,xh
	sts		InternalTemp_Table+1,xl					

	ldi		r16,0x20								; Reading temperature of thermocouple 2
	sts		portd_outclr,r16
	rcall	Thermocouple_read							
	ldi		r16,0x20
	sts		portd_outset,r16
	sts		Thermocouple_Table+2,yh
	sts		Thermocouple_Table+3,yl
	sts		InternalTemp_Table+2,xh
	sts		InternalTemp_Table+3,xl					

	ldi		r16,0x40								; Reading temperature of thermocouple 3
	sts		portd_outclr,r16
	rcall	Thermocouple_read						
	ldi		r16,0x40
	sts		portd_outset,r16
	sts		Thermocouple_Table+4,yh
	sts		Thermocouple_Table+5,yl
	sts		InternalTemp_Table+4,xh
	sts		InternalTemp_Table+5,xl					


	ldi		r16,0x02								; Reading temperature of thermocouple 4
	sts		portr_outclr,r16
	rcall	Thermocouple_read						
	ldi		r16,0x02
	sts		portr_outset,r16
	sts		Thermocouple_Table+6,yh
	sts		Thermocouple_Table+7,yl
	sts		InternalTemp_Table+6,xh
	sts		InternalTemp_Table+7,xl					

	rcall	Thermocouples_Read_Disable
	ret

;----------------------------------------------------
Thermocouples_Read_Disable:			
	ldi		r16,0x03
	sts		portr_outclr,r16						; Set low SS3 and SCK
	ldi		r16,0x74
	sts		portd_outclr,r16						; Disable power supply for thermocouples and set low CS0,CS1,CS2
	ret

;----------------------------------------------------
Thermocouples_Read_Enable:
	ldi		r16,0x74
	sts		portd_dirset,r16			
	ldi		r16,0x74
	sts		portd_outset,r16						; Enable power supply for thermocouples, CS0,CS1,CS2
	ldi		r16,0x03
	sts		portr_dirset,r16
	ldi		r16,0x02
	sts		portr_outset,r16						; Enable SS3 and SCK
	ldi		r16,0x80
	sts		portd_dirclr,r16
	ldi		r16,PORT_OPC_TOTEM_gc
	sts		portd_pin7ctrl,r16						; High-Z: MISO		
	ret

;----------------------------------------------------
; Read thermocouple and internal temperature from IC
; Input: None
; Output: x - Thermocouple temperature
;		  y	- Internal temperature

.cseg
Thermocouple_read:
	rcall	Spi_Thermocouple_Byte_Rx
	mov		yh,r16
	rcall	Spi_Thermocouple_Byte_Rx
	mov		yl,r16
	rcall	Spi_Thermocouple_Byte_Rx
	mov		xh,r16
	rcall	Spi_Thermocouple_Byte_Rx
	mov		xl,r16		
	ret

;----------------------------------------------------
; Spi simulated
; Input: none
; Output: r16

Spi_Thermocouple_Byte_Rx:
	ldi		r16,0x01								; Data byte
Spi_Thermocouple_Byte_Rx_Loop:
	ldi		r17,0x01
	sts		portr_outset,r17						; Sck High
	lsl		r16										; Rotate left
	lds		r18,portd_in							; Read Miso bit
	sbrc	r18,7									
	ori		r16,0x01					
	ldi		r17,0x01								; Sckl Low
	sts		portr_outclr,r17
	brcc	Spi_Thermocouple_Byte_Rx_Loop			; Repeated for eigth bits 
	ret

;----------------------------------------------------
InternalTemperature_Check:
	ldi		yh,high(InternalTemperatureMax)
	ldi		yh,low(InternalTemperatureMax)	
InternalTemperature_Check_FirstThemocouple:
	lds		xh,InternalTemp_Table
	lds		xl,InternalTemp_Table+1
	rcall	Thermocouples_Compare
	brne	InternalTemperature_Check_SecondThemocouple
	lds		xl,Thermocouple_Table+1
	ldi		r16,0x02
	or		xl,r16
	sts		Thermocouple_Table+1,xl
InternalTemperature_Check_SecondThemocouple:
	lds		xh,InternalTemp_Table+2
	lds		xl,InternalTemp_Table+3
	rcall	Thermocouples_Compare
	brne	InternalTemperature_Check_ThirdThemocouple
	lds		xl,Thermocouple_Table+3
	ldi		r16,0x02
	or		xl,r16
	sts		Thermocouple_Table+3,xl
InternalTemperature_Check_ThirdThemocouple:
	lds		xh,InternalTemp_Table+4
	lds		xl,InternalTemp_Table+5
	rcall	Thermocouples_Compare
	brne	InternalTemperature_Check_FourthThemocouple
	lds		xl,Thermocouple_Table+5
	ldi		r16,0x02
	or		xl,r16
	sts		Thermocouple_Table+5,xl
InternalTemperature_Check_FourthThemocouple:
	lds		xh,InternalTemp_Table+6
	lds		xl,InternalTemp_Table+7
	rcall	Thermocouples_Compare
	brne	InternalTemperature_Check_End
	lds		xl,Thermocouple_Table+7
	ldi		r16,0x02
	or		xl,r16
	sts		Thermocouple_Table+7,xl
InternalTemperature_Check_End:
	ret

;----------------------------------------------------
Thermocouples_Compare:
	ldi		r16,0xf0
	and		xl,r16
	cp		xl,yl
	cpc		xh,yh
	brsh	Thermocouples_Compare_Higher
	clz
	rjmp	Thermocouples_Compare_End
Thermocouples_Compare_Higher:
	sez
Thermocouples_Compare_End:
	ret

;----------------------------------------------------
; Battery reading
;----------------------------------------------------
.if Node==0
	.equ Battery_Low = 0xbb
.else
	.equ Battery_Low = 0xc1
.endif

.cseg
 Battery_Read:	
	ldi		r16,0x08
	sts		porta_outset,r16						; Enable power for battery measurement
	ldi		r17,ADC_REFSEL_INT1V_gc 
	ldi		r18,ADC_SAMPNUM_256X_gc
	rcall	Adc_On									; Configure Adc
	ldi		r16,3
	rcall	DelayMs									; Delay for stable signal
	ldi		r16, ADC_CH_MUXPOS_PIN2_gc
	rcall	Adc_Read								; Read battery
	lsl		r17
	rol		r16
	sts		Battery_Level,r16
	ldi		r16,0x08
	sts		porta_outclr,r16						; Disable power for battery measurement
	rcall	Adc_Off									; Turn off Adc
	lds		r16,Battery_Level
	ret		

;----------------------------------------------------
Battery_CheckIfCritical:
	cpi		r16,Battery_Low
	brsh	Battery_CheckIfCritical_End
	ldi		r16,0x02
	sts		porta_outset,r16						; Led High
	ldi		r16,50
	rcall	DelayMs									; 50 ms
	ldi		r16,0x02
	sts		porta_outclr,r16						; Led Low
	ldi		r16,100
	rcall	DelayMs									; 50 ms
	ldi		r16,0x02
	sts		porta_outset,r16						; Led High
	ldi		r16,50
	rcall	DelayMs									; 50 ms
	ldi		r16,0x02
	sts		porta_outclr,r16						; Led Low

	ldi		r16,0x20
	sts		porta_outclr,r16						; Disable Locking
Battery_CheckIfCritical_Loop:
	nop
	rjmp	Battery_CheckIfCritical_Loop
Battery_CheckIfCritical_End:
	ret

;----------------------------------------------------
; Serial number reading
;----------------------------------------------------
.dseg
Temporal:		.byte 2

.cseg
SerialNumber_Read:	
	ldi		r16,0x80
	sts		porta_outset,r16
	ldi		r17,ADC_REFSEL_INTVCC_gc
	ldi		r18,ADC_SAMPNUM_512X_gc
	rcall	Adc_on
	ldi		r16,3
	rcall	DelayMs	

	ldi		r16, ADC_CH_MUXPOS_PIN8_gc
	rcall	Adc_Read	
	sts		Temporal,r16
		
	ldi		r16, ADC_CH_MUXPOS_PIN9_gc
	rcall	Adc_Read	
	sts		Temporal+1,r16
	
	lds		r16,Temporal
	rcall	SerialNumber_Calculate
	push	r16
	lds		r16,Temporal+1
	rcall	SerialNumber_Calculate
	mov		r17,r16								; High value
	pop		r16									; Low value
	andi	r17,0x0f
	swap	r17
	or		r16,r17								; Serial number readed
	mov		r17,r16								; Save value
	andi	r16,0x1f							; Mask five bits
	sts		Serial_Number,r16
	andi	r17,0xe0
	lsr		r17	
	ldi		r16,0x50
	add		r17,r16
	sts		Spread_Factor,r17
	ret

SerialNumber_Table:
.db		0x63, 0x60, 0x5c, 0x58, 0x53, 0x4e, 0x49, 0x42
.db		0x3b, 0x34, 0x2c, 0x24, 0x1b, 0x10, 0x05, 0x00

SerialNumber_Calculate:
	ldi		r17,0
	ldi		zl,	low(SerialNumber_Table*2)
	ldi		zh,	high(SerialNumber_Table*2)
SerialNumber_Calculate_Loop:	
	inc		r17
	lpm		r18,z+	
	cp		r16,r18
	brlo	SerialNumber_Calculate_Loop
	dec		r17
	mov		r16,r17	
SerialNumber_End:
	ret