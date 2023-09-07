.section data:
    byte        _LC0            "a very very big string. like reallyyyy big. some serious important data packing enormous string! massive us top secret packing sha2048base256aeswepwpawpa2wpa3wpa10 ecnrypted encoded humongous string!! humongougobongolo ascii string! hangalanganonogangas string!!!!!!!!", 10, 0
    byte        _LC1            "%b, ", 0
    byte        _LC2            10, 0

.section bss:
    byte        BSSBUFFER           256

.section code:
    call @_main

    ; Return code (0).
    movq $0, %r0

    ; Halt the VM.
    end

@_main:
    ; Create our stack frame.
    pushq %bp
    movq %sp, %bp

    leaq _LC0, %r0          ; Move address of _LC0 to %r0.        
    leaq BSSBUFFER, %r1     ; Move address of BSSBUFFER to %r1.
    pushq %r1               ; Push %r1 as our Destination Pointer.
    pushq %r0               ; Push %r0 as our Source Pointer.
    pushq sizeof(_LC0)      ; Push size of _LC0 in bytes as our size.
    call @memcpy            ; Call memcpy procedure.
    call @memprt            ; Call memprt procedure.
    add $24, %sp            ; Cleanup

    ; Destroy our stack frame.
    leave
    ret

@memprt:
    ; Create our stack frame.
    pushq %bp
    movq %sp, %bp

    ; Allocate a single byte of space on the stack.
    dec %sp

    ; Preserve our general purpose registers.
    pusharq

    ; Load our arguments off the stack.
    ldq 24(%bp), %r0        ; Source pointer
    ldq 16(%bp), %r1        ; Size

    ; Load the address of _LC2 to %r4.
    leaq _LC1, %r4

    ; Copy %bp to %sp and decrement it.
    movq %bp, %r5
    dec %r5

    ; for (%r3 = 0; %r3 != %r1; ++%r3)
    xor %r3, %r3
.l1:
    cmp %r1, %r3
    jue .l2
    ldb (%r0, %r3), %r2
    stb %r2, -1(%bp)
    printf %r4, %r5
    inc %r3
    jmp .l1

.l2:
    ; Print new line.
    pstr _LC2

    ; Restore our registers back.
    poparq
       
    ; Destroy our stack frame and return.
    leave
    ret

@memset:
    ; Create our stack frame.
    pushq %bp
    movq %sp, %bp

    ; Preserve our general purpose registers.
    pusharq
    
    ; Load our arguments of the stack.
    ldq 32(%bp), %r0        ; Source Pointer
    ldq 24(%bp), %r1        ; Value
    ldq 16(%bp), %r2        ; Size

    ; for (%r3 = 0; %r3 != %r2; ++%r3)
    xor %r3, %r3            
.l1:
    cmp %r2, %r3
    jue .l2                 
    stb %r1, (%r0, %r3)     ; Write our Value to Source Pointer + Index.
    inc %r3
    jmp .l1

.l2:
    ; Restore our registers back.
    poparq

    ; Destroy our stack frame and return.
    leave
    ret

@memcpy:
    pushq %bp
    movq %sp, %bp
    
    ; Preserve our general purpose registers.
    pusharq

    ; Load our arguments of the stack.
    ldq 32(%bp), %r0        ; Destination Pointer
    ldq 24(%bp), %r1        ; Source Pointer
    ldq 16(%bp), %r2        ; Size

    ; for (%r3 = 0; %r3 != %r2; ++%r3)
    xor %r3, %r3
.l1:
    cmp %r2, %r3
    jue .l3
    ldb (%r1, %r3), %r4     ; Load from Source Pointer + Index to %r4.
    stb %r4, (%r0, %r3)     ; Write %r4 to Dest Ptr + Index.
    inc %r3
    jmp .l1

.l3:
    ; Restore our registers back.
    poparq

    ; Destroy our stack frame and return.
    leave
    ret
