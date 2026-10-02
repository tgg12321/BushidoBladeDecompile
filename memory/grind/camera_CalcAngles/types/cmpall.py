"""cmpall.py <ref.dis> <new.dis> [comma-funcs]: order-free per-function diff of `objdump -dr` output with
relocations resolved to absolute addresses where the symbol's address is known (named_syms.txt,
undefined_syms_auto.txt, symbol_addrs.txt), so `sym+8` and `D_<addr+8>` compare equal. Section-relative
relocations compare as section+addend. Branch targets masked."""
import re, sys, difflib

addr = {}
for fn in ("undefined_syms_auto.txt", "named_syms.txt", "symbol_addrs.txt"):
    for line in open(fn, encoding="utf-8", errors="replace"):
        m = re.match(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+)", line)
        if m:
            addr.setdefault(m.group(1), int(m.group(2), 16))
            n = re.match(r"(\w+?)_([0-9A-F]{8})$", m.group(1))
            if n:
                addr.setdefault(n.group(1), int(m.group(2), 16))

def simm(v):
    v &= 0xFFFF
    return v - 0x10000 if v & 0x8000 else v

def fns(p):
    d, order, cur = {}, [], None
    lines = open(p).read().splitlines()
    i = 0
    while i < len(lines):
        l = lines[i]; i += 1
        m = re.match(r"^[0-9a-f]+ <([^>]+)>:", l)
        if m:
            cur = m.group(1); d[cur] = []; order.append(cur); continue
        if cur is None or not l.strip() or l.startswith("Disassembly"):
            continue
        if re.match(r"^\s*[0-9a-f]+: R_MIPS", l):
            continue
        s = re.sub(r"^\s*[0-9a-f]+:\s*", "", l.rstrip())
        s = re.sub(r"[0-9a-f]+ <[^>]+>", "T", s)
        # attach following relocation
        if i < len(lines):
            r = re.match(r"^\s*[0-9a-f]+: (R_MIPS_\w+)\s+(\S+)", lines[i])
            if r:
                kind, sym = r.group(1), r.group(2)
                base = addr.get(sym)
                mm = re.search(r"(-?0x[0-9a-f]+|-?\d+)(\(\w+\))?$", s)
                imm = int(mm.group(1), 0) if mm else 0
                if kind == "R_MIPS_HI16":
                    key = ("HI", hex(((base + (imm << 16)) >> 16) & 0xFFFF)) if base is not None else ("HI", sym, imm)
                elif kind in ("R_MIPS_LO16", "R_MIPS_GPREL16"):
                    key = (kind[7:], hex((base + simm(imm)) & 0xFFFFFFFF)) if base is not None else (kind[7:], sym, simm(imm))
                else:
                    key = (kind, sym)
                if mm:
                    s = s[:mm.start(1)] + "R" + s[mm.end(1):]
                s = s + "  " + str(key)
        d[cur].append(s)
    return d, [f for f in order if not f.startswith(".L")]

a, oa = fns(sys.argv[1])
b, ob = fns(sys.argv[2])
only = set(x for x in (sys.argv[3] if len(sys.argv) > 3 else "").split(",") if x)
diff = [f for f in oa if (not only or f in only) and a.get(f) != b.get(f)]
print("functions differing: %d" % len(diff))
for f in diff:
    x, y = a.get(f, []), b.get(f, [])
    print("  %s %d -> %d" % (f, len(x), len(y)))
    for l in list(difflib.unified_diff(x, y, n=0, lineterm=""))[2:16]:
        print("     ", l)
print("function order identical:", oa == ob)
