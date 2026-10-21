#!/usr/bin/env python3
"""Shift test: relink with padding and look for address assumptions.

A moddable build must survive code and data moving. This links the already
built objects again with padding inserted at chosen points of bb2.ld and checks:

  link    each padded link succeeds with no new linker message
  alias   no linker-script absolute (a pin) shared an address with a
          definition in .main that the padding moved while the pin stayed
  absref  no relocation in a linked object resolves to an absolute symbol
          inside program RAM (0x80010000..0x80200000): such a reference
          would not follow the data it names
  escape  every relocation's target (symbol + addend) stays inside the input
          section that defines the symbol (an off-the-end offset lands in a
          neighbour that padding can separate from it)

and reports, without failing on them:

  rawptr  4-aligned words in .data/.rodata/.sdata with a program-RAM value
          and no relocation (a pointer the linker cannot move)
  rawlui  `lui` of 0x8001..0x801F with no HI16 relocation in .text

The matching build is not touched: scripts, ELFs and maps go to build/shift/,
and nothing makes an EXE. Run `make shift` (WSL, repo root, venv active), or
    python3 tools/shift_test.py [--pad 0x10] [--points text,data,rodata]
Exit 0 when no link/alias/absref/escape finding, 1 otherwise. Findings are
written to build/shift/report.tsv.
"""
import argparse
import os
import struct
import subprocess
import sys
from collections import defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pins_audit import map_sections  # noqa: E402

LD = "mipsel-linux-gnu-ld"
LD_SCRIPT = "bb2.ld"
PIN_SCRIPTS = ["undefined_funcs_auto.txt", "undefined_syms_auto.txt", "named_syms.txt"]
BASE_ELF = "build/bb2.elf"
BASE_MAP = "build/bb2.map"
OUT = "build/shift"
RAM_LO, RAM_HI = 0x80010000, 0x80200000
SHN_UNDEF, SHN_ABS, SHN_COMMON = 0, 0xFFF1, 0xFFF2
SHT_SYMTAB, SHT_REL, SHF_ALLOC = 2, 9, 2
R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16, R_MIPS_PC16 = 2, 4, 5, 6, 7, 10
POINTS = ("text", "data", "rodata")


# -- minimal ELF32 little-endian reader --------------------------------------

