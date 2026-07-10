/*
 * CANDIDATE: saTan2Main = PsyQ 4.0 LIBSND vs_vh SsVabOpenHeadWithMode
 * (verbatim-linked Sony object, census 2026-07-09; C ref: sotn-decomp
 * src/main/psxsdk/libsnd/vs_vh.c). Session 2026-07-10.
 *
 * MEASURED FLOORS (sandbox saTan2Main --disable all):
 *   - THIS FILE (per-arm-sum form):        score 5, 247/247 insns
 *   - SOTN single-expression form:         score 4, 245/247 insns
 *     (drop `sum`, use `spuAllocMem = sbaddr;` unconditionally before the
 *      arg2 test and `if ((spuAllocMem + var_s0) > 0x80000U)` single compare)
 * HEAD placeholder floor: whole function unmatched (asmfix whole-body splice).
 *
 * RESIDUAL (per-arm form, ALL in the malloc-fail/overflow join, bytes
 * otherwise byte-identical incl. both loops, all error tails, epilogue):
 *   target: bne a3,v0,.L94 ; delay ADDU v1,a3,s0   <- sum in $v1, taken-thread fill
 *           j   .La8       ; delay MOVE a0,zero
 *           .L90: addu v1,a3,s0
 *           .L94: lui v0,0x8 ; sltu v0,v0,v1
 *   mine:   beq/bne mirror: cond-branch carries the ERROR side, j carries OK,
 *           delay contents swapped, sum lands in $a0.
 * MECHANISM (verified via .greg/.lreg dumps, tmp/gccdump/):
 *   - sum pseudo (87) inherits var_s0(79)'s $a0 preference through
 *     global.c expand_preferences: the sum add carries REG_DEAD var_s0 and
 *     87/79 don't conflict; var_s0 has the a0 copy-pref from the
 *     SpuMalloc arg copy (a0 <- s0). find_reg's preference pass then picks
 *     a0 over free v1 (MIPS has no REG_ALLOC_ORDER; prefs override scan).
 *   - With SOTN's single-expression form the compare temp is block-local
 *     (local_alloc -> v1 correct!) but jump2 fully cross-jump-merges the
 *     malloc errbody to a lone `j` and then collapses bne+j -> beq,
 *     losing the 2 duplicated insns (245 vs 247).
 * RULED OUT (measured):
 *   - split-init `sum = spuAllocMem; sum += var_s0;` in both arms: combine
 *     refolds to (plus 84 79); still a0; score 5 unchanged.
 *   - `u32 sum` declaration position (first/last among locals): no effect.
 *   - do-while(0) around the malloc errbody stmts: blocks the cross-jump
 *     merge entirely -> 260 insns, score 14. REVERTED.
 *   - do-while(0) around the -1 if: no effect on the collapse (score 4,
 *     same beq shape as plain SOTN form). REVERTED.
 * NEXT HYPOTHESES (untested, ~ordered):
 *   1. Find an honest spelling that makes sum conflict var_s0 (blocks the
 *      expand_preferences merge) without combine refolding - e.g. an
 *      intervening real use of sum between init and +=.
 *   2. jump.c:2513 "If cross-jumping here will feed a jump-around-jump"
 *      comment - read that code path; the SOTN-form collapse may be
 *      suppressible by honest layout (a block between the errbody j and
 *      the bne target label blocks the collapse - the per-arm form proved
 *      the structure; only the reg is wrong there).
 *   3. Permuter with PERM_* over this region (register-asm-pins hint class /
 *      compare-operand-order) seeded from the per-arm form.
 *   4. Check whether reorg's taken-thread steal of `addu $a0` (per-arm form)
 *      is rejected because the fall-through errbody's jal NEEDS a0
 *      (mark_target_live_regs conservatism) - if so, ANY form with sum in
 *      v1 + the per-arm structure snaps to target; the v1 problem is the
 *      only real one.
 * Symbol map (BB2 -> Sony), also appended to memory/closer/sony-naming-map.md:
 *   func_8008AF84 -> _spu_getInTransfer ; ReturnVSMode -> _spu_setInTransfer
 *   coli_HitPauseKatana -> SpuMalloc (size-collision name; kengo:HIGH wrong)
 *   D_80102A68 -> _svm_vab_used (u8[16]) ; D_80107808 -> _svm_vab_count (u16)
 *   D_800F66B8 -> _svm_vab_vh ; D_800F6660 -> _svm_vab_pg ; D_800F6700 -> _svm_vab_tn
 *   D_800FF634 -> kMaxPrograms (s16) ; D_80107810 -> _svm_vab_start
 *   D_801077C8 -> _svm_vab_total
 *   PsyQ 4.0 deltas vs SOTN ref: NO _svm_brr_start_addr store after the vag
 *   loop; direct `D_80102A68[vabid]` in the else arm (no var_a2 staging);
 *   VabHdr.vs read as u8 (declare u8 vs + u8 vspad - our fork's subreg
 *   narrowing of a u16 field takes the HIGH byte, lbu 0x17 vs target 0x16);
 *   arg2 is s16 (sll<<16 test), sbaddr u32.
 * Requires prototype line 12 of src/main.c changed to:
 *   extern s16 saTan2Main(u8 *, s16, s16, u32);
 * (callers' codegen unaffected - verified byte-identity of callers not yet
 *  done; check func_80088058/coli_CheckBukiPreHit_* when applying.)
 */

