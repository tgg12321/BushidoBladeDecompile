#!/usr/bin/env python3
# Ablations for w2b5 (run from the repo root with the batch applied): every FAKE in every moved body
# (new and carried) and the in-place sweep of the moved bodies' holders / aliases / reused locals.
# Each candidate undoes one construct in place. Writes tmp/w2/abl5/<name>.c + list.txt for
# ../f08/run_abl_w2b3.ps1 -List tmp/w2/abl5/list.txt.
import os, re, sys


def body(path, fn):
    s = open(path, encoding="utf-8").read()
    m = re.search(r"\n[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{" % fn, s)
    i = m.start() + 1
    line = s[i:s.index("\n", i) + 1]
    if line.rstrip().endswith("}"):   # a one-line definition
        return line
    return s[i:s.index("\n}\n", i) + 3]


def sub(b, a, c, n=1):
    if b.count(a) != n:
        raise SystemExit("expected %d x %r, found %d" % (n, a[:70], b.count(a)))
    return b.replace(a, c)


def drop_fake(b, start):
    """remove the /* FAKE ... */ comment beginning with `start` (and its line)"""
    i = b.index(start)
    j = b.index("*/", i) + 2
    k = b.rindex("\n", 0, i) + 1
    rest = b[j:]
    if rest.startswith("\n"):
        rest = rest[1:]
    return b[:k] + rest


def inline(b, decl, name, expr, setline=None):
    """drop the holder `name` (its declaration line `decl`, and its set line `setline` when the value
    is assigned separately) and read `expr` wherever the holder was read"""
    b = sub(b, decl, "")
    if setline:
        b = sub(b, setline, "")
    rep_ = ("(" + expr + ")") if " " in expr else expr
    b2 = re.sub(r"(?<![\w.>])%s\b" % re.escape(name), lambda m: rep_, b)
    if b2 == b:
        raise SystemExit("no use of " + name)
    return b2


F17, F287, F2B, F3AB, F87, F9F, F368, F31D = ("src/main/17AFC.c", "src/main/28708.c", "src/main/2B344.c",
    "src/main/3AB48.c", "src/main/87A0.c", "src/main/9F9C.c", "src/main/368E4.c", "src/main/31D3C.c")
FPAD, FBIOS, FPRIM, FSMM, FSMU = ("src/main/psxsdk/libapi/pad.c", "src/main/psxsdk/libcd/bios.c",
    "src/main/psxsdk/libgpu/prim.c", "src/main/psxsdk/libspu/s_m_m.c", "src/main/psxsdk/libspu/s_m_util.c")
FVSH, FALO, FVSU, FVIN = ("src/main/psxsdk/libsnd/vs_vh.c", "src/main/psxsdk/libsnd/vm_aloc2.c",
    "src/main/psxsdk/libsnd/vm_vsu.c", "src/main/psxsdk/libsnd/vm_init.c")
A = []

# ---- new FAKEs
A.append(("clr", "func_80032040", F17, lambda b: sub(drop_fake(b, "/* FAKE: byte-offset walk"),
    "    for (i = 0x84; i >= 0; i -= 0x2C) {\n        ((u8 *)D_80104E88)[i] = 0;\n",
    "    for (i = 3; i >= 0; i--) {\n        D_80104E88[i].unk_00 = 0;\n")))


def no_a3(b):
    b = drop_fake(b, "/* FAKE: a second, byte cursor a3")
    b = sub(b, "    u8 *a3 = &D_80104E88[0].unk_02;\n", "")
    b = sub(b, "        s32 v1_v = (*(u8 *)(a3 + 1) == 0);\n", "        s32 v1_v = (t0->unk_03 == 0);\n")
    b = sub(b, "        s32 dx = ent->unk_F4.x - *(s32 *)(a3 + 2);\n        s32 dy = ent->unk_F4.y - *(s32 *)(a3 + 6);\n"
               "        s32 dz = ent->unk_F4.z - *(s32 *)(a3 + 0xA);\n",
            "        s32 dx = ent->unk_F4.x - t0->unk_04.x;\n        s32 dy = ent->unk_F4.y - t0->unk_04.y;\n"
            "        s32 dz = ent->unk_F4.z - t0->unk_04.z;\n")
    b = sub(b, "            s32 v1 = *a3;\n", "            s32 v1 = t0->unk_02;\n") if "s32 v1 = *a3;" in b else sub(b, "*a3 * 30", "t0->unk_02 * 30")
    return sub(b, "    a3 += 0x2C;\n", "")


