#!/usr/bin/env python3
# Worker-2 batch 5: FZZ in the worker-2 lane (library TUs + the remaining game-TU sites), on main f2bcb4406
# (batches 3 / 4 landed, after worker 1's afbefb601). One section per object; each section is measured on its own.
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/fzz/w2b5.py [opt=...] [out=DIR | apply]
#   default out=tmp/w2/b5 (scratch copies); `apply` writes the working tree.
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f09"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import w2b2 as B2
sys.argv = _argv
NL = chr(10)
BASE = "370450dcc"   # main with batches 3 / 4, worker 1's afbefb601 and fclose1
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT = set(a[4:].split(",")) - {""}
rep, span, body, fn = B2.rep, B2.span, B2.body, B2.fn
SC = {}   # measured ablation scores (sandbox --disable all), filled in after measurement


def base(p):
    B2.BASE = BASE
    return B2.show(p)


SPU = "src/main/psxsdk/libspu/"
MACRO = """/* Self-referential on purpose: the object `_spu_memList` (Sony's SPU_MALLOC list pointer,
   declared s32 in libspu_internal.h) is viewed as SpuMemRec* through this macro; a macro
   name inside its own replacement list is not re-expanded (C90 6.8.3.4). */
#define _spu_memList ((SpuMemRec *)_spu_memList)
"""


# ------------------------------------------------------------------ libspu: _spu_memList is SpuMemRec *
def spu_internal(s):
    return rep(s, "extern s32 _spu_memList;\n", "extern SpuMemRec *_spu_memList;\n")


def s_m_f(s):
    s = rep(s, MACRO + "\n", "")
    return s.replace("((SpuMemRec *)_spu_memList)", "_spu_memList")


def s_m_int(s):
    return rep(s, MACRO + "\n", "")


def s_m_m(s):
    return rep(s, MACRO, "")


def s_m_util(s):
    s = rep(s, MACRO + "\n", "")
    return rep(s, "    SpuMemRec *list = (SpuMemRec *)_spu_memList;\n", "    SpuMemRec *list = _spu_memList;\n", 2)


def s_m_init(s):
    return rep(s, "        _spu_memList = (s32)top;\n", "        _spu_memList = (SpuMemRec *)top;\n")


def s_m_m_kb(s):
    s = s_m_m(s)
    if "kb_sub" in OPT:
        s = rep(s, """            SpuMemRec *kb =
                (SpuMemRec *)((_spu_AllocLastNum << 3) + (s32)_spu_memList);
""", "            SpuMemRec *kb = &_spu_memList[_spu_AllocLastNum];\n")
    if "kb_add" in OPT:
        s = rep(s, """            SpuMemRec *kb =
                (SpuMemRec *)((_spu_AllocLastNum << 3) + (s32)_spu_memList);
""", "            SpuMemRec *kb = _spu_memList + _spu_AllocLastNum;\n")
    if "kb_rev" in OPT:
        s = rep(s, """            SpuMemRec *kb =
                (SpuMemRec *)((_spu_AllocLastNum << 3) + (s32)_spu_memList);
""", "            SpuMemRec *kb = _spu_AllocLastNum + _spu_memList;\n")
    if "kb_sotn" in OPT:
        s = rep(s, """            SpuMemRec *kb =
                (SpuMemRec *)((_spu_AllocLastNum << 3) + (s32)_spu_memList);
            u32 swapAddr = kb->addr;
            u32 swapSize = kb->size;

            kb->addr = _addr | 0x80000000;
            kb->size = _size;
            _spu_AllocLastNum++;
            kb[1].addr = swapAddr;
            kb[1].size = swapSize;
""", """            u32 swapAddr = _spu_memList[_spu_AllocLastNum].addr;
            u32 swapSize = _spu_memList[_spu_AllocLastNum].size;

            _spu_memList[_spu_AllocLastNum].addr = _addr | 0x80000000;
            _spu_memList[_spu_AllocLastNum].size = _size;
            _spu_AllocLastNum++;
            _spu_memList[_spu_AllocLastNum].addr = swapAddr;
            _spu_memList[_spu_AllocLastNum].size = swapSize;
""")
    if not OPT & {"kb_sub", "kb_add", "kb_rev", "kb_sotn"}:
        s = rep(s, "            SpuMemRec *kb =\n                (SpuMemRec *)((_spu_AllocLastNum << 3) + (s32)_spu_memList);\n",
                "            /* FAKE: record address as integer arithmetic (index first); &_spu_memList[n]\n"
                "               adds base first (addu operand order, score %s). */\n"
                "            SpuMemRec *kb =\n                (SpuMemRec *)((_spu_AllocLastNum << 3) + (s32)_spu_memList);\n"
                % SC.get("kb", "?"))
    return s


# ------------------------------------------------------------------ libsnd: the VAB tables and _svm_rattr
SND = "src/main/psxsdk/libsnd/"


def snd_i(s):
    s = rep(s, """extern s32 _svm_vab_vh[];
extern s32 _svm_vab_pg[];
extern s32 _svm_vab_tn[];
/* _svm_rattr: the voice manager's reverb attribute block, declared by its splat per-field
   names (a typed SpuReverbAttr declaration is Phase 2 work). */
extern s32 _svm_rattr;
extern s32 _svm_rattr_plus_0x4;
extern s16 _svm_rattr_plus_0x8;
extern s16 _svm_rattr_plus_0xA;
""", """extern VabHdr *_svm_vab_vh[];
extern ProgAtr *_svm_vab_pg[];
extern VagAtr *_svm_vab_tn[];
extern SpuReverbAttr _svm_rattr; /* the voice manager's reverb attribute block */
""")
    return s


def ut_rdep(s):
    if "rdep_holder" not in OPT:   # SOTN's SsUtSetReverbDepth (ut_rdep.c): no buf holder
        return body(s, "SsUtSetReverbDepth", """void SsUtSetReverbDepth(s16 a0, s16 a1) {
    _svm_rattr.mask = 6;
    _svm_rattr.depth.left = (s16)a0 * 32767 / 127;
    _svm_rattr.depth.right = (s16)a1 * 32767 / 127;
    SpuSetReverbModeParam(&_svm_rattr);
}
""")
    s = rep(s, "    s32 *buf = &_svm_rattr;\n    *buf = 6;\n    _svm_rattr_plus_0x8 = x;\n    _svm_rattr_plus_0xA = y;\n",
            "    SpuReverbAttr *buf = &_svm_rattr;\n    buf->mask = 6;\n    _svm_rattr.depth.left = x;\n"
            "    _svm_rattr.depth.right = y;\n")
    return s


def ut_rev(s):
    s = rep(s, "        _svm_rattr = 1;\n", "        _svm_rattr.mask = 1;\n")
    s = rep(s, "            _svm_rattr_plus_0x4 = (s16)", "            _svm_rattr.mode = (s16)", 2)
    if "rev_view" not in OPT:   # SOTN's SsUtGetReverbType: returns the s32 member as s16
        s = rep(s, "    return *(s16 *)&_svm_rattr_plus_0x4;\n", "    return _svm_rattr.mode;\n")
    else:
        s = rep(s, "    return *(s16 *)&_svm_rattr_plus_0x4;\n", "    return *(s16 *)&_svm_rattr.mode;\n")
    return s


def vm_init(s):
    s = rep(s, "    _svm_rattr_plus_0x8 = 0x3FFF;\n    _svm_rattr_plus_0xA = 0x3FFF;\n",
            "    _svm_rattr.depth.left = 0x3FFF;\n    _svm_rattr.depth.right = 0x3FFF;\n")
    s = rep(s, "    _svm_rattr = 0;\n    _svm_rattr_plus_0x4 = 0;\n", "    _svm_rattr.mask = 0;\n    _svm_rattr.mode = 0;\n")
    return voice_attr(s, "i")


VA_OLD = """    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;
"""
VA_NEW = """    attr.pitch = 0x1000;
    attr.addr = 0x1000;
    attr.adsr1 = 0x80FF;
    attr.volume.left = 0;
    attr.volume.right = 0;
    attr.adsr2 = 0x4000;
"""


def voice_attr(s, i):
    s = rep(s, "    s32 buf[16];\n", "    SpuVoiceAttr attr;\n")
    s = rep(s, "    buf[1] = 0x60093;\n", "    attr.mask = 0x60093;\n")
    s = rep(s, VA_OLD, VA_NEW)
    s = rep(s, "            buf[0] = 1 << %s;\n            func_8008B488(buf);\n" % i,
            "            attr.voice = 1 << %s;\n            func_8008B488(&attr);\n" % i)
    return s


def snd_760d0(s):
    return voice_attr(s, "var_s0")


def vm_vsu(s):
    s = rep(s, """    s32 v0;
    int v1;
    s32 v2;
""", """    VabHdr *v0;
    ProgAtr *v1;
    VagAtr *v2;
""")
    s = rep(s, """    ret = idx << 2;
    v0 = *(s32 *)((u8 *)_svm_vab_vh + ret);
    v1 = *(s32 *)((u8 *)_svm_vab_pg + ret);
    v2 = *(s32 *)((u8 *)_svm_vab_tn + ret);
    ret = sa1 << 4;
    _svm_cur.vabId = (u8) a0h;
    _svm_cur.prog = (u8) a1h;
    ret += v1;
    entry = *((s32 *) (ret + 8));
    _svm_vh = (VabHdr *)v0;
    _svm_pg = v1;
    _svm_tn = (VagAtr *)v2;
""", """    v0 = _svm_vab_vh[idx];
    v1 = _svm_vab_pg[idx];
    v2 = _svm_vab_tn[idx];
    _svm_cur.vabId = (u8) a0h;
    _svm_cur.prog = (u8) a1h;
    entry = v1[sa1].reserved1;
    _svm_vh = v0;
    _svm_pg = v1;
    _svm_tn = v2;
""")
    s = rep(s, "    s32 ret;\n", "")
    return s


def vs_vh(s):
    s = rep(s, "    _svm_vab_vh[vabId_2] = (s32)var_a2;\n", "    _svm_vab_vh[vabId_2] = (VabHdr *)var_a2;\n")
    s = rep(s, "        _svm_vab_pg[vabId_2] = (s32)var_a2;\n", "        _svm_vab_pg[vabId_2] = (ProgAtr *)var_a2;\n")
    s = rep(s, "        _svm_vab_tn[vabId_2] = (s32)var_a2;\n", "        _svm_vab_tn[vabId_2] = (VagAtr *)var_a2;\n")
    return s


def vm_aloc2(s):
    s = rep(s, "((ProgAtr *)_svm_pg)[progIdx]", "_svm_pg[progIdx]", 2)
    return s


# ------------------------------------------------------------------ libcd: CD_status is Sony's int
CD = "src/main/psxsdk/libcd/"


def cd_internal(s):
    return rep(s, "extern u8 CD_status;\n", "")


def cd_bios(s):
    s = rep(s, """ * value). CD_status is Sony's `int`; this TU declares the libc-side u_char
 * view, hence the s32 accesses (as in CD_initintr / CD_init below). */
typedef char Result_t[8];
""", """ * value). */
typedef char Result_t[8];
/* Sony's `int CD_status` (the drive status byte the BIOS layer stores whole; lw / sw here). The
   command layer (sys.c) declares it u_char and reads its low byte (lbu), so each TU carries its
   own declaration, as Sony's two modules do. */
extern s32 CD_status;
""")
    return s.replace("*(s32 *)&CD_status", "CD_status")


