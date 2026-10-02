"""mk.py <variant> : write tmp/camera_CalcAngles/types/<variant>/text1b.c and inc/code6cac.h
(the two transform-node records typed as Unk80101DF0Record). Variants select per-function spellings."""
import os, sys

V = sys.argv[1]
D = f"tmp/camera_CalcAngles/types/{V}"
os.makedirs(D + "/inc", exist_ok=True)

def rep(s, a, b, n=1):
    assert s.count(a) == n, (a[:60], s.count(a))
    return s.replace(a, b)

h = open("include/code6cac.h", encoding="utf-8").read()
h = rep(h, "    u8 unk2[6];            /* +0x02 */\n",
        "    s16 unk2;              /* +0x02 */\n"
        "    s16 unk4;              /* +0x04 */\n"
        "    s16 unk6;              /* +0x06 */\n")
h = rep(h, "extern Unk80101DF0Record D_800FF638;\n",
        "extern Unk80101DF0Record D_800FF638;\n"
        "extern Unk80101DF0Record g_cam_bone_data2;\n"
        "extern Unk80101DF0Record D_800EF070;\n")
open(D + "/inc/code6cac.h", "w", encoding="utf-8", newline="\n").write(h)

s = open("src/text1b.c", encoding="utf-8").read()
s = rep(s, "extern u8 g_cam_bone_data2;\n", "")
s = rep(s, "\nextern s16 g_cam_interp;\n", "\n")

# camera_InitRotation
old_ir = s[s.index("void camera_InitRotation(u8 *a0) {"):s.index('INCLUDE_ASM("asm/funcs", camera_CalcAngles);')]
if "ir_keep" in V:
    pass
else:
    new_ir = '''void camera_InitRotation(Unk80101DF0Record *a0) {
    Unk80101DF0Record *s0 = a0;
    s0->unk4 = 8;
    {
        s16 v0 = 4;
        s0->unk8 = 0;
        {
            Unk80101DF0Rot *a0_arg = &s0->xf.rot;
            s0->unk2 = 0;
            s0->unk0 = 0;
            s0->unk1 = 0;
            s0->unkC = 0;
            s0->unkA = v0;
            s0->xf.rot.vx = 0;
            s0->xf.rot.vy = 0;
            s0->xf.rot.vz = 0;
            ((void (*)(Unk80101DF0Rot *, Unk80101DF0Mat *))g_anim_func_table[s0->unk8])(a0_arg, &s0->work);
        }
    }
    s0->work.t[2] = 0;
    s0->work.t[1] = 0;
    s0->work.t[0] = 0;
    s0->xf.mat = s0->work;
}

'''
    if "ir_plain" in V:
        new_ir = '''void camera_InitRotation(Unk80101DF0Record *node) {
    node->unk4 = 8;
    node->unk8 = 0;
    node->unk2 = 0;
    node->unk0 = 0;
    node->unk1 = 0;
    node->unkC = 0;
    node->unkA = 4;
    node->xf.rot.vx = 0;
    node->xf.rot.vy = 0;
    node->xf.rot.vz = 0;
    ((void (*)(Unk80101DF0Rot *, Unk80101DF0Mat *))g_anim_func_table[node->unk8])(&node->xf.rot, &node->work);
    node->work.t[2] = 0;
    node->work.t[1] = 0;
    node->work.t[0] = 0;
    node->xf.mat = node->work;
}

'''
    s = s.replace(old_ir, new_ir)

# camera_InitBone2
if "ir_keep" in V:
    s = rep(s, "    camera_InitRotation(&g_cam_bone_data2);\n    g_cam_interp = 4;\n",
            "    camera_InitRotation((u8 *)&g_cam_bone_data2);\n    g_cam_bone_data2.unk8 = 4;\n")
else:
    s = rep(s, "    camera_InitRotation(&g_cam_bone_data2);\n    g_cam_interp = 4;\n",
            "    camera_InitRotation(&g_cam_bone_data2);\n    g_cam_bone_data2.unk8 = 4;\n")

# func_800475A4
s = rep(s, "extern s16 D_800EEE00;\nextern s16 D_800EEE02;\nextern s32 D_800EEE1C;\nextern s32 D_800EEE20;\nextern s32 D_800EEE24;\nextern s32 D_800F66B0;\n",
        "extern s16 D_800EEE00;\nextern s16 D_800EEE02;\nextern s32 D_800EEE1C;\nextern s32 D_800EEE20;\nextern s32 D_800EEE24;\n" if "tc" in V else "")
