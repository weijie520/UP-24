; posneg: 
;   test if registers are positive or negative.
;   if ( eax >= 0 ) { var1 = 1 } else { var1 = -1 }
;   if ( ebx >= 0 ) { var2 = 1 } else { var2 = -1 }
;   if ( ecx >= 0 ) { var3 = 1 } else { var3 = -1 }
;   if ( edx >= 0 ) { var4 = 1 } else { var4 = -1 } 
;   var1 @ 0x600000-600004
;   var2 @ 0x600004-600008
;   var3 @ 0x600008-60000c
;   var4 @ 0x60000c-600010

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