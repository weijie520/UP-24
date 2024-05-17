// recur: implement a recursive function

//   r(n) = 0, if n <= 0
//        = 1, if n == 1
//        = 2*r(n-1) + 3*r(n-2), otherwise

//   please call r(28) and store the result in RAX
// ======
// ======

  mov rdi, 19
  call func
  jmp end

func:
  cmp rdi, 0
  jle r0

  cmp rdi, 1
  je r1

  dec rdi
  push rdi

  call func
  mov rbx, rax

  pop rdi
  dec rdi
  push rbx
  push rdi

  call func
  mov rcx, rax

  pop rdi
  pop rbx
  shl rbx, 1
  imul rcx, 3
  add rbx, rcx
  mov rax, rbx

  ret

r0:
  mov rax, 0
  ret
r1:
  mov rax, 1
  ret

end:
