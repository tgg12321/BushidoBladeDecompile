#!/usr/bin/env python3
"""mk_struct.py <cand.c> <pv_work4.c> <outdir>: the work4 / work5 structural respellings.
  pvs_work4a  pv_work4 with the waypoint walk as `for (; n_ >= 2; n_--)`
  pvs_work4b  pv_work4 with the walk index re-read (`n_ = p->unk_362 - 1;`) instead of copied
  pvs_work5a  no work5 at all: `if (work1 >> 27)`, the skill offset written inline twice, et tested directly
  pvs_work5b  only the et copy removed (et tested directly): the Q34 no-copy body"""
import sys, os
cand, pv4, out = sys.argv[1], sys.argv[2], sys.argv[3]
os.makedirs(out, exist_ok=True)
s = open(pv4).read()
old = """                        while (n_ >= 2) {
                            dist += SquareRoot0(CPU_SQ(p->unk_364[n_].x - p->unk_364[n_ - 1].x) +
                                                CPU_SQ(p->unk_364[n_].z - p->unk_364[n_ - 1].z));
                            n_--;
                        }
"""
assert old in s
open(f"{out}/pvs_work4a.c", "w", newline="\n").write(s.replace(old, """                        for (; n_ >= 2; n_--) {
                            dist += SquareRoot0(CPU_SQ(p->unk_364[n_].x - p->unk_364[n_ - 1].x) +
                                                CPU_SQ(p->unk_364[n_].z - p->unk_364[n_ - 1].z));
                        }
"""))
assert "n_ = wi;" in s
open(f"{out}/pvs_work4b.c", "w", newline="\n").write(s.replace("n_ = wi;", "n_ = p->unk_362 - 1;"))
f = open(cand).read()
ET = ("""                            work5 = et;
                            if (work5 < 5) {""", """                            if (et < 5) {""")
c = f.replace("                                work5 = work1 >> 27;\n", "").replace("if (work5) {", "if (work1 >> 27) {", 1)
c = c.replace("""                                work5 = (((0x1000 - lv) * 625) >> 10) - 400;
                                work1 += work5 + p->unk_40A;
                                hi += work5 + p->unk_40A;""", """                                work1 += ((((0x1000 - lv) * 625) >> 10) - 400) + p->unk_40A;
                                hi += ((((0x1000 - lv) * 625) >> 10) - 400) + p->unk_40A;""")
c = c.replace(*ET).replace("switch (work5) {", "switch (et) {")
assert "work5 =" not in c.split("    s32 work5;\n")[1]
open(f"{out}/pvs_work5a.c", "w", newline="\n").write(c)
d = f.replace(*ET).replace("switch (work5) {", "switch (et) {")
open(f"{out}/pvs_work5b.c", "w", newline="\n").write(d)
