#!/usr/bin/env python3
# Ablations for w2b7 (run from the repo root with the batch applied): the carried FAKEs of the moved bodies
# not measured by an earlier batch's script, and the in-place sweep of their holders / aliases. Writes
# tmp/w2/abl7/<name>.c + list.txt for ../f08/run_abl_w2b3.ps1 -List tmp/w2/abl7/list.txt.
import os, re, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "fzz"))
from abl_w2b5 import body, sub, inline, drop_fake

F2B, F9F, F368, F315, F31D, F350, F32D, F3AB = ("src/main/2B344.c", "src/main/9F9C.c", "src/main/368E4.c",
    "src/main/31548.c", "src/main/31D3C.c", "src/main/35000.c", "src/main/32D04.c", "src/main/3AB48.c")
A = []


# ---- carried FAKEs
def e6d8_shift(b):
    b = sub(b, "        for (j = 0; j < 0x1F; j++) {\n", "        for (j = 0; j < 0x1F; j++, bits <<= 1) {\n")
    b = re.sub(r"            if \(col < 0\) \{\n( +)/\* FAKE: the column shift.*?\*/\n +bits <<= 1;\n +continue;\n",
               "            if (col < 0) {\n                continue;\n", b, flags=re.S)
    return sub(b, "            }\n            bits <<= 1;\n        }\n    }\n", "            }\n        }\n    }\n")


A.append(("e6d8_shift", "func_8003E6D8", F2B, e6d8_shift))
A.append(("4800_rec", "func_80044800", F350, lambda b: sub(sub(sub(sub(b, "                ent = rec;\n", ""),
    "                rec = (Unk800A9CF8Entry *)((Unk800A6690Rec *)D_800A9CF8.unk10 + i);\n",
    "                ent = (Unk800A9CF8Entry *)((Unk800A6690Rec *)D_800A9CF8.unk10 + i);\n"),
    "                rec = ent;\n", ""), "                rec->node.", "                ent->node.", 7)))
A.append(("75a4_base", "func_800475A4", F368, lambda b: re.sub(r"\bbase->", "g_cam_bone_data2.", sub(sub(b,
    "    base = &g_cam_bone_data2;\n", ""), "        *temp = base;\n", "        *temp = &g_cam_bone_data2;\n"))))
A.append(("75a4_neg", "func_800475A4", F368, lambda b: sub(b, "        s16 neg = -ratan2(result.vy, computed);\n        base->xf.rot.vx = neg;\n",
    "        base->xf.rot.vx = -ratan2(result.vy, computed);\n")))
A.append(("7a90_w1", "func_80047A90", F368, lambda b: re.sub(r"    do \{\n( +)/\* FAKE: loop-note ref weighting lifts a3's.*?\*/\n +a3 = 0;\n    \} while \(0\);\n",
    "    a3 = 0;\n", b, flags=re.S)))
A.append(("7a90_w2", "func_80047A90", F368, lambda b: re.sub(r"    do \{\n( +)/\* FAKE: loop-note ref weighting lifts pa2.*?\*/\n +pa2 = pt2;\n    \} while \(0\);\n",
    "    pa2 = pt2;\n", b, flags=re.S)))
A.append(("7a90_w3", "func_80047A90", F368, lambda b: re.sub(r"    do \{\n( +)/\* FAKE: loop-note ref weighting keeps pt1.*?\*/\n +pt3 = pt1 \+ 0x11;\n    \} while \(0\);\n",
    "    pt3 = pt1 + 0x11;\n", b, flags=re.S)))
A.append(("7a90_tail", "func_80047A90", F368, lambda b: re.sub(
    r"        \*\(s32 \*\)\(\(s8 \*\)D_800EF800 \+ a3\) = v1;\n        pa1\+\+;\n        a3 \+= 4;\n        pa2\+\+;\n    \} else \{\n.*?\*/\n        pa1\+\+;\n        a3 \+= 4;\n        pa2\+\+;\n    \}\n",
    "        *(s32 *)((s8 *)D_800EF800 + a3) = v1;\n    }\n    pa1++;\n    a3 += 4;\n    pa2++;\n", b, flags=re.S)))
