/* func_80078654 — BEST FORM as of grind session 1 (2026-08-13, modality: recon).
 *
 * Honest pure-C sandbox floor: 19  (baseline at session start was 23).
 * Instruction count now EXACTLY matches target (116 == 116).
 * Zero inline asm: the session-start body carried a forbidden
 * INLINE_MOVE_ALIASING construct
 *     __asm__ volatile("move %0, %1" : "=r"(v_copy) : "r"(v));
 * which has been REMOVED and replaced by the natural short-typed clamp
 * below. That single change dropped the floor 23 -> 19 AND fixed the
 * instruction count (115 -> 116), so the cheat was not merely inert, it was
 * actively worse than the honest form.
 *
 * RESIDUAL (all 19 points of it): a single 2-way callee-save allocation
 * inversion. Target puts arg0 in $s1 and var_s0 in $s0; our build puts arg0
 * in $s0 and var_s0 in $s1. Nothing else differs — see evidence.md.
 *
 * SESSION 2 (structural) left this form UNCHANGED and still best at 19: every
 * structural partition variant measured worse or equal (the base/walk-pointer
 * merge scores 22 — rejected/base-merge-walk-pointer-pri-2448.c), and the whole
 * global.c decision procedure is now read and closed (evidence.md §SESSION 2).
 * It IS applied to src/text1b_b.c as of the end of session 2.
 *
 * SESSION 3 (structural) ALSO left this form unchanged and still best at 19.
 * It measured the ledger's F4 duplication axis on both shapes this CFG admits
 * and KILLED it: jump2's find_cross_jump re-merges neither (walk-init into both
 * arms = +2 insns / sandbox 25; whole loop into both arms = +39 insns / sandbox
 * 58), and the loop body holds 6 arg0 references against 4 walk references, so
 * duplication raises the PARAMETER's priority 1.5x faster than the walk
 * pointer's (measured 5671 vs 2125 at k=2). See evidence.md SESSION 3.
 *
 * SESSION 4 (permuter) ALSO left this form unchanged and still best at 19. It
 * built a clean single-function permuter rig (tmp/grind/func_80078654/s4/mkws.sh:
 * target.o from asm/funcs/func_80078654.s + the r3000-ified prelude, so the
 * function sits at offset 0 and the score is the real weighted diff; validated
 * base score 108 whose objdump diff prints exactly the twelve known $s0/$s1
 * lines) and ran two campaigns totalling 89,084 iterations with ZERO
 * score-improving finds � chassis A random from this base (best 108 == base,
 * both saved finds semantics-changing) and chassis B seeded from the s2 H4
 * base/walk-pointer merge (base 331, best 191, strictly worse basin). The
 * permuter axis is closed; see evidence.md SESSION 4.
 *
 * SESSION 5 (permuter) ALSO left this form unchanged and still best at 19. It ran
 * the ledger's directed-permutation probe: a PERM_* cross-product over six chosen
 * axes (three walk-initialiser partitions, three block-A shapes incl. the
 * `s32 *ot = &arg0[5]` arg0-lowering sub-pointer, four loop-head read shapes incl.
 * the double-read ref-lift, a PERM_LINESWAP over the increment pair, four loop-test
 * spellings, two PERM_RANDOMIZE regions) in tmp/grind/func_80078654/s5/ws —
 * 39,950 iterations, base 108, ZERO score-improving finds, both equal-score finds
 * semantics-CHANGING in exactly s4's way (they clobber var_s0 inside the loop).
 * It also killed two never-before-built chassis on their allocno tables alone with
 * the new one-call instrument tmp/grind/func_80078654/s5/eval.sh: the INDEX-walk
 * form (strength reduction => 120 insns, walk pri 1098 -> 851) and the SUB-POINTER
 * arg0-lowering form (arg0 13 -> 6 refs / pri 3979 -> 1212 but the walk pointer is
 * STILL below it at 1086, and the extracted pointer takes $s0 itself and forces a
 * fourth callee-save). Sharpest bound to date: arg0 must reach <= 5 references
 * before the walk pointer wins the sort. And note m2c is NOT installed in this tree
 * (ModuleNotFoundError) — the rederive probe needs it installed first.
 * See evidence.md SESSION 5.
 *
 * SESSION 6 (forensics) ALSO left this form unchanged and still best at 19. It
 * built the full RTL dump set (tmp/grind/func_80078654/s6/dump.sh -> in.c.rtl
 * .jump .cse .loop .cse2 .flow .combine .sched .lreg .greg .sched2 .jump2 .dbr)
 * and produced the ledger's long-outstanding PER-INSN PROVENANCE TABLE: pseudo
 * 72 (arg0) is mentioned in exactly 13 insns (uids 4, 118, 129, 142, 164, 169,
 * 173, 203, 214, 227, 249, 257, 261) and pseudo 73 (walk) in exactly 4 insns
 * (uids 47, 185, 254, 265; uid 254 carries two mentions, hence nrefs 5), and the
 * IDENTICAL uid sets appear at every stage from .rtl through .sched. No pass
 * creates or destroys a reference between RTL expansion and register allocation,
 * and there are no reload-generated address reloads at all — so the
 * "some reference was created after RA" frontier item is KILLED.
 * Three further results, all new: (a) reg_n_refs is frozen at flow_analysis
 * (toplev.c:2983) and never recomputed before global_alloc (3080), so a genuine
 * "free reference" window [flow, global_alloc) exists — but its measured yield
 * here is zero (split-increment: allocno table byte-identical; nx-chain: +0 refs,
 * +1 insn) or byte-materializing (double read: +1 ref for +2 insns).
 * (b) allocno_compare's tie-break is `return *v1 - *v2`, lower allocno index
 * first, and the parameter's pseudo is always created before any local's — so the
 * flip condition is STRICT and the walk pointer needs >= 13 references (+8).
 * (c) The find_reg pass-0 seeding route is killed by the target bytes: the target
 * mentions s0/s1/s2 seven/fifteen/five times and s3-s7 zero times, so there is no
 * fourth callee-save-resident value for local_alloc to have seeded.
 * Net: in ANY compile emitting the target bytes, reg_n_refs(arg0) >= 13, so the
 * original must have carried >= 8 walk-pointer references that died between flow
 * and global_alloc. See evidence.md SESSION 6.
 *
 * SESSION 7 (forensics) ALSO left this form unchanged and still best at 19
 * (sandbox re-measured this session with the body applied: score 19, 116 ==
 * 116, rules_dropped 6). It closed the s6 frontier's named mechanism and one
 * previously-unknown sibling:
 *   (a) local-alloc.c's optimize_reg_copy_1/2 reference TRANSFER is KILLED.
 *       The RTL local_alloc consumes holds exactly five reg<-reg copies and
 *       only ONE with a pseudo destination (uid 4, reg72 <- a0, the parameter
 *       home); none targets pseudo 73. Every pseudo's post-local_alloc
 *       ALLOCDBG nrefs equals its note-free mention count in .combine/.lreg
 *       exactly, so the transfer fires zero times. The C-level round-trip
 *       alias that would create an eligible copy is deleted by cse
 *       (rejected/alias-writeback-copy-deleted-by-cse-zero-ref-transfer.c:
 *       0 copies at .cse, allocno table byte-identical, 116 insns). Generally:
 *       a copy survives cse only as an EMITTED `move`, and the target's twelve
 *       moves have no $s0 destination and no $s1 source, so neither direction
 *       of the transfer can exist in a compile emitting the target bytes.
 *   (b) Allocno SHARING (global.c:450 sums reg_n_refs across pseudos sharing
 *       an allocno — a genuinely byte-free priority lift) is KILLED: the only
 *       producer of regs_may_share is loop.c:1659, gated on m->partial, which
 *       is set only for the const0 + STRICT_LOW_PART narrow-load idiom
 *       (loop.c:838-849) that an SImode pointer can never satisfy — and this
 *       body has zero NOTE_INSN_LOOP_BEG notes, so loop.c never runs on it.
 * Net: the reference counts are pinned from expansion to RA (s6),
 * un-transferable at local_alloc (s7) and un-shareable at global_alloc (s7).
 * Only a different DECOMPOSITION (rederive) can move them. New cheap
 * pre-screen for any future candidate: tmp/grind/func_80078654/s7/dump.sh
 * then s7/mentions.py — `mentions in in.c.combine` == `reg_n_refs at RA` is a
 * measured identity for this function, so a form can be rejected on its
 * reference census before it is ever assembled.
 *
 * SESSION 8 (rederive) ALSO left this form unchanged and still best at 19
 * (sandbox re-measured with the body applied: score 19, 116 == 116,
 * rules_dropped 6). It executed the mandated re-derivation BY HAND from
 * asm/funcs/func_80078654.s (m2c is still not installed) and accounted for
 * all 116 target instructions; the derivation converges on exactly this
 * body — there is no second reading of these bytes. The one structural
 * freedom it exposes, giving the single gp-load of D_800A3610 its own named
 * local, is codegen-identical (116 insns, same 38 diff lines, allocno table
 * identical up to renumbering; rejected/rederive-explicit-base-local-
 * byte-identical.c). It then measured the last two unbuilt partitions:
 *   (a) block-scoped PURE ALIASES of the parameter (`s32 *p = arg0;`), the
 *       only split that keeps the target's 0xC/0x14 displacements — the
 *       alias survives cse (unlike s7's round-trip alias) and becomes a
 *       FOURTH call-crossing pseudo (7 refs / 47 live / pri 2978) that takes
 *       $s0; 119 insns, frame 0x60 vs 0x58, 47 diff lines.
 *   (b) the JOINT quadrant of the s3 2-D frontier (alias split + H4 base
 *       merge together): both pointers land at an EXACT tie, 8 refs / 99
 *       live / pri 2424, and s6's strict tie-break gives $s0-precedence to
 *       the parameter's lower allocno index; 119 insns, 53 diff lines.
 * Net (the FORCED-DECOMPOSITION THEOREM, evidence.md SESSION 8): the target
 * has exactly three callee-save-resident values, every one of the twelve
 * buffer accesses must be a direct dereference of a pseudo holding arg0's
 * value (a derived pointer emits displacement 0 instead), and every such
 * pseudo carrying a proper subset is live across a call and costs a fourth
 * callee-save — so reg_n_refs(arg0) = 13 / pri 3979 in every conforming
 * compile, against a walk-pointer ceiling of 8 refs / pri 2448. The rederive
 * modality is CLOSED. Endgame gate 1 is pre-measured for the escalation:
 * scan_hand_coded --single func_80078654 = tier LOW, score 0/8.
 *
 * SESSION 9 (escalation) ALSO left this form unchanged and still best at 19
 * (sandbox re-measured this session with the body applied to src/text1b_b.c:
 * score 19, target_insns 116 == build_insns 116, rules_dropped 6). It closed
 * the last inductive premise of the s8 forced-decomposition theorem and then
 * reached the mandated disposition.
 *   (a) The RMW-WINDOW-ONLY split — the one un-built partition the ledger's
 *       frontier named — is KILLED. A block-scoped PURE alias of the parameter
 *       confined to the call-free tail RMW window (`p = arg0; p[5] = p[5] +
 *       0xC;` in each block, pure alias so the 0x14 displacement survives) is
 *       DELETED BY cse before it ever reaches register allocation: the allocno
 *       table is byte-identical to the base (pseudo 72 nrefs=13 / livelen 98 /
 *       pri 3979 -> $s0; pseudo 73 nrefs=5 / livelen 91 / pri 1098 -> $s1),
 *       116 insns, the same 38 diff lines. Zero references move off the
 *       parameter. Banked at rejected/rmw-window-alias-deleted-by-cse.c.
 *       Together with s8(a) — a CALL-CROSSING pure alias survives cse but
 *       becomes a fourth call-crossing pseudo (7 refs / pri 2978) that takes
 *       $s0 itself, 119 insns / frame 0x60 — the alias axis is closed at BOTH
 *       endpoints with no middle: cheap enough to avoid a callee-save means
 *       deleted before RA; surviving to RA means paying for one. Premise (3)
 *       of the forced-decomposition theorem is now deductive.
 *   (b) DISPOSITION. Both endgame-lock AND-gates FAIL. Gate 1:
 *       `scan_hand_coded.py --single func_80078654` = tier LOW, score 0/8, all
 *       eight signals absent ("no strong hand-coded indicators") — canonical
 *       asm refused. Gate 2: no in-hand SOTN-master precedent exists, and none
 *       can exist, because six modalities have failed to identify ANY closing
 *       construct for which a precedent could be sought. Per the owner's
 *       standing ruling 2026-07-27 (.claude/rules/endgame-lock-disposition.md)
 *       the both-gates-fail disposition is pre-decided: REFUSED /
 *       OWNER-ACCEPTED INCOMPLETE, filed at docs/grind/decisions.md
 *       (2026-08-13, func_80078654). src/text1b_b.c was REVERTED to HEAD so
 *       the six regfix rules keep holding the byte match and the full-build
 *       oracle stays green, per that ruling's disposition clause.
 *
 * NOTE FOR THE NEXT SESSION: HEAD does NOT carry this body — the s2/s3 ledger
 * commits are ledger-only, so src/text1b_b.c at HEAD still has the inherited
 * `s32 v;` + `__asm__ volatile("move %0, %1" ...)` form that scores 23. Apply
 * this file FIRST or every measurement you take will be against the stale base.
 *
 * Apply this body over src/text1b_b.c's func_80078654 (replacing the whole
 * function AND deleting the now-unused `s32 v;` local). NOTE: with these
 * edits the six existing regfix.txt rules for func_80078654 no longer line
 * up (they were written against the inline-asm body and its maspsx indices),
 * so a FULL build will not be SHA1-clean until the function reaches distance
 * 0 and those rules are retired. Grind with the sandbox, not the full build.
 */

