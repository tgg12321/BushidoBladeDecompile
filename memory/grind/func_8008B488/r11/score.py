#!/usr/bin/env python3
"""score.py cand.c [cand2.c ...] -- quick standalone scorer for func_8008B488.
Compiles head.h+cand via pp.sh/compile.sh, objdumps, and diffs against the
target .o (normalising branch targets and relocated immediates). Prints the
count of differing insns and (with -v) the differing lines."""
import subprocess, sys, re, difflib, os
ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
D = ROOT + "/tmp/f8b488s2"
FN = "func_8008B488"

def dis(o):
    out = subprocess.run(["mipsel-linux-gnu-objdump", "-d", "-z", o], capture_output=True, text=True).stdout
    lines, p = [], False
    for l in out.splitlines():
        if re.match(r"^[0-9a-f]+ <%s>:" % FN, l):
            p = True; continue
        if p and re.match(r"^[0-9a-f]+ <", l):
            break
        if p and "\t" in l:
            parts = l.split("\t")
            if len(parts) >= 3:
                ins = "\t".join(parts[2:]).strip()
                ins = re.sub(r"\s+", " ", ins)
                ins = re.sub(r"^(b\w*|j|jal) (.*?)([0-9a-f]+)( <.*>)?$", lambda m: m.group(1) + " " + m.group(2) + "T", ins)
                ins = re.sub(r"<[^>]*>", "", ins).strip()
                lines.append(ins)
    return lines

def ensure_target():
    t = D + "/target.o"
    if not os.path.exists(t):
        pre = open(ROOT + "/tools/decomp-permuter/prelude.inc").read().replace(".set gp=64", "")
        open(D + "/target.s", "w").write(pre + open(ROOT + "/asm/funcs/%s.s" % FN).read())
        subprocess.run(["mipsel-linux-gnu-as", "-I" + ROOT + "/include", "-march=r3000", "-mtune=r3000",
                        "-no-pad-sections", "-O1", "-G0", "-o", t, D + "/target.s"], check=True)
    return t

def norm_imm(l):
    # lui/lw/addiu %hi/%lo of symbols appear as 0 or addends in unlinked objects
    l = re.sub(r"^(lui \w+),0x[0-9a-f]+$", r"\1,IMM", l)
    l = re.sub(r"^(lui \w+),\d+$", r"\1,IMM", l)
    l = re.sub(r"^(l[wh]u?|s[wh]|addiu) (\w+),-?\d+\((at|v[01]|a[0-3])\)$", lambda m: m.group(0), l)
    return l

def score(c, verbose=False):
    i = D + "/_s.i"; o = D + "/_s.o"
    subprocess.run(["bash", D + "/pp.sh", c, i], check=True)
    r = subprocess.run(["bash", D + "/compile.sh", i, "-o", o])
    if r.returncode:
        return None, []
    a = [norm_imm(x) for x in dis(ensure_target())]
    b = [norm_imm(x) for x in dis(o)]
    sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
    n = 0; hunks = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            continue
        n += max(i2 - i1, j2 - j1)
        hunks.append((i1, a[i1:i2], b[j1:j2]))
    return (n, len(b)), hunks

if __name__ == "__main__":
    v = "-v" in sys.argv
    for c in [x for x in sys.argv[1:] if x != "-v"]:
        s, h = score(c, v)
        print(os.path.basename(c), s)
        if v:
            for i1, ta, ob in h:
                print("  @%d  T: %s\n       O: %s" % (i1, " ; ".join(ta), " ; ".join(ob)))