A.append(("3f08_temp", "func_80023F08", F9F, None))   # the four-role local: measured by its label (see below)
A.pop()
A.append(("3f08_twist", "func_80023F08", F9F, lambda b: sub(b, "twist = -ratan2(m1.m[0][0], m1.m[2][0]) + ratan2(m2.m[0][0], m2.m[2][0]);",
    "twist = ratan2(m2.m[0][0], m2.m[2][0]) - ratan2(m1.m[0][0], m1.m[2][0]);")))
A.append(("3f08_range", "func_80023F08", F9F, lambda b: sub(b, "rec->unk_A6 >= rec->unk_40)", "rec->unk_40 <= rec->unk_A6)")))
# ---- sweep
A.append(("1eb0_cam", "func_80041EB0", F31D, lambda b: inline(b, "    cam = &D_800A9B28;\n", "cam->", "D_800A9B28.").replace("    VECTOR *cam;\n", "")
         if False else re.sub(r"\bcam->", "D_800A9B28.", sub(sub(b, "    cam = &D_800A9B28;\n", ""), "    VECTOR *cam;\n", ""))))
A.append(("1eb0_fp", "func_80041EB0", F31D, lambda b: re.sub(r"\bfp_ptr\b", "D_800F62E0", sub(sub(b, "    fp_ptr = D_800F62E0;\n", ""), "    Unk800F62E0Rec *fp_ptr;\n", ""))))
A.append(("1eb0_cross", "func_80041EB0", F31D, lambda b: sub(b, """            s32 sin_val = rsin(angle);
            s32 cross = (cos_val * dz + sin_val * dx) >> 12;
            tbl->light[1].pitch = (s16)-ratan2(dy, cross);
""", """            tbl->light[1].pitch = (s16)-ratan2(dy, (cos_val * dz + rsin(angle) * dx) >> 12);
""")))
A.append(("4504_s0", "func_80044504", F32D, lambda b: re.sub(r"\bs0\b", "&D_80101BD0", sub(b, "    MATRIX *s0 = &D_80101BD0;\n", ""))))
A.append(("4504_v1", "func_80044504", F32D, lambda b: sub(b, """    {
        s32 v1;
        if (D_800A3790 & 8) {
            v1 = func_8003E2C8();
        } else {
            v1 = 0x7FFFFFFF;
        }
        *(s32 *)0x1F800010 = v1;
    }
""", """    if (D_800A3790 & 8) {
        *(s32 *)0x1F800010 = func_8003E2C8();
    } else {
        *(s32 *)0x1F800010 = 0x7FFFFFFF;
    }
""")))
A.append(("4504_v0", "func_80044504", F32D, lambda b: sub(b, """    {
        s32 v0 = func_8003F268();
        if (v0 != 0) {
            v0 = 0xBE;
        } else {
            v0 = func_80046E7C();
            if (v0 != 0) {
                v0 = 0x182;
            } else {
                v0 = 0xBE;
            }
        }
        *(s32 *)0x1F800018 = v0;
    }
""", """    if (func_8003F268() != 0) {
        *(s32 *)0x1F800018 = 0xBE;
    } else if (func_80046E7C() != 0) {
        *(s32 *)0x1F800018 = 0x182;
    } else {
        *(s32 *)0x1F800018 = 0xBE;
    }
""")))
A.append(("4f68_v3", "func_80054F68", F3AB, lambda b: sub(sub(sub(sub(b, "    v3 = (s32)g_gpu_ot_ptr;\n", ""), "    D_800A3808 = v3;\n", "    D_800A3808 = (s32)g_gpu_ot_ptr;\n"),
    "    D_800A378C = (u32 *)(v3 + 0x10);\n", "    D_800A378C = (u32 *)((s32)g_gpu_ot_ptr + 0x10);\n"), "    s32 v3;\n", "")))
