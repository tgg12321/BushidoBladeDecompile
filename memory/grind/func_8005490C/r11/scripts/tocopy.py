"""tocopy.py <in> <out>: nocopy rotation form -> x/z-copy form (generator input only; post.py reverts)."""
import sys
s = open(sys.argv[1]).read()
for ind in ('        ', '                '):
    old = (f"{ind}rot_z = (vec.vz * c - vec.vx * sn) >> 12;\n{ind}vec.vx = (vec.vz * sn + vec.vx * c) >> 12;\n")
    assert s.count(old) == 1, ind
    s = s.replace(old, f"{ind}s32 x = vec.vx;\n{ind}s32 z = vec.vz;\n{ind}rot_z = (z * c - x * sn) >> 12;\n{ind}vec.vx = (z * sn + x * c) >> 12;\n")
open(sys.argv[2], 'w', newline='\n').write(s)
