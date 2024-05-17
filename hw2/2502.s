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