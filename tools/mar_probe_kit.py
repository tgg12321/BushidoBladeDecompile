#!/usr/bin/env python3
"""Marionation lift probe: apply a source tweak, run cpp|cc1 -da, print the
七 contested pseudo dispositions + function insn count. Restores src.
Usage: probe_mar.py <tag>"""
import re
import subprocess
import sys
from pathlib import Path

SRC = Path("src/system.c")
orig = SRC.read_text()
tag = sys.argv[1]
body = orig

if tag == "t1":  # tbl_125c +1 via chain-extender on the D_800F19C0 store
    body = body.replace(
        "  idx_1496 = idx_1494 + 2;\n  D_800F19BC = 0;\n  D_800F19C0 = &D_80016248;\n",
        "  idx_1496 = idx_1494 + 2;\n  D_800F19BC = 0;\n"
        "  D_800F19C0 = (void *)((u8 *)tbl_125c + ((s32)&D_80016248 - (s32)D_800A125C)); /* FAKE */\n",
        1)
    assert "(s32)&D_80016248" in body, "t1 splice failed"
elif tag in ("t3", "t4"):
    # both copy arms: dst=a1 + src=... BEFORE the null test (target's byte order)
    body = body.replace(
        "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    if (a1 != 0)\n    {\n      dst = a1;\n      i = 7;\n",
        "    *idx_1496 = 0;\n    dst = a1;\n    src = (u8 *) (&D_800F19B0);\n    if (dst != 0)\n    {\n      i = 7;\n",
        1)
    body = body.replace(
        "      *(idx_1496 - 1) = 0;\n      dst = a1;\n      src = (u8 *) (&D_800F19A8);\n      if (dst != 0)\n      {\n",
        "      *(idx_1496 - 1) = 0;\n      dst = a1;\n      src = (u8 *) (&D_800F19A8);\n      if (dst != 0)\n      {\n",
        1)
    assert body.count("if (dst != 0)") == 2, "t3 splice failed"
    if tag == "t4":  # + t1 tbl chain
        body = body.replace(
            "  D_800F19C0 = &D_80016248;\n",
            "  D_800F19C0 = (void *)((u8 *)tbl_125c + ((s32)&D_80016248 - (s32)D_800A125C)); /* FAKE */\n",
            1)
        assert "(s32)&D_80016248" in body
elif tag in ("t5", "t6"):
    # t3 arm reshape
    body = body.replace(
        "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    if (a1 != 0)\n    {\n      dst = a1;\n      i = 7;\n",
        "    *idx_1496 = 0;\n    dst = a1;\n    src = (u8 *) (&D_800F19B0);\n    if (dst != 0)\n    {\n      i = 7;\n",
        1)
    # tbl chain via copy1 src (ref + range extension)
    body = body.replace(
        "    src = (u8 *) (&D_800F19B0);\n",
        "    src = (u8 *)((u8 *)tbl_125c + ((s32)&D_800F19B0 - (s32)D_800A125C)); /* FAKE */\n",
        1)
    # i1495 chain via copy2 src (ref + range extension)
    body = body.replace(
        "      src = (u8 *) (&D_800F19A8);\n",
        "      src = (u8 *)((u8 *)idx_1495 + ((s32)&D_800F19A8 - ((s32)&D_800A1494 + 1))); /* FAKE */\n",
        1)
    assert "(s32)&D_800F19B0 - (s32)D_800A125C" in body, "t5 tbl splice failed"
    assert "(s32)&D_800F19A8 - ((s32)&D_800A1494 + 1)" in body, "t5 1495 splice failed"
    if tag == "t6":
        # i1495 second ref via printf fmt chain
        body = body.replace(
            "debug_printf(&D_800161C8, ",
            "debug_printf((void *)((u8 *)idx_1495 + ((s32)&D_800161C8 - ((s32)&D_800A1494 + 1))), /* FAKE */ ",
            1)
        assert "(s32)&D_800161C8 - ((s32)&D_800A1494 + 1)" in body
elif tag == "u1":
    # two-var + mask check shape, pin dropped; arms unchanged
    body = body.replace(
        "  {\n    register s32 check asm(\"$6\");\n    check = *idx_1496;\n    if (!check) goto check2;\n",
        "  {\n    u8 raw;\n    s32 check;\n    raw = *idx_1496;\n    check = raw & 0xFF;\n    if (!check) goto check2;\n",
        1)
    body = body.replace(
        "    check2:\n    check = *(idx_1496 - 1);\n    if (check)\n",
        "    check2:\n    raw = *(idx_1496 - 1);\n    check = raw & 0xFF;\n    if (check)\n",
        1)
    assert "register s32 check" not in body and body.count("& 0xFF") >= 2, "u1 splice failed"
