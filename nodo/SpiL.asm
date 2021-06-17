;----------------------------------------------------
; Spi for Lora subroutines
;----------------------------------------------------

; Input: r16
; Output: r16
SpiL_Byte_TxRx:
	sts		spic_data,r16
SpiL_Byte_TxRx_Loop:
	lds		r16,spic_status
	sbrs	r16,7
	rjmp	SpiL_Byte_TxRx_Loop
	lds		r16,spic_data
	ret

;----------------------------------------------------
SpiL_Init:
	ldi		r16,0x00					; Turn off SPI
	sts		spic_ctrl,r16
	sts		spic_ctrlb,r16				; Unbuffer mode
	ldi		r16,SPI_IF_bm
	sts		spic_status,r16				; Clear interrupt flag
	ldi		r16,SPI_MASTER_bm|SPI_ENABLE_bm|SPI_PRESCALER0_bm		
	sts		spic_ctrl,r16				; Enable | master mode | Clock/4
	ret

;----------------------------------------------------
SpiL_DeInit:
	ldi		r16,0x00
	sts		spic_ctrl,r16				; Turn off SPI
	ldi		r16,0x00
	sts		portc_out,r16				; Set Low CS, RST, Power, SCK, MOSI
	ldi		r16,port_opc_pulldown_gc
	sts		portc_pin6ctrl,r16			; Set Pull-down Miso		
	ret