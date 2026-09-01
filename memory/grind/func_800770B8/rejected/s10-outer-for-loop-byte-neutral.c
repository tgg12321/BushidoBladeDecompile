/* candidate.c — func_800770B8 (src/text1b.c) — s3 structural, 2026-09-01
 * Honest floor THIS form, measured s3: sandbox --disable all = 9 (175/175 insns).
 * (s2 form re-measured 10 on today's chassis at the start of s3; s3 dropped it 10 -> 9
 * and closed the last insn-COUNT gap: ours is now 175 insns, exactly the target's.)
 * 100% ordinary C, zero cheats, zero annotations, no FAKE-annotated constructs.
 *
 * s3 addition over the s2 form (one edit, ordinary C, measured):
 *   Inner loop 2 (the arg2 bit-scan) is a `for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1))`
 *   instead of a preheader `a2 = 0;` plus a do-while with a trailing increment.
 *   This closes residual class D (the reorg.c delay-slot fill choice, PROVEN in s2):
 *   with the `for` shape reorg no longer steals the `ori` from the fall-through
 *   thread; it fills the beqz from the branch-target thread by copying
 *   `addiu $v0,$a2,1` and redirecting past it — exactly the target's rows 103-105
 *   and 113. Insn count 174 -> 175, score 10 -> 9.
 *   NOTE the a2 initialiser must stay a `for`-header init here; s2's H6 (a2 = 0 as the
 *   FIRST statement of the outer do-body) is still what fixes rows 30-34/37, and the
 *   `for` header supplies exactly that RTL position for the second inner loop.
 *
 * NOTE: applying this body also requires the two caller-side edits (see
 * tmp/grind/func_800770B8/s3/try.py): the prototype near the caller becomes
 * `s32 func_800770B8(s32, s32, s32);` and the call site passes
 * `(s32)&D_8009BD24` (was `(s32 *)&D_8009BD24`). Byte-neutral for the caller.
 *
 * Residual at floor 9 (3 classes; class D is CLOSED):
 *   A. prologue rows 7-12 (4 diffs): sched2 (post-RA!) ordering of
 *      {sw $ra, sw $s1, addiu $a1 0x1008, lui/lw D_800A374C, addiu $s1,$s0,0x58}.
 *      s3 correction: this is a SCHED2 question, not sched1 — the register-save
 *      insns do not exist until after reload. 12 statement orderings measured, all
 *      neutral-or-worse. See evidence.md s3.
 *   B. rows 35-36 (2 diffs): the 0x30/0x34 stores go through $s1 (p_old) in ours,
 *      through $v0 (the raw call-result pseudo) in target. Second-handle spellings
 *      KILLED three times (K1 s1, K4 s2, and every s3 restatement is byte-neutral).
 *   C. rows 62-64 (3 diffs): the p_6a/p_7e base `addu` ties its dest to the lw pseudo
 *      ($v0) in ours and to the sll pseudo ($v1) in target — a local-alloc
 *      dest-coalesce decision. Six address spellings measured; all neutral or worse.
 *
 * s4 (permuter modality) — candidate BODY UNCHANGED, still floor 9 on today's
 * chassis (re-measured 9 / 175 insns at session start). s4's finds are all
 * cheat-family and live in rejected/:
 *   - A bare `do { } while (0);` immediately before the `p_old` assign measures 7,
 *     and combined with `p_old` moved after the ClearOTagR call it measures 5.
 *     Dumps prove the mechanism is a mid-block NOTE_INSN_LOOP_BEG/END pair that
 *     stops sched2 interleaving the reload-emitted save stores with the first
 *     body insns — i.e. a scheduling barrier. NOT submittable.
 *   - Ordinary inner brace scopes do NOT reproduce it (block notes migrate to the
 *     top of the function); measured byte-neutral.
 *   - IMPORTANT for the next session: `p_old` moved after ClearOTagR measures 10
 *     UNFENCED but 5 FENCED. s3's "twelve orderings dead" is chassis-conditional.
 *
 * s5 (synthesis modality) — candidate BODY UNCHANGED, re-measured 9 (175/175) on
 * today's chassis. Two results reset the picture; full detail in evidence.md s5.
 *   - The honest-fence hunt is CLOSED. A real first-statement loop DOES anchor a
 *     NOTE_INSN_LOOP_BEG at the V1 fence position (dumped and read), but costs
 *     +11 insns (175 -> 186, score 41-42), and our count already equals the
 *     target's — so no real loop is on the path to 0. Decisively, the TARGET has
 *     no loop, label or branch in its prologue at all, so its contiguous save
 *     emission is NOT a note fence; the s4 do-while(0) is a coincidental route to
 *     the same order, not the original mechanism.
 *   - The residual is majority-RA, not scheduling: goal_from_tgt.py classify
 *     reports FIRST DIVERGENCE = RA ($v0->$v1 x4, $s1->$v0 x2, $v1->$v0 x1).
 *     Class B ($s1->$v0, i.e. residual B above) attributes uniquely to pseudo 75
 *     (p_old) and inverse.py global returns FORECLOSED: p_old crosses 4 calls and
 *     $v0 is call-used, so prune_preferences (global.c:897) strips the $v0
 *     preference before find_reg runs. No C spelling moving refs / live span /
 *     birth order / conflicts / preferences / calls-crossed can close it.
 *   - Class C (residual C above) is the one unspent typed-verdict axis:
 *     local_extract.py + inverse.py local. That is s6's first move.
 *
 * s6 (synthesis) - candidate BODY UNCHANGED, re-measured 9 / 175 insns on today's
 * chassis. Two durable corrections to the residual description above:
 *   1. Class C is NOT a local-alloc dest-coalesce decision. text1b.lreg insn 173 is
 *      (set (reg 110) (plus (reg 109) (reg 108))) with reg 109 = the lw of
 *      D_800A36A0 and reg 108 = the sll. Rows 60/61 are byte-identical to the
 *      target, so BOTH input pseudos already get the target's hard registers
 *      ($v0/$v1); in both builds the dest ties to operand 1 of addu %0,%1,%2. The
 *      whole class is the RTL plus's OPERAND ORDER, decided at expand/fold time.
 *      Int-domain spellings DO flip it (D2) but delete the target's second lw of
 *      D_800A36A0 at row 60; E4 keeps the re-read and flips the order but costs one
 *      insn (176) because 0x6A/0x7E fold onto the shift side. 13 more spellings
 *      measured s6 (19 total for this seat) - see hypotheses.md.
 *   2. Residual row 50 (addiu $2,$2,0 vs addiu $v0,$v0,%lo(D_800A35D0)) is a
 *      SCORING ARTIFACT: the object carries R_MIPS_HI16/LO16 against D_800A35D0
 *      with a zero addend and the scorer masks HI16 but not LO16. The honest
 *      residual is 8 real rows, not 9.
 *
 * s7 (solver) - candidate BODY UNCHANGED, re-measured 9 / 175 insns on today's
 * chassis. Two durable corrections to the residual description above:
 *   1. Class B is NOT a register-allocation question. The last unspent RA-layer
 *      mechanism (local-alloc's suggested-register pass, qty_phys_copy_sugg /
 *      qty_phys_sugg) is dead: pseudo 75 (p_old) forms no local-alloc quantity at
 *      all, and the whole function carries exactly one suggestion anywhere
 *      (blk0 qty0, copysugg=[$a0]). Class B is cse PSEUDO IDENTITY: cse.c
 *      make_regs_eqv makes the first pseudo the call result is copied into the
 *      quantity's qty_first_reg, and canon_reg never substitutes a hard reg, so
 *      all four post-call stores canonicalise onto ONE register. The target's
 *      2+2 $s1/$v0 split is REACHABLE - hoisting a `u8 *pp` to function scope,
 *      assigning it after the global + 0x4 stores and reusing it in the tail
 *      makes pp's regno_last_uid fall past the cse block end, pp becomes
 *      canonical, and text1b.cse then shows insns 68/71 on pseudo 75 and 77/80
 *      on pseudo 76 - but the copy that makes pp join the quantity is a real
 *      insn: 176 / score 12. Store-base SPELLING is byte-neutral (9/175 for every
 *      p_old-vs-global permutation).
 *   2. s6's D3 address form is 175 insns on this chassis, not 174. There is no
 *      insn credit anywhere on record to pay for the class-B split copy.
 *
 * s8 (forensics) - candidate BODY UNCHANGED, re-measured 9 (175/175) at session
 * start and again at session end. s8 corrected and then closed the class-B
 * picture with instrumented-cc1 dumps:
 *   - The target's 2+2 store split costs ZERO extra insns (target asm rows 29-38:
 *     one copy `addu $s1,$v0,$zero` with the raw call result still live in $v0).
 *     s7's "+1 insn, structural" was a property of H10's spelling.
 *   - cse PASS 1 reproduces the split exactly when a NOTE_INSN_LOOP_END breaks the
 *     extended basic block; cse PASS 2 (after_loop=1, so the note is ignored)
 *     re-merges the block and canonicalises both pseudos onto one register, and
 *     flow.c deletes the dead copy. Note-based fences cannot buy class B.
 *   - Within one cse block a pseudo-to-pseudo copy always collapses, so no naming
 *     or ordering of the two pointers produces the split (7 forms, 170 or 175).
 *   - The make_regs_eqv promotion IS a free C-controllable lever (reusing p_old for
 *     the tail's D_800A36A0 re-read: 175 insns, not 170) but it collapses onto $s1
 *     and damages the tail.
 *
 * s9 (forensics) - candidate BODY UNCHANGED, re-measured 9 (175/175) at session
 * start and again at session end. s9 typed two of the three residual classes:
 *   - CLASS A IS FORECLOSED TO STATEMENT ORDER. The sched2 dump was finally read
 *     out (tmp/grind/func_800770B8/s9/d/text1b.sched2, block 0) and the 15
 *     prologue insns mapped 1:1 to both asm streams: rows 1-7 and 13-15 already
 *     match, so class A is exactly the 5-insn permutation ours 566,15,26,28,560
 *     vs target 560,566,28,26,15. `sw $ra` (UID 560) is released in the backward
 *     pass by the first jal and `sw $s1` (566) by `addu $s1,$s0,88`; the jal is
 *     emitted after that addu, so 560 is ALWAYS released first, and as the only
 *     function-unit insn in its priority-1 group it is promoted by
 *     schedule_select's potential-hazard rule (sched.c:2708-2721). tools/sched_solver
 *     reproduces block 0 exactly and an EXHAUSTIVE depth-1 sweep of all 3234 input
 *     atoms finds 0 that reach the target order; no luid / luid_move (statement
 *     move) atom even flips the 560/566 pair. Hoisting p_old past the ClearOTagR
 *     setup - the s4 lead - is simulated dead (gives 26,28,566,15,560).
 *   - CLASS B HAS NO UNTESTED MECHANISM LEFT. The reserved probe (a real branch
 *     between the copy and the 0x30/0x34 stores) measures 176 insns / score 16.
 *   - CLASS C's operand flip is now FREE in instructions (175, was 176): making the
 *     shift the pointer operand and the global the integer operand -
 *     `(s16 *)((u8 *)(t0 * 10) + (s32)D_800A36A0 + 0x6A)` - flips insn 173 because
 *     c-typeck.c:1988/2695 put the POINTER-typed side at operand 0 unconditionally.
 *     Rows 62-64 then reduce to a register-seat question (target: lw in $v0, sll in
 *     $v1; ours reversed). Whole-function score is 33, so it is not yet a win - the
 *     inner loop's addressing chain re-allocates around the new tree shape.
 *   - A hard-reg address is unreachable from C (calls.c:2039 / calls.c:2114).
 *   Full detail in evidence.md s8; s9 should work class C or class A.
 */
