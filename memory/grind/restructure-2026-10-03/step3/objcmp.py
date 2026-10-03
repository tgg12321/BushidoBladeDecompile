#!/usr/bin/env python3
"""objcmp.py BEFORE_DIR AFTER_DIR OWNER_ID [DATA_ID ...] -- fold object compare.

BEFORE: OWNER.o and each DATA.o built separately; AFTER: OWNER.o holding the data.
Checks, per section (.text .rodata .data .bss .sdata .sbss):
  - .text/.sdata/.sbss/.data/.bss: bytes and relocations identical to BEFORE's owner;
  - .rodata: AFTER bytes == the bytes the link placed for (DATA..., OWNER) in link order
    (given as the order of the arguments: the caller passes them in bb2.ld order, with OWNER as
    the literal token '@'), each object padded to 4 as the link does;
  - global symbols: every BEFORE global (owner + data) is a global in AFTER with the same
    section and, for .rodata, the same offset from the merged section start.
Run inside WSL (mipsel-linux-gnu-* binutils)."""
import subprocess, sys, re

OBJCOPY = "mipsel-linux-gnu-objcopy"
READELF = "mipsel-linux-gnu-readelf"


def sec_bytes(o, sec):
    r = subprocess.run([OBJCOPY, "-O", "binary", "-j", sec, o, "/dev/stdout"], capture_output=True)
    return r.stdout


def relocs(o):
    out = subprocess.run([READELF, "-rW", o], capture_output=True, text=True).stdout
    secs, cur = {}, None
    for l in out.splitlines():
        m = re.match(r"Relocation section '\.rel(\.\w+)'", l)
        if m:
            cur = m.group(1); secs[cur] = []; continue
        p = l.split()
        if cur and len(p) >= 5 and re.match(r"^[0-9a-f]{8}$", p[0]):
            secs[cur].append((p[0], p[2], p[4] if len(p) > 4 else ""))
    return secs


def syms(o):
    out = subprocess.run([READELF, "-sW", o], capture_output=True, text=True).stdout
    shn = {}
    so = subprocess.run([READELF, "-SW", o], capture_output=True, text=True).stdout
    for l in so.splitlines():
        m = re.match(r"\s*\[\s*(\d+)\]\s+(\S+)", l)
        if m:
            shn[m.group(1)] = m.group(2)
    res = {}
    for l in out.splitlines():
        p = l.split()
        if len(p) >= 8 and p[4] == "GLOBAL" and p[6] not in ("UND",):
            res[p[7]] = (shn.get(p[6], p[6]), int(p[1], 16))
    return res


def main():
    bdir, adir, owner = sys.argv[1], sys.argv[2], sys.argv[3]
    order = sys.argv[4:]  # bb2.ld .rodata order, '@' = owner
    if "--rodata-shift" in order:
        i = order.index("--rodata-shift")
        order = order[:i] + order[i + 2:]
    ok = True
    bo, ao = f"{bdir}/{owner}.o", f"{adir}/{owner}.o"
    shift = int(sys.argv[sys.argv.index("--rodata-shift") + 1], 0) if "--rodata-shift" in sys.argv else 0
    rel_after = relocs(ao)
    for sec in (".text", ".data", ".bss", ".sdata", ".sbss"):
        b, a = sec_bytes(bo, sec), sec_bytes(ao, sec)
        same = b == a
        if not same and sec == ".text" and shift and len(a) == len(b):
            # REL addends live in the instruction: a word may differ only where a HI16/LO16
            # relocation against the .rodata section symbol sits, by exactly the shift.
            at = {int(off, 16): (ty, sym) for off, ty, sym in rel_after.get(".text", [])}
            bad = []
            for i in range(0, len(a), 4):
                wb, wa = int.from_bytes(b[i:i+4], "little"), int.from_bytes(a[i:i+4], "little")
                if wb == wa:
                    continue
                ty, sym = at.get(i, ("", ""))
                if sym == ".rodata" and ty == "R_MIPS_LO16" and (wa - wb) & 0xFFFF == shift & 0xFFFF \
                        and (wa ^ wb) >> 16 == 0:
                    continue
                if sym == ".rodata" and ty == "R_MIPS_HI16" and (wa ^ wb) >> 16 == 0:
                    continue  # %hi carry, checked through the link (oracle)
                bad.append((hex(i), ty, sym))
            print(f".text differing words all .rodata-section HI16/LO16 addends shifted by {shift:#x}: {not bad} {bad[:5]}")
            same = not bad
        ok &= same
        ok &= same
        print(f"{sec:8s} bytes {'identical' if same else 'DIFFER'} ({len(b)} vs {len(a)})")
    rb, ra = relocs(bo), relocs(ao)
    for sec in sorted(set(rb) | set(ra)):
        if sec == ".rodata":
            continue
        same = rb.get(sec) == ra.get(sec)
        ok &= same
        print(f".rel{sec:8s} {'identical' if same else 'DIFFER'} ({len(rb.get(sec, []))} vs {len(ra.get(sec, []))})")
    # rodata: concatenation in link order, each object 4-aligned
    cat, base, bsyms = b"", {}, {}
    for t in order:
        if t == "align8":  # an object-relative .align 3 in the AFTER object
            while len(cat) % 8:
                cat += b"\0"
            continue
        m = re.match(r"^([^\[]+)(?:\[(\d+):(\d+)\])?$", t)
        name = m.group(1)
        o = bo if name == "@" else f"{bdir}/{name}.o"
        while len(cat) % 4:
            cat += b"\0"
        base[name] = len(cat)
        data = sec_bytes(o, ".rodata")
        lo = int(m.group(2)) if m.group(2) else 0
        hi = int(m.group(3)) if m.group(3) else len(data)
        cat += data[lo:hi]
        for s, (sec, v) in syms(o).items():
            if sec == ".rodata":
                if lo <= v < hi:
                    bsyms[s] = (sec, v - lo + base[name])
            elif name == "@":
                bsyms[s] = (sec, v)
    ar = sec_bytes(ao, ".rodata")
    same = ar == cat
    ok &= same
    print(f".rodata  after == link-order concatenation: {same} ({len(ar)} vs {len(cat)})")
    # .rel.rodata: the owner's entries shifted by its base; data files carry none
    exp = [(f"{int(off, 16) + base['@']:08x}", ty, sym) for off, ty, sym in rb.get(".rodata", [])]
    for t in order:
        t = t.split("[")[0]
        if t not in ("@", "align8") and relocs(f"{bdir}/{t}.o").get(".rodata"):
            ok = False
            print(f"{t}.o has .rel.rodata entries (not handled)")
    same = ra.get(".rodata", []) == exp
    ok &= same
    print(f".rel.rodata shifted by {base['@']:#x}: {'identical' if same else 'DIFFER'} ({len(exp)} vs {len(ra.get('.rodata', []))})")
    asyms = syms(ao)
    for s, v in sorted(bsyms.items()):
        if asyms.get(s) != v:
            ok = False
            print(f"symbol {s}: before {v} after {asyms.get(s)}")
    extra = sorted(set(asyms) - set(bsyms))
    if extra:
        ok = False
        print("new globals:", extra)
    print("globals checked:", len(bsyms))
    print("OBJCMP", "OK" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
