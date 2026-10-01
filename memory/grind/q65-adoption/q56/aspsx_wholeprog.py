#!/usr/bin/env python3
"""aspsx_wholeprog.py — every symbol load/store the POC build's C emits, decided by Sony's ASPSX.

For each C file of the POC (/tmp/q56/model; split TUs as their two parts) take the maspsx INPUT stream
(cpp | cc1 | prologue_fix) and build a probe:
  - the file's own definitions FIRST (cc1psx -G8 emits them before the bodies): `.comm s,size` as
    written by cc1 (third field dropped); the POC's initialized/static stand-ins as `.sdata` objects
    (the definition kind the original file had, to which ASPSX gives gp at every offset);
  - every macro load/store whose operand is a symbol (`lw $2,sym`, `sb $3,sym+1`, `sw $2,sym($4)`),
    in order, each followed by a marker `addiu $0,$0,k`.
The probe is assembled twice: by the real ASPSX 2.34 (-G8) and by the POC maspsx (-G8). The per-access gp
decisions must agree; the POC maspsx output is the one that links to the oracle.
usage (WSL, repo root): python3 tmp/q56/aspsx_wholeprog.py -> tmp/q56/aspsx_wholeprog.txt"""
import re, subprocess, sys
from pathlib import Path
sys.path.insert(0, "tmp/q56")
import psyqobj

M = "/tmp/q56/model"
OUT = Path("tmp/q56/probes/whole"); OUT.mkdir(parents=True, exist_ok=True)
GP_FILES = "text1a_pre text1a_pre_tu2 text1a_post code6cac_b3 code6cac_b4 code6cac_b5 text1b_tu1d".split()
SPLIT = {"text1a_c", "code6cac_b_tu2"}
NONCOMM = set(l.split()[0] for l in open(M + "/poc_noncomm_syms.txt") if l.strip() and not l.startswith("#"))
LS = r"(lb|lbu|lh|lhu|lw|sb|sh|sw|lwl|lwr|swl|swr)"
INS = re.compile(r"^\t%s\t(\$\w+),([A-Za-z_][\w.]*)((?:\+\d+)?)((?:\(\$\w+\))?)$" % LS)
CPP = ("mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ "
       "-D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C")
CC1 = "tools/gcc-2.7.2/build/cc1 -O2 {g} -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"

def stream(tu):
    g = "-G8" if tu in GP_FILES else "-G0"
    return subprocess.run(f"cd {M} && {CPP} src/{tu}.c 2>/dev/null | {CC1.format(g=g)} | python3 tools/prologue_fix.py",
                          shell=True, capture_output=True, text=True).stdout

def probe(name, text):
    defs, body, k = [], [], 0
    for l in text.split("\n"):
        if l.startswith("\t.comm\t") or l.startswith("\t.lcomm\t"):
            d, rest = l.split("\t")[1], l.split("\t")[2]
            sym, size = rest.split(",")[:2]
            if sym in NONCOMM:
                defs += ["\t.sdata", f"{sym}:", f"\t.space\t{size}"]
            else:
                defs.append(f"\t{d}\t{sym},{size}")
        m = INS.match(l)
        if m:
            k += 1
            body += [l, f"\taddiu\t$0,$0,{k}"]
    src = defs + ["\t.text"] + body
    p = OUT / f"{name}.s"
    p.write_text("\n".join(src) + "\n")
    return p, k, [l for l in body if not l.startswith("\taddiu\t$0")]

def aspsx_decisions(p, k):
    o = p.with_suffix(".obj")
    r = subprocess.run(["bash", "tmp/research36140/aspsx.sh", str(p), str(o), "-G8"], capture_output=True, text=True)
    if r.returncode:
        raise SystemExit(f"aspsx failed on {p}: {r.stderr[:300]}")
    t = psyqobj.read_text(o.read_bytes())
    dec, cur = [], False
    for i in range(0, len(t), 4):
        w = int.from_bytes(t[i:i + 4], "little")
        if (w >> 16) == 0x2400:          # addiu $0,$0,k marker
            dec.append(cur); cur = False
        elif (w >> 26) in (0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x28, 0x29, 0x2A, 0x2B, 0x2E) and ((w >> 21) & 31) == 28:
            cur = True
    return dec

def maspsx_decisions(p):
    r = subprocess.run(f"python3 {M}/tools/maspsx/maspsx.py --aspsx-version=2.34 --use-comm-section -G8 < {p}",
                       shell=True, capture_output=True, text=True)
    dec, cur = [], False
    for l in r.stdout.split("\n"):
        if re.match(r"^\s*addiu\s+\$0,\s*\$0,\s*\d+", l):
            dec.append(cur); cur = False
        elif "%gp_rel(" in l and not l.lstrip().startswith("#"):
            cur = True
    return dec

def main():
    tus = sorted(p.stem for p in Path(M + "/src").glob("*.c"))
    report, tot, gp_n, bad = [], 0, 0, []
    for tu in tus:
        parts = []
        if tu in SPLIT:
            for part in ("p1", "p2"):
                parts.append((f"{tu}.{part}", open(f"{M}/build/src/{tu}.{part}.s").read()))
        else:
            parts.append((tu, stream(tu)))
        for name, text in parts:
            p, k, ins = probe(name, text)
            if k == 0:
                continue
            a, m = aspsx_decisions(p, k), maspsx_decisions(p)
            if len(a) != k or len(m) != k:
                report.append(f"{name}: COUNT MISMATCH aspsx={len(a)} maspsx={len(m)} expected={k}"); bad.append(name); continue
            dif = [(ins[i], a[i], m[i]) for i in range(k) if a[i] != m[i]]
            tot += k; gp_n += sum(a)
            report.append(f"{name}: {k} symbol accesses, {sum(a)} gp by ASPSX, {sum(m)} gp by POC maspsx, disagreements {len(dif)}")
            for d in dif[:10]:
                report.append(f"    {d[0].strip()}  aspsx_gp={d[1]} maspsx_gp={d[2]}")
            if dif:
                bad.append(name)
    report.append(f"TOTAL: {tot} accesses, {gp_n} gp; files with disagreements: {bad or 'none'}")
    open("tmp/q56/aspsx_wholeprog.txt", "w", newline="\n").write("\n".join(report) + "\n")
    print("\n".join(report[-40:]))


if __name__ == "__main__":
    main()
