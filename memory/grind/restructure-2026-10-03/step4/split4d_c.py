#!/usr/bin/env python3
"""Restructure step 4d, split C: the tail of src/main/psxsdk/libgpu/sys.c (as committed at 2e14ce027)
-> LIBAPI C73, the 37 LIBGTE modules (GEO_00 .. PATCHGTE) and LIBCD EVENT, one file per LIBSCAN
module; sys.c keeps exactly LIBGPU SYS (0x8007AE7C..0x8007DF10).

Source lines move verbatim; only declarations a part needs to compile are added or moved. Dropped:
the old display.c's unused typedef block before ratan2 (NULL, Vec2s16..MATRIX, GameObj)."""
import os, sys

ROOT = os.path.dirname(os.path.abspath(__file__))
while not os.path.exists(os.path.join(ROOT, "bb2.ld")):  # the repo root, from tmp/ or the banked copy
    ROOT = os.path.dirname(ROOT)
SYS = os.path.join(ROOT, "src/main/psxsdk/libgpu/sys.c")
t = open(SYS, encoding="utf-8", newline="").read()
assert "\r" not in t
L = t.split("\n")
assert L[-1] == ""
L = L[:-1]


def ln(a, b=None):
    return L[a - 1:(a if b is None else b)]


def at(n, s):
    assert L[n - 1].startswith(s), (n, L[n - 1], s)


at(1, "/* PsyQ 4.0 LIBGPU SYS:")
at(4, "/* One file across the old gpu.c|display.c cut")
at(6, " * EVENT (.text 0x8007DF10..0x8008008C), the rest of the old display.c. */")
assert ln(7, 11) == ['#include "common.h"', '#include "include_asm.h"', '#include "bios.h"',
                     '#include "gpu.h"', '#include "psx.h"']
assert L[16 - 1] == "extern void DeliverEvent(s32, s32);"
assert L[1043 - 1] == "extern s32 printf();"
at(1145, "void memset(u8 *a0, u8 a1, s32 a2) {")
assert L[1150 - 1] == "}" and len(L) == 1591
at(1151, "BIOS_A_FUNCTION(GPU_cw, 0x49);")
assert L[1152 - 1] == "extern s32 sin_1(s32);"
at(1153, "s32 rsin(")
assert ln(1161, 1164) == ["extern s16 rsin_tbl[];", "extern s16 g_sin_lut_q3[];", "extern s16 g_cos_lut_q2[];",
                          "extern s16 g_cos_lut_q4[];"]
at(1166, "s32 sin_1(")
at(1178, "s32 rcos(")
at(1195, "/* D_8007E08C: the two leading words of LIBGTE module MSC00")
at(1410, "#define NULL ((void *)0)")
at(1447, "} GameObj;")
assert L[1448 - 1] == "extern s16 ratan_tbl[];"
at(1452, "s32 ratan2(")
assert L[1497 - 1] == "}" and L[1498 - 1] == "__asm__(" and L[1504 - 1] == '    "glabel _patch_gte\\n"'
assert L[1552 - 1] == ");"
at(1553, "extern s32 CdReset(s32);")
at(1565, "s32 CdInit(void) {")
assert L[1591 - 1] == "}"

