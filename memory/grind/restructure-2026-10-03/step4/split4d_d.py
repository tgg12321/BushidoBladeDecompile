#!/usr/bin/env python3
"""Restructure step 4d, rodata: distribute src/text1a_b_post_rodata.c's LIBGPU and LIBCD EVENT strings
(as committed at 60a56f5e6) to their owning module files (Q106 D4: sole referrer + position + link order).

- 0x80015D58..0x80015E28 -> src/main/psxsdk/libgpu/prim.c (DumpTPage, DumpClut, DumpDrawEnv, DumpDispEnv)
- 0x80015E28..0x8001605C -> src/main/psxsdk/libgpu/sys.c (the rcsid "$Id: sys.c,v 1.129 ..." that SYS's
  device table D_8009BE2C points at, then the SYS functions' strings)
- 0x8001605C..0x80016074 -> src/main/psxsdk/libcd/event.c (CdInit)
- the LIBCD SYS/BIOS strings 0x80016074..0x8001622C stay in text1a_b_post_rodata.c for step 4e.

The one array that straddles two modules, D_80015E1C[64] ("isrgb24 %d\\n" + the rcsid), is cut at the
module boundary into D_80015E1C[12] (PRIM) and D_80015E28[52] (SYS), byte for byte. Every other array
and its comment moves verbatim. Each receiving file's declarations of the moved symbols are replaced
by / folded into the definitions (the bodies keep their `&X` / `X` uses)."""
import os, re, sys

ROOT = os.path.dirname(os.path.abspath(__file__))
while not os.path.exists(os.path.join(ROOT, "bb2.ld")):
    ROOT = os.path.dirname(ROOT)


def rd(rel):
    t = open(os.path.join(ROOT, rel), encoding="utf-8", newline="").read()
    assert "\r" not in t and t.endswith("\n")
    return t


PR = "src/text1a_b_post_rodata.c"
pr = rd(PR)
# blocks: comment line, `const char NAME[N] =`, string lines, `    ;`
BLK = re.compile(r"(/\* (\w+): [^\n]*\*/\nconst char (\w+)\[(\d+)\] =\n(?:    \"[^\n]*\"\n)+    ;\n)")
blocks = [(m.group(3), int(m.group(4)), m.group(1), m.start(), m.end()) for m in BLK.finditer(pr)]
assert all(b[0] for b in blocks) and len(blocks) == 43, len(blocks)
names = [b[0] for b in blocks]


def cstr_bytes(block):
    """the bytes a block's string pieces spell (C escapes \\n, \\0 only)."""
    out = ""
    for piece in re.findall(r'^    "(.*)"$', block, re.M):
        out += piece
    out = out.replace("\\n", "\n").replace("\\0", "\0")
    assert "\\" not in out
    return out


addr = 0x80015D58
for n, size, blk, _, _ in blocks:
    assert f"@ 0x{addr:08X}" in blk.split("\n")[0], (n, hex(addr))
    assert len(cstr_bytes(blk)) in (size, size - 1), (n, len(cstr_bytes(blk)), size)  # [N] adds the NUL
    addr += size
assert addr == 0x8001622C

PRIM = names[:names.index("D_80015E1C") + 1]
SYS = names[names.index("D_80015E5C"):names.index("D_80016044") + 1]
EVENT = ["g_str_cdinit_fail"]
REST = names[names.index("g_str_none"):]
assert len(PRIM) == 11 and len(SYS) == 23 and len(PRIM) + len(SYS) + len(EVENT) + len(REST) == 43
by = {b[0]: b for b in blocks}

# D_80015E1C: cut at PRIM's end 0x80015E28
e1c = by["D_80015E1C"][2]
assert e1c == ('/* D_80015E1C: 2 string(s), 64B @ 0x80015E1C */\n'
               'const char D_80015E1C[64] =\n'
               '    "isrgb24 %d\\n\\0$Id: sys.c,v 1.129 1"\n'
               '    "996/12/25 03:36:20 noda Exp $\\0\\0\\0"\n'
               '    ;\n'), e1c
E1C = ('/* D_80015E1C: 1 string(s), 12B @ 0x80015E1C */\n'
       'const char D_80015E1C[12] =\n'
       '    "isrgb24 %d\\n\\0"\n'
       '    ;\n')