A.append(("a3", "func_80032314", F17, no_a3))
A.append(("c820", "func_8001C820", F9F, lambda b: sub(drop_fake(b, "/* FAKE: player 1's unk_F4"),
    "    func_800325E0(a0, (s32 *)((u8 *)s0 + 0x536));\n", "    func_800325E0(a0, &D_80101EC8[1].unk_F4.x);\n")))
A.append(("bcf0", "func_8001BCF0", F9F, lambda b: sub(drop_fake(b, "/* FAKE: block copy of unk_B8"),
    "    D_800F6608.unk_00 = *(Vec3i32 *)&arg0->unk_B8;\n",
    "    D_800F6608.unk_00.x = arg0->unk_B8.vx;\n    D_800F6608.unk_00.y = arg0->unk_B8.vy;\n    D_800F6608.unk_00.z = arg0->unk_B8.vz;\n")))
A.append(("a728", "func_8003A728", F287, lambda b: drop_fake(b, "/* FAKE: the record's held / type stores")
    .replace("            u32 *held = &a0->held;\n            s16 *type = a0->type;\n\n", "")
    .replace("*held = ", "a0->held = ").replace("                type[0] = ", "                a0->type[0] = ")
    .replace("                type[1] = ", "                a0->type[1] = ")))
A.append(("f40", "func_80037F40", F287, lambda b: drop_fake(b, "/* FAKE: the save-block view")
    .replace("        Unk800F34D8Save *base = (Unk800F34D8Save *)a0;\n", "").replace("base->", "((Unk800F34D8Save *)a0)->")))
A.append(("kb", "SpuMalloc", FSMM, lambda b: sub(drop_fake(b, "/* FAKE: record address as integer"),
    "            SpuMemRec *kb =\n                (SpuMemRec *)((_spu_AllocLastNum << 3) + (s32)_spu_memList);\n",
    "            SpuMemRec *kb = &_spu_memList[_spu_AllocLastNum];\n")))
A.append(("sdl_xy", "SetDrawLoad", FPRIM, lambda b: sub(b, "    p->code[1] = *(u32 *)&rect->x;\n", "    p->code[1] = (rect->y << 16) | (u16)rect->x;\n")))
A.append(("sdl_wh", "SetDrawLoad", FPRIM, lambda b: sub(b, "    p->code[2] = *(u32 *)&rect->w;\n", "    p->code[2] = (rect->h << 16) | (u16)rect->w;\n")))

# ---- carried FAKEs
A.append(("314_v1v", "func_80032314", F17, lambda b: sub(b, "        s32 v1_v = (*(u8 *)(a3 + 1) == 0);\n        ent = &D_80101EC8[v1_v];\n",
    "        ent = &D_80101EC8[*(u8 *)(a3 + 1) == 0];\n")))
A.append(("314_wrap", "func_80032314", F17, lambda b: sub(b.replace("    do {\n    if (", "    if (", 1),
    "    } while (0);\nnext:\n", "next:\n")))
A.append(("80c_j", "func_8003800C", F287, lambda b: sub(sub(b, "        j = 0;\n        do {\n            sum += *bp;\n            bp++;\n            j++;\n        } while (j < 0x24U);\n",
    "        k = 0;\n        do {\n            sum += *bp;\n            bp++;\n            k++;\n        } while (k < 0x24U);\n"),
    "        s32 sum;\n", "        s32 sum;\n        s32 k;\n")))
A.append(("93c_frame", "func_8003993C", F287, lambda b: sub(b, "    s32 sp120[34];\n", "    s32 sp120[16];\n")))
A.append(("93c_next", "func_8003993C", F287, lambda b: sub(b, "          s32 next = D_800A37D0 + 1;\n          temp = D_800A36F8 - next;\n",
    "          temp = D_800A36F8 - (D_800A37D0 + 1);\n") if "          s32 next" in b else
    sub(b, "        s32 next = D_800A37D0 + 1;\n        temp = D_800A36F8 - next;\n", "        temp = D_800A36F8 - (D_800A37D0 + 1);\n")))
