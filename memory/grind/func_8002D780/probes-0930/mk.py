"""Build func_8002D780 candidates with verbatim inline_o.h statements from the retro-audit body."""
import sys
from pathlib import Path

src = Path("memory/grind/func_8002D780/rejected/retro-audit-2026-09-29.c").read_bytes().decode()
body = src[src.index("s32 func_8002D780("):]
CL = '"$12","$13","$14","$15","memory"'


def st(txt, op=None):
    if op is None:
        return f'__asm__ volatile ("{txt}": : :{CL});'
    return f'__asm__ volatile ("{txt}": :"r"({op}):{CL});'


def span(text, start_marker, end_marker):
    i0 = text.index(start_marker)
    i0 = text.rindex("\n", 0, i0) + 1
    i1 = text.index(end_marker, i0)
    return i0, i1


# island 1: from the first asm statement to the closing of `if (flag == 0)` block
i0, i1 = span(body, "__asm__ volatile(\n            \"addu $t4, %0, $zero\\n\"\n            \"lwc2", "    }\n\n    {\n        s32 y")
ind = " " * 8
isl1 = "".join(ind + l + "\n" for l in [
    "vout = (s32 *)(obj + 0x100);",
    "/* gte_ApplyRotMatrix(vin, vout) -- gtemac.h 4.3 :354-357 = inline_o.h 4.3 gte_ldv0 :16-20,",
    " * gte_rtv0 :426-430 (post-DMPSX word 0x4A486012 for the placeholder 0x0000013f),",
    " * gte_stlvnl :904-909 */",
    st("move  $12,%0", "vin"), st("lwc2  $0,($12)"), st("lwc2  $1,4($12)"),
    st("nop   "), st("nop   "), st(".word 0x4A486012"),
    st("move  $12,%0", "vout"), st("swc2  $25,($12)"), st("swc2  $26,4($12)"), st("swc2  $27,8($12)"),
])
body = body[:i0] + isl1 + body[i1:]

i0, i1 = span(body, "__asm__ volatile(\n                    \"addu $t4, %0, $zero\\n\"\n                    \"mtc2", "lzcr = sp_var;")
i1 = body.rindex("\n", 0, i1) + 1
ind = " " * 16
lzc = "".join(ind + l + "\n" for l in [
    "/* gte_Lzc(m, &sp_var) -- gtemac.h 4.3 :174-178 = inline_o.h 4.3 gte_ldlzc :207-210,",
    " * gte_nop :1095-1097 (x2), gte_stlzc :1074-1077 */",
    st("move  $12,%0", "m"), st("mtc2  $12,$30"), st("nop   "), st("nop   "),
    st("move  $12,%0", "&sp_var"), st("swc2  $31,($12)"),
])
body = body[:i0] + lzc + body[i1:]
assert "addu $t4" not in body, "leftover"
out = sys.argv[1] if len(sys.argv) > 1 else "tmp/D780/v1.c"
Path(out).write_bytes(body.encode())
print("wrote", out, body.count("__asm__"))
