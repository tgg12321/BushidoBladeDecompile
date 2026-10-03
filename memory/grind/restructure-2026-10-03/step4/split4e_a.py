#!/usr/bin/env python3
"""Restructure step 4e, text split: src/main/psxsdk/libcd/bios.c (ex system.c, as committed at e32c75e81)
and src/main/psxsdk/libetc/intr.c (ex ings2.c, eef81d4af) -> one file per LIBSCAN module, the CDREAD module
joined across the old system.c|ings2.c cut at 0x8008289C, plus one file per unidentified gap (the two SN
runtime gaps around LIBAPI C57). bios.c keeps exactly LIBCD BIOS, intr.c exactly LIBETC INTR.

Source lines move verbatim; only declarations a part needs to compile are added (copies of declarations,
or prototypes spelled as the callee's definition, where the old file's call saw that definition) or moved.
Dropped, used by no part: the two include lists, the stale "--- Functions 0x... ---" / "--- text3 segment
functions ---" banners, ings2.c's duplicate `extern s32 g_CdReadCallback_func;` (system.c's copy moves with
CDREAD)."""
import os
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
while not os.path.exists(os.path.join(ROOT, "bb2.ld")):  # the repo root, from tmp/ or the banked copy
    ROOT = os.path.dirname(ROOT)


def rd(rel):  # the sources as committed at eef81d4af (re-runnable after --apply)
    import subprocess
    t = subprocess.run(["git", "-C", ROOT, "show", f"eef81d4af:{rel}"], capture_output=True,
                       check=True).stdout.decode("utf-8")
    assert "\r" not in t and t.endswith("\n")
    return t.split("\n")[:-1]


SP, IP = "src/main/psxsdk/libcd/bios.c", "src/main/psxsdk/libetc/intr.c"
S, I = rd(SP), rd(IP)
assert len(S) == 1130 and len(I) == 611, (len(S), len(I))


def s(a, b=None):
    return S[a - 1:(a if b is None else b)]


def i(a, b=None):
    return I[a - 1:(a if b is None else b)]


def at(L, n, txt):
    assert L[n - 1].startswith(txt), (n, L[n - 1], txt)


# ---- system.c anchors
assert s(1, 7) == ['#include "common.h"', '#define INCLUDE_ASM_USE_MACRO_INC 1', '#include "include_asm.h"',
                   '#include "bios.h"', '#include "system.h"', '#include "psx.h"', '#include "libcd.h"']
at(S, 9, "/* Forward declarations */")
at(S, 10, "extern void CD_flush(void);")
at(S, 19, "/* Externs for globals */")
assert s(20, 25) == ["extern u8 CD_status;",
                     "extern u8 CD_pos[4]; /* Sony's u_char CD_pos[4] (SOTN: src/main/psxsdk/libcd/bios.c:42 @aa53500) */",
                     "extern u8 CD_mode;", "extern u8 CD_com;", "extern s32 CD_cbsync;", "extern s32 CD_cbready;"]
at(S, 27, "/* --- Functions 0x8008008C - 0x800807A8 --- */")
assert S[29 - 1] == "BIOS_B_FUNCTION(DeliverEvent, 0x7);"
at(S, 31, "u32 CdStatus(void) {")
assert s(70, 72) == ["extern s32 CD_debug;", "extern s32 CD_comstr[];", "extern s32 CD_intstr[];"]
at(S, 315, "/* --- text3 segment functions (0x800807A8-0x800827D0, 17 funcs) --- */")
assert S[314 - 1] == "" and S[316 - 1] == ""
at(S, 317, "s32 CdPosToInt(u8 *a0) {")
assert S[331 - 1] == "}"
at(S, 332, "/* libcd bios.c module types/helpers, hoisted above getintr")
at(S, 883, "void cdrom_IrqHandler(void) {")
assert S[900 - 1] == "}"
at(S, 901, "/* PsyQ 4.0 LIBC2 puts: puts")
at(S, 906, "void puts(void *a0) {")
assert S[916 - 1] == "}"
assert s(917, 918) == ["extern s32 D_800162EC;", "extern s32 D_80016304;"]
at(S, 926, "extern s32 g_CdReadCallback_func;")
at(S, 933, "static void cb_read(u8 intr, u8 *result) {")
at(S, 1111, "s32 CdReadSync(s32 mode, s32 result) {")
assert S[1130 - 1] == "}"

# ---- ings2.c anchors
assert i(1, 6) == ['#include "common.h"', '#include "include_asm.h"', '#include "bios.h"', '#include "system.h"',
                   '#include "libcd.h"', '#include "sound.h"']
