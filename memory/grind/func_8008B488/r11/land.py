#!/usr/bin/env python3
"""Apply the func_8008B488 landing (WSL, repo root, under the landing lock):
  src/main.c        INCLUDE_ASM -> tmp/f8b488s4/final.c; delete the hand-transcribed
                    jtbl_80016460/jtbl_80016480 consts (the compiler now emits both
                    ADDR_VECs); update the two comments that describe them.
  Makefile          drop `main` from RODATA_ALIGN2_FILES.
  engine/buildconfig.py  the same, in the mirror.
Every edit asserts its anchor occurs exactly once."""
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]


def edit(rel, pairs):
    p = ROOT / rel
    s = p.read_bytes().decode()
    assert "\r" not in s, rel
    for old, new in pairs:
        n = s.count(old)
        assert n == 1, (rel, old[:60], n)
        s = s.replace(old, new)
    p.write_bytes(s.encode())
    print("edited", rel)


body = (ROOT / "tmp/f8b488s4/final.c").read_bytes().decode()
assert body.endswith("\n")
edit("src/main.c", [
    ('INCLUDE_ASM("asm/funcs", func_8008B488);\n', body),
    ("const u32 jtbl_80016460[8] = {\n"
     "    0x8008B5CC, 0x8008B5D4, 0x8008B5DC, 0x8008B5E4,\n"
     "    0x8008B5EC, 0x8008B5F4, 0x8008B5FC, 0x00000000,\n"
     "};\n"
     "const u32 jtbl_80016480[7] = {\n"
     "    0x8008B6AC, 0x8008B6B4, 0x8008B6BC, 0x8008B6C4,\n"
     "    0x8008B6CC, 0x8008B6D4, 0x8008B6DC,\n"
     "};\n", ""),
    (" * see the comment above D_800163D8). */\n",
     " * see the comment above D_800163D8). jtbl_80016460/jtbl_80016480 are likewise\n"
     " * compiler-emitted, by func_8008B488's two volume-mode switches (2026-09-28);\n"
     " * the zero word between them at 0x8001647C is the `.align 3` the compiler\n"
     " * puts before the second table, which is why main is not in\n"
     " * RODATA_ALIGN2_FILES. */\n"),
    ("(already so at HEAD; the sole caller func_8008B488 is INCLUDE_ASM).",
     "(already so at HEAD; the sole caller is func_8008B488)."),
])
edit("Makefile", [(" text1b text1b_b main\n", " text1b text1b_b\n")])
edit("engine/buildconfig.py", [('"text1a_c2", "text1b", "text1b_b", "main",\n', '"text1a_c2", "text1b", "text1b_b",\n')])
