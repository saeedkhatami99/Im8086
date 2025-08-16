; Sample 15 - Push/Pop All Registers (Advanced)
; Test PUSHA and POPA instructions

MOV AX, 1111h
MOV BX, 2222h
MOV CX, 3333h
MOV DX, 4444h
MOV SI, 5555h
MOV DI, 6666h
MOV BP, 7777h
;reg
PUSHA
;stack
MOV AX, 0h
MOV BX, 0h
MOV CX, 0h
MOV DX, 0h
MOV SI, 0h
MOV DI, 0h
MOV BP, 0h
;reg
POPA
;reg