A.append(("4f68_s0", "func_80054F68", F3AB, lambda b: sub(b, "    s0 = func_8005490C();\n    func_800444E0();\n    return s0;\n",
    "    {\n        s32 r = func_8005490C();\n        func_800444E0();\n        return r;\n    }\n").replace("    s32 s0;\n", "")))
A.append(("4800_angle", "func_80044800", F350, lambda b: inline(b.replace("    s32 angle;\n", ""), "            angle = rec->unk5C;\n", "angle", "rec->unk5C")))
A.append(("4800_cs", "func_80044800", F350, lambda b: sub(b, """            cz = cos_val * sv.vz;
            sin_val = Judge[angle & 0xFFF];
            sz = sin_val * sv.vz;
            cx = cos_val * sv.vx;
            sx = sin_val * sv.vx;
            rec->node.xf.mat.t[0] += (sz + cx) >> 12;
            rec->node.xf.mat.t[1] += sv.vy;
            rec->node.xf.mat.t[2] += (cz - sx) >> 12;
""", """            sin_val = Judge[angle & 0xFFF];
            rec->node.xf.mat.t[0] += (sin_val * sv.vz + cos_val * sv.vx) >> 12;
            rec->node.xf.mat.t[1] += sv.vy;
            rec->node.xf.mat.t[2] += (cos_val * sv.vz - sin_val * sv.vx) >> 12;
""")))
A.append(("4800_last", "func_80044800", F350, lambda b: inline(b.replace("    s32 last;\n", ""), "                last = D_800A9CF8.unk2 - 1;\n", "last", "D_800A9CF8.unk2 - 1")))
A.append(("304c_lim", "func_8002304C", F9F, lambda b: inline(b.replace("  s32 lim;\n", ""), "  lim = 0x1F8002B8;\n", "lim", "0x1F8002B8")))
A.append(("304c_d", "func_8002304C", F9F, lambda b: inline(b.replace("  s16 *scratch_d;\n", ""), "  scratch_d = (s16 *) 0x1F8001D0;\n", "scratch_d", "(s16 *) 0x1F8001D0")))
A.append(("7a90_jb", "func_80047A90", F368, lambda b: re.sub(r"\bjb\[", "Judge[", sub(sub(b, "    jb = Judge;\n", ""), "    s16 *jb;\n", ""))))
A.append(("e6d8_cam", "func_8003E6D8", F2B, lambda b: sub(b, """    {
        Unk80101DF0Xform *cam = &D_80101DF0.xf;
        pos[0] = -cam->rot.vx;
        pos[1] = -cam->rot.vy;
        pos[2] = -cam->rot.vz;
        func_800620B8(pos, cam->mat.t);
    }
""", """    pos[0] = -D_80101DF0.xf.rot.vx;
    pos[1] = -D_80101DF0.xf.rot.vy;
    pos[2] = -D_80101DF0.xf.rot.vz;
    func_800620B8(pos, D_80101DF0.xf.mat.t);
""")))
A.append(("3f08_first", "func_80023F08", F9F, lambda b: sub(sub(sub(b, "    first = extra + *rec->unk_54;\n", ""),
    "    cur_frame = first + rec->unk_40;\n    next_frame = first + next;\n",
    "    cur_frame = extra + *rec->unk_54 + rec->unk_40;\n    next_frame = extra + *rec->unk_54 + next;\n"), "    s32 first;\n", "")))
A.append(("3f08_perp", "func_80023F08", F9F, lambda b: inline(b.replace("    s32 perp;\n", ""), "        perp = ang[0] + 0x400;\n", "perp", "ang[0] + 0x400")))
A.append(("3f08_bones", "func_80023F08", F9F, lambda b: sub(sub(b, "MATRIX **bones = game_GetPlayerData(arg0);\n", ""),
    "(Vec3i32 *)bones[0x12]->t", "(Vec3i32 *)((MATRIX **)game_GetPlayerData(arg0))[0x12]->t")))


def f08_temp(b):
    """the four-role temp split into one local per value"""
    anchors = ["temp = (rec->unk_58[2] >> 4) * 0x88;", "temp = (rec->unk_1D8 - rec->unk_1C8.vy) & 0xFFF;",
               "temp = (rec->unk_14C * rec->unk_44) / 24576;", "temp = (rec->unk_24.held & 0x1000) ? 1"]
    idx = [b.index(a) for a in anchors] + [None]
    head = b[:idx[0]]
    parts = [b[idx[k]:idx[k + 1]] if idx[k + 1] else b[idx[k]:] for k in range(4)]
    out = head.replace("    s32 temp;\n", "    s32 t1, t2, t3, t4;\n", 1)
    for k, part in enumerate(parts):
        if k == 3:   # the last role ends at the second rec->unk_134 update
            j = part.index("rec->unk_134.vz +=")
            j = part.index("\n", j)
            out += re.sub(r"\btemp\b", "t4", part[:j]) + part[j:]
        else:
            out += re.sub(r"\btemp\b", "t%d" % (k + 1), part)
    return out


A.append(("3f08_temp", "func_80023F08", F9F, f08_temp))


def f08_gotos(b):
    pre = ("    if (rec->unk_86 == rec->unk_88 && rec->unk_8A != 0) {\n        if (func_8002798C(rec) == 0) {\n"
           "            goto clear_8c;\n        }\n        goto set_8c;\n    }\n")
    b = sub(b, pre, "")
    m = re.search(r"\*/\n    if \((.*?)\) \{\n    set_8c:\n", b, flags=re.S)
    cond = m.group(1)
    b = b[:m.start()] + "*/\n    if ((rec->unk_86 == rec->unk_88 && rec->unk_8A != 0) ? func_8002798C(rec) != 0 : (" + cond + ")) {\n" + b[m.end():]
    return sub(b, "    } else {\n    clear_8c:\n", "    } else {\n")


A.append(("3f08_gotos", "func_80023F08", F9F, f08_gotos))
A.append(("3f08_mask", "func_80023F08", F9F, lambda b: sub(b, "    u32 mask;\n", "    s32 mask;\n") if "    u32 mask;\n" in b else
         sub(b, "            u32 mask;\n", "            s32 mask;\n")))
A.append(("3f08_state1", "func_80023F08", F9F, lambda b: sub(b, "        s32 state = rec->unk_6A;\n\n        if (rec->unk_7A != 0 && state == 6) {",
    "\n        if (rec->unk_7A != 0 && rec->unk_6A == 6) {") if "        s32 state = rec->unk_6A;\n\n        if (rec->unk_7A" in b else b))
A.append(("3f08_state2", "func_80023F08", F9F, lambda b: sub(b, "        s32 state = rec->unk_6A;\n\n        if (state == 8 || state == 0x22) {",
    "\n        if (rec->unk_6A == 8 || rec->unk_6A == 0x22) {") if "        s32 state = rec->unk_6A;\n\n        if (state == 8" in b else b))
A.append(("304c_c", "func_8002304C", F9F, lambda b: inline(b.replace("  s16 *scratch_c = (s16 *) 0x1F8001C0;\n", "  s16 *scratch_c;\n  scratch_c = (s16 *) 0x1F8001C0;\n").replace("  s16 *scratch_c;\n", ""),
    "  scratch_c = (s16 *) 0x1F8001C0;\n", "scratch_c", "(s16 *) 0x1F8001C0")))


def main():
    only = set(sys.argv[1:])
    os.makedirs("tmp/w2/abl7", exist_ok=True)
    rows = []
    for name, fn, path, t in A:
        if only and name not in only:
            continue
        p = "tmp/w2/abl7/%s.c" % name
        b0 = body(path, fn)
        try:
            b1 = t(b0)
        except SystemExit as e:
            print("SKIP %s: %s" % (name, e))
            continue
        if b1 == b0:
            print("SKIP %s: no change" % name)
            continue
        open(p, "w", encoding="utf-8", newline="\n").write(b1)
        rows.append("%s %s %s" % (name, fn, p))
    open("tmp/w2/abl7/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
    print(len(rows), "candidates")


if __name__ == "__main__":
    main()
