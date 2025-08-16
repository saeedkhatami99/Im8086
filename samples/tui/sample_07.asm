; Sample 07 - Multiplication and Division (Intermediate)
; Test MUL, IMUL, DIV, IDIV instructions

MOV AX, 10h
MOV BL, 5h
MUL BL
;reg
MOV AX, 100h
MOV BL, 4h
DIV BL
;reg
MOV AX, 0FFFFh
MOV BL, 0FFh
IMUL BL
;reg
MOV AX, 200h
MOV DX, 0h
MOV BX, 10h
DIV BX
;reg
