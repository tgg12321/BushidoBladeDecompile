// func_80040CB8 — candidate at grind session s2 (2026-07-30, modality: structural)
//
// Honest pure-C floor: `sandbox func_80040CB8 --disable all` == 0
//   (s1 inherited 17 -> left 13; s2 measured 13 -> 6 -> 0)
// build_insns == 36 == target_insns. Zero regfix/asmfix rules, zero register
// pins, zero inline asm. Byte-identical to asm/funcs/func_80040CB8.s.
//
// WHY THE SHAPE IS WHAT IT IS (full derivation in hypotheses.md H6-H10)
//
// The whole function is 36 instructions that s1 already reproduced
// opcode-for-opcode; every residual was a register NAME plus prologue order.
// s2 found the governing mechanism: flow.c computes REG_N_REFS as
// `reg_n_refs[regno] += loop_depth` (flow.c:2081/2329/2515/2725), and
// loop_depth is derived purely from NOTE_INSN_LOOP_BEG/END notes. Since MIPS
// defines no REG_ALLOC_ORDER, global.c:find_reg is ascending first-fit and each
// variable's hard register is decided solely by its rank under
// allocno_compare = floor_log2(n_refs) * n_refs / live_length. Target's
// register assignment is ONLY reachable when the loop body sits inside loop
// notes: with depth-1 (plain goto loop) refs the GCC-generated id copy scores
// 1.00 against the 0x90C cursor's 1.50 and loses $v1; with depth-2 refs it
// scores 4.00 against 3.82 and wins it, and every other variable falls into
// place behind them. That 4.00-vs-3.82 margin is the entire function.
//
// But a GCC-RECOGNISED loop cannot be used: loop.c strength reduction invents a
// third induction pointer at arg0+0x8B8 for the -0x57..-0x4C displacement
// cluster (38 insns, score 25 — s1 do-while probe, s2 for-loop probe p2).
// The resolution is to have the notes WITHOUT loop.c accepting the region:
// initialising `ent` between NOTE_INSN_LOOP_BEG and the `loop:` label makes
// scan_loop's scan_start a non-CODE_LABEL insn, so loop.c prints
// "Loop from 31 to 112 is phony" and returns before biv/giv analysis. The
// notes — and therefore the ref weighting — survive.
//
// The first wrapper exists for prologue ORDER, not weighting: cc1's first-pass
// scheduler treats a loop note as a hard barrier (sched.c:2072, "no
// instructions are scheduled across it. Otherwise, the reg_n_refs info ...
// would become incorrect"), and without such a barrier it sinks the three
// single-set constant loads below the link/tbl cursor initialisers (score 6,
// tmp/grind/func_80040CB8/s2/p4.txt). Its BEG note pins `none` and its END
// note pins `link`, reproducing target's prologue order exactly.
//
// The two wraps are SEQUENTIAL, not nested, so prerequisite 3 of
// .claude/rules/do-while-zero-exception.md (nested-wrap justification) does not
// apply; prerequisites 1 (inline FAKE annotation at the construct site) and 2
// (natural geometry tried first) are satisfied and the measurements for each
// are cited inline below.
//
// `id` is declared FIRST so its pseudo number is lower than the 0x90C cursor's:
// the two tie at priority 4.00 and allocno_compare breaks ties on the lower
// allocno index, which is what actually hands $v1 to the copy and $a1 to the
// cursor.
/* kengo:MED  |  my_rob/rob_calc_2d_position  |  93i */
extern s16 D_80094B9E[];
void func_80040CB8(void *arg0) {
    s16 id;
    s8 *slot = (s8 *)arg0 + 0x8B4;
    s32 i = 0;
    // FAKE (none/kind/one): the three loop-invariant constants must be held in
    // registers across the loop, as target holds them in $t4/$t3/$t2. The
    // goto-loop body carries no LICM (its loop region is rejected as phony), so
    // writing -1/3/1 as literals cannot reproduce them: measured 35 insns /
    // score 23 in grind s1 (rejected/literal-constants-no-licm-in-goto-form.c).
    s32 none;
    s32 kind;
    s32 one;
    s32 link;
    s16 *tbl;
    s32 ent;

    // FAKE: wrap emits NOTE_INSN_LOOP_BEG/END around the three constant loads.
    // Both notes act as cc1 first-pass-scheduler barriers, which keeps the
    // three single-set constant loads ahead of the link/tbl cursor
    // initialisers instead of being sunk below them (target's prologue order).
    // Natural geometry was tried first: with plain declaration order the
    // scheduler sinks all three (measured score 6, s2/p4.txt).
    do {
        none = -1;
        kind = 3;
        one = 1;
    } while (0);
    link = (s32)arg0 + 0x94;
    tbl = D_80094B9E;
    // FAKE: wrap emits loop notes around the body, so flow.c weights every
    // reference inside it by loop_depth 2. That weighting is what seats the
    // id copy in $v1 and the 0x90C cursor in $a1 (and the rest on target);
    // without it the id copy loses its allocno-priority race and the whole
    // register assignment rotates (measured score 13, s1/build4.txt).
    // `ent` is initialised INSIDE the region on purpose: that makes the region
    // start on a non-label insn, so loop.c rejects it as phony and its
    // strength reduction cannot invent a third induction pointer for the
    // -0x57..-0x4C displacement cluster (measured 38 insns / score 25 with a
    // real for-loop, s2/p2.txt).
    do {
        ent = (s32)arg0 + 0x90C;
    loop:
        id = *tbl;
        if (id != none) {
            *(s16 *)(ent - 0x56) = id;
            *slot = kind;
            *(s8 *)(ent - 0x57) = 0;
            *(s16 *)(ent - 0x50) = 0;
            *(s32 *)(ent - 0x4C) = link;
            *(s16 *)(ent - 0x52) = one;
            *(s16 *)(ent - 0x4E) = 0;
            {
                u16 w = *(u16 *)((s32)arg0 + 0x16);
                slot += 0x68;
                *(s32 *)ent = 0;
                *(s16 *)(ent - 0x54) = w;
                ent += 0x68;
            }
        }
        link += 0x68;
        i++;
        tbl = (s16 *)((s32)tbl + 0xA);
        if (i < 0x12) goto loop;
    } while (0);
    *(s16 *)((s32)slot + 2) = -1;
}
