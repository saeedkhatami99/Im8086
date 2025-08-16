; Test program for IDE with labels
MOV AX, 5        ; Initialize counter
MOV BX, 0        ; Initialize sum

loop_start:
ADD BX, AX       ; Add counter to sum
DEC AX           ; Decrement counter
CMP AX, 0        ; Compare with zero
JNE loop_start   ; Jump if not equal
HLT              ; Halt when done

; This program calculates the sum 5+4+3+2+1 = 15
; Result will be in BX register
