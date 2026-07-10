/* CANDIDATE (Closer phase-3, 2026-07-10): SpuSetReverbModeParam.
 * State: masked sandbox 23 = 20 struct-reloc-addend artifacts (D_800A2888
 * SpuRevAttr struct spelling; self-heal on driver rebuild) + 3 REAL words:
 * sp58's spill slot lands at sp+0x5C (mine) vs sp+0x58 (target).
 * Root cause (measured, tmp/closer/srmp/f.greg): our reload runs TWO spill
 * passes ("Spilling reg 8 / reg 64" x2) leaking two transient stack slots
 * (0x54,0x58) before sp58's; Sony's compile leaked one. Registers and all
 * other 315 words are exact. Killed levers: flag-init reorder (rotates
 * s-regs, 31), shared-quotient q local (inert, CSE already unified).
 * Next probes: identify the pass-1-stacked pseudo set (instrument reload),
 * or respell the mult/hi-pressure delay block to survive one reload pass.
 * NOTE: this body is ALSO live in src/main.c (safe: whole-body asmfix
 * splice still active); the D_800A2888 linker symbol was added to
 * undefined_syms_auto.txt. */
/* PsyQ 4.0 LIBSPU s_srmp: SpuSetReverbModeParam — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_srmp.c.
   4.0 deltas vs the SOTN revision: the DELAYTIME/FEEDBACK gates are range
   compares (mode >= ECHO && mode <= DELAY) with no default-arm clears, and
   the depth/zero split threads off the var_s4 flag. Reverb preset table:
   D_800A2D94 (= Sony rev_param table, 10 entries x 0x44). */
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

typedef struct {
    /* 0x00 */ u32 mask;
    /* 0x04 */ s32 mode;
    /* 0x08 */ SpuVolume depth;
    /* 0x0C */ s32 delay;
    /* 0x10 */ s32 feedback;
} SpuReverbAttr;

extern RevParamEntry D_800A2D94[]; /* rev_param preset table */

/* Sony _spu_rev_attr — ONE struct (sotn libspu_internal.h:87 struct
   SpuRevAttr), base 0x800A2888. Members == the split splat symbols
   D_800A288C (mode) / D_800A2890/92 (depth L/R) / D_800A2894 (delay) /
   D_800A2898 (feedback), which other already-matched functions in this TU
   still reference by their per-member names (same linked bytes). */
typedef struct {
    /* 0x00 */ u32 unk0;
    /* 0x04 */ s32 mode;
    /* 0x08 */ SpuVolume depth;
    /* 0x0C */ s32 delay;
    /* 0x10 */ s32 feedback;
} SpuRevAttr;
extern SpuRevAttr D_800A2888; /* _spu_rev_attr */

void func_8008A434(s32 *arg0);
s32 md_game_check_change_main_mode_katinuki(u32 rev_mode);

static inline void _memcpy(char *dst, char *src, u32 size) {
    while (size--) {
        *dst++ = *src++;
    }
}