/* PsyQ VabHdr (libsnd) - VAB bank header */
typedef struct {
    s32 form;
    s32 ver;
    s32 id;
    u32 fsize;
    u16 reserved0;
    u16 ps;
    u16 ts;
    u8 vs; /* low byte read directly (lbu 0x16) */
    u8 vspad;
    u8 mvol;
    u8 pan;
    u8 attr1;
    u8 attr2;
    u32 reserved1;
} VabHdr;
/* PsyQ ProgAtr (libsnd) - program attribute table entry (16 bytes) */
typedef struct {
    u8 tones;
    u8 mvol;
    u8 prior;
    u8 mode;
    u8 mpan;
    s8 reserved0;
    s16 attr;
    u32 reserved1; /* program index (skips empties) */
    u16 reserved2; /* even vag spu ptr */
    u16 reserved3; /* odd vag spu ptr */
} ProgAtr;
extern u16 D_80107808;   /* _svm_vab_count */
extern s32 D_80107810[]; /* _svm_vab_start */
extern s32 D_801077C8[]; /* _svm_vab_total */
extern s32 coli_HitPauseKatana(s32);
/* PsyQ 4.0 LIBSND vs_vh: SsVabOpenHeadWithMode — verbatim-linked Sony object (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/vs_vh.c */
s16 saTan2Main(u8 *addr, s16 vabid, s16 arg2, u32 sbaddr) {
    int vagLens[256];
    s32 i;
    s32 var_s0;
    s16 vabId_2;
    u16 temp_v1;
    u16 *ptr_vag_off_table;
    u32 magic;
    u32 spuAllocMem;
    u8 num_vags;
    ProgAtr *pProgTable;
    u8 *var_a2;
    VabHdr *vab_hdr_2;
    u32 sum;
    vabId_2 = 0x10;
    if (func_8008AF84() == 1) {
        return -1;
    }
    ReturnVSMode(1);
    if (vabid >= 0x10) {
        ReturnVSMode(0);
        return -1;
    }
    if (vabid == -1) {
        for (i = 0; i < 16; i++) {
            if (D_80102A68[i] == 0) {
                D_80102A68[i] = 1;
                vabId_2 = i;
                D_80107808++;
                break;
            }
        }
    } else {
        var_a2 = D_80102A68;
        if (var_a2[vabid] == 0) {
            D_80102A68[vabid] = 1;
            vabId_2 = vabid;
            D_80107808++;
        }
    }
    if (vabId_2 >= 0x10) {
        ReturnVSMode(0);
        return -1;
    }
    var_a2 = addr;
    D_800F66B8[vabId_2] = (s32)var_a2;

    var_a2 = var_a2 + 0x20;
    vab_hdr_2 = (VabHdr *)addr;
    magic = vab_hdr_2->form;
    if ((magic >> 8) != ('V' << 0x10 | 'A' << 0x8 | 'B')) {
        D_80102A68[vabId_2] = 0;
        ReturnVSMode(0);
        D_80107808 -= 1;
        return -1;
    }
    if ((magic & 0xFF) == 'p') {
        if (vab_hdr_2->ver >= 5) {
            D_800FF634 = 0x80;
        } else {
            D_800FF634 = 0x40;
        }
    } else {
        D_800FF634 = 0x40;
    }
    if (vab_hdr_2->ps <= D_800FF634) {
        D_800F6660[vabId_2] = (s32)var_a2;
        pProgTable = (ProgAtr *)var_a2;
        var_a2 = var_a2 + (D_800FF634 * 0x10);
        var_s0 = 0;
        for (i = 0; i < D_800FF634; i++) {
            pProgTable[i].reserved1 = var_s0;
            if (pProgTable[i].tones != 0) {
                var_s0++;
            }
        }
        var_s0 = 0;
        D_800F6700[vabId_2] = (s32)var_a2;
        ptr_vag_off_table = (u16 *)(var_a2 + (vab_hdr_2->ps << 9));
        num_vags = vab_hdr_2->vs;
        for (i = 0; i < 256; i++) {
            if (num_vags >= i) {
                temp_v1 = *ptr_vag_off_table;
                if (vab_hdr_2->ver >= 5) {
                    vagLens[i] = temp_v1 * 8;
                } else {
                    vagLens[i] = temp_v1 * 4;
                }
                var_s0 += vagLens[i];
            }
            ptr_vag_off_table++;
        }
        if (arg2 == 0) {
            spuAllocMem = coli_HitPauseKatana(var_s0);
            if (spuAllocMem == -1) {
                D_80102A68[vabId_2] = 0;
                ReturnVSMode(0);
                D_80107808 -= 1;
                return -1;
            }
            sum = spuAllocMem + var_s0;
        } else {
            spuAllocMem = sbaddr;
            sum = spuAllocMem + var_s0;
        }
        if (sum > 0x80000U) {
        end:
            D_80102A68[vabId_2] = 0;

            ReturnVSMode(0);
            D_80107808 -= 1;
            return -1;
        }
        D_80107810[vabId_2] = spuAllocMem;
        var_s0 = 0;
        for (i = 0; i <= num_vags; i++) {
            var_s0 += vagLens[i];
            if (!(i & 1)) {
                pProgTable[i / 2].reserved2 = (spuAllocMem + var_s0) >> 3;
            } else {
                pProgTable[i / 2].reserved3 = (spuAllocMem + var_s0) >> 3;
            }
        }

        D_801077C8[vabId_2] = var_s0;
        D_80102A68[vabId_2] = 2;
    } else {
        goto end;
    }
    return vabId_2;
}
