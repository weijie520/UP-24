; swapmem: 
;   swap the values in val1 and val2
;   val1 @ 0x600000-600008
;   val2 @ 0x600008-600010

mov rax, [0x600000]
xchg rax, [0x600008]
mov [0x600000], rax