assert i(9, 11) == ["extern void SpuInit(void);", "extern void _SsInit(void);", "extern void SpuQuit(void);"]
assert i(14, 20) == ["extern s32 g_CdReadCallback_func;", "extern s32 g_sys_video_mode;",
                     "extern u16 g_sys_vblank_count;",
                     "extern volatile u16 *i_mask; /* libetc intr.c i_mask = (u16 *)0x1F801074, I_MASK (MMIO) */",
                     "extern s32 *g_sys_irq_vtable;", "extern volatile s32 Vcount;",
                     "extern void SpuSetCommonAttr(s32 *);"]
at(I, 22, "/* --- Functions 0x8008289C - 0x80083BE4 --- */")
at(I, 24, "s32 CdReadCallback(s32 a0) {")
assert I[38 - 1] == "}" and I[39 - 1] == ""
at(I, 40, "extern volatile s32 *g_vsync_gpu_stat_reg;")
assert i(94, 97) == ["extern s32 D_80016318;", "extern void puts(void *);", "extern void ChangeClearPAD(s32);",
                     "extern void ChangeClearRCnt(s32, s32);"]
assert I[115 - 1] == "}"
assert I[116 - 1] == "BIOS_C_FUNCTION(ChangeClearRCnt, 0xA);"
at(I, 117, "void ResetCallback(void) {")
at(I, 315, "void func_800831A4(u16 *ptr, s32 size) {")
assert I[322 - 1] == "}" and I[323 - 1] == "__asm__(" and I[344 - 1] == ");"
assert i(345, 348) == ["BIOS_B_FUNCTION(ReturnFromException, 0x17);", "BIOS_B_FUNCTION(ResetEntryInt, 0x18);",
                       "BIOS_B_FUNCTION(HookEntryInt, 0x19);", 'INCLUDE_ASM("asm/funcs", setjmp);']
at(I, 349, "extern s32 D_800A2614[8];")
assert I[385 - 1] == "}"
at(I, 386, "extern s32 D_800A2640[8];")
assert I[398 - 1] == "" and I[397 - 1] == "s32 setIntrDMA(s32, s32);"
at(I, 399, "void sys_MemClear(s32 *a0, s32 a1) {")
assert I[404 - 1] == "}"
at(I, 405, "s32 startIntrDMA(void) {")
assert I[458 - 1] == "}" and I[459 - 1] == ""
at(I, 460, "s32 SetVideoMode(s32 a0) {")
assert I[468 - 1] == "}" and I[469 - 1] == ""
assert I[470 - 1] == 'INCLUDE_ASM("asm/funcs", PCopen);'
assert I[482 - 1] == 'INCLUDE_ASM("asm/funcs", __do_global_dtors);'
assert I[483 - 1] == "BIOS_A_FUNCTION(InitHeap, 0x39);"
assert I[484 - 1] == "extern s32 _SN_read(s32, s32, s32, s32);"
assert I[512 - 1] == 'INCLUDE_ASM("asm/funcs", _SN_read);'
assert i(513, 514) == ["extern void EnterCriticalSection(void);", "extern void ExitCriticalSection(void);"]
at(I, 516, "void SsEnd(void) {")
assert I[536 - 1] == "}" and I[537 - 1] == ""
at(I, 538, "void SsInit(void) {")
assert I[542 - 1] == "}" and I[543 - 1] == ""
at(I, 544, "extern u16 D_800A269C;")
assert I[581 - 1] == "}" and I[582 - 1] == ""
at(I, 583, "void SsQuit(void) {")
assert I[585 - 1] == "}" and I[586 - 1] == ""
at(I, 587, "void SsSetSerialAttr(s32 a0, s32 a1, s32 a2) {")
assert I[611 - 1] == "}"

SPANS = {  # LIBSCAN verbatim starts (docs/naming/libscan/matches.json); "gap" rows are not LIBSCAN modules
    "A07": 0x8008008C, "SYS": 0x8008009C, "BIOS": 0x80080828, "PUTS": 0x80082000, "CDREAD": 0x80082050,
    "VSYNC": 0x800828CC, "L10": 0x80082AB0, "INTR": 0x80082AC0, "C114": 0x800831D0, "A23": 0x800831F0,
    "A24": 0x80083200, "A25": 0x80083210, "SETJMP": 0x80083220, "INTR_VB": 0x800832A0,
    "INTR_DMA": 0x800833C8, "VMODE": 0x80083670, "gap1": 0x80083698, "C57": 0x8008386C, "gap2": 0x8008387C,
    "SSEND": 0x80083954, "SSINIT_C": 0x80083A18, "SSINIT": 0x80083A48, "SSQUIT": 0x80083B30,
    "SSSATTR": 0x80083B50, "_END": 0x80083BE4}
