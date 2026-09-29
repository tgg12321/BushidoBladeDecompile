#!/usr/bin/env python3
"""Build tmp/f8b488s4/final.c (the submission) and split.c (its Ruling 11 (C)(1)
one-variable-per-value twin) from the generated partition bodies."""
from pathlib import Path

D = Path(__file__).resolve().parent
HDR = """/* func_8008B488: per-voice SPU attribute setter with the shape of PsyQ
 * LIBSPU's SpuSetVoiceAttr (C ref: sotn-decomp src/main/psxsdk/libspu/s_sva.c
 * and psyz decomp/src/libspu/sr_sv.c). BB2 links an older build: no min/max
 * voice range, a different block order, and the SR mode defaulting to 0x100.
 * The name stays auto (near-tier-ruling-2026-09-07: no verbatim caller pins
 * it). SpuVoiceAttr per PsyQ libspu.h (sizeof = 0x40, the callers' s32[16]). */
"""
NOTE = ("        u16 temp; /* two values: the clamped sustain rate (SR block), then the\n"
        "                   * clamped sustain level (SL block); Ruling 11, proof in\n"
        "                   * memory/grind/func_8008B488/r11/proof.md */\n")

def nocast(s):
    # the (s16) casts on the volume-mode switches are no-ops (SpuVolume fields are s16):
    # measured byte-identical without them, so they are dropped
    for side in ("left", "right"):
        o = "switch ((s16)attr->volmode.%s)" % side
        assert s.count(o) == 1
        s = s.replace(o, "switch (attr->volmode.%s)" % side)
    # the SPU settle-wait locals: named like the same idiom's sibling in src/main.c
    # (`volatile s32 i; volatile s32 v;`, ~line 2513); names only, codegen-neutral
    for o, n in (("    volatile s32 sp10;\n", "    volatile s32 i;\n"),
                 ("    volatile s32 sp14;\n", "    volatile s32 v;\n"),
                 ("    sp14 = 1;\n    for (sp10 = 0; sp10 < 2; sp10++) {\n        sp14 *= 13;\n",
                  "    v = 1;\n    for (i = 0; i < 2; i++) {\n        v *= 13;\n")):
        assert s.count(o) == 1, o
        s = s.replace(o, n)
    assert "sp10" not in s and "sp14" not in s
    return s


final = nocast((D / "parts" / "p_AR_DR_SRSL_RR.c").read_text())
assert final.count("        u16 temp;\n") == 1
final = HDR + final.replace("        u16 temp;", NOTE, 1)
(D / "final.c").write_text(final, newline="\n")

split = nocast((D / "parts" / "p_AR_DR_SR_RR_SL.c").read_text())
(D / "split.c").write_text(HDR + split, newline="\n")
print("ok")
