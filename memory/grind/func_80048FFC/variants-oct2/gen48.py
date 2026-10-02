import re
b = open('ffc_f6.c').read()
def w(n, s): open('x_%s.c' % n, 'w', newline='\n').write(s)
OS = "        old = phase;\n        phase >>= 1;\n"
assert OS in b
H = "        rect.h = old;\n"
C2 = "        SetDrawMove(p, &rect, nx, ny);\n"
# a: comma expression in the rect.h store
w('a', b.replace(OS, "").replace(H, "        rect.h = (old = phase, phase >>= 1, old);\n"))
# b: shift inside the call's argument
w('b', b.replace(OS, "        old = phase;\n").replace(C2, "        SetDrawMove(p, &rect, nx, (phase >>= 1, ny));\n"))
# d: RECT pointer for the second strip
w('d', b.replace("        rect.y = cy + dy;\n        rect.h = old;\n        SetDrawMove(p, &rect, nx, ny);\n",
                 "        r->y = cy + dy;\n        r->h = old;\n        SetDrawMove(p, r, nx, ny);\n").replace("    RECT rect;\n", "    RECT rect;\n    RECT *r = &rect;\n"))
# e: dst recomputed from x2/x2f, y2/y2f halved after call two
e = b.replace("        x2f >>= 1;\n        y2f >>= 1;\n", "").replace(C2, "        SetDrawMove(p, &rect, x2 + x2f, y2 + y2f);\n").replace("        p++;\n    } while", "        p++;\n        x2f >>= 1;\n        y2f >>= 1;\n    } while")
w('e', e)
# f: all other halvings + i++ after call two
f = b.replace("        xf >>= 1;\n        yf >>= 1;\n        x2f >>= 1;\n        y2f >>= 1;\n        h >>= 1;\n        end >>= 1;\n        i++;\n", "").replace("        p++;\n    } while", "        p++;\n        xf >>= 1;\n        yf >>= 1;\n        x2f >>= 1;\n        y2f >>= 1;\n        h >>= 1;\n        end >>= 1;\n        i++;\n    } while")
w('f', f)
# g: second strip's rect stores before addPrim one
g = b.replace("        rect.y = cy + dy;\n        rect.h = old;\n", "").replace("        i++;\n        ot =", "        i++;\n        rect.y = cy + dy;\n        rect.h = old;\n        ot =", 1)
w('g', g)
# h: pre-increment in the call
h = b.replace("        p++;\n        rect.y", "        rect.y", 1).replace(C2, "        SetDrawMove(++p, &rect, nx, ny);\n")
w('h', h)
# j: s16 old
w('j', b.replace("        s32 old;\n", "        s16 old;\n"))
# k: copy+shift right before call two
k = b.replace(OS, "").replace(H, "        old = phase;\n        phase >>= 1;\n        rect.h = old;\n")
w('k', k)
# l: shift via old after store
l = b.replace(OS, "        old = phase;\n").replace(H, "        rect.h = old;\n        phase = old >> 1;\n")
w('l', l)