# (module, first line, last line, function list, kind); kinds: asm (INCLUDE_ASM / asm blocks), c
GTE = [
    ("GEO_00", 1152, 1177, "rsin and sin_1", "c"),
    ("GEO_01", 1178, 1194, "rcos", "c"),
    ("MSC00", 1195, 1205, "InitGeom, after the module's two leading data words D_8007E08C", "asm"),
    ("MSC01", 1206, 1206, "SquareRoot0", "asm"),
    ("MSC06", 1207, 1224, "LoadAverage12, LoadAverage0, LoadAverageShort12, LoadAverageShort0, "
                          "LoadAverageByte and LoadAverageCol", "asm"),
    ("MSC09", 1225, 1225, "SquareRoot12", "asm"),
    ("MTX_000", 1226, 1233, "MulMatrix0", "asm"),
    ("MTX_003", 1234, 1234, "CompMatrix", "asm"),
    ("MTX_004", 1235, 1242, "ApplyMatrixLV", "asm"),
    ("MTX_005", 1243, 1250, "ApplyRotMatrix", "asm"),
    ("MTX_00A", 1251, 1258, "ScaleMatrixL", "asm"),
    ("MTX_01", 1259, 1265, "ApplyRotMatrixLV", "asm"),
    ("MTX_03", 1266, 1271, "MulMatrix", "asm"),
    ("MTX_04", 1272, 1280, "MulMatrix2", "asm"),
    ("MTX_05", 1281, 1286, "ApplyMatrix", "asm"),
    ("MTX_08", 1287, 1302, "ScaleMatrix", "asm"),
    ("MTX_09", 1303, 1303, "SetRotMatrix", "asm"),
    ("MTX_11", 1304, 1304, "SetColorMatrix", "asm"),
    ("MTX_12", 1305, 1305, "SetTransMatrix", "asm"),
    ("REG04", 1306, 1306, "ReadSZfifo3", "asm"),
    ("REG09", 1307, 1308, "ReadGeomScreen", "asm"),
    ("REG10", 1309, 1316, "SetBackColor", "c"),
    ("REG11", 1317, 1324, "SetFarColor", "c"),
    ("REG12", 1325, 1326, "SetGeomOffset", "asm"),
    ("REG13", 1327, 1327, "SetGeomScreen (Sony's hand-written asm module, padded to 16 bytes; owner ruling Q104)",
     "asm"),
    ("SMP_00", 1328, 1352, "LightColor, DpqColorLight, DpqColor3, Intpl, Square12, Square0, AverageZ3, "
                           "AverageZ4, OuterProduct12, OuterProduct0 and Lzc", "asm"),
    ("SMP_02", 1353, 1354, "RotTransPers", "asm"),
    ("SMP_03", 1355, 1360, "RotTransPers3", "asm"),
    ("SMP_04", 1361, 1362, "RotTrans", "asm"),
    ("CMB_00", 1363, 1367, "RotTransPers4", "asm"),
    ("FGO_01", 1368, 1374, "RotMatrix", "asm"),
    ("FGO_03", 1375, 1384, "RotMatrixZYX", "asm"),
    ("FGO_04", 1385, 1394, "RotMatrixX", "asm"),
    ("FGO_05", 1395, 1401, "RotMatrixY", "asm"),
    ("FGO_06", 1402, 1409, "RotMatrixZ", "asm"),
    ("RATAN", 1448, 1497, "ratan2", "c"),
    ("PATCHGTE", 1498, 1552, "_patch_gte and the words it copies (D_8007FF44 up to the module end)", "asm"),
]
SPANS = {  # LIBSCAN verbatim starts (docs/naming/libscan/matches.json); the next start ends a module
    "C73": 0x8007DF10, "GEO_00": 0x8007DF20, "GEO_01": 0x8007DFEC, "MSC00": 0x8007E08C, "MSC01": 0x8007E11C,
    "MSC06": 0x8007E1AC, "MSC09": 0x8007E43C, "MTX_000": 0x8007E4DC, "MTX_003": 0x8007E5EC,
    "MTX_004": 0x8007E74C, "MTX_005": 0x8007E8AC, "MTX_00A": 0x8007E8DC, "MTX_01": 0x8007EA0C,
    "MTX_03": 0x8007EB4C, "MTX_04": 0x8007EC5C, "MTX_05": 0x8007ED6C, "MTX_08": 0x8007EDBC,
    "MTX_09": 0x8007EEEC, "MTX_11": 0x8007EF1C, "MTX_12": 0x8007EF4C, "REG04": 0x8007EF6C,
    "REG09": 0x8007EF8C, "REG10": 0x8007EF9C, "REG11": 0x8007EFBC, "REG12": 0x8007EFDC,
    "REG13": 0x8007EFFC, "SMP_00": 0x8007F00C, "SMP_02": 0x8007F21C, "SMP_03": 0x8007F24C,
    "SMP_04": 0x8007F2AC, "CMB_00": 0x8007F2DC, "FGO_01": 0x8007F35C, "FGO_03": 0x8007F5EC,
    "FGO_04": 0x8007F87C, "FGO_05": 0x8007FA1C, "FGO_06": 0x8007FBBC, "RATAN": 0x8007FD5C,
    "PATCHGTE": 0x8007FEDC, "EVENT": 0x8007FF7C, "_END": 0x8008008C}
