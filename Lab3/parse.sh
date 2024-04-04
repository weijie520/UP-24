#!bin/bash

python3 pwntool.py  | awk 'NR>8 {if (NR==9) printf("{"); printf("0x%s%s", $2, (FNR < 1207?", ":""))} END {print "}"}' > offset.txt