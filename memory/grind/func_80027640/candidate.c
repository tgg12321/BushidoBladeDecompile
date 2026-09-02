/* CANDIDATE -- func_80027640 -- s1 (2026-09-01) -- sandbox --disable all == 0 (158/158 insns
 * instruction-identical; target 160 words incl. the two mfhi->mult hazard nops objdump collapses).
 * Pure C, no FAKE constructs, no volatile. The retired chassis body (8 volatile s32 locals,
 * 4 never referenced) is replaced by two local VECTORs (include/gte.h, libgte canonical):
 *   tgt at sp+0x10 (vx@0x10, vz@0x18; vy/pad untouched)  -- the blended target position
 *   dir at sp+0x20 (vx@0x20, vz@0x28; vy/pad untouched)  -- the push direction
 * GCC 2.7.2 keeps aggregate locals in memory, which reproduces every sp-slot round trip.
 * The direction constants MUST be spelled `v = A; if (cond) v = B; dir.v = v;` -- ternary
 * flips branch sense (rejected/ternary-direction-flips-branch-sense.c), and storing into
 * dir.vx inside if/else arms forces a frame reload (rejected/struct-store-in-both-arms-reloads.c).
 * RESIDUAL (s3, structural): sandbox --disable all == 1 on the stock chassis. The single missing
 * word is the load-delay `nop` at 0x800277A4: target is `lh $v0,0x4($a0)` @0x800277A0 ->
 * `.L800277A4:` -> `nop` -> `sw $v0,0x18($sp)` @0x800277A8, and `j .L800277A4` @0x80027770 pins the
 * label to the nop's address (j word 0x08009DE9 -> 0x009DE9<<2 = 0x800277A4; re-decoded s5).
 * ATTRIBUTION CORRECTED IN s5: cc1's stream is target-identical in the ORDER of its real
 * instructions, but it does NOT contain this nop at all -- tmp/grind/func_80027640/s2/cc1.s:526-531
 * shows `lh $2,4($4)` / `.L75:` / `sw $2,24($sp)` with NO `#nop` marker between them. The nop is
 * suppressed inside cc1 by mips.c:705 (`GET_CODE (next_insn) == CODE_LABEL` zeroes
 * dslots_number_nops), so maspsx cannot be "dropping" a marker that was never emitted; maspsx is
 * only the SECOND blind emitter (its is_label() matches `$L`, not this fork's `.L`). s3 proved
 * this is invariant under every C spelling: any byte-correct form must define a basic-block label
 * at 0x800277A4, textually between the load and its consumer. So NO C change can improve this
 * body -- it is final pure C.
 * The residual is a maspsx fidelity defect; s3 measured a 4-line general repair (apply maspsx's own
 * $at/$gp delay-fill test inside the .L-label branch) that is byte-neutral across all 31 other C
 * objects, relinks to SHA1 == oracle, and makes the per-function maspsx_label_nop_funcs.txt list
 * dead code. That is an operator/tools surface, NOT a gate-list entry (the gate-list route was
 * foreclosed by the Judge and is not re-proposed). Details: memory/grind/func_80027640/evidence.md
 * s3 + docs/grind/decisions.md (2026-09-01 s3 entry).
 * s4 (2026-09-02, permuter modality) left this body UNCHANGED and confirmed it is the best
 * form that exists on the C axis: a faithful stand-alone permuter workspace
 * (tmp/grind/func_80027640/s4/perm, fidelity-proved -- the reduced TU differs from the
 * 160-word target by exactly the one missing nop) scores this chassis at base 100 = one
 * insertion, zero register/reordering penalty, and 56,275 iterations produced no output at
 * all; a structurally different scalar-join chassis (rejected/scalar-join-chassis-perm-basin.c)
 * bottomed out at 750 over 58,381 more. 114,656 iterations, zero score-0 forms.
 * s5 (2026-09-02, synthesis) re-measured this body on the current chassis: sandbox --disable all
 * == 1 (target_insns 158, build_insns 157), and closed the last open escape from mips.c:705 --
 * a project-wide cc1 census (32 TUs, tmp/grind/func_80027640/s5/cc1s) found ZERO cases of a
 * `#nop` surviving a `.L` label and 5 other sites where the suppression fires, and the only
 * insn kind that defeats the predicate (a bare USE/CLOBBER, which final.c:1548-1550 skips before
 * FINAL_PRESCAN_INSN at final.c:1956) has no ordinary-C spelling that leaves it dangling. */
void func_80027640(s32 arg0)
{
    VECTOR tgt;
    VECTOR dir;
    s32 idx;
    s16 *tbl;
    s32 rate;
    u8 cnt;
    void *r1;
    void *r2;
    s32 vx;
    s32 vz;

    idx = *(s16 *)(arg0 + 4);
    tbl = (s16 *)stage_GetDataPtr();
    cnt = *(u8 *)(arg0 + 0x34C);
    if (cnt < 0x40) {
        *(u8 *)(arg0 + 0x34C) = cnt + 1;
    }
    rate = 0x3C - ((*(u8 *)(arg0 + 0x34C) - 1) * 4);
    if (rate < 10) {
        rate = 10;
    }
    if (D_800A36A4 == 3) {
        vx = 0x2EE0;
        if (*(s32 *)(*(s32 *)arg0 + 0xF4) >= 0x3E9) {
            vx = -0x2710;
        }
        dir.vx = vx;
        vz = 0x1770;
        if (*(s32 *)(*(s32 *)arg0 + 0xFC) > 0) {
            vz = -0x1770;
        }
        dir.vz = vz;
        tgt.vx = (dir.vx * rate + *(s32 *)(arg0 + 0xF4) * (100 - rate)) / 100;
        tgt.vz = (dir.vz * rate + *(s32 *)(arg0 + 0xFC) * (100 - rate)) / 100;
    } else {
        tbl += (D_800A36A4 * 12 + idx * 3);
        tgt.vx = tbl[0];
        tgt.vz = tbl[2];
    }
    tgt.vx -= *(s32 *)(arg0 + 0xF4);
    tgt.vz -= *(s32 *)(arg0 + 0xFC);
    *(s32 *)(arg0 + 0xF4) += tgt.vx;
    *(s32 *)(arg0 + 0xFC) += tgt.vz;
    *(s32 *)(arg0 + 0xD8) += tgt.vx;
    *(s32 *)(arg0 + 0xE0) += tgt.vz;
    *(s32 *)(arg0 + 0xB8) += tgt.vx;
    *(s32 *)(arg0 + 0xC0) += tgt.vz;
    *(s32 *)(arg0 + 0x104) = 0;
    *(s32 *)(arg0 + 0x108) = 0;
    *(s32 *)(arg0 + 0x10C) = 0;
    *(s32 *)(arg0 + 0x134) = 0;
    *(s32 *)(arg0 + 0x138) = 0;
    *(s32 *)(arg0 + 0x13C) = 0;
    r1 = func_80021424(arg0, **(u16 **)(arg0 + 0x50), arg0 + 0x5E);
    r2 = func_80021424(arg0, *(u16 *)((s32)r1 + 0x3A), arg0 + 0x5E);
    func_80021A98(idx, r2, *(s16 *)(arg0 + 0x5E));
    func_80032854(*(s16 *)(arg0 + 4), 0x30, (s32 *)(arg0 + 0xF4), 0);
}
