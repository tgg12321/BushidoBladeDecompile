#!/usr/bin/env python3
"""Near-verbatim library-module audit (owner ruling 2026-09-07: tier admitted, audit-gated).

For each candidate (module, claimed Sony symbol, claimed BB2 address) and EVERY acquired
LIBSND/LIBSPU build (4.0J, 4.0U, 4.1, 4.2, 4.3, 4.4):
  1. exhaustive masked placement over the WHOLE text image (not just the gaps), with the
     second-best position's mismatch as the uniqueness margin;
  2. at the best placement: every differing fully/partially-unmasked word, disassembled
     side by side (lib vs EXE) so a reviewer can judge whether the drift is a source-level
     edit (addu->subu, shift amount) or noise;
  3. XDEF/LOCAL symbol offsets -> BB2 addresses vs our function starts (bb2.map);
  4. REL26 (jal) relocations: the module's external callee name vs the name OUR tree has at
     the jal target the EXE actually encodes.
Writes tmp/near_audit/report.md + report.json (scratch; the reviewed copies live in
docs/naming/libscan/). Read-only w.r.t. the tree. Needs numpy in the WSL venv and the PsyQ
.LIB sets under tmp/libscan/psyq40 + tmp/libsnd_hunt/psyq4* (third-party, not in the repo).
"""
import glob, json, os, re, struct, subprocess, sys
import numpy as np

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))  # tools/libscan/x.py -> repo root
sys.path.insert(0, os.path.join(REPO, "tmp", "closer"))
from psyq_lib import lib_modules, parse_obj  # noqa: E402

HERE = os.path.join(REPO, "tmp", "near_audit")
EXE = os.path.join(REPO, "disc", "SLUS_006.63")
exeb = open(EXE, "rb").read()
pc, gp, taddr, tsize = struct.unpack_from("<IIII", exeb, 0x10)
body = exeb[2048:]
NW = len(body) // 4
exe = np.frombuffer(body[:NW * 4], dtype="<u4").astype(np.uint32)
def vaddr(w): return taddr + w * 4
def widx(va): return (va - taddr) // 4

VERSIONS = [("40J", os.path.join(REPO, "tmp", "libscan", "psyq40")),
            ("40U", os.path.join(REPO, "tmp", "libsnd_hunt", "psyq40u")),
            ("41", os.path.join(REPO, "tmp", "libsnd_hunt", "psyq41")),
            ("42", os.path.join(REPO, "tmp", "libsnd_hunt", "psyq42", "LIB")),
            ("43", os.path.join(REPO, "tmp", "libsnd_hunt", "psyq43")),
            ("44", os.path.join(REPO, "tmp", "libsnd_hunt", "psyq44"))]

CANDIDATES = [
    ("LIBSND", "UT_KEYV", "SsUtKeyOnV", 0x80085A40, "SsUtKeyOnV"),
    ("LIBSND", "UT_KEYV", "SsUtKeyOffV", 0x80085DD4, "(interior of SsUtKeyOnV)"),
    ("LIBSND", "VM_NOWOF", "_SsVmKeyOffNow", 0x800871D4, "_SsVmKeyOffNow"),
    ("LIBSND", "UT_VVOL", "SsUtGetDetVVol", 0x80085FD8, "func_80085FD8 (C)"),
    ("LIBSND", "UT_VVOL3", "SsUtGetVVol", 0x80086080, "func_80086080 (C)"),
    ("LIBSND", "VM_SEQ_2", "_SsVmGetSeqLVol", 0x80087D10, "func_80087D10 (C)"),
    ("LIBSND", "VM_SEQ_2", "_SsVmGetSeqRVol", 0x80087D58, "func_80087D58 (C)"),
    ("LIBSND", "UT_VVOL2", "SsUtSetDetVVol", 0x80086018, "SsUtSetDetVVol (starts 4 bytes EARLIER)"),
    ("LIBSPU", "S_SAV", "_SpuSetAnyVoice", 0x80089A54, "_SpuSetAnyVoice (starts 12 bytes EARLIER)"),
    ("LIBSND", "VM_ALOC2", "_SsVmDoAllocate", 0x800861B8, "_SsVmDoAllocate (starts 4 bytes LATER)"),
]

# ---- our names by address (link map: object-provided definitions)
MAPDEF = re.compile(r"^\s+0x0*([0-9a-f]{8})\s+([A-Za-z_]\w*)\s*$")
ours = {}
for ln in open(os.path.join(REPO, "build", "bb2.map"), encoding="utf-8", errors="replace"):
    m = MAPDEF.match(ln)
    if m:
        a = int(m.group(1), 16)
        ours.setdefault(a, []).append(m.group(2))
