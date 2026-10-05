# relnorm.py BASE_O NEW_O ALIAS=BASE+OFF ... : compare two objects' .text disassembly after folding
# each alias symbol into BASE + OFF (LO16 immediates get +OFF; HI16 lines keep the base name).
# Prints differing lines; exit 0 when identical after folding.
import subprocess, sys, re
a, b = sys.argv[1], sys.argv[2]
alias = {}
for x in sys.argv[3:]:
    k, v = x.split("=")
    base, off = v.split("+")
    alias[k] = (base, int(off, 0))
def norm(o, fold):
    out = subprocess.run(["mipsel-linux-gnu-objdump", "-d", "-r", "--no-show-raw-insn", o], capture_output=True, text=True).stdout
    lines = [re.sub(r"^\s*[0-9a-f]+:\s*", "", l) for l in out.splitlines() if l.strip() and "file format" not in l]
    res = []; pend = None
    for i, l in enumerate(lines):
        m = re.match(r"R_MIPS_(HI16|LO16|32|26)\s+(\S+)", l)
        if m and fold and m.group(2) in alias:
            base, off = alias[m.group(2)]
            if m.group(1) == "LO16" and res:
                # previous line is the instruction carrying the low immediate
                ins = res[-1]
                mm = re.search(r"(-?\d+)\((\w+)\)$", ins) or re.search(r"(-?\d+)$", ins)
                if mm:
                    v = int(mm.group(1)) + off
                    res[-1] = ins[:mm.start(1)] + str(v) + ins[mm.end(1):]
            l = "R_MIPS_%s\t%s" % (m.group(1), base)
        res.append(l)
    return res
x, y = norm(a, True), norm(b, False)
import difflib
d = [l for l in difflib.unified_diff(x, y, lineterm="", n=0) if not l.startswith(("---", "+++", "@@"))]
print("\n".join(d[:40]) if d else "IDENTICAL after folding")
sys.exit(1 if d else 0)
