"""stat1 kept as its own allocno (for its FINDREG / own-preference trace): stat1 split alone,
plus `do { } while (0);` after its write and a chain-extender in the `lo` arm.
usage: python3 tmp/func_80055138/r11/mk5.py -> x/abl_stat1_keep.c, x/abl_stat1_keep2.c"""
D = 'tmp/func_80055138/r11'
s = open(D + '/v/abl_stat1.c').read()


def one(s, o, n):
    assert s.count(o) == 1, o
    return s.replace(o, n)


a = one(s, "                    stat1 = e[1];\n",
        "                    stat1 = e[1];\n                    do { } while (0); /* FAKE */\n")
a = one(a, "                        lo = stat1;\n",
        "                        lo = stat1 + lo - lo; /* FAKE */\n")
open(D + '/x/abl_stat1_keep.c', 'w', newline='\n').write(a)
b = one(s, "                        lo = stat1;\n",
        "                        lo = stat1 + lo - lo; /* FAKE */\n")
open(D + '/x/abl_stat1_keep2.c', 'w', newline='\n').write(b)
print('ok')
