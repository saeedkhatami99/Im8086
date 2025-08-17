; Comprehensive stack test program
; Tests multiple stack operations and verifies stack viewer functionality

MOV AX, 1234h       ; Load test value into AX  
MOV BX, 5678h       ; Load test value into BX
MOV CX, 9ABCh       ; Load test value into CX
MOV DX, DEF0h       ; Load test value into DX

; Push multiple values onto stack
PUSH AX             ; Stack: 1234h
PUSH BX             ; Stack: 5678h, 1234h  
PUSH CX             ; Stack: 9ABCh, 5678h, 1234h
PUSH DX             ; Stack: DEF0h, 9ABCh, 5678h, 1234h

; Pop some values back
POP SI              ; SI = DEF0h, Stack: 9ABCh, 5678h, 1234h
POP DI              ; DI = 9ABCh, Stack: 5678h, 1234h

; Call subroutine (uses stack for return address)
CALL subroutine

; Pop remaining values
POP BX              ; BX = 5678h, Stack: 1234h
POP AX              ; AX = 1234h, Stack: empty

HLT                 ; End program

subroutine:
    PUSH BP         ; Save base pointer
    MOV BP, SP      ; Set up stack frame
    
    ; Do some work
    MOV AX, FFFFh
    
    MOV SP, BP      ; Restore stack
    POP BP          ; Restore base pointer
    RET             ; Return (pops return address)