A.append(("a728_zero", "func_8003A728", F287, lambda b: sub(sub(sub(b, "        zero = 0;\n", ""), "D_800A3730 != zero", "D_800A3730 != 0"), "    s32 zero;\n", "")))
A.append(("a728_buf8", "func_8003A728", F287, lambda b: sub(sub(sub(b, "    s32 c0lo;\n", "    s32 c0lo;\n    s32 lo;\n"),
    "                buf8 = (u16)g_comb_send_buf;\n                *held = (c0lo << 16) | buf8;\n",
    "                lo = (u16)g_comb_send_buf;\n                *held = (c0lo << 16) | lo;\n"),
    "                buf8 = (u16)D_800A36D0;\n                *held = (buf8 << 16) | c0lo;\n",
    "                lo = (u16)D_800A36D0;\n                *held = (lo << 16) | c0lo;\n")))
A.append(("cf84_pad", "func_8003CF84", F2B, lambda b: sub(b, "    volatile u32 pre_pad[4];\n", "")))
A.append(("cf84_pad2", "func_8003CF84", F2B, lambda b: sub(b, "    volatile u32 pad2[2];\n", "")))
A.append(("90c_s", "func_8005490C", F3AB, lambda b: re.sub(r"\bs->", "D_800EFAE8.", sub(b, "    Unk800EFAE8Ctrl *s = &D_800EFAE8;\n", ""))))
A.append(("568_arms", "func_80019568", F87, lambda b: sub(sub(b, "            pad.valid[i] = valid;\n            type_m1", "            type_m1"),
    "            pad.valid[i] = valid;\n            bits = 0;\n        }\n", "            bits = 0;\n        }\n        pad.valid[i] = valid;\n")))
A.append(("79c_nd", "func_8001979C", F9F, lambda b: b.replace("            nd = 0xC - bits_left;\n", "", 1)
    .replace("            needed = nd;\n", "            needed = 0xC - bits_left;\n", 1)))
A.append(("79c_hi", "func_8001979C", F9F, lambda b: b.replace("            hi = 0x20 - bits_left;\n            hi = cur >> hi;\n",
    "            hi = cur >> (0x20 - bits_left);\n", 1)))
A.append(("79c_val", "func_8001979C", F9F, lambda b: b.replace("            val = 0x20 - needed;\n            bits_left = val;\n",
    "            bits_left = 0x20 - needed;\n", 1)))


def nd2(b):
    i = b.rindex("            nd = 2 - bits_left;\n")
    b = b[:i] + b[i:].replace("            nd = 2 - bits_left;\n", "", 1)
    j = b.rindex("            needed = nd;\n")
    return b[:j] + b[j:].replace("            needed = nd;\n", "            needed = 2 - bits_left;\n", 1)


A.append(("79c_nd2", "func_8001979C", F9F, nd2))
A.append(("79c_neg2", "func_8001979C", F9F, lambda b: sub(sub(b, "    neg2 = -2;\n", ""), "frame = neg2;", "frame = -2;")))
A.append(("79c_tail", "func_8001979C", F9F, lambda b: sub(b, "    val = 0;\n    base->ctr = val;\n", "    base->ctr = 0;\n")))
A.append(("e404_pad", "func_8001E404", F9F, lambda b: sub(b, "    volatile u32 pre_pad[2];\n", "")))
A.append(("e6e4_pad", "func_8001E6E4", F9F, lambda b: sub(b, "    volatile u32 pre_pad[2];\n", "")))
A.append(("e74_loads", "func_80020E74", F9F, lambda b: sub(b, "    u16 loads[130];", "    u16 loads[2];")))
A.append(("a98_w1", "func_80021A98", F9F, lambda b: sub(b, "        do { } while (0);\n", "")))
A.append(("a98_w2", "func_80021A98", F9F, lambda b: sub(b, "                do { s0->unk_6A = *a0_58; } while (0);\n", "                s0->unk_6A = *a0_58;\n")))
A.append(("a98_mask", "func_80021A98", F9F, lambda b: sub(b, "                s32 v1k = kind2 & 0xFFFF;\n", "                s32 v1k = kind2;\n")))
A.append(("gi_vol", "getintr", FBIOS, lambda b: b.replace("    volatile char nReg;", "    char nReg;").replace("    volatile Result_t buf;", "    Result_t buf;")))
A.append(("tp_arms", "SetDrawTPage", FPRIM, lambda b: re.sub(r"    if \(a1\) \{\n( *)/\*.*?\*/\n *val = \(a3 & GPU_DRAW_MODE_MASK\) \| GPU_DRAW_MODE_TEXOFF;\n    \} else \{\n        val = a3 & GPU_DRAW_MODE_MASK;\n    \}\n",
    "    val = a3 & GPU_DRAW_MODE_MASK;\n    if (a1) {\n        val |= GPU_DRAW_MODE_TEXOFF;\n    }\n", b, flags=re.S)))

