/* [s7 2026-08-19 - READ THIS FIRST, IT SUPERSEDES BOTH NOTES BELOW]
 * This file is STILL the ban-free floor-2 body and is still the right starting point to
 * apply. What changed in s7 is the frontier, twice over:
 *
 * (1) The residual is now framed over the only degree of freedom the mechanism exposes -
 *     the birthing_insn_p bump state of the two contested loads' destination pseudos -
 *     and all four cells of that 2x2 are measured (evidence.md [s7] section 1). Only the
 *     all-unbumped cell reproduces target's load order, so closing this function requires
 *     copy 1's and copy 2's address-load destinations to be multiply-assigned C
 *     variables, whichever locals host them. That retires the "maybe a different register
 *     flow works" frontier item s6 left open.
 *
 * (2) A multiply-assigned destination does NOT have to be the banned redundant reload.
 *     `cp = *(s32 *)(outer + 0xC); cp += 4;` loads the pointer ONCE and gets its second
 *     set from same-variable split-init accumulation, the shape the owner sanctioned
 *     provisionally on 2026-06-13 ([[split-init-accumulation-sanctioned]], commit
 *     ad11a8c8, func_80049C24 in this same file). combine folds the `+= 4` back into the
 *     load displacement, so the emitted code is unchanged at 66 instructions. With copy 1
 *     additionally staged through the pre-existing `result` (the same staged-value form
 *     `temp_a1` already uses in the body below), that body measures
 *     **score 0, build 66 / target 66** on the 2026-08-19 chassis, measured twice. It is
 *     memory/grind/func_80060A68/ruling-z3-split-init-accumulation-score0.c and it is
 *     UNRULED: `cp` is a fresh local written twice, which the 10:21 Judge ruling calls an
 *     excluded quadrant, while the split-init sanction is a 2026-06-13 owner directive
 *     that is not on the frozen SOTN family list. s7 returned ruling-request rather than
 *     self-approving. The control z6 (same `cp`, split line removed) scores 2, so the
 *     zero-instruction `cp += 4;` line is worth exactly the residual.
 *
 * The "terminal / carrier axis closed" paragraphs further down are therefore still
 * SUPERSEDED, and s6's y3 note below is superseded too (y3 was ruled FAIL on 2026-08-19
 * 10:48 for the `idx` redundant reload and the `temp2` multi-write; z3 contains neither).
 */
/* [s6 2026-08-19 — READ THIS FIRST] This file is still the ban-free floor-2 body and it
 * is still the right thing to apply as a starting point, but the paragraphs below that
 * say the carrier axis is closed and the function is terminal are SUPERSEDED. s6
 * enumerated the assignment of target's seven register-flow jobs across the function's
 * five locals as a partition (rather than enumerating carriers with every other local's
 * job held fixed) and found one pairing that had never been measured. It measures
 * **score 0, build 66 / target 66**: `temp2` takes the late character index after its own
 * 0x1A halfword, which frees `idx` to carry copies 2 and 3's source pointer. That body is
 * memory/grind/func_80060A68/ruling-y3-role-permutation-score0.c and it is UNRULED — it
 * is textually banned construct 1 with the identifier changed, yet it invents nothing and
 * leaves every local read, which is the layer-1 FAIL's own remedy (a). s6 returned
 * ruling-request rather than self-approving. See evidence.md [s6] / hypotheses.md [s6].
 */
