# Interrupt Service Routines

# Common function called by all ISRs
isr_common:
	# Push all the registers we want to back up
	pusha			# Push edi, esi, ebp, esp, ebx, edx, ecx, eax
	mov	%ds, %ax
	pushl	%eax

	# Load kernel mode segments
	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs

	call	isr_handler	# Call our C ISR handler

	# Restore all the registers we backed up
	popl	%eax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs

	popa
	addl	$8, %esp
	sti
	iret

# Common function called by all IRQs
irq_common:
	# Push all the registers we want to back up
	pusha			# Push edi, esi, ebp, esp, ebx, edx, ecx, eax
	mov	%ds, %ax
	pushl	%eax

	# Load kernel mode segments
	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs

	call	irq_handler	# Call our C IRQ handler

	# Restore all the registers we backed up
	popl	%eax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs

	popa
	addl	$8, %esp
	sti
	iret

# Macro for ISRs that don't push an error code; we need to push a dummy one
.macro ISR_NO_ERR num
	.global isr\num
	isr\num:
		cli
		push 	$0
		push 	$\num
		jmp 	isr_common
.endm

# Macro for ISRs that push an error code
.macro ISR_ERR num
	.global isr\num
	isr\num:
		cli
		push 	$\num
		jmp 	isr_common
.endm

# Macro for IRQs; different stub
.macro IRQ num
	.global isr\num
	isr\num:
		cli
		push	$0
		push 	$\num
		jmp 	irq_common
.endm

# Create all the ISRs
# Only ISRs 8, and 10-14 push error codes, the rest need dummy error codes
ISR_NO_ERR 0
ISR_NO_ERR 1
ISR_NO_ERR 2
ISR_NO_ERR 3
ISR_NO_ERR 4
ISR_NO_ERR 5
ISR_NO_ERR 6
ISR_NO_ERR 7
ISR_ERR 8
ISR_NO_ERR 9
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NO_ERR 15
ISR_NO_ERR 16
ISR_NO_ERR 17
ISR_NO_ERR 18
ISR_NO_ERR 19
ISR_NO_ERR 20
ISR_NO_ERR 21
ISR_NO_ERR 22
ISR_NO_ERR 23
ISR_NO_ERR 24
ISR_NO_ERR 25
ISR_NO_ERR 26
ISR_NO_ERR 27
ISR_NO_ERR 28
ISR_NO_ERR 29
ISR_NO_ERR 30
ISR_NO_ERR 31
# IRQ handlers
IRQ 32
IRQ 33
IRQ 34
IRQ 35
IRQ 36
IRQ 37
IRQ 38
IRQ 39
IRQ 40
IRQ 41
IRQ 42
IRQ 43
IRQ 44
IRQ 45
IRQ 46
IRQ 47