def cd_sys(s):
    return rep(s, """/* Forward declarations */
extern s32 DMACallback(s32, s32);
""", """/* Forward declarations */
extern s32 DMACallback(s32, s32);
/* bios.c's `int CD_status`, read here as Sony's command layer declares it: u_char, the low byte
   (lbu in CdStatus, CdControl, CdControlF, CdControlB); bios.c stores the whole word. */
extern u8 CD_status;
""")


# ------------------------------------------------------------------ libapi: the register pointers
API = "src/main/psxsdk/libapi/"


def counter(s):
    s = rep(s, """extern s32 D_8009BD68;
extern s32 D_8009BD6C;
extern s32 D_8009BD70;
""", """/* One root counter's registers (psx-spx "Timers": 0x1F801100 + n * 0x10). */
typedef struct {
    u16 count;  /* +0 current value */
    u16 pad2;
    u16 mode;   /* +4 counter mode */
    u16 pad6;
    u16 target; /* +8 counter target */
    u16 padA[3];
} RCnt;

extern volatile s32 *D_8009BD68; /* .word 0x1F801070 (I_STAT; [1] = I_MASK), asm/data/7D920.data.s */
extern volatile RCnt *D_8009BD6C; /* .word 0x1F801100 (root counters 0..2), asm/data/7D920.data.s */
extern s32 D_8009BD70[]; /* each counter's I_MASK bit: 0x10, 0x20, 0x40, 0x01 */
""")
    s = rep(s, """    s32 v0;
    s32 base;
    t0 = arg0 & 0xFFFF;
""", """    s32 v0;
    t0 = arg0 & 0xFFFF;
""")
    s = rep(s, """    base = (t0 * 0x10) + D_8009BD6C;
    *(volatile u16 *) (base + 4) = 0;
    *(volatile u16 *) (base + 8) = arg1;
""", """    D_8009BD6C[t0].mode = 0;
    D_8009BD6C[t0].target = arg1;
""")
    s = rep(s, "    *(volatile u16 *) (((t0 * 0x10) + D_8009BD6C) + 4) = a3;\n", "    D_8009BD6C[t0].mode = a3;\n")
    s = rep(s, "    return *(volatile u16 *)(D_8009BD6C + v * 0x10);\n", "    return D_8009BD6C[v].count;\n")
    s = rep(s, "    *(volatile u16 *)(D_8009BD6C + v * 0x10) = 0;\n", "    D_8009BD6C[v].count = 0;\n")
    if "rc_old" not in OPT:   # SetRCnt in SOTN's shape (counter.c): no dead v0, no empty statement
        s = body(s, "SetRCnt", """s32 SetRCnt(u32 arg0, s32 arg1, s32 arg2) {
    s32 t0 = arg0 & 0xFFFF;
    s32 a3 = 0x48;
    if (t0 >= 3) {
        return 0;
    }
    D_8009BD6C[t0].mode = 0;
    D_8009BD6C[t0].target = arg1;
    if ((u32)t0 < 2U) {
        if (arg2 & 0x10) {
            a3 = 0x49;
        }
        if (!(arg2 & 1)) {
            a3 |= 0x100;
        }
    } else if (t0 == 2) {
        if (!(arg2 & 1)) {
            a3 = 0x248;
        }
    }
    if ((arg2 & 0x1000) != 0) {
        a3 |= 0x10;
    }
    D_8009BD6C[t0].mode = a3;
    return 1;
}
""")
    if "rc_holder" in OPT:
        s = rep(s, "    base = (volatile s32 *)D_8009BD68;\n", "    base = D_8009BD68;\n", 2)
        s = rep(s, "(&D_8009BD70)[v]", "D_8009BD70[v]", 2)
    else:
        s = rep(s, """    volatile s32 *base;
    v = arg0 & 0xFFFF;
    base = (volatile s32 *)D_8009BD68;
    base[1] = base[1] | (&D_8009BD70)[v];
""", """    v = arg0 & 0xFFFF;
    D_8009BD68[1] |= D_8009BD70[v];
""")
        s = rep(s, """    volatile s32 *base;
    v = arg0 & 0xFFFF;
    base = (volatile s32 *)D_8009BD68;
    base[1] = base[1] & ~(&D_8009BD70)[v];
""", """    v = arg0 & 0xFFFF;
    D_8009BD68[1] &= ~D_8009BD70[v];
""")
    return s


def pad(s):
    s = rep(s, """extern s32 D_8009BD84;
extern s32 D_8009BD88;
""", """extern volatile u16 *D_8009BD84; /* .word 0x1F801040 (JOY_DATA; [5] = JOY_CTRL), asm/data/7D920.data.s */
extern volatile s32 *D_8009BD88; /* .word 0x1F801070 (I_STAT; [1] = I_MASK), asm/data/7D920.data.s */
""")
    s = rep(s, "    *(s16 *)((u8 *)D_8009BD84 + 0xA) = 0;\n", "    D_8009BD84[5] = 0;\n")
    if "pad_holder" in OPT:
        s = rep(s, "    s32 *p = (s32 *)D_8009BD88;\n", "    volatile s32 *p = D_8009BD88;\n")
    else:
        s = rep(s, "    s32 *p = (s32 *)D_8009BD88;\n", "")
        s = rep(s, "    if ((p[1] & 1) == 0) return 0;\n    if ((p[0] & 1) != 0) {\n",
                "    if ((D_8009BD88[1] & 1) == 0) return 0;\n    if ((D_8009BD88[0] & 1) != 0) {\n")
    return s


# ------------------------------------------------------------------ libgpu prim.c: PsyQ's parameter types
GPUH = "include/psxsdk/libgpu.h"


def libgpu_h(s):
    s = rep(s, "#define getcode(p) (u8)(((P_TAG *)(p))->code)\n",
            "#define getcode(p) (u8)(((P_TAG *)(p))->code)\n#define getlen(p) (u8)(((P_TAG *)(p))->len)\n")
    s = rep(s, """/* PsyQ DR_MOVE: DMA tag word, then five GPU command words. */
typedef struct { u32 tag; u32 code[5]; } DR_MOVE;
""", """/* PsyQ DR_MOVE: DMA tag word, then five GPU command words. */
typedef struct { u32 tag; u32 code[5]; } DR_MOVE;

/* PsyQ DR_LOAD: tag, three command words, up to 13 data words; DR_TPAGE: tag, one draw-mode word. */
typedef struct { u32 tag; u32 code[3]; u32 p[13]; } DR_LOAD;
typedef struct { u32 tag; u32 code[1]; } DR_TPAGE;
""")
    s = rep(s, "extern void SetDrawMove(DR_MOVE *, RECT *, u32, u32); /* PsyQ: (DR_MOVE *, RECT *, int, int) */\n",
            """extern void SetDrawMove(DR_MOVE *, RECT *, u32, u32); /* PsyQ: (DR_MOVE *, RECT *, int, int) */
extern void SetDrawLoad(DR_LOAD *, RECT *);
extern void SetDrawTPage(DR_TPAGE *, s32, s32, s32);
extern s32 MargePrim(void *, void *);
extern void DumpDrawEnv(DRAWENV *);
extern void DumpDispEnv(DISPENV *);
""")
    return s


def prim(s):
    s = body(s, "DumpDrawEnv", """void DumpDrawEnv(DRAWENV *env) {
    u32 val;
    GPU_printf(&D_80015D80, env->clip.x, env->clip.y, env->clip.w, env->clip.h);
    GPU_printf(&D_80015D98, env->ofs[0], env->ofs[1]);
    GPU_printf(&D_80015DA8, env->tw.x, env->tw.y, env->tw.w, env->tw.h);
    GPU_printf(&D_80015DC0, env->dtd);
    GPU_printf(&D_80015DCC, env->dfe);
    val = env->tpage;
    GPU_printf(&D_80015D58, (val >> 7) & 3, (val >> 5) & 3, (val << 6) & 0x7C0,
               ((val << 4) & 0x100) + ((val >> 2) & 0x200));
}
""")
    s = body(s, "DumpDispEnv", """void DumpDispEnv(DISPENV *env) {
    GPU_printf(&D_80015DD8, env->disp.x, env->disp.y, env->disp.w, env->disp.h);
    GPU_printf(&D_80015DF4, env->screen.x, env->screen.y, env->screen.w, env->screen.h);
    GPU_printf(&D_80015E10, env->isinter);
    GPU_printf(&D_80015E1C, env->isrgb24);
}
""")
    if "load_end" in OPT:
        tail = "    end = (u32 *)p + size;\n    *end = OT_TERMINATOR;\n"
        decl = "    u32 *end;\n"
    else:
        tail = "    p->code[size - 1] = OT_TERMINATOR;\n"
        decl = ""
    s = body(s, "SetDrawLoad", """void SetDrawLoad(DR_LOAD *p, RECT *rect) {
    u32 nwords;
    s32 size;
%s    nwords = (rect->w * rect->h + 1) / 2;
    size = nwords + 4;
    if (nwords >= 13) {
        size = 0;
    }
    setlen(p, size);
    p->code[0] = GP0_COPY_RECT_C2V;
    /* FAKE: packed RECT word read follows matched Sony-library precedent (as SetDrawMove). */
    p->code[1] = *(u32 *)&rect->x;
    /* FAKE: packed RECT word read follows matched Sony-library precedent (as SetDrawMove). */
    p->code[2] = *(u32 *)&rect->w;
%s}
""" % (decl, tail))
    if "abl_xy" in OPT:
        s = rep(s, "    p->code[1] = *(u32 *)&rect->x;\n", "    p->code[1] = (rect->y << 16) | (u16)rect->x;\n")
    if "abl_wh" in OPT:
        s = rep(s, "    p->code[2] = *(u32 *)&rect->w;\n", "    p->code[2] = (rect->h << 16) | (u16)rect->w;\n")
    if "tpage_val" not in OPT:   # psyz's spelling: env->tpage read in place, no holder
        s = rep(s, "    u32 val;\n    GPU_printf(&D_80015D80, env->clip.x", "    GPU_printf(&D_80015D80, env->clip.x")
        s = rep(s, """    val = env->tpage;
    GPU_printf(&D_80015D58, (val >> 7) & 3, (val >> 5) & 3, (val << 6) & 0x7C0,
               ((val << 4) & 0x100) + ((val >> 2) & 0x200));
""", """    GPU_printf(&D_80015D58, (env->tpage >> 7) & 3, (env->tpage >> 5) & 3, (env->tpage << 6) & 0x7C0,
               ((env->tpage << 4) & 0x100) + ((env->tpage >> 2) & 0x200));
""")
    s = rep(s, "void SetDrawTPage(u8 *a0, s32 a1, s32 a2, u32 a3) {\n",
            "void SetDrawTPage(DR_TPAGE *a0, s32 a1, s32 a2, s32 a3) {\n")
    s = rep(s, "    a0[3] = 1;\n    cmd = GP0_DRAW_MODE;\n", "    setlen(a0, 1);\n    cmd = GP0_DRAW_MODE;\n")
    s = rep(s, "    *(u32 *)(a0 + 4) = cmd | val;\n", "    a0->code[0] = cmd | val;\n")
    s = body(s, "MargePrim", """s32 MargePrim(void *p0, void *p1) {
    s32 size;
    size = getlen(p0) + getlen(p1) + 1;
    if (size >= 17) {
        return -1;
    }
    setlen(p0, size);
    *(u32 *)p1 = 0;
    return 0;
}
""")
    return s


