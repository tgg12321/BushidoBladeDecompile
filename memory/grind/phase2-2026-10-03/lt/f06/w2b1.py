#!/usr/bin/env python3
# Worker-2 batch 1: F06 (libspu's SPU register block _spu_RXX + _spu_setReverbAttr's argument) and
# F12 (libgpu internals, sys.c / ext.c), on top of main 110ccc84e.
# - libspu: _spu_RXX becomes PsyQ's `union SpuUnion *` (SPU_RXX register block / raw u16 view,
#   the layout SOTN and psyz spell for the same Sony library); every SPU register access reads a
#   member. RevParamEntry moves to libspu_internal.h; _spu_setReverbAttr takes it.
# - libgpu: the device table, DR_ENV / DRAWENV / DISPENV / DR_PRIO / P_TAG views by member.
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/f06/w2b1.py [opt=<name>,...]
#   writes the batch's files into the working tree, rebuilt from BASE (git show).
import os, re, subprocess, sys

BASE = "72802430d"
NL = chr(10)
# The landed spellings; `opt=` replaces the whole set (ablations / alternatives).
OPT = {"gaks_raw", "gvex_raw", "gks_raw", "init_ptr", "rev_sotn", "de_c", "bcr_vol", "kon_plain", "snc_one"}
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT = set(a[4:].split(",")) - {""}


MAIN_GIT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/.git"   # WSL in a Windows worktree


def show(p):
    for pre in (["git"], ["git", "--git-dir=" + MAIN_GIT]):
        r = subprocess.run(pre + ["show", "%s:%s" % (BASE, p)], capture_output=True, text=True,
                           encoding="utf-8")
        if r.returncode == 0:
            return r.stdout
    raise SystemExit("git show %s:%s failed" % (BASE, p))


def rep(s, old, new, n=1):
    c = s.count(old)
    if c != n:
        raise SystemExit("expected %d x %r, found %d" % (n, old[:70], c))
    return s.replace(old, new)


def fn(s, name, g):
    m = re.search(r"\n[A-Za-z0-9_]+[ *]+%s\([^;{]*\)\s*\{" % name, s)
    i = m.start() + 1
    j = s.index("\n}\n", i) + 3
    return s[:i] + g(s[i:j]) + s[j:]


# ------------------------------------------------------------------ libspu
SPU = "src/main/psxsdk/libspu/"

# byte offset in the register block -> SPU_RXX member
RXX = {0x180: "main_vol.left", 0x182: "main_vol.right", 0x184: "rev_vol.left",
       0x186: "rev_vol.right", 0x188: "key_on[0]", 0x18A: "key_on[1]", 0x18C: "key_off[0]",
       0x18E: "key_off[1]", 0x190: "chan_fm[0]", 0x192: "chan_fm[1]", 0x194: "noise_mode[0]",
       0x196: "noise_mode[1]", 0x198: "rev_mode[0]", 0x19A: "rev_mode[1]", 0x1A6: "trans_addr",
       0x1A8: "trans_fifo", 0x1AA: "spucnt", 0x1AC: "data_trans", 0x1AE: "spustat",
       0x1B0: "cd_vol.left", 0x1B2: "cd_vol.right", 0x1B4: "ex_vol.left", 0x1B6: "ex_vol.right"}
REV = ["dAPF1", "dAPF2", "vIIR", "vCOMB1", "vCOMB2", "vCOMB3", "vCOMB4", "vWALL", "vAPF1", "vAPF2",
       "mLSAME", "mRSAME", "mLCOMB1", "mRCOMB1", "mLCOMB2", "mRCOMB2", "dLSAME", "dRSAME", "mLDIFF",
       "mRDIFF", "mLCOMB3", "mRCOMB3", "mLCOMB4", "mRCOMB4", "dLDIFF", "dRDIFF", "mLAPF1", "mRAPF1",
       "mLAPF2", "mRAPF2", "vLIN", "vRIN"]
for i, n in enumerate(REV):
    RXX[0x1C0 + 2 * i] = n

RXX_SITE = re.compile(r"\*\((?:volatile )?u16 \*\)\((?:\(u8 \*\))?_spu_RXX \+ 0x([0-9A-F]+)\)")


def rxx_sites(s):
    return RXX_SITE.sub(lambda m: "_spu_RXX->rxx." + RXX[int(m.group(1), 16)], s)


INTERNAL_OLD = """/* The SPU register block _spu_RXX points at, as the key-on / key-off code uses it. */
typedef struct {
    u16 pad[196];
    volatile u16 key_on[2];  /* +0x188 SPU KEY-ON (MMIO via _spu_RXX) */
    volatile u16 key_off[2]; /* +0x18C SPU KEY-OFF */
} SpuRXX;
typedef union {
    SpuRXX rxx;
    volatile u16 raw[0x100];
} SpuUnion;
"""

