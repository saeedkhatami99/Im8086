; Sample 23 - Interrupt Handling (Expert)
; Test software interrupts with BIOS simulation

MOV AH, 0Eh
MOV AL, 'A'
INT 10h
;reg
MOV AH, 02h
MOV DL, 0h
INT 21h
;reg
INTO
;reg
MOV AH, 4Ch
MOV AL, 0h
INT 21h