# ------------------------------------------------------------------ 9F9C: the motion decoder's cache records
MOT_TYPES = """
/* One cached decoded frame of a character's motion stream (func_800198D0): the frame number
 * (-2 = empty, func_8001979C), the decoded pose and per-channel rates (u16 channels, the layout
 * of func_800198D0's `work`), and the bitstream reader state after the frame. */
typedef struct Unk800F1B18Slot {
    s32 frame;        /* +0x000 */
    u16 pose[0x42];   /* +0x004 */
    u16 rate[0x42];   /* +0x088 */
    u32 *ptr;         /* +0x10C next stream word */
    s32 bits;         /* +0x110 valid bits in cur */
    u32 cur;          /* +0x114 unread bits, left-aligned */
} Unk800F1B18Slot;    /* sizeof == 0x118 */

/* D_800F1B18[obj]: one character's motion decoder record (func_8001979C sets it up,
 * func_800198D0 decodes through it). stream is the motion bitstream (a byte table of 3-byte
 * keyframe offsets at +0x70); pose / rate the initial channel values func_8001979C unpacks;
 * ctr the round-robin counter over the four cache slots. */
typedef struct Unk800F1B18Rec {
    u32 *stream;              /* +0x000 */
    u16 pose[0x42];           /* +0x004 */
    u16 rate[0x42];           /* +0x088 */
    s32 ctr;                  /* +0x10C */
    Unk800F1B18Slot slot[4];  /* +0x110 */
} Unk800F1B18Rec;             /* sizeof == 0x570 */
"""


E88_TYPE = """
/* D_80104E88[4]: the four 0x2C-byte records func_80032064 claims (first with unk_00 == 0) and
 * func_800321E8 steps each frame (unk_02 counts frames; unk_10 keeps the previous unk_04;
 * unk_1C is added to unk_04 with unk_1C.y growing by 0xD; the record is freed when
 * func_8005344C reports a hit or unk_04.y passes unk_28). func_80032314 measures the distance
 * from player unk_03's unk_F4 to unk_04. func_80032040 clears unk_00 of all four. */
typedef struct Unk80104E88Rec {
    u8 unk_00;        /* 0 = free; func_80032064's type (1 / 2) */
    u8 unk_01;
    u8 unk_02;
    u8 unk_03;        /* player index (Unk80101EC8Record.index) */
    Vec3i32 unk_04;
    Vec3i32 unk_10;
    Vec3i32 unk_1C;
    s32 unk_28;
} Unk80104E88Rec;     /* sizeof == 0x2C */
"""


SAVE_TYPE = """
/* The 0x100-byte save block after the memory-card header (D_800F34D8 = the 0x200-byte card
 * buffer + 0x100): func_80037F40 writes three copies of D_80106A50 with their byte checksums
 * and clears the pointer / value table; func_8003800C restores the first copy whose checksum
 * holds (unless its flags bit 7 is set) and writes each val[j] through ptr[j] when ptr[j] is
 * a KSEG0 RAM address. */
typedef struct Unk800F34D8Save {
    FileRecord rec[3];   /* +0x00 */
    s32 sum[3];          /* +0x6C byte sums of rec[] */
    u16 *ptr[0x16];      /* +0x78 */
    u16 val[0x16];       /* +0xD0 */
    s32 unk_FC;
} Unk800F34D8Save;       /* sizeof == 0x100 */
"""


PACK_TYPE = """
/* A loaded motion pack's five section pointers. D_801027B0[ch] for character ch's pack (func_80020E74:
 * unk_00 = record + 0x6C + (unk_03 - 1) * 6, unk_04..unk_10 = record + unk_04[0..3]); D_80102760 for
 * the common pack (func_80020DDC: unk_00 = file + 0x14, the others file + its header words 1 / 2 / 4;
 * unk_0C unset). unk_00: halfword streams func_80021424 indexes (the f4E / f54 / f66 / f16 / f18
 * entries); unk_04: 4-byte entries whose second halfword offsets unk_08 (func_80021A98,
 * func_8003993C); unk_0C: func_80055138's table; unk_10: func_8001979C's motion bitstream. */
typedef struct Unk801027B0Pack {
    u16 *unk_00;
    u16 *unk_04;
    u8 *unk_08;
    u16 *unk_0C;
    u32 *unk_10;
} Unk801027B0Pack;            /* sizeof == 0x14 */
"""


def game_h(s):
    s = rep(s, "    u16 f66[11][3];\n} Tbl800A3860Entry;\n", "    u16 f66[11][3];\n} Tbl800A3860Entry;\n" + PACK_TYPE)
    s = rep(s, " * fields at +0x4E are indices into D_801027B0[ch][0], as func_80021424 reads",
            " * fields at +0x4E are indices into D_801027B0[ch].unk_00, as func_80021424 reads")
    s = rep(s, " * halfwords end exactly at f4E) are indices into D_80102760; both readers load",
            " * halfwords end exactly at f4E) are indices into D_80102760.unk_00; both readers load")
    s = rep(s, "    u8 unk_03;                     /* D_801027B0[ch][0] = record + 0x6C + (unk_03 - 1) * 6 (func_80020E74) */\n"
               "    s32 unk_04[4];                 /* D_801027B0[ch][1 + k] = record + unk_04[k] (func_80020E74) */\n",
            "    u8 unk_03;                     /* D_801027B0[ch].unk_00 = record + 0x6C + (unk_03 - 1) * 6 (func_80020E74) */\n"
            "    s32 unk_04[4];                 /* D_801027B0[ch].unk_04 .. unk_10 = record + unk_04[k] (func_80020E74) */\n")
    s = rep(s, "    u8 flags;               /* 0x80106A73: bits 0/1/2 = file_GetFlag0/1/2 */\n} FileRecord;\n",
            "    u8 flags;               /* 0x80106A73: bits 0/1/2 = file_GetFlag0/1/2 */\n} FileRecord;\n" + SAVE_TYPE)
    s = rep(s, "} MotionFrame;                     /* sizeof == 0x84 */\n",
            "} MotionFrame;                     /* sizeof == 0x84 */\n" + MOT_TYPES)
    s = rep(s, "typedef struct { s16 vx, vy, vz, pad; } SVec4i16;\n",
            "typedef struct { s16 vx, vy, vz, pad; } SVec4i16;\n" + E88_TYPE)
    return s


def bb2_h(s):
    s = rep(s, "extern u8 D_800F1B18[];\n", "extern Unk800F1B18Rec D_800F1B18[];\n")
    s = rep(s, "extern s32 D_80102760;\nextern s32 D_80102764;\nextern s32 D_80102768;\nextern s32 D_80102770;\n",
            "extern Unk801027B0Pack D_80102760; /* the common motion pack */\n")
    s = rep(s, "extern s32 D_801027B0[][5];\n", "extern Unk801027B0Pack D_801027B0[]; /* per character */\n")
    s = rep(s, "extern s32 D_800A36B4;\n", "extern Rec44 *D_800A36B4; /* the active camera record (func_8001E404 / func_8001E6E4) */\n")
    s = rep(s, "extern u8 D_80104E88;\n", "extern Unk80104E88Rec D_80104E88[];\n")
    s = rep(s, "extern u8 *func_80032064(Unk80101EC8Record *, s32);\n",
            "extern Unk80104E88Rec *func_80032064(Unk80101EC8Record *, s32);\n")
    s = rep(s, "extern void func_8003A728(s32);\n", "extern void func_8003A728(PadState *);\n")
    s = rep(s, "extern void func_800418D0(s32 *);\n", "extern void func_800418D0(Unk80101DF0Record *);\n")
    s = rep(s, "extern void func_80032854(s32, s32, s32 *, s16 *);\n",
            "extern void func_800325E0(s32, s32 *);\nextern void func_80032854(s32, s32, s32 *, s16 *);\n")
    return s


def f17afc(s):
    if "clr_idx" in OPT:
        s = fn(s, "func_80032040", lambda b: rep(b, """    for (i = 0x84; i >= 0; i -= 0x2C) {
        (&D_80104E88)[i] = 0;
    }
""", """    for (i = 3; i >= 0; i--) {
        D_80104E88[i].unk_00 = 0;
    }
"""))
    elif "clr_ptr" in OPT:
        s = fn(s, "func_80032040", lambda b: rep(rep(b, "    s32 i;\n", "    Unk80104E88Rec *p;\n"), """    for (i = 0x84; i >= 0; i -= 0x2C) {
        (&D_80104E88)[i] = 0;
    }
""", """    for (p = &D_80104E88[3]; p >= D_80104E88; p--) {
        p->unk_00 = 0;
    }
"""))
    else:
        s = fn(s, "func_80032040", lambda b: rep(b, "        (&D_80104E88)[i] = 0;\n",
                                                 "        /* FAKE: byte-offset walk over the records' unk_00; D_80104E88[i].unk_00 for\n"
                                                 "           i = 3..0 keeps a separate counter (score %s). */\n"
                                                 "        ((u8 *)D_80104E88)[i] = 0;\n" % SC.get("clr", "?")))
    s = rep(s, """void func_8002906C(void) {
    s16 *ptr = (s16 *)func_8004678C();
    while (*(s16 *)ptr != 0) {
        *(s16 *)((u8 *)ptr + 2) = 0;
        ptr = (s16 *)((u8 *)ptr + 0x10);
    }
}
/* One 16-byte entry of the list func_8004678C returns (terminated by a zero
 * `type`); func_8002906C clears every entry's `used`. */
typedef struct {
    s16 type;
    s16 used;
    s32 x;
    s32 y;
    s32 z;
} PosRec;
""", """/* One 16-byte entry of the list func_8004678C returns (terminated by a zero
 * `type`); func_8002906C clears every entry's `used`. */
typedef struct {
    s16 type;
    s16 used;
    s32 x;
    s32 y;
    s32 z;
} PosRec;

void func_8002906C(void) {
    PosRec *ptr = (PosRec *)func_8004678C();
    while (ptr->type != 0) {
        ptr->used = 0;
        ptr++;
    }
}
""")
    if "rnd_sub" in OPT:
        s = fn(s, "func_800342A0", lambda b: rep(rep(b, """            s32 v0 = D_800A3874;
            v0 = v0 << 1;
            v0 = v0 + (s32)a1;
            v0 = *((u8 *)v0 + v1);
            D_80102778.unk_4[v1] = v0;
""", """            D_80102778.unk_4[v1] = (a1 + (D_800A3874 << 1))[v1];
"""), """            s32 v0 = D_800A3874;
            v0 = v0 << 1;
            v0 = v0 + (s32)a0;
            v0 = *((u8 *)v0 + v1);
            D_80102778.unk_4[2 + v1] = v0;
""", """            D_80102778.unk_4[2 + v1] = (a0 + (D_800A3874 << 1))[v1];
"""))
    if "ws" in OPT:   # trial only: index arithmetic changes (debt)
        def ws(b):
            b = rep(b, "    s32 *p;\n", "    LeafPos *p;\n")
            return rep(b, """            p = (s32 *)((u8 *)ws + (i * 0x60 + j * 0x18));
            p[0] >>= 1;
            p[1] >>= 1;
            p[2] >>= 1;
            p[3] >>= 1;
            p[4] >>= 1;
            p[5] >>= 1;
""", """            p = &ws[i * 8 + j * 2];
            p[0].x >>= 1;
            p[0].y >>= 1;
            p[0].z >>= 1;
            p[1].x >>= 1;
            p[1].y >>= 1;
            p[1].z >>= 1;
""")
        s = fn(s, "func_80029454", ws)
    if "fdb0" in OPT:
        def fdb0(b):
            for off, (i, m) in {0xB4: (1, "x"), 0xB8: (1, "y"), 0xBC: (1, "z"), 0xC0: (2, "x"), 0xC4: (2, "y"),
                                0xC8: (2, "z"), 0xCC: (3, "x"), 0xD0: (3, "y"), 0xD4: (3, "z")}.items():
                b = b.replace("*(s32 *)((u8 *)0x%X + stride)" % (0x1F800000 + off), "SPAD->unkA8[arg0->index][%d].%s" % (i, m))
            if "fdb0b" in OPT:
                b = b.replace("SPAD->unkA8[arg0->index]", "pts").replace("    s32 stride;\n", "    LeafPos *pts;\n")
                b = rep(b, "    stride = arg0->index * 264;\n", "    pts = SPAD->unkA8[arg0->index];\n")
            return b
        s = fn(s, "func_8002FDB0", fdb0)
    if "rnd_sub2" in OPT:
        s = s.replace("(a1 + (D_800A3874 << 1))[v1]", "((D_800A3874 << 1) + a1)[v1]").replace(
            "(a0 + (D_800A3874 << 1))[v1]", "((D_800A3874 << 1) + a0)[v1]")
    s = fn(s, "func_80032064", e88_new)
    s = fn(s, "func_800321E8", e88_step)
    s = fn(s, "func_80032314", e88_dist)
    s = fn(s, "func_800325E0", lambda b: rep(rep(b, """    dx = *(s32 *)((u8 *)D_800A36B4 + 0x20) - arg1[0];
    dy = *(s32 *)((u8 *)D_800A36B4 + 0x24) - arg1[1];
    dz = *(s32 *)((u8 *)D_800A36B4 + 0x28) - arg1[2];
""", """    dx = D_800A36B4->w20 - arg1[0];
    dy = D_800A36B4->w24 - arg1[1];
    dz = D_800A36B4->w28 - arg1[2];
"""), "    listener_angle = *(s16 *)((u8 *)D_800A36B4 + 0x12);\n", "    listener_angle = D_800A36B4->h12;\n"))
    return s


