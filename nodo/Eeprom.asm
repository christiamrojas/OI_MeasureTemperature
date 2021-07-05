;----------------------------------------------------
; Eeprom
;----------------------------------------------------
.equ EEdir_Time			= 0x1000
.equ EEdir_BatCritic	= 0x1002
.equ EEdir_Batmin		= 0x1003
.equ EEdir_Batmax		= 0x1004

;----------------------------------------------------
Eeprom_ReadSamplingTime:
	ldi		xh,high(EEdir_Time)
	ldi		xl,low(EEdir_Time)
	rcall	Eeprom_Read				
	mov		r17,r16
	rcall	Eeprom_Read
	mov		xl,r17
	mov		xh,r16
	ret

;----------------------------------------------------
Eeprom_WriteSamplingTime:
	mov		yl,xl
	mov		yh,xh
	ldi		xh,high(EEdir_Time)
	ldi		xl,low(EEdir_Time)
	mov		r16,yl
	rcall	Eeprom_Write
	mov		r16,yh
	rcall	Eeprom_Write
	ret

;----------------------------------------------------
; Input:	X
; Output:	r16
Eeprom_Read:
	ld	r16,x+
	ret

;----------------------------------------------------
; Input:	(X) <-- r16
Eeprom_Write:
	ldi		r17,0x35
	sts		nvm_cmd,r17
	clr		r17
	st		x,r16
	sts		nvm_addr0,xl
	sts		nvm_addr1,xh
	sts		nvm_addr2,r17
	sts		nvm_data0,r16
	sts		nvm_data1,r17
	sts		nvm_data2,r17
	cli
	ldi		r16,0xd8
	sts		cpu_ccp,r16
	ldi		r16,1
	sts		nvm_ctrla,r16
	sei
EE_Write_Loop:
	nop
	lds		r16,nvm_status
	sbrc	r16,7
	rjmp	EE_Write_Loop
	adiw	xl,1	
	ret

