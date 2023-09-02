.section data:
	byte			STR			"cool stuff very string lol", $255
	byte			NL			$10, $0

.section bss:
	byte			BSSVAR			$8
	byte			ANOTHERVSS		$16
	byte			LDOE			$1

.section code:
	stq $1283, BSSVAR
	call @_main
	movq $0, %r0
	end	

@_main:
	pushq %bp
	movq %sp, %bp

	pstr STR
	;stq $881928, BSSVAR
	ldq BSSVAR, %r0
	
	pint %r0
	pstr NL

	leave
	ret	