INTERNAL_NEW = """/* PsyQ LIBSPU's SPU register block (Sony's SPU_RXX and union SpuUnion; the same spelling in
 * SOTN src/main/psxsdk/libspu/libspu_internal.h and psyz decomp/src/libspu/libspu_private.h):
 * one register set per voice, then the common registers; `raw` indexes it by halfword.
 * _spu_RXX points at the hardware block (.word 0x1F801C00, asm/data/91C98.data.s), the SPU MMIO
 * range, so the members are volatile at type level (mmio-volatile-type-level). */
typedef struct tagSpuVoiceRegister {
    /* 0x00 */ SpuVolume volume;
    /* 0x04 */ u16 pitch;
    /* 0x06 */ u16 addr;
    /* 0x08 */ u16 adsr[2];
    /* 0x0C */ u16 volumex;
    /* 0x0E */ u16 loop_addr;
} SPU_VOICE_REG;

typedef struct tagSpuControl {
    /* 0x000 */ SPU_VOICE_REG voice[24];
    /* 0x180 */ SpuVolume main_vol;
    /* 0x184 */ SpuVolume rev_vol;
    /* 0x188 */ u16 key_on[2];
    /* 0x18C */ u16 key_off[2];
    /* 0x190 */ u16 chan_fm[2];
    /* 0x194 */ u16 noise_mode[2];
    /* 0x198 */ u16 rev_mode[2];
    /* 0x19C */ u32 chan_on;
    /* 0x1A0 */ u16 unk;
    /* 0x1A2 */ u16 rev_work_addr;
    /* 0x1A4 */ u16 irq_addr;
    /* 0x1A6 */ u16 trans_addr;
    /* 0x1A8 */ u16 trans_fifo;
    /* 0x1AA */ u16 spucnt;
    /* 0x1AC */ u16 data_trans;
    /* 0x1AE */ u16 spustat;
    /* 0x1B0 */ SpuVolume cd_vol;
    /* 0x1B4 */ SpuVolume ex_vol;
    /* 0x1B8 */ SpuVolume main_volx;
    /* 0x1BC */ SpuVolume unk_vol;
    /* 0x1C0 */ u16 dAPF1;
    /* 0x1C2 */ u16 dAPF2;
    /* 0x1C4 */ u16 vIIR;
    /* 0x1C6 */ u16 vCOMB1;
    /* 0x1C8 */ u16 vCOMB2;
    /* 0x1CA */ u16 vCOMB3;
    /* 0x1CC */ u16 vCOMB4;
    /* 0x1CE */ u16 vWALL;
    /* 0x1D0 */ u16 vAPF1;
    /* 0x1D2 */ u16 vAPF2;
    /* 0x1D4 */ u16 mLSAME;
    /* 0x1D6 */ u16 mRSAME;
    /* 0x1D8 */ u16 mLCOMB1;
    /* 0x1DA */ u16 mRCOMB1;
    /* 0x1DC */ u16 mLCOMB2;
    /* 0x1DE */ u16 mRCOMB2;
    /* 0x1E0 */ u16 dLSAME;
    /* 0x1E2 */ u16 dRSAME;
    /* 0x1E4 */ u16 mLDIFF;
    /* 0x1E6 */ u16 mRDIFF;
    /* 0x1E8 */ u16 mLCOMB3;
    /* 0x1EA */ u16 mRCOMB3;
    /* 0x1EC */ u16 mLCOMB4;
    /* 0x1EE */ u16 mRCOMB4;
    /* 0x1F0 */ u16 dLDIFF;
    /* 0x1F2 */ u16 dRDIFF;
    /* 0x1F4 */ u16 mLAPF1;
    /* 0x1F6 */ u16 mRAPF1;
    /* 0x1F8 */ u16 mLAPF2;
    /* 0x1FA */ u16 mRAPF2;
    /* 0x1FC */ u16 vLIN;
    /* 0x1FE */ u16 vRIN;
} SPU_RXX;

typedef union SpuUnion {
    volatile SPU_RXX rxx;
    volatile u16 raw[0x100];
} SpuUnion;

/* One reverb preset (Sony rev_param_entry): a mask of the registers to set, then one value per
 * reverb register, in register order (0x1C0..0x1FE). The preset table _spu_rev_param holds ten;
 * SpuSetReverbModeParam builds one and _spu_setReverbAttr writes it to the registers. */
typedef struct {
    /* 0x00 */ u32 flags;
    /* 0x04 */ u16 dAPF1, dAPF2;
    /* 0x08 */ u16 vIIR, vCOMB1, vCOMB2, vCOMB3, vCOMB4;
    /* 0x12 */ u16 vWALL, vAPF1, vAPF2;
    /* 0x18 */ u16 mLSAME, mRSAME, mLCOMB1, mRCOMB1, mLCOMB2, mRCOMB2;
    /* 0x24 */ u16 dLSAME, dRSAME;
    /* 0x28 */ u16 mLDIFF, mRDIFF, mLCOMB3, mRCOMB3, mLCOMB4, mRCOMB4;
    /* 0x34 */ u16 dLDIFF, dRDIFF;
    /* 0x38 */ u16 mLAPF1, mRAPF1, mLAPF2, mRAPF2;
    /* 0x40 */ u16 vLIN, vRIN;
} RevParamEntry;
"""


def internal(s):
    s = rep(s, INTERNAL_OLD, INTERNAL_NEW)
    s = rep(s, "extern s32 _spu_RXX;\n", "extern SpuUnion *_spu_RXX;\n")
    s = rep(s, "extern void _spu_setReverbAttr(s32 *);\n", "extern void _spu_setReverbAttr(RevParamEntry *);\n")
    return s


SRMP_TYPE = """typedef struct {
    /* 0x00 */ u32 flags;
    /* 0x04 */ u16 dAPF1, dAPF2;
    /* 0x08 */ u16 vIIR, vCOMB1, vCOMB2, vCOMB3, vCOMB4;
    /* 0x12 */ u16 vWALL, vAPF1, vAPF2;
    /* 0x18 */ u16 mLSAME, mRSAME, mLCOMB1, mRCOMB1, mLCOMB2, mRCOMB2;
    /* 0x24 */ u16 dLSAME, dRSAME;
    /* 0x28 */ u16 mLDIFF, mRDIFF, mLCOMB3, mRCOMB3, mLCOMB4, mRCOMB4;
    /* 0x34 */ u16 dLDIFF, dRDIFF;
    /* 0x38 */ u16 mLAPF1, mRAPF1, mLAPF2, mRAPF2;
    /* 0x40 */ u16 vLIN, vRIN;
} RevParamEntry;

"""


def s_srmp(s):
    s = rep(s, SRMP_TYPE, "")
    s = rep(s, "_spu_setReverbAttr((s32 *)&entry);", "_spu_setReverbAttr(&entry);")
    return rxx_sites(s)


def s_sra(s):
    s = rep(s, "void _spu_setReverbAttr(s32 *arg0) {\n    s32 flags = arg0[0];\n",
            "void _spu_setReverbAttr(RevParamEntry *arg0) {\n    s32 flags = arg0->flags;\n")
    for i, n in enumerate(REV):
        s = rep(s, "*(u16 *)((s32)arg0 + 0x%X)" % (4 + 2 * i), "arg0->" + n)
    return rxx_sites(s)


SPU_CTRL_OLD = """/* PsyQ LIBSPU spu.c: `_spu_FwriteByIO` (static) — verbatim-linked Sony object.
   C refs: Xeeynamo/psyz decomp/src/libspu/spu.c:111 and
   sotn-decomp psxsdk/libspu/spu.c (_spu_writeByIO).

   `_spu_RXX` (0x800A2CDC) holds the SPU register-file base (0x1F801C00), so
   `_spu_RXX + 0x1A6` is the SPU transfer/control register block at 0x1F801DA6:
   transfer address, data FIFO, SPUCNT, transfer control, SPUSTAT — five
   consecutive 16-bit hardware registers.  Sony's own libspu reaches them
   through `union SpuUnion *_spu_RXX` with the SPUR()/SPUW() field macros; the
   struct below is that same register block, and every access in this function
   goes through it, exactly as the original source does. */
typedef struct {
    u16 trans_addr;  /* 0x1DA6 */
    u16 trans_fifo;  /* 0x1DA8 */
    u16 spucnt;      /* 0x1DAA */
    u16 trans_ctrl;  /* 0x1DAC */
    u16 spustat;     /* 0x1DAE */
} SpuCtrlRegs;

#define SPU_CTRL ((volatile SpuCtrlRegs *)(_spu_RXX + 0x1A6))
"""
SPU_CTRL_NEW = """/* PsyQ LIBSPU spu.c: `_spu_FwriteByIO` (static) — verbatim-linked Sony object.
   C refs: Xeeynamo/psyz decomp/src/libspu/spu.c:111 and
   sotn-decomp psxsdk/libspu/spu.c (_spu_writeByIO). */
"""


