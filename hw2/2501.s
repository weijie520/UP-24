; addsub2:
;   final = val1 + val2 - val3
;    val1 @ 0x600000-600004
;    val2 @ 0x600004-600008
;    val3 @ 0x600008-60000c
;   final @ 0x60000c-600010

mov eax,[0x600000]
mov ebx,[0x600004]
mov ecx,[0x600008]
add eax,ebx
sub eax,ecx
mov [0x60000c],eax