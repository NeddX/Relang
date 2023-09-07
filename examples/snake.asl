.section data:

.section bss:
    byte        g_Arena         100

.section code:
    call @_main
    movq $0, %r0
    end

@_main:
    pushq %bp
    movq %sp, %bp
    
    call @init

    leave
    ret

@init:
    sconio $1
    ret
