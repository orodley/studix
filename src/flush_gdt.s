.global flush_gdt
flush_gdt:
	movl	4(%esp), %eax	# Get GDT pointer
	lgdt	(%eax)

	mov	$0x10, %ax	# 0x10 is the GDT offset to the data segment
	# Load the data segment registers
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	%ax, %ss

	ljmp	$8, $flush	# Load 8 (code segment offset) into cs
flush:
	ret