# ---- sweep: holders, aliases, reused locals of the moved bodies (each ablated in place)
A.append(("064_speed", "func_80032064", F17, lambda b: inline(b, "    s32 speed = 0x50;\n", "speed", "0x50")))
A.append(("064_vely", "func_80032064", F17, lambda b: inline(b, "    s32 vel_y = -0xC8;\n", "vel_y", "-0xC8")))
A.append(("064_v1", "func_80032064", F17, lambda b: sub(sub(b, "        s32 *v1 = &s0->unk_04.x;\n", ""), "func_80032854(a0_arg, cmd, v1, sp_area);", "func_80032854(a0_arg, cmd, &s0->unk_04.x, sp_area);")))
A.append(("064_a0", "func_80032064", F17, lambda b: inline(b, "        s32 a0_arg = src->unk_B2;\n", "a0_arg", "src->unk_B2")))
A.append(("314_a0", "func_80032314", F17, lambda b: inline(b, "    s32 a0;\n", "a0", "state & 0xFFFF", "    a0 = state & 0xFFFF;\n")))
A.append(("314_v0", "func_80032314", F17, lambda b: sub(b, "            s32 v1 = *a3;\n            s32 v0 = v1 << 4;\n            v0 = v0 - v1;\n            v0 = v0 << 1;\n            v0 = v0 + 0x1F4;\n",
    "            s32 v0 = *a3 * 30 + 0x1F4;\n")))
A.append(("bcf0_diff", "func_8001BCF0", F9F, lambda b: inline(b, "    s32 diff = 0x1000 - arg1;\n", "diff", "0x1000 - arg1")))
A.append(("bcf0_div4", "func_8001BCF0", F9F, lambda b: inline(b, "        s32 div4 = arg1 / 4;\n", "div4", "arg1 / 4")))
A.append(("bcf0_sum", "func_8001BCF0", F9F, lambda b: inline(b, "        s32 sum = arg1 * 3000 + diff * 8000;\n", "sum", "arg1 * 3000 + diff * 8000")))
A.append(("bcf0_lhu", "func_8001BCF0", F9F, lambda b: inline(b, "        u16 lhu_val = arg0->unk_1C8.vy;\n", "lhu_val", "(u16)arg0->unk_1C8.vy")))
A.append(("bcf0_val", "func_8001BCF0", F9F, lambda b: inline(b, "        s32 val;\n", "val", "0xB00 - div4", "        val = 0xB00 - div4;\n")))
A.append(("bf4_cnt", "func_80046BF4", F368, lambda b: inline(b, "        u16 cnt = D_800A38D6;\n", "cnt", "D_800A38D6")))
A.append(("bf4_old", "func_80046BF4", F368, lambda b: inline(b, "        s32 old_ptr = (s32)g_gpu_ot_ptr;\n", "old_ptr", "(s32)g_gpu_ot_ptr")))
A.append(("bf4_nv2", "func_80046BF4", F368, lambda b: inline(b, "    u16 count1;\n", "count1", "cnt + 1", "        count1 = cnt + 1;\n") if "count1" in b else inline(b, "    u16 new_var2;\n", "new_var2", "cnt + 1", "        new_var2 = cnt + 1;\n")))
A.append(("bf4_chain", "func_80046BF4", F368, lambda b: sub(b, "        trans[1] = (trans[0] = 0);\n", "        trans[0] = 0;\n        trans[1] = 0;\n")))
A.append(("bf4_rpap", "func_80046BF4", F368, lambda b: sub(b, """            s32 *rp = result;
            s32 *ap = a0;
            D_80101DF0.work.t[0] = *rp++ + *ap++;
            D_80101DF0.work.t[1] = *rp++ + *ap++;
            D_80101DF0.work.t[2] = *rp++ + *ap++;
""", """            D_80101DF0.work.t[0] = result[0] + a0[0];
            D_80101DF0.work.t[1] = result[1] + a0[1];
            D_80101DF0.work.t[2] = result[2] + a0[2];
""")))
A.append(("568_packets", "func_80019568", F87, lambda b: inline(b, "    u8 *packets;\n", "packets", "(u8 *)&pkts[0]", "    packets = (u8 *)&pkts[0];\n")))
A.append(("568_mode", "func_80019568", F87, lambda b: inline(b, "        s32 mode = D_800A38DC;\n", "mode", "D_800A38DC")))
A.append(("568_rec", "func_80019568", F87, lambda b: inline(b, "        u8 *rec = &packets[i * 8];\n", "rec", "&packets[i * 8]") if "packets" in b else inline(b, "        u8 *rec = (u8 *)pkts + i * 8;\n", "rec", "((u8 *)pkts + i * 8)")))
A.append(("e404_v3", "func_8001E404", F9F, lambda b: inline(b, "        s32 v3 = D_800A36FA;\n", "v3", "D_800A36FA")))
A.append(("e404_fov", "func_8001E404", F9F, lambda b: sub(b, "            s32 fov = 0x2D;\n            if (D_800A36FA == 0) {\n                fov = 0x50;\n            }\n            SetGeomScreen(math_FovToScreenDist(fov));\n",
    "            SetGeomScreen(math_FovToScreenDist(D_800A36FA == 0 ? 0x50 : 0x2D));\n")))