def spu(s):
    s = rep(s, SPU_CTRL_OLD, SPU_CTRL_NEW)
    s = s.replace("SPU_CTRL->", "_spu_RXX->rxx.")
    if "voice_holder" in OPT:
        s = rep(s, "        volatile u16 *vp;\n", "        volatile SPU_VOICE_REG *voices;\n")
        s = rep(s, "        vp = (volatile u16 *)_spu_RXX;\n", "        voices = _spu_RXX->rxx.voice;\n")
        pre = "voices[channel]."
    else:
        s = rep(s, "        volatile u16 *vp;\n", "")
        s = rep(s, "        vp = (volatile u16 *)_spu_RXX;\n", "")
        pre = "_spu_RXX->rxx.voice[channel]."
    for k, m in enumerate(["volume.left", "volume.right", "pitch", "addr", "adsr[0]", "adsr[1]"]):
        s = rep(s, "vp[channel * 8 + %d] =" % k, pre + m + " =")
    if "kon_plain" in OPT:
        s = rep(s, "        s32 kon;\n        s32 koff;\n", "")
        s = rep(s, "        kon = 0xFFFF;\n        koff = 0xFF;\n", "")
        s = rep(s, "_spu_RXX + 0x188) = kon;", "_spu_RXX + 0x188) = 0xFFFF;")
        s = rep(s, "_spu_RXX + 0x18A) = koff;", "_spu_RXX + 0x18A) = 0xFF;")
        s = rep(s, "_spu_RXX + 0x18C) = kon;", "_spu_RXX + 0x18C) = 0xFFFF;")
        s = rep(s, "_spu_RXX + 0x18E) = koff;", "_spu_RXX + 0x18E) = 0xFF;")
    else:
        s = rep(s, "        s32 kon;\n        s32 koff;\n", "        s32 lo;\n        s32 hi;\n")
        s = rep(s, "        kon = 0xFFFF;\n        koff = 0xFF;\n", "        lo = 0xFFFF;\n        hi = 0xFF;\n")
        s = rep(s, "_spu_RXX + 0x188) = kon;", "_spu_RXX + 0x188) = lo;")
        s = rep(s, "_spu_RXX + 0x18A) = koff;", "_spu_RXX + 0x18A) = hi;")
        s = rep(s, "_spu_RXX + 0x18C) = kon;", "_spu_RXX + 0x18C) = lo;")
        s = rep(s, "_spu_RXX + 0x18E) = koff;", "_spu_RXX + 0x18E) = hi;")
    s = rep(s, "*(volatile u16 *)(arg0 * 2 + _spu_RXX) = arg1;", "_spu_RXX->raw[arg0] = arg1;")
    s = rep(s, "*(volatile u16 *)(arg0 * 2 + _spu_RXX) = arg1 >> _spu_mem_mode_plus;",
            "_spu_RXX->raw[arg0] = arg1 >> _spu_mem_mode_plus;")
    s = rep(s, "((s16 *)_spu_RXX)[mode] = (s16)aligned;", "_spu_RXX->raw[mode] = aligned;")
    # _spu_FsetRXXa's first argument is the register index (-1 / -2 return the value instead)
    s = fn(s, "_spu_FsetRXXa", lambda b: re.sub(r"\bmode\b", "index", b))
    s = rep(s, "u16 val = ((u16 *)_spu_RXX)[index];", "u16 val = _spu_RXX->raw[index];")
    s = rep(s, "   (0x30) in SPUCNT (_spu_RXX + 0x1AA) to clear", "   (0x30) in SPUCNT to clear")
    # _spu_Fr_'s second argument is the SPU transfer address (it goes to trans_addr), not a mode
    s = rep(s, """void _spu_Fr_(s32 addr, u16 mode, s32 size) {
    *(volatile u16 *)(_spu_RXX + 0x1A6) = mode;""", """void _spu_Fr_(s32 addr, u16 spu_addr, s32 size) {
    *(volatile u16 *)(_spu_RXX + 0x1A6) = spu_addr;""")
    if "init_ptr" in OPT:
        s = rep(s, """    _spu_inTransfer = 1;
    *(volatile u16 *)(_spu_RXX + 0x1AA) = 0xC000;
""", """    _spu_inTransfer = 1;
    spucnt = &_spu_RXX->rxx.spucnt;
    *spucnt = 0xC000;
""")
        s = rep(s, """    u32 i;
    s32 channel;

    *D_800A2CEC |= 0xB0000;
""", """    u32 i;
    s32 channel;
    volatile u16 *spucnt;

    *D_800A2CEC |= 0xB0000;
""")
    return rxx_sites(s)


def s_sk(s):
    s = s.replace("((SpuRXX *)_spu_RXX)->", "_spu_RXX->rxx.")
    return s


def s_sav(s):
    return s.replace("((SpuUnion *)_spu_RXX)->", "_spu_RXX->")


def s_snc(s):
    if "snc_one" in OPT:
        return rep(s, """    {
        volatile u16 *ptr = (volatile u16 *)(_spu_RXX + 0x1AA);
        u16 tmp = *ptr;
        *ptr = (tmp & 0xC0FF) | ((val & 0x3F) << 8);
    }
""", """    _spu_RXX->rxx.spucnt = (_spu_RXX->rxx.spucnt & 0xC0FF) | ((val & 0x3F) << 8);
""")
    return rep(s, """    {
        volatile u16 *ptr = (volatile u16 *)(_spu_RXX + 0x1AA);
        u16 tmp = *ptr;
        *ptr = (tmp & 0xC0FF) | ((val & 0x3F) << 8);
    }
""", """    {
        u16 cnt = _spu_RXX->rxx.spucnt;
        _spu_RXX->rxx.spucnt = (cnt & 0xC0FF) | ((val & 0x3F) << 8);
    }
""")


