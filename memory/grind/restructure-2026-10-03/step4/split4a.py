#!/usr/bin/env python3
"""Restructure step 4a: split the PsyQ library tail out of src/main/64FD8.c.

Line numbers are 1-based in the file as committed at 59272fc29. Every module's
source lines move verbatim; only declarations it needs to compile are added.
"""
import os, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SRC = os.path.join(ROOT, "src/main/64FD8.c")
L = open(SRC, encoding="utf-8", newline="").read().split("\n")
assert L[-1] == ""
L = L[:-1]


def ln(a, b=None):
    """lines a..b inclusive (1-based)."""
    b = a if b is None else b
    return L[a - 1:b]


# sanity anchors
assert L[2771 - 1].startswith("BIOS_A_FUNCTION(Exec, 0x43);")
assert L[2788 - 1].startswith("BIOS_B_FUNCTION(ChangeClearPAD, 0x5B);")
assert L[2789 - 1].startswith("s32 SetRCnt(")
assert L[2856 - 1] == "extern s32 D_8009BD80;"
assert L[2971 - 1] == "}" and L[2960 - 1].startswith("s32 _IsVSync(void)")
assert L[2972 - 1].startswith("BIOS_B_FUNCTION(InitPAD2")
assert L[2977 - 1].startswith("BIOS_C_FUNCTION(SysDeqIntRP")
assert L[2979 - 1].startswith("extern void (*jtbl_800A3624)")
assert L[2988 - 1] == 'INCLUDE_ASM("asm/funcs", _patch_pad);'
assert L[2989 - 1] == 'INCLUDE_ASM("asm/funcs", FlushCache);'
assert L[2991 - 1] == 'INCLUDE_ASM("asm/funcs", func_800790A4);'
assert L[2992 - 1] == 'INCLUDE_ASM("asm/funcs", _remove_ChgclrPAD);'
assert L[2993 - 1].startswith("u8* memcpy(")
assert L[3007 - 1] == "extern u32 D_800F1848;"
assert L[3016 - 1].startswith("u8 *strcpy(")
assert L[3029 - 1].startswith("s32 strlen(")
assert L[3039 - 1].startswith("void printf(")
assert L[3045 - 1] == "}"
assert L[1576 - 1] == "s32 _Pad1(void);"
assert ln(1758, 1762) == ["extern s32 D_8009BD68;", "extern s32 D_8009BD6C;", "extern s32 D_8009BD70;",
                          "extern s32 D_8009BD84;", "extern s32 D_8009BD88;"]
assert L[3047 - 1].startswith("/* Q65: this file's initialized small data")

MODSPAN = "a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3"


def bios_file(lib, mod, start, end, what):
    return lambda body: (
        f"/* PsyQ 4.0 {lib} {mod}: {what}. .text 0x{start:08X}..0x{end:08X}, {MODSPAN}. */\n"
        "#define INCLUDE_ASM_USE_MACRO_INC 1\n"
        '#include "include_asm.h"\n'
        '#include "bios.h"\n'
        "\n" + "\n".join(body) + "\n")


def asm_file(lib, mod, start, end, what):
    return lambda body: (
        f"/* PsyQ 4.0 {lib} {mod}: {what}. .text 0x{start:08X}..0x{end:08X}, {MODSPAN}. */\n"
        "#define INCLUDE_ASM_USE_MACRO_INC 1\n"
        '#include "include_asm.h"\n'
        "\n" + "\n".join(body) + "\n")


def c_file(lib, mod, start, end, what, decls=(), asm=False, bios=False, common=True):
    def f(body):
        s = f"/* PsyQ 4.0 {lib} {mod}: {what}. .text 0x{start:08X}..0x{end:08X}, {MODSPAN}. */\n"
        if asm:
            s += "#define INCLUDE_ASM_USE_MACRO_INC 1\n"
        s += '#include "common.h"\n'
        if asm:
            s += '#include "include_asm.h"\n'
        if bios:
            s += '#include "bios.h"\n'
        s += "\n"
        if decls:
            s += "/* Declarations from the file this module was split from (text1b_b.c). */\n"
            s += "\n".join(decls) + "\n\n"
        return s + "\n".join(body) + "\n"
    return f


