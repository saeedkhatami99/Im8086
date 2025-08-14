; Simple 8086 test program
MOV AX, 10h      ; Load 16 into AX
MOV BX, 20h      ; Load 32 into BX
ADD AX, BX       ; Add BX to AX (result: 48h = 72)
MOV [100h], AX   ; Store result in memory at 100h
INC AX           ; Increment AX to 49h
CMP AX, 50h      ; Compare with 80
JL loop_start    ; Jump if less
HLT              ; Halt

loop_start:
  ADD AX, 1      ; Add 1 to AX
  CMP AX, 50h    ; Compare with 80
  JL loop_start  ; Loop if less
  HLT            ; Halt when done
