#!/usr/bin/env python3
# Worker-2 batch 4: F05 (368E4's transform nodes) + P7c (the second OT g_gpu_ot256_ptr, PsyQ ClearOTagR)
# + the model object's remaining sites in 368E4 (func_80049718 / func_80049A2C), chained on w2b3 (the
# model object, lt/f08/w2b3.py).
# - The 0x68-byte draw nodes those functions build (the D_800A38B4 pool, the D_800A33E4 table) take
#   the model object's node type Unk80045878Node (same layout: Unk80101DF0Record + a word at +0x58).
# - g_gpu_ot256_ptr: s32 -> u32 *; g_gpu_ot256_db: u8[] -> u32 [][256], unsized: a sized [2][256] extern reorders the object's symbol table (the span is exactly two OTs); ClearOTagR's PsyQ prototype
#   goes to libgpu.h (368E4's int prototype goes); sys.c's ClearOTagR calls the device table's otc.
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/f05/w2b4.py [opt=...] [out=DIR | apply]
#   default out=tmp/w2/b4 (scratch copies); `apply` writes the working tree.
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f08"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import w2b3 as B3
sys.argv = _argv
B2 = B3.B2
NL = chr(10)
OPT = {"n_var57"}   # the landed spellings; `opt=` replaces the set
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT = set(a[4:].split(",")) - {""}
rep, span, body, fn = B3.rep, B3.span, B3.body, B3.fn
B3FILES = dict(B3.FILES)


BASE = "cb2c44d78"   # batch 3 committed on main 12e8c26bd


def base(p):
    B2.BASE = BASE
    return B2.show(p)


# ------------------------------------------------------------------ libgpu.h / sys.c (P7c)
def libgpu_h(s):
    return rep(s, "extern void DrawOTag(u32 *);\n", "extern void DrawOTag(u32 *);\nextern u32 *ClearOTagR(u32 *, s32);\n")


def sys_c(s):
    s = rep(s, """u32 *ClearOTagR(u32 *ot, s32 n) {
    u32 *new_var;
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015F98, ot, n);
        new_var = ot; /* FAKE: cse.c make_regs_eqv beyond-block gate; flow-deleted pre-RA */
    }
    {
        u32 *v0 = (u32 *)g_gpu_dev_table;
        ((void (*)(u32 *, s32))v0[11])(ot, n);
    }
    new_var = ot;
    *new_var = ((u32)&g_gpu_ot_end) & 0xFFFFFF;
    return new_var;
}""", """u32 *ClearOTagR(u32 *ot, s32 n) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015F98, ot, n);
    }
    g_gpu_dev_table->otc(ot, n);
    *ot = ((u32)&g_gpu_ot_end) & 0xFFFFFF;
    return ot;
}""")
    s = rep(s, """ * PutDrawEnv call through the members (measured byte-identical), as do the
 * other users except ClearOTagR (its word view is the P7c debt row). */""",
            """ * PutDrawEnv call through the members (measured byte-identical), as do the
 * other users. */""")
    return s


# ------------------------------------------------------------------ 368E4.c
B_48BA4_TAIL_OLD = """    goto test_index;
copy_index:
        *(MATRIX *)(prim + 0x18) = *player[index];
        ot = (s32 *)D_800A3820;
        D_800A3820 = (s32)(ot + 1);
        *ot = (s32)prim;
        prim += 0x68;
test_index:
    index = *indices;
    indices++;
    if (index >= 0) {
        goto copy_index;
    }
    if (arg1 >= 0) {
        *(MATRIX *)(prim + 0x18) = *player[18];
        ot = (s32 *)D_800A3820;
        *(s16 *)(prim + 2) = arg1 + 0xF;
        D_800A3820 = (s32)(ot + 1);
        *ot = (s32)prim;
        prim += 0x68;
    }
    if (arg2 != 0) {
        *(MATRIX *)(prim + 0x18) = *player[19];
        ot = (s32 *)D_800A3820;
        *(s16 *)(prim + 2) = 0x15;
        D_800A3820 = (s32)(ot + 1);
        *ot = (s32)prim;
    }

    g_gpu_ot256_ptr = (s32)(g_gpu_ot256_db + ((D_800A36AC & 1) << 10));
    ClearOTagR(g_gpu_ot256_ptr, 0x100);
    old = D_800A378C[0];
    D_800A378C[0] = (g_gpu_ot256_ptr + 0x3FC) & 0xFFFFFF;
    *(s32 *)g_gpu_ot256_ptr = old;
}"""
B_48BA4_TAIL_NEW = """    goto test_index;
copy_index:
        node->node.xf.mat = *player[index];
        list = (s32 *)D_800A3820;
        D_800A3820 = (s32)(list + 1);
        *list = (s32)node;
        node++;
test_index:
    index = *indices;
    indices++;
    if (index >= 0) {
        goto copy_index;
    }
    if (arg1 >= 0) {
        node->node.xf.mat = *player[18];
        list = (s32 *)D_800A3820;
        node->node.unk2 = arg1 + 0xF;
        D_800A3820 = (s32)(list + 1);
        *list = (s32)node;
        node++;
    }
    if (arg2 != 0) {
        node->node.xf.mat = *player[19];
        list = (s32 *)D_800A3820;
        node->node.unk2 = 0x15;
        D_800A3820 = (s32)(list + 1);
        *list = (s32)node;
    }

    g_gpu_ot256_ptr = g_gpu_ot256_db[D_800A36AC & 1];
    ClearOTagR(g_gpu_ot256_ptr, 0x100);
    old = D_800A378C[0];
    D_800A378C[0] = (u32)(g_gpu_ot256_ptr + 0xFF) & 0xFFFFFF;
    *g_gpu_ot256_ptr = old;
}"""