ORDER = list(SPANS)
SPAN = "a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3"


def wrap(text):
    out, cur = [], ""
    for w in text.split(" "):  # wrap at 100 columns
        if len(cur) + len(w) + 1 > 100:
            out.append(cur)
            cur = " *"
        cur = (cur + " " + w) if cur else w
    return "\n".join(out + [cur])


def comment(lib, mod, what, extra=""):
    st, en = SPANS[mod], SPANS[ORDER[ORDER.index(mod) + 1]]
    return wrap(f"/* PsyQ 4.0 {lib} {mod}: {what}. .text 0x{st:08X}..0x{en:08X}, {SPAN}.{extra} */")


def gap_comment(mod, what):
    st, en = SPANS[mod], SPANS[ORDER[ORDER.index(mod) + 1]]
    return wrap(f"/* {what}. .text 0x{st:08X}..0x{en:08X}: an unidentified region between verbatim LIBSCAN "
                f"modules (docs/naming/libscan/matches.json; memory/closer/psyq-library-census.md), one file "
                f"per gap (Q106 D3), named by its ROM offset. */")


FROM_S = "/* Declarations from the file this module was split from (src/main/psxsdk/libcd/bios.c, ex system.c). */"
FROM_I = "/* Declarations from the file this module was split from (src/main/psxsdk/libetc/intr.c, ex ings2.c). */"


def stub(lib, mod, what, line):
    return (comment(lib, mod, what) + "\n#define INCLUDE_ASM_USE_MACRO_INC 1\n#include \"include_asm.h\"\n"
            "#include \"bios.h\"\n\n" + line + "\n")


def body(lines):
    while lines and lines[-1] == "":
        lines = lines[:-1]
    return "\n".join(lines) + "\n"


files = []  # (path, text) in link order
used_s, used_i = set(), set()


def use(sset, a, b):
    sset |= set(range(a, b + 1))


# A07
files.append(("src/main/psxsdk/libapi/a07.c",
              stub("LIBAPI", "A07", "DeliverEvent, the BIOS B(0x07) trampoline", S[29 - 1])))
use(used_s, 29, 29)
# SYS: the file head's declarations (all used by SYS) + CdStatus .. CdPosToInt
sys_lines = s(9, 25) + [""] + s(31, 314) + s(317, 331)
use(used_s, 9, 25); use(used_s, 31, 314); use(used_s, 317, 331)
files.append(("src/main/psxsdk/libcd/sys.c",
              comment("LIBCD", "SYS", "the CD command layer (CdStatus .. CdPosToInt; SOTN libcd/sys.c)")
              + '\n#include "common.h"\n#include "libcd.h"\n\n' + body(sys_lines)))
# BIOS
bios_decl = [FROM_S, S[10 - 1]] + s(20, 25) + s(70, 72)
use(used_s, 332, 900)
BIOS_TOP = comment("LIBCD", "BIOS", "the CD-ROM controller driver (getintr .. cdrom_IrqHandler, the module's "
                   "static `callback`; $Id: bios.c,v 1.86; SOTN libcd/bios.c)")
files.append((SP, BIOS_TOP + '\n#include "common.h"\n#include "system.h"\n#include "psx.h"\n#include "libcd.h"\n\n'
              + "\n".join(bios_decl) + "\n\n" + body(s(332, 900))))
# PUTS
use(used_s, 901, 916)
files.append(("src/main/psxsdk/libc2/puts.c",
              comment("LIBC2", "PUTS", "puts") + '\n#include "common.h"\n\n' + body(s(901, 916))))
# CDREAD: system.c's head of the module + ings2.c's tail
use(used_s, 917, 1130); use(used_i, 24, 38)
cdread_decl = [
    "/* Declarations from the file this module was split from (src/main/psxsdk/libcd/bios.c, ex system.c): the",
    " * LIBCD SYS and BIOS functions CDREAD calls, declared as their definitions declare them (the old file",
    " * defined them above this module). */",
    "extern s32 VSync(s32);",
    "extern void puts(void *);",
    "u32 CdStatus(void);",
    "u32 CdMode(void);",
    "void *CdLastPos(void);",
    "void CdFlush(void);",
    "s32 CdReady(s32 mode, u8 *result);",
    "s32 CdSyncCallback(s32 a0);",
    "s32 CdReadyCallback(s32 a0);",
    "s32 CdGetSector(s32 madr, s32 size);",
    "s32 CdGetSector2(s32 madr, s32 size);",
    "s32 CdDataCallback(s32 a0);",
    "void CdDataSync(s32 a0);",
    "s32 CdPosToInt(u8 *a0);",
]
files.append(("src/main/psxsdk/libcd/cdread.c",
              comment("LIBCD", "CDREAD", "CdRead and its callbacks (cb_read .. CdReadMode; SOTN libcd/cdread.c)",
                      " One file across the old system.c|ings2.c cut at 0x8008289C, which was mid-module (Q106 D3).")
              + '\n#include "common.h"\n#include "libcd.h"\n\n' + "\n".join(cdread_decl) + "\n\n"
              + body(s(917, 1130) + [""] + i(24, 38))))