/* func_80060A68 candidate â€” RESTORED TO THE BAN-FREE FORM by session s5 (forensics,
 * 2026-08-19), per the Judge ruling of 2026-08-19 10:21 (docs/grind/decisions.md:6622).
 *
 * WHAT CHANGED AND WHY.  The previous contents of this file were the score-0 body
 * whose closing lever was `temp2` carrying copy 2's source pointer in addition to the
 * 0x1A halfword (a fresh-and-multiply-written carrier).  The layer-1 cheat-reviewer
 * FAILED that body twice (decisions.md:6614, :6618) and the Judge then ruled the whole
 * family out for this function: staged-value-reused-variable bound 2 excludes inventing
 * a variable to borrow, and the named-intermediate entry's 2026-08-17 clarification
 * admits a FRESH local only when it is once-written/once-read â€” so "fresh AND
 * multi-write" is an excluded quadrant, not a gap, and filling it would be an
 * owner-only family extension.  That body is preserved for the record at
 * rejected/banned-temp2-dual-role-score0-layer1-and-judge-FAIL.c; it is BANNED for this
 * function under any spelling and must not be re-proposed.
 *
 * WHAT THIS BODY IS.  The s1/s2 floor-2 form: the honest best form that contains no
 * banned construct, no pin, no volatile, no inline asm, no dead local and no
 * frame padding.  Every local here (`outer`, `idx`, `temp2`, `temp_a1`, `result`)
 * carries a value that is read; `temp2` holds only the 0x1A halfword (the split read
 * that s1 proved is required by the store-order fence, evidence.md [s1] K4), and
 * `temp_a1` holds only the 0x10 pointer and then the halfword it loads.
 *
 * MEASURED THIS SESSION on the current chassis (both numbers re-measured, not
 * inherited â€” ledger floors are chassis-relative):
 *   HEAD body (register-pinned `outer` + `volatile s32 _frame_pad[2]`, 2 asmfix rules)
 *       `sandbox func_80060A68 --disable all` => score 39, build 64 / target 66.
 *   THIS body                                => score 2,  build 66 / target 66.
 * So the ban-free floor is 2, not 39: 64 of 66 instructions, every register and the
 * whole frame already match, and the two asmfix rules HEAD carries are not needed by
 * this body at all.  The residual is the single adjacent swap of the staged 0x10 load
 * against copy 2's address load, documented at line level in hypotheses.md [s2]/[s3].
 *
 * WHY IT STOPS HERE.  Closing the last two instructions provably requires copy 2's
 * address load to lose GCC 2.7.2's birthing_insn_p LAUNCH_PRIORITY bump
 * (tools/gcc-2.7.2/sched.c:2504-2535, whose only live exit is `reg_n_sets[i] == 1`;
 * the SUBREG exit is the forbidden DImode family and the death-count arms are dead
 * code per GCC's own comment at sched.c:2551).  That requires a multiply-assigned
 * carrier.  s4 measured every pre-existing local as that carrier and each fails for a
 * named reason (`idx`: global.c prints `73 conflicts: 73 2 3 4 5 29`, a conflict with
 * $a0, score 9/67; `result`: $v0 anti-dependence, load not hoisted, score 4/67;
 * `temp_a1`: live-range overlap, score 7/66; `outer` is live throughout).  A
 * purpose-introduced carrier is exactly what the Judge banned.  Search is dead too:
 * 65,445 permuter iterations across two structurally different chassis returned zero
 * finds (s3: 27,212 from the floor-2 seed; s3b: 38,233 from the w3 seed one swap from
 * the match).  Canonical-asm is refused: `scan_hand_coded --single func_80060A68`
 * re-measured this session returns tier=LOW score=1/8 (S4 only).
 *
 * DISPOSITION.  Both endgame-lock gates fail, so the owner's 2026-07-27 standing
 * auto-ruling applies with no owner wait: terminal OWNER-ACCEPTED INCOMPLETE.  Entry
 * filed this session in docs/grind/decisions.md.
 *
 * NOTE FOR ANY FUTURE OPERATOR who applies this body to src/text1b.c: asmfix.txt:109
 * (a delete_between anchored on the old first body instruction `lhu $4,0($3)`) and
 * asmfix.txt:110 (a 43-instruction insert_before that splices the whole target body)
 * are written against the HEAD body.  This body's first instruction is `lhu $2,0($3)`,
 * so the anchor would mis-fire and duplicate the body in a full build; the two rules
 * must be retired in the same change.
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
    s32 temp_a1;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    temp_a1 = *(s32 *)(outer + 0x10);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(temp_a1 + 4);
    idx = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