elif tag == "u2":
    # check via reused wide v0 temp (sanctioned var-reuse) + mask; pin dropped
    body = body.replace(
        "  {\n    register s32 check asm(\"$6\");\n    check = *idx_1496;\n    if (!check) goto check2;\n",
        "  {\n    s32 check;\n    v0 = *idx_1496;\n    check = v0 & 0xFF;\n    if (!check) goto check2;\n",
        1)
    body = body.replace(
        "    check2:\n    check = *(idx_1496 - 1);\n    if (check)\n",
        "    check2:\n    v0 = *(idx_1496 - 1);\n    check = v0 & 0xFF;\n    if (check)\n",
        1)
    assert "register s32 check" not in body and body.count("& 0xFF") >= 2, "u2 splice failed"
elif tag == "u3":
    # check via the sibling's opaque new_var mask (fixes the undeclared landmine)
    body = body.replace(
        "  u8 *idx_1494;\n  u8 *idx_1495;\n  u8 *idx_1496;\n  s32 *tbl_125c;\n  u8 saved;\n  s32 status;\n  u8 *src;\n  u8 *dst;\n  s32 i;\n  u8 b;\n  D_800F19B8 = sys_VSync(-1) + 0x3C0;\n  tbl_125c = D_800A125C;\n  idx_1494 = &D_800A1494;\n  idx_1495 = 1 + idx_1494;\n",
        "  u8 *idx_1494;\n  u8 *idx_1495;\n  u8 *idx_1496;\n  s32 *tbl_125c;\n  u8 saved;\n  s32 status;\n  u8 *src;\n  u8 *dst;\n  s32 i;\n  u8 b;\n  int new_var;\n  D_800F19B8 = sys_VSync(-1) + 0x3C0;\n  tbl_125c = D_800A125C;\n  idx_1494 = &D_800A1494;\n  idx_1495 = 1 + idx_1494;\n",
        1)
    body = body.replace(
        "  {\n    register s32 check asm(\"$6\");\n    check = *idx_1496;\n    if (!check) goto check2;\n",
        "  {\n    s32 check;\n    check = *idx_1496 & new_var;\n    if (!check) goto check2;\n",
        1)
    body = body.replace(
        "    check2:\n    check = *(idx_1496 - 1);\n    if (check)\n",
        "    check2:\n    check = *(idx_1496 - 1) & new_var;\n    if (check)\n",
        1)
    assert "int new_var;" in body and "register s32 check" not in body, "u3 splice failed"
elif tag == "u4":
    # single-use new_var mask (check site 1 only) — diagnostic
    body = body.replace(
        "  u8 b;\n  D_800F19B8",
        "  u8 b;\n  int new_var;\n  D_800F19B8", 1)
    body = body.replace(
        "  {\n    register s32 check asm(\"$6\");\n    check = *idx_1496;\n    if (!check) goto check2;\n",
        "  {\n    s32 check;\n    check = *idx_1496 & new_var;\n    if (!check) goto check2;\n",
        1)
    body = body.replace(
        "    check2:\n    check = *(idx_1496 - 1);\n    if (check)\n",
        "    check2:\n    check = *(idx_1496 - 1);\n    if (check)\n",
        1)
    assert "int new_var;" in body and "register s32 check" not in body, "u4 splice failed"
elif tag == "u5":
    # two-set new_var: existing set at 555 + re-set before check2
    body = body.replace(
        "  u8 b;\n  D_800F19B8",
        "  u8 b;\n  int new_var;\n  D_800F19B8", 1)
    body = body.replace(
        "  {\n    register s32 check asm(\"$6\");\n    check = *idx_1496;\n    if (!check) goto check2;\n",
        "  {\n    s32 check;\n    check = *idx_1496 & new_var;\n    if (!check) goto check2;\n",
        1)
    body = body.replace(
        "    check2:\n    check = *(idx_1496 - 1);\n    if (check)\n",
        "    check2:\n    new_var = 0xFF;\n    check = *(idx_1496 - 1) & new_var;\n    if (check)\n",
        1)
    assert "int new_var;" in body and body.count("new_var = 0xFF;") == 3, "u5 splice failed"
