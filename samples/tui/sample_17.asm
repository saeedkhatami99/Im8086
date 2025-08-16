; Sample 17 - String Compare Operations (Expert)
; Test CMPS with REPE and REPNE

MOV SI, 1000h
MOV DI, 2000h
MOV AL, 55h
MOV CX, 3h
REP STOSB
;mem 2000 8
MOV SI, 1000h
MOV AL, 55h
MOV CX, 3h
REP STOSB
;mem 1000 8
MOV SI, 1000h
MOV DI, 2000h
MOV CX, 5h
CLD
REPE CMPSB
;reg