A = "src/main/psxsdk/libapi/"
C2 = "src/main/psxsdk/libc2/"
files = []  # (path, generator, lines) in link order

stubs = [  # (file, module, start, BIOS what) lines 2771..2788
    ("c67", "C67", 0x80078948, "Exec, the BIOS A(0x43) trampoline", "bios"),
    ("c112", "C112", 0x80078958, "_bu_init, the BIOS A(0x70) trampoline", "bios"),
    ("c159", "C159", 0x80078968, "SetMem, the BIOS A(0x9F) trampoline", "bios"),
    ("a08", "A08", 0x80078978, "OpenEvent, the BIOS B(0x08) trampoline", "bios"),
    ("a09", "A09", 0x80078988, "CloseEvent, the BIOS B(0x09) trampoline", "bios"),
    ("a11", "A11", 0x80078998, "TestEvent, the BIOS B(0x0B) trampoline", "bios"),
    ("a12", "A12", 0x800789A8, "EnableEvent, the BIOS B(0x0C) trampoline", "bios"),
    ("a36", "A36", 0x800789B8, "EnterCriticalSection (syscall 1)", "asm"),
    ("a37", "A37", 0x800789C8, "ExitCriticalSection (syscall 2)", "asm"),
    ("a39", "A39", 0x800789D8, "SetSp", "asm"),
    ("a50", "A50", 0x800789E8, "open, the BIOS B(0x32) trampoline", "bios"),
    ("a52", "A52", 0x800789F8, "read, the BIOS B(0x34) trampoline", "bios"),
    ("a53", "A53", 0x80078A08, "write, the BIOS B(0x35) trampoline", "bios"),
    ("a54", "A54", 0x80078A18, "close, the BIOS B(0x36) trampoline", "bios"),
    ("a65", "A65", 0x80078A28, "format, the BIOS B(0x41) trampoline", "bios"),
    ("a66", "A66", 0x80078A38, "firstfile, the BIOS B(0x42) trampoline", "bios"),
    ("a67", "A67", 0x80078A48, "nextfile, the BIOS B(0x43) trampoline", "bios"),
    ("a91", "A91", 0x80078A58, "ChangeClearPAD, the BIOS B(0x5B) trampoline", "bios"),
]
for i, (fn, mod, st, what, kind) in enumerate(stubs):
    g = (bios_file if kind == "bios" else asm_file)("LIBAPI", mod, st, st + 0x10, what)
    files.append((A + fn + ".c", g, ln(2771 + i)))

files.append((A + "counter.c",
              c_file("LIBAPI", "COUNTER", 0x80078A68, 0x80078BE0,
                     "the root counters (SetRCnt, GetRCnt, StartRCnt, StopRCnt, ResetRCnt)",
                     decls=ln(1758, 1760)),
              ln(2789, 2855)))
files.append((A + "pad.c",
              c_file("LIBAPI", "PAD", 0x80078BE0, 0x80078F00,
                     "the controller init/start/stop wrappers and their interrupt patch (SetInitPadFlag .. _IsVSync)",
                     decls=[L[1576 - 1]] + ln(1761, 1762)),
              ln(2856, 2971)))
for i, (fn, mod, what) in enumerate([
        ("a18", "A18", "InitPAD2, the BIOS B(0x12) trampoline"),
        ("a19", "A19", "StartPAD2, the BIOS B(0x13) trampoline"),
        ("a20", "A20", "StopPAD2, the BIOS B(0x14) trampoline"),
        ("a21", "A21", "PAD_init2, the BIOS B(0x15) trampoline"),
        ("l02", "L02", "SysEnqIntRP, the BIOS C(0x02) trampoline"),
        ("l03", "L03", "SysDeqIntRP, the BIOS C(0x03) trampoline")]):
    st = 0x80078F00 + 0x10 * i
    files.append((A + fn + ".c", bios_file("LIBAPI", mod, st, st + 0x10, what), ln(2972 + i)))