A.append(("e404_p20", "func_8001E404", F9F, lambda b: inline(b, "        s32 *p20 = &s2->w20;\n", "p20", "&s2->w20")))
A.append(("e6e4_p20", "func_8001E6E4", F9F, lambda b: inline(b, "        s32 *p20 = &s2->w20;\n", "p20", "&s2->w20")))
A.append(("f34_mode", "func_80022F34", F9F, lambda b: inline(b, "            s32 mode = D_800A38DC;\n", "mode", "D_800A38DC")))
A.append(("f34_idx", "func_80022F34", F9F, lambda b: inline(inline(b, "                s16 idx1 = rec->unk_4A;\n", "idx1", "rec->unk_4A"),
    "                    s16 idx2 = rec->other->unk_4A;\n", "idx2", "rec->other->unk_4A")))
A.append(("f34_val1", "func_80022F34", F9F, lambda b: inline(b, "                u16 *val1 = D_801027B0[idx1].unk_0C;\n", "val1", "D_801027B0[idx1].unk_0C")))
A.append(("80c_off", "func_8003800C", F287, lambda b: sub(sub(sub(sub(b, "    s32 offset;\n", ""), "    offset = 0;\n", ""),
    "        bp = (u8 *)arg0 + offset;\n", "        bp = (u8 *)arg0 + i * 0x24;\n"), "        offset += 0x24;\n", "")))
A.append(("ddc_v0", "func_80020DDC", F9F, lambda b: sub(sub(b, "    v0 = func_80036EA8(1, 1);\n    cdrom_StartRead(v0, D_800A3830);\n",
    "    cdrom_StartRead(func_80036EA8(1, 1), D_800A3830);\n"), "    s32 v0;\n", "")))
A.append(("ddc_v2", "func_80020DDC", F9F, lambda b: sub(sub(sub(b, "    v2 = *(s32 *)(v1 + 0x10);\n", ""), "(u32 *)(v1 + v2)", "(u32 *)(v1 + *(s32 *)(v1 + 0x10))"), "    s32 v2;\n", "")))
A.append(("ia_list", "_SpuIsInAllocateArea", FSMU, lambda b: inline(b, "    SpuMemRec *list = _spu_memList;\n", "list", "_spu_memList")))
A.append(("ia2_list", "_SpuIsInAllocateArea_", FSMU, lambda b: inline(b, "    SpuMemRec *list = _spu_memList;\n", "list", "_spu_memList")))
A.append(("gi_err", "getintr", FBIOS, lambda b: sub(b, "        bHasError = CD_status;\n        bHasError &= 0x1D;\n", "        bHasError = CD_status & 0x1D;\n")))
A.append(("alo_idx", "_SsVmDoAllocate", FALO, lambda b: sub(sub(b, "    if ((_svm_cur.tone_vag_idx & 1) > 0) {\n        progIdx = (_svm_cur.tone_vag_idx - 1) / 2;\n",
    "    progIdx = (_svm_cur.tone_vag_idx - 1) / 2;\n    if ((_svm_cur.tone_vag_idx & 1) > 0) {\n"), "    } else {\n        progIdx = (_svm_cur.tone_vag_idx - 1) / 2;\n", "    } else {\n")))
