; loop15:
;   str1 is a string contains 15 lowercase and uppercase alphbets.
;   implement a loop to convert all alplabets to lowercase,
;   and store the result in str2.
;   str1 @ 0x600000-600010
;   str2 @ 0x600010-600020

mov edx, 0
mov eax, 0
loop: 
mov ebx,0x5FFFFF
add ebx,edx
mov al, byte [ebx]
cmp al,0x41
jl check
cmp al,0x5A
jg check
add al,0x20
check:
add ebx,0x10
mov byte [ebx],al
inc edx
cmp edx,15
jle loop