s32 func_80089F3C(SpuReverbAttr *attr) {
    RevParamEntry entry;
    u32 var_s0;
    u16 cnt;

    s32 var_s7 = 0;
    s32 var_s4 = 0;
    s32 var_s6 = 0;
    s32 sp58 = 0;
    s32 var_fp = 0;

    u32 mask = attr->mask;
    s32 bSetAll = attr->mask == 0;

    entry.flags = 0;
    if (bSetAll || (mask & 0x1)) {
        var_s0 = attr->mode;
        if (attr->mode & 0x100) {
            var_s0 &= ~0x100;
            sp58 = 1;
        }
        if (var_s0 >= 0xA || func_80089EB0(D_800A2D44[var_s0])) {
            return -1;
        }
        var_s4 = 1;
        D_800A2888.mode = var_s0;
        D_800A2884 = D_800A2D44[D_800A2888.mode];
        _memcpy((char *)&entry, (char *)&D_800A2D94[D_800A2888.mode],
                sizeof(RevParamEntry));
        switch (D_800A2888.mode) {
        case 7: /* SPU_REV_MODE_ECHO */
            D_800A2888.feedback = 0x7F;
            D_800A2888.delay = 0x7F;
            break;
        case 8: /* SPU_REV_MODE_DELAY */
            D_800A2888.feedback = 0;
            D_800A2888.delay = 0x7F;
            break;
        default:
            D_800A2888.feedback = 0;
            D_800A2888.delay = 0;
            break;
        }
    }
    if (bSetAll || (mask & 0x8)) {
        switch (D_800A2888.mode) {
        case 7: /* SPU_REV_MODE_ECHO */
        case 8: /* SPU_REV_MODE_DELAY */
            var_s6 = 1;
            if (!var_s4) {
                _memcpy((char *)&entry, (char *)&D_800A2D94[D_800A2888.mode],
                        sizeof(RevParamEntry));
                entry.flags = 0x0C011C00;
            }
            D_800A2888.delay = attr->delay;
            entry.mLSAME = ((D_800A2888.delay << 0xD) / 0x7F) - entry.dAPF1;
            entry.mRSAME = ((D_800A2888.delay << 0xC) / 0x7F) - entry.dAPF2;
            entry.mLCOMB1 = ((D_800A2888.delay << 0xC) / 0x7F) + entry.mRCOMB1;
            entry.dLSAME = ((D_800A2888.delay << 0xC) / 0x7F) + entry.dRSAME;
            entry.mLAPF1 = ((D_800A2888.delay << 0xC) / 0x7F) + entry.mLAPF2;
            entry.mRAPF1 = ((D_800A2888.delay << 0xC) / 0x7F) + entry.mRAPF2;
            break;
        default:
            break;
        }
    }
    if (bSetAll || (mask & 0x10)) {
        switch (D_800A2888.mode) {
        case 7: /* SPU_REV_MODE_ECHO */
        case 8: /* SPU_REV_MODE_DELAY */
            var_fp = 1;
            if (!var_s4) {
                if (!var_s6) {
                    _memcpy((char *)&entry, (char *)&D_800A2D94[D_800A2888.mode],
                            sizeof(RevParamEntry));
                    entry.flags = 0x80;
                } else {
                    entry.flags |= 0x80;
                }
            }
            D_800A2888.feedback = attr->feedback;
            entry.vWALL = (D_800A2888.feedback * 0x8100) / 0x7F;
            break;
        default:
            break;
        }
    }
    if (var_s4) {
        var_s7 = (*(volatile u16 *)(D_800A2CDC + 0x1AA) >> 7) & 1;
        if (var_s7) {
            cnt = *(volatile u16 *)(D_800A2CDC + 0x1AA);
            cnt &= ~0x80;
            *(volatile u16 *)(D_800A2CDC + 0x1AA) = cnt;
        }
    }
    if (!var_s4) {
        if (bSetAll || (mask & 0x2)) {
            *(u16 *)(D_800A2CDC + 0x184) = attr->depth.left;
            D_800A2888.depth.left = attr->depth.left;
        }
        if (bSetAll || (mask & 0x4)) {
            *(u16 *)(D_800A2CDC + 0x186) = attr->depth.right;
            D_800A2888.depth.right = attr->depth.right;
        }
    } else {
        *(u16 *)(D_800A2CDC + 0x184) = 0;
        *(u16 *)(D_800A2CDC + 0x186) = 0;
        D_800A2888.depth.left = 0;
        D_800A2888.depth.right = 0;
    }
    if (var_s4 || var_s6 || var_fp) {
        func_8008A434((s32 *)&entry);
    }
    if (sp58) {
        md_game_check_change_main_mode_katinuki(D_800A2888.mode);
    }
    if (var_s4) {
        spu_WriteReg(0xD1, D_800A2884, 0);
        if (var_s7) {
            cnt = *(volatile u16 *)(D_800A2CDC + 0x1AA);
            cnt |= 0x80;
            *(volatile u16 *)(D_800A2CDC + 0x1AA) = cnt;
        }
    }
    return 0;
}