def b_48ba4(b):
    b = rep(b, "    s32 *ot;\n    MATRIX **player;\n    u8 *prim;\n", "    s32 *list;\n    MATRIX **player;\n    Unk80045878Node *node;\n")
    b = rep(b, "    prim = (u8 *)D_800A33E4;\n", "    node = (Unk80045878Node *)D_800A33E4;\n")
    b = rep(b, B_48BA4_TAIL_OLD, B_48BA4_TAIL_NEW)
    head = "    rot.vx = 0x1770;\n    rot.vy = 0;\n    rot.vz = 0;\n    scale = 0x1770;\n"
    if "a_scale" in OPT:     # ablation: the 0x1770 literal in both products
        b = rep(b, head, "    rot.vx = 0x1770;\n    rot.vy = 0;\n    rot.vz = 0;\n")
        b = rep(b, "* scale) >> 12;\n", "* 0x1770) >> 12;\n", 2)
    elif "a_rotdead" in OPT:  # ablation: the two stores the products overwrite
        b = rep(b, head, "    rot.vy = 0;\n    scale = 0x1770;\n")
    else:
        # rot.vx / rot.vz's first stores stay as at HEAD, unlabelled: they are not dead at the
        # machine level (the target executes them: li 6000; sh v0,48(sp) before the products
        # overwrite the fields), so they are the program's stores, not a dead-store FAKE.
        b = rep(b, head, "    rot.vx = 0x1770;\n    rot.vy = 0;\n    rot.vz = 0;\n"
                "    /* FAKE: constant holder for the 0x1770 radius; the literal S_SCALE */\n    scale = 0x1770;\n")
    return b


