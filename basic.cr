.section data:
	byte		nl			"\n", #0	

.section code:		
	call @_main
	mov %r0, #1
	end

@_main:
	push %bp
	mov %bp, %sp

	push #30
	push #5
	
	call @expression	
	add %sp, #16	

	leave
	ret

@expression:
	push %bp
	mov %bp, %sp

	load %r0, qword [%bp+#24]
	load %r1, qword [%bp+#16]

	pint %r0
	pstr nl
	pint %r1
	pstr nl

	leave
	ret