DATA = {}
for f in ("named_syms.txt", "symbol_addrs.txt", "undefined_syms_auto.txt", "undefined_funcs_auto.txt"):
    for ln in open(os.path.join(REPO, f), encoding="utf-8", errors="replace"):
        m = re.match(r"\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)", ln)
        if m:
            DATA.setdefault(int(m.group(2), 16), []).append(m.group(1))
def data_name(a):
    ns = DATA.get(a, [])
    real = [n for n in ns if not re.match(r"^(func|D)_[0-9A-Fa-f]{8}$", n)]
    return (real or ns or ["?"])[0]
def our_name(a):
    ns = ours.get(a, [])
    real = [n for n in ns if not re.match(r"^(func|D)_[0-9A-Fa-f]{8}$", n)]
    return (real or ns or ["?"])[0]
# function starts from asm/funcs + map
func_starts = sorted(a for a, ns in ours.items() if any(re.match(r"^(func_8|[A-Za-z_])", n) for n in ns))
def containing_func(a):
    lo = [f for f in func_starts if f <= a]
    return (lo[-1], our_name(lo[-1])) if lo else (None, "?")

def mask_for(t):
    if t in (82, 84): return 0xFFFF0000   # HI16/LO16: keep opcode+regs
    if t == 74: return 0xFC000000         # REL26: keep opcode
    if t == 16: return 0                  # FULL32
    return 0

def module_text(o):
    for sid, s in o.sections.items():
        if s["name"] == ".text" and len(s["bytes"]) >= 8:
            return sid, bytes(s["bytes"])
    return None, None

def load_module(libdir, lib, mod):
    p = os.path.join(libdir, lib + ".LIB")
    if not os.path.exists(p):
        return None
    for mn, od in lib_modules(open(p, "rb").read()):
        if mn == mod:
            return parse_obj(od)
    return None

def scan(words, masks):
    n = len(words)
    span = NW - n + 1
    mism = np.zeros(span, dtype=np.int32)
    for i in range(n):
        if masks[i] == 0:
            continue
        mism += ((exe[i:i + span] & masks[i]) != (words[i] & masks[i]))
    j = int(mism.argmin())
    lo, hi = max(0, j - n), min(span, j + n)
    m2 = mism.copy(); m2[lo:hi] = 10**9
    j2 = int(m2.argmin())
    return j, int(mism[j]), j2, int(mism[j2])

def disasm(blob, base):
    fn = os.path.join(HERE, "_tmp.bin")
    open(fn, "wb").write(blob)
    out = subprocess.run(["mipsel-linux-gnu-objdump", "-D", "-b", "binary", "-mmips:3000", "-EL",
                          "--adjust-vma=0x%x" % base, fn], capture_output=True, text=True).stdout
    res = {}
    for ln in out.splitlines():
        m = re.match(r"^\s*([0-9a-f]+):\s+([0-9a-f]{8})\s+(.*)$", ln)
        if m:
            res[int(m.group(1), 16)] = m.group(3).strip()
    return res