B_49718 = """/* Appends one or two 0x68-byte draw nodes at D_800A38B4 for animation entry arg0 and queues
 * each on the D_800A3820 draw list. The first node (type 0) gets its rotation and position either
 * from rot_in / pos (flags == 1: g_anim_func_table[0] turns rot_in into the node's matrix) or from
 * node 19 + (flags & 1) of player flags >> 1's model object: that node's offset (work.t) is scaled
 * by the object's unk_12, the root node's matrix times the node's work matrix becomes the new
 * node's matrix, the scaled offset run through the parent's matrix (plus its translation) gives its
 * position, and the matrix is copied back into the object's node. The second node (type 3, parent =
 * the first) follows unless flags is still 1. */
void func_80049718(s32 arg0, s32 flags, s32 *pos, s16 *rot_in) {
    SVECTOR ofs;
    s32 val58;
    Unk80045878Obj *player;
    Unk80045878Node *obj;
    Unk80045878Node *part;
    /* FAKE: named intermediate (no-new-park-categories entry 6).  Set before
     * the call, sched1 moves the andi past func_8004153C but ahead of the copy
     * of its result (`andi v1,s3,1; move s1,v0`, as in the target); written
     * inside the part expression or after the call it follows the copy and
     * takes $v0. Ablated (2026-10-06): score 5. */
    s32 side;
    if (D_800EF980[arg0] < 0) {
        func_80052C10();
    }
    obj = (Unk80045878Node *)D_800A38B4;
    /* FAKE: dead store (dead-store-fake-exception).  The 0 is never read: the
     * flags == 1 path skips the second node.  Flow cannot tell, so the store
     * stays as the target's `move s5,zero`; without it that instruction is
     * missing. Ablated (2026-10-06): score 1. */
    val58 = 0;
    obj->node.unk0 = 0;
    obj->node.unk1 = 0;
    obj->node.unk2 = D_800EF980[arg0] * 2;
    obj->node.unk4 = 6;
    obj->node.unk8 = 0;
    obj->node.unkC = 0;
    obj->node.unkA = 4;
    if (flags != 0) {
        if (flags == 1) {
            obj->node.xf.rot.vx = rot_in[0];
            obj->node.xf.rot.vy = rot_in[1];
            obj->node.xf.rot.vz = rot_in[2];
            g_anim_func_table[0](&obj->node.xf.rot, &obj->node.xf.mat);
            obj->node.xf.mat.t[0] = pos[0];
            obj->node.xf.mat.t[1] = pos[1];
            obj->node.xf.mat.t[2] = pos[2];
        } else {
            /* FAKE: flags is rewritten in place - compound-assigned, read (>> 1, & 1),
             * compound-assigned again, read (!= 1) - as SOTN reuses a parameter
             * (Q51).  Copied into a local instead, global.c's allocno order flips:
             * rot_in's pseudo (priority 3333) outranks the table-address pseudo
             * (3000) for $s0, against 2962 / 3333 in place. Ablated (2026-10-06): score 63. */
            /* SOTN: src/st/lib/e_shop.c:4621 @aa53500 */
            flags &= 0x7FFF;
            side = flags & 1;
            player = func_8004153C(flags >> 1);
            part = &player->unk_2C[19 + side];
            part->node.work.t[0] = (part->node.work.t[0] * player->unk_12) >> 12;
            part->node.work.t[1] = (part->node.work.t[1] * player->unk_12) >> 12;
            part->node.work.t[2] = (part->node.work.t[2] * player->unk_12) >> 12;
            MulMatrix0(&player->unk_2C[0].node.xf.mat, &part->node.work, &obj->node.xf.mat);
            ofs.vx = part->node.work.t[0];
            ofs.vy = part->node.work.t[1];
            ofs.vz = part->node.work.t[2];
            ApplyMatrix(&part->node.unkC->xf.mat, &ofs, (VECTOR *)obj->node.xf.mat.t);
            obj->node.xf.mat.t[0] = obj->node.xf.mat.t[0] + part->node.unkC->xf.mat.t[0];
            obj->node.xf.mat.t[1] = obj->node.xf.mat.t[1] + part->node.unkC->xf.mat.t[1];
            obj->node.xf.mat.t[2] = obj->node.xf.mat.t[2] + part->node.unkC->xf.mat.t[2];
            /* SOTN: src/st/lib/e_shop.c:4625 @aa53500 */
            flags |= 0x8000;
            part->node.xf.mat = obj->node.xf.mat;
            val58 = player->unk_1A84;
        }
        {
            s32 *list = (s32 *)D_800A3820;
            D_800A3820 = (s32)(list + 1);
            *list = (s32)obj;
        }
        obj++;
        if (flags != 1) {
            /* FAKE: named intermediate (no-new-park-categories entry 6).  The
             * table is read before the node's fields are written, as in the
             * target (lh first); storing D_800EF980[arg0] * 2 + 1 directly at
             * the +2 store reads it last, and moving that store first
             * reorders the stores. Ablated (2026-10-06): score 22. */
            s32 frame = D_800EF980[arg0];
            s32 *list;
            obj->node.unk0 = 3;
            obj->node.unk1 = 0;
            UNK58
            list = (s32 *)D_800A3820;
            obj->node.unkC = &obj[-1].node;
            obj->node.unk6 = 1;
            obj->node.unk8 = 0;
            obj->node.unkA = 0;
            obj->node.unk4 = 6;
            obj->node.unk2 = frame * 2 + 1;
            D_800A3820 = (s32)(list + 1);
            *list = (s32)obj;
            obj++;
        }
        D_800A38B4 = (u32)obj;
    }
}
"""


