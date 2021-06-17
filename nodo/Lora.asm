;----------------------------------------------------
; Lora Settings
;----------------------------------------------------

.equ Lora_BandWidth			=	125000
.equ Lora_SpreadFactor		=	12
.equ Lora_Frequency			=	915000000		
.equ Lora_SyncWord			=	0x12				
.equ Lora_CodeRate			=	5
.equ Lora_PreambleLength	=	8				
.equ Lora_CRCEnable			=	1				; [0:1]
.equ Lora_TxPower			=	14				; [14:20] dBm

;----------------------------------------------------
; Lora Transmission
;----------------------------------------------------

.cseg
LoRa_Transmit:
	rcall	Lora_EnableHardware
	rcall	Lora_Config
	rcall	Lora_Tx
	rcall	Lora_DisableHardware
	ret

;----------------------------------------------------
; Lora Reception
;----------------------------------------------------
.equ Lora_RxTimeout_s = 60							; Timeout Lora in reception 

.dseg
Lora_Tmr_RxTimeout_s:		.byte 1

.cseg
Lora_Reception:
	TurnOnLed
	rcall	Lora_EnableHardware						; Turn on HW and init low level HW
	rcall	Lora_Config								; Set Lora standard configuration
	rcall	Lora_RxSet								; Set Lora Reception configuration
	ldi		r16,Lora_RxTimeout_s					; Set Lora Timeout
	sts		Lora_Tmr_RxTimeout_s,r16
Lora_Reception_Loop:
	Task_Change
	lds		r16,Lora_Tmr_RxTimeout_s
	cpi		r16,0
	breq	Lora_Reception_End						; Evaluate timeout
	ldi		r16,100
	rcall	DelayMs									; Delay
	rcall	Lora_RxParsePacket						; There is a packet received?
	brne	Lora_Reception_Loop
	rcall	Lora_RxRead								; Read bytes received
	rcall	Lora_RxProcess
	
Lora_Reception_End:
	rcall	Lora_DisableHardware					; Turn off HW and deinit low level HW
	TurnOffLed
	ret

;----------------------------------------------------
Lora_RxProcess:
	lds		r16,Lora_RxLen
	cpi		r16,3
	brne	Lora_RxProcess_End
	lds		r16,Lora_RxData
	lds		r17,Serial_number
	cp		r16,r17
	brne	Lora_RxProcess_End

	lds		xl,Lora_RxData+1
	lds		xh,Lora_RxData+2
	rcall	Eeprom_WriteSamplingTime

	TurnOffLed
	ldi		r16,200
	rcall	DelayMs
	TurnOnLed
	ldi		r16,200
	rcall	DelayMs
	TurnOffLed
	ldi		r16,200
	rcall	DelayMs
	TurnOnLed
	ldi		r16,200
	rcall	DelayMs
	TurnOffLed
	ldi		r16,200
	rcall	DelayMs
	TurnOnLed
	ldi		r16,200
	rcall	DelayMs
	TurnOffLed

	rcall	Eeprom_ReadSamplingTime					; Read sampling time
	rcall	Rtc_Config								; Configure timer to wake up from sleep
	rcall	Button_Config							; Configure button to wake up from sleep

Lora_RxProcess_End:
	ret


;----------------------------------------------------
.dseg
Lora_RxIrq:		.byte 1
Lora_RxIdx:		.byte 1
Lora_RxLen:		.byte 1
Lora_RxData:	.byte 64

.cseg
Lora_RxRead:
	ldi		r16,REG_RX_NB_BYTES
	ldi		r17,0
	rcall	LOra_RReg
	lds		r17,Lora_RxIdx
	cp		r16,r17
	breq	Lora_RxRead_End

	ldi		r16,REG_FIFO
	ldi		r17,0
	rcall	LOra_RReg

	ldi		xl,low(Lora_RxData)
	ldi		xh,high(Lora_RxData)
	lds		r17,Lora_RxIdx
	add		xl,r17
	ldi		r17,0
	adc		xh,r17
	st		x,r16

	lds		r17,Lora_RxIdx
	inc		r17
	sts		Lora_RxIdx,r17
	rjmp	Lora_RxRead
Lora_RxRead_End:
	ret

