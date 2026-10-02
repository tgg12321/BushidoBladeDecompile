"""Turn the generated text1b_ro.c into the landed src/text1a_b_pre_rodata.c (header/comments only)."""
from pathlib import Path
t = Path("tmp/cam/land/text1b_ro.c").read_text(encoding="utf-8")
old_head = '''#include "common.h"

typedef struct {
    u16 v[22];
} Unk800153F0Record;

/* ---- merged from text1a_b_pre_rodata.c (owner ruling Q67: one original file) ---- */
/* Rodata sub-TU split out for the 101C.rodata_text1a_b_pre cluster
 * (rodata-cleanup project, docs/rodata-cleanup-project.md, 2026-06-09).
 * MULTI-FILE cluster: 23 symbols (12 jtbls + 5 strings + 6 data words)
 * spanning text1a.c and text1b.c. Sub-TU packs all bytes into one file
 * at the asm/data slot (between text1a_b.o and text1b_b.o). */
#include "common.h"

/* Auto-extracted from asm/data/101C.rodata_text1a_b_pre.s */
'''
new_head = '''/* Rodata-only file linked between text1b.o (Q89 head; joins GP_FILES with the camera_CalcAngles landing) and
 * text1b_tu1b.o (tail), owner ruling Q94
 * (.claude/rules/compiler-flags-canonical.md): under -G8 cc1 emits every file-scope data object before the
 * head's jump tables, which the original places first. Restored from text1b.c's "merged from
 * text1a_b_pre_rodata.c" block (Q67/A7) verbatim.
 * Rodata sub-TU split out for the 101C.rodata_text1a_b_pre cluster
 * (rodata-cleanup project, docs/rodata-cleanup-project.md, 2026-06-09).
 * MULTI-FILE cluster: 23 symbols (12 jtbls + 5 strings + 6 data words)
 * spanning text1a.c and text1b.c. */
#include "common.h"

/* 0x800153F0: the 22-halfword record func_8004A09C unpacks; func_80049F4C (text1b.c) copies it whole. */
typedef struct {
    u16 v[22];
} Unk800153F0Record;

/* Auto-extracted from asm/data/101C.rodata_text1a_b_pre.s */
'''
assert t.count(old_head) == 1
t = t.replace(old_head, new_head)
old_note = "/* NOTE: the cluster continues in src/text1b.c (func_80058580's three switch tables at\n * 0x8001585C, text1b.o's rodata;"
assert t.count(old_note) == 1
t = t.replace(old_note, "/* NOTE: the cluster continues in src/text1b_tu1b.c (func_80058580's three switch tables at\n * 0x8001585C, text1b_tu1b.o's rodata;")
Path("tmp/cam/land/text1a_b_pre_rodata.c").write_bytes(t.encode("utf-8"))
print("ok")