s = rep(s, "    u8 *base;\n\n    if (stage_GetVariant() != 0) {", "    Unk80101DF0Record *base;\n\n    if (stage_GetVariant() != 0) {")
old_tail = s[s.index("    {\n        s16 neg = -ratan2(result.vy, computed);"):s.index("void game_AnimStart(void) {")]
tails = {
 "a": '''    g_cam_bone_data2.xf.rot.vx = -ratan2(result.vy, computed);
    g_cam_bone_data2.xf.rot.vy = angle;
    g_cam_bone_data2.xf.mat.t[0] = D_80101DF0.xf.mat.t[0];
    g_cam_bone_data2.xf.mat.t[1] = D_80101DF0.xf.mat.t[1];
    g_cam_bone_data2.xf.mat.t[2] = D_80101DF0.xf.mat.t[2] + 0x6590;
    base = &g_cam_bone_data2;
    ((void (*)(Unk80101DF0Rot *, MATRIX *))g_anim_func_table[4])(&base->xf.rot, &buf1);
    ((void (*)(Unk80101DF0Rot *, MATRIX *))g_anim_func_table[0])(&D_80101DF0.xf.rot, &buf2);
    MulMatrix0(&buf2, &buf1, (MATRIX *)&base->xf.mat);

    {
        s32 *temp = (s32 *)D_800A3820;
        D_800A3820 = (s32)(temp + 1);
        *temp = (s32)base;
    }
}

''',
 "b": '''    {
        s16 neg = -ratan2(result.vy, computed);
        base = &g_cam_bone_data2;
        g_cam_bone_data2.xf.rot.vx = neg;
    }
    g_cam_bone_data2.xf.rot.vy = angle;
    g_cam_bone_data2.xf.mat.t[0] = D_80101DF0.xf.mat.t[0];
    g_cam_bone_data2.xf.mat.t[1] = D_80101DF0.xf.mat.t[1];
    g_cam_bone_data2.xf.mat.t[2] = D_80101DF0.xf.mat.t[2] + 0x6590;
    ((void (*)(Unk80101DF0Rot *, MATRIX *))g_anim_func_table[4])(&base->xf.rot, &buf1);
    ((void (*)(Unk80101DF0Rot *, MATRIX *))g_anim_func_table[0])(&D_80101DF0.xf.rot, &buf2);
    MulMatrix0(&buf2, &buf1, (MATRIX *)&base->xf.mat);

    {
        s32 *temp = (s32 *)D_800A3820;
        D_800A3820 = (s32)(temp + 1);
        *temp = (s32)base;
    }
}

''',
}
tails["c"] = """    {
        s16 neg = -ratan2(result.vy, computed);
        base = &g_cam_bone_data2;
        D_800EEE00 = neg;
    }
    D_800EEE02 = angle;
    D_800EEE1C = D_80101DF0.xf.mat.t[0];
    D_800EEE20 = D_80101DF0.xf.mat.t[1];
    D_800EEE24 = D_80101DF0.xf.mat.t[2] + 0x6590;
    ((void (*)(Unk80101DF0Rot *, MATRIX *))g_anim_func_table[4])(&base->xf.rot, &buf1);
    ((void (*)(Unk80101DF0Rot *, MATRIX *))g_anim_func_table[0])(&D_80101DF0.xf.rot, &buf2);
    MulMatrix0(&buf2, &buf1, (MATRIX *)&base->xf.mat);

    {
        s32 *temp = (s32 *)D_800A3820;
        D_800A3820 = (s32)(temp + 1);
        *temp = (s32)base;
    }
}

"""
tk = "b" if "tb" in V else ("c" if "tc" in V else "a")
COMMON_CALLS_BASE = """    ((void (*)(Unk80101DF0Rot *, MATRIX *))g_anim_func_table[4])(&base->xf.rot, &buf1);
    ((void (*)(Unk80101DF0Rot *, MATRIX *))g_anim_func_table[0])(&D_80101DF0.xf.rot, &buf2);
    MulMatrix0(&buf2, &buf1, (MATRIX *)&base->xf.mat);

    {
        s32 *temp = (s32 *)D_800A3820;
        D_800A3820 = (s32)(temp + 1);
        *temp = (s32)base;
    }
}

"""
STORES_G = """    g_cam_bone_data2.xf.rot.vy = angle;
    g_cam_bone_data2.xf.mat.t[0] = D_80101DF0.xf.mat.t[0];
    g_cam_bone_data2.xf.mat.t[1] = D_80101DF0.xf.mat.t[1];
    g_cam_bone_data2.xf.mat.t[2] = D_80101DF0.xf.mat.t[2] + 0x6590;
"""
tails["d"] = """    g_cam_bone_data2.xf.rot.vx = -ratan2(result.vy, computed);
""" + STORES_G + """    ((void (*)(Unk80101DF0Rot *, MATRIX *))g_anim_func_table[4])(&g_cam_bone_data2.xf.rot, &buf1);
    ((void (*)(Unk80101DF0Rot *, MATRIX *))g_anim_func_table[0])(&D_80101DF0.xf.rot, &buf2);
    MulMatrix0(&buf2, &buf1, (MATRIX *)&g_cam_bone_data2.xf.mat);

    {
        s32 *temp = (s32 *)D_800A3820;
        D_800A3820 = (s32)(temp + 1);
        *temp = (s32)&g_cam_bone_data2;
    }
}

"""
tails["e"] = """    base = &g_cam_bone_data2;
    base->xf.rot.vx = -ratan2(result.vy, computed);
    base->xf.rot.vy = angle;
    base->xf.mat.t[0] = D_80101DF0.xf.mat.t[0];
    base->xf.mat.t[1] = D_80101DF0.xf.mat.t[1];
    base->xf.mat.t[2] = D_80101DF0.xf.mat.t[2] + 0x6590;
""" + COMMON_CALLS_BASE
tails["f"] = """    {
        s16 neg = -ratan2(result.vy, computed);
        g_cam_bone_data2.xf.rot.vx = neg;
    }
""" + STORES_G + """    base = &g_cam_bone_data2;
""" + COMMON_CALLS_BASE
tails["g"] = """    {
        s16 neg = -ratan2(result.vy, computed);
        base = &g_cam_bone_data2;
        base->xf.rot.vx = neg;
    }
    g_cam_bone_data2.xf.rot.vy = angle;
    g_cam_bone_data2.xf.mat.t[0] = D_80101DF0.xf.mat.t[0];
    g_cam_bone_data2.xf.mat.t[1] = D_80101DF0.xf.mat.t[1];
    g_cam_bone_data2.xf.mat.t[2] = D_80101DF0.xf.mat.t[2] + 0x6590;
""" + COMMON_CALLS_BASE
tails["h"] = """    base = &g_cam_bone_data2;
    base->xf.rot.vx = -ratan2(result.vy, computed);
""" + STORES_G.replace("    g_cam_bone_data2.xf.rot.vy", "    g_cam_bone_data2.xf.rot.vy") + COMMON_CALLS_BASE
tails["i"] = """    base->xf.rot.vx = -ratan2(result.vy, computed);
""" + STORES_G + COMMON_CALLS_BASE
tails["j"] = """    {
        s16 neg = -ratan2(result.vy, computed);
        base->xf.rot.vx = neg;
    }
""" + STORES_G + COMMON_CALLS_BASE
tails["k"] = """    base = &g_cam_bone_data2;
    {
        s16 neg = -ratan2(result.vy, computed);
        base->xf.rot.vx = neg;
    }
""" + STORES_G + COMMON_CALLS_BASE
for k in "hijk":
    if "t" + k in V:
        tk = k
if tk in "ij":
    s_base_init = True

tails["l"] = """    base = &g_cam_bone_data2;
    {
        s16 neg = -ratan2(result.vy, computed);
        base->xf.rot.vx = neg;
    }
    base->xf.rot.vy = angle;
    base->xf.mat.t[0] = D_80101DF0.xf.mat.t[0];
    base->xf.mat.t[1] = D_80101DF0.xf.mat.t[1];
    base->xf.mat.t[2] = D_80101DF0.xf.mat.t[2] + 0x6590;
""" + COMMON_CALLS_BASE
tails["m"] = """    {
        s16 neg = -ratan2(result.vy, computed);
        base = &g_cam_bone_data2;
        base->xf.rot.vx = neg;
    }
    base->xf.rot.vy = angle;
    base->xf.mat.t[0] = D_80101DF0.xf.mat.t[0];
    base->xf.mat.t[1] = D_80101DF0.xf.mat.t[1];
    base->xf.mat.t[2] = D_80101DF0.xf.mat.t[2] + 0x6590;
""" + COMMON_CALLS_BASE
for k in "lm":
    if "t" + k in V:
        tk = k

T_TAIL = """    base->xf.mat.t[0] = D_80101DF0.xf.mat.t[0];
    base->xf.mat.t[1] = D_80101DF0.xf.mat.t[1];
    base->xf.mat.t[2] = D_80101DF0.xf.mat.t[2] + 0x6590;
"""
tails["n"] = """    base = &g_cam_bone_data2;
    base->xf.rot.vy = angle;
    base->xf.rot.vx = -ratan2(result.vy, computed);
""" + T_TAIL + COMMON_CALLS_BASE
tails["o"] = """    base = &g_cam_bone_data2;
    base->xf.rot.vy = angle;
""" + T_TAIL + """    base->xf.rot.vx = -ratan2(result.vy, computed);
""" + COMMON_CALLS_BASE
tails["p"] = """    base = &g_cam_bone_data2;
    base->xf.rot.vx = -ratan2(result.vy, computed);
    base->xf.rot.vy = angle;
""" + T_TAIL + COMMON_CALLS_BASE
tails["q"] = """    base = &g_cam_bone_data2;
    base->xf.rot.vx = -(s16)ratan2(result.vy, computed);
    base->xf.rot.vy = angle;
""" + T_TAIL + COMMON_CALLS_BASE
for k in "nopq":
    if "t" + k in V:
        tk = k

for k in "defg":
    if "t" + k in V:
        tk = k

s = s.replace(old_tail, tails[tk])
if tk in ("i", "j"):
    _a = "    rot.vx = 0;\n    rot.vy = 0;\n    rot.vz = 0x6590;\n"
    s = rep(s, _a, "    base = &g_cam_bone_data2;\n" + _a)

# func_800477E8 tail block + its externs
s = rep(s, "extern s8 D_800EF070;\nextern s8 D_800EF071;\nextern s16 D_800EF076;\nextern s16 D_800EF078;\nextern s16 D_800EF07A;\nextern s32 D_800EF07C;\nextern s16 D_800EF080;\nextern s16 D_800EF082;\nextern s16 D_800EF084;\nextern s32 D_800EF0BC;\nextern s32 D_800EF0C0;\nextern s32 D_800EF0C4;\n", "")
s = rep(s, "extern void func_800417D0(s32 *);\n", "extern void func_800417D0(s32 *);\n", 1)
old_blk = s[s.index("    {\n        s32 *a0p;\n        a0p = (s32 *)&D_800EF070;"):s.index("    a3 = 0;\n    w = 0;\n")]
new_blk = '''    {
        Unk80101DF0Record *node = &D_800EF070;
        node->unk0 = 0xE;
        D_800EF070.unkA = 4;
        D_800EF070.work.t[0] = -0x2EE0;
        D_800EF070.unk1 = 0;
        D_800EF070.work.t[1] = 0;
        D_800EF070.work.t[2] = -0xFA0;
        D_800EF070.xf.rot.vx = 0;
        D_800EF070.xf.rot.vy = 0;
        D_800EF070.xf.rot.vz = 0;
        D_800EF070.unk8 = 0;
        D_800EF070.unkC = 0;
        D_800EF070.unk6 = 0;
        func_800417D0((s32 *)node);
    }

'''
if "bd" in V:
    new_blk = new_blk.replace("        Unk80101DF0Record *node = &D_800EF070;\n        node->unk0 = 0xE;\n", "        D_800EF070.unk0 = 0xE;\n").replace("func_800417D0((s32 *)node);", "func_800417D0((s32 *)&D_800EF070);")
if "bp" in V:
    new_blk = new_blk.replace("D_800EF070.", "node->")
s = s.replace(old_blk, new_blk)
open(D + "/text1b.c", "w", encoding="utf-8", newline="\n").write(s)
print("wrote", D)
