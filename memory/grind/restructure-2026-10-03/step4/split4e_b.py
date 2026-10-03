#!/usr/bin/env python3
"""Restructure step 4e, rodata: fold src/text1a_b_post_rodata.c (LIBCD SYS/BIOS strings 0x80016074..0x8001622C)
and src/text1a_b_tail_rodata.c (0x80016240..0x800163C0) into their owning module files (Q106 D4: sole
referrer + contiguity + link order), as committed at c13e49ca8; both data files are deleted.

- 0x80016074..0x8001607C -> libcd/sys.c      g_str_none (CdComstr / CdIntstr)
- 0x8001607C..0x8001622C -> libcd/bios.c     block A, before getintr: the CD_comstr / CD_intstr names (read
                                             through those .data tables), get_alarm's and getintr's strings
- 0x8001622C..0x80016240    libcd/bios.c     getintr's jump table (offset 0x1B0 in bios.o: 8-aligned)
- 0x80016240..0x800162CC -> libcd/bios.c     block B, right after getintr: CD_sync .. CD_datasync's strings
                                             and the rcsid "$Id: bios.c,v 1.86 ..." (inside D_8001626C)
- 0x800162CC..0x800162D4 -> libc2/puts.c     "<NULL>"
- 0x800162D4..0x80016318 -> libcd/cdread.c   cb_read's and cd_read_retry's strings
- 0x80016318..0x80016328 -> libetc/vsync.c   v_wait's "VSync: timeout\\n"
- 0x80016328..0x80016394 -> libetc/intr.c    the rcsid "$Id: intr.c,v 1.76 ..." (pointed at by INTR's callbacks
                                             table, asm/data/91C98.data.s:1271), trapIntr's two strings
- 0x80016394..0x800163C0 -> libetc/intr_dma.c trapIntrDMA's two strings

The one array spanning a module boundary, D_80016318[68] ("VSync: timeout\\n\\0" + the intr.c rcsid), is cut
at VSYNC|INTR into D_80016318[16] and D_80016328[52], byte for byte. Every other array and its comment moves
verbatim; each receiving file's declarations of the moved symbols are dropped (the bodies keep their uses).
bios.c's and intr.c's "split from" headers, which named the file itself, are reworded."""
import os
import re
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
while not os.path.exists(os.path.join(ROOT, "bb2.ld")):
    ROOT = os.path.dirname(ROOT)


def rd(rel):  # as committed at c13e49ca8 (re-runnable after --apply)
    import subprocess
    t = subprocess.run(["git", "-C", ROOT, "show", f"c13e49ca8:{rel}"], capture_output=True,
                       check=True).stdout.decode("utf-8")
    assert "\r" not in t and t.endswith("\n")
    return t


BLK = re.compile(r"(/\* (\w+): [^\n]*\*/\nconst char (\w+)\[(\d+)\] =\n(?:    \"[^\n]*\"\n)+    ;\n)")


def blocks(text):
    return [(m.group(3), int(m.group(4)), m.group(1)) for m in BLK.finditer(text)]


def cstr_bytes(block):
    out = "".join(re.findall(r'^    "(.*)"$', block, re.M))
    out = out.replace("\\n", "\n").replace("\\0", "\0")
    assert "\\" not in out
    return out


PR, TR = "src/text1a_b_post_rodata.c", "src/text1a_b_tail_rodata.c"
pre, tail = blocks(rd(PR)), blocks(rd(TR))
assert len(pre) == 8 and len(tail) == 17, (len(pre), len(tail))
for lst, addr, end in ((pre, 0x80016074, 0x8001622C), (tail, 0x80016240, 0x800163C0)):
    for n, size, blk in lst:
        assert f"@ 0x{addr:08X}" in blk.split("\n")[0], (n, hex(addr))
        assert len(cstr_bytes(blk)) in (size, size - 1), (n, len(cstr_bytes(blk)), size)
        addr += size
    assert addr == end, hex(addr)
by = {n: blk for n, _, blk in pre + tail}
names = [n for n, _, _ in pre + tail]


def span(a, b):
    return names[names.index(a):names.index(b) + 1]


SYS = ["g_str_none"]
BIOS_A = span("D_8001607C", "D_80016220")
BIOS_B = span("D_80016240", "D_800162C0")
PUTS = ["D_800162CC"]
CDREAD = span("D_800162D4", "D_80016304")
INTR = ["D_8001635C", "D_80016378"]
DMA = ["D_80016394", "D_800163B0"]
assert len(SYS + BIOS_A + BIOS_B + PUTS + CDREAD + ["D_80016318"] + INTR + DMA) == 25

e318 = by["D_80016318"]
assert e318 == ('/* D_80016318: 2 string(s), 68B @ 0x80016318 */\n'
                'const char D_80016318[68] =\n'
                '    "VSync: timeout\\n\\0$Id: intr.c,v 1."\n'
                '    "76 1997/02/12 12:45:05 makoto Ex"\n'
                '    "p $\\0"\n'
                '    ;\n'), e318
E318 = ('/* D_80016318: 1 string(s), 16B @ 0x80016318 */\n'
        'const char D_80016318[16] =\n'
        '    "VSync: timeout\\n\\0"\n'
        '    ;\n')
E328 = ('/* D_80016328: 1 string(s), 52B @ 0x80016328 (the rcsid; only INTR\'s callbacks table, '
        'asm/data/91C98.data.s:1271, points here) */\n'
        'const char D_80016328[52] =\n'
        '    "$Id: intr.c,v 1.76 1997/02/12 12:45:05 makoto Exp $\\0"\n'
        '    ;\n')
assert cstr_bytes(E318) + cstr_bytes(E328) == cstr_bytes(e318)
assert len(cstr_bytes(E318)) == 16 and len(cstr_bytes(E328)) == 52  # both exact fits, as the [68] original


def head(a, b, src, why):
    return wrap(f"/* .rodata 0x{a:08X}..0x{b:08X}: {why} (moved from {src}, Q106 D4: every reader is in this "
                f"file, in link order). */") + "\n"


def wrap(text):
    out, cur = [], ""
    for w in text.split(" "):
        if len(cur) + len(w) + 1 > 100:
            out.append(cur)
            cur = " *"
        cur = (cur + " " + w) if cur else w
    return "\n".join(out + [cur])


def defs(lst):
    return "\n".join(by[n] for n in lst)


DECL = re.compile(r"^extern (?:const char|char|s32|u8|void) (\w+)(?:\[\])?;[^\n]*\n", re.M)


def drop_decls(text, moved):
    hits = []

    def f(m):
        if m.group(1) in moved:
            hits.append(m.group(1))
            return ""
        return m.group(0)
    return DECL.sub(f, text), hits


def put_after(text, anchor, ins):
    assert text.count(anchor) == 1, anchor
    return text.replace(anchor, anchor + ins)


out = {}
INC_C = '#include "common.h"\n\n'

# sys.c
t = rd("src/main/psxsdk/libcd/sys.c")
t, h = drop_decls(t, set(SYS)); assert h == SYS, h
t = put_after(t, '#include "libcd.h"\n\n',
              head(0x80016074, 0x8001607C, PR, "CdComstr's and CdIntstr's out-of-range name") + "\n" + defs(SYS) + "\n")
out["src/main/psxsdk/libcd/sys.c"] = t

# bios.c
t = rd("src/main/psxsdk/libcd/bios.c")
t, h = drop_decls(t, set(BIOS_A + BIOS_B))
assert sorted(h) == sorted(["D_800161E4", "D_800161F0", "D_8001620C", "D_80016220", "D_80016240", "D_800161B8",
                            "D_800161C8", "D_80016248", "D_80016254", "D_8001625C", "D_8001626C", "D_800162A8",
                            "D_800162B4", "D_800161C8", "D_800162C0"]), sorted(h)
old_from = "/* Declarations from the file this module was split from (src/main/psxsdk/libcd/bios.c, ex system.c). */\n"
new_from = "/* Declarations from the head of the old system.c (now in libcd/sys.c) that this module uses. */\n"
assert t.count(old_from) == 1
t = t.replace(old_from, new_from)
A =(wrap("/* .rodata 0x8001607C..0x8001622C: the module's strings in front of getintr's jump table "
          "(0x8001622C..0x80016240, emitted with getintr below): the CD_comstr / CD_intstr command and "
          "interrupt names (read through those .data tables, asm/data/7D920.data.s), then get_alarm's and "
          "getintr's messages (moved from src/text1a_b_post_rodata.c, Q106 D4: every C reader is in this "
          "file, in link order). */") + "\n\n" + defs(BIOS_A) + "\n")
