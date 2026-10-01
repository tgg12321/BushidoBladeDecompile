"""Apply func_80023F08's landing edits to copies of the affected src files.
usage: python tu_edit.py <body.c> <outdir>   (reads src/*.c from the repo root)"""
import sys
from pathlib import Path

body, outdir = sys.argv[1:3]
out = Path(outdir)
out.mkdir(parents=True, exist_ok=True)
B = open(body).read().rstrip("\n")


def edit(stem, reps):
    s = Path(f"src/{stem}.c").read_text(encoding="utf-8")
    for old, new, cnt in reps:
        n = s.count(old)
        assert n == cnt, (stem, n, old[:90])
        s = s.replace(old, new)
    open(out / f"{stem}.c", "w", encoding="utf-8", newline="\n").write(s)


edit("code6cac_tu2", [
    ('INCLUDE_ASM("asm/funcs", func_80023F08);', B, 1),
    ("extern void func_80023F08(s32, s32);", "extern void func_80023F08(s32, PadState *);", 1),
    ("func_80023F08(0, (s32)&buf);", "func_80023F08(0, &buf);", 1),
    ("func_80023F08(1, (s32)&buf);", "func_80023F08(1, &buf);", 1),
    ("func_80023F08(0, (s32)&sp10);", "func_80023F08(0, &sp10);", 2),
    ("func_80023F08(1, (s32)&sp10);", "func_80023F08(1, &sp10);", 2),
    ("    D_800A3888 = (s32)0x80118800;\n    D_800A388C = (s32)0x8011C400;\n",
     "    D_800A3888[0] = (MotionFrame *)0x80118800;\n    D_800A3888[1] = (MotionFrame *)0x8011C400;\n", 1),
])
edit("code6cac_b", [
    ("current->unk_30 & 0x20", "current->unk_24.pressed & 0x20", 1),
    ("current->unk_30 & 0x40", "current->unk_24.pressed & 0x40", 1),
    ("current->unk_2C & 0x1000", "current->unk_24.held & 0x1000", 1),
    ("current->unk_2C & 0x4000", "current->unk_24.held & 0x4000", 1),
    ("record->unk_2C & 0x8000", "record->unk_24.held & 0x8000", 1),
    ("partner->unk_2C & 0x8000", "partner->unk_24.held & 0x8000", 2),
])
edit("text1b", [
    ("rec->unk_50[8]", "rec->unk_50->unk_08", 2),
    ("p->unk_50[8]", "p->unk_50->unk_08", 2),
])
s = Path("src/code6cac_b_tu2.c").read_text(encoding="utf-8")
i = s.index("/* Scratchpad point tables at 0x1F800000, as far as this function uses them.")
j = s.index("#define SPAD ((ScrPad *)0x1F800000)\n") + len("#define SPAD ((ScrPad *)0x1F800000)\n")
s = s[:i] + s[j:]
open(out / "code6cac_b_tu2.c", "w", encoding="utf-8", newline="\n").write(s)
edit("code6cac", [("extern void func_80023F08(s32, s32);", "extern void func_80023F08(s32, PadState *);", 1)])