ORDER = list(SPANS)
SPAN = "a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3"
FROM = "/* Declarations from the file this module was split from (src/main/psxsdk/libgpu/sys.c, ex display.c). */"


def comment(lib, mod, what):
    st, en = SPANS[mod], SPANS[ORDER[ORDER.index(mod) + 1]]
    s = f"/* PsyQ 4.0 {lib} {mod}: {what}. .text 0x{st:08X}..0x{en:08X}, {SPAN}. */"
    out, cur = [], ""
    for w in s.split(" "):  # wrap at 100 columns
        if len(cur) + len(w) + 1 > 100:
            out.append(cur)
            cur = " *"
        cur = (cur + " " + w) if cur else w
    return "\n".join(out + [cur])


def strip_blank_tail(body):
    while body and body[-1].strip() == "":
        body = body[:-1]
    return body


files, used = [], set()
files.append(("src/main/psxsdk/libapi/c73.c",
              comment("LIBAPI", "C73", "GPU_cw, the BIOS A(0x49) trampoline") + "\n"
              "#define INCLUDE_ASM_USE_MACRO_INC 1\n#include \"include_asm.h\"\n#include \"bios.h\"\n\n"
              + L[1151 - 1] + "\n"))
used.add(1151)
for mod, a, b, what, kind in GTE:
    body = strip_blank_tail(ln(a, b))
    used |= set(range(a, b + 1))
    head = comment("LIBGTE", mod, what) + "\n"
    if mod == "GEO_00":  # rcos's two tables go with rcos; rsin_tbl is read by both modules
        body = [l for l in body if l not in ("extern s16 g_cos_lut_q2[];", "extern s16 g_cos_lut_q4[];")]
        assert len(body) == len(strip_blank_tail(ln(a, b))) - 2
    if kind == "asm":
        head += '#include "include_asm.h"\n'
    else:
        head += '#include "common.h"\n'
    head += "\n"
    if mod == "GEO_01":
        head += FROM + "\n" + "\n".join(["extern s16 rsin_tbl[];", "extern s16 g_cos_lut_q2[];",
                                         "extern s16 g_cos_lut_q4[];"]) + "\n\n"
    files.append((f"src/main/psxsdk/libgte/{mod.lower()}.c", head + "\n".join(body) + "\n"))
ev = strip_blank_tail(ln(1553, 1591))
used |= set(range(1553, 1592))
files.append(("src/main/psxsdk/libcd/event.c",
              comment("LIBCD", "EVENT", "CdInit and its default callbacks def_cbsync, def_cbready, def_cbread")
              + "\n#include \"common.h\"\n\n" + FROM + "\n" + L[16 - 1] + "\n" + L[1043 - 1] + "\n\n"
              + "\n".join(ev) + "\n"))
IDS = [p[len("src/"):-2] for p, _ in files]
assert len(IDS) == 39

dropped = set(range(1410, 1448))
for i in range(1151, 1592):
    assert i in used or i in dropped, i

SYS_TOP = ['/* PsyQ 4.0 LIBGPU SYS: the GPU system layer (ResetGraph .. memset; $Id: sys.c,v 1.129). .text',
           ' * 0x8007AE7C..0x8007DF10, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106',
           ' * D3. */',
           '/* One file across the old gpu.c|display.c cut at 0x8007B244, which was mid-module (Q106 D3). */']
assert ln(1, 3) == SYS_TOP[:3]
sys_ = SYS_TOP + ['#include "common.h"'] + ln(10, 11) + [l for i, l in enumerate(L[11:1150], 12) if i != 16]

if "--apply" in sys.argv:
    for path, text in files:
        p = os.path.join(ROOT, path)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        assert not os.path.exists(p), p
        open(p, "w", encoding="utf-8", newline="\n").write(text)
    open(SYS, "w", encoding="utf-8", newline="\n").write("\n".join(sys_) + "\n")
if "--ids" in sys.argv:
    print(" ".join(IDS))
else:
    for path, text in files:
        print("=====", path); print(text)
    print("===== sys head"); print("\n".join(sys_[:20])); print("..."); print("\n".join(sys_[-8:]))
