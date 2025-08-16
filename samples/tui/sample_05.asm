; Sample 05 - Compare and Test (Intermediate)
; Test CMP and TEST instructions with flag effects

MOV AX, 100h
MOV BX, 100h
CMP AX, BX
;reg
MOV CX, 50h
CMP CX, 100h
;reg
MOV DX, 0Fh
TEST DX, 0Fh
;reg
TEST DX, 0F0h
;reg