class Elf:
    def __init__(self, path):
        self.path = path
        with open(path, "rb") as f:
            self.data = d = f.read()
        if d[:4] != b"\x7fELF" or d[4] != 1 or d[5] != 1:
            sys.exit("shift_test: %s is not an ELF32 LE file" % path)
        shoff, = struct.unpack_from("<I", d, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from("<HHH", d, 0x2E)
        self.secs = []
        for i in range(shnum):
            f = struct.unpack_from("<10I", d, shoff + i * shentsize)
            self.secs.append(dict(name_off=f[0], type=f[1], flags=f[2], addr=f[3],
                                  offset=f[4], size=f[5], link=f[6], info=f[7]))
        strtab = self.secs[shstrndx]
        for s in self.secs:
            s["name"] = self.cstr(strtab["offset"] + s["name_off"])
        self.syms = []
        for s in self.secs:
            if s["type"] == SHT_SYMTAB:
                names = self.secs[s["link"]]["offset"]
                for off in range(s["offset"], s["offset"] + s["size"], 16):
                    nm, val, size, info, _, ndx = struct.unpack_from("<IIIBBH", d, off)
                    self.syms.append(dict(name=self.cstr(names + nm), value=val, size=size,
                                          bind=info >> 4, type=info & 0xF, shndx=ndx))

    def cstr(self, off):
        return self.data[off:self.data.index(b"\0", off)].decode("latin-1")

    def sec_bytes(self, i):
        s = self.secs[i]
        return self.data[s["offset"]:s["offset"] + s["size"]]

    def rels(self):
        """{target section index: [(offset, type, symbol index)]}"""
        out = {}
        for s in self.secs:
            if s["type"] != SHT_REL:
                continue
            lst = []
            for off in range(s["offset"], s["offset"] + s["size"], 8):
                r_off, info = struct.unpack_from("<II", self.data, off)
                lst.append((r_off, info & 0xFF, info >> 8))
            out[s["info"]] = lst
        return out


def final_symbols(elf):
    """name -> (value, is_abs) for globals of a linked ELF (locals ignored)."""
    out = {}
    for s in elf.syms:
        if s["bind"] in (1, 2) and s["shndx"] != SHN_UNDEF and s["name"]:
            out[s["name"]] = (s["value"], s["shndx"] == SHN_ABS)
    return out


# -- padded links --------------------------------------------------------------

def padded_script(lines, point, pad):
    """bb2.ld with `. += pad;` inserted at one point (or every point for 'all')."""
    def mid(sec):
        idx = [i for i, l in enumerate(lines) if l.strip().endswith(".o(%s);" % sec)]
        return idx[len(idx) // 2]
    at = {
        "text": next(i for i, l in enumerate(lines) if "main_TEXT_START = .;" in l) + 1,
        "data": mid(".data"),
        "rodata": mid(".rodata"),
    }
    wanted = POINTS if point == "all" else (point,)
    out = list(lines)
    for p in sorted(wanted, key=lambda p: -at[p]):
        out.insert(at[p], "        . += 0x%X; /* shift_test: %s */\n" % (pad, p))
    return out


def link(script, tag):
    elf, mp = "%s/%s.elf" % (OUT, tag), "%s/%s.map" % (OUT, tag)
    cmd = [LD, "-nostdlib", "--no-check-sections", "-Map", mp, "-T", script]
    for p in PIN_SCRIPTS:
        cmd += ["-T", p]
    cmd += ["-o", elf]
    r = subprocess.run(cmd, capture_output=True, text=True)
    msgs = [l for l in (r.stdout + r.stderr).splitlines() if l.strip()]
    return r.returncode, msgs, elf


# -- static reference analysis over the linked objects --------------------------

def sext16(v):
    return v - 0x10000 if v & 0x8000 else v


def reloc_addends(rels, insns):
    """Implicit (REL) addend for each relocation, HI16/LO16 paired as ld does:
    a HI16 combines with the next LO16 of the same symbol; a LO16 uses the
    most recent HI16 of its symbol."""
    adds = [None] * len(rels)
    last_hi = {}
    for i, (off, typ, sym) in enumerate(rels):
        w = insns(off)
        if typ == R_MIPS_32:
            adds[i] = w - (1 << 32) if w & 0x80000000 else w
        elif typ == R_MIPS_26:
            adds[i] = (w & 0x3FFFFFF) << 2
        elif typ == R_MIPS_GPREL16:
            adds[i] = sext16(w & 0xFFFF)
        elif typ == R_MIPS_PC16:
            adds[i] = (sext16(w & 0xFFFF) << 2) + 4  # branch target relative to the field

        elif typ == R_MIPS_HI16:
            lo = next((insns(o2) & 0xFFFF for o2, t2, s2 in rels[i + 1:]
                       if t2 == R_MIPS_LO16 and s2 == sym), 0)
            adds[i] = ((w & 0xFFFF) << 16) + sext16(lo)
            last_hi[sym] = w & 0xFFFF
        elif typ == R_MIPS_LO16:
            adds[i] = (last_hi.get(sym, 0) << 16) + sext16(w & 0xFFFF)
    return adds


def analyse_objects(objs, final):
    """absref / escape findings and rawptr / rawlui notes over every linked object."""
    elfs = {o: Elf(o) for o in objs}
    # global definitions: name -> (obj, section name, offset in section, section size)
    gdef = {}
    for o, e in elfs.items():
        for s in e.syms:
            if s["bind"] in (1, 2) and 0 < s["shndx"] < len(e.secs):
                sec = e.secs[s["shndx"]]
                gdef.setdefault(s["name"], (o, sec["name"], s["value"], sec["size"]))
    absref, escape, rawptr, rawlui = [], [], [], []
    checked = [0]
    for o, e in elfs.items():
        allrels = e.rels()
        for si, sec in enumerate(e.secs):
            if not sec["flags"] & SHF_ALLOC or sec["type"] == 8:  # skip NOBITS
                continue
            body = e.sec_bytes(si)
            rels = allrels.get(si, [])
            word = lambda off, b=body: struct.unpack_from("<I", b, off)[0]
            adds = reloc_addends(rels, word)
            covered = set()
            for (off, typ, symi), add in zip(rels, adds):
                covered.add(off)
                sym = e.syms[symi]
                where = "%s(%s)+0x%X" % (o, sec["name"], off)
                if add is None:
                    escape.append((where, "unhandled relocation type %d" % typ, sym["name"]))
                    continue
                if sym["shndx"] in (SHN_UNDEF, SHN_COMMON) or (
                        sym["bind"] in (1, 2) and sym["name"] in final
                        and final[sym["name"]][1]):
                    v = final.get(sym["name"])
                    if v and v[1] and RAM_LO <= v[0] < RAM_HI:
                        absref.append((where, "0x%08X" % v[0], sym["name"]))
                        continue
                if sym["shndx"] == SHN_ABS:
                    continue
                if sym["type"] == 3 or (0 < sym["shndx"] < len(e.secs)
                                        and not (sym["bind"] in (1, 2))):
                    tsec = e.secs[sym["shndx"]]
                    base, size, tname = sym["value"], tsec["size"], tsec["name"]
                    owner = "%s(%s)" % (o, tname)
                elif sym["name"] in gdef:
                    do, dsec, base, size = gdef[sym["name"]]
                    owner = "%s(%s)" % (do, dsec)
                else:
                    continue  # resolved outside the objects (script symbol)
                if typ in (R_MIPS_26, R_MIPS_PC16) and sym["type"] != 3:
                    continue  # jump/branch: addend is the field, target is the symbol
                checked[0] += 1
                tgt = base + add
                if typ in (R_MIPS_HI16, R_MIPS_LO16, R_MIPS_32, R_MIPS_GPREL16, R_MIPS_26, R_MIPS_PC16) \
                        and not 0 <= tgt <= size:
                    escape.append((where, "%s%+d leaves %s (size 0x%X)"
                                   % (sym["name"] or "<section>", add, owner, size),
                                   sym["name"] or tname))
            name = sec["name"]
            if name.startswith((".data", ".rodata", ".sdata")):
                for off in range(0, len(body) - 3, 4):
                    v = word(off)
                    if RAM_LO <= v < RAM_HI and off not in covered:
                        rawptr.append(("%s(%s)+0x%X" % (o, name, off), "0x%08X" % v, ""))
            elif name.startswith(".text"):
                for off in range(0, len(body) - 3, 4):
                    w = word(off)
                    if w >> 26 == 0x0F and 0x8001 <= (w & 0xFFFF) < 0x8020 and off not in covered:
                        rawlui.append(("%s(%s)+0x%X" % (o, name, off), "0x%04X" % (w & 0xFFFF), ""))
    print("shift_test: %d relocations range-checked against their defining section" % checked[0])
    return absref, escape, rawptr, rawlui


# -- main ------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--pad", type=lambda s: int(s, 0), default=0x10)
    ap.add_argument("--points", default=",".join(POINTS) + ",all")
    args = ap.parse_args()
    if not os.path.exists(BASE_ELF):
        sys.exit("shift_test: %s missing; build first" % BASE_ELF)
    os.makedirs(OUT, exist_ok=True)
    findings = []  # (check, point, where, detail, symbol)

    with open(LD_SCRIPT, encoding="utf-8") as f:
        ld_lines = f.readlines()
    # baseline relinked here so linker messages compare like with like
    with open("%s/base.ld" % OUT, "w", encoding="utf-8", newline="\n") as f:
        f.writelines(ld_lines)
    rc, base_msgs, base_elf = link("%s/base.ld" % OUT, "base")
    if rc != 0:
        sys.exit("shift_test: the unpadded relink failed:\n" + "\n".join(base_msgs))
    base = Elf(base_elf)
    with open(BASE_ELF, "rb") as f:
        if f.read() != base.data:
            print("shift_test: note: relinked base.elf differs from %s (stale build?)" % BASE_ELF)
    bsyms = final_symbols(base)
    main_sec = next(s for s in base.secs if s["name"] == ".main")
    lo, hi = main_sec["addr"], main_sec["addr"] + main_sec["size"]
    defs_at = defaultdict(list)
    for s in base.syms:
        if 0 < s["shndx"] < SHN_ABS and lo <= s["value"] < hi and s["name"] \
                and not s["name"].endswith(".NON_MATCHING") and s["type"] != 3:
            defs_at[s["value"]].append(s["name"])
    aliases = [(n, v, defs_at[v][0]) for n, (v, isabs) in sorted(bsyms.items())
               if isabs and v in defs_at]

    for point in [p.strip() for p in args.points.split(",") if p.strip()]:
        tag = "pad_" + point
        script = "%s/%s.ld" % (OUT, tag)
        with open(script, "w", encoding="utf-8", newline="\n") as f:
            f.writelines(padded_script(ld_lines, point, args.pad))
        rc, msgs, elf = link(script, tag)
        new = [m for m in msgs if m not in base_msgs]
        if rc != 0 or new:
            for m in new or ["ld exit %d" % rc]:
                findings.append(("link", point, script, m, ""))
            if rc != 0:
                continue
        ssyms = final_symbols(Elf(elf))
        for pin, v, d in aliases:
            dv = ssyms.get(d, (None,))[0]
            if dv is None:  # local definition: look it up by value shift instead
                continue
            if dv != v and ssyms[pin][0] == v:
                findings.append(("alias", point, "0x%08X" % v,
                                 "pin stayed, %s moved to 0x%08X" % (d, dv), pin))
        print("shift_test: pad 0x%X at %-6s linked (%d linker messages, %d new)"
              % (args.pad, point, len(msgs), len(new)))

    objs, _ = map_sections(BASE_MAP)
    absref, escape, rawptr, rawlui = analyse_objects(objs, bsyms)
    findings += [("absref", "-") + r for r in absref]
    findings += [("escape", "-") + r for r in escape]
    notes = [("rawptr", "-") + r for r in rawptr] + [("rawlui", "-") + r for r in rawlui]

    with open("%s/report.tsv" % OUT, "w", encoding="utf-8", newline="\n") as f:
        f.write("# generated by tools/shift_test.py (pad 0x%X)\n" % args.pad)
        f.write("check\tpoint\twhere\tdetail\tsymbol\n")
        for r in findings + notes:
            f.write("\t".join(r) + "\n")

    count = defaultdict(int)
    syms = defaultdict(set)
    for r in findings + notes:
        count[r[0]] += 1
        if r[4]:
            syms[r[0]].add(r[4])
    print("shift_test: %d linked objects, %d pin/definition aliases in the base link"
          % (len(objs), len(aliases)))
    for c in ("link", "alias", "absref", "escape", "rawptr", "rawlui"):
        extra = " (%d symbols)" % len(syms[c]) if syms[c] else ""
        kind = "note" if c in ("rawptr", "rawlui") else "FAIL" if count[c] else "ok"
        print("  %-6s %-4s %d%s" % (c, kind, count[c], extra))
    print("shift_test: report -> %s/report.tsv" % OUT)
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