elif tag == "u6":
    # two single-use cross-block mask constants (new_var + new_var3)
    body = body.replace(
        "  u8 b;\n  D_800F19B8",
        "  u8 b;\n  int new_var;\n  int new_var3;\n  D_800F19B8", 1)
    body = body.replace(
        "  new_var = 0xFF;\n  if (sys_GetVblankCount() != 0)\n  {\n    saved = (*D_800A147C_2) & 3;\n    poll:",
        "  new_var = 0xFF;\n  new_var3 = 0xFF;\n  if (sys_GetVblankCount() != 0)\n  {\n    saved = (*D_800A147C_2) & 3;\n    poll:",
        1)
    body = body.replace(
        "  {\n    register s32 check asm(\"$6\");\n    check = *idx_1496;\n    if (!check) goto check2;\n",
        "  {\n    s32 check;\n    check = *idx_1496 & new_var;\n    if (!check) goto check2;\n",
        1)
    body = body.replace(
        "    check2:\n    check = *(idx_1496 - 1);\n    if (check)\n",
        "    check2:\n    check = *(idx_1496 - 1) & new_var3;\n    if (check)\n",
        1)
    assert "new_var3 = 0xFF;" in body and "& new_var3" in body, "u6 splice failed"
elif tag == "u7":
    # u6 masks + t3 arm-1 reshape combined
    body = body.replace(
        "  u8 b;\n  D_800F19B8",
        "  u8 b;\n  int new_var;\n  int new_var3;\n  D_800F19B8", 1)
    body = body.replace(
        "  new_var = 0xFF;\n  if (sys_GetVblankCount() != 0)\n",
        "  new_var = 0xFF;\n  new_var3 = 0xFF;\n  if (sys_GetVblankCount() != 0)\n",
        1)
    body = body.replace(
        "  {\n    register s32 check asm(\"$6\");\n    check = *idx_1496;\n    if (!check) goto check2;\n",
        "  {\n    s32 check;\n    check = *idx_1496 & new_var;\n    if (!check) goto check2;\n",
        1)
    body = body.replace(
        "    check2:\n    check = *(idx_1496 - 1);\n    if (check)\n",
        "    check2:\n    check = *(idx_1496 - 1) & new_var3;\n    if (check)\n",
        1)
    body = body.replace(
        "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    if (a1 != 0)\n    {\n      dst = a1;\n      i = 7;\n",
        "    *idx_1496 = 0;\n    dst = a1;\n    src = (u8 *) (&D_800F19B0);\n    if (dst != 0)\n    {\n      i = 7;\n",
        1)
    assert "new_var3 = 0xFF;" in body and body.count("if (dst != 0)") == 2, "u7 splice failed"
elif tag == "u8":
    # u6 masks + arm-1: src-first then dst then test-dst
    body = body.replace(
        "  u8 b;\n  D_800F19B8",
        "  u8 b;\n  int new_var;\n  int new_var3;\n  D_800F19B8", 1)
    body = body.replace(
        "  new_var = 0xFF;\n  if (sys_GetVblankCount() != 0)\n",
        "  new_var = 0xFF;\n  new_var3 = 0xFF;\n  if (sys_GetVblankCount() != 0)\n",
        1)
    body = body.replace(
        "  {\n    register s32 check asm(\"$6\");\n    check = *idx_1496;\n    if (!check) goto check2;\n",
        "  {\n    s32 check;\n    check = *idx_1496 & new_var;\n    if (!check) goto check2;\n",
        1)
    body = body.replace(
        "    check2:\n    check = *(idx_1496 - 1);\n    if (check)\n",
        "    check2:\n    check = *(idx_1496 - 1) & new_var3;\n    if (check)\n",
        1)
    body = body.replace(
        "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    if (a1 != 0)\n    {\n      dst = a1;\n      i = 7;\n",
        "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    dst = a1;\n    if (dst != 0)\n    {\n      i = 7;\n",
        1)
    assert "new_var3 = 0xFF;" in body and body.count("if (dst != 0)") == 2, "u8 splice failed"
elif tag in ("u9", "u10"):
    body = body.replace(
        "  u8 b;\n  D_800F19B8",
        "  u8 b;\n  int new_var;\n  int new_var3;\n  D_800F19B8", 1)
    body = body.replace(
        "  new_var = 0xFF;\n  if (sys_GetVblankCount() != 0)\n",
        "  new_var = 0xFF;\n  new_var3 = 0xFF;\n  if (sys_GetVblankCount() != 0)\n",
        1)
    m1, m2 = ("new_var", "new_var3") if tag == "u9" else ("new_var3", "new_var")
    if tag == "u10" and __import__("os").environ.get("SWAP_SETS"):
        body = body.replace(
            "  new_var = 0xFF;\n  new_var3 = 0xFF;\n",
            "  new_var3 = 0xFF;\n  new_var = 0xFF;\n", 1)
    body = body.replace(
        "  {\n    register s32 check asm(\"$6\");\n    check = *idx_1496;\n    if (!check) goto check2;\n",
        "  {\n    s32 check;\n    check = *idx_1496 & " + m1 + ";\n    if (!check) goto check2;\n",
        1)
    body = body.replace(
        "    check2:\n    check = *(idx_1496 - 1);\n    if (check)\n",
        "    check2:\n    check = *(idx_1496 - 1) & " + m2 + ";\n    if (check)\n",
        1)
    # u9 additionally: arm-1 reshape as u8
    if tag == "u9":
        body = body.replace(
            "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    if (a1 != 0)\n    {\n      dst = a1;\n      i = 7;\n",
            "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    dst = a1;\n    if (dst != 0)\n    {\n      i = 7;\n",
            1)
    else:
        body = body.replace(
            "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    if (a1 != 0)\n    {\n      dst = a1;\n      i = 7;\n",
            "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    dst = a1;\n    if (dst != 0)\n    {\n      i = 7;\n",
            1)
    assert "new_var3" in body, "u9/u10 splice failed"