report = []
jsonout = []
os.makedirs(HERE, exist_ok=True)
for lib, mod, sym, claimed, ourfunc in CANDIDATES:
    sec = {"lib": lib, "mod": mod, "sym": sym, "claimed": "0x%08X" % claimed, "our": ourfunc, "versions": []}
    report.append(f"\n## {lib}/{mod} — {sym} claimed @0x{claimed:08X} ({ourfunc})\n")
    report.append("| ver | words | scored | best @ | mism | 2nd-best @ | mism | sym offset -> addr | our fn at addr |")
    report.append("|---|---:|---:|---|---:|---|---:|---|---|")
    best_rec = None
    for ver, d in VERSIONS:
        o = load_module(d, lib, mod)
        if o is None:
            report.append(f"| {ver} | - | - | (module absent) | | | | | |"); continue
        sid, data = module_text(o)
        if sid is None:
            continue
        n = len(data) // 4
        words = np.frombuffer(data[:n * 4], dtype="<u4").astype(np.uint32)
        masks = np.full(n, 0xFFFFFFFF, dtype=np.uint32)
        relocs = []
        for rsec, t, roff, e in o.relocs:
            if rsec != sid or roff // 4 >= n:
                continue
            masks[roff // 4] &= np.uint32(mask_for(t))
            relocs.append((roff // 4, t, e))
        j, m, j2, m2 = scan(words, masks)
        scored = int((masks != 0).sum())
        base = vaddr(j)
        syms = [(nm, off, "XDEF") for nm, s2, off in o.xdefs if s2 == sid] + \
               [(nm, off, "local") for nm, s2, off in o.locals if s2 == sid]
        symtxt = []; ourtxt = []
        for nm, off, kind in sorted(syms, key=lambda x: x[1]):
            a = base + off
            symtxt.append(f"{nm}{'(static)' if kind=='local' else ''}+0x{off:X}->0x{a:08X}")
            fa, fn = containing_func(a)
            ourtxt.append(f"{fn}{'' if fa == a else ' (+0x%X into it)' % (a - fa)}")
        rec = dict(ver=ver, words=n, scored=scored, best="0x%08X" % base, mism=m,
                   second="0x%08X" % vaddr(j2), mism2=m2, syms=symtxt, ours=ourtxt)
        sec["versions"].append(rec)
        report.append(f"| {ver} | {n} | {scored} | 0x{base:08X} | {m} | 0x{vaddr(j2):08X} | {m2} | "
                      f"{'; '.join(symtxt)} | {'; '.join(ourtxt)} |")
        if best_rec is None or m / max(scored, 1) < best_rec[0]:
            best_rec = (m / max(scored, 1), ver, words, masks, relocs, j, o, sid, n)
    if best_rec is None:
        continue
    ratio, ver, words, masks, relocs, j, o, sid, n = best_rec
    base = vaddr(j)
    exe_span = body[j * 4:(j + n) * 4]
    lib_dis = disasm(words.astype("<u4").tobytes(), base)
    exe_dis = disasm(exe_span, base)
    report.append(f"\n**Best build {ver} @0x{base:08X}, mismatch {int(sum(((exe[j:j+n] & masks) != (words & masks)) & (masks != 0)))}/{int((masks != 0).sum())} scored words. Differing words (lib | EXE):**\n")
    report.append("| addr | lib word | lib insn | EXE word | EXE insn |")
    report.append("|---|---|---|---|---|")
    diffs = []
    for i in range(n):
        if masks[i] == 0:
            continue
        if (int(exe[j + i]) & int(masks[i])) != (int(words[i]) & int(masks[i])):
            a = base + i * 4
            diffs.append(dict(addr="0x%08X" % a, lib="%08X" % int(words[i]), exe="%08X" % int(exe[j + i]),
                              lib_insn=lib_dis.get(a, "?"), exe_insn=exe_dis.get(a, "?")))
            report.append(f"| 0x{a:08X} | {int(words[i]):08X} | `{lib_dis.get(a,'?')}` | {int(exe[j+i]):08X} | `{exe_dis.get(a,'?')}` |")
    sec["best"] = dict(ver=ver, base="0x%08X" % base, diffs=diffs)
    # callee cross-check
    report.append(f"\n**REL26 callees at the {ver} placement (module says -> EXE jal target -> our name):**\n")
    callees = []
    for wi, t, e in relocs:
        if t != 74 or "secbase" in e or "secstart" in e:
            continue
        e = re.sub(r"sym:(\d+)", lambda m: "sym:" + str(o.xrefs.get(int(m.group(1)), m.group(1))), e)
        w = int(exe[j + wi]); a = base + wi * 4
        tgt = ((w & 0x3FFFFFF) << 2) | (a & 0xF0000000)
        nm = our_name(tgt)
        ok = e.replace("sym:", "") == nm
        callees.append(dict(at="0x%08X" % a, expects=e, target="0x%08X" % tgt, ours=nm, agree=ok))
        report.append(f"- 0x{a:08X}: module `{e}` -> EXE jal 0x{tgt:08X} = **{nm}** {'OK' if ok else 'MISMATCH'}")
    sec["best"]["callees"] = callees
    # HI16/LO16 data refs (informative)
    hi = {}
    seen = set()
    for wi, t, e in relocs:
        e = re.sub(r"sym:(\d+)", lambda m: "sym:" + str(o.xrefs.get(int(m.group(1)), m.group(1))), e)
        if t == 82:
            hi[e] = int(exe[j + wi]) & 0xFFFF
        elif t == 84 and e in hi:
            lo = int(exe[j + wi]) & 0xFFFF
            lo = lo - 0x10000 if lo >= 0x8000 else lo
            a = ((hi[e] << 16) + lo) & 0xFFFFFFFF
            if (e, a) in seen:
                continue
            seen.add((e, a))
            report.append(f"- data ref `{e}` -> EXE hi/lo 0x{a:08X} = {data_name(a)}")
    jsonout.append(sec)

open(os.path.join(HERE, "report.md"), "w").write("# Near-verbatim audit — " + os.path.basename(EXE) + "\n" + "\n".join(report) + "\n")
json.dump(jsonout, open(os.path.join(HERE, "report.json"), "w"), indent=1)
print("wrote", os.path.join(HERE, "report.md"))
