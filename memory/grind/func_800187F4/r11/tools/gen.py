"""Expand @gte_xxx(arg); markers in a template into inline_o.h GTE statements.

Usage: python3 gen.py template.c out.c [--joined]
  default: verbatim inline_o.h form (one __asm__ per header statement)
  --joined: each macro joined into one __asm__ (scorable by the current sandbox
            for macros not yet in engine/gtemacro.py PINNED)
"""
import re
import sys

CL = '"$12","$13","$14","$15","memory"'

# macro -> list of (instr, has_input)
MACROS = {
    "ldv0": [("move  $12,%0", True), ("lwc2  $0,($12)", False), ("lwc2  $1,4($12)", False)],
    "ldlvl": [("move  $12,%0", True), ("lwc2  $9,($12)", False), ("lwc2  $10,4($12)", False),
              ("lwc2  $11,8($12)", False)],
    "stlvnl": [("move  $12,%0", True), ("swc2  $25,($12)", False), ("swc2  $26,4($12)", False),
               ("swc2  $27,8($12)", False)],
    "stlvl": [("move  $12,%0", True), ("swc2  $9,($12)", False), ("swc2  $10,4($12)", False),
              ("swc2  $11,8($12)", False)],
    "ldlzc": [("move  $12,%0", True), ("mtc2  $12,$30", False)],
    "stlzc": [("move  $12,%0", True), ("swc2  $31,($12)", False)],
    "lddp": [("move  $12,%0", True), ("mtc2  $12,$8", False)],
    "nop": [("nop   ", False)],
    "rtv0tr": [("nop   ", False), ("nop   ", False), (".word 0x4A480012", False)],
    "sqr0": [("nop   ", False), ("nop   ", False), (".word 0x4AA00428", False)],
    "gpf0": [("nop   ", False), ("nop   ", False), (".word 0x4B90003D", False)],
    "gpl12": [("nop   ", False), ("nop   ", False), (".word 0x4BA8003E", False)],
}

PAT = re.compile(r'^(\s*)@gte_(\w+)\((.*)\);\s*$')


def expand(indent, name, arg, joined):
    if name == "Lzc":
        a, b = [x.strip() for x in arg.split(",", 1)]
        return (expand(indent, "ldlzc", a, False) + expand(indent, "nop", "", False) * 2 +
                expand(indent, "stlzc", b, False))
    stmts = MACROS[name]
    out = []
    if joined and name != "nop":
        body = "".join(f"\"{i.replace(',($12)', ',0($12)')}\\n\"" for i, _ in stmts)
        inp = f'"r"({arg})' if any(h for _, h in stmts) else ""
        out.append(f"{indent}__asm__ volatile({body}: :{inp if inp else ' '}:{CL});")
    else:
        for instr, has in stmts:
            inp = f'"r"({arg})' if has else ""
            out.append(f'{indent}__asm__ volatile ("{instr}": :{inp if inp else ' '}:{CL});')
    return out


def main():
    src, dst = sys.argv[1], sys.argv[2]
    joined = "--joined" in sys.argv
    lines = []
    for line in open(src).read().split("\n"):
        m = PAT.match(line)
        if m:
            lines.extend(expand(m.group(1), m.group(2), m.group(3), joined))
        else:
            lines.append(line)
    open(dst, "w", newline="\n").write("\n".join(lines))


main()