s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {
    u16 sp[2];
    s32 *p_old;
    s32 r;
    s16 t0;
    s16 a2;

    p_old = (s32 *)(arg0 + 0x58);
    sp[0] = 0;
    sp[1] = 0;
    ClearOTagR(D_800A374C, 0x1008);
    D_800A35D8 = arg0;
    snd_StopAll();
    func_8006E950(6, p_old);
    r = func_80076FF8(p_old);
    {
        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        *(s32 *)((u8 *)p_old + 4) = (s32)prev;
        *(s32 *)(D_800A36A0 + 0x30) = 0;
        *(s16 *)(D_800A36A0 + 0x34) = 0;
    }
    for (t0 = 0; t0 < 2; t0 = (s16)(t0 + 1)) {
        u8 *base = D_800A36A0;
        u8 *ptr;
        a2 = 0;
        ptr = (u8 *)((t0 * 2) + (s32)base);
        *(s16 *)(ptr + 0x10) = 0;
        *(s16 *)(ptr + 0x8) = 0;
        *(s16 *)(ptr + 0xC) = 0;
        *(s16 *)(ptr + 0x14) = 0;
        *(s16 *)(ptr + 0x3C) = 0;
        ptr = (u8 *)&D_800A35D0;
        ptr = (t0 * 4) + ptr;
        *(s16 *)(ptr + 2) = 0;
        *(s16 *)(ptr + 0) = 0;
        ptr = base + (t0 * 4);
        *(s16 *)(ptr + 0x42) = 0;
        *(s16 *)(ptr + 0x40) = 0;
        *(u8 *)(base + t0 + 0x68) = (u8)t0;
        {
            s16 *p_6a = (s16 *)(D_800A36A0 + (t0 * 10) + 0x6A);
            s16 *p_7e = (s16 *)(D_800A36A0 + (t0 * 10) + 0x7E);
            do {
                p_6a[a2] = -1;
                p_7e[a2] = 0;
                a2 = (s16)(a2 + 1);
            } while (a2 < 5);
        }
        a2 = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x5C) = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x60) = 5;
        for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) {
            s16 idx = (s16)(a2 + (t0 * 10));
            s32 mask = 1 << idx;
            (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] & 0xF2);
            if ((arg2 & mask) != 0) {
                (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] | 1);
                sp[t0] += 1;
            }
        }
    }
    {
        u8 *p = D_800A36A0;
        *(s32 *)(p + 0x20) = 0;
        *(s32 *)(p + 0x1C) = 0;
        if ((s16)sp[0] < (s16)sp[1]) {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[0] - 3);
        } else {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[1] - 3);
        }
    }
    if (*(u8 *)(D_800A36A0 + 0x64) >= 3) {
        *(u8 *)(D_800A36A0 + 0x64) = 2;
    }
    {
        u8 *q = D_800A36A0;
        *(s32 *)q = arg1;
        *(s8 *)(q + 0x65) = 0;
    }
    *(u8 *)(D_800A36A0 + 0x67) = 1;
    *(u8 *)(D_800A36A0 + 0x66) = (&D_8009BD21)[*(u8 *)(D_800A36A0 + 0x67) * 2];
    D_800A35DC = 1;
    return 1;
}