void func_80078654(s32 *arg0) {
    S78654 s;
    s32 *var_s0;
    s32 zero;

    zero = 0;
    s.f = 2;
    s.cd_flag = 0;
    s.e = 0;
    s.g = 0;
    s.a = D_800A3610[0xF];
    s.h = 0;
    s.b = s.a + 0xC;
    var_s0 = D_800A3610 + 5;
    if (D_800A3608 >= 0xAAA) {
        if (D_800A3608 >= 0xB04) {
            s16 sv;
            s.cd_flag = 1;
            sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
            if (sv < 0) {
                sv = 0;
            }
            s.r = (s.g_ = (s.b_ = (u8) sv));
        }
        s.c = arg0[3];
        arg0[3] = func_8007352C(&s.a);
        SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
        AddPrim(D_800A374C + (s.f * 4), arg0[5]);
        arg0[5] = arg0[5] + 0xC;
    }
    s.cd_flag = 0;
    goto check;
loop:
    s.a = var_s0[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    s.c = arg0[3];
    arg0[3] = func_8007352C(&s.a);
    SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
    AddPrim(D_800A374C + (s.f * 4), arg0[5]);
    var_s0++;
    arg0[5] = arg0[5] + 0xC;
check:
    if (var_s0[1] != -1) goto loop;
}
