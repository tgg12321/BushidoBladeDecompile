"""Generate island-form variants of func_8002DE20 candidate.c.

Replaces each of the 9 joined island statements with the SEPARATE header
statements of PsyQ 4.3 inline_o.h gte_ldv0 (:16-20), gte_rtv0 (:426-430),
gte_stlvnl (:904-909), copied from tmp/func_8002DE20/hdr/sh_inline_o.h
(sha256 76f28032...c6e47d == engine/gtemacro.py PINNED header_sha256).

usage: python3 mkvar.py <in.c> <out.c> --mem {paren,zero} --word {ph,real} [--vin {local,expr}]
  --mem paren : header text `($12)` (verbatim)
  --mem zero  : `0($12)` for the zero-offset memory operands only
  --word ph   : header DMPSX placeholder `.word 0x0000013f` (verbatim)
  --word real : post-DMPSX MVMVA word `.word 0x4A486012` (target bytes)
"""
import argparse
import re

CL = '"$12","$13","$14","$15","memory"'


def st(tmpl, inp=None):
    ins = f'"r"({inp})' if inp else ""
    return f'    __asm__ volatile ("{tmpl}": :{ins}:{CL});\n' if inp else \
           f'    __asm__ volatile ("{tmpl}": : :{CL});\n'


def ldv0(r1, mem):
    z = "" if mem == "paren" else "0"
    return (f"    /* gte_ldv0({r1}): inline_o.h:16-20 */\n"
            + st("move  $12,%0", r1)
            + st(f"lwc2  $0,{z}($12)")
            + st("lwc2  $1,4($12)"))


def rtv0(word):
    w = "0x0000013f" if word == "ph" else "0x4A486012"
    return ("    /* gte_rtv0(): inline_o.h:426-430 */\n"
            + st("nop   ") + st("nop   ") + st(f".word {w}"))


def stlvnl(r1, mem):
    z = "" if mem == "paren" else "0"
    return (f"    /* gte_stlvnl({r1}): inline_o.h:904-909 */\n"
            + st("move  $12,%0", r1)
            + st(f"swc2  $25,{z}($12)")
            + st("swc2  $26,4($12)")
            + st("swc2  $27,8($12)"))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("inp")
    ap.add_argument("out")
    ap.add_argument("--mem", choices=("paren", "zero"), required=True)
    ap.add_argument("--word", choices=("ph", "real"), required=True)
    ap.add_argument("--vin", choices=("local", "expr"), default="local")
    a = ap.parse_args()
    src = open(a.inp, newline="").read()
    blk = re.compile(r"    __asm__ volatile\(\n(?:        \"[^\n]*\n)+        : [^\n]*\);\n")
    vin = "vin" if a.vin == "local" else "&obj->unkF8"

    def rep(m):
        t = m.group(0)
        if "lwc2" in t:
            return ldv0(vin, a.mem)
        if ".word" in t:
            return rtv0(a.word)
        r = re.search(r'"r"\((.*)\) :', t).group(1)
        return stlvnl(r, a.mem)

    out, n = blk.subn(rep, src)
    assert n == 9, n
    if a.vin == "expr":
        out = out.replace("    s32 *vin;\n", "").replace("    vin = (s32 *)&obj->unkF8;\n", "")
    open(a.out, "w", newline="\n").write(out)
    print(f"{a.out}: {n} islands rewritten")


main()
