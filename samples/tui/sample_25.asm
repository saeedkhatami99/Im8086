; Sample 25 - Comprehensive Test (Expert)
; Complete program demonstrating multiple instruction categories

; Initialize data
MOV AX, 1000h
MOV DS, AX
MOV ES, AX
MOV SS, AX
MOV SP, 0FFFEh

; Data movement and arithmetic
MOV AX, 1234h
MOV BX, 5678h
ADD AX, BX
MUL BX

; Stack operations
PUSHA
PUSHF

; String operations
MOV SI, 100h
MOV DI, 200h
MOV CX, 10h
MOV AL, 0AAh
CLD
REP STOSB

; Restore and display
POPF
POPA
;reg
;mem 1200 20

; Final operations
XOR AX, AX
HLT
