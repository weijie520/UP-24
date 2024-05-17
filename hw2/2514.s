mov eax, [0x600000]
mov ecx, [0x600004]
neg ecx
cdq
imul eax, ecx
mov ecx, [0x600008]
sub ecx, ebx
cdq
idiv eax, ecx
mov [0x600008], eax