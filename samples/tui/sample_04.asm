; Sample 04 - Logical Operations (Beginner-Intermediate)
; Test AND, OR, XOR, NOT instructions

MOV AX, 0FFh
MOV BX, 55h
AND AX, BX
;reg
MOV CX, 33h
MOV DX, CCh
OR CX, DX
;reg
MOV SI, 0F0h
XOR SI, 0Fh
NOT SI
;reg
