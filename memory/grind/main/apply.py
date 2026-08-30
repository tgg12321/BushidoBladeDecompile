#!/usr/bin/env python3
"""s33 apply: NEW CHASSIS (asm-until-matched). src/ings.c now carries
`INCLUDE_ASM("asm/funcs", main);` instead of a C body for main, and
func_80016E60 is itself INCLUDE_ASM (no C definition to widen).
Edits:
  1. func_80016A8C signature -> (u8 *arg0, u8 *arg1, s32 arg2)   [still C]
  2. replace the INCLUDE_ASM("asm/funcs", main); line with candidate.c's body
Preserves LF."""
SRC = "src/ings.c"
CAND = "memory/grind/main/candidate.c"

src = open(SRC, "rb").read().decode("utf-8").split("\n")
cand = open(CAND, "rb").read().decode("utf-8").split("\n")

mstart = next(i for i, l in enumerate(cand) if l.startswith("void main(void)"))
body = cand[mstart:]
while body and body[-1] == "":
    body.pop()

n = 0
for i, l in enumerate(src):
    if l == "void func_80016A8C(u8 *arg0) {":
        src[i] = "void func_80016A8C(u8 *arg0, u8 *arg1, s32 arg2) {"
        n += 1

s = next(i for i, l in enumerate(src) if l.strip() == 'INCLUDE_ASM("asm/funcs", main);')
out = src[:s] + body + src[s+1:]
open(SRC, "wb").write("\n".join(out).encode("utf-8"))
print(f"applied: sig-widen {n}; INCLUDE_ASM main at line {s+1} -> {len(body)} candidate lines")