def f28708(s):
    # the save block: func_8003800C's caller keeps its splat label D_800F34D8 (now typed); func_80037F40's
    # caller keeps HEAD's D_800F33D8 + 0x100 (the target forms it from the card buffer's register) and its
    # u8 * parameter, viewed as the save block inside. Not merged (debt row): D_800F33D8 is a shared work buffer with several layouts.
    s = pack_28708(s)
    s = rep(s, "extern s32 D_800F34D8;\n", "extern Unk800F34D8Save D_800F34D8; /* = D_800F33D8 + 0x100 (debt row) */\n")
    s = rep(s, "extern s32 func_8003800C(s32 *);\n", "extern s32 func_8003800C(Unk800F34D8Save *);\n")
    s = fn(s, "func_8003A728", lambda b: rep(rep(rep(rep(rep(rep(rep(b,
        "void func_8003A728(s32 a0) {\n", "void func_8003A728(PadState *a0) {\n"),
        "        buf8 = *(s32 *)(a0 + 8);\n", "        buf8 = a0->held;\n"),
        "               | (*(s16 *)a0 << 16) | (buf8 & 0xFFFF);\n", "               | (a0->type[0] << 16) | (buf8 & 0xFFFF);\n"),
        "                *(s32 *)(a0 + 8) = (c0lo << 16) | buf8;\n", "                a0->held = (c0lo << 16) | buf8;\n"),
        "                *(s32 *)(a0 + 8) = (buf8 << 16) | c0lo;\n", "                a0->held = (buf8 << 16) | c0lo;\n"),
        "                *(s16 *)a0 = t & 0xF;\n", "                a0->type[0] = t & 0xF;\n"),
        "                *(s16 *)(a0 + 2) = t & 0xF;\n", "                a0->type[1] = t & 0xF;\n", 2))
    if "a7_member" not in OPT:
        s = fn(s, "func_8003A728", lambda b: rep(rep(rep(rep(rep(b,
            "        if (D_800A3916 == 0) {\n", "        if (D_800A3916 == 0) {\n"
            "            /* FAKE: the record's held / type stores go through pointers; stored as members\n"
            "               (in-struct memory) sched moves the D_800A36C2 / D_800A36D2 loads above them\n"
            "               (score %s). */\n"
            "            u32 *held = &a0->held;\n            s16 *type = a0->type;\n\n" % SC.get("a728", "?")),
            "                a0->held = (c0lo << 16) | buf8;\n", "                *held = (c0lo << 16) | buf8;\n"),
            "                a0->held = (buf8 << 16) | c0lo;\n", "                *held = (buf8 << 16) | c0lo;\n"),
            "                a0->type[0] = t & 0xF;\n", "                type[0] = t & 0xF;\n"),
            "                a0->type[1] = t & 0xF;\n", "                type[1] = t & 0xF;\n", 2))
    if "a7_h" in OPT:
        s = fn(s, "func_8003A728", lambda b: rep(rep(rep(b,
            "        if (D_800A3916 == 0) {\n", "        if (D_800A3916 == 0) {\n            u32 *held = &a0->held;\n\n"),
            "                a0->held = (c0lo << 16) | buf8;\n", "                *held = (c0lo << 16) | buf8;\n"),
            "                a0->held = (buf8 << 16) | c0lo;\n", "                *held = (buf8 << 16) | c0lo;\n"))
    if "a7_t" in OPT:
        s = fn(s, "func_8003A728", lambda b: rep(rep(rep(b,
            "        if (D_800A3916 == 0) {\n", "        if (D_800A3916 == 0) {\n            s16 *type = a0->type;\n\n"),
            "                a0->type[0] = t & 0xF;\n", "                type[0] = t & 0xF;\n"),
            "                a0->type[1] = t & 0xF;\n", "                type[1] = t & 0xF;\n", 2))
    if "a7_raw" in OPT:
        s = fn(s, "func_8003A728", lambda b: b.replace("                a0->held = ", "                *(s32 *)&a0->held = ")
               .replace("                a0->type[0] = ", "                *(s16 *)a0->type = ").replace("                a0->type[1] = ", "                *(s16 *)&a0->type[1] = "))
    s = body(s, "func_8003AA48", """void func_8003AA48(void) {
    PadState pad;
    pad.held = 0;
    pad.type[1] = 4;
    pad.type[0] = 4;
    func_8003A728(&pad);
}
""")
    s = fn(s, "func_80037F40", save_write)
    s = fn(s, "func_8003800C", save_read)
    return s


def save_write(b):
    # the parameter stays HEAD's u8 * (the caller passes the card buffer's second half, D_800F33D8 + 0x100)
    b = rep(b, "        u8 *base = a0;\n",
            "        /* FAKE: the save-block view in its own local; through a0 (a cast at each use) the copy\n"
            "           moves into the checksum loop's delay slot (score %s). */\n"
            "        Unk800F34D8Save *base = (Unk800F34D8Save *)a0;\n" % SC.get("f40", "?"))
    b = rep(b, """            ((FileRecord *)base)[i] = D_80106A50;
            *(s32 *)(base + i * 4 + 0x6C) = checksum;
            {
                s32 j = 0;
                s16 *hp = (s16 *)base;
                u8 *bp = base;
                do {
                    *(s32 *)(bp + 0x78) = 0;
                    *(s16 *)((u8 *)hp + 0xD0) = 0;
                    hp++;
                    j++;
                    bp += 4;
                } while (j < 0x16);
            }
""", """            base->rec[i] = D_80106A50;
            base->sum[i] = checksum;
            {
                s32 j = 0;
                do {
                    base->ptr[j] = 0;
                    base->val[j] = 0;
                    j++;
                } while (j < 0x16);
            }
""")
    b = rep(b, "        *(s32 *)(base + 0xFC) = 0;\n", "        base->unk_FC = 0;\n")
    if "sw_nobase" in OPT:
        b = re.sub(r"        /\* FAKE: the save-block view.*?Unk800F34D8Save \*base = \(Unk800F34D8Save \*\)a0;\n", "", b, flags=re.S).replace("base->", "((Unk800F34D8Save *)a0)->")
    return b


def save_read(b):
    b = rep(b, "s32 func_8003800C(s32 *arg0) {\n    u8 *base = (u8 *)arg0;\n",
            "s32 func_8003800C(Unk800F34D8Save *arg0) {\n    u8 *base = (u8 *)arg0;\n")
    b = rep(b, "        u8 *src = base + i * 0x24;\n\n        if (!(*(src + 0x23) & 0x80)) {\n            D_80106A50 = *(FileRecord *)src;\n",
            "        FileRecord *src = &arg0->rec[i];\n\n        if (!(src->flags & 0x80)) {\n            D_80106A50 = *src;\n")
    b = rep(b, """            u16 *ptr = *(u16 **)(base + j * 4 + 0x78);
            if ((u32)((u32)ptr - 0x80000000U) <= 0x1FFFFF) {
                *ptr = *(u16 *)(base + j * 2 + 0xD0);
""", """            u16 *ptr = arg0->ptr[j];
            if ((u32)((u32)ptr - 0x80000000U) <= 0x1FFFFF) {
                *ptr = arg0->val[j];
""")
    if "sr_base" not in OPT:   # no u8 copy of the parameter
        b = rep(b, "    u8 *base = (u8 *)arg0;\n", "")
        b = rep(b, "    chkptr = (s32 *)base;\n", "    chkptr = (s32 *)arg0;\n")
        b = rep(b, "        bp = base + offset;\n", "        bp = (u8 *)arg0 + offset;\n")
    if "sr_chk" not in OPT:   # the checksum compare reads arg0->sum[i] (no chkptr cursor)
        b = rep(b, "    s32 *chkptr;\n", "")
        b = rep(b, "    chkptr = (s32 *)arg0;\n", "")
        b = rep(b, "        if (sum == *(s32 *)((u8 *)chkptr + 0x6C)) {\n", "        if (sum == arg0->sum[i]) {\n")
        b = rep(b, "        chkptr++;\n", "")
    if "sr_sum" in OPT:
        b = rep(b, "    chkptr = (s32 *)arg0;\n", "    chkptr = arg0->sum;\n")
        b = rep(b, "        if (sum == *(s32 *)((u8 *)chkptr + 0x6C)) {\n", "        if (sum == *chkptr) {\n")
    return b


def f31d3c(s):
    if "df0_blk" in OPT:
        copy = """    ((Block16 *)&a0->xf.mat)[0] = ((Block16 *)&a0->work)[0];
    ((Block16 *)&a0->xf.mat)[1] = ((Block16 *)&a0->work)[1];
"""
    else:
        copy = "    a0->xf.mat = a0->work;\n"
        s = rep(s, "typedef struct { s32 w[4]; } Block16;\n", "")
    s = body(s, "func_800418D0", """void func_800418D0(Unk80101DF0Record *a0) {
    SVECTOR sp10;
    AnimRotFunc func;
    sp10.vx = -(u16)a0->xf.rot.vx;
    sp10.vy = -(u16)a0->xf.rot.vy;
    sp10.vz = -(u16)a0->xf.rot.vz;
    func = g_anim_func_table[a0->unk8];
    func(&sp10, &a0->work);
%s}
""" % copy)
    return s