GKS_OLD = """    base = bit_found << 4;
    mask = _spu_RXX;
    flags = _spu_keystat;
    base = base + mask;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = *(u16 *)(base + 0xC);
"""
GKS = {
    "gks_a": """    voices = _spu_RXX->rxx.voice;
    flags = _spu_keystat;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = voices[bit_found].volumex;
""",
    "gks_b": """    voices = _spu_RXX->rxx.voice;
    base = voices[bit_found].volumex;
    flags = _spu_keystat & (1 << bit_found);
""",
    "gks_c": """    voices = _spu_RXX->rxx.voice;
    flags = _spu_keystat;
    base = voices[bit_found].volumex;
    mask = 1 << bit_found;
    flags = flags & mask;
""",
    "gks_d": """    voices = _spu_RXX->rxx.voice;
    flags = _spu_keystat;
    voices += bit_found;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = voices->volumex;
""",
    "gks_e": """    voices = bit_found + _spu_RXX->rxx.voice;
    flags = _spu_keystat;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = voices->volumex;
""",
    "gks_f": """    flags = _spu_keystat;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = _spu_RXX->raw[bit_found * 8 + 6];
""",
    "gks_g": """    voices = &_spu_RXX->rxx.voice[bit_found];
    flags = _spu_keystat;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = voices->volumex;
""",
    "gks_i": """    base = bit_found << 3;
    flags = _spu_keystat;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = _spu_RXX->raw[base + 6];
""",
    "gks_j": """    base = bit_found;
    voices = _spu_RXX->rxx.voice;
    flags = _spu_keystat;
    voices += base;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = voices->volumex;
""",
    "gks_raw": """    base = bit_found << 4;
    mask = (s32)_spu_RXX;
    flags = _spu_keystat;
    base = base + mask;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = *(u16 *)(base + 0xC);
""",
    "gks_sep1": """    base = bit_found << 4;
    rxx = (s32)_spu_RXX;
    flags = _spu_keystat;
    base = base + rxx;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = *(u16 *)(base + 0xC);
""",
    "gks_sepall": """    off = bit_found << 4;
    rxx = (s32)_spu_RXX;
    flags = _spu_keystat;
    addr = off + rxx;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = *(u16 *)(addr + 0xC);
""",
    "gks_h": """    voices = _spu_RXX->rxx.voice;
    flags = _spu_keystat;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = voices[bit_found].volumex;
""",
}


GKS_PSYZ = """s32 SpuGetKeyStatus(s32 arg0) {
    volatile SPU_VOICE_REG *voices;
    s32 voice;
    s32 i;
    s32 voice_mask;
    u16 volumex;

    voice = -1;
    for (i = 0; i < 24; i++) {
        if (arg0 & (1 << i)) {
            voice = i;
            break;
        }
    }
    if (voice == -1) {
        return -1;
    }
    voices = _spu_RXX->rxx.voice;
    volumex = voices[voice].volumex;
    voice_mask = 1 << voice;
    if (_spu_keystat & voice_mask) {
        if (volumex > 0) {
            return 1;
        } else {
            return 3;
        }
    } else if (volumex > 0) {
        return 2;
    } else {
        return 0;
    }
}
"""


def s_gks(s):
    k = ([o for o in OPT if o.startswith("gks_")] or ["gks_a"])[0]
    if k == "gks_psyz":
        i = s.index("s32 SpuGetKeyStatus(")
        return s[:i] + GKS_PSYZ
    if k == "gks_sep1":
        s = rep(s, "    s32 base;\n", "    s32 rxx;\n    s32 base;\n")
    elif k == "gks_sepall":
        s = rep(s, "    s32 base;\n", "    s32 off;\n    s32 rxx;\n    s32 addr;\n    s32 base;\n")
    elif k != "gks_raw":
        s = rep(s, "    s32 base;\n", "    volatile SPU_VOICE_REG *voices;\n    s32 base;\n")
    return rep(s, GKS_OLD, GKS[k])


GVEX = {
    "": "    *a1 = _spu_RXX->rxx.voice[a0].volumex;\n",
    "gvex_holder": "    volatile SPU_VOICE_REG *voices = _spu_RXX->rxx.voice;\n    *a1 = voices[a0].volumex;\n",
    "gvex_ptradd": "    *a1 = (_spu_RXX->rxx.voice + a0)->volumex;\n",
    "gvex_raw": "    *a1 = _spu_RXX->raw[a0 * 8 + 6];\n",
    "gvex_cast": "    *a1 = ((SPU_VOICE_REG *)_spu_RXX)[a0].volumex;\n",
    "gvex_vp": "    volatile SPU_VOICE_REG *v = &_spu_RXX->rxx.voice[a0];\n    *a1 = v->volumex;\n",
}


def s_gvex(s):
    k = ([o for o in OPT if o.startswith("gvex_")] or [""])[0]
    return rep(s, """    a0 = (a0 << 4) + _spu_RXX;
    *a1 = *(u16 *)(a0 + 0xC);
""", GVEX[k])


def s_gvv(s):
    s = rep(s, "    s32 temp;\n", "")
    s = rep(s, """    temp = (arg0 << 4) + _spu_RXX;
    temp_v1 = *(u16 *)(temp);
    temp_a0_2 = *(u16 *)(temp + 2);
""", """    temp_v1 = _spu_RXX->raw[arg0 * 8];
    temp_a0_2 = _spu_RXX->raw[arg0 * 8 + 1];
""")
    return s


def sr_gaks(s):
    if "gaks_holder" in OPT:
        a = """        volatile SPU_VOICE_REG *voices = _spu_RXX->rxx.voice;
        s32 bit;
        volumex = voices[voice].volumex;
"""
        b = """        volatile SPU_VOICE_REG *voices = _spu_RXX->rxx.voice;
        u16 volumex;
        s32 bit;
        volumex = voices[voice].volumex;
"""
    elif "gaks_raw" in OPT:
        a = """        s32 bit;
        volumex = _spu_RXX->raw[(8 * voice) + 6];
"""
        b = """        u16 volumex;
        s32 bit;
        volumex = _spu_RXX->raw[(8 * voice) + 6];
"""
    else:
        a = """        s32 bit;
        volumex = _spu_RXX->rxx.voice[voice].volumex;
"""
        b = """        u16 volumex;
        s32 bit;
        volumex = _spu_RXX->rxx.voice[voice].volumex;
"""
    s = rep(s, """        s32 off = voice << 4;
        s32 bit;
        volumex = *(u16 *)((off + _spu_RXX) + 0xC);
""", a)
    s = rep(s, """        s32 off = voice << 4;
        u16 volumex;
        s32 bit;
        volumex = *((u16 *)((off + _spu_RXX) + 0xC));
""", b)
    return s


