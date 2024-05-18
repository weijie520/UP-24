; bubble:
;   bubble sort for 10 integers
;   a[0] @ 0x600000-600004
;   a[1] @ 0x600004-600008
;   a[2] @ 0x600008-60000c
;   a[3] @ 0x60000c-600010
;   a[4] @ 0x600010-600014
;   a[5] @ 0x600014-600018
;   a[6] @ 0x600018-60001c
;   a[7] @ 0x60001c-600020
;   a[8] @ 0x600020-600024
;   a[9] @ 0x600024-600028

mov edi,9
mov esi,0x600000
loop1:
  mov ebx,esi
  mov edx,edi
loop2:
  mov eax,[ebx]
  cmp eax,[ebx+4]
  jle noswap
  
  mov ecx, [ebx+4]
  mov [ebx+4],eax
  mov [ebx],ecx
noswap: 
  add ebx,4
  dec edx
  cmp edx,0
  jne loop2

  dec edi
  cmp edi,0
  jne loop1