# VSYNC
use(used_i, 40, 115)
files.append(("src/main/psxsdk/libetc/vsync.c",
              comment("LIBETC", "VSYNC", "VSync and v_wait (SOTN libetc/vsync.c)") + '\n#include "common.h"\n\n'
              + FROM_I + "\n" + I[19 - 1] + "\n\n" + body(i(40, 115))))
# L10
use(used_i, 116, 116)
files.append(("src/main/psxsdk/libapi/l10.c",
              stub("LIBAPI", "L10", "ChangeClearRCnt, the BIOS C(0x0A) trampoline", I[116 - 1])))
# INTR
use(used_i, 117, 322)
intr_decl = [FROM_I] + i(16, 18) + i(96, 97)
INTR_TOP = comment("LIBETC", "INTR", "the interrupt dispatcher (ResetCallback .. memclr; $Id: intr.c,v 1.76; "
                   "SOTN libetc/intr.c)")
files.append((IP, INTR_TOP + '\n#include "common.h"\n\n' + "\n".join(intr_decl) + "\n\n" + body(i(117, 322))))
# C114
use(used_i, 323, 344)
files.append(("src/main/psxsdk/libapi/c114.c",
              comment("LIBAPI", "C114", "_96_remove, the BIOS A(0x72) trampoline, after the module's two leading "
                      "data words") + '\n#include "include_asm.h"\n\n' + body(i(323, 344))))
# A23 A24 A25
for mod, n, what in (("A23", 345, "ReturnFromException, the BIOS B(0x17) trampoline"),
                     ("A24", 346, "ResetEntryInt, the BIOS B(0x18) trampoline"),
                     ("A25", 347, "HookEntryInt, the BIOS B(0x19) trampoline")):
    use(used_i, n, n)
    files.append((f"src/main/psxsdk/libapi/{mod.lower()}.c", stub("LIBAPI", mod, what, I[n - 1])))
# SETJMP
use(used_i, 348, 348)
files.append(("src/main/psxsdk/libc2/setjmp.c",
              comment("LIBC2", "SETJMP", "setjmp") + '\n#include "include_asm.h"\n\n' + I[348 - 1] + "\n"))
# INTR_VB: startIntrVSync, trapIntrVSync, setIntrVSync, then its memclr (sys_MemClear)
use(used_i, 349, 385); use(used_i, 399, 404)
files.append(("src/main/psxsdk/libetc/intr_vb.c",
              comment("LIBETC", "INTR_VB", "the VSync interrupt hooks (startIntrVSync, trapIntrVSync, "
                      "setIntrVSync and the module's memclr, sys_MemClear; SOTN libetc/intr_vb.c)")
              + '\n#include "common.h"\n\n' + FROM_I + "\nvoid InterruptCallback(void);\n\n"
              + body(i(349, 385) + [""] + i(399, 404))))
# INTR_DMA
use(used_i, 386, 398); use(used_i, 405, 458)
files.append(("src/main/psxsdk/libetc/intr_dma.c",
              comment("LIBETC", "INTR_DMA", "the DMA interrupt hooks (startIntrDMA, trapIntrDMA, setIntrDMA and "
                      "the module's memclr, sys_MemClear2; SOTN libetc/intr_dma.c)")
              + '\n#include "common.h"\n\n' + FROM_I + "\nvoid InterruptCallback(void);\n\n"
              + body(i(386, 397) + i(405, 458))))
# VMODE
use(used_i, 460, 468)
files.append(("src/main/psxsdk/libetc/vmode.c",
              comment("LIBETC", "VMODE", "SetVideoMode, GetVideoMode (SOTN libetc/vmode.c)")
              + '\n#include "common.h"\n\n' + FROM_I + "\n" + I[15 - 1] + "\n\n" + body(i(460, 468))))
# gap 1: SN runtime
use(used_i, 470, 482)
files.append(("src/main/psxsdk/libsn/73E98.c",
              gap_comment("gap1", "SN Systems runtime: PCopen, PCclose, PClseek (the PC file server), "
                          "__SN_ENTRY_POINT, __main and __do_global_dtors")
              + '\n#include "common.h"\n#include "include_asm.h"\n\n' + body(i(470, 482))))