def f7bc88(s):
    s = re.sub(r"\*\(volatile u16 \*\)\(_spu_RXX \+ \(pos \+ (\d)\) \* 2\)", r"_spu_RXX->raw[pos + \1]", s)
    s = rep(s, "*(volatile u16 *)(_spu_RXX + pos * 2)", "_spu_RXX->raw[pos]")
    if "a_svaplain" in OPT:   # ablation: the existing volatile-locals FAKE
        s = rep(s, "    volatile s32 i; /*", "    s32 i; /*")
        s = rep(s, "    volatile s32 v; /*", "    s32 v; /*")
    return s


SPU_FILES = [("libspu_internal.h", internal), ("s_srmp.c", s_srmp), ("s_sra.c", s_sra), ("spu.c", spu),
             ("s_sca.c", rxx_sites), ("s_sr.c", rxx_sites), ("s_sk.c", s_sk),
             ("s_snc.c", s_snc), ("s_gks.c", s_gks), ("s_gvex.c", s_gvex), ("s_gvv.c", s_gvv),
             ("sr_gaks.c", sr_gaks), ("7BC88.c", f7bc88)]

# ------------------------------------------------------------------ libgpu
GPU = "src/main/psxsdk/libgpu/"


def libgpu_h(s):
    s = rep(s, """#define getcode(p) (u8)(((P_TAG *)(p))->code)
""", """#define getcode(p) (u8)(((P_TAG *)(p))->code)
#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (u32)(_addr))
""")
    s = rep(s, """/* PsyQ DR_MOVE: DMA tag word, then five GPU command words. */""",
            """typedef struct { u32 tag; u32 code[2]; } DR_PRIO;   /* Mask Priority */

/* PsyQ DR_MOVE: DMA tag word, then five GPU command words. */""")
    return s


REV_SOTN = """u32 SetGraphReverse(s32 a0) {
    u32 old = g_gpu_ctx.reverse;
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015E90, a0);
    }
    g_gpu_ctx.reverse = a0;
    g_gpu_dev_table->ctl(0x08000000 | (g_gpu_ctx.reverse ? 0x80 : 0) | g_gpu_dev_table->getctl(8));
    if (g_gpu_ctx.type == 2) {
        g_gpu_dev_table->ctl(0x20000000 | (g_gpu_ctx.reverse ? 0x501 : 0x504));
    }
    return old;
}
"""


