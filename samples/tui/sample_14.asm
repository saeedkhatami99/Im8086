; Sample 14 - Flag Register Operations (Advanced)
; Test LAHF, SAHF, PUSHF, POPF

MOV AX, 100h
ADD AX, 200h
LAHF
;reg
MOV AH, 85h
SAHF
;reg
PUSHF
;stack
MOV AX, 0h
POPF
;reg