B_49A2C = """void func_80049A2C(s32 arg0, s32 arg1, s32 arg2) {
    volatile u32 pre_pad[2]; // !FAKE: phantom-frame-slot volatile filler (.claude/rules/no-new-park-categories.md): target reserves 8 locals bytes at sp+0x10..sp+0x17 that no instruction touches; mechanism: GCC 2.7.2 get_frame_size reserves declared locals. Ablated (2026-10-06): score 12.
    u8 *new_var6;
    s16 *new_var5;
    s16 *new_var7;
    u8 temp_v1;
    u8 *new_var8;
    s16 *p_anim;
    s16 new_var2;
    s16 *src;
    Unk80045878Node *obj;
    Unk80045878Obj *player;
    s16 a1_val;
    s32 *list;

    /* FAKE: the table base in its own holder; indexing D_80099CC8 directly emits the row
       shift after the base load (score S_VAR6). */
    new_var6 = D_80099CC8;
    {
        u8 *p = new_var6 + (arg0 * 2);
        temp_v1 = p[arg2];
    }
    if (temp_v1 == 0xFF) {
        return;
    }
    /* FAKE: D_800EF980 reached as bytes through a holder; &D_800EF980[temp_v1] forms the address
       in the other order (score S_VAR8). */
    new_var8 = (u8 *) D_800EF980;
    p_anim = (s16 *) (new_var8 + (temp_v1 * 2));
    if ((*p_anim) < 0) {
        func_80052C10();
    }
    player = func_8004153C(arg1 >> 1);
    obj = (Unk80045878Node *)D_800A38B4;
    obj->node.unk0 = 0;
    obj->node.unk1 = 0;
    a1_val = (*p_anim) * 2;
    obj->node.unk4 = 6;
    obj->node.unk8 = 0;
    obj->node.unkA = 4;
    obj->node.unk2 = a1_val;
    src = &D_80099D3C[(arg1 & 1) * 6];
    obj->node.work.t[0] = ((s32) ((*src) * player->unk_12)) >> 12;
    src++;
    obj->node.work.t[1] = ((s32) ((*src) * player->unk_12)) >> 12;
    src++;
    obj->node.work.t[2] = ((s32) ((*src) * player->unk_12)) >> 12;
    src++;
    obj->node.xf.rot.vx = *src;
    src++;
    obj->node.xf.rot.vy = *src;
    /* FAKE: rot.vz's value read into a local ahead of the parent store; read at its own store
       it scores 8. */
    new_var2 = src[1];
    obj->node.unkC = &player->unk_2C[12].node;
    obj->node.unk6 = 0;
    obj->node.xf.rot.vz = new_var2;
    func_800417D0(&obj->node);
    list = (s32 *)D_800A3820;
    D_800A3820 = (s32)(list + 1);
    *list = (s32)obj;
    obj++;
    new_var5 = &obj->node.unkA;
    a1_val = (*p_anim) * 2;
    obj->node.unk0 = 3;
    obj->node.unkC = &obj[-1].node;
    obj->node.unk1 = 0;
    new_var7 = &obj->node.unk6;
    obj->node.unk8 = 0;
    *new_var7 = 1;
    *new_var5 = 0;
    obj->node.unk4 = 6;
    obj->node.unk2 = a1_val + 1;
A2C58
    list = (s32 *)D_800A3820;
    D_800A3820 = (s32)(list + 1);
    *list = (s32)obj;
    D_800A38B4 = (u32)(obj + 1);
}
"""
A2C58 = {
    "a2c_member": "    obj->unk58 = player->unk_1A84;\n",
    "a2c_p58": """    /* FAKE: unk58 stored through a pointer; the member store lets sched lift the D_800A3820
       reload above the node's stores (score S_P58A). */
    {
        s32 *p58 = &obj->unk58;
        *p58 = player->unk_1A84;
    }
""",
}


B_483DC_TRY = """void func_800483DC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    s32 *list;
    s32 count;
    struct { u32 off; s16 x0, y0, x1, y1; } *e;
    list = (s32 *)(arg0 + ((s32 *)arg0)[arg1]);
    count = *list;
    e = (void *)(list + 1);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            func_800484A0((u8 *)(arg0 + e->off), e->x1 + sx_arg2, e->y1 + sx_arg3);
            e++;
        } while ((count--) != 0);
    }
}
"""