def f368e4(s):
    s = rep(s, "        func_800418D0((s32 *)&D_80101DF0);\n", "        func_800418D0(&D_80101DF0);\n")
    s = rep(s, "    func_800418D0((s32 *)p1);\n", "    func_800418D0(p1);\n")
    s = rep(s, "    func_800418D0((s32 *)p2);\n", "    func_800418D0(p2);\n")
    return s


def f35000(s):
    s = rep(s, "extern s16 D_800963EE;\n", "", 2)
    s = rep(s, "(s32)*(s16 *)((u8 *)&D_800963EE + a0 * 4) << 11", "D_800963EC[a0].length_sectors << 11", 3)
    return s


def named_syms(s):
    s = re.sub(r"_svm_rattr( +)= 0x800F5750;  /\* command ID \(1 = pos input, 6 = analog xy\) \*/",
               r"_svm_rattr\1= 0x800F5750;  /* SpuReverbAttr: the libsnd voice manager's reverb attributes */", s)
    if s.count("SpuReverbAttr: the libsnd voice manager's reverb attributes") != 1:
        raise SystemExit("named_syms _svm_rattr line")
    for sym in ("_svm_rattr_plus_0x4", "_svm_rattr_plus_0x8", "_svm_rattr_plus_0xA", "g_motion_frame_offset_table",
                "g_practice_halfword_table_plus_4", "g_practice_halfword_table_plus_8",
                "g_practice_halfword_table_plus_8_plus_8", "g_motion_keyframe_table_ptr_plus_4",
                "g_motion_keyframe_table_ptr_plus_8"):
        s, n = re.subn(r"(?m)^%s +=.*\n" % sym, "", s)
        if n != 1:
            raise SystemExit("named_syms " + sym)
    s = rep(s, "= 0x80102760;  /* u16 table base; D + idx*2 = idx'th halfword */",
            "= 0x80102760;  /* Unk801027B0Pack: the common motion pack's section pointers */")
    s = rep(s, "= 0x801027B0;  /* ref: g_motion_data_base_ptr */",
            "= 0x801027B0;  /* Unk801027B0Pack[]: each character's motion pack section pointers */")
    return s


def undef_syms(s):
    for sym in ("_svm_rattr_plus_0x4", "_svm_rattr_plus_0x8", "_svm_rattr_plus_0xA", "D_800963EE",
                "D_80102764", "D_80102768", "D_80102770"):
        s, n = re.subn(r"(?m)^%s = .*\n" % sym, "", s)
        if n != 1:
            raise SystemExit("undefined_syms " + sym)
    return s


def e88_new(b):
    b = rep(b, "u8 *func_80032064(Unk80101EC8Record *src, s32 type) {\n",
            "Unk80104E88Rec *func_80032064(Unk80101EC8Record *src, s32 type) {\n")
    b = rep(b, "    u8 *ptr = &D_80104E88;\n    u8 *s0;\n", "    Unk80104E88Rec *ptr = D_80104E88;\n    Unk80104E88Rec *s0;\n")
    b = rep(b, "        if (*s0 == 0) break;\n        ptr = s0 + 0x2C;\n", "        if (s0->unk_00 == 0) break;\n        ptr = s0 + 1;\n")
    b = rep(b, """    *s0 = type;
    *(s0 + 1) = 1;
    *(s0 + 2) = 0;
    *(s0 + 3) = src->index;
    *(s32 *)(s0 + 4) = src->unk_F4.x;
""", """    s0->unk_00 = type;
    s0->unk_01 = 1;
    s0->unk_02 = 0;
    s0->unk_03 = src->index;
    s0->unk_04.x = src->unk_F4.x;
""")
    b = rep(b, "        *(s32 *)(s0 + 8) = src->unk_B8.vy - (v1 >> 5);\n", "        s0->unk_04.y = src->unk_B8.vy - (v1 >> 5);\n")
    b = rep(b, """    *(s32 *)(s0 + 0xC) = src->unk_F4.z;
    *(s32 *)(s0 + 0x1C) = ((s32)Judge[src->unk_1C8.vy & 0xFFF] * speed) >> 12;
    *(s32 *)(s0 + 0x20) = vel_y;
    *(s32 *)(s0 + 0x24) = ((s32)Judge[(src->unk_1C8.vy + 0x400) & 0xFFF] * speed) >> 12;
    *(Vec3_copy *)(s0 + 0x10) = *(Vec3_copy *)(s0 + 4);
    *(s32 *)(s0 + 0x28) = src->unk_B8.vy;
""", """    s0->unk_04.z = src->unk_F4.z;
    s0->unk_1C.x = ((s32)Judge[src->unk_1C8.vy & 0xFFF] * speed) >> 12;
    s0->unk_1C.y = vel_y;
    s0->unk_1C.z = ((s32)Judge[(src->unk_1C8.vy + 0x400) & 0xFFF] * speed) >> 12;
    s0->unk_10 = s0->unk_04;
    s0->unk_28 = src->unk_B8.vy;
""")
    b = rep(b, "        u8 *v1 = s0 + 4;\n", "        s32 *v1 = &s0->unk_04.x;\n")
    b = rep(b, "        func_80032854(a0_arg, cmd, (s32 *)v1, sp_area);\n", "        func_80032854(a0_arg, cmd, v1, sp_area);\n")
    return b


def e88_step(b):
    b = rep(b, "    u8 *base = &D_80104E88;\n", "    Unk80104E88Rec *base = D_80104E88;\n")
    b = rep(b, """        if (*base != 0) {
            *(u8 *)(base + 2) += 1;
            *(Vec3i32 *)(base + 0x10) = *(Vec3i32 *)(base + 4);
            *(s32 *)(base + 0x20) += 0xD;
            scr->unk00.x = *(s32 *)(base + 4) + *(s32 *)(base + 0x1C);
            scr->unk00.y = *(s32 *)(base + 8) + *(s32 *)(base + 0x20);
            scr->unk00.z = *(s32 *)(base + 0xC) + *(s32 *)(base + 0x24);
            if (func_8005344C((s32 *)(base + 4), &scr->unk00.x, &scr->unk10.x, scr->unk30, (s32)&scr->unk38) != 0 || *(s32 *)(base + 8) > *(s32 *)(base + 0x28)) {
                *base = 0;
            } else {
                *(Vec3i32 *)(base + 4) = scr->unk00;
            }
""", """        if (base->unk_00 != 0) {
            base->unk_02 += 1;
            base->unk_10 = base->unk_04;
            base->unk_1C.y += 0xD;
            scr->unk00.x = base->unk_04.x + base->unk_1C.x;
            scr->unk00.y = base->unk_04.y + base->unk_1C.y;
            scr->unk00.z = base->unk_04.z + base->unk_1C.z;
            if (func_8005344C(&base->unk_04.x, &scr->unk00.x, &scr->unk10.x, scr->unk30, (s32)&scr->unk38) != 0 || base->unk_04.y > base->unk_28) {
                base->unk_00 = 0;
            } else {
                base->unk_04 = scr->unk00;
            }
""")
    b = rep(b, "        base += 0x2C;\n", "        base++;\n")
    return b


def e88_dist(b):
    b = rep(b, "    u8 *t0 = &D_80104E88;\n", "    Unk80104E88Rec *t0 = D_80104E88;\n")
    b = rep(b, "    if (*t0 == 0) goto next;\n", "    if (t0->unk_00 == 0) goto next;\n")
    b = rep(b, "                *t0 = 0;\n", "                t0->unk_00 = 0;\n")
    b = rep(b, "    t0 += 0x2C;\n", "    t0++;\n")
    if "e88_noa3" in OPT:
        b = rep(b, "    u8 *a3 = &D_80104E88 + 2;\n", "")
        b = rep(b, "        s32 v1_v = (*(u8 *)(a3 + 1) == 0);\n", "        s32 v1_v = (t0->unk_03 == 0);\n")
        b = rep(b, """        s32 dx = ent->unk_F4.x - *(s32 *)(a3 + 2);
        s32 dy = ent->unk_F4.y - *(s32 *)(a3 + 6);
        s32 dz = ent->unk_F4.z - *(s32 *)(a3 + 0xA);
""", """        s32 dx = ent->unk_F4.x - t0->unk_04.x;
        s32 dy = ent->unk_F4.y - t0->unk_04.y;
        s32 dz = ent->unk_F4.z - t0->unk_04.z;
""")
        b = rep(b, "            s32 v1 = *a3;\n", "            s32 v1 = t0->unk_02;\n")
        b = rep(b, "    a3 += 0x2C;\n", "")
    else:
        b = rep(b, "    u8 *a3 = &D_80104E88 + 2;\n",
                "    /* FAKE: a second, byte cursor a3 at each record's unk_02, read for unk_02 (*a3),\n"
                "       unk_03 (a3 + 1) and unk_04 (a3 + 2..); reading them through t0 drops the second\n"
                "       induction register (score %s). */\n"
                "    u8 *a3 = &D_80104E88[0].unk_02;\n" % SC.get("a3", "?"))
    return b


def f9f9c(s):
    s = pack_9f9c(s)
    s = fn(s, "func_8001979C", mot_init)
    s = fn(s, "func_800198D0", mot_decode)
    s = rep(s, "   D_800F1B18[obj * 0x570] caches", "   D_800F1B18[obj] caches")
    s = body(s, "func_8001BCF0", """void func_8001BCF0(Unk80101EC8Record *arg0, s32 arg1) {
    s32 diff = 0x1000 - arg1;

    func_8003F1E4(0);

    /* FAKE: block copy of unk_B8's x / y / z (three lw, then three sw through one base); the
       three member assignments interleave the loads and stores (score %s). */
    D_800F6608.unk_00 = *(Vec3i32 *)&arg0->unk_B8;

    D_800F6608.unk_00.y -= 0x44C;

    D_800F6608.h10 = 0x100 - (arg1 * 288) / 4096;

    {
        s32 div4 = arg1 / 4;
        s32 sum = arg1 * 3000 + diff * 8000;
        u16 lhu_val = arg0->unk_1C8.vy;
        s32 val;
        D_800F6608.w18 = sum >> 12;
        val = 0xB00 - div4;
        D_800F6608.h14 = 0;
        D_800F6608.h12 = val - lhu_val;
    }
}
""" % SC.get("bcf0", "?"))
    s = rep(s, "    func_8001BCF0((u8 *)&D_80101EC8[D_800A3748], ", "    func_8001BCF0(&D_80101EC8[D_800A3748], ")
    s = rep(s, "extern void func_800325E0(s32, s32);\n", "")
    if "c820_rec" in OPT:
        s = fn(s, "func_8001C820", lambda b: rep(rep(rep(b, "    s16 *s0 = &D_80101EC8[0].unk_0A;\n",
                                                         "    Unk80101EC8Record *s0 = D_80101EC8;\n"),
                                                     "    if (D_8008D9EC[*s0] != 0) {\n", "    if (D_8008D9EC[s0->unk_0A] != 0) {\n"),
                                                 "    func_800325E0(a0, (s32)((u8 *)s0 + 0x536));\n",
                                                 "    func_800325E0(a0, &s0[1].unk_F4.x);\n"))
    elif "c820_direct" in OPT:
        s = rep(s, "    func_800325E0(a0, (s32)((u8 *)s0 + 0x536));\n", "    func_800325E0(a0, &D_80101EC8[1].unk_F4.x);\n")
    else:
        s = rep(s, "    func_800325E0(a0, (s32)((u8 *)s0 + 0x536));\n",
                "    /* FAKE: player 1's unk_F4 reached from s0 (player 0's unk_0A) so the call reuses s0's\n"
                "       register; &D_80101EC8[1].unk_F4.x rematerialises the address (score %s). */\n"
                "    func_800325E0(a0, (s32 *)((u8 *)s0 + 0x536));\n" % SC.get("c820", "?"))
    s = rep(s, "    D_800A36B4 = (s32)s2;\n", "    D_800A36B4 = s2;\n", 2)
    if "sc_c" in OPT:
        s = fn(s, "func_8002304C", lambda b: b.replace("*((s16 *) (((u8 *) scratch) + 0x10))", "scratch_c[0]")
               .replace("*((s16 *) (((u8 *) scratch) + 0x12))", "scratch_c[1]")
               .replace("*((s16 *) (((u8 *) scratch) + 0x14))", "scratch_c[2]"))
    if "bcf0_abl" in OPT:
        s = rep(s, "    D_800F6608.unk_00 = *(Vec3i32 *)&arg0->unk_B8;\n",
                "    D_800F6608.unk_00.x = arg0->unk_B8.vx;\n    D_800F6608.unk_00.y = arg0->unk_B8.vy;\n"
                "    D_800F6608.unk_00.z = arg0->unk_B8.vz;\n")
    return s


