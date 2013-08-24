.global flush_idt
flush_idt:
	movl	4(%esp), %eax	# Get the pointer to the IDT
	lidt	(%eax)		# Load it up!
	ret
