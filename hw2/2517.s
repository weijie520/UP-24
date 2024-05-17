  test eax, 0x80000000
  jz positive1
  mov dword ptr [0x600000], -1
  jmp test2
positive1:
  mov dword ptr [0x600000], 1
test2:
  test ebx, 0x80000000
  jz positive2
  mov dword ptr [0x600004], -1
  jmp test3
positive2:
  mov dword ptr [0x600004], 1
test3:
  test ecx, 0x80000000
  jz positive3
  mov dword ptr [0x600008], -1
  jmp test4
positive3:
  mov dword ptr [0x600008], 1
test4:
  test edx, 0x80000000
  jz positive4
  mov dword ptr [0x60000C], -1
  jmp end
positive4:
  mov dword ptr [0x60000C], 1
end: