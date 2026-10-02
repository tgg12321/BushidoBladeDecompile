"""final.py [--inplace | --out DIR]: the transform-node typing landing (Q89 prerequisite).
Types g_cam_bone_data2 (0x800EEDF0) and D_800EF070 as Unk80101DF0Record and rewrites their text1b consumers
(camera_InitRotation, camera_InitBone2, func_800475A4, func_800477E8) as member accesses; refines the record's
+0x02..+0x07 bytes to the three s16 fields the code stores. --out writes copies (DIR/text1b.c, DIR/inc/code6cac.h);
--inplace edits src/text1b.c and include/code6cac.h (landing lock only)."""
import os, sys

inplace = "--inplace" in sys.argv
out = sys.argv[sys.argv.index("--out") + 1] if "--out" in sys.argv else None
assert inplace or out

def rep(s, a, b, n=1):
    assert s.count(a) == n, (a[:70], s.count(a))
    return s.replace(a, b)

h = open("include/code6cac.h", encoding="utf-8", newline="").read()
h = rep(h, " * base+0x1C (xf.mat.t). Rot / Mat are the PsyQ SVECTOR / MATRIX layouts\n",
        " * base+0x1C (xf.mat.t). g_cam_bone_data2 (0x800EEDF0) and D_800EF070 are\n"
        " * two more records of this layout: camera_InitRotation fills every field of\n"
        " * the first (s16 stores at +0x02/+0x04/+0x08/+0x0A, rot, work, then\n"
        " * xf.mat = work), func_800477E8 sets up the second (+0x06 as s16) and\n"
        " * passes it to func_800417D0, which reads +0x06 as an s16 state.\n"
        " * Rot / Mat are the PsyQ SVECTOR / MATRIX layouts\n")
h = rep(h, "    u8 unk2[6];            /* +0x02 */\n",
        "    s16 unk2;              /* +0x02 */\n"
        "    s16 unk4;              /* +0x04 */\n"
        "    s16 unk6;              /* +0x06 */\n")
h = rep(h, "extern Unk80101DF0Record D_800FF638;\n",
        "extern Unk80101DF0Record D_800FF638;\n"
        "extern Unk80101DF0Record g_cam_bone_data2;\n"
        "extern Unk80101DF0Record D_800EF070;\n")

s = open("src/text1b.c", encoding="utf-8", newline="").read()
s = rep(s, "extern u8 g_cam_bone_data2;\n", "")
s = rep(s, "\nextern s16 g_cam_interp;\n", "\n")

a = s.index("void camera_InitRotation(u8 *a0) {")
b = s.index('INCLUDE_ASM("asm/funcs", camera_CalcAngles);')
s = s[:a] + '''void camera_InitRotation(Unk80101DF0Record *node) {
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

''' + s[b:]

s = rep(s, "    camera_InitRotation(&g_cam_bone_data2);\n    g_cam_interp = 4;\n",
        "    camera_InitRotation(&g_cam_bone_data2);\n    g_cam_bone_data2.unk8 = 4;\n")

s = rep(s, "extern s16 D_800EEE00;\nextern s16 D_800EEE02;\nextern s32 D_800EEE1C;\nextern s32 D_800EEE20;\n"
           "extern s32 D_800EEE24;\nextern s32 D_800F66B0;\n", "")
s = rep(s, "    u8 *base;\n\n    if (stage_GetVariant() != 0) {",
        "    Unk80101DF0Record *base;\n\n    if (stage_GetVariant() != 0) {")
a = s.index("    {\n        s16 neg = -ratan2(result.vy, computed);")
b = s.index("void game_AnimStart(void) {")
s = s[:a] + '''    base = &g_cam_bone_data2;
    {
        /* FAKE: s16 temporary for the negated pitch: storing -ratan2() straight
         * into the field moves its negu one slot (vb_tp in
         * memory/grind/camera_CalcAngles/types/receipts.txt) */
        s16 neg = -ratan2(result.vy, computed);
        base->xf.rot.vx = neg;
    }
    base->xf.rot.vy = angle;
    base->xf.mat.t[0] = D_80101DF0.xf.mat.t[0];
    base->xf.mat.t[1] = D_80101DF0.xf.mat.t[1];
    base->xf.mat.t[2] = D_80101DF0.xf.mat.t[2] + 0x6590;
    ((void (*)(Unk80101DF0Rot *, MATRIX *))g_anim_func_table[4])(&base->xf.rot, &buf1);
    ((void (*)(Unk80101DF0Rot *, MATRIX *))g_anim_func_table[0])(&D_80101DF0.xf.rot, &buf2);
    MulMatrix0(&buf2, &buf1, (MATRIX *)&base->xf.mat);

    {
        s32 *temp = (s32 *)D_800A3820;
        D_800A3820 = (s32)(temp + 1);
        *temp = (s32)base;
    }
}

''' + s[b:]

s = rep(s, "extern s8 D_800EF070;\nextern s8 D_800EF071;\nextern s16 D_800EF076;\nextern s16 D_800EF078;\n"
           "extern s16 D_800EF07A;\nextern s32 D_800EF07C;\nextern s16 D_800EF080;\nextern s16 D_800EF082;\n"
           "extern s16 D_800EF084;\nextern s32 D_800EF0BC;\nextern s32 D_800EF0C0;\nextern s32 D_800EF0C4;\n", "")
a = s.index("    {\n        s32 *a0p;\n        a0p = (s32 *)&D_800EF070;")
b = s.index("    a3 = 0;\n    w = 0;\n")
s = s[:a] + '''    {
        Unk80101DF0Record *node = &D_800EF070;
        node->unk0 = 0xE;
        node->unkA = 4;
        node->work.t[0] = -0x2EE0;
        node->unk1 = 0;
        node->work.t[1] = 0;
        node->work.t[2] = -0xFA0;
        node->xf.rot.vx = 0;
        node->xf.rot.vy = 0;
        node->xf.rot.vz = 0;
        node->unk8 = 0;
        node->unkC = 0;
        node->unk6 = 0;
        func_800417D0((s32 *)node);
    }

''' + s[b:]

if inplace:
    open("include/code6cac.h", "w", encoding="utf-8", newline="").write(h)
    open("src/text1b.c", "w", encoding="utf-8", newline="").write(s)
    print("edited src/text1b.c include/code6cac.h")
else:
    os.makedirs(out + "/inc", exist_ok=True)
    open(out + "/inc/code6cac.h", "w", encoding="utf-8", newline="").write(h)
    open(out + "/text1b.c", "w", encoding="utf-8", newline="").write(s)
    print("wrote", out)
