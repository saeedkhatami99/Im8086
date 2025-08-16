; Sample 11 - ASCII Adjustments (Advanced)
; Test ASCII adjustment instructions

MOV AL, 9h
MOV BL, 8h
ADD AL, BL
AAA
;reg
MOV AL, 15h
MOV BL, 27h
SUB AL, BL
AAS
;reg
MOV AL, 9h
MOV BL, 9h
MUL BL
AAM
;reg
MOV AX, 0125h
AAD
;reg
