#!/usr/bin/env python3
"""Step 6 (Q65, per-file-gp-model.md "Merge": declarations reconciled FIRST, own byte-identical commit):
code6cac_c2.c and config.c declare two symbols differently; each gets its one truthful type.

  func_8004153C (defined in text1a_post.c as `s32 func_8004153C(s32 a0)`, returns a record pointer):
    code6cac_c2 calls it with one argument (`extern s32 *func_8004153C(s32);`), config with none
    (`extern void *func_8004153C(void);`, the call sites `func_8004153C()`). One declaration serves both call
    forms only unprototyped: `extern s32 *func_8004153C();` in both files (the argument is an s32 either way,
    so default promotion changes nothing; the return is used as `s32 *` / `void *`).
  D_800A3708 (a pointer to the 0x58-byte Unk80101DF0Record at 0x80101DF0, include/code6cac.h):
    code6cac_c2 declares it `Unk80101DF0Record *` and reads work.t[0] / work.t[2]; config declares it `u8 *`
    and reads the same two words as `*(s32 *)(D_800A3708 + 0x4C)` / `+ 0x54` (work is at +0x38, t at
    +0x14 in it). config takes the record type and reads the members, exactly as code6cac_c2's func_8003E6D8
    does. Body change in config's stage_InitCollision: its own layer-2 review.
usage: s06_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])
sub1("src/code6cac_c2.c", "extern s32 *func_8004153C(s32);", "extern s32 *func_8004153C();")
sub1("src/config.c", "extern void *func_8004153C(void);", "extern s32 *func_8004153C();")
sub1("src/config.c", '#include "game.h"\n', '#include "game.h"\n#include "code6cac.h"\n')  # the record type
sub1("src/config.c", "extern u8 *D_800A3708;", "extern Unk80101DF0Record *D_800A3708;")
sub1("src/config.c", "(*(s32 *)(D_800A3708 + 0x4C) + 0x7D00) / 2000", "(D_800A3708->work.t[0] + 0x7D00) / 2000")
sub1("src/config.c", "(*(s32 *)(D_800A3708 + 0x54) + 0x7D00) / 2000", "(D_800A3708->work.t[2] + 0x7D00) / 2000")
print("step 6 applied")
