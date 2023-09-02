.section data:
	byte			_LC0		$10, $0
	const			_LC1		$20
	byte			_LC2		"%lu %lu %lu", $10, $0	
	byte			_LC3		"%lu, ", $0

.section code:
	;printf %sp, _LC2
	call @_main
	movq $0, %r0
	end

@_main:
	pushq %bp
	movq %sp, %bp
	sub _LC1, %sp
	
	pushq _LC1
	pushb $1
	pushq %bp
	call @memset
	add $17, %sp

	pushq _LC1
	pushq %bp
	call @memprt
	add $16, %sp

	leave
	ret

@memprt:
	pushq %bp
	movq %sp, %bp

	pushq %r0
	pushq %r1			 
	pushq %r2				
	pushq %r3
	pushq %r4

	ldq 16(%bp), %r0				; Pointer
	ldq 24(%bp), %r1				; Size

	xorq %r2, %r2					; Counter
	xorq %r3, %r3					; Temporary byte
	movq %sp, %r4					; Temporary sp holder
	add $8, %r4
.l1:
	cmp %r3, %r1
	jue .l2
	ldb (%r0, %r2), %r3
	pushq %r3
	printf _LC3, %r4
	popq
	inc %r2
	jmp .l1

.l2:
	pstr _LC0

	popq %r4
	popq %r3
	popq %r2
	popq %r1
	popq %r0

	leave
	ret

@memset:
	pushq %bp
	movq %sp, %bp

	pushq %r0
	pushq %r1
	pushq %r2
	pushq %r3
	pushq %r4	

	ldq 16(%bp), %r0					; Pointer
	ldb 24(%bp), %r1					; Value
	ldq 25(%bp), %r2					; Size

	; Bug that needs to be fixed!
	;movq %sp, %r4
	;pushq %r2
	;pushq %r1
	;pushq %r0
	;leaq _LC2, %r4
	;printf %r4, %r3

	xorq %r3, %r3
.l1:
	cmp %r3, %r2
	jue .l2
	stb %r1, (%r0, %r3)
	inc %r3
	jmp .l1

.l2:
	popq %r4
	popq %r3
	popq %r2
	popq %r1
	popq %r0

	leave
	ret
