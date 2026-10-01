import sys, os, re
sys.path.insert(0, "memory/grind/func_80058580/r11")
import roles
s = open("memory/grind/func_80058580/candidate.c").read()

def gen(groups):
    """groups: list of (newname, [(var, val), ...]) -> rename every occurrence of those values to newname"""
    edits = []
    decls = []
    for new, vals in groups:
        for var, val in vals:
            occ = roles.occurrences(s, var)
            for k in roles.VALUES[var][val]:
                edits.append((occ[k], len(var), new))
        decls.append(f"    s32 {new};\n")
    out = s
    for p, ln, new in sorted(edits, reverse=True):
        out = out[:p] + new + out[p + ln:]
    anchor = "    s32 pick;\n"
    assert out.count(anchor) == 1
    return out.replace(anchor, anchor + "".join(decls), 1)

V = {}
# single-value splits (two per local)
for var, val in [("work1","kind2"),("work1","n449"),("work2","pace"),("work2","best"),("work3","idx"),("work3","cmask"),
                 ("work4","i"),("work4","k"),("work5","adj"),("work5","kindet"),("work3","sel3"),("work3","coin")]:
    V["s_"+val] = [(val+"_", [(var,val)])]
# subset splits
V["sub_flags3"] = [("n449_",[("work1","n449")]),("n445_",[("work2","n445")]),("n447_",[("work3","n447")])]
V["sub_m"] = [("m445_",[("work1","m445")]),("m449_",[("work2","m449")])]
V["sub_xy"] = [("x1_",[("work1","x1")]),("y1_",[("work2","y1")])]
V["sub_kind12"] = [("kind_",[("work1","kind"),("work1","kind2")])]
V["sub_ik"] = [("ctr_",[("work4","i"),("work4","k")])]
V["sub_slotmask"] = [("smask_",[("work3","slot"),("work3","mask")])]
V["sub_flags3m"] = [("n449_",[("work1","n449")]),("n445_",[("work2","n445")]),("n447_",[("work3","n447")]),("m445_",[("work1","m445")]),("m449_",[("work2","m449")])]
# work3 corrected grouping: V = side,n447,ang,prod,force,farflag,dang,coin,sel3 stays; others split
V["w3_regroup"] = [(v+"_",[("work3",v)]) for v in ["idx","dist","slot","mask","cmask","ok","script4"]]
# pairwise shared idx+pace+kind2 (the waypoint block triple)
V["sub_wptriple"] = [("kind2_",[("work1","kind2")]),("pace_",[("work2","pace")]),("idx_",[("work3","idx")])]
V["sub_cmask_ok"] = [("cm_",[("work3","cmask"),("work3","ok")])]
V["sub_top_adj"] = [("t5_",[("work5","top"),("work5","adj")])]
for name, g in V.items():
    open(f"tmp/rev58580r11/v/{name}.c","w",newline="\n").write(gen(g))
print(len(V))
