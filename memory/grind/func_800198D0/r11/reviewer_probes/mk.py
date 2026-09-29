import re
src = open('memory/grind/func_800198D0/candidate.c').read()
def rep(s, a, b, n=1):
    assert s.count(a) == n, (a, s.count(a))
    return s.replace(a, b)
# P1: case-1 suffix length inline, no second write to nbits
p1 = rep(src, """                    nbits = nbits - 1;
                    GETBITS_PRE(temp, nbits, 1 << nbits);""", """                    GETBITS_PRE(temp, nbits - 1, 1 << (nbits - 1));""")
p1 = rep(p1, """                        nbits2 = nbits2 - 1;
                        GETBITS_PRE(temp, nbits2, 1 << nbits2);""", """                        GETBITS_PRE(temp, nbits2 - 1, 1 << (nbits2 - 1));""")
open('tmp/rev800198D0/p1_inline_len.c','w').write(p1)
# P2: case-2 flag own local, inverted condition
p2 = rep(src, """                GETBITS(temp, 1);
                if (temp) {
                    temp = 0;
                } else {
                    GETBITS(temp, 12);
                }""", """                u32 flag;

                GETBITS(flag, 1);
                if (!flag) {
                    GETBITS(temp, 12);
                } else {
                    temp = 0;
                }""")
open('tmp/rev800198D0/p2_flag_inv.c','w').write(p2)
# P3: case-2 flag own local, temp = 0 default then conditional read
p3 = rep(src, """                GETBITS(temp, 1);
                if (temp) {
                    temp = 0;
                } else {
                    GETBITS(temp, 12);
                }""", """                s16 flag;

                GETBITS(flag, 1);
                temp = 0;
                if (!flag) {
                    GETBITS(temp, 12);
                }""")
open('tmp/rev800198D0/p3_flag_default.c','w').write(p3)
# P4: suffix length as fresh local in the else-if block
p4 = rep(src, """                    nbits = nbits - 1;
                    GETBITS_PRE(temp, nbits, 1 << nbits);""", """                    s32 len = nbits - 1;
                    GETBITS_PRE(temp, len, 1 << len);""")
p4 = rep(p4, """                        nbits2 = nbits2 - 1;
                        GETBITS_PRE(temp, nbits2, 1 << nbits2);""", """                        s32 len2 = nbits2 - 1;
                        GETBITS_PRE(temp, len2, 1 << len2);""")
open('tmp/rev800198D0/p4_len_block.c','w').write(p4)
print("ok")