def c368e4(s):
    # func_8005C2A8 takes the VAB pack since worker 1's 4290c05ec; the stale local declaration follows
    s = rep(s, "extern s32 func_8005C2A8(s32 *, s16, s32);\n", "extern s32 func_8005C2A8(Unk8005C2A8Pack *, s16, s32);\n")
    s = rep(s, "func_8005C2A8((s32 *)var_s1, 2, var_s7)", "func_8005C2A8((Unk8005C2A8Pack *)var_s1, 2, var_s7)")
    s = rep(s, "func_8005C2A8((s32 *)var_s1, 5, var_s7)", "func_8005C2A8((Unk8005C2A8Pack *)var_s1, 5, var_s7)")
    s = rep(s, "extern s32 ClearOTagR(s32, s32);\nextern s32 g_gpu_ot256_ptr;\nextern u8 g_gpu_ot256_db[];\n",
            "extern u32 *g_gpu_ot256_ptr;\nextern u32 g_gpu_ot256_db[][256];\n")
    s = rep(s, "\ns32 g_gpu_ot256_ptr;\n", "\nu32 *g_gpu_ot256_ptr;\n")
    s = fn(s, "func_80048BA4", b_48ba4)
    i, j = span(s, "func_80049718")
    k = s.rindex("/* Appends one or two 0x68-byte draw objects", 0, i)
    u58 = ("obj->unk58 = val58;" if "u58_member" in OPT else
           "/* FAKE: unk58 stored through a pointer; the member store lets sched sink it below\n"
           "             * the D_800A3820 load (score SCORE58). */\n"
           "            {\n"
           "                s32 *p58 = &obj->unk58;\n"
           "                *p58 = val58;\n"
           "            }")
    s = s[:k] + B_49718.replace("UNK58", u58) + s[j:]
    k2 = ([o for o in OPT if o.startswith("a2c_")] or ["a2c_p58"])[0]
    b = B_49A2C.replace("A2C58\n", A2C58[k2])
    if "n_var8" in OPT:      # the anim-table entry without the byte view
        b = rep(b, "    u8 *new_var8;\n", "")
        b = rep(b, "    new_var8 = (u8 *) D_800EF980;\n    p_anim = (s16 *) (new_var8 + (temp_v1 * 2));\n",
                "    p_anim = &D_800EF980[temp_v1];\n")
    if "n_var6" in OPT:      # the table row without the holder
        b = rep(b, "    u8 *new_var6;\n", "")
        b = rep(b, "    new_var6 = D_80099CC8;\n    {\n        u8 *p = new_var6 + (arg0 * 2);\n",
                "    {\n        u8 *p = &D_80099CC8[arg0 * 2];\n")
    if "n_var57" in OPT:     # the second node's unk6 / unkA stores without the holders
        b = rep(b, "    s16 *new_var5;\n    s16 *new_var7;\n", "")
        b = rep(b, "    new_var5 = &obj->node.unkA;\n", "")
        b = rep(b, "    new_var7 = &obj->node.unk6;\n", "")
        b = rep(b, "    *new_var7 = 1;\n    *new_var5 = 0;\n", "    obj->node.unk6 = 1;\n    obj->node.unkA = 0;\n")
    s = body(s, "func_80049A2C", b)
    if "c24_raw" not in OPT:   # F11: func_80049C24's header reads through the file's s32 word table
        def b_49c24(b):
            b = rep(b, "    s32 a0_arg;\n\n    count = *(s32 *)arg0;\n", "    s32 a0_arg;\n    s32 *tbl = (s32 *)arg0;\n\n    count = tbl[0];\n")
            b = rep(b, "    temp_v0 = *(s32 *)(arg0 + (count * 4) + 4);\n    temp_a2 = *(s32 *)(arg0 + 8);\n",
                    "    temp_v0 = tbl[count + 1];\n    temp_a2 = tbl[2];\n")
            b = rep(b, "    v0 = *(s32 *)(arg0 + 4);\n", "    v0 = tbl[1];\n")
            b = rep(b, "        var_s4 = *(s32 *)(arg0 + 0xC) - temp_a2;\n", "        var_s4 = tbl[3] - temp_a2;\n")
            return b
        s = fn(s, "func_80049C24", b_49c24)
    if "try_483dc" in OPT:   # F11 probe: a typed 12-byte entry walk for the simplest stream reader
        s = body(s, "func_800483DC", B_483DC_TRY)
    s = rep(s, """/* func_80049A2C: the only non-ordinary construct is the first declaration,
 * `volatile u32 pre_pad[2];`, a labelled FAKE""", """/* func_80049A2C: the first declaration,
 * `volatile u32 pre_pad[2];`, is a labelled FAKE""")
    for k, v in SCORES.items():
        s = s.replace(k, v)
    return s


