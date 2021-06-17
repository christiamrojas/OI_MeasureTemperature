;----------------------------------------------------
; Lora registers
;----------------------------------------------------

.equ reg_fifo					=	0x00
.equ reg_op_mode				=	0x01
.equ REG_FRF_MSB				=	0x06
.equ REG_FRF_MID				=	0x07
.equ REG_FRF_LSB				=	0x08
.equ REG_PA_CONFIG				=	0x09
.equ REG_OCP					=	0x0b
.equ REG_LNA					=	0x0c
.equ REG_FIFO_ADDR_PTR			=	0x0d
.equ REG_FIFO_TX_BASE_ADDR		=	0x0e
.equ REG_FIFO_RX_BASE_ADDR		=	0x0f
.equ REG_FIFO_RX_CURRENT_ADDR	=	0x10
.equ REG_IRQ_FLAGS				=	0x12
.equ REG_RX_NB_BYTES			=	0x13
.equ REG_PKT_SNR_VALUE			=	0x19
.equ REG_PKT_RSSI_VALUE			=	0x1a
.equ REG_RSSI_VALUE				=	0x1b
.equ REG_MODEM_CONFIG_1			=	0x1d
.equ REG_MODEM_CONFIG_2			=	0x1e
.equ REG_SYMB_TIMEOUT_LSB		=	0x1f
.equ REG_PREAMBLE_MSB			=	0x20
.equ REG_PREAMBLE_LSB			=	0x21
.equ REG_PAYLOAD_LENGTH			=	0x22
.equ REG_MODEM_CONFIG_3			=	0x26
.equ REG_FREQ_ERROR_MSB			=	0x28
.equ REG_FREQ_ERROR_MID			=	0x29
.equ REG_FREQ_ERROR_LSB			=	0x2a
.equ REG_RSSI_WIDEBAND			=	0x2c
.equ REG_DETECTION_OPTIMIZE		=	0x31
.equ REG_INVERTIQ				=	0x33
.equ REG_DETECTION_THRESHOLD	=	0x37
.equ REG_SYNC_WORD				=	0x39
.equ REG_INVERTIQ2				=	0x3b
.equ REG_DIO_MAPPING_1			=	0x40
.equ REG_VERSION				=	0x42
.equ REG_PA_DAC					=	0x4d

;----------------------------------------------------
; Lora modes
;----------------------------------------------------
.equ Mode_LoRa					=	0x80
.equ Mode_Sleep					=	0x00
.equ Mode_StandBy               =	0x01
.equ Mode_Tx					=	0x03
.equ Mode_Rx_Continuous			=	0x05
.equ Mode_Rx_Single				=	0x06

.equ HeaderImplicit				=	0x01
.equ HeaderExplicit				=	0x00


; IRQ masks
.equ Irq_TxDone					=	0x08
.equ IRQ_PayloadCRCError		=	0x20
.equ IRQ_RXDone					=	0x40

;----------------------------------------------------
; Lora settings calculated
;----------------------------------------------------
.if Lora_SpreadFactor == 6
	.equ DetectionOptimize	= 0xc5
	.equ DetectionThreshold = 0x0c
.else
	.equ DetectionOptimize	= 0xc3
	.equ DetectionThreshold = 0x0a
.endif

.equ Lora_SymbolDuration	=	10000/(Lora_BandWidth/(1<<Lora_SpreadFactor))

.if Lora_SymbolDuration >16
	.equ Lora_LowDROptimize = 0x08
.else
	.equ Lora_LowDROptimize = 0x00
.endif

.if Lora_BandWidth <= 7800
	.equ Lora_BW = 0<<4
.elif Lora_BandWidth <= 10400
	.equ Lora_BW = 1<<4
.elif Lora_BandWidth <= 15600
	.equ Lora_BW = 2<<4
.elif Lora_BandWidth <= 20800
	.equ Lora_BW = 3<<4
.elif Lora_BandWidth <= 31250
	.equ Lora_BW = 4<<4
.elif Lora_BandWidth <= 41700
	.equ Lora_BW = 5<<4
.elif Lora_BandWidth <= 62500
	.equ Lora_BW = 6<<4
.elif Lora_BandWidth <= 125000
	.equ Lora_BW = 7<<4
.elif Lora_BandWidth <= 250000
	.equ Lora_BW = 8<<4
.else 
	.equ Lora_BW = 9<<4
.endif

.if Lora_CodeRate<5
	.equ Lora_CR = 1<<1
.elif Lora_CodeRate > 8
	.equ Lora_CR = 4<<1
.else
	.equ Lora_CR = (Lora_CodeRate-4)<<1
.endif

.if Lora_TxPower>17
	.equ level = Lora_TxPower-5
	.equ dac = 0x87
	.equ ocpTrim = 27
.else	
	.equ level = Lora_TxPower-2
	.equ dac = 0x84
	.equ ocpTrim = 11
.endif

.equ frf	=	((Lora_Frequency<<19))/32000000