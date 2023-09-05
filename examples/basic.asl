.section data:
	; Our source string.
	byte		STRING			"The source string.", 10, 0
	byte		FMT				"Source: %s Dest: %s", 10, 0
	byte		BSSBUFFER		10;fill(0, 20)

.section bss:
	; Our destination buffer (256 bytes of uninitialized memory).
	byte		_BSSBUFFER		256

.section code:
	call @_main

	; Set exit code to 0.
	movq $0, %r0	

	; Halt the VM.
	end

@_main:
	; Create our stack frame.
	pushq %bp
	movq %sp, %bp

	; Get the pointer to the first bytes of SRCSTR and BSSBUFFER
	; and store them in %r0 and %r1.
	leaq STRING, %r0
	leaq BSSBUFFER, %r1

	; Push our function arguments onto the stack.
	pushq %r0
	pushq %r1
	;call @strcpy

	; Cleanup
	add $16, %sp

	leaq FMT, %r2
	pushq %r0
	pushq %r1
	;printf %r2, %sp

	; Cleanup
	add $16, %sp

	pstr STRING
	pstr BSSBUFFER

	leave
	ret

@strcpy:
	; Create our stack frame.
	pushq %bp
	movq %sp, %bp	

	; Preserve all general purpose registers.
	pusharq
	
	; Load our arguments.
	ldq 24(%bp), %r0			; Source pointer
	ldq 16(%bp), %r1			; Destination pointer

	xor %r2, %r2
.l1:
	ldb (%r0, %r2), %r3
	cmp $0, %r3
	jue .l2
	stb %r3, (%r1, %r2)	
	inc %r2
	jmp .l1

.l2:
	; Restore the registers back.
	poparq

	leave
	ret
