# Interrupt Service Routines

# Common function called by all ISRs
isr_common:
	# Push all the registers we want to back up
	pusha
	mov	%ds, %ax
	push	%eax

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs

	call	isr_handler	# Call our C ISR handler

	# Restore all the registers we backed up
	pop	%eax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs

	popa
	addl	$8, %esp
	sti
	iret


.macro ISR_NO_ERR num
	.global isr\num
	isr\num:
		cli
		push $0
		push $\num
		jmp isr_common
.endm

.macro ISR_ERR num
	.global isr\num
	isr\num:
		cli
		push $\num
		jmp isr_common
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