def sys_c(s):
    # the device table, by member
    s = rep(s, "        ((void (*)(s32))((u32 *)g_gpu_dev_table)[0x34 / 4])(1);\n",
            "        g_gpu_dev_table->reset(1);\n", 2)
    s = rep(s, "    val = ((u32 (*)(s32))((u32 *)g_gpu_dev_table)[0x28 / 4])(8);\n",
            "    val = g_gpu_dev_table->getctl(8);\n")
    s = rep(s, "    ((void (*)(u32))((u32 *)g_gpu_dev_table)[0x10 / 4])(val);\n",
            "    g_gpu_dev_table->ctl(val);\n")
    if "rev_sotn" in OPT:   # SOTN's spelling of SetGraphReverse (rev-w2b1: 0 hunks)
        s = fn(s, "SetGraphReverse", lambda b: REV_SOTN)
    elif "rev_tbl" in OPT:
        s = rep(s, "        u32 *tbl = (u32 *)g_gpu_dev_table;\n", "        GpuDevTable *tbl = g_gpu_dev_table;\n")
        s = rep(s, "        ((void (*)(u32))tbl[0x10 / 4])(val);\n", "        tbl->ctl(val);\n")
    else:
        s = rep(s, "        u32 *tbl = (u32 *)g_gpu_dev_table;\n", "")
        s = rep(s, "        ((void (*)(u32))tbl[0x10 / 4])(val);\n", "        g_gpu_dev_table->ctl(val);\n")
    if "prim_dev" in OPT:
        s = rep(s, """    u32 *dev = (u32 *)g_gpu_dev_table;
    u32 size = a0[3];
    ((void (*)(s32))dev[15])(0);
    dev = (u32 *)g_gpu_dev_table;
    ((void (*)(u32 *, u32))dev[5])(a0 + 4, size);
""", """    GpuDevTable *dev = g_gpu_dev_table;
    u32 size = a0[3];
    dev->sync(0);
    dev = g_gpu_dev_table;
    dev->cwb(a0 + 4, size);
""")
    else:
        s = rep(s, """    u32 *dev = (u32 *)g_gpu_dev_table;
    u32 size = a0[3];
    ((void (*)(s32))dev[15])(0);
    dev = (u32 *)g_gpu_dev_table;
    ((void (*)(u32 *, u32))dev[5])(a0 + 4, size);
""", """    u32 size = a0[3];
    g_gpu_dev_table->sync(0);
    g_gpu_dev_table->cwb(a0 + 4, size);
""")
    if "otenv_dev" in OPT:
        s = rep(s, """    u32 *dev;

    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015FDC, arg0, env);""", """    GpuDevTable *dev;

    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015FDC, arg0, env);""")
        s = rep(s, """    dev = (u32 *)g_gpu_dev_table;
    ((s32 (*)(u32, DR_ENV *, s32, s32))dev[2])(dev[6], &env->dr_env, 0x40, 0);
""", """    dev = g_gpu_dev_table;
    dev->addque2(dev->cwc, &env->dr_env, 0x40, 0);
""")
    else:
        s = rep(s, """    u32 *dev;

    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015FDC, arg0, env);""", """    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015FDC, arg0, env);""")
        s = rep(s, """    dev = (u32 *)g_gpu_dev_table;
    ((s32 (*)(u32, DR_ENV *, s32, s32))dev[2])(dev[6], &env->dr_env, 0x40, 0);
""", """    g_gpu_dev_table->addque2(g_gpu_dev_table->cwc, &env->dr_env, 0x40, 0);
""")
    s = rep(s, """    s32 (*func)(void) = ((s32 (**)(void))g_gpu_dev_table)[0xE];
    return (u32)func() >> 31;
""", """    return g_gpu_dev_table->status() >> 31;
""")
    s = rep(s, """ * PutDrawEnv call through the members (measured byte-identical). A few other
 * call sites in this file still use a `(u32 *)` word view of the same
 * pointer; that is recorded debt, not a codegen requirement. */""",
            """ * PutDrawEnv call through the members (measured byte-identical), as do the
 * other users except ClearOTagR (its word view is the P7c debt row). */""")
    # ClearOTag: the OT tag word through PsyQ's setlen / setaddr
    s = rep(s, """    a1--;
    if (a1) {
        u32 mask = 0xFFFFFF;
        u32 himask = 0xFF000000;
        do {
            u32 *next;
            a1--;
            next = a0 + 1;
            ((u8 *)a0)[3] = 0;
            *a0 = (*a0 & himask) | ((u32)next & mask);
            a0 = next;
        } while (a1);
    }
""", """    while (--a1) {
        setlen(a0, 0);
        setaddr(a0, a0 + 1);
        a0++;
    }
""")
    # SetPriority: PsyQ's DR_PRIO
    s = rep(s, """void SetPriority(u8 *a0, s32 a1, s32 a2) {
    u32 v0;
    a0[3] = 2;
""", """void SetPriority(DR_PRIO *p, s32 a1, s32 a2) {
    u32 v0;
    setlen(p, 2);
""")
    s = rep(s, """    *(u32 *)(a0 + 4) = v0;
    *(u32 *)(a0 + 8) = 0;
""", """    p->code[0] = v0;
    p->code[1] = 0;
""")
    # get_dx takes the DISPENV
    s = rep(s, "s32 get_dx(s16 *arg0);\n", "s32 get_dx(DISPENV *env);\n")
    s = rep(s, "(get_dx((s16 *)env) & 0xFFF)", "(get_dx(env) & 0xFFF)")
    s = rep(s, "s32 get_dx(s16 *arg0) {\n", "s32 get_dx(DISPENV *env) {\n")
    s = rep(s, "            v1 = arg0[2];\n            a = arg0[0];\n",
            "            v1 = env->disp.w;\n            a = env->disp.x;\n")
    s = rep(s, "        t = arg0[0];\n        goto ret;\n", "        t = env->disp.x;\n        goto ret;\n")
    s = rep(s, "            v1 = ((s16)(*((u16 *)(arg0 + 2)))) / 2;\n            a = arg0[0];\n",
            "            v1 = env->disp.w / 2;\n            a = env->disp.x;\n")
    s = rep(s, "        t = ((s32)((s16)(*((u16 *)arg0)))) / 2;\n", "        t = env->disp.x / 2;\n")
    s = rep(s, "    default:\n        t = arg0[0];\n", "    default:\n        t = env->disp.x;\n")
    # SetDrawEnv / SetDrawEnv2: PsyQ's (DR_ENV *, DRAWENV *)
    s = rep(s, """typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
    s16 u;
    s16 v;
    u8 pad12[4];
    s16 ax;
    s16 ay;
    u16 cx;
    u16 cy;
    u8 flag;
    u8 r;
    u8 g;
    u8 b;
} Rect;
""", "")
    for f in ("SetDrawEnv", "SetDrawEnv2"):
        s = fn(s, f, drawenv_body)
    # the MMIO register pointers are volatile at type level; their casts are no-ops
    if "bcr_vol" in OPT:
        s = rep(s, "extern u32 *g_gpu_dma_bcr;\n", "extern volatile u32 *g_gpu_dma_bcr;\n")
        s = rep(s, "    *(volatile u32 *)g_gpu_dma_bcr = 0;\n", "    *g_gpu_dma_bcr = 0;\n")
    s = rep(s, "    *(volatile u32 *)g_gpu_stat_reg = GP1_DMA_DIR;\n", "    *g_gpu_stat_reg = GP1_DMA_DIR;\n")
    s = rep(s, "        *(volatile u32 *)g_gpu_data_reg = *a0++;\n", "        *g_gpu_data_reg = *a0++;\n")
    s = rep(s, "    *(volatile u32 *)g_gpu_stat_reg = GP1_DMA_DIR_FIFO;\n    *(volatile u32 *)g_gpu_dma_madr = a0;\n",
            "    *g_gpu_stat_reg = GP1_DMA_DIR_FIFO;\n    *g_gpu_dma_madr = a0;\n")
    s = rep(s, "    *(volatile u32 *)g_gpu_dma_chcr = DMA_GPU_LINKED_LIST;\n", "    *g_gpu_dma_chcr = DMA_GPU_LINKED_LIST;\n")
    s = s.replace("*(volatile s32 *)g_gpu_stat_reg", "*g_gpu_stat_reg")
    s = s.replace("*(volatile s32 *)g_gpu_data_reg", "*g_gpu_data_reg")
    if "a_pdvol" in OPT:      # ablation: PutDispEnv's volatile-read FAKE cluster
        s = rep(s, "*(volatile s16 *)&g_gpu_ctx.", "g_gpu_ctx.", 8)
    if "a_pdarm" in OPT:      # ablation: PutDispEnv's empty then-arm
        s = rep(s, """        if (env->disp.h <= (env->pad0 ? 288 : 256)) {
        } else {
            mode |= 0x24;
        }
""", """        if (env->disp.h > (env->pad0 ? 288 : 256)) {
            mode |= 0x24;
        }
""")
    if "a_dxwrap" in OPT:     # ablation: get_dx's do-while(0)
        s = rep(s, "            do { t = 0x400; } while (0);\n", "            t = 0x400;\n")
    return s