# C57
use(used_i, 483, 483)
files.append(("src/main/psxsdk/libapi/c57.c",
              stub("LIBAPI", "C57", "InitHeap, the BIOS A(0x39) trampoline", I[483 - 1])))
# gap 2: SN runtime
use(used_i, 484, 512)
files.append(("src/main/psxsdk/libsn/7407C.c",
              gap_comment("gap2", "SN Systems runtime: PCread and _SN_read (the PC file server)")
              + '\n#include "common.h"\n#include "include_asm.h"\n\n' + body(i(484, 512))))
# SSEND
use(used_i, 513, 536)
files.append(("src/main/psxsdk/libsnd/ssend.c",
              comment("LIBSND", "SSEND", "SsEnd (SOTN libsnd/ssend.c)") + '\n#include "common.h"\n#include "sound.h"\n\n'
              + FROM_I + "\nvoid VSyncCallback(s32 a0);\nvoid InterruptCallback(void);\n\n" + body(i(513, 536))))
# SSINIT_C
use(used_i, 538, 542)
files.append(("src/main/psxsdk/libsnd/ssinit_c.c",
              comment("LIBSND", "SSINIT_C", "SsInit (the cold-start variant; SSINIT_H, SsInitHot, is SOTN's "
                      "libsnd/ssinit_h.c; docs/naming/libscan/ambiguous_resolutions.md)")
              + '\n#include "common.h"\n\n' + FROM_I + "\n" + "\n".join(i(9, 10)) + "\nvoid ResetCallback(void);\n\n"
              + body(i(538, 542))))
# SSINIT
use(used_i, 544, 581)
files.append(("src/main/psxsdk/libsnd/ssinit.c",
              comment("LIBSND", "SSINIT", "_SsInit (SOTN libsnd/ssinit.c)") + '\n#include "common.h"\n\n'
              + body(i(544, 581))))
# SSQUIT
use(used_i, 583, 585)
files.append(("src/main/psxsdk/libsnd/ssquit.c",
              comment("LIBSND", "SSQUIT", "SsQuit (SOTN libsnd/ssquit.c; docs/naming/libscan/ambiguous_resolutions.md)")
              + '\n#include "common.h"\n\n' + FROM_I + "\n" + I[11 - 1] + "\n\n" + body(i(583, 585))))
# SSSATTR
use(used_i, 587, 611)
files.append(("src/main/psxsdk/libsnd/scssattr.c",
              comment("LIBSND", "SSSATTR", "SsSetSerialAttr (SOTN's file for it is libsnd/scssattr.c)")
              + '\n#include "common.h"\n\n' + FROM_I + "\n" + I[20 - 1] + "\n\n" + body(i(587, 611))))

# every source line is used or knowingly dropped
drop_s = set(range(1, 9)) | {26, 27, 28, 30, 315, 316}
drop_i = set(range(1, 24)) | {39, 459, 469, 537, 543, 582, 586}
for n in range(1, len(S) + 1):
    assert n in used_s or n in drop_s or (n in (10, 20, 21, 22, 23, 24, 25, 70, 71, 72)), ("S", n, S[n - 1])
for n in range(1, len(I) + 1):
    assert n in used_i or n in drop_i or n == 398, ("I", n, I[n - 1])
for n in drop_s:
    assert S[n - 1] in ("", "/* --- Functions 0x8008008C - 0x800807A8 --- */") or S[n - 1].startswith("#") \
        or n == 315, (n, S[n - 1])
# ings2 head lines 1-23: includes (dropped), forward decls 9-11 (-> ssinit_c, ssquit), externs 14-20 (14 dup of
# system.c:926, 15 -> vmode, 16-18 -> intr, 19 -> vsync, 20 -> scssattr), banner 22
for n in drop_i:
    if n < 24:
        assert I[n - 1] == "" or I[n - 1].startswith(("#", "/*", "extern")), (n, I[n - 1])

IDS = [p[len("src/"):-2] for p, _ in files]
assert len(IDS) == 24, len(IDS)

if "--apply" in sys.argv:
    for path, text in files:
        p = os.path.join(ROOT, path)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        assert path in (SP, IP) or not os.path.exists(p), p
        open(p, "w", encoding="utf-8", newline="\n").write(text)
if "--ids" in sys.argv:
    print(" ".join(IDS))
elif "--apply" not in sys.argv:
    for path, text in files:
        print("=====", path)
        print(text if len(text) < 2500 else text[:1800] + "\n...\n" + text[-400:])
