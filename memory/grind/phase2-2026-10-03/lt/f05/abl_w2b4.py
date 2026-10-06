#!/usr/bin/env python3
# FAKE ablations for w2b4 (run from the repo root with the batch applied): every FAKE in every moved body,
# each removed (its plain alternative) in a copy of the applied body. Writes tmp/w2/abl4/<name>.c +
# tmp/w2/abl4/list.txt for ../f08/run_abl_w2b3.ps1 -List tmp/w2/abl4/list.txt.
import os, re


def body(path, fn):
    s = open(path, encoding="utf-8").read()
    m = re.search(r"\n[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{" % fn, s)
    i = m.start() + 1
    return s[i:s.index("\n}\n", i) + 3]


def sub(b, a, c, n=1):
    if b.count(a) != n:
        raise SystemExit("expected %d x %r, found %d" % (n, a[:70], b.count(a)))
    return b.replace(a, c)


F68, FSYS = "src/main/368E4.c", "src/main/psxsdk/libgpu/sys.c"
A = []
# otr_if / otr_all: ClearOTagR's new_var FAKE ablated to 0 (both stores) and is gone
A.append(("ba4_scale", "func_80048BA4", F68, lambda b: sub(sub(b, "    scale = 0x1770;\n", ""), "* scale) >> 12;", "* 0x1770) >> 12;", 2)))
A.append(("718_p58", "func_80049718", F68, lambda b: re.sub(r"            \{\n                s32 \*p58 = &obj->unk58;\n                \*p58 = val58;\n            \}\n",
                                                            "            obj->unk58 = val58;\n", b)))
A.append(("718_side", "func_80049718", F68, lambda b: sub(sub(b, "            side = flags & 1;\n", ""),
                                                        "part = &player->unk_2C[19 + side];", "part = &player->unk_2C[19 + (flags & 1)];")))
A.append(("718_val58", "func_80049718", F68, lambda b: sub(b, "    val58 = 0;\n", "")))
A.append(("718_flags", "func_80049718", F68, lambda b: sub(sub(sub(sub(b, "    s32 side;\n", "    s32 side;\n    s32 f2;\n"),
                                                                        "            flags &= 0x7FFF;\n            side = flags & 1;\n            player = func_8004153C(flags >> 1);\n",
                                                                        "            f2 = flags & 0x7FFF;\n            side = f2 & 1;\n            player = func_8004153C(f2 >> 1);\n"),
                                                                    "            flags |= 0x8000;\n", "            flags = f2 | 0x8000;\n"),
                                                                "            /* SOTN: src/st/lib/e_shop.c:4621 @aa53500 */\n", "")))
A.append(("718_frame", "func_80049718", F68, lambda b: sub(sub(b, "            s32 frame = D_800EF980[arg0];\n", ""),
                                                         "obj->node.unk2 = frame * 2 + 1;", "obj->node.unk2 = D_800EF980[arg0] * 2 + 1;")))
A.append(("a2c_p58", "func_80049A2C", F68, lambda b: re.sub(r"    \{\n        s32 \*p58 = &obj->unk58;\n        \*p58 = player->unk_1A84;\n    \}\n",
                                                            "    obj->unk58 = player->unk_1A84;\n", b)))
A.append(("a2c_var6", "func_80049A2C", F68, lambda b: sub(sub(sub(b, "    u8 *new_var6;\n", ""), "    new_var6 = D_80099CC8;\n", ""),
                                                        "u8 *p = new_var6 + (arg0 * 2);", "u8 *p = &D_80099CC8[arg0 * 2];")))
A.append(("a2c_var8", "func_80049A2C", F68, lambda b: sub(sub(sub(b, "    s16 *new_var8;\n", ""),
                                                            "    new_var8 = D_800EF980;\n", ""),
                                                        "p_anim = new_var8 + temp_v1;", "p_anim = &D_800EF980[temp_v1];")))
# rev-w2b4: the carried a1_val holder (each read, then both) and func_80049C24's sweep


def a1v(which):
    def t(b):
        i = b.index("    a1_val = (*p_anim) * 2;\n")
        j = b.index("    a1_val = (*p_anim) * 2;\n", i + 1)
        if which in ("first", "both"):
            b = b[:i] + b[i:].replace("    a1_val = (*p_anim) * 2;\n", "", 1).replace(
                "    obj->node.unk2 = a1_val;\n", "    obj->node.unk2 = (*p_anim) * 2;\n", 1)
        if which in ("second", "both"):
            k = b.rindex("    a1_val = (*p_anim) * 2;\n")
            b = b[:k] + b[k:].replace("    a1_val = (*p_anim) * 2;\n", "", 1).replace(
                "    obj->node.unk2 = a1_val + 1;\n", "    obj->node.unk2 = (*p_anim) * 2 + 1;\n", 1)
        return b
    return t


A.append(("a2c_a1v1", "func_80049A2C", F68, a1v("first")))
A.append(("a2c_a1v2", "func_80049A2C", F68, a1v("second")))
A.append(("a2c_a1vb", "func_80049A2C", F68, a1v("both")))
A.append(("c24_tv0", "func_80049C24", F68, lambda b: sub(sub(sub(b, "    temp_v0 = tbl[count + 1];\n", ""),
                                                            "    var_s7 += temp_v0;\n", "    var_s7 += tbl[count + 1];\n"), "    s32 temp_v0;\n", "")))
A.append(("c24_s7", "func_80049C24", F68, lambda b: sub(b, "    var_s7 = arg0;\n    var_s7 += temp_v0;\n", "    var_s7 = arg0 + temp_v0;\n")))
A.append(("c24_s0s2", "func_80049C24", F68, lambda b: re.sub(
    r"        if \(var_s2 == var_s0\) \{\n( *)/\* FAKE: a no-op copy[^\n]*\n *var_s0 = var_s2;\n        \} else \{\n            var_s2 = 0;\n        \}\n",
    "        if (var_s2 != var_s0) {\n            var_s2 = 0;\n        }\n", b)))
A.append(("c24_hdr", "func_80049C24", F68, lambda b: sub(sub(b, "    hdr = ~var_s0;\n", ""), "    hdr = (u32)hdr >> 31;\n", "    hdr = (u32)~var_s0 >> 31;\n")))
A.append(("c24_v1", "func_80049C24", F68, lambda b: sub(sub(b, "    v1 = var_s3;\n    var_s3 += 4;\n", ""),
                                                        "    *(s32 *)v1 = hdr;\n", "    *(s32 *)var_s3 = hdr;\n    var_s3 += 4;\n")))
A.append(("a2c_nv2", "func_80049A2C", F68, lambda b: sub(sub(re.sub(r"    /\* FAKE: rot.vz's value[^*]*\*/\n", "", sub(b, "    s16 new_var2;\n", "")),
                                                            "    new_var2 = src[1];\n", ""), "obj->node.xf.rot.vz = new_var2;", "obj->node.xf.rot.vz = src[1];")))
A.append(("a2c_pad", "func_80049A2C", F68, lambda b: re.sub(r"    volatile u32 pre_pad\[2\];[^\n]*\n", "", b)))
os.makedirs("tmp/w2/abl4", exist_ok=True)
rows = []
for name, fn, path, t in A:
    p = "tmp/w2/abl4/%s.c" % name
    b0 = body(path, fn)
    b1 = t(b0)
    if b1 == b0:
        raise SystemExit("no change for " + name)
    open(p, "w", encoding="utf-8", newline="\n").write(b1)
    rows.append("%s %s %s" % (name, fn, p))
open("tmp/w2/abl4/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
print(len(rows), "candidates")
