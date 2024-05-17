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