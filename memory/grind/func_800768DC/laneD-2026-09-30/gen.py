"""Generate the SelWork-cluster full-file variant from tmp/laneD/v0.c (the rev-45 reviewed file).
usage: python3 tmp/laneD/gen.py <out.c> [flags...]"""
import sys, re

src = open("tmp/laneD/v0.c", encoding="utf-8").read()
out = sys.argv[1]
flags = set(sys.argv[2:])


def rep(old, new, count=1):
    global src
    n = src.count(old)
    if n != count:
        raise SystemExit(f"expected {count} of {old!r}, found {n}")
    src = src.replace(old, new)


STRUCT = open("tmp/laneD/struct_A.snip" if "no770" in flags else "tmp/laneD/struct.h", encoding="utf-8").read()

# ---- one SelWork type: replaces S_800747D8 (and, below, SelWork_800768DC) ----
m = re.search(r"typedef struct \{\n    u8 pad00\[0x10\];\n    union \{\n        s32 word10;.*?\} S_800747D8;\n\n#define MENU_800747D8 \(\(S_800747D8 \*\)D_800A36A0\)\n", src, re.S)
src = src[:m.start()] + STRUCT + src[m.end():]
m = re.search(r"/\* Shared select work area at D_800A36A0.*?#define SELWORK_800768DC \(\(SelWork_800768DC \*\)D_800A36A0\)\n\n", src, re.S)
src = src[:m.start()] + src[m.end():]
rep("#undef MENU_800747D8\n", "")
src = src.replace("SELWORK_800768DC", "SELWORK")
src = src.replace("MENU_800747D8", "SELWORK")
src = src.replace("S_800747D8", "SelWork")

# ---- func_800747D8 ----
rep("""    base = D_800A36A0;
    result = 0;
    if (*(s32 *)(base + 0x10) == 0) {
        sp10 = (input & 0xFFFF) | (input >> 16);
        ret = func_800692C0((u32 *)&sp10, 0, (s16 *)(base + 0x40), D_800A35D0[0]);""",
"""    base = SELWORK;
    result = 0;
    if (base->f10.word == 0) {
        sp10 = (input & 0xFFFF) | (input >> 16);
        ret = func_800692C0((u32 *)&sp10, 0, base->f40[0], D_800A35D0[0]);""")
rep("""s32 func_800747D8(u32 input) {
    u8 *base;""", """s32 func_800747D8(u32 input) {
    SelWork *base;""")
src = src.replace("->field34", "->f34")
src = src.replace("->field3C", "->f3C.half[0]")
src = src.replace("->field10.half10[i]", "->f10.half[i]")
for f in ("18", "38", "64", "65", "66", "67"):
    src = src.replace(f"->field{f}", f"->f{f}")
rep("if ((s16)SELWORK->f3C.half[0] >= 5)", "if (SELWORK->f3C.half[0] >= 5)")
rep("if ((s16)SELWORK->f3C.half[0] < 0)", "if (SELWORK->f3C.half[0] < 0)")
rep("state = (s16)SELWORK->f3C.half[0];", "state = SELWORK->f3C.half[0];")
rep("menu = (SelWork *)D_800A36A0;", "menu = SELWORK;", 2)
rep("work = (SelWork *)D_800A36A0;", "work = SELWORK;", 2)

src = re.sub(r"SELWORK->(f10|f14|f3C)\[", r"SELWORK->\1.half[", src)

# ---- whole-function bodies: tmp/laneD/b_<func>.c (or b_<func>_<variant>.c via flag func=variant) ----
import os
for fn in ["func_800747D8", "func_800768DC", "func_8007636C", "func_80075670", "func_8007526C", "func_80075830", "func_800770B8", "func_80077374"]:
    var = [f.split("=", 1)[1] for f in flags if f.startswith(fn + "=")]
    path = f"tmp/laneD/b_{fn}{'_' + var[0] if var else ''}.c"
    if not os.path.exists(path):
        continue
    body = open(path, encoding="utf-8").read()
    m = re.search(r"\n[a-z0-9 ]+%s\([^)]*\) \{.*?\n\}\n" % fn, src, re.S)
    src = src[:m.start() + 1] + body + src[m.end():]

if "no770" in flags:
    m = re.search(r"\ns32 func_800770B8\([^)]*\) \{.*?\n\}\n", src, re.S)
    src = src[:m.start() + 1] + 'INCLUDE_ASM("asm/funcs", func_800770B8);\n' + src[m.end():]
    src = re.sub(r"->(f1C|f20|f3C)\.half\[", r"->\1[", src)
    assert ".word" not in src.replace("f10.word", "").replace("f14.word", ""), "stray word view"
    m = re.search(r"/\* func_800747D8 - s10 \(forensics\).*?\*/\n", src, re.S)
    src = src[:m.start()] + ("/* func_800747D8: the duplicated `sound = 4;` below is claimed under\n"
                             " * .claude/rules/duplicated-statement-into-arms.md and carries its FAKE\n"
                             " * annotation inline; self-vet: memory/grind/func_800747D8/self_vet.md. */\n") + src[m.end():]
    rep("extern u8 D_8009BD20[][2];\nextern s16 D_800A35D0[2][2];\n", "extern s16 D_800A35D0[2][2];\n")
    rep("annotation inline; self-vet: memory/grind/func_800747D8/self_vet.md. */\nextern s16 D_800A35D0[2][2];\n",
        "annotation inline; self-vet: memory/grind/func_800747D8/self_vet.md. */\n"
        "/* 0x800A35D0: one {s16, s16} pair per player (two words,\n"
        " * asm/data/91C98.data.s:4279-4282), passed to func_800692C0 beside SelWork\n"
        " * f40[player]; func_800768DC indexes it by player * 4 (0x80076948/5C). */\n"
        "extern s16 D_800A35D0[2][2];\n")
    rep("}\nextern u8 D_8009BCE4[20];\n\nINCLUDE_ASM(\"asm/funcs\", func_800759D0);",
        "}\n/* 0x8009BCE4: 20 flag bytes indexed by entry id (asm/data/7D920.data.s:23732-23753;\n"
        " * func_800768DC lbu/sb at 0x80076B90/0x80076BA4). */\n"
        "extern u8 D_8009BCE4[20];\n\nINCLUDE_ASM(\"asm/funcs\", func_800759D0);")
    # one declaration per object in this TU
    rep("extern u8 *D_800A36A0;\nextern s16 D_800A35D0[2][2];\nextern void func_8005C650", "extern u8 *D_800A36A0;\nextern void func_8005C650")
    rep('INCLUDE_ASM("asm/funcs", func_800759D0);\nextern u8 D_8009BCE4[20];\n', 'INCLUDE_ASM("asm/funcs", func_800759D0);\n')
    rep("extern s8 D_800A35DC;\nextern u8 D_8009BCE4[20];\nextern s16 D_800A35D0[2][2];\nextern s32 g_gpu_ot_ptr;",
        "extern s8 D_800A35DC;\nextern u8 D_8009BD21;\nextern s32 g_gpu_ot_ptr;")

open(out, "w", encoding="utf-8", newline="\n").write(src)
