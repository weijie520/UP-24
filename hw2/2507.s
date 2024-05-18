; isolatebit:
;   get the value bit-11 ~ bit-5 in AX and store the result in val1
;   (zero-based bit index)
;   val1 @ 0x600000-600001
;   val2 @ 0x600001-600002

and ax, 0x0FFF
shr ax, 5
mov [0x600000], al