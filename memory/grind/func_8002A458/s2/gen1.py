"""Site-1 dataflow sweep on the r1_dxyz chassis (dx/dy/dz reuse; current stock cc1).
Axes: which variable receives the sum (A), where the copy B = A (or A = B after computing into B)
sits, which variable each consumer reads, and where the table byte and the root go.
Writes tmp/func_8002A458/g1/<tag>.c and prints the list."""
import itertools, os

ROOT = os.path.dirname(os.path.abspath(__file__))
b = open(os.path.join(ROOT, 'r1_dxyz.c'), encoding='utf-8').read()
S = b.index("    h_sq = dx * dx + dz * dz - dy * dy;\n")
E = b.index("    *(s16 *)(scr + 0xF8) = -ratan2(dy, len);")
pre, post = b[:S], b[E:]
out_dir = os.path.join(ROOT, 'g1')
os.makedirs(out_dir, exist_ok=True)
BS = chr(92) + "n"
ISL = ('        __asm__ volatile(\n'
       '            "move   $12, %0' + BS + '"\n'
       '            "mtc2   $12, $30' + BS + '"\n'
       '            "nop' + BS + '"\n'
       '            "nop' + BS + '"\n'
       '            :: "r"(VAR) : "$12");\n'
       '        __asm__ volatile(\n'
       '            "move   $12, %0' + BS + '"\n'
       '            "swc2   $31, 0($12)' + BS + '"\n'
       '            :: "r"(&sp_tmp) : "$12", "memory");\n')
names = []
seen = set()
# A = h_sq (sum), B = n (copy).  dirn: 'AB' = sum into A, B = A;  'BA' = sum into B, A = B.
for dirn, cpos, prv, ltv, cmpv, lutv, islv, shv, tbv, resv in itertools.product(
        ['AB', 'BA'], ['pre', 'post'],
        ['A', 'B'], ['A', 'B'], ['A', 'B'], ['A', 'B'], ['A', 'B'], ['A', 'B'],
        ['tbl'], ['len', 'A']):
    V = {'A': 'h_sq', 'B': 'n'}
    src_, dst_ = ('h_sq', 'n') if dirn == 'AB' else ('n', 'h_sq')
    s = "    %s = dx * dx + dz * dz - dy * dy;\n" % src_
    cp = "    %s = %s;\n" % (dst_, src_)
    if cpos == 'pre':
        s += cp
    s += "    if (%s < 0) {\n        printf(D_80010478, %s);\n        return;\n    }\n" % (V[ltv], V[prv])
    if cpos == 'post':
        s += cp
    # a consumer before the copy must read the source
    if cpos == 'post' and (V[ltv] == dst_ or V[prv] == dst_):
        continue
    res = 'len' if resv == 'len' else 'h_sq'
    s += "    if ((u32)%s < 0x400) {\n" % V[cmpv]
    s += "        %s = (u32)*(((u8 *)&D_8008D118) + %s) >> 3;\n    } else {\n" % (res, V[lutv])
    s += ISL.replace('VAR', V[islv])
    s += "        {\n            s32 lz = ~1;\n            s32 shift;\n            s32 tbl;\n"
    s += "            lz &= sp_tmp;\n            shift = 0x16 - lz;\n"
    s += "            tbl = *(((u8 *)&D_8008D118) + ((u32)%s >> shift));\n" % V[shv]
    s += "            %s = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));\n        }\n    }\n" % res
    body = s
    p2 = post
    if res == 'h_sq':
        p2 = post.replace("-ratan2(dy, len);", "-ratan2(dy, h_sq);", 1)
    if body + p2 in seen:
        continue
    seen.add(body + p2)
    tag = '%s_%s_p%s%s_c%s%s_i%s%s_%s_%s' % (dirn, cpos, prv, ltv, cmpv, lutv, islv, shv, tbv, resv)
    name = os.path.join(out_dir, tag + '.c')
    open(name, 'w', encoding='utf-8', newline='\n').write(pre + body + p2)
    names.append('tmp/func_8002A458/g1/' + tag + '.c')
open(os.path.join(ROOT, 'g1_list.txt'), 'w').write(','.join(names))
print(len(names))
