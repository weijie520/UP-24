; math4: 
;   32-bit signed arithmetic
;   var4 = (var1 * -5) / (-var2 % var3)
;   note: overflowed part should be truncated
;   var1 @ 0x600000-600004
;   var2 @ 0x600004-600008
;   var3 @ 0x600008-60000c
;   var4 @ 0x60000c-600010

mov eax, [0x600000]
cdq
imul eax, -5
mov ebx, [0x600004]
mov ecx, [0x600008]
xchg eax, ebx
neg eax
cdq
idiv ecx
xchg eax, ebx
mov ecx,edx
cdq
idiv ecx
mov [0x60000c], eax