files.append((A + "patch.c",
              c_file("LIBAPI", "PATCH", 0x80078F60, 0x80078FF0,
                     "EnablePAD, DisablePAD and _patch_pad (hand-written asm)", asm=True, common=False),
              ln(2979, 2988)))
files.append((A + "c68.c",
              lambda body: (
                  "/* PsyQ 4.0 LIBAPI C68 and SENDPAD, kept in one file. C68 is FlushCache, the BIOS A(0x44)\n"
                  " * trampoline (.text 0x80078FF0..0x80079000); SENDPAD is _SendPAD, _send_pad and the patch\n"
                  " * stub func_800790A4 (.text 0x80079000..0x800790C0). Both are verbatim LIBSCAN module spans\n"
                  " * (docs/naming/libscan/matches.json), but asm/funcs/FlushCache.s runs past C68's end into\n"
                  " * SENDPAD's first function (_SendPAD at 0x80079000 is a mid-function XDEF,\n"
                  " * docs/naming/libscan/boundary_fixes.md), so the boundary cannot be cut by moving source\n"
                  " * lines alone; splitting that asm function is a separate decision. */\n"
                  "#define INCLUDE_ASM_USE_MACRO_INC 1\n"
                  '#include "include_asm.h"\n'
                  "\n" + "\n".join(body) + "\n"),
              ln(2989, 2991)))
files.append((A + "chclrpad.c",
              asm_file("LIBAPI", "CHCLRPAD", 0x800790C0, 0x80079120, "_remove_ChgclrPAD (hand-written asm)"),
              ln(2992)))
files.append((C2 + "memcpy.c", c_file("LIBC2", "MEMCPY", 0x80079120, 0x80079154, "memcpy"), ln(2993, 3006)))
files.append((C2 + "rand.c", c_file("LIBC2", "RAND", 0x80079154, 0x80079194, "rand and srand"), ln(3007, 3015)))
files.append((C2 + "strcpy.c", c_file("LIBC2", "STRCPY", 0x80079194, 0x800791D8, "strcpy"), ln(3016, 3028)))
files.append((C2 + "strlen.c", c_file("LIBC2", "STRLEN", 0x800791D8, 0x80079208, "strlen"), ln(3029, 3038)))
files.append((C2 + "printf.c",
              c_file("LIBC2", "PRINTF", 0x80079208, 0x80079244,
                     "printf (formats through prnt, LIBC2 PRNT)"),
              ln(3039, 3045)))

# game part: lines 1..2769, minus the moved declarations, then the Q65 data block 3046..3050
moved = {1576, 1758, 1759, 1760, 1761, 1762}
game = [l for i, l in enumerate(L[:2769], 1) if i not in moved] + ln(3046, len(L))
assert game[7] == '#include "bios.h"', game[7]
del game[7]
TOP = ("/* Game functions func_800747D8 .. func_800788B0. .text 0x800747D8 (ROM 0x64FD8). Start boundary:\n"
       " * PHASE (site 5). Ends where the PsyQ 4.0 LIBAPI C67 module starts (0x80078948, LIBSCAN, Q106\n"
       " * D3); the library modules that followed are in src/main/psxsdk/libapi/ and libc2/. */")

used = set(range(2771, 2789)) | set(range(2789, 2972)) | set(range(2972, 2978)) | set(range(2979, 2993)) \
    | set(range(2993, 3046))
unused = [i for i in range(2770, 3046) if i not in used and i <= len(L)]
for i in unused:
    assert L[i - 1].strip() == "", (i, L[i - 1])

if "--apply" in sys.argv:
    for path, gen, body in files:
        p = os.path.join(ROOT, path)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        assert not os.path.exists(p), p
        with open(p, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(gen(body))
    with open(SRC, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(TOP + "\n" + "\n".join(game) + "\n")
for path, gen, body in files:
    print(path, len(body))
