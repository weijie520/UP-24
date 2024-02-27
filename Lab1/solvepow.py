#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import base64
import hashlib
import time
import sys
from pwn import *

zero        =   ' │   │ '
one         =   '   │   '
two         =   ' ┌───┘ '
three       =   '  ───┤ '
four_n_nine =   ' └───┤ '
five        =   ' └───┐ '
six         =   ' ├───┐ '
seven       =   '     │ '
eight       =   ' ├───┤ '
# nine =  ' └───┤ '
add         =   ' ──┼── '
mul         =   '   ╳   '
div         =   ' ───── '

four_top    =   ' │   │ '
nine_top    =   ' ┌───┐ '

word_mapping = {zero:'0', one:'1', two:'2', three:'3', four_n_nine:'4', five:'5',six:'6', seven:'7',eight:'8', add: '+', mul:'*', div:'/'}
four_nine = {four_top:'4', nine_top:'9'}

def solve_pow(r):
    prefix = r.recvline().decode().split("'")[1]
    print(time.time(), "solving pow ...")
    solved = b''
    for i in range(1000000000):
        h = hashlib.sha1((prefix + str(i)).encode()).hexdigest()
        if h[:6] == '000000':
            solved = str(i).encode()
            print("solved =", solved)
            break
    print(time.time(), "done.")
    r.sendlineafter(b'string S: ', base64.b64encode(solved))

def translate(s):
    one_len = (len(s)-4)/5
    expr = ''
    for i in range(int(one_len*2+2),int(one_len*3),7):
        # for j in range(7):
        # print(word_mapping[s[i:i+7]])
        tmp = word_mapping[s[i:i+7]]
        if tmp == '4':
            expr += four_nine[s[i-int(one_len*2+2):i+7-int(one_len*2+2)]]
        else: expr += tmp
    
    # print(expr)  
    # print(eval(expr))
    return int(eval(expr))
        
   




if __name__ == "__main__":

    r = remote('up.zoolab.org',10681)
    solve_pow(r)

    cnt = int(r.recvuntil(b'challenges').decode().split(' ')[-2])
    r.recvline()

    for i in range(cnt):
        msg = r.recvuntil(b'?').decode()
        # print('msg:')
        # print(msg)
        # print('=======')
        expression = msg.split(' ')[-3]
        # expression = hashlib.sha1((expression).decode()).hexdigest()
        expression = base64.b64decode(expression).decode('utf-8')
        # print(expression)
        ans = str(translate(expression))
        # expression = base64.decode(expression)
        # print(ans)
        r.sendline(ans.encode())
        
        # print('=======')

    r.interactive()
    r.close()

# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :
