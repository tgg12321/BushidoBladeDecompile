#!/usr/bin/env python3
"""land.py [--rows keep|retire] : apply the func_80021424 merge completion (run ONLY while holding the landing lock).

Edits (all LF, all asserted against the exact current text):
  include/code6cac.h   Tbl800A3860Entry gains u16 f16 + u16 f18[27] (pad16 shrinks to pad00's tail);
                       the six per-word externs (D_800A3864, D_801027B4/B8/BC/C0/D4) are deleted.
  src/code6cac_tu2.c   ten consumers respelled through D_801027B0[ch][k] / D_800A3860[ch]->field
                       (bodies from tmp/func_80021424/<func>.m.c); D_800100E0 (alignment pad that `.align 3`
                       regenerates) deleted; D_80010428 spelled as one const word with its evidence comment.
  src/code6cac_c2.c    func_8003CF84's two reads respelled.
  undefined_syms_auto.txt  D_801027B4/B8/BC/C0 rows suffixed as aliases (func_80020E74 is INCLUDE_ASM and
                       names them); D_801027D4 / D_800A3864 rows retired (--rows retire, default) or suffixed
                       (--rows keep; only if the sandbox needs them to resolve the targets of C functions).
Writes tmp/func_80021424/mine.patch (git diff of exactly these files) afterwards.
"""
import re
import subprocess
import sys
from pathlib import Path

import os
SRCROOT = Path(__file__).resolve().parents[2]
root = Path(os.environ.get("L1_ROOT") or SRCROOT)
d = SRCROOT / "tmp/func_80021424"
rows = "keep" if "--rows" in sys.argv and sys.argv[sys.argv.index("--rows") + 1] == "keep" else "retire"


def rd(p):
    return (root / p).read_bytes().decode("utf-8")


DRY = "--dry" in sys.argv


def wr(p, s):
    if DRY:
        print("dry: would write", p, len(s))
        return
    (root / p).write_bytes(s.encode("utf-8"))


def sub1(s, a, b, what):
    assert s.count(a) == 1, (what, s.count(a))
    return s.replace(a, b)


def func_span(s, func):
    m = re.search(rf"^[A-Za-z_][^\n;]*\b{func}\s*\([^;{{]*\)\s*\{{", s, re.M)
    i = m.end()
    depth = 1
    while depth:
        depth += {"{": 1, "}": -1}.get(s[i], 0)
        i += 1
    return m.start(), i


def replace_body(s, func, base_file, new_file):
    a, b = func_span(s, func)
    base = (d / base_file).read_bytes().decode("utf-8").rstrip("\n")
    assert s[a:b] == base, f"{func}: body changed since the probes were measured"
    new = (d / new_file).read_bytes().decode("utf-8").rstrip("\n")
    return s[:a] + new + s[b:]


# ---- include/code6cac.h
h = rd("include/code6cac.h")
h = sub1(h, """ * fields at +0x4E are indices into D_801027B0[ch][0], as func_80021424 reads
 * them: f4E[rec+0x84] (id 0x7FF0) / f4E[rec+0x86] (ids 0x7FF1/2/4),
 * f54[rec+0x86][t] (id 0x7FF3), and
 * f66[id - 0x7FF5][rec+0x86] (ids 0x7FF5..0x7FFF). */
typedef struct {
    u8 pad00[0x14];
    s16 f14;
    u8 pad16[0x4E - 0x16];
""", """ * fields at +0x4E are indices into D_801027B0[ch][0], as func_80021424 reads
 * them: f4E[rec+0x84] (id 0x7FF0) / f4E[rec+0x86] (ids 0x7FF1/2/4),
 * f54[rec+0x86][t] (id 0x7FF3), and
 * f66[id - 0x7FF5][rec+0x86] (ids 0x7FF5..0x7FFF). f16 (func_800219E4) and
 * f18[class] (func_80021A3C; class = PracticeMenuRec.unk_0A, 0..26 as D_8008D538
 * holds and as the [27][6] class tables D_8008DE34 / D_8008DF78 are sized; 27
 * halfwords end exactly at f4E) are indices into D_80102760; both readers load
 * them with lhu. */
typedef struct {
    u8 pad00[0x14];
    s16 f14;
    u16 f16;
    u16 f18[27];
""", "Tbl800A3860Entry")
h = sub1(h, "extern Tbl800A3860Entry *D_800A3860[];\nextern s32 D_800A3864;\n",
         "extern Tbl800A3860Entry *D_800A3860[];\n", "D_800A3864 extern")
h = sub1(h, "extern s32 D_801027B0[][5];\nextern s32 D_801027B4;\nextern s32 D_801027B8;\n"
            "extern s32 D_801027BC[][5];\nextern s32 D_801027C0;\nextern s32 D_801027D4;\n",
         "extern s32 D_801027B0[][5];\n", "D_801027B4..D4 externs")
wr("include/code6cac.h", h)

# ---- src/code6cac_tu2.c
s = rd("src/code6cac_tu2.c")
for f in ("func_80020D70", "func_80021210", "func_800213A0", "func_80021904", "func_80021974",
          "func_800219E4", "func_80021A3C", "func_80021A98", "func_80022F34"):
    s = replace_body(s, f, f"{f}.base.c", f"{f}.m.c")
s = sub1(s, "/* Tail word after func_8001C8DC's seven-entry compiler-generated switch table. */\n"
            "const u32 D_800100E0[1] = { 0x00000000 };\n", "", "D_800100E0")
s = sub1(s, "/* Tail word after func_80021424's five-entry compiler-generated switch table. */\n"
            "const u32 D_80010428[1] = { 0x00000000 };\n",
         """/* 0x80010428: a zero word after func_80021424's jump table, before code6cac_b.o's rodata.
 * It is data, not padding: Sony's PSYLINK 2.37 does not pad an object's end
 * (docs/grind/rodata-align-2026-09-30.md section 1, test t1), and code6cac_b.o must start at
 * 0x8001042C, the phase of its first jump table (object-relative .align 3). No instruction
 * references it, so its original declaration is not recoverable; any 4-byte zero object gives
 * these bytes. Spelled as one unreferenced filler named D_<addr> at its exact size, in the spirit of
 * owner Q71 (per-file-gp-model.md (A6)). Evidence: memory/grind/func_80021424/retro-audit-2026-09-30.md. */
const u32 D_80010428 = 0;
""", "D_80010428")
wr("src/code6cac_tu2.c", s)

# ---- src/code6cac_c2.c
c2 = rd("src/code6cac_c2.c")
c2 = replace_body(c2, "func_8003CF84", "func_8003CF84.base.c", "func_8003CF84.m.c")
wr("src/code6cac_c2.c", c2)

# ---- undefined_syms_auto.txt
u = rd("undefined_syms_auto.txt")
for sym, off in (("D_801027B4", "0x4"), ("D_801027B8", "0x8"), ("D_801027BC", "0xC"), ("D_801027C0", "0x10")):
    u = sub1(u, f"{sym} = 0x{sym[2:]};\n",
             f"{sym} = 0x{sym[2:]};  /* alias of D_801027B0+{off}; retire with func_80020E74 "
             f"(asm/funcs/func_80020E74.s is its only assembled referrer) */\n", sym)
for sym, alias in (("D_801027D4", "D_801027B0+0x24"), ("D_800A3864", "D_800A3860+0x4")):
    line = f"{sym} = 0x{sym[2:]};\n"
    if rows == "retire":
        u = sub1(u, line, "", sym)
    else:
        u = sub1(u, line, f"{sym} = 0x{sym[2:]};  /* alias of {alias}; scoring reference only "
                          f"(no assembled referrer; the sandbox resolves C functions' targets by name) */\n", sym)
wr("undefined_syms_auto.txt", u)

if DRY:
    sys.exit(0)
files = ["include/code6cac.h", "src/code6cac_tu2.c", "src/code6cac_c2.c", "undefined_syms_auto.txt"]
if os.environ.get("L1_ROOT"):
    sys.exit(0)
patch = subprocess.run(["git", "diff", "--"] + files, cwd=root, capture_output=True).stdout
(d / "mine.patch").write_bytes(patch)
print("applied; rows =", rows, "; patch bytes", len(patch))