;----------------------------------------------------
Lora_RxParsePacket:
	ldi		r16,REG_IRQ_FLAGS					; Read IRQ Register
	ldi		r17,0
	rcall	Lora_RReg
	sts		Lora_RxIrq,r16

	mov		r17,r16								; Clear IRQ Flags
	ldi		r16,Reg_Irq_Flags
	rcall	Lora_WReg

	lds		r16,Lora_RxIrq						; If()
	andi	r16,0x60
	cpi		r16,0x40
	brne	Lora_Rx_Ko							

	ldi		r16,0								; Packet index = 0
	sts		Lora_RxIdx,r16				

	ldi		r16,REG_RX_NB_BYTES					; Read packet length
	ldi		r17,0
	rcall	Lora_RReg
	sts		Lora_RxLen,r16
	

	ldi		r16,REG_FIFO_RX_CURRENT_ADDR		; Set fifo Address to current Rx Address
	ldi		r17,0
	rcall	Lora_RReg
	mov		r17,r16
	ldi		r16,REG_FIFO_ADDR_PTR
	rcall	Lora_WReg

	ldi		r16,REG_OP_MODE						; To idle
	ldi		r17,Mode_Lora|Mode_standby
	rcall	Lora_WReg

	sez
	rjmp	Lora_Rx_End
Lora_Rx_Ko:
	clz
Lora_Rx_End:	
	ret

;----------------------------------------------------
Lora_RxSet:
	ldi		r16,REG_DIO_MAPPING_1
	ldi		r17,0x00
	rcall	Lora_WReg

	ldi		r16,REG_OP_MODE
	ldi		r17,MODE_LORA | MODE_RX_CONTINUOUS
	rcall	Lora_WReg

	ldi		r16,100
	rcall	DelayMs
	ret

;----------------------------------------------------
Lora_Tx:
;	Begin Packet
	ldi		r16,Reg_Op_Mode			
	ldi		r17,0
	rcall	Lora_RReg				; Wait to finish any transmission
	andi	r16,0x07
	cpi		r16,0x03
	breq	Lora_Tx

	ldi		r16,Reg_Irq_Flags
	ldi		r17,Irq_TxDone
	rcall	Lora_WReg

	ldi		r16,Reg_Op_Mode			; Idle
	ldi		r17,Mode_LoRa|Mode_StandBy
	rcall	Lora_WReg						

	ldi		r16,Reg_Fifo_Addr_Ptr	; Clear Tx Fifo
	ldi		r17,0
	rcall	Lora_WReg
	ldi		r16,Reg_Payload_Length
	ldi		r17,0
	rcall	Lora_WReg
    
;	Write message	
	ldi		r16,Reg_Fifo
	lds		r17,Serial_Number
	rcall	Lora_WReg
	ldi		r16,Reg_Fifo
	lds		r17,Thermocouple_Table
	rcall	Lora_WReg
	ldi		r16,Reg_Fifo
	lds		r17,Thermocouple_Table+1
	rcall	Lora_WReg
	ldi		r16,Reg_Fifo
	lds		r17,Thermocouple_Table+2
	rcall	Lora_WReg
	ldi		r16,Reg_Fifo
	lds		r17,Thermocouple_Table+3
	rcall	Lora_WReg
	ldi		r16,Reg_Fifo
	lds		r17,Thermocouple_Table+4
	rcall	Lora_WReg
	ldi		r16,Reg_Fifo
	lds		r17,Thermocouple_Table+5
	rcall	Lora_WReg
	ldi		r16,Reg_Fifo
	lds		r17,Thermocouple_Table+6	
	rcall	Lora_WReg
	ldi		r16,Reg_Fifo
	lds		r17,Thermocouple_Table+7
	rcall	Lora_WReg
	ldi		r16,Reg_Fifo
	lds		r17,Battery_Level
	rcall	Lora_WReg

	ldi		r16,Reg_payload_Length
	ldi		r17,10
	rcall	Lora_WReg
	
;	End packet
	ldi		r16,Reg_Op_Mode
	ldi		r17,Mode_Lora|Mode_Tx
	rcall	Lora_WReg	
Lora_tx_Loop:
	ldi		r16,Reg_Irq_Flags
	ldi		r17,0
	rcall	Lora_RReg
	sbrs	r16,3	
	rjmp	Lora_tx_Loop
	
	ldi		r16,Reg_Irq_Flags
	ldi		r17,Irq_TxDone
	rcall	Lora_WReg
		
	ret
