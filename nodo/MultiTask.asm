/*
 * MultiTask.asm
 *
 *  Created: 14/04/2021 17:25:55
 *   Author: CHRISTIAM
 */ 

 ;---------------------------------------------------- 
 .equ	multitask_TASK_STACK_SIZE = 128

;----------------------------------------------------
.cseg
Multitask_task_table:	
	.dw		TaskMain	
	.dw		TaskTimer			
Multitask_task_table_end:

.equ	multitask_TASKS_COUNT = (multitask_task_table_end-multitask_task_table)

.dseg
Multitask_curr_task:		.byte 1
Multitask_sp_table:			.byte 2*multitask_TASKS_COUNT							; LO-HI
Multitask_stack_pool:		.byte multitask_TASK_STACK_SIZE*multitask_TASKS_COUNT
Multitask_stack_pool_end:

;----------------------------------------------------
.cseg
Multitask_Init:
	cli												;disable global interrupts
	ldi		R16,high(RAMEND)						;initialize stack
	out		CPU_SPH,R16
	ldi		R16,low(RAMEND)
	out		CPU_SPL,R16
			
	rcall	Multitask_Ports_Init					;initialize ports	

	ldi		ZH,high(Multitask_task_table*2)
	ldi		ZL,low(Multitask_task_table*2)
	ldi		XH,high(multitask_sp_table)
	ldi		XL,low(multitask_sp_table)
	ldi		YH,high(multitask_stack_pool+multitask_TASK_STACK_SIZE)
	ldi		YL,low(multitask_stack_pool+multitask_TASK_STACK_SIZE)
	ldi		R18,multitask_TASKS_COUNT
multitask_init_loop:
	lpm		R16,Z+									; Task start address
	lpm		R17,Z+
	st		-Y,R16									; Push task address to stack
	st		-Y,R17
	sbiw	YL,1
	st		X+,YL									; Store task sp to table
	st		X+,YH
	ldi		R16,low(multitask_TASK_STACK_SIZE+3)	; Next stack
	add		YL,R16
	ldi		R16,high(multitask_TASK_STACK_SIZE+3)
	adc		YH,R16
	dec		R18										; Next task
	brne	multitask_init_loop

	ldi		R16,0									; Start on first task
	sts		multitask_curr_task,R16
	lds		R16,multitask_sp_table					; SPL
	lds		R17,multitask_sp_table+1				; SPH
	out		CPU_SPH,R17
	out		CPU_SPL,R16

	ldi		r16,0x07								; Enable high-Mid-low level interruptions
	sts		PMIC_ctrl,r16		
	sei												; Enable global interrupts
	ret




Multitask_Ports_init:
;----------------------------------------------------
;	PA0:	(High-Z)	Adc-Ref
;	PA1:	(Out-Low)	Rf-Led
;	PA2:	(High-Z)	Adc-Bat
;	PA3:	(Out-Low)	Adc-BatEn
;	PA4:	(Pull-Down)	Button
;	PA5:	(Out-High)	Locking
;	PA6:	(Pull-Down)	NC
;	PA7:	(Out-Low)	SN-En

	ldi		r16,0xaa
	sts		porta_dir,r16
	ldi		r16,0x20
	sts		porta_out,r16					; Enable locking
	ldi		r16,Port_Opc_Totem_gc
	sts		porta_pin0ctrl,r16	
	sts		PORTA_PIN1CTRL,R16
	sts		porta_pin2ctrl,r16
	sts		porta_pin3ctrl,r16
	sts		PORTA_PIN4CTRL,R16
	sts		PORTA_PIN5CTRL,R16

	ldi		r16,port_opc_wiredor_gc|port_opc_wiredorpull_gc
	sts		PORTA_PIN7CTRL,R16

	ldi		r16,PORT_OPC_PULLDOWN_gc		
	sts		PORTA_PIN6CTRL,R16
		
;----------------------------------------------------
;	PC0:	(Out-Low)	Rf-Cs
;	PC1:	(Out-Low)	Rf-Rst
;	PC2:	(High-Z)	Rf-Dio0
;	PC3:	(High-Z)	Rf-Dio1
;	PC4:	(Out-Low)	Rf-PwrEn
;	PC5:	(Out-Low)	Rf-Sck
;	PC6:	(High-Z)	Rf-Miso
;	PC7:	(Out-Low)	RF-Mosi

	ldi		r16,0xb3
	sts		portc_dir,r16
	ldi		r16,0x00
	sts		portc_out,r16
	ldi		r16,PORT_OPC_totem_gc
	sts		PORTC_PIN0CTRL,r16
	sts		PORTC_PIN1CTRL,r16
	sts		PORTC_PIN2CTRL,r16
	sts		PORTC_PIN3CTRL,r16	
	sts		PORTC_PIN4CTRL,r16	
	sts		PORTC_PIN5CTRL,r16	
	sts		PORTC_PIN6CTRL,r16
	sts		PORTC_PIN7CTRL,r16	
		
;----------------------------------------------------
;	PD0:	(High-Z)	SN-Low
;	PD1:	(High-Z)	SN-High
;	PD2:	(Out-Low)	Temp-PwrEn
;	PD3:	(Out-Low)	Dbg-Tx
;	PD4:	(Out-Low)	Temp-SS0
;	PD5:	(Out-Low)	Temp-SS1
;	PD6:	(Out-Low)	Temp-SS2
;	PD7:	(High-Z)	Temp-Miso

	ldi		r16,0x7c
	sts		portd_dir,r16
	ldi		r16,0x00
	sts		portd_out,r16
	ldi		r16,PORT_OPC_Totem_gc	
	sts		PORTD_PIN0CTRL,r16
	sts		PORTD_PIN1CTRL,r16
	sts		PORTD_PIN2CTRL,r16
	sts		PORTD_PIN3CTRL,r16
	sts		PORTD_PIN4CTRL,r16
	sts		PORTD_PIN5CTRL,r16
	sts		PORTD_PIN6CTRL,r16
	sts		PORTD_PIN7CTRL,r16
	

;----------------------------------------------------
;	PR0:	(Out-Low)	Temp-Sck
;	PR1:	(Out-Low)	Temp-SS3
	
	ldi		r16,0x03
	sts		portr_dir,r16
	ldi		r16,0x00
	sts		portr_out,r16	
	ldi		r16,PORT_OPC_Totem_gc	
	sts		PORTr_PIN0CTRL,r16
	sts		PORTr_PIN1CTRL,r16
	ret

;----------------------------------------------------

.macro Task_Change
	call	multitask_task_change
.endm

multitask_task_change:
	ldi		XH,high(multitask_sp_table)				; save sp in table
	ldi		XL,low(multitask_sp_table)
	lds		R16,multitask_curr_task
	lsl		R16
	add		XL,R16
	clr		R16
	adc		XH,R16
	in		R16,CPU_SPL
	st		X+,R16
	in		R16,CPU_SPH
	st		X+,R16

	lds		R16,multitask_curr_task					; next task
	inc		R16
	cpi		R16,multitask_TASKS_COUNT
	brlo	multitask_task_change_no_first
	clr		R16										; roll to first task
	ldi		XH,high(multitask_sp_table)
	ldi		XL,low(multitask_sp_table)
multitask_task_change_no_first:
	sts		multitask_curr_task,R16

	ld		R16,X+									; load next sp
	ld		R17,X+
	cli
	out		CPU_SPH,R17
	out		CPU_SPL,R16
	sei
	ret
