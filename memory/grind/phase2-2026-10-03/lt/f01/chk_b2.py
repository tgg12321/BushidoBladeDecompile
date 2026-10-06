# chk_b2.py [NAME...] : for each abl_b2.py variant, the instruction diff (relocations dropped) of the
# scratch-compiled variant against the target object: which store moves where.
import subprocess, re, difflib, sys
V = sys.argv[1:] or ["80064E90_last", "80064ED8_last", "80064F20_last", "80065000_last", "8006505C_last", "800650A4_last",
     "800650EC_last", "80065134_last", "80065264_last", "800652AC_last", "80064F68_last", "80064FB4_last",
     "8006517C_t", "8006517C_t2", "800651F0_t", "800651F0_t2", "80064F68_q", "80064FB4_q"]
def od(f, which):
    o = subprocess.run(["wsl", "bash", "memory/grind/phase2-2026-10-03/lt/f01/objd_b.sh", f] + ([which] if which else []), capture_output=True, text=True).stdout
    return [re.sub(r"\s+", " ", l.strip()) for l in o.splitlines()[1:] if l.strip() and "R_MIPS" not in l]
for v in V:
    f = "func_" + v.split("_")[0]
    subprocess.run(["python3", "memory/grind/phase2-2026-10-03/lt/f01/objabl_b2.py", v, f], capture_output=True)
    a, b = od(f, "base"), od(f, None)
    d = [l for l in difflib.unified_diff(a, b, lineterm="", n=0) if l[:1] in "+-" and l[:3] not in ("+++", "---")]
    print("==", v); print("   " + " | ".join(d))