;----------------------------------------------------
Lora_Config:
	ldi		r16,Reg_op_mode					; Sleep
	ldi		r17,mode_LoRa|mode_sleep
	rcall	Lora_WReg						
	
	ldi		r16,Reg_frf_msb					; Set frequency
	ldi		r17,(frf>>16) & 0xff
	rcall	Lora_WReg
	ldi		r16,Reg_frf_mid
	ldi		r17,(frf>>8)  & 0xff
	rcall	Lora_WReg
	ldi		r16,Reg_frf_lsb
	ldi		r17,(frf>>0)  & 0xff
	rcall	Lora_WReg						
		
	ldi		r16,Reg_fifo_tx_base_addr		; Set base address
	ldi		r17,0x00
	rcall	Lora_WReg
	ldi		r16,Reg_fifo_rx_base_addr
	ldi		r17,0x00
	rcall	Lora_WReg

	ldi		r16,Reg_lna						; Set Lna configuration (Max Gain & Boost on)
	ldi		r17,0x23
	rcall	Lora_Wreg

	ldi		r16,Reg_Modem_Config_3			; Agc Auto On
	ldi		r17,0x04
	rcall	Lora_Wreg

	ldi		r16,Reg_Pa_Dac					; Set dBm on Pa_Boost
	ldi		r17,dac
	rcall	Lora_WReg
	ldi		r16,Reg_Ocp						; OCP enable
	ldi		r17,0x20|(0x1f&ocpTrim)
	rcall	Lora_WReg							
	ldi		r16,Reg_Pa_Config				; PA_Boost
	ldi		r17,0x80|level
	rcall	Lora_WReg
	
	ldi		r16,Reg_Op_Mode					; Idle
	ldi		r17,Mode_LoRa|Mode_StandBy
	rcall	Lora_WReg
	
	ldi		r16,Reg_Sync_Word				; Set sync word
	ldi		r17,Lora_SyncWord
	rcall	Lora_WReg
		
	ldi		r16,Reg_Detection_Optimize				
	ldi		r17,DetectionOptimize
	rcall	Lora_WReg
	ldi		r16,Reg_Detection_Threshold				
	ldi		r17,DetectionThreshold
	rcall	Lora_WReg
	ldi		r16,Reg_Modem_Config_2			; Set spreading factor & CRC enable	
	lds		r18, Spread_Factor
	ldi		r17,(Lora_CRCEnable<<2)		
	or		r17,r18
	rcall	Lora_WReg
	ldi		r16,Reg_Modem_Config_3				
	ldi		r17,Lora_LowDROptimize|0x04				
	rcall	Lora_WReg

	ldi		r16,Reg_Modem_Config_1			; Set bandwidth & CodingRate & ImplicitMode
	ldi		r17,Lora_BW|Lora_CR|HeaderExplicit		
	rcall	Lora_WReg
	
	ldi		r16,Reg_Preamble_Msb			; Preamble length
	ldi		r17,(Lora_PreambleLength>>8) & 0xff				
	rcall	Lora_WReg
	ldi		r16,Reg_Preamble_Lsb
	ldi		r17,(Lora_PreambleLength>>0) & 0xff				
	rcall	Lora_WReg

	ret

;----------------------------------------------------
Lora_WReg:
	ldi		r18,0x01
	sts		portc_outclr,r18		; CS Low
	ori		r16,0x80		
	rcall	SpiL_Byte_TxRx			; Address
	mov		r16,r17
	rcall	SpiL_Byte_TxRx			; Data
	ldi		r18,0x01
	sts		portc_outSet,r18		; CS High
	ret

;----------------------------------------------------
Lora_RReg:
	ldi		r18,0x01
	sts		portc_outclr,r18		; CS Low
	andi	r16,0x7f
	rcall	SpiL_Byte_TxRx			; Address
	mov		r16,r17	
	rcall	SpiL_Byte_TxRx			; Data
	ldi		r18,0x01
	sts		portc_outSet,r18		; CS High
	ret

;----------------------------------------------------
Lora_EnableHardware:	
	ldi		r16,0xb3				;PC7: MOSI		PC5: SCK		PC4:PWR		PC1: RST		PC0: CS		
	sts		portc_dir,r16			;PC2: DIO0		PC3: DIO0		PC6: MISO
	ldi		r16,0x13		
	sts		portc_out,r16	
	ldi		r16,port_opc_totem_gc
	sts		portc_pin6ctrl,r16
	rcall	SpiL_Init				; Configure SPI	
	ldi		r16,10
	rcall	DelayMs					; Delay 10 ms
	ldi		r16,0x02
	sts		portc_outclr,r16		; RST=Low
	ldi		r16,10
	rcall	DelayMs					; Delay 10 ms
	ldi		r16,0x02
	sts		portc_outset,r16		; RST=High
	ldi		r16,10
	rcall	DelayMs					; Delay 10 ms
	ret

;----------------------------------------------------
Lora_DisableHardware:	
	rcall	SpiL_DeInit
	ldi		r16,0x10
	sts		portc_outclr,r16	
	ret