elif tag == "u11":
    # u8 CORRECTED: marionation-unique anchors (the twins share text!)
    body = body.replace(
        "  u8 *idx_1496;\n  s32 *tbl_125c;",
        "  u8 *idx_1496;\n  int new_var;\n  int new_var3;\n  s32 *tbl_125c;", 1)
    body = body.replace(
        "  new_var = 0xFF;\n  if (sys_GetVblankCount() != 0)\n  {\n    saved = (*D_800A147C_2) & 3;",
        "  new_var = 0xFF;\n  new_var3 = 0xFF;\n  if (sys_GetVblankCount() != 0)\n  {\n    saved = (*D_800A147C_2) & 3;",
        1)
    body = body.replace(
        "  {\n    register s32 check asm(\"$6\");\n    check = *idx_1496;\n    if (!check) goto check2;\n",
        "  {\n    s32 check;\n    check = *idx_1496 & new_var;\n    if (!check) goto check2;\n",
        1)
    body = body.replace(
        "    check2:\n    check = *(idx_1496 - 1);\n    if (check)\n",
        "    check2:\n    check = *(idx_1496 - 1) & new_var3;\n    if (check)\n",
        1)
    body = body.replace(
        "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    if (a1 != 0)\n    {\n      dst = a1;\n      i = 7;\n",
        "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    dst = a1;\n    if (dst != 0)\n    {\n      i = 7;\n",
        1)
    assert "int new_var3;\n  s32 *tbl_125c;" in body and "new_var3 = 0xFF;" in body, "u11 splice failed"
