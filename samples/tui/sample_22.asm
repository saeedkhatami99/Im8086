; Sample 22 - I/O Port Operations (Expert)
; Test IN and OUT instructions with enhanced simulation

MOV AL, 55h
OUT 60h, AL
IN AL, 60h
;reg
MOV AX, 1234h
OUT 3F8h, AX
IN AX, 3F8h
;reg
MOV AL, 0AAh
OUT 378h, AL
IN AL, 378h
;reg
