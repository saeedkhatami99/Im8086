; Sample 20 - Complex Arithmetic with Carry (Expert)
; Test multi-precision arithmetic using ADC and SBB

MOV AX, 0FFFFh
MOV BX, 0FFFFh
ADD AX, 1h
ADC BX, 0h
;reg
MOV CX, 1000h
MOV DX, 2000h
SUB CX, 1001h
SBB DX, 2001h
;reg
CLC
MOV SI, 5555h
MOV DI, 3333h
ADC SI, DI
;reg