elif tag in ("u12", "u13", "w8", "w9", "w10", "w11", "w12", "w13", "w14", "w15", "w16", "w17", "w18", "w19", "w20", "w21", "w22", "w23", "w24", "w25", "w26", "w27"):
    # u11 masks + csmd4's byte-proven arm shape (dst; src; i=7; if (a1))
    body = body.replace(
        "  u8 *idx_1496;\n  s32 *tbl_125c;",
        "  u8 *idx_1496;\n  int new_var;\n  int new_var3;\n  s32 *tbl_125c;", 1)
    body = body.replace(
        "  new_var = 0xFF;\n  if (sys_GetVblankCount() != 0)\n  {\n    saved = (*D_800A147C_2) & 3;",
        "  new_var = 0xFF;\n  new_var3 = 0xFF;\n  if (sys_GetVblankCount() != 0)\n  {\n    saved = (*D_800A147C_2) & 3;",
        1)
    body = body.replace(
        "  {\n    register s32 check asm(\"$6\");\n    check = *idx_1496;\n    if (!check) goto check2;\n",
        "  {\n    s32 check;\n    check = *idx_1496 & new_var;\n    if (!check) goto check2;\n",
        1)
    body = body.replace(
        "    check2:\n    check = *(idx_1496 - 1);\n    if (check)\n",
        "    check2:\n    check = *(idx_1496 - 1) & new_var3;\n    if (check)\n",
        1)
    body = body.replace(
        "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    if (a1 != 0)\n    {\n      dst = a1;\n      i = 7;\n      do\n",
        "    *idx_1496 = 0;\n    dst = a1;\n    src = (u8 *) (&D_800F19B0);\n    i = 7;\n    if (a1 != 0)\n    {\n      do\n",
        1)
    body = body.replace(
        "      *(idx_1496 - 1) = 0;\n      dst = a1;\n      src = (u8 *) (&D_800F19A8);\n      if (dst != 0)\n      {\n        i = 7;\n        do\n",
        "      *(idx_1496 - 1) = 0;\n      dst = a1;\n      src = (u8 *) (&D_800F19A8);\n      i = 7;\n      if (dst != 0)\n      {\n        do\n",
        1)
    print("u12 dbg:", "nv3" if "int new_var3;" in body else "NO-nv3",
          "arm1" if "i = 7;\n    if (a1 != 0)" in body else "NO-arm1",
          "arm2" if "i = 7;\n      if (dst != 0)" in body else "NO-arm2",
          "mask1" if "& new_var;" in body else "NO-mask1")
    assert "int new_var3;" in body, "u12 splice failed"
    if tag in ("u13", "w9", "w10", "w11", "w12", "w13", "w14", "w15", "w16", "w17", "w18", "w19", "w20", "w21", "w22", "w23", "w24", "w25", "w26", "w27"):
        # block-local byte temps per copy loop (b becomes local-alloc class)
        body = body.replace(
            "      do\n      {\n        b = *src;\n        src++;\n        i--;\n        *dst = b;\n        dst++;\n      }\n      while (i != (-1));\n",
            "      do\n      {\n        u8 bb;\n        bb = *src;\n        src++;\n        i--;\n        *dst = bb;\n        dst++;\n      }\n      while (i != (-1));\n",
            1)
        body = body.replace(
            "        do\n        {\n          b = *src;\n          src++;\n          i--;\n          *dst = b;\n          dst++;\n        }\n        while (i != (-1));\n",
            "        do\n        {\n          u8 bb;\n          bb = *src;\n          src++;\n          i--;\n          *dst = bb;\n          dst++;\n        }\n        while (i != (-1));\n",
            1)
        print("u13 dbg:", "loops" if body.count("u8 bb;") == 2 else "NO-loops")
    if tag == "w10":
        # csmd4-candidate printf shape: split-index + arg5-first + arg4
        # INLINE in the call (v16-style stretch -> arg4 addr chain to a0)
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg5;\n    u8 t0;\n    t0 = idx_1494[0];\n    arg5 = tbl_125c[idx_1494[1]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], tbl_125c[t0], arg5);\n  }",
            1)
        print("w10 dbg:", "printf" if "tbl_125c[t0], arg5);" in body else "NO-printf")
    if tag == "w11":
        # csmd4 candidate EXACT: split-index t0 first, arg5 local, arg4 LOCAL
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg4, arg5;\n    u8 t0;\n    t0 = idx_1494[0];\n    arg5 = tbl_125c[idx_1494[1]];\n    arg4 = tbl_125c[t0];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, arg5);\n  }",
            1)
        print("w11 dbg:", "printf" if "arg4 = tbl_125c[t0];" in body else "NO-printf")
    if tag == "w12":
        # all-locals in target order: t0, arg5, arg2v (11DC[11D5]), arg4
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg4, arg5;\n    s32 arg2v;\n    u8 t0;\n    t0 = idx_1494[0];\n    arg5 = tbl_125c[idx_1494[1]];\n    arg2v = D_800A11DC[D_800A11D5];\n    arg4 = tbl_125c[t0];\n    debug_printf(&D_800161C8, D_800F19C0, arg2v, arg4, arg5);\n  }",
            1)
        print("w12 dbg:", "printf" if "arg2v = D_800A11DC" in body else "NO-printf")
    if tag == "w13":
        # s32 t0 split (zero-extending SI load at stmt pos), arg5+arg4 locals
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg4, arg5;\n    s32 t0;\n    t0 = idx_1494[0];\n    arg5 = tbl_125c[idx_1494[1]];\n    arg4 = tbl_125c[t0];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, arg5);\n  }",
            1)
        print("w13 dbg:", "printf" if "s32 t0;" in body else "NO-printf")
    if tag == "w14":
        # s32 t0 early stmt (uservar pins birth) + arg4 INLINE (lw after sw)
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg5;\n    s32 t0;\n    t0 = idx_1494[0];\n    arg5 = tbl_125c[idx_1494[1]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], tbl_125c[t0], arg5);\n  }",
            1)
        print("w14 dbg:", "printf" if "tbl_125c[t0], arg5);" in body else "NO-printf")
    if tag == "w15":
        # t0 early + ADDRESS local p4 (sll/addu at stmt-3) + *p4 inline (lw after sw)
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg5;\n    s32 t0;\n    s32 *p4;\n    t0 = idx_1494[0];\n    arg5 = tbl_125c[idx_1494[1]];\n    p4 = &tbl_125c[t0];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], *p4, arg5);\n  }",
            1)
        print("w15 dbg:", "printf" if "p4 = &tbl_125c[t0];" in body else "NO-printf")
    if tag == "w16":
        # w15 + address-local pp for the a1 arg: combine substitutes the
        # symbol into the mem IN PLACE -> (set a1 (mem sym)) direct load,
        # no clock-5 a1-move -> fmt-la wins clock 5 -> a0 free for chain4
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg5;\n    s32 t0;\n    s32 *p4;\n    void **pp;\n    t0 = idx_1494[0];\n    pp = (void **)&D_800F19C0;\n    arg5 = tbl_125c[idx_1494[1]];\n    p4 = &tbl_125c[t0];\n    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *p4, arg5);\n  }",
            1)
        print("w16 dbg:", "printf" if "*pp, D_800A11DC" in body else "NO-printf")
    if tag == "w17":
        # w16 + CROSS-BLOCK u8 *pi = &D_800A11D5 (head init): sched1-opaque
        # (mem (reg pi)) forces the sw AFTER the 11D5 lbu (anti-dep) ->
        # val5->v1 -> chain4 -> a0; pi equiv-substituted away at local-alloc.
        body = body.replace(
            "  u8 *idx_1496;\n  int new_var;\n  int new_var3;\n  s32 *tbl_125c;",
            "  u8 *idx_1496;\n  int new_var;\n  int new_var3;\n  u8 *pi_11d5;\n  s32 *tbl_125c;", 1)
        _h, _s, _t = body.partition("s32 marionation_Exec")
        _t = _t.replace(
            "  idx_1496 = idx_1494 + 2;\n  D_800F19BC = 0;",
            "  idx_1496 = idx_1494 + 2;\n  pi_11d5 = (u8 *)&D_800A11D5;\n  D_800F19BC = 0;", 1)
        body = _h + _s + _t
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg5;\n    s32 t0;\n    s32 *p4;\n    void **pp;\n    t0 = idx_1494[0];\n    pp = (void **)&D_800F19C0;\n    arg5 = tbl_125c[idx_1494[1]];\n    p4 = &tbl_125c[t0];\n    debug_printf(&D_800161C8, *pp, D_800A11DC[*pi_11d5], *p4, arg5);\n  }",
            1)
        print("w17 dbg:", "pi" if "pi_11d5 = (u8 *)&D_800A11D5;" in body else "NO-pi",
              "printf" if "D_800A11DC[*pi_11d5]" in body else "NO-printf")
    if tag == "w18":
        # w16 + 11D5 index precomputed as stmt locals (low luids stall the
        # 11D5 launch cascade -> sw slots after its lbu -> val5->v1 ->
        # chain4 -> a0)
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg5;\n    s32 t0;\n    s32 *p4;\n    void **pp;\n    s32 i5;\n    s32 i5s;\n    t0 = idx_1494[0];\n    pp = (void **)&D_800F19C0;\n    arg5 = tbl_125c[idx_1494[1]];\n    i5 = D_800A11D5;\n    i5s = i5 * 4;\n    p4 = &tbl_125c[t0];\n    debug_printf(&D_800161C8, *pp, *(s32 *)((u8 *)D_800A11DC + i5s), *p4, arg5);\n  }",
            1)
        print("w18 dbg:", "printf" if "i5s = i5 * 4;" in body else "NO-printf")
    if tag == "w19":
        # w18 but i5/i5s adjacent to the call (densify the 11D5 qty so it
        # allocates before chain4 -> v0; chain4 stays sparse-last -> a0)
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg5;\n    s32 t0;\n    s32 *p4;\n    void **pp;\n    s32 i5;\n    s32 i5s;\n    t0 = idx_1494[0];\n    pp = (void **)&D_800F19C0;\n    arg5 = tbl_125c[idx_1494[1]];\n    p4 = &tbl_125c[t0];\n    i5 = D_800A11D5;\n    i5s = i5 * 4;\n    debug_printf(&D_800161C8, *pp, *(s32 *)((u8 *)D_800A11DC + i5s), *p4, arg5);\n  }",
            1)
        print("w19 dbg:", "printf" if "i5s = i5 * 4;" in body else "NO-printf")
    if tag == "w20":
        # multi-set accumulator chains (reg_n_sets>1 -> de-launched from the
        # backward-sched storm): t0 3-set same-reg chain (target byte form),
        # i5 2-set index; pp keeps the clock-5 la win.
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg5;\n    s32 t0;\n    void **pp;\n    s32 i5;\n    t0 = idx_1494[0];\n    pp = (void **)&D_800F19C0;\n    arg5 = tbl_125c[idx_1494[1]];\n    t0 *= 4;\n    t0 = (s32)((u8 *)tbl_125c + t0);\n    i5 = D_800A11D5;\n    i5 *= 4;\n    debug_printf(&D_800161C8, *pp, *(s32 *)((u8 *)D_800A11DC + i5), *(s32 *)t0, arg5);\n  }",
            1)
        print("w20 dbg:", "printf" if "t0 = (s32)((u8 *)tbl_125c + t0);" in body else "NO-printf")
    if tag == "w21":
        # t0 3-set chain (de-launched, byte-indexed arg4) + a2 ARRAY-form
        # inline (direct a2 load, no clock-5 thief) + pp a1
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg5;\n    s32 t0;\n    void **pp;\n    t0 = idx_1494[0];\n    pp = (void **)&D_800F19C0;\n    arg5 = tbl_125c[idx_1494[1]];\n    t0 *= 4;\n    t0 = (s32)((u8 *)tbl_125c + t0);\n    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);\n  }",
            1)
        print("w21 dbg:", "printf" if "*(s32 *)t0, arg5" in body else "NO-printf")
    if tag == "w22":
        # w21 + arg5 INLINE (value temp adjacent to the sw -> densest ->
        # val5 allocated before chain4 -> v1; chain4 -> a0)
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 t0;\n    void **pp;\n    t0 = idx_1494[0];\n    pp = (void **)&D_800F19C0;\n    t0 *= 4;\n    t0 = (s32)((u8 *)tbl_125c + t0);\n    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, tbl_125c[idx_1494[1]]);\n  }",
            1)
        print("w22 dbg:", "printf" if "tbl_125c[idx_1494[1]]);" in body else "NO-printf")
    if tag == "w23":
        # w22 + arg5 via addr-local p5 (*p5 inline): stmt-luid addr chain,
        # store-phase value load gated by aliasing
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 t0;\n    void **pp;\n    s32 *p5;\n    t0 = idx_1494[0];\n    pp = (void **)&D_800F19C0;\n    p5 = &tbl_125c[idx_1494[1]];\n    t0 *= 4;\n    t0 = (s32)((u8 *)tbl_125c + t0);\n    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, *p5);\n  }",
            1)
        print("w23 dbg:", "printf" if "*(s32 *)t0, *p5);" in body else "NO-printf")
    if tag == "w24":
        # w21 but t0's final set FIRST-declared... variant: swap stmt order
        # (t0-chain completes before arg5's stmt: chain4 luids < arg5's)
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg5;\n    s32 t0;\n    void **pp;\n    t0 = idx_1494[0];\n    pp = (void **)&D_800F19C0;\n    t0 *= 4;\n    t0 = (s32)((u8 *)tbl_125c + t0);\n    arg5 = tbl_125c[idx_1494[1]];\n    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);\n  }",
            1)
        print("w24 dbg:", "printf" if "arg5 = tbl_125c" in body else "NO-printf")
    if tag in ("w25", "w26", "w27"):
        # w24 + arm-1 reverted to ORIGINAL param-test shape (target bytes:
        # beqz s4 with move a1,s4 in the delay; u12's arm-1 reshape undone)
        body = body.replace(
            "  {\n    s32 arg5;\n    s32 arg4;\n    ;\n    arg4 = tbl_125c[idx_1494[0]];\n    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);\n  }",
            "  {\n    s32 arg5;\n    s32 t0;\n    void **pp;\n    t0 = idx_1494[0];\n    pp = (void **)&D_800F19C0;\n    t0 *= 4;\n    t0 = (s32)((u8 *)tbl_125c + t0);\n    arg5 = tbl_125c[idx_1494[1]];\n    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);\n  }",
            1)
        body = body.replace(
            "    *idx_1496 = 0;\n    dst = a1;\n    src = (u8 *) (&D_800F19B0);\n    i = 7;\n    if (a1 != 0)\n    {\n      do\n",
            "    *idx_1496 = 0;\n    src = (u8 *) (&D_800F19B0);\n    if (a1 != 0)\n    {\n      dst = a1;\n      i = 7;\n      do\n",
            1)
        print("w25 dbg:", "printf" if "*(s32 *)t0, arg5" in body else "NO-printf",
              "arm1" if "if (a1 != 0)\n    {\n      dst = a1;" in body else "NO-arm1")
    if tag in ("w26", "w27"):
        # w25 + arm-2 stmt swap (src before dst=a1): arg1 livelen +1 to
        # break the 952/952 tie in saved's favor
        body = body.replace(
            "      *(idx_1496 - 1) = 0;\n      dst = a1;\n      src = (u8 *) (&D_800F19A8);\n      i = 7;\n      if (dst != 0)\n",
            "      *(idx_1496 - 1) = 0;\n      src = (u8 *) (&D_800F19A8);\n      dst = a1;\n      i = 7;\n      if (dst != 0)\n",
            1)
        print("w26 dbg:", "arm2" if "src = (u8 *) (&D_800F19A8);\n      dst = a1;" in body else "NO-arm2")
    if tag == "w27":
        # w26 + dst=a1 after i=7 too (arg1 livelen 85 -> 86-87, into the
        # (666, 933) window under i1494)
        body = body.replace(
            "      src = (u8 *) (&D_800F19A8);\n      dst = a1;\n      i = 7;\n      if (dst != 0)\n",
            "      src = (u8 *) (&D_800F19A8);\n      i = 7;\n      dst = a1;\n      if (dst != 0)\n",
            1)
        print("w27 dbg:", "arm2" if "i = 7;\n      dst = a1;\n      if (dst != 0)" in body else "NO-arm2")
    if tag in ("w9", "w10", "w11", "w12", "w13", "w14", "w15", "w16", "w17", "w18", "w19", "w20", "w21", "w22", "w23", "w24", "w25", "w26", "w27"):
        # poll cycle as a REAL do-while (LOOP notes -> loop_depth weighting
        # on poll-resident refs). Masks stay depth-1 (outer stays goto-loop).
        # The saved-line `_2` suffix keeps anchors marionation-unique.
        body = body.replace(
            "    saved = (*D_800A147C_2) & 3;\n    poll:\n    status = func_80080828();\n\n    if (status != 0)\n    {\n",
            "    saved = (*D_800A147C_2) & 3;\n    do\n    {\n    status = func_80080828();\n\n    if (status == 0) break;\n    {\n",
            1)
        body = body.replace(
            "      goto poll;\n    }\n    *D_800A147C_2 = saved;",
            "    }\n    }\n    while (1);\n    *D_800A147C_2 = saved;",
            1)
        print("w9 dbg:", "poll-loop" if "while (1);\n    *D_800A147C_2 = saved;" in body else "NO-poll-loop")