t = put_after(t, '#include "libcd.h"\n\n', A)
B = ("\n" + wrap("/* .rodata 0x80016240..0x800162CC: the rest of the module's strings, after getintr's jump "
                 "table: CD_sync, CD_ready, CD_cw (with the rcsid \"$Id: bios.c,v 1.86 ...\", which the "
                 "module's .data block D_800A1498 points at), CD_init and CD_datasync's (moved from "
                 "src/text1a_b_tail_rodata.c, Q106 D4: every C reader is in this file, in link order). */")
     + "\n\n" + defs(BIOS_B) + "\n")
anchor = "        printf(D_80016220, nReg);\n        return 0;\n    }\n}\n"
t = put_after(t, anchor, B)
out["src/main/psxsdk/libcd/bios.c"] = t

# puts.c
t = rd("src/main/psxsdk/libc2/puts.c")
t, h = drop_decls(t, set(PUTS)); assert h == PUTS, h
t = put_after(t, INC_C, head(0x800162CC, 0x800162D4, TR, "puts's NULL-pointer text") + "\n" + defs(PUTS) + "\n")
out["src/main/psxsdk/libc2/puts.c"] = t

# cdread.c
t = rd("src/main/psxsdk/libcd/cdread.c")
t, h = drop_decls(t, set(CDREAD)); assert sorted(h) == sorted(CDREAD), h
t = put_after(t, '#include "libcd.h"\n\n',
              head(0x800162D4, 0x80016318, TR, "cb_read's and cd_read_retry's messages") + "\n" + defs(CDREAD) + "\n")
out["src/main/psxsdk/libcd/cdread.c"] = t

# vsync.c
t = rd("src/main/psxsdk/libetc/vsync.c")
t, h = drop_decls(t, {"D_80016318"}); assert h == ["D_80016318"], h
t = put_after(t, INC_C, head(0x80016318, 0x80016328, TR, "v_wait's timeout message") + "\n" + E318 + "\n")
out["src/main/psxsdk/libetc/vsync.c"] = t

# intr.c
t = rd("src/main/psxsdk/libetc/intr.c")
t, h = drop_decls(t, set(INTR)); assert sorted(h) == sorted(INTR), h
old_from = "/* Declarations from the file this module was split from (src/main/psxsdk/libetc/intr.c, ex ings2.c). */\n"
new_from = "/* Declarations from the old ings2.c (its head and its VSYNC module) that this module uses. */\n"
assert t.count(old_from) == 1
t = t.replace(old_from, new_from)
t = put_after(t, INC_C, wrap("/* .rodata 0x80016328..0x80016394: the module's rcsid \"$Id: intr.c,v 1.76 ...\" "
                             "(pointed at only by INTR's callbacks table, asm/data/91C98.data.s:1271) and trapIntr's two messages (moved "
                             "from src/text1a_b_tail_rodata.c, Q106 D4: every C reader is in this file, in "
                             "link order). */") + "\n\n" + E328 + "\n" + defs(INTR) + "\n")
out["src/main/psxsdk/libetc/intr.c"] = t

# intr_dma.c
t = rd("src/main/psxsdk/libetc/intr_dma.c")
t, h = drop_decls(t, set(DMA)); assert sorted(h) == sorted(DMA), h
t = put_after(t, INC_C, head(0x80016394, 0x800163C0, TR, "trapIntrDMA's bus-error report") + "\n" + defs(DMA) + "\n")
out["src/main/psxsdk/libetc/intr_dma.c"] = t

if "--apply" in sys.argv:
    for rel, text in out.items():
        open(os.path.join(ROOT, rel), "w", encoding="utf-8", newline="\n").write(text)
    for rel in (PR, TR):
        os.remove(os.path.join(ROOT, rel))
else:
    for rel, text in out.items():
        print("=====", rel)
        print(text[:2600])
