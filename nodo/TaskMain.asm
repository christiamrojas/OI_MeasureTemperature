;----------------------------------------------------
;----------------------------------------------------
; Temperature node
;----------------------------------------------------
;----------------------------------------------------
; V1.1.5	01/05/21 1:09 pm
;	Get spread factor from dip switch

;----------------------------------------------------
; Main Definitions
;----------------------------------------------------
.equ Main_Frequency = 2000000
.equ NOde = 1

;----------------------------------------------------
; Macros
;----------------------------------------------------
.macro TurnOnLed
	ldi		r16,0x02
	sts		porta_outset,r16
.endmacro

.macro TurnOffLed
	ldi		r16,0x02
	sts		porta_outclr,r16
.endmacro

;----------------------------------------------------
; Vector interruptions
;----------------------------------------------------
.cseg 
.org 0x00					; (0) Reset
	rjmp	MultiTask_Init		

.org RTC_COMP_vect			; (16) RTC compare interrupt
	rjmp	ISR_RTC	

.org Porta_int_vect			; (60) External interrupt
	rjmp	ISR_PortA

.org TCD5_OVF_vect			; (72) Timer D5 overflow interrupt
	rjmp	ISR_TCD5_OVF


;----------------------------------------------------
; Libraries
;----------------------------------------------------
.include "MultiTask.asm"
.include "Adc.asm"
.include "Eeprom.asm"
.include "SpiL.asm"
.include "Lora.asm"
.include "LoraReg.asm"
.include "Rtc.asm"
.include "Sensors.asm"
.include "TaskTimer.asm"
.include "Button.asm"

;----------------------------------------------------
; Variables
;----------------------------------------------------
.dseg
Tmr_ms:	.byte 1

;----------------------------------------------------
; Main program
;----------------------------------------------------
.cseg
TaskMain:		
	;rjmp   Read_BatMin	
	;rjmp   Read_BatCritic	
	rcall	Battery_Read
	rcall	Battery_CheckIfCritical					; If battery low turn off device
	rcall	Eeprom_ReadSamplingTime					; Read sampling time
	rcall	Rtc_Config								; Configure timer to wake up from sleep
	rcall	Button_Config							; Configure button to wake up from sleep
TaskMain_Loop:	
	TurnOnLed
	rcall	Sensors_Read							; Read serial number, battery level and thermocouples
	rcall	Lora_Transmit							; Transmist packet through Lora
	TurnOffLed
	rcall	LoPower									; To sleep
	rcall	Battery_Read
	rcall	Battery_CheckIfCritical					; If battery low turn off device	
	lds		r16,Button_Flag
	cpi		r16,1									; If wake up is due to button wait for long pressed
	brne	TaskMain_Loop
	rcall	Button_DetectLongPressed
	brne	TaskMain_Loop
	rcall	Lora_Reception							; Receive Lora Message
	rjmp	TaskMain_Loop

;----------------------------------------------------
LoPower:
	cli	
	rcall	RTC_Reset								; Clear RTC variables
	rcall	Button_reset							; Clear button variables
	ldi		r16,0x07
	sts		sleep_ctrl,r16							; Power save mode & sleep
	sei
	sleep
	ret

;----------------------------------------------------
DelayMs:
	sts Tmr_ms,r16
DelayMs_Loop:
	Task_Change
	lds		r16,Tmr_ms
	cpi		r16,0
	brne	DelayMs_Loop
	ret

;----------------------------------------------------
Read_BatMin:
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
	ldi		xh,high(EEdir_BatMin)
	ldi		xl,low(EEdir_BatMin)	
	rcall	Eeprom_Write
	ldi		r16,0x08
	sts		porta_outclr,r16						; Disable power for battery measurement
	rcall	Adc_Off									; Turn off Adc		
Read_BatMin_End:
	rjmp	Read_BatMin_End

;----------------------------------------------------
Read_BatCritic:
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
	ldi		xh,high(EEdir_BatCritic)
	ldi		xl,low(EEdir_BatCritic)	
	rcall	Eeprom_Write
	ldi		r16,0x08
	sts		porta_outclr,r16						; Disable power for battery measurement
	rcall	Adc_Off									; Turn off Adc		
Read_BatCritic_End:
	rjmp	Read_BatCritic_End