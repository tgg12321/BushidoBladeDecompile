"""Q24 procedure for func_8001CE60's reference form (the exact landing body in the landing code6cac.c, score/tiebreak bytes
declared as [2] arrays): governed set = mechanism part 1, score-byte addressing (position NOT decided: judged by presence
with multiplicity), the 14 score/tiebreak accesses listed with target addresses. Also measures the admissible
scalar-declaration spelling (ordinary conditional selects, sel_*.fn) under both compilers."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import q24lib as L

O = Path(sys.argv[1] if len(sys.argv) > 1 else "memory/grind/func_8001CE60/probes/calib_800340A0/landing_tu_func_8001CE60")
TARGET_ADDRS = ["0x8001D0E0", "0x8001D0F4", "0x8001D150", "0x8001D164", "0x8001D190", "0x8001D280", "0x8001D480",
                "0x8001D49C", "0x8001D4C0", "0x8001D4DC", "0x8001D714", "0x8001D724", "0x8001D72C", "0x8001D740"]
SYMS = ("D_800A3898", "D_800A3899", "D_800A38AA", "D_800A38AB")
tgt = L.parse(O / "land.s.fn")
psx = L.parse(O / "land.psx.s.fn")
gov = [x for x in tgt if any(s in x["norm"] for s in SYMS)]
assert len(gov) == len(TARGET_ADDRS), len(gov)
print(f"instructions: build cc1 {len(tgt)}, cc1psx {len(psx)}")
print("governed (part 1, score-byte addressing; position not decided), program order = target order:")
for x, a in zip(gov, TARGET_ADDRS):
    print(f"   {a}  land.s.fn:{x['line']:3d}  {x['norm']}")
norms = [x["norm"] for x in gov]
ex = L.presence_exempt(norms, psx)
print(f"EXEMPT (governed forms absent from the cc1psx reference output): {dict(ex) if ex else 'none'}")
print("\nagreement:")
for name, a, b in (("reference (arrays, landing body)", "land.s.fn", "land.psx.s.fn"),
                   ("sel: scalar declaration, conditional selects", "sel.s.fn", "sel.psx.s.fn")):
    m1 = L.presence_miss(norms, ex, L.parse(O / a))
    m2 = L.presence_miss(norms, ex, L.parse(O / b))
    v = "AGREE" if bool(m1) == bool(m2) else "DISAGREE"
    fmt = lambda m: "HIT" if not m else "miss " + "; ".join(f"{f} need {n} have {h}" for f, (n, h) in m.items())
    print(f"  {name}\n     cc1: {fmt(m1)}\n     cc1psx: {fmt(m2)}\n     => {v}")