def drawenv_body(b):
    b = rep(b, "(s32 *out, Rect *r)", "(DR_ENV *out, DRAWENV *r)")
    if "a_dealias" in OPT:    # ablation: the existing prologue-order alias FAKE
        i = b.index("  s32 *o = out;")
        j = b.index("*/\n", i) + 3
        b = b[:i] + b[j:]
        b = rep(b, "(DR_ENV *out, DRAWENV *r)", "(DR_ENV *o, DRAWENV *r)")
    else:
        b = rep(b, "  s32 *o = out;", "  DR_ENV *o = out;")
    b = rep(b, "  o[1] = get_cs(r->x, r->y);", "  o->code[0] = get_cs(r->clip.x, r->clip.y);")
    if "de_casts" in OPT:   # the m2c casts kept
        b = rep(b, "  o[2] = get_ce((s16) ((((u16) r->w) + ((u16) r->x)) - 1), (s16) ((((u16) r->y) + ((u16) r->h)) - 1));",
                "  o->code[1] = get_ce((s16) ((((u16) r->clip.w) + ((u16) r->clip.x)) - 1), (s16) ((((u16) r->clip.y) + ((u16) r->clip.h)) - 1));")
    else:
        b = rep(b, "  o[2] = get_ce((s16) ((((u16) r->w) + ((u16) r->x)) - 1), (s16) ((((u16) r->y) + ((u16) r->h)) - 1));",
                "  o->code[1] = get_ce(r->clip.w + r->clip.x - 1, r->clip.y + r->clip.h - 1);")
        for a, c in (("(u16) r->x;", "r->x;"), ("(u16) r->y;", "r->y;"), ("(s16) r->w;", "r->w;"),
                     ("(u16) r->w;", "r->w;"), ("(u16) r->h;", "r->h;"), ("(u16) r->u;", "r->u;"),
                     ("(u16) r->v;", "r->v;")):
            b = b.replace(a, c)
        b = b.replace("buf[0] = r->x;", "buf[0] = r->clip.x;").replace("buf[1] = r->y;", "buf[1] = r->clip.y;")
        b = b.replace("new_var = r->w;", "new_var = r->clip.w;").replace("buf[2] = r->w;", "buf[2] = r->clip.w;")
        b = b.replace("buf[3] = r->h;", "buf[3] = r->clip.h;")
        b = b.replace(" r->u;", " r->ofs[0];").replace(" r->v;", " r->ofs[1];")
    b = rep(b, "  o[3] = get_ofs(r->u, r->v);", "  o->code[2] = get_ofs(r->ofs[0], r->ofs[1]);")
    b = rep(b, "  o[4] = get_mode(*(((u8 *) r) + 23), *(((u8 *) r) + 22), *((u16 *) (((u8 *) r) + 20)));",
            "  o->code[3] = get_mode(r->dfe, r->dtd, r->tpage);")
    b = rep(b, "  o[5] = get_tw(((u8 *) r) + 12);", "  o->code[4] = get_tw(&r->tw);")
    b = rep(b, "  o[6] = (s32) 0xE6000000;\n  var_a3 = 7;", "  o->code[5] = 0xE6000000;\n  var_a3 = 6;")
    b = rep(b, "  if (r->flag != 0)", "  if (r->isbg != 0)")
    b = b.replace("buf[0] = (u16) r->x;", "buf[0] = (u16) r->clip.x;")
    b = b.replace("buf[1] = (u16) r->y;", "buf[1] = (u16) r->clip.y;")
    b = b.replace("new_var = (s16) r->w;", "new_var = (s16) r->clip.w;")
    b = b.replace("buf[2] = (u16) r->w;", "buf[2] = (u16) r->clip.w;")
    b = b.replace("buf[3] = (u16) r->h;", "buf[3] = (u16) r->clip.h;")
    b = b.replace("(u16) r->u;", "(u16) r->ofs[0];").replace("(u16) r->v;", "(u16) r->ofs[1];")
    b = b.replace("((((*(((u8 *) r) + 27)) << 16) | 0x60000000) | ((*(((u8 *) r) + 26)) << 8)) | (*(((u8 *) r) + 25))",
                  "(((r->b0 << 16) | 0x60000000) | (r->g0 << 8)) | r->r0")
    b = b.replace("((((*(((u8 *) r) + 27)) << 16) | 0x02000000) | ((*(((u8 *) r) + 26)) << 8)) | (*(((u8 *) r) + 25))",
                  "(((r->b0 << 16) | 0x02000000) | (r->g0 << 8)) | r->r0")
    if "de_c" in OPT:
        b = rep(b, "  var_a3 = 6;", "  var_a3 = 7;")
        b = b.replace("o[var_a3++] =", "*(o->code + var_a3++ - 1) =")
        b = rep(b, "  *(((s8 *) o) + 3) = (s8) (var_a3 - 1);", "  setlen(o, var_a3 - 1);")
    elif "de_d" in OPT:
        b = rep(b, "  var_a3 = 6;", "  var_a3 = 6;\n  code = o->code;")
        b = rep(b, "  s32 var_a3;\n", "  s32 var_a3;\n  u32 *code;\n")
        b = b.replace("o[var_a3++] =", "code[var_a3++] =")
        b = rep(b, "  *(((s8 *) o) + 3) = (s8) (var_a3 - 1);", "  setlen(o, var_a3);")
    elif "de_b" in OPT:
        b = rep(b, "  var_a3 = 6;", "  var_a3 = 7;")
        b = b.replace("o[var_a3++] =", "o->code[var_a3++ - 1] =")
        b = rep(b, "  *(((s8 *) o) + 3) = (s8) (var_a3 - 1);", "  setlen(o, var_a3 - 1);")
    else:
        b = b.replace("o[var_a3++] =", "o->code[var_a3++] =")
        b = rep(b, "  *(((s8 *) o) + 3) = (s8) (var_a3 - 1);", "  setlen(o, var_a3);")
    return b


def ext_c(s):
    s = rep(s, """s16 *SetDefDrawEnv(s16 *a0, s16 a1, s16 a2, s16 a3, s32 a4) {
    s32 ret;
    ret = GetVideoMode();
    a0[0] = a1;
    a0[1] = a2;
    a0[2] = a3;
    a0[6] = 0;
    a0[7] = 0;
    a0[8] = 0;
    a0[9] = 0;
    ((s8 *)a0)[0x19] = 0;
    ((s8 *)a0)[0x1A] = 0;
    ((s8 *)a0)[0x1B] = 0;
    ((s8 *)a0)[0x16] = 1;
    a0[3] = a4;
    if (ret) {
        ((s8 *)a0)[0x17] = (a4 < 0x121);
    } else {
        ((s8 *)a0)[0x17] = (a4 < 0x101);
    }
    a0[4] = a1;
    a0[5] = a2;
    a0[0xA] = 10;
    ((s8 *)a0)[0x18] = 0;
    return a0;
}
""", """DRAWENV *SetDefDrawEnv(DRAWENV *env, s32 x, s32 y, s32 w, s32 h) {
    s32 ret;
    ret = GetVideoMode();
    env->clip.x = x;
    env->clip.y = y;
    env->clip.w = w;
    env->tw.x = 0;
    env->tw.y = 0;
    env->tw.w = 0;
    env->tw.h = 0;
    env->r0 = 0;
    env->g0 = 0;
    env->b0 = 0;
    env->dtd = 1;
    env->clip.h = h;
    if (ret) {
        env->dfe = (h < 0x121);
    } else {
        env->dfe = (h < 0x101);
    }
    env->ofs[0] = x;
    env->ofs[1] = y;
    env->tpage = 10;
    env->isbg = 0;
    return env;
}
""")
    s = rep(s, """s16 *SetDefDispEnv(s16 *a0, s16 a1, s16 a2, s16 a3, s32 a4) {
    a0[0] = a1;
    a0[1] = a2;
    a0[2] = a3;
    a0[4] = 0;
    a0[5] = 0;
    a0[6] = 0;
    a0[7] = 0;
    ((s8 *)a0)[0x11] = 0;
    ((s8 *)a0)[0x10] = 0;
    ((s8 *)a0)[0x13] = 0;
    ((s8 *)a0)[0x12] = 0;
    a0[3] = a4;
    return a0;
}""", """DISPENV *SetDefDispEnv(DISPENV *env, s32 x, s32 y, s32 w, s32 h) {
    env->disp.x = x;
    env->disp.y = y;
    env->disp.w = w;
    env->screen.x = 0;
    env->screen.y = 0;
    env->screen.w = 0;
    env->screen.h = 0;
    env->isrgb24 = 0;
    env->isinter = 0;
    env->pad1 = 0;
    env->pad0 = 0;
    env->disp.h = h;
    return env;
}""")
    return s