E28 = ('/* D_80015E28: 1 string(s), 52B @ 0x80015E28 (the rcsid; only the device table D_8009BE2C points here) */\n'
       'const char D_80015E28[52] =\n'
       '    "$Id: sys.c,v 1.129 1996/12/25 03:36:20 noda Exp $\\0\\0\\0"\n'
       '    ;\n')
assert cstr_bytes(E1C) + cstr_bytes(E28) == cstr_bytes(e1c) and len(cstr_bytes(E1C)) == 12 \
    and len(cstr_bytes(E28)) == 52


def defs(lst, head):
    out = [head]
    for n in lst:
        out.append(E1C if n == "D_80015E1C" else by[n][2])
    return "\n".join(out)


HEAD = ("/* .rodata 0x{:08X}..0x{:08X}: this module's strings (moved from src/text1a_b_post_rodata.c, Q106 D4:\n"
        " * every reader is in this file, in link order). */\n")
prim_defs = defs(PRIM, HEAD.format(0x80015D58, 0x80015E28))
sys_defs = ("/* .rodata 0x80015E28..0x8001605C: this module's strings (moved from src/text1a_b_post_rodata.c, Q106\n"
            " * D4: every C reader is in this file, in link order; the leading rcsid is referenced only by SYS's\n"
            " * device table D_8009BE2C, asm/data/7D920.data.s). */\n") + "\n" + E28 + "\n" + "\n".join(by[n][2] for n in SYS)
event_defs = defs(EVENT, HEAD.format(0x8001605C, 0x80016074))

DECL = re.compile(r"^extern (?:const char|s32|u32) (\w+)(?:\[\])?;\n", re.M)


def drop_decls(text, moved):
    hits = []
    def f(m):
        if m.group(1) in moved:
            hits.append(m.group(1))
            return ""
        return m.group(0)
    return DECL.sub(f, text), hits


out = {}
# prim.c: the "Externs for globals" block becomes the definitions
p = rd("src/main/psxsdk/libgpu/prim.c")
p2, hits = drop_decls(p, set(PRIM))
assert sorted(hits) == sorted(PRIM), hits
p2 = p2.replace("/* Externs for globals */\n\n", prim_defs + "\n")
assert prim_defs in p2
out["src/main/psxsdk/libgpu/prim.c"] = p2
# sys.c: definitions after the includes; every declaration of a moved symbol goes
s = rd("src/main/psxsdk/libgpu/sys.c")
s2, hits = drop_decls(s, set(SYS))
assert sorted(hits) == sorted(SYS), sorted(set(SYS) - set(hits))
anchor = '#include "psx.h"\n\n'
assert s2.count(anchor) == 1
s2 = s2.replace(anchor, anchor + sys_defs + "\n")
# blank-line leftovers where whole extern groups vanished
a = "extern s32 g_gpu_draw_count;\n\n\n\n/* Declarations from the file ResetGraph"
assert s2.count(a) == 1
s2 = s2.replace(a, "extern s32 g_gpu_draw_count;\n\n/* Declarations from the file ResetGraph")
out["src/main/psxsdk/libgpu/sys.c"] = s2
# event.c
e = rd("src/main/psxsdk/libcd/event.c")
e2, hits = drop_decls(e, set(EVENT))
assert hits == EVENT
anchor = '#include "common.h"\n\n'
assert e2.count(anchor) == 1
e2 = e2.replace(anchor, anchor + event_defs + "\n")
out["src/main/psxsdk/libcd/event.c"] = e2
# the data file keeps LIBCD SYS/BIOS's strings
first_rest = by[REST[0]][3]
PR_TOP = ("/* .rodata 0x80016074..0x8001622C: the LIBCD SYS and BIOS strings (the CD_comstr / CD_intstr\n"
          " * names and the CD timeout / DiskError messages), read by src/system.c. A data-only file between\n"
          " * libcd/event.o and system.o in .rodata; restructure step 4e folds it into its LIBCD modules. The\n"
          " * LIBGPU and LIBCD EVENT strings that preceded them moved to libgpu/prim.c, libgpu/sys.c and\n"
          " * libcd/event.c (step 4d, Q106 D4). */\n"
          '#include "common.h"\n\n')
out[PR] = PR_TOP + pr[first_rest:]

if "--apply" in sys.argv:
    for rel, text in out.items():
        open(os.path.join(ROOT, rel), "w", encoding="utf-8", newline="\n").write(text)
else:
    for rel, text in out.items():
        print("=====", rel)
        print(text[:3000])