def pack_9f9c(s):
    s = rep(s, "void func_80020DDC(void) {    s32 v0;    s32 v1;    s32 v2;    v0 = func_80036EA8(1, 1);    cdrom_StartRead(v0, D_800A3830);"
               "    game_FrameLoop();    v1 = D_800A3830;    D_80102760 = v1 + 0x14;    D_80102764 = v1 + *(s32 *)(v1 + 4);"
               "    D_80102768 = v1 + *(s32 *)(v1 + 8);    v2 = *(s32 *)(v1 + 0x10);    D_800A3880 = 1;    D_80102770 = v1 + v2;}\n",
            """void func_80020DDC(void) {
    s32 v0;
    s32 v1;
    s32 v2;
    v0 = func_80036EA8(1, 1);
    cdrom_StartRead(v0, D_800A3830);
    game_FrameLoop();
    v1 = D_800A3830;
    D_80102760.unk_00 = (u16 *)(v1 + 0x14);
    D_80102760.unk_04 = (u16 *)(v1 + *(s32 *)(v1 + 4));
    D_80102760.unk_08 = (u8 *)(v1 + *(s32 *)(v1 + 8));
    v2 = *(s32 *)(v1 + 0x10);
    D_800A3880 = 1;
    D_80102760.unk_10 = (u32 *)(v1 + v2);
}
""")
    s = rep(s, """            D_801027B0[i][0] = (s32)D_800A3860[i] + 0x6C + (D_800A3860[i]->unk_03 - 1) * 6;
            D_801027B0[i][1] = (s32)D_800A3860[i] + D_800A3860[i]->unk_04[0];
            D_801027B0[i][2] = (s32)D_800A3860[i] + D_800A3860[i]->unk_04[1];
            D_801027B0[i][3] = (s32)D_800A3860[i] + D_800A3860[i]->unk_04[2];
            D_801027B0[i][4] = (s32)D_800A3860[i] + D_800A3860[i]->unk_04[3];
""", """            D_801027B0[i].unk_00 = (u16 *)((s32)D_800A3860[i] + 0x6C + (D_800A3860[i]->unk_03 - 1) * 6);
            D_801027B0[i].unk_04 = (u16 *)((s32)D_800A3860[i] + D_800A3860[i]->unk_04[0]);
            D_801027B0[i].unk_08 = (u8 *)((s32)D_800A3860[i] + D_800A3860[i]->unk_04[1]);
            D_801027B0[i].unk_0C = (u16 *)((s32)D_800A3860[i] + D_800A3860[i]->unk_04[2]);
            D_801027B0[i].unk_10 = (u32 *)((s32)D_800A3860[i] + D_800A3860[i]->unk_04[3]);
""")
    s = rep(s, "    func_8001979C(0, D_80102770);\n    if (D_800A38C4[0]) {\n        func_8001979C(1, D_801027B0[0][4]);\n"
               "    }\n    if (D_800A38C4[1]) {\n        func_8001979C(2, D_801027B0[1][4]);\n",
            "    func_8001979C(0, D_80102760.unk_10);\n    if (D_800A38C4[0]) {\n        func_8001979C(1, D_801027B0[0].unk_10);\n"
            "    }\n    if (D_800A38C4[1]) {\n        func_8001979C(2, D_801027B0[1].unk_10);\n")
    s = re.sub(r"return \(void \*\)\(D_801027B0\[([^]]+)\]\[0\]\n( +)\+ ([^;]+) \* 2\);",
               r"return &D_801027B0[\1].unk_00[\3];", s)
    s = rep(s, "        return (void *)(D_801027B0[ch][0] + (id & 0x7FFF) * 2);\n", "        return &D_801027B0[ch].unk_00[id & 0x7FFF];\n")
    s = rep(s, "    return (void *)(D_80102760 + id * 2);\n", "    return &D_80102760.unk_00[id];\n")
    s = rep(s, "    return D_801027B0[v1][0] + D_800A3860[v1]->f4E[v0] * 2;\n",
            "    return (s32)&D_801027B0[v1].unk_00[D_800A3860[v1]->f4E[v0]];\n", 2)
    s = rep(s, "    return D_80102760 + D_800A3860[D_80101EC8[a0].unk_4A]->f16 * 2;\n",
            "    return (s32)&D_80102760.unk_00[D_800A3860[D_80101EC8[a0].unk_4A]->f16];\n")
    s = rep(s, "    return D_80102760 + D_800A3860[D_80101EC8[a0].unk_4A]->f18[a1] * 2;\n",
            "    return (s32)&D_80102760.unk_00[D_800A3860[D_80101EC8[a0].unk_4A]->f18[a1]];\n")
    s = rep(s, """            u16 *v0 = (u16 *)(D_80102764 + (v1 * 4));
            s0->unk_54 = v0;
            s0->unk_58 = (u8 *)(D_80102768 + v0[1]);
        } else {
            u16 *v0 = (u16 *)(D_801027B0[a3][1] + (v1 * 4));
            s0->unk_54 = v0;
            s0->unk_58 = (u8 *)(D_801027B0[a3][2] + v0[1]);
""", """            u16 *v0 = &D_80102760.unk_04[v1 * 2];
            s0->unk_54 = v0;
            s0->unk_58 = D_80102760.unk_08 + v0[1];
        } else {
            u16 *v0 = &D_801027B0[a3].unk_04[v1 * 2];
            s0->unk_54 = v0;
            s0->unk_58 = D_801027B0[a3].unk_08 + v0[1];
""")
    s = rep(s, "                s32 val1 = D_801027B0[idx1][3];\n", "                u16 *val1 = D_801027B0[idx1].unk_0C;\n")
    s = rep(s, "                    func_80055138(i, (u16 *)val1, (u16 *)D_801027B0[idx2][3]);\n",
            "                    func_80055138(i, val1, D_801027B0[idx2].unk_0C);\n")
    if "D_801027B0[" in s and re.search(r"D_801027B0\[[^]]*\]\[", s):
        raise SystemExit("9F9C: D_801027B0 2-D use left")
    return s


def pack_28708(s):
    s = rep(s, """            entry = D_80102764 + p->w0->unk_04 * 4;
            rob->unk_58 = (u8 *)(D_80102768 + *(u16 *)(entry + 2));
""", """            entry = &D_80102760.unk_04[p->w0->unk_04 * 2];
            rob->unk_58 = D_80102760.unk_08 + entry[1];
""")
    s = rep(s, """            entry = D_801027B0[temp][1] + p->w0->unk_04 * 4;
            rob->unk_58 = (u8 *)(D_801027B0[temp][2] + *(u16 *)(entry + 2));
""", """            entry = &D_801027B0[temp].unk_04[p->w0->unk_04 * 2];
            rob->unk_58 = D_801027B0[temp].unk_08 + entry[1];
""")
    s = rep(s, "        s32 entry;\n", "        u16 *entry;\n")
    return s


def pack_2b344(s):
    return rep(s, "        func_8001979C(0, (u32 *)D_80102770);\n        func_8001979C(1, (u32 *)D_801027B0[0][4]);\n"
                  "        func_8001979C(2, (u32 *)D_801027B0[1][4]);\n",
               "        func_8001979C(0, D_80102760.unk_10);\n        func_8001979C(1, D_801027B0[0].unk_10);\n"
               "        func_8001979C(2, D_801027B0[1].unk_10);\n")


def mot_init(s):
    s = rep(s, "    u32 base;\n", "    Unk800F1B18Rec *base;\n")
    s = rep(s, "    u32 dst;\n    u32 dst2;\n    u32 out;\n", "    u16 *dst;\n    u16 *dst2;\n    Unk800F1B18Slot *out;\n")
    s = rep(s, "    base = (u32)&D_800F1B18[arg0 * 0x570];\n\n    *(u32 **)base = arg1;\n",
            "    base = &D_800F1B18[arg0];\n\n    base->stream = arg1;\n")
    s = rep(s, "        dst = base + i * 2;\n", "        dst = &base->pose[i];\n")
    s = rep(s, "        dst2 = base + i * 2;\n", "        dst2 = &base->rate[i];\n")
    s = rep(s, "            *(s16 *)(dst + 0xA) = (s16)hi;\n", "            dst[3] = hi;\n")
    s = rep(s, "            *(s16 *)(dst + 0xA) = (s16)(cur >> 20);\n", "            dst[3] = cur >> 20;\n")
    s = rep(s, "            *(s16 *)(dst2 + 0x8E) = (s16)hi;\n", "            dst2[3] = hi;\n")
    s = rep(s, "            *(s16 *)(dst2 + 0x8E) = (s16)(cur >> 30);\n", "            dst2[3] = cur >> 30;\n")
    s = rep(s, "    out = base + 0x348;\n    do {\n        *(s32 *)(out + 0x110) = neg2;\n        i--;\n        out -= 0x118;\n",
            "    out = &base->slot[3];\n    do {\n        out->frame = neg2;\n        i--;\n        out--;\n")
    s = rep(s, "    *(s32 *)(base + 0x10C) = val;\n", "    base->ctr = val;\n")
    if "mi_dst" not in OPT:      # no dst holder: base->pose[i + 3]
        s = rep(s, "    u16 *dst;\n    u16 *dst2;\n", "")
        s = rep(s, "        dst = &base->pose[i];\n", "")
        s = rep(s, "        dst2 = &base->rate[i];\n", "")
        s = s.replace("dst[3] = ", "base->pose[i + 3] = ").replace("dst2[3] = ", "base->rate[i + 3] = ")
    if "mi_out" not in OPT:      # the slot loop indexes base->slot[i]
        s = rep(s, "    Unk800F1B18Slot *out;\n", "")
        s = rep(s, "    out = &base->slot[3];\n    do {\n        out->frame = neg2;\n        i--;\n        out--;\n",
                "    do {\n        base->slot[i].frame = neg2;\n        i--;\n")
    if "mi_dst6" in OPT:
        s = rep(s, "        dst = &base->pose[i];\n", "        dst = &base->pose[i + 3];\n")
        s = rep(s, "        dst2 = &base->rate[i];\n", "        dst2 = &base->rate[i + 3];\n")
        s = s.replace("dst[3] = ", "*dst = ").replace("dst2[3] = ", "*dst2 = ")
    return s