GPU_FILES = [("include/psxsdk/libgpu.h", libgpu_h), (GPU + "sys.c", sys_c), (GPU + "ext.c", ext_c)]

FILES = [(SPU + n, g) for n, g in SPU_FILES] + GPU_FILES
RAW_OK = {SPU + "s_gks.c"} if OPT & {"gks_raw", "gks_sep1", "gks_sepall"} else set()


# ------------------------------------------------------------------ comments (FAKE labels, scores)
NOTES = {
    SPU + "s_gks.c": [("gks_raw", """work:
    base = bit_found << 4;
""", """work:
    /* FAKE: mask doubles as the _spu_RXX holder and base as offset / address / value; a separate
       _spu_RXX local scores 16, separate locals for every role 13. */
    base = bit_found << 4;
""")],
    SPU + "spu.c": [("init_ptr", """    _spu_inTransfer = 1;
    spucnt = &_spu_RXX->rxx.spucnt;
""", """    _spu_inTransfer = 1;
    /* FAKE: SPUCNT stored through a pointer; the member store
       `_spu_RXX->rxx.spucnt = 0xC000` lets sched lift the _spu_IRQCallback zero store
       above the sh (score 4). */
    spucnt = &_spu_RXX->rxx.spucnt;
""")],
    SPU + "s_gvex.c": [("gvex_holder", """    volatile SPU_VOICE_REG *voices = _spu_RXX->rxx.voice;
""", """    /* FAKE: the voices holder (psyz's SR_GAKS / S_GKS spelling); reading
       _spu_RXX->rxx.voice[a0].volumex directly adds the index into the base register
       (addu v0,v0,a0 / lhu 12(v0) for addu a0,a0,v0 / lhu 12(a0)), score 2. */
    volatile SPU_VOICE_REG *voices = _spu_RXX->rxx.voice;
""")],
    SPU + "sr_gaks.c": [("gaks_holder", """        volatile SPU_VOICE_REG *voices = _spu_RXX->rxx.voice;
        s32 bit;
""", """        /* FAKE: the voices holder (psyz's SR_GAKS spelling); reading
           _spu_RXX->rxx.voice[voice].volumex directly swaps the index and base registers
           (sll v0 / lw a0 for sll a0 / lw v0), score 3 (also in SpuGetAllKeysStatus). */
        volatile SPU_VOICE_REG *voices = _spu_RXX->rxx.voice;
        s32 bit;
"""), ("gaks_holder", """        volatile SPU_VOICE_REG *voices = _spu_RXX->rxx.voice;
        u16 volumex;
""", """        /* FAKE: the voices holder, as in SpuRGetAllKeysStatus above (score 3). */
        volatile SPU_VOICE_REG *voices = _spu_RXX->rxx.voice;
        u16 volumex;
""")],
    GPU + "sys.c": [("rev_tbl", """        GpuDevTable *tbl = g_gpu_dev_table;
""", """        /* FAKE: the tbl holder; g_gpu_dev_table->ctl(val) directly reloads the table
           after the reverse test (lui / lw / nop ahead of the lw 16) instead of loading it
           above the beqz, score 6. */
        GpuDevTable *tbl = g_gpu_dev_table;
"""), ("de_c", """  o->code[5] = 0xE6000000;
  var_a3 = 7;
""", """  o->code[5] = 0xE6000000;
  /* FAKE: var_a3 counts the tag word, and the pushes are pointer arithmetic
     (*(o->code + var_a3++ - 1)); the subscript o->code[var_a3++ - 1] adds the base
     first (addu aN,s1,aN for addu aN,aN,s1 at every push): score 3 in SetDrawEnv,
     6 in SetDrawEnv2. */
  var_a3 = 7;
""", 2), ("!a_dealias", """                   alias, K&R decl-block reversal or do-while(0) entry wrap
                   leaves the pair order unchanged. */""", """                   alias, K&R decl-block reversal or do-while(0) entry wrap
                   leaves the pair order unchanged. Without it: score 4. */"""),
        ("!a_dealias", """                   prologue save+def pair emit order to match target
                   (s0-pair first). */""", """                   prologue save+def pair emit order to match target
                   (s0-pair first). Without it: score 4. */"""),
        ("", """       words and has none.
       SOTN: src/main/psxsdk/libspu/s_m_m.c:48 @db41b28 */""", """       words and has none. The plain reads: score 71.
       SOTN: src/main/psxsdk/libspu/s_m_m.c:48 @db41b28 */"""),
        ("", """           and its respellings add 4 insns.""", """           and its respellings add 4 insns (score 5)."""),
        ("", """               fills the jump delay slot instead of hoisting to block top */""",
         """               fills the jump delay slot instead of hoisting to block top
               (score 10) */""")],
}


def notes(p, s):
    for row in NOTES.get(p, []):
        o, a, b = row[:3]
        if o == "" or o in OPT or (o[0] == "!" and o[1:] not in OPT):
            s = rep(s, a, b, row[3] if len(row) > 3 else 1)
    return s


def main():
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or [""])[0]
    for p, g in FILES:
        s = notes(p, g(show(p)))
        if re.search(r"_spu_RXX \+|\+ _spu_RXX|\)_spu_RXX|= _spu_RXX;", s) and p not in RAW_OK:
            raise SystemExit("raw _spu_RXX site left in " + p)
        if out:
            os.makedirs(os.path.dirname(out + "/" + p), exist_ok=True)
        open((out + "/" if out else "") + p, "w", encoding="utf-8", newline=NL).write(s)
    print("w2b1 wrote %d files %s" % (len(FILES), sorted(OPT)))


if __name__ == "__main__":
    main()
