.section data:
    ; Global game variables
    byte        g_bRun                          1
    byte        g_cWall                         '#'
    byte        g_cFloor                        ' '
    byte        g_cSnakeHead                    '@'
    byte        g_cSnakeSegment                 'o'
    byte        g_cSnakeSkull                   'X'
    byte        g_cSnakeBone                    '+'
    byte        g_cFruit                        '*'
    byte        g_bDead                         0
    byte        g_eDir                          0

    ; String Literals
    byte        _LC0                            "clear", 0
    byte        _LC1                            10

    ; Player properties
    dword       g_dSnakePosX                    0
    dword       g_dSnakePosY                    0
    byte        g_cSnakeDir                     0
    byte        g_cSnakePrevDir                 0
    dword       g_dSnakeTailSize                0
    dword       g_dSnakeScore                   0
    dword       g_dSnakeHighScore               0
    byte        g_cSnakeTail                    fill(0, 4500)

    ; Game constant properties
    const       WIDTH                           10
    const       HEIGHT                          10
    const       DELAY                           50
    const       MAX_TAIL_SIZE                   500
    const       MAX_OUT_LINES                   255

    ; Game properties
    word        MAP_SIZE                        0

    ; Directions
    const       DIR_IDLE                        0
    const       DIR_UP                          1
    const       DIR_DOWN                        2
    const       DIR_LEFT                        3
    const       DIR_RIGHT                       4
    const       DIR_NONE                        5

.section bss:
    ; Global uninitialized variables
    byte        g_cMap                          100

.section code:
    call @_main
    movq $0, %r0
    end

@_main:
    pushq %bp
    movq %sp, %bp
    
    call @init

.l1:
    ldb g_bRun, %r0
    subq $0, %r0
    jue .l2
    call @update
    call @renderMap
    call @movePlayer
    jmp .l1

.l2:
    leave
    ret

@update:
    pushq %bp
    movq %sp, %bp

    

    leave
    ret

@renderMap:
    pushq %bp
    movq %sp, %bp

    pusharq
    
    leaq _LC0, %r0
    leaq _LC1, %r1

    ; Clear the screen.
    system %r0

    movq $0, %r2

.l1:
    
    jmp .l1

    poparq

    leave
    ret

@movePlayer:
    ret
@init:
    pushq %bp
    movq %sp, %bp

    pusharq
    
    ;;;;;;;;;;;;; Game initialization code ;;;;;;;;;;;;
    ; Enable conio.h terminal mode
    sconio $1



    ;;;;;;;;;;;;; Snake player code ;;;;;;;;;;;;;;;;
    ; Set the initial direction.
    stb DIR_UP, g_cSnakeDir
   
    ; Set the initial position (centre map).
    movq HEIGHT, %r0
    movq $2, %r1
    div %r1
    movq %r0, %r1
    movq WIDTH, %r0
    div %r1
    std %r0, g_dSnakePosX
    std %r1, g_dSnakePosY

    ; Set the initial tail size.
    std $4, g_dSnakeTailSize
    std $0, g_dSnakeScore

    ; TODO: Initialize snake tail

    poparq

    leave
    ret