A.append(("vin_masked", "_SsVmInit", FVIN, lambda b: sub(b, """    {
        u16 masked = (u8)a0;
        if (masked >= 0x18) {
            _SsVmMaxVoice = 0x18;
        } else {
            _SsVmMaxVoice = masked;
        }
    }
""", """    if ((u8)a0 >= 0x18) {
        _SsVmMaxVoice = 0x18;
    } else {
        _SsVmMaxVoice = (u8)a0;
    }
""")))
A.append(("vsu_sotn", "_SsVmVSetUp", FVSU, lambda b: """s32 _SsVmVSetUp(s32 a0, s32 a1) {
    s16 vabId = a0;
    s16 prog = a1;
    if (vabId < 0 || vabId >= 0x10 || _svm_vab_used[vabId] != 1 || prog >= kMaxPrograms) {
        return -1;
    }
    _svm_vh = _svm_vab_vh[vabId];
    _svm_pg = _svm_vab_pg[vabId];
    _svm_tn = _svm_vab_tn[vabId];
    _svm_cur.vabId = vabId;
    _svm_cur.prog = prog;
    _svm_cur.field_7_fake_program = _svm_pg[prog].reserved1;
    return 0;
}
"""))
A.append(("vsh_a2", "SsVabOpenHeadWithMode", FVSH, lambda b: sub(b, "        var_a2 = _svm_vab_used;\n        if (var_a2[vabid] == 0) {\n",
    "        if (_svm_vab_used[vabid] == 0) {\n")))

def p90c_player(b):
    b = sub(b, "    Unk80045878Obj *player;\n", "    Unk80045878Obj *player;\n    Unk80045878Obj *p0;\n    Unk80045878Obj *p1;\n")
    b = sub(b, "        player = func_8004153C(0);\n        if (player != 0) {\n            func_8003FFC4(player);",
            "        p0 = func_8004153C(0);\n        if (p0 != 0) {\n            func_8003FFC4(p0);")
    return sub(b, "        player = func_8004153C(1);\n        if (player != 0) {\n            func_8003FFC4(player);",
               "        p1 = func_8004153C(1);\n        if (p1 != 0) {\n            func_8003FFC4(p1);")


def p90c_rotz(b):
    b = sub(b, "    s32 rot_z;\n", "    s32 rot_z;\n    s32 rot_z2;\n")
    i = b.rindex("rot_z = (vec.vz * c - vec.vx * sn) >> 12;")
    return b[:i] + b[i:].replace("rot_z = (vec.vz", "rot_z2 = (vec.vz", 1).replace("vec.vz = rot_z;", "vec.vz = rot_z2;", 1)


A.append(("90c_player", "func_8005490C", F3AB, p90c_player))
A.append(("90c_rotz", "func_8005490C", F3AB, p90c_rotz))
A.append(("90c_vy", "func_8005490C", F3AB, lambda b: sub(b, "            vec.vy = frame[0];\n            vec.vy = (vec.vy * player->unk_12) >> 12;\n",
    "            vec.vy = (frame[0] * player->unk_12) >> 12;\n")))
A.append(("cf84_q63", "func_8003CF84", F2B, lambda b: sub(b, "            (&D_800A37D2)[D_800A3748] = (&D_800A37D2)[D_800A3748] + 1;\n",
    "            if (D_800A3748 == 0) {\n                D_800A37D2 = D_800A37D2 + 1;\n            } else {\n                D_800A37D3 = D_800A37D3 + 1;\n            }\n")))


def e74_j(b):
    b = sub(b, "    s32 j; /*", "    s32 j;\n    s32 c; /*")
    return sub(b, "            j = chr0;\n            if (i != 0) {\n                j = chr1;\n            }\n            if (D_800A38C0[i] != j) {\n                D_800A38C0[i] = j;\n"
                  "                cdrom_StartReadAt(func_80036EA8(1, 0), (s32)D_800A3888[i], j * 7, 7);\n",
               "            c = chr0;\n            if (i != 0) {\n                c = chr1;\n            }\n            if (D_800A38C0[i] != c) {\n                D_800A38C0[i] = c;\n"
               "                cdrom_StartReadAt(func_80036EA8(1, 0), (s32)D_800A3888[i], c * 7, 7);\n")


A.append(("e74_j", "func_80020E74", F9F, e74_j))



def main():
    only = set(sys.argv[1:])
    os.makedirs("tmp/w2/abl5", exist_ok=True)
    rows = []
    for name, fn, path, t in A:
        if only and name not in only:
            continue
        p = "tmp/w2/abl5/%s.c" % name
        b0 = body(path, fn)
        b1 = t(b0)
        if b1 == b0:
            raise SystemExit("no change for " + name)
        open(p, "w", encoding="utf-8", newline="\n").write(b1)
        rows.append("%s %s %s" % (name, fn, p))
    open("tmp/w2/abl5/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
    print(len(rows), "candidates")


if __name__ == "__main__":
    main()
