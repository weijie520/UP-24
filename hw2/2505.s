mov ecx, 16
mov ebx, 0x600000

loop1:
  test ax, 0x8000
  jz zero
  mov byte ptr [ebx], 0x31
  jmp next
zero:
  mov byte ptr [ebx], 0x30
next:
  shl ax, 1
  inc ebx
  dec ecx
  jnz loop1