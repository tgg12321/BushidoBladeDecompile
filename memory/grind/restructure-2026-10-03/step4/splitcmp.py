#!/usr/bin/env python3
"""splitcmp.py BEFORE.o[+BEFORE2.o...] AFTER1.o [AFTER2.o ...] -- split object compare (run in WSL).

BEFORE is the unsplit object; AFTER objects are its parts in bb2.ld link order.
For every allocated section (.text .rodata .data .bss .sdata .sbss): the AFTER parts'
sections concatenated in order (each 4-aligned; all part sizes here are multiples of 4)
must equal BEFORE's bytes; every BEFORE relocation must appear at the same concatenated
offset with the same type and symbol (a section-symbol relocation is resolved to the
section and checked through the bytes + oracle); every BEFORE global symbol must be
defined in exactly one part at the same concatenated offset and section; COMMON symbols
must keep size/alignment. Prints per-part spans."""
import re, subprocess, sys

OBJCOPY, READELF = "mipsel-linux-gnu-objcopy", "mipsel-linux-gnu-readelf"
SECS = (".text", ".rodata", ".data", ".bss", ".sdata", ".sbss")


def run(*a):
    return subprocess.run(a, capture_output=True, text=True).stdout


def shdrs(o):
    out = {}
    for l in run(READELF, "-SW", o).splitlines():
        m = re.match(r"\s*\[\s*(\d+)\]\s+(\S+)\s+\S+\s+[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)", l)
        if m:
            out[m.group(2)] = (int(m.group(1)), int(m.group(3), 16))
    return out


def sec_bytes(o, sec, size):
    if sec in (".bss", ".sbss"):
        return b"\0" * size
    return subprocess.run([OBJCOPY, "-O", "binary", "-j", sec, o, "/dev/stdout"], capture_output=True).stdout


def relocs(o):
    secs, cur = {}, None
    for l in run(READELF, "-rW", o).splitlines():
        m = re.match(r"Relocation section '\.rel(\.\w+)'", l)
        if m:
            cur = m.group(1); secs[cur] = []; continue
        p = l.split()
        if cur and len(p) >= 4 and re.match(r"^[0-9a-f]{8}$", p[0]):
            secs[cur].append((int(p[0], 16), p[2], p[4] if len(p) > 4 else p[3]))
    return secs


def syms(o):
    idx = {str(i): n for n, (i, _s) in shdrs(o).items()}
    res = {}
    for l in run(READELF, "-sW", o).splitlines():
        p = l.split()
        if len(p) >= 8 and p[4] == "GLOBAL" and p[6] != "UND":
            sec = "COMMON" if p[6] == "COM" else idx.get(p[6], p[6])
            res[p[7]] = (sec, int(p[1], 16), int(p[2]))
    return res


def concat(objs):
    """sections, relocations and globals of OBJS laid end to end (each part 4-aligned)."""
    base, data = {}, {}
    for sec in SECS:
        cat, off = b"", {}
        for p in objs:
            size = shdrs(p).get(sec, (0, 0))[1]
            while len(cat) % 4:
                cat += bytes(1)
            off[p] = len(cat)
            cat += sec_bytes(p, sec, size)
        base[sec], data[sec] = off, cat
    rel = {}
    for p in objs:
        for sec, lst in relocs(p).items():
            if sec in SECS:
                rel.setdefault(sec, []).extend((o + base[sec][p], t, s) for o, t, s in lst)
    sy, dup = {}, []
    for p in objs:
        for s, (sec, v, sz) in syms(p).items():
            if s in sy:
                dup.append(s)
            sy[s] = (sec, v + (base[sec][p] if sec in base else 0), sz, p)
    return base, data, rel, sy, dup


def main():
    """BEFORE may be several objects joined with '+' (a re-cut across an old boundary)."""
    befores, parts = sys.argv[1].split("+"), sys.argv[2:]
    ok = True
    bbase, bdata, brel, bsy, bdup = concat(befores)
    base, adata, arel, asy, adup = concat(parts)
    for d in bdup + adup:
        ok = False
        print("defined twice:", d)
    allp = befores + parts
    prel = {p: {o: (t, s) for o, t, s in relocs(p).get(".text", [])} for p in allp}
    for sec in SECS:
        old, cat = bdata[sec], adata[sec]
        same = old == cat
        note = ""
        if not same and len(cat) == len(old) and sec == ".text":
            # REL addends live in the instruction: a J/JAL against the .text SECTION symbol
            # encodes the object-relative target, so it shifts by the part's base.
            def owner(i, bb):  # the part whose non-empty .text holds offset i
                return [q for b0, q in sorted((b0, q) for q, b0 in bb.items()
                                              if shdrs(q).get(sec, (0, 0))[1]) if b0 <= i][-1]
            bad, shifted = [], 0
            for i in range(0, len(old), 4):
                if old[i:i+4] == cat[i:i+4]:
                    continue
                pa, pb = owner(i, base[sec]), owner(i, bbase[sec])
                ta, sa = prel[pa].get(i - base[sec][pa], ("", ""))
                tb, sb = prel[pb].get(i - bbase[sec][pb], ("", ""))
                wo, wn = int.from_bytes(old[i:i+4], "little"), int.from_bytes(cat[i:i+4], "little")
                if ta == tb == "R_MIPS_26" and sa == sb == ".text" and wo >> 26 == wn >> 26                         and ((wo & 0x3FFFFFF) - (wn & 0x3FFFFFF)) * 4 == base[sec][pa] - bbase[sec][pb]:
                    shifted += 1
                    continue
                bad.append((hex(i), pa, ta, sa))
            same = not bad
            note = f"; {shifted} J/JAL .text-section addends re-based by their part's offset, others: {bad[:5]}"
        ok &= same
        print(f"{sec:8s} {'identical' if same else 'DIFFER'} ({len(old):#x} vs {len(cat):#x}){note}")
    for sec in sorted(set(brel) | set(arel)):
        bl, al = sorted(brel.get(sec, [])), sorted(arel.get(sec, []))
        same = bl == al
        if not same:
            diff = [x for x in bl if x not in al][:6], [x for x in al if x not in bl][:6]
            print(f".rel{sec} DIFFER ({len(bl)} vs {len(al)}): before-only {diff[0]} after-only {diff[1]}")
        else:
            print(f".rel{sec:8s} identical ({len(bl)})")
        ok &= same
    for s, (sec, v, sz, _p) in sorted(bsy.items()):
        a = asy.get(s)
        if not a or a[:3] != (sec, v, sz) and not (sec == "COMMON" and a[0] == "COMMON" and a[2] == sz):
            ok = False
            print(f"symbol {s}: before {(sec, v, sz)} after {a}")
    extra = sorted(set(asy) - set(bsy))
    if extra:
        ok = False
        print("new globals:", extra)
    print("globals checked:", len(bsy))
    for p in parts:
        h = shdrs(p)
        nz = {s: h[s][1] for s in SECS if h.get(s, (0, 0))[1]}
        print(f"  {p}: " + " ".join(f"{s}+{base[s][p]:#x}/{n:#x}" for s, n in nz.items()))
    print("SPLITCMP", "OK" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