def mot_decode(s):
    s = rep(s, "    u8 *rec;\n    u8 *tbl;\n    u8 *slot;\n    u8 *prev;\n",
            "    Unk800F1B18Rec *rec;\n    u8 *tbl;\n    Unk800F1B18Slot *slot;\n    Unk800F1B18Slot *prev;\n")
    s = rep(s, "    rec = &D_800F1B18[obj * 0x570];\n    tbl = (u8 *)*(u32 **)rec + 0x70;\n",
            "    rec = &D_800F1B18[obj];\n    tbl = (u8 *)rec->stream + 0x70;\n")
    s = rep(s, "        COPY33(work + 0x84, rec + 0x88);\n", "        COPY33(work + 0x84, rec->rate);\n")
    s = rep(s, "    ctr = *(s32 *)(rec + 0x10C);\n    *(s32 *)(rec + 0x10C) = ctr + 1;\n"
               "    slot = rec + (((ctr + 1) & 3) * 0x118 + 0x110);\n    if (*(s32 *)slot == frame) {\n"
               "        COPY33(out, slot + 4);\n        *(s32 *)(rec + 0x10C) -= 1;\n",
            "    ctr = rec->ctr;\n    rec->ctr = ctr + 1;\n    slot = &rec->slot[(ctr + 1) & 3];\n"
            "    if (slot->frame == frame) {\n        COPY33(out, slot->pose);\n        rec->ctr -= 1;\n")
    s = rep(s, "    prev = rec + ((ctr & 3) * 0x118 + 0x110);\n    if (*(s32 *)prev == frame) {\n"
               "        COPY33(out, prev + 4);\n        *(s32 *)(rec + 0x10C) -= 1;\n",
            "    prev = &rec->slot[ctr & 3];\n    if (prev->frame == frame) {\n        COPY33(out, prev->pose);\n"
            "        rec->ctr -= 1;\n")
    s = rep(s, "    if (*(s32 *)slot != frame - 1) {\n        if (*(s32 *)prev == frame - 1) {\n",
            "    if (slot->frame != frame - 1) {\n        if (prev->frame == frame - 1) {\n")
    s = rep(s, "            ptr = *(u32 **)rec + (off >> 5);\n", "            ptr = rec->stream + (off >> 5);\n")
    s = rep(s, "            COPY33(work, rec + 4);\n", "            COPY33(work, rec->pose);\n")
    s = rep(s, "    ptr = *(u32 **)(slot + 0x10C);\n    bits = *(s32 *)(slot + 0x110);\n    cur = *(u32 *)(slot + 0x114);\n"
               "    COPY33(work, slot + 4);\n    if (sub >= 2) {\n        COPY33(work + 0x42, slot + 0x88);\n",
            "    ptr = slot->ptr;\n    bits = slot->bits;\n    cur = slot->cur;\n"
            "    COPY33(work, slot->pose);\n    if (sub >= 2) {\n        COPY33(work + 0x42, slot->rate);\n")
    s = rep(s, "        *(s32 *)slot = frame;\n        COPY33(slot + 4, work);\n        COPY33(slot + 0x88, work + 0x42);\n"
               "        *(u32 **)(slot + 0x10C) = ptr;\n        *(s32 *)(slot + 0x110) = bits;\n        *(u32 *)(slot + 0x114) = cur;\n",
            "        slot->frame = frame;\n        COPY33(slot->pose, work);\n        COPY33(slot->rate, work + 0x42);\n"
            "        slot->ptr = ptr;\n        slot->bits = bits;\n        slot->cur = cur;\n")
    return s


FILES = [("include/game.h", game_h), ("include/bb2.h", bb2_h), ("src/main/9F9C.c", f9f9c),
         ("src/main/17AFC.c", f17afc), ("src/main/28708.c", f28708), ("src/main/2B344.c", pack_2b344),
         ("src/main/3AB48.c", lambda s: rep(s, "        func_800418D0((s32 *)&D_80101DF0);\n", "        func_800418D0(&D_80101DF0);\n")),
         ("src/main/87A0.c", lambda s: rep(s, "    func_8003A728((s32)&pad);\n", "    func_8003A728(&pad);\n")),
         ("src/main/31D3C.c", f31d3c), ("src/main/368E4.c", f368e4), ("src/main/35000.c", f35000),
         ("named_syms.txt", named_syms), ("undefined_syms_auto.txt", undef_syms),
         (GPUH, libgpu_h), ("src/main/psxsdk/libgpu/prim.c", prim),
         (API + "counter.c", counter), (API + "pad.c", pad),
         (CD + "libcd_internal.h", cd_internal), (CD + "bios.c", cd_bios), (CD + "sys.c", cd_sys),
         (SPU + "libspu_internal.h", spu_internal), (SPU + "s_m_f.c", s_m_f), (SPU + "s_m_int.c", s_m_int),
         (SPU + "s_m_m.c", s_m_m_kb), (SPU + "s_m_util.c", s_m_util), (SPU + "s_m_init.c", s_m_init),
         (SND + "libsnd_i.h", snd_i), (SND + "ut_rdep.c", ut_rdep), (SND + "ut_rev.c", ut_rev),
         (SND + "vm_init.c", vm_init), (SND + "760D0.c", snd_760d0), (SND + "vm_vsu.c", vm_vsu),
         (SND + "vs_vh.c", vs_vh), (SND + "vm_aloc2.c", vm_aloc2)]


# worker 1's lane (team-lead, 2026-10-06): pad.c and libsnd 760D0.c are worker 1's; their versions stand.
FILES.append(("src/main/32D04.c", lambda s: s))   # rev-w2b5: its stale D_800963EE extern goes
W1_FILES = {API + "pad.c", SND + "760D0.c"}
FILES = [(p, g) for p, g in FILES if p not in W1_FILES]


# ------------------------------------------------------------------ the B4 sweep of the moved bodies
# Holders / aliases that ablate to IDENTICAL are dropped (abl_w2b5.py's transforms, applied in this
# order); the constructs that score get FAKE labels; carried FAKE labels get their measured score.
sys.path.insert(0, HERE)
import abl_w2b5 as ABL
SC.update({"bcf0": "27", "clr": "4", "c820": "11", "kb": "1", "a3": "21", "f40": "2", "a728": "22"})
DROP = ["064_v1", "314_a0", "314_v0", "bcf0_sum", "bf4_chain", "bf4_rpap", "568_packets",
        "568_mode", "e404_v3", "e404_p20", "e6e4_p20", "f34_val1", "f34_idx", "f34_mode", "80c_off", "ia_list",
        "ia2_list", "gi_err", "vsu_sotn", "ddc_v0"]
LABELS = [   # (file, anchor line, label) -- the label goes on its own line(s) above the anchor
    ("src/main/17AFC.c", "    s32 speed = 0x50;\n",
     "constant holders for the speed (0x50) and the initial y velocity (-0xC8); as literals the li pair "
     "reorders (speed: score 12; vel_y: score 7)"),
    ("src/main/17AFC.c", "        s32 a0_arg = src->unk_B2;\n",
     "unk_B2 read into a0_arg before cmd is chosen; read at the call the argument registers rotate (score 20)"),
    ("src/main/9F9C.c", "    s32 diff = 0x1000 - arg1;\n",
     "diff computed at entry (li 4096; subu s1 before the func_8003F1E4 call); at its use the saved "
     "registers swap (score 17)"),
    ("src/main/9F9C.c", "        s32 div4 = arg1 / 4;\n",
     "div4 taken first; at its use the multiply chain is rebuilt after the shift (score 17)"),
    ("src/main/9F9C.c", "        u16 lhu_val = arg0->unk_1C8.vy;\n",
     "unk_1C8.vy read as u16 (lhu) ahead of the w18 store; at its use it sinks below the shift (score 10)"),
    ("src/main/9F9C.c", "        val = 0xB00 - div4;\n",
     "val holds 0xB00 - div4 ahead of the h14 store; at the h12 store the subu moves below it (score 2)"),
    ("src/main/368E4.c", "        u16 cnt = D_800A38D6;\n",
     "cnt and old_ptr read D_800A38D6, then g_gpu_ot_ptr, ahead of the stores; read at their uses the "
     "text is identical (sandbox 0) but the object's undefined symbols reorder (CMP)"),
    ("src/main/368E4.c", "        count1 = cnt + 1;\n",
     "count1 holds the count + 1 ahead of the D_800A3808 store; at the D_800A38D6 store the loads "
     "reorder (score 12)"),
    ("src/main/87A0.c", "        u8 *rec = (u8 *)pkts + i * 8;\n",
     "rec holds the packet's address for the loop body; indexing the packet bytes at each read adds a "
     "second induction (score 18)"),
    ("src/main/psxsdk/libsnd/vm_aloc2.c", "    if ((_svm_cur.tone_vag_idx & 1) > 0) {\n",
     "progIdx computed in both arms (duplicated-statement-into-arms); hoisted above the test the lhu "
     "becomes lh and the frame grows (score 19)"),
    ("src/main/psxsdk/libsnd/vm_init.c", "        u16 masked = (u8)a0;\n",
     "masked holds (u8)a0 as a u16; read from a0 twice the andi lands in v0 and the byte store takes s1 "
     "(score 3)"),
    ("src/main/psxsdk/libsnd/vs_vh.c", "        var_a2 = _svm_vab_used;\n",
     "var_a2 is the _svm_vab_used base here and the header cursor below (one local, two roles); indexing "
     "_svm_vab_used directly swaps a2 / a3 through the body (score 14)"),
    ("src/main/9F9C.c", "    v2 = *(s32 *)(v1 + 0x10);\n",
     "v2 reads the +0x10 header word before the D_800A3880 store; inline it moves below (score 8)"),
]
CARRIED = [   # (file, comment anchor, score) -- the measured score is appended to the comment
    ("src/main/17AFC.c", "/* FAKE: named intermediate (no-new-park-categories.md, named-intermediate entry)", 13),
    ("src/main/17AFC.c", "/* FAKE: single-level do-while(0) wrap (body executes once)", 15),
    ("src/main/28708.c", "/* FAKE: one counter 'j' serves both", 16),
    ("src/main/28708.c", "/* FAKE: constant-holder local; mechanism", 3),
    ("src/main/28708.c", "FAKE: the u16 low-half loads in the tail reuse the buf8 local", 33),
    ("src/main/2B344.c", "/* FAKE: unwritten leading pad ([[dead-vars-local-array]] re-scoped carve-out, owner rulings", 17),
    ("src/main/2B344.c", "/* FAKE: unwritten TRAILING pad", 8),
    ("src/main/2B344.c", "/* FAKE: indexes past D_800A37D2 into D_800A37D3", 16),
    ("src/main/87A0.c", "/* FAKE: the `pad.valid[i] = valid;` store is written into BOTH arms", 21),
    ("src/main/9F9C.c", "/* FAKE: nd names the width subtraction", 2),
    ("src/main/9F9C.c", "/* FAKE: hi carries its own shift amount", 16),
    ("src/main/9F9C.c", "/* FAKE: new bits_left routed through val", 2),
    ("src/main/9F9C.c", "/* FAKE: nd - same construct as loop 1", 2),
    ("src/main/9F9C.c", "/* FAKE: named constant holder for the fill value", 2),
    ("src/main/9F9C.c", "/* FAKE: tail reuse of val", 4),
    ("src/main/9F9C.c", "u16 loads[130]; /* FAKE: frame layout", 20),
    ("src/main/9F9C.c", "s32 j; /* FAKE: one local for loop 1's character", 3),
    ("src/main/psxsdk/libapi/pad.c", "ret = 1; /* FAKE: two-set else arm", 3),
    ("src/main/psxsdk/libcd/bios.c", "/* FAKE: volatile locals admitted on SOTN precedent", 68),
    ("src/main/psxsdk/libgpu/prim.c", "/* FAKE: `a3 & MASK` duplicated into both arms", 5),
]
PADS = [("src/main/9F9C.c", "func_8001E404", 23), ("src/main/9F9C.c", "func_8001E6E4", 21)]


