#!/usr/bin/env python3
"""citems_selftest.py: regression check for citems.items / proto (any cwd; exit 1 on a failure).
Pins the K&R case: `T f(a, b) T1 a; T2 b; { ... }` is one 'func' item, and the items after it are
keyed (63D2C's func_80073C78 once swallowed func_80074220 / func_80074488, hiding them from l2diff)."""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from citems import items, proto

SRC = """\
#include "common.h"
typedef struct { s32 a; } S;
extern s32 g(s32);
int h(a);
INCLUDE_ASM("asm/funcs", func_80010000);

/* multi-line parameter declarations */
s32 func_80073C78(env, angle, mode)
    S *env;
    s16 angle, *p; /* comment; with a semicolon */
    s32 mode;
{
    return env->a;
}
void knr_one_line(arg0) S *arg0; {
    arg0->a = 0;
}
void knr_fptr(cb, x)
    int (*cb)();
    s16 x; s16 *unused;
{
    cb(x);
}
s32 func_80074220(s32 a) {
    return a;
}
void func_80074488(void)
{
}
s32 D_800A36A0[2] = { 1, 2 };
"""

WANT = [("pp", "#include"), ("decl", None), ("decl", None), ("decl", None), ("stub", "func_80010000"),
        ("func", "func_80073C78"), ("func", "knr_one_line"), ("func", "knr_fptr"), ("func", "func_80074220"),
        ("func", "func_80074488"), ("decl", None)]
PROTOS = {"func_80073C78": "s32 func_80073C78();", "knr_one_line": "void knr_one_line();",
          "knr_fptr": "void knr_fptr();", "func_80074220": "s32 func_80074220(s32 a);",
          "func_80074488": "void func_80074488(void);"}

its, lines = items(SRC)
got = [(i["kind"], i["name"]) for i in its]
fail = []
if got != WANT:
    fail.append(f"items: got {got}")
spans = {i["name"]: (i["start"], i["end"]) for i in its if i["kind"] == "func"}
if spans.get("func_80073C78") != (8, 14):
    fail.append(f"func_80073C78 span {spans.get('func_80073C78')} != (8, 14)")
for i in its:
    if i["kind"] == "func" and proto(lines, i) != PROTOS[i["name"]]:
        fail.append(f"proto {i['name']}: {proto(lines, i)!r}")
if its[3]["names"] != {"h"}:
    fail.append(f"`int h(a);` names {its[3]['names']}")
for f in fail:
    print("FAIL", f)
print("citems selftest:", "FAIL" if fail else "OK")
sys.exit(1 if fail else 0)