# measured ablation scores (abl_w2b4.ps1 on the applied tree)
SCORES = {"S_VAR6": "2", "S_VAR8": "4", "S_P58A": "17", "SCORE58": "2", "S_SCALE": "scores 20"}


FILES = [("include/psxsdk/libgpu.h", libgpu_h), ("src/main/psxsdk/libgpu/sys.c", sys_c), ("src/main/368E4.c", c368e4)]


def fix4_368e4(s):
    """rev-w2b4 fixes (FAIL B4 / B6), the reviewer's measured patch"""
    def a2c(b):
        b = rep(b, "    /* FAKE: the table base in its own holder; indexing D_80099CC8 directly emits the row\n"
                   "       shift after the base load (score 2). */\n",
                "    /* FAKE: the table base in its own holder; indexing D_80099CC8 directly emits the row\n"
                "       shift ahead of the base load (score 2). */\n")
        b = rep(b, "    u8 *new_var8;\n", "    s16 *new_var8;\n")
        b = rep(b, "    /* FAKE: D_800EF980 reached as bytes through a holder; &D_800EF980[temp_v1] forms the address\n"
                   "       in the other order (score 4). */\n"
                   "    new_var8 = (u8 *) D_800EF980;\n    p_anim = (s16 *) (new_var8 + (temp_v1 * 2));\n",
                "    /* FAKE: the table base in its own holder; &D_800EF980[temp_v1] emits the shift ahead of\n"
                "       the base load and swaps the addu operands (score 4). */\n"
                "    new_var8 = D_800EF980;\n    p_anim = new_var8 + temp_v1;\n")
        b = rep(b, "    obj->node.unk1 = 0;\n    a1_val = (*p_anim) * 2;\n",
                "    obj->node.unk1 = 0;\n"
                "    /* FAKE: a1_val reads *p_anim ahead of each node's stores; read at the unk2 stores the lh\n"
                "       moves down to them (first 29, second 6, both 35). */\n"
                "    a1_val = (*p_anim) * 2;\n")
        b = rep(b, "((s32) ((*src) * player->unk_12)) >> 12;", "((*src) * player->unk_12) >> 12;", 3)
        return b

    def c24(b):
        b = rep(b, "    temp_v0 = tbl[count + 1];\n",
                "    /* FAKE: temp_v0 reads the word here; read at its use, arg0 is copied to s6 and the saved registers shift (score 11). */\n"
                "    temp_v0 = tbl[count + 1];\n")
        b = rep(b, "    var_s7 = arg0;\n",
                "    /* FAKE: var_s7 built in two steps; `var_s7 = arg0 + temp_v0` swaps s7 / s8 (score 6). */\n"
                "    var_s7 = arg0;\n")
        b = rep(b, "            var_s0 = var_s2;\n",
                "            /* FAKE: a no-op copy (both are -1); the target's `move s0,s2`; without it the test inverts (score 4). */\n"
                "            var_s0 = var_s2;\n")
        b = rep(b, "    hdr = ~var_s0;\n",
                "    /* FAKE: hdr built in two steps (one expression emits the nor after the slot copy, score 2); v1 holds the header slot so var_s3 steps before the store (stored through var_s3, the step follows the store, score 4). */\n"
                "    hdr = ~var_s0;\n")
        return b
    if not OPT & {"n_var8", "n_var6"}:
        s = fn(s, "func_80049A2C", a2c)
    if "c24_raw" not in OPT:
        s = fn(s, "func_80049C24", c24)
    return s


def _fix4(g, p):
    def h(s):
        s = g(s)
        return fix4_368e4(s) if p == "src/main/368E4.c" else s
    return h


FILES = [(p, _fix4(g, p)) for p, g in FILES]


def main():
    apply = "apply" in sys.argv[1:]
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or ["tmp/w2/b4"])[0]
    done = set()
    for p, g in FILES:
        s = g(base(p))
        done.add(p)
        dst = p if apply else out + "/" + p
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "w", encoding="utf-8", newline=NL).write(s)
    print("w2b4 wrote to %s %s" % ("the tree" if apply else out, sorted(OPT)))


if __name__ == "__main__":
    main()
