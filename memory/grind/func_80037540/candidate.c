/* func_80037540 — BEST FORM (byte-correct, sandbox --disable all == 0)
 *
 * STATUS: byte-correct but NOT cleared, and NOT the answer. This is the form on
 * main. It carries the flagged unwritten-tail frame reservation (sp[6]/sp[7]
 * never written). Do NOT treat as COMPLETED-C. Do NOT commit it as done.
 *
 * ############################################################################
 * # s3 (2026-07-13): THE SEARCH SPACE IS CLOSED **AGAIN**. s5's "the space is  #
 * # open" claim is REFUTED — not by argument, but by ~80 measurements. The     #
 * # phantom-slot mechanism s5 found is REAL, is now fully characterized, and   #
 * # is PROVABLY UNREACHABLE in this function. Do not re-open it.               #
 * ############################################################################
 *
 * THE FRAME EQUATION (unchanged, still true — mips.c:compute_frame_size):
 *     frame = ALIGN8(get_frame_size) + ALIGN8(args=16) + ALIGN8(gp=24)
 *     => frame 0x48  <=>  get_frame_size in [25,32].
 *   The target writes exactly 24 bytes (six `sw`, 0x10..0x24). So 1-8 declared-
 *   but-unwritten bytes are REQUIRED. There are only three ways to get them:
 *     (a) WRITE the extra bytes    -> emits stores the target lacks (+2, measured)
 *     (b) an UNWRITTEN TAIL        -> sp[7]/sp[8]; scores 0; FORBIDDEN family
 *     (c) a PHANTOM SLOT           -> live locals, zero stores          [s5's hope]
 *
 * WHAT s3 SETTLED — branch (c) IS UNREACHABLE HERE.
 *
 * THE MECHANISM (gdb backtrace on assign_stack_local inside cc1 — no longer a
 * guess). A phantom slot is a GCC 2.7.2 STALE-REF BUG, not a feature:
 *   1. `flow` (life analysis) records reg_n_refs[P] > 0 for a pseudo P.
 *   2. `combine` — which runs AFTER flow — deletes every insn referencing P,
 *      WITHOUT decrementing reg_n_refs.
 *   3. `regclass` now has no cost data for P, so its preferred class defaults to
 *      **ST_REGS** (degenerate — holds no integer regs).  [tslLineG5Init: the
 *      lreg dump literally says "Register 92 ... ST_REGS or none."]
 *   4. local/global alloc therefore cannot give P a hard register.
 *   5. `reload` -> alter_reg() (reload1.c:658) sees reg_renumber<0 && reg_n_refs>0
 *      && no equiv -> assign_stack_local() -> frame_offset grows.
 *   6. No insn references P, so NO load or store is ever emitted. -> PHANTOM.
 *
 * THE ENTRY CONDITION — and why it cannot be met here.
 *   Step 2 only fires for ONE combine path: eliminating a redundant HImode->SImode
 *   sign-extension (the ashift/ashiftrt pair on a paradoxical subreg). Combine's
 *   OTHER folds decrement refs correctly and produce NO phantom — measured: a
 *   two-stage shift (`idx*2` then *4) folds to a single `sll $2,$2,3`, emits the
 *   IDENTICAL 41 insns, and still gives vars=24.
 *   Combine can only prove the extension redundant when the narrow value is ALREADY
 *   sign-extended in its register — i.e. it came from a NARROW LOAD (`lh`/`lb`,
 *   which extend on load). Every other way a narrow value can enter a function was
 *   measured and produces NO phantom:
 *       s16/u16/s8/u8 PARAM      -> promoted subreg, no extension pseudo   (44 variants)
 *       s16 CALL RETURN          -> promoted subreg, no extension pseudo
 *       truncated ARITHMETIC     -> extension NOT redundant; emits sll/sra
 *       lhu / lbu (zero-extend)  -> no phantom
 *   => a phantom slot REQUIRES at least one `lh`/`lb` in the emitted function.
 *
 *   **func_80037540's target contains SEVEN loads, ALL `lw`. Zero narrow loads.**
 *   And there is no room to add one: the target is 43 insns and the tail-free
 *   `s32 sp[6]` form already emits 43. Therefore branch (c) is closed.
 *
 * IN-BINARY CORROBORATION (the necessity condition holds across all of BB2).
 *   Of the 28 census functions that reserve untouched locals, 24 contain a narrow
 *   load in their target asm. The 4 that do not are ALL non-witnesses:
 *       func_800520B8, func_8006BD28 -> AUTHORIZED CANONICAL-ASM (not compiled C)
 *       func_800644FC  -> INCOMPLETE, 13 regfix rules; its "phantom" is a cheat:
 *                         `s32 dummy_pad; __asm__ volatile("" : "=m"(dummy_pad));`
 *       file_LoadSectors (=func_800165F8, src/ings.c:163) -> INCOMPLETE, IN QUEUE;
 *                         ships `s32 _pad[2];` — a dead unwritten pad, i.e. the very
 *                         family under dispute.
 *   => NOT ONE cheat-free compiled-C phantom slot in this binary lacks a narrow load.
 *
 * CONSEQUENCE: s4's trichotomy is RESTORED, with its missing fourth branch now
 * explicitly measured and killed. This function is NOT a search problem. It is the
 * POLICY question it was already held on: the target's own prologue reserves 25-32
 * bytes of locals while the target writes 24, so the ORIGINAL SOURCE declared a
 * locals object strictly larger than what it wrote. Nothing an agent can write
 * changes that. Do NOT re-grind branch (c); do NOT re-litigate the ruling.
 *
 * MEASURED LADDER (unchanged, all sandbox --disable all):
 *     s32 sp[6]                 (24B, tail-free)          -> 15   [43 insns, only sp offsets differ]
 *     s32 argv[6]; s32 idx[1]   (scalar-promoted -> 24B)  -> 15
 *     s32 argv[6]; s32 idx[2]   (32B, every byte written) ->  2   [PROVEN MIN of branch (a)]
 *     s32 sp[7] / s32 sp[8]     (28B/32B, unwritten tail) ->  0   [branch (b) — this file]
 *
 * DO NOT RE-TRY (all measured, ~80 compiles; tmp/grind/func_80037540/s3/):
 *   narrow params (any/all of the 5, s16/u16/s8/u8) · narrow index locals ·
 *   s16-returning callee · two-stage shifts / word-index / byte-index spellings ·
 *   pointer-arithmetic and CamRec* reshapes · long long index · walking pointer ·
 *   grafted HImode triggers (they DO give vars=32/frame 0x48 — but only by emitting
 *   lh/nor/and/andi/branches the target does not have).
 */
extern s32 func_800392B8(void);
extern void marionation_camera_Init_80037468(s32, s32 *, s32);

void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 sp[8];
    s32 v0;

    v0 = func_80036EA8(6, a2);
    sp[0] = (s32)&SpecialCam + v0 * 8;
    sp[1] = a3;
    sp[2] = a0;
    sp[3] = a1;
    v0 = func_80036EA8(6, 2);
    sp[4] = (s32)&SpecialCam + v0 * 8;
    sp[5] = a4;
    v0 = func_800392B8();
    marionation_camera_Init_80037468(6, sp, v0 + 0x7FC);
}
