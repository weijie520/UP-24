; ul+lu:
;   convert the alphabet in CH from upper
;   to lower or from lower to upper

cmp ch, 0x61
jl to_lower
sub ch, 0x20
jmp end

to_lower:
  add ch, 0x20
end: