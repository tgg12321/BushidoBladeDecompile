/* [s27 2026-09-05 - rederive modality.  SPINE CHANGE: candidate.c is now the "Pt2" body - the T1
 * statement set with `p10 = *(s32 *)(outer + 0x10);` RELOCATED from the top of the function to the
 * slot immediately above the `D_800A3478 = outer + 0x18;` store.  Measured on today's HEAD chassis:
 * score 2, build_insns 66, target_insns 66.  The engine score is unchanged at 2, but this body is
 * strictly closer than T1 on every other axis and it CHANGES THE RESIDUAL COMPLETELY.
 *
 * WHAT THIS BODY IS.  Its object is 66 instructions - target's exact count - and it is
 * byte-identical to target at EVERY slot except two:
 *
 *     slot   ours                 target
 *      11    lw a1,16(v1)         lw a1,16(v1)      <-- TARGET'S SLOT-11 SEAT, REACHED
 *      19    lw a0,16(v1)         lw a0,16(v1)
 *      21    lhu v0,0(a0)         lhu v0,0(a0)
 *      22    nop                  lw a0,16(v1)      <-- THE WHOLE RESIDUAL (1)
 *      23    sh v0,24(v1)         sh v0,24(v1)
 *      24    lhu a0,2(a1)         lhu a0,2(a0)      <-- THE WHOLE RESIDUAL (2)
 *      25..65  identical, instruction for instruction, including every nop
 *
 * For 26 sessions the framing was "the p10 load is stranded at slot 4 (T1) or slot 25 (the
 * below-store family) and the scheduler will not put it at slot 11".  THAT PROBLEM IS SOLVED.  The
 * p10 load sits at slot 11 in $a1, exactly where target puts it.  The entire remaining gap is that
 * CSE folds two source-level reads of `*(s32 *)(outer + 0x10)` into one, so the +2 halfword read
 * borrows the slot-11 pointer in $a1 instead of reloading it into $a0 at slot 22, and the freed
 * slot 22 degenerates to a load-delay nop.
 *
 * WHY THE RELOCATION WORKS (the s27 mechanism, do not re-derive it).  `pwsh tools/grinder/dump.ps1
 * func_80060A68` on the s5 three-inline-read body shows the third `lw ?,16(reg72)` (uid 66) carrying
 * LOG_LINKS `(insn_list 9 (insn_list 22 (insn_list 60 (nil))))` - a MEMORY dependence on uid 60, the
 * `sw ?,D_800A3478` gp store - while the other two 0x10 loads (uid 46, uid 53) carry only
 * `(insn_list 9 (insn_list 22 (nil)))`.  The gp store is a symbol-based address and the 0x10 load is
 * a pseudo-based one, so memrefs_conflict_p cannot disambiguate them (sched.c:697-705) and the
 * dependence is created unconditionally.  Any 0x10 read placed BELOW the D_800A3478 store therefore
 * inherits that dep and cannot be hoisted past it; any 0x10 read placed ABOVE it has deps {9, 22}
 * only and IS hoistable to slot 11.  T1 put the read at the very top of the function, where its LUID
 * pulls it into the slot-4 load-delay nop; the s5/s17 inline bodies put it below the gp store, where
 * the dep strands it at slot 26-32.  The window between copy 3's store and the D_800A3478 store is
 * the only region that produces the slot-11 seat, and s27 is the first session to test it.
 *
 * THE FULL POSITION SWEEP (8 bodies, zero FAKE constructs, today's HEAD chassis).  `p10 = *(s32 *)
 * (outer + 0x10);` at every statement boundary from the Z0 store to the 0x1A store:
 *     after the Z0 store   (X4)   score 5   67 insns   load at slot 25
 *     after copy 1         (Pc1)  score 5   67 insns   load at slot 25
 *     after copy 2         (Pc2)  score 5   67 insns   load at slot 25
 *     after copy 3         (Pc3)  score 2   66 insns   SLOT 11; cse merges p10 with the +0 read
 *     after the 0x18 store (P18)  score 2   66 insns   SLOT 11; cse merges p10 with the +2 read
 *     after the +2 read    (Pt2)  score 2   66 insns   SLOT 11; cse merges p10 with the +2 read
 *     after the gp store   (Pgp)  score 3   66 insns   THREE distinct loads, p10 stranded at slot 26
 *     after the 0x1A store (P1A)  score 3   66 insns   THREE distinct loads, p10 stranded at slot 26
 * Pt2 and P18 produce byte-identical objects.  Pgp and P1A produce byte-identical objects, and their
 * ONLY divergence from target is that same one load: three distinct 0x10 loads, everything else
 * identical, the third one at slot 26 in $v0 instead of slot 11 in $a1.  So the two halves of the
 * answer are each one probe away, in two different bodies:
 *     Pt2/P18 : right seat, right position, wrong number of loads (2 where target has 3)
 *     Pgp/P1A : right number of loads, wrong seat and position for the third
 *
 * THE MERGE IS CSE, CONFIRMED IN THE DUMPS, NOT INFERRED.  Counting
 * `(mem:SI (plus:SI (reg/v:SI 72) (const_int 16)))` per pass on the Pt2 body:
 *     .rtl 3   .jump 3   .cse 2   .loop 2   .cse2 2   .combine 2   .sched 2
 * i.e. all three source reads survive into cse and cse folds two of them.  This reproduces s20's
 * class kill at cse.c:1701 (only a STORE or a CALL separating two identical memory reads defeats
 * the fold) on the new spine.
 *
 * WHY A SECOND SEPARATOR IS NOT AVAILABLE IN THAT WINDOW (s27's negative result).  The three 0x10
 * reads are the +0 read (A, inline in the 0x18 store), the +2 read (B, inline in temp2's
 * initialiser) and the p10 read (C).  All three must sit above the D_800A3478 store to get deps
 * {9, 22}; the only store in that window is the 0x18 halfword store, which separates exactly ONE
 * adjacent pair.  All three orderings of {A, B, C} were measured (Pc3, P18, Pt2) and each merges the
 * one pair the 0x18 store does not separate.  Both candidate second separators were measured and
 * both fail:
 *   - copy 3's store, split out through a named intermediate and placed inside the window:
 *     S1 (below B) score 11/66, S4 (below C) score 8/67, S5 (above A) score 5/67 - the copy block
 *     shatters.  Moving copy 3's WHOLE statement into the window is worse still: R1 11/67,
 *     R3 8/66, R4 10/67.
 *   - the D_800A3478 gp store itself: that is Pgp/P1A, and using it as the separator is exactly what
 *     hands the read the dep that strands it.  The separator and the dep are the same insn.
 *   - hoisting the 0x1A store above the gp store to serve as the separator (R2) flips their emission
 *     order (they are mutually memory-dependent, symbol vs pseudo again): 6/66.
 *   - deleting temp2 and inlining the +2 read into the 0x1A store below the gp store (U1 7/67,
 *     U3 8/68, U4 12/68) moves the +2 read below the gp store, which is the wrong side.
 *
 * KILL RE-AUDIT (mandated; floor had been flat 3 sessions).  fake_ablate is a no-op - this body and
 * every probe this session carry ZERO FAKE constructs.  Seven banked forms were re-measured on
 * today's HEAD chassis and every one reproduces its recorded score digit for digit
 * (s5-three-fresh-inline-reads 3/66, no-p10-local-inline-4-read-late 3/66, s17-A2 3/66,
 * s18-U2 5/66, s5-struct-typed-inner-reads 6/66, no-p10-local-inline-4-read-early 8/65,
 * s5-v2-named-address-locals 5/67).  No banked kill is VOID under the current scope.
 *
 * CONSTRUCT INVENTORY.  This body carries ZERO FAKE constructs and re-declares NONE of the five
 * banned constructs: `p10` is written exactly once and read exactly once, `temp2` is written exactly
 * once and read exactly once, no local carries two values, and no local is written twice.  The
 * dispatch call keeps the s23 `()` spelling.
 *
 * DISPOSITION.  ACTIVE.  `progress`; the floor is unchanged at 2 but the residual is now two
 * instructions in one slot pair and is a CSE-fold question, not a scheduler-readiness question.
 */
void func_80060A68(void) {
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    s32 outer;
    s32 idx;
    u16 temp2;
    s32 p10;
    u16 temp_a1;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);
    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    p10 = *(s32 *)(outer + 0x10);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(p10 + 4);
    idx = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(void)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))();
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
