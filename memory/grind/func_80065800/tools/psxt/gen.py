import sys
T = open('tmp/f65800/psxt/t1.c').read()
A = T.index("        t = &D_800F0BA8[arg0];")
B = T.index("        break;\n    }\n    D_800A37D4")
THEN = """            D_800A3488 = (s32)D_8009B8C8;
            prim->r0 = n << 6;
            prim->g0 = n * 0x70 / 3;
            prim->b0 = n * 0x60 / 15;
            *(s32 *)D_800A3490 = 0x2E;
"""
ELSE = """            if (*t >= 5) {
                D_800A3488 = (s32)D_8009B9F0;
            } else {
                D_800A3488 = (s32)D_8009B9E8;
            }
            *(s32 *)D_800A3490 = 0x2F;
"""
I = "        t = &D_800F0BA8[arg0];\n"
V = {
 'blk': I + "        if (*t >= 8) {\n            s32 m = 10 - *t;\n            n = m;\n" + THEN + "        } else {\n" + ELSE + "        }\n",
 'sexp': I + "        if (({ s16 v_ = *t; v_; }) >= 8) {\n            n = 10 - *t;\n" + THEN + "        } else {\n" + ELSE + "        }\n",
 'ter': I + "        n = (*t >= 8) ? 10 - *t : 0;\n        if (*t >= 8) {\n" + THEN + "        } else {\n" + ELSE + "        }\n",
 'sw': I + "        switch (*t >= 8) {\n        case 1:\n            n = 10 - *t;\n" + THEN + "            break;\n        default:\n" + ELSE + "        }\n",
 'orr': I + "        if (*t >= 8 || *t > 9) {\n            n = 10 - *t;\n" + THEN + "        } else {\n" + ELSE + "        }\n",
 'and': I + "        if (*t >= 8 && *t >= 0) {\n            n = 10 - *t;\n" + THEN + "        } else {\n" + ELSE + "        }\n",
}
for k, s in V.items():
    open(f'tmp/f65800/psxt/g_{k}.c', 'w', newline='\n').write(T[:A] + s + T[B:])
print(' '.join(f'tmp/f65800/psxt/g_{k}.c' for k in V))