elif tag != "t0":
    sys.exit("unknown tag")

SRC.write_text(body)
try:
    cpp = subprocess.run(
        ["mipsel-linux-gnu-cpp", "-Iinclude", "-undef", "-Wall", "-lang-c",
         "-fno-builtin", "-Dmips", "-D__GNUC__=2", "-D__OPTIMIZE__",
         "-D__mips__", "-D__mips", "-Dpsx", "-D__psx__", "-D__psx",
         "-D_PSYQ", "-D__EXTENSIONS__", "-D_MIPSEL", "-D_LANGUAGE_C",
         "-DLANGUAGE_C", str(SRC)], capture_output=True, text=True)
    subprocess.run(["mkdir", "-p", "tmp/rtl"])
    Path("tmp/rtl/mar.i").write_text(cpp.stdout)
    subprocess.run(
        ["tools/gcc-2.7.2/build/cc1", "-O2", "-G0", "-funsigned-char",
         "-quiet", "-mcpu=3000", "-mips1", "-mno-abicalls", "-fno-builtin",
         "-w", "-da", "tmp/rtl/mar.i", "-o", "tmp/rtl/mar.s"],
        capture_output=True, text=True)
    greg = Path("tmp/rtl/mar.i.greg").read_text()
    i = greg.index(";; Function marionation_Exec")
    sect = greg[i:i + 20000]
    m = re.search(r";; Register dispositions:\n((?:[^\n]*\n){1,8})", sect)
    disp = dict(re.findall(r"(\d+) in (\d+)", m.group(1)))
    names = {"72": "arg0", "73": "arg1", "76": "i1494", "77": "i1495",
             "78": "i1496", "79": "tbl", "80": "saved", "81": "p81"}
    rn = {16: "s0", 17: "s1", 18: "s2", 19: "s3", 20: "s4", 21: "s5",
          22: "s6", 23: "s7"}
    out = []
    for p, nm in names.items():
        r = int(disp.get(p, -1))
        out.append(f"{nm}={rn.get(r, r)}")
    s = Path("tmp/rtl/mar.s").read_text()
    fm = re.search(r"marionation_Exec:\n(.*?)\n\t\.end\s+marionation_Exec",
                   s, re.DOTALL)
    lines = [ln for ln in fm.group(1).splitlines()
             if (ln.startswith("\t") and not ln.startswith("\t.set")
                 and not ln.startswith("\t.end")) or ln.endswith(":")]
    insns = len([ln for ln in lines
                 if ln.startswith("\t") and not ln.startswith("\t.")])
    print(f"=== {tag} insns={insns} ===")
    print("  " + "  ".join(out))
    import os
    if os.environ.get("MAR_TAIL"):
        for ln in lines[-int(os.environ["MAR_TAIL"]):]:
            print(ln.replace("\t", " "))
finally:
    SRC.write_text(orig)