def add_score(s, anchor, n, start=0):
    i = s.index(anchor, start)
    j = s.index("*/", i)
    return s[:j].rstrip() + " Ablated (2026-10-06): score %d. " % n + s[j:]


def sweep(p, s):
    byname = {a[0]: a for a in ABL.A}
    for d in DROP:
        name, fname, path, t = byname[d]
        if path == p:
            s = fn(s, fname, t)
    if p == "src/main/87A0.c":   # 568_packets: the packet bytes read straight from pkts
        s = rep(s, "        u8 *rec = &((u8 *)&pkts[0])[i * 8];\n", "        u8 *rec = (u8 *)pkts + i * 8;\n")
    if p == "src/main/28708.c":   # 80c_off: the checksum walk starts at record i's bytes
        s = rep(s, "        bp = (u8 *)arg0 + i * 0x24;\n", "        bp = (u8 *)&arg0->rec[i];\n")
    if p == "src/main/368E4.c":   # the m2c name new_var2 (the count + 1) becomes count1
        s = fn(s, "func_80046BF4", lambda b: b.replace("new_var2", "count1"))
    for f, anchor, text in LABELS:
        if f == p:
            ind = anchor[:len(anchor) - len(anchor.lstrip(" "))]
            s = rep(s, anchor, "%s/* FAKE: %s. */\n%s" % (ind, text, anchor))
    for f, anchor, n in CARRIED:
        if f == p:
            if s.count(anchor) != 1:
                raise SystemExit("carried anchor %r: %d" % (anchor, s.count(anchor)))
            s = add_score(s, anchor, n)
    for f, fname, n in PADS:
        if f == p:
            i = span(s, fname)[0]
            s = s[:i] + add_score(s[i:], "/* FAKE: frame layout -- unwritten leading pad", n)
    if p == "src/main/psxsdk/libapi/pad.c":
        s = rep(s, "a scalar counter gives 8 (score 2).", "a scalar counter gives 8 (score 2). Ablated (2026-10-06): "
                   "a non-volatile i[1] scores 23, a volatile i[1] 2.")
    if p == "src/main/psxsdk/libgpu/prim.c":
        s = rep(s, "    /* FAKE: packed RECT word read follows matched Sony-library precedent (as SetDrawMove). */\n"
                   "    p->code[1] = *(u32 *)&rect->x;\n",
                "    /* FAKE: packed RECT word read follows matched Sony-library precedent (as SetDrawMove; psyz\n"
                "       prim.c SetDrawLoad reads the same words); (y << 16) | (u16)x scores 12. */\n"
                "    p->code[1] = *(u32 *)&rect->x;\n")
        s = rep(s, "    /* FAKE: packed RECT word read follows matched Sony-library precedent (as SetDrawMove). */\n"
                   "    p->code[2] = *(u32 *)&rect->w;\n",
                "    /* FAKE: packed RECT word read follows matched Sony-library precedent (as SetDrawMove; psyz\n"
                "       prim.c SetDrawLoad reads the same words); (h << 16) | (u16)w scores 10. */\n"
                "    p->code[2] = *(u32 *)&rect->w;\n")
    return s


# rev-w2b5 fixes (FAIL, measured patch tmp/w2b5_fix.patch) + the ruling on func_80046BF4's cnt / old_ptr
# + DR_TPAGE now comes from main (fclose1). Applied after sweep().
def fix5(p, s):
    if p == "include/game.h":
        s = rep(s, " * keyframe offsets at +0x70); pose / rate the initial channel values func_8001979C unpacks;",
                " * keyframe offsets at +0x70); pose / code the initial channel values and channel codes func_8001979C unpacks;")
        s = rep(s, "    u16 rate[0x42];           /* +0x088 */\n", "    u16 code[0x42];           /* +0x088 channel codes (2 bits each) */\n")
        s = rep(s, " * from player unk_03's unk_F4 to unk_04.", " * from the other player's unk_F4 to unk_04 (unk_03 is the owner).")
    if p == "src/main/17AFC.c":
        s = fn(s, "func_80032040", lambda b: rep(b, "    for (i = 0x84; i >= 0; i -= 0x2C) {\n"
            "        /* FAKE: byte-offset walk over the records' unk_00; D_80104E88[i].unk_00 for\n"
            "           i = 3..0 keeps a separate counter (score 4). */\n        ((u8 *)D_80104E88)[i] = 0;\n",
            "    for (i = 0; i < 4; i++) {\n        D_80104E88[i].unk_00 = 0;\n"))

        def f314(b):
            b = rep(b, "                u32 v0_m = (u32)-2;\n", "")
            b = rep(b, "                v0_m &= clz;\n                v1_m = 0x16 - v0_m;\n", "                v1_m = 0x16 - (clz & (u32)-2);\n")
            return rep(b, "            s32 v0 = *a3 * 30 + 0x1F4;\n            if (log2_val < (u32)v0) {\n",
                       "            if (log2_val < (u32)(*a3 * 30 + 0x1F4)) {\n")
        s = fn(s, "func_80032314", f314)

        def f5e0(b):
            b = rep(b, "            u32 clz;\n", "")
            b = rep(b, "            clz = sp_tmp;\n", "")
            b = rep(b, "                u32 v0_m = (u32)-2;\n",
                    "                /* FAKE: v0_m holds the -2 mask (li -2 into v0, and v0,v1,v0); as a literal the mask\n"
                    "                   lands in v1 and the clz in v0 (score 3). */\n                u32 v0_m = (u32)-2;\n")
            return rep(b, "                v0_m &= clz;\n", "                v0_m &= sp_tmp;\n")
        s = fn(s, "func_800325E0", f5e0)
    if p == "src/main/9F9C.c":
        s = rep(s, "            base->rate[i + 3] = hi;\n", "            base->code[i + 3] = hi;\n")
        s = rep(s, "            base->rate[i + 3] = cur >> 30;\n", "            base->code[i + 3] = cur >> 30;\n")
        s = rep(s, "        COPY33(work + 0x84, rec->rate);\n", "        COPY33(work + 0x84, rec->code);\n")
    if p == "src/main/31D3C.c":
        s = fn(s, "func_800418D0", lambda b: rep(rep(b, "    AnimRotFunc func;\n", ""),
            "    func = g_anim_func_table[a0->unk8];\n    func(&sp10, &a0->work);\n", "    g_anim_func_table[a0->unk8](&sp10, &a0->work);\n"))
    if p == "src/main/35000.c":
        s = rep(s, "s32 func_80045080(s32 a0) {\n",
                "s32 func_80045080(s32 a0) {\n    /* FAKE: val reads the length ahead of the func_800457DC call; at its use the lh moves after\n"
                "       the call and a0 * 4 is kept in s0 instead (score 9). */\n")
    if p == "src/main/368E4.c":   # ruling: cnt / old_ptr dropped (the CMP difference is a relocation symbol-index swap)
        s = rep(s, "        /* FAKE: cnt and old_ptr read D_800A38D6, then g_gpu_ot_ptr, ahead of the stores; read at their uses the "
                   "text is identical (sandbox 0) but the object's undefined symbols reorder (CMP). */\n"
                   "        u16 cnt = D_800A38D6;\n        s32 old_ptr = (s32)g_gpu_ot_ptr;\n", "")
        s = rep(s, "        count1 = cnt + 1;\n        D_800A3808 = old_ptr;\n",
                "        count1 = D_800A38D6 + 1;\n        D_800A3808 = (s32)g_gpu_ot_ptr;\n")
        s = rep(s, "        D_800A378C = (u32 *)(old_ptr + 0x10);\n", "        D_800A378C = (u32 *)((s32)g_gpu_ot_ptr + 0x10);\n")
    if p == "src/main/32D04.c":
        s = rep(s, "extern s16 D_800963EE;\n", "")
    if p == CD + "bios.c":
        s = rep(s, """/* Sony's `int CD_status` (the drive status byte the BIOS layer stores whole; lw / sw here). The
   command layer (sys.c) declares it u_char and reads its low byte (lbu), so each TU carries its
   own declaration, as Sony's two modules do. */
extern s32 CD_status;
""", "")
    if p == CD + "libcd_internal.h":
        s = rep(s, "#include <psxsdk/libcd.h>\n\n", "#include <psxsdk/libcd.h>\n\nextern s32 CD_status; /* Sony's int CD_status (bios.c) */\n")
    if p == CD + "sys.c":
        s = rep(s, """/* bios.c's `int CD_status`, read here as Sony's command layer declares it: u_char, the low byte
   (lbu in CdStatus, CdControl, CdControlF, CdControlB); bios.c stores the whole word. */
extern u8 CD_status;
""", "")
        s = rep(s, "u32 CdStatus(void) {\n    return CD_status;\n", "inline u32 CdStatus(void) {\n    return (u8)CD_status;\n")
        s = rep(s, "        if (CD_status & 0x10) {\n", "        if (CdStatus() & 0x10) {\n", 3)
    if p == "src/main/psxsdk/libgpu/prim.c":
        s = rep(s, "    p->code[size - 1] = OT_TERMINATOR;\n", "    p->p[size - 4] = OT_TERMINATOR;\n")
    if p == SND + "ut_rev.c":
        def rev(b):
            b = rep(b, "    s32 s0;\n    if ((s32)(a0 << 16) < 0) {\n", "    if (a0 < 0) {\n")
            b = rep(b, "            _svm_rattr.mode = (s16)((v1 | 0x100) << 16 >> 16);\n", "            _svm_rattr.mode = v1 | 0x100;\n")
            b = rep(b, "            _svm_rattr.mode = (s16)(v1 << 16 >> 16);\n", "            _svm_rattr.mode = v1;\n")
            b = rep(b, "        s0 = (s16)(v1 << 16 >> 16);\n        if (s0 == 0) {\n", "        if (v1 == 0) {\n")
            return rep(b, "        return s0;\n", "        return v1;\n")
        s = fn(s, "SsUtSetReverbType", rev)
    if p == GPUH:   # fclose1 brought PsyQ's DR_TPAGE (with its comment); batch 5 keeps DR_LOAD only
        s = rep(s, "/* PsyQ DR_LOAD: tag, three command words, up to 13 data words; DR_TPAGE: tag, one draw-mode word. */\n"
                   "typedef struct { u32 tag; u32 code[3]; u32 p[13]; } DR_LOAD;\ntypedef struct { u32 tag; u32 code[1]; } DR_TPAGE;\n",
                "/* PsyQ DR_LOAD: tag, three command words, up to 13 data words. */\n"
                "typedef struct { u32 tag; u32 code[3]; u32 p[13]; } DR_LOAD;\n")
    return s


def main():
    apply = "apply" in sys.argv[1:]
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or ["tmp/w2/b5"])[0]
    for p, g in FILES:
        s = fix5(p, sweep(p, g(base(p))))
        dst = p if apply else out + "/" + p
        if os.path.dirname(dst):
            os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "w", encoding="utf-8", newline=NL).write(s)
    print("w2b5 wrote %d files to %s %s" % (len(FILES), "the tree" if apply else out, sorted(OPT)))


if __name__ == "__main__":
    main()
