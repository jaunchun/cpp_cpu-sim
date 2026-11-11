; A tiny example program.
; It counts down from 10 to 0, subtracting 1 each time.


LDI r1 10    ; r1 = 10
LDI r2 1     ; r2 = 1
SUB r1 r2 r1 ; r1 = r1 - r2
JNZ 2        ; if r1 is not 0, repeat the subtraction
HLT          ; stop when r1 reaches 0