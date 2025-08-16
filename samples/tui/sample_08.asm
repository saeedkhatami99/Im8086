; Sample 08 - Exchange and Load Effective Address (Intermediate)
; Test XCHG and LEA instructions

MOV AX, 1111h
MOV BX, 2222h
XCHG AX, BX
;reg
MOV CX, 3333h
XCHG CX, AX
;reg
MOV SI, 1000h
MOV DI, 2000h
LEA BX, [SI+DI+10h]
;reg
LEA DX, [SI+100h]
;reg
