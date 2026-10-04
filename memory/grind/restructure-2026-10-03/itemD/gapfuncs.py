"""List every function in each LIBSND/LIBSPU gap object (address from build/bb2.map + nm offset)
with its census origin/tier/action, and the libscan libsyms entry at that address if any."""
import csv, json, re, subprocess
GAPS = ["libsnd/749E0", "libsnd/75174", "libsnd/760D0", "libsnd/76240", "libsnd/767D8", "libsnd/76BDC",
        "libsnd/774F8", "libspu/7A248", "libspu/7BC88"]
mp = open("build/bb2.map").read()
cen = {r["address"].upper().replace("0X", ""): r for r in csv.DictReader(open("docs/naming/function-names.csv", encoding="utf-8"))}
ls = json.load(open("docs/naming/libscan/libsyms.json"))
for g in GAPS:
    o = f"build/src/main/psxsdk/{g}.o"
    m = re.search(r"^ \.text\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) " + re.escape(o) + r"$", mp, re.M)
    base, size = int(m.group(1), 16), int(m.group(2), 16)
    print(f"=== {g} .text {base:08X}..{base+size:08X}")
    nm = subprocess.run(["mipsel-linux-gnu-nm", "-n", "-S", o], capture_output=True, text=True).stdout
    for l in nm.splitlines():
        p = l.split()
        if len(p) >= 3 and p[-2] in "Tt" and len(p) == 4:
            a = base + int(p[0], 16)
            A = f"{a:08X}"
            r = cen.get(A, {})
            lib = ls.get("0x" + A, [])
            libs = ";".join(f'{e["name"]}:{e["kind"]}:{e["mod"]}[{e["mod_start"]}..{e["mod_end"]}]' for e in lib)
            print(f"  {A} {p[3]:28s} sz={int(p[1],16):4X} {r.get('origin','')}/{r.get('tier','')} {libs}")
