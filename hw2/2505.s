; dispbin:
;   given a number in AX, store the corresponding bit string in str1.
;   for example, if AX = 0x1234, the result should be:
;   str1 = 0001001000111000
;   str1 @ 0x600000-600014

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