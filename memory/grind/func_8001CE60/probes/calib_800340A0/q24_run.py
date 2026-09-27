"""Apply the Q24 procedure (q24lib) to func_800340A0's reference form (landing_tu/A340_0.c = its exact landing body, measured
in the landing code6cac_b.c) and every banked single-declaration spelling (measured in the same TU).
Governed set = EVERY instruction of func_800340A0, in two mechanism parts:
  part 1, score-byte addressing — the 8 score/tiebreak accesses; position NOT decided (judged by presence, multiplicity);
  part 2, round-result store structure — every other instruction, branches included; position decided (aligned-position
          procedure with the destination test).
Exempt: part 1 forms absent from the cc1psx reference output; part 2 instructions exempt by the aligned-position rule.
Agreement = each spelling hits or misses the non-exempt governed set identically under both compilers (miss = a part-1
form short of its count, or a non-exempt part-2 reference instruction that differs)."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import q24lib as L

P = Path(sys.argv[1] if len(sys.argv) > 1 else "memory/grind/func_8001CE60/probes/calib_800340A0/landing_tu")
SYMS = ("D_800A3898", "D_800A3899", "D_800A38AA", "D_800A38AB")
is_p1 = lambda x: any(s in x["norm"] for s in SYMS)
tgt = L.parse(P / "A340_0.cc1.s")
psx = L.parse(P / "A340_0.cc1psx.s")
ref = {"cc1": tgt, "cc1psx": psx}
p1_norms = [x["norm"] for x in tgt if is_p1(x)]
ex1 = L.presence_exempt(p1_norms, psx)
ops, ex_t, ex_p = L.exempt(tgt, psx)
ex2 = {"cc1": {tgt[i]["line"] for i in ex_t if not is_p1(tgt[i])}, "cc1psx": {psx[j]["line"] for j in ex_p if not is_p1(psx[j])}}
print("== func_800340A0 reference form: build cc1 (== target) vs cc1psx")
print(f"part 1 governed ({len(p1_norms)}):", p1_norms)
print("part 1 EXEMPT (absent from cc1psx reference output):", dict(ex1) if ex1 else "none")
for tag, i1, i2, j1, j2 in ops:
    print(f"   {tag:8s} target[{i1}:{i2}] cc1psx[{j1}:{j2}]")
print("part 2 EXEMPT target-side:", [(tgt[i]["line"], tgt[i]["norm"]) for i in sorted(ex_t) if not is_p1(tgt[i])])
print("part 2 EXEMPT cc1psx-side:", [(psx[j]["line"], psx[j]["norm"]) for j in sorted(ex_p) if not is_p1(psx[j])])

print("\n== agreement (Q22/Q23 set-aside spellings listed but excluded from the test)")
SET_ASIDE = ("Q22-refused", "Q23-set-aside")
bad = 0
for c1 in sorted(P.glob("A340_*.cc1.s")):
    name = c1.name[:-len(".cc1.s")]
    if name == "A340_0":
        continue
    res = {}
    for comp in ("cc1", "cc1psx"):
        spell = L.parse(P / f"{name}.{comp}.s")
        m1 = L.presence_miss(p1_norms, ex1, spell)
        _, da, _ = L.differing(ref[comp], spell)
        m2 = [ref[comp][i]["line"] for i in sorted(da) if not is_p1(ref[comp][i]) and ref[comp][i]["line"] not in ex2[comp]]
        res[comp] = (bool(m1) or bool(m2), len(m1), len(m2))
    aside = any(t in name for t in SET_ASIDE)
    verdict = "set aside (Q22/Q23)" if aside else ("AGREE" if res["cc1"][0] == res["cc1psx"][0] else "DISAGREE")
    bad += verdict == "DISAGREE"
    fmt = lambda c: f"{'miss' if res[c][0] else 'HIT '} (part-1 forms short: {res[c][1]}, part-2 differing: {res[c][2]})"
    print(f"{name:30s} cc1 {fmt('cc1')} | cc1psx {fmt('cc1psx')} | {verdict}")
print(f"\nDISAGREE rows: {bad}")
