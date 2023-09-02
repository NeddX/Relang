.section data:
	byte		_LC0		" ,", $0
	byte		_LC1		$10, $0

.section code:
	call @_main
	movq $0, %r0
	end

@_main:
	; Create our stack frame.
	pushq %bp
	movq %sp, %bp
	sub $18, %sp					; Allocate 18 bytes of memory on the stack.

	leaq -10(%bp), %r0				; Get the address of the array from the address.
	stq %r0, -18(%bp)				; Store the address onto the stack.
	
	; Push our arguments onto the stack.
	pushq %r0						; Push the pointer.
	pushq $1						; Push the value to be set.
	pushq $10						; Push the size of the memory block.
	call @memset					
	add $24, %sp					; Cleanup

	; Print our memory block.
	pushq %r0
	pushq $10
	call @memprt
	add $16, %sp

	; Destroy our stack frame.
	leave								
	ret

@memset:
	; Create our stack frame.
	pushq %bp
	movq %sp, %bp
	
	; Reserve these registers
	pushq %r0
	pushq %r1
	pushq %r2
	pushq %r3

	; Load our arguments
	ldq 32(%bp), %r0				; Source pointer
	ldq 24(%bp), %r1				; Value
	ldq 16(%bp), %r2				; Size

	; Initialize our counter.
	xor %r3, %r3

	; for (%r3 = 0; %r3 != %r2; ++%r3)
.l1:
	cmpq %r3, %r2
	jue .l2
	stb %r1, (%r0, %r3)				; *(ptr + counter) = value
	inc %r3
	jmp .l1

.l2:
	; Restore our preserved registers.
	popq %r3
	popq %r2
	popq %r1
	popq %r0

	; Destroy our stack frame.
	leave
	ret

@memprt:
	; Create our stack frame.
	pushq %bp
	movq %sp, %bp
	
	; Preserve these registers.
	pushq %r0
	pushq %r1
	pushq %r2
	pushq %r3

	ldq 32(%bp), %r0				; Pointer to our memory block.
	ldq 24(%bp), %r1				; Size of our memory block.

	xor %r2, %r2					; Initialize our counter.
.l1:
	cmpq %r2, %r1
	jue .l2
	ldb (%r0, %r2), %r3 
	inc %r2
	jmp .l1
	
.l2:
	; Restore our preserved registers.
	popq %r3
	popq %r2
	popq %r1
	popq %r0

	; Destroy our stack frame.
	leave						
	ret
