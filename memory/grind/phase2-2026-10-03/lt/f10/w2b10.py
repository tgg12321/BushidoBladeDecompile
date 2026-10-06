#!/usr/bin/env python3
# Worker-2 batch 10: F10, 31548 func_80040D48's frame reads. arg4 (s1) is the s16 channel array of one decoded
# motion frame (3AB48 func_8005490C passes its s16 frame[0x42]): triple k at element 3k, the per-node rotations
# at D_80094CFC's indices, triples 0x12 / 0x13 / 0x14 / 0x15 the two extra nodes' positions and angles. The
# byte-offset views become element reads; no prototype or caller changes.
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/f10/w2b10.py [out=DIR | apply]
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f09"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import w2b2 as B2
sys.argv = _argv
NL = chr(10)
BASE = "4dea4ed42"   # batch 9 on p2/w2 (main 9462a0204)
rep, fn = B2.rep, B2.fn


def base(p):
    B2.BASE = BASE
    return B2.show(p)


def d48(b):
    b = rep(b, "*(u16 *)((u8 *)s1 + idx * 6)", "s1[idx * 3]")
    b = rep(b, "-(s16)*(u16 *)((u8 *)s1 + idx * 6 + 2)", "-s1[idx * 3 + 1]")
    b = rep(b, "-(s16)*(u16 *)((u8 *)s1 + idx * 6 + 4)", "-s1[idx * 3 + 2]")
    n = len(re.findall(r"\(\(u8 \*\)s1 \+ 0x", b))
    b = re.sub(r"-\(s16\)\*\(u16 \*\)\(\(u8 \*\)s1 \+ (0x[0-9A-F]+)\)", lambda m: "-s1[0x%X]" % (int(m.group(1), 16) // 2), b)
    b = re.sub(r"-\(s32\)\*\(s16 \*\)\(\(u8 \*\)s1 \+ (0x[0-9A-F]+)\)", lambda m: "-s1[0x%X]" % (int(m.group(1), 16) // 2), b)
    b = re.sub(r"\*\((?:s16|u16) \*\)\(\(u8 \*\)s1 \+ (0x[0-9A-F]+)\)", lambda m: "s1[0x%X]" % (int(m.group(1), 16) // 2), b)
    if n != 12 or "(u8 *)s1" in b:
        raise SystemExit("func_80040D48: %d fixed-offset reads, leftover %s" % (n, "(u8 *)s1" in b))
    return b


def c31548(s):
    return fn(s, "func_80040D48", d48)


FILES = [("src/main/31548.c", c31548)]


def main():
    apply = "apply" in sys.argv[1:]
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or ["tmp/w2/b10"])[0]
    for p, g in FILES:
        s = g(base(p))
        dst = p if apply else out + "/" + p
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "w", encoding="utf-8", newline=NL).write(s)
    print("w2b10 wrote %d files to %s" % (len(FILES), "the tree" if apply else out))


if __name__ == "__main__":
    main()
