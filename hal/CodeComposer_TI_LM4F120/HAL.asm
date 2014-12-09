	.thumb

	.def SwitchContextToFirstTaskFP
	.def SwitchContextToFirstTask
	.def OS_CPU_SR_Save
	.def OS_CPU_SR_Restore

	.ref SPvalue          

	.text

NVIC_VTABLE_R: .word  	0xE000ED08

 	
   	.align 2                        
SPvalue2: .word SPvalue
	
	
SwitchContextToFirstTaskFP:
	LDR	 	R1, SPvalue2
	LDR  	R0, [R1] 	
	LDM     R0, {R4-R11,R14}
	ADDS    R0, R0, #0x24
	MSR     PSP, R0
	ORR     LR, LR, #0x04
	CPSIE   I	
	BX      LR

SwitchContextToFirstTask:
	LDR	 	R1, SPvalue2
	LDR  	R0, [R1]
	LDM     R0, {R4-R11}
	ADDS    R0, R0, #0x20
	MSR     PSP, R0
	ORR     LR, LR, #0x04
	CPSIE   I
	BX      LR

OS_CPU_SR_Save:
	MRS     R0, PRIMASK
	CPSID   I
	BX      LR

OS_CPU_SR_Restore:
	MSR     PRIMASK, R0
	BX      LR

	.end
