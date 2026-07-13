/* REJECTED — the PHANTOM-SLOT escape route (s5's "the search space is open").
 *
 * s5 (session 2) found that GCC 2.7.2 reserves frame bytes for ordinary live
 * locals with zero stores (a "phantom slot"), witnessed by tslLineG5Init — a
 * COMPLETED-C, cheat-free, sandbox-0 function with get_frame_size()==8 and
 * nothing touching those 8 bytes. It concluded s4's trichotomy was FALSE and
 * that func_80037540 had "real, un-searched space": find the live-code construct
 * that induces a 1-8 byte phantom on top of a tail-free `s32 sp[6]`.
 *
 * s3 ran that hunt to the end. The mechanism is REAL. It is UNREACHABLE here.
 *
 * ---------------------------------------------------------------------------
 * WHAT THE PHANTOM SLOT ACTUALLY IS (gdb backtrace on assign_stack_local in cc1):
 *
 *   flow records reg_n_refs[P] > 0
 *     -> combine (runs AFTER flow) deletes every insn referencing P, WITHOUT
 *        decrementing reg_n_refs                                  [the stale-ref bug]
 *     -> regclass has no cost data for P -> prefclass = ST_REGS (degenerate)
 *     -> local/global alloc cannot allocate P -> reg_renumber[P] < 0
 *     -> reload: alter_reg() (reload1.c:658) sees reg_renumber<0 && reg_n_refs>0
 *        && no equiv -> assign_stack_local() -> the frame grows
 *     -> nothing references P -> NO load, NO store is emitted     -> PHANTOM SLOT
 *
 * It is a compiler BUG, not a construct. You cannot "declare" one.
 *
 * ---------------------------------------------------------------------------
 * THE ENTRY CONDITION — and why func_80037540 cannot meet it:
 *
 * Step 2 fires for exactly ONE combine path: eliminating a redundant HImode->SImode
 * sign-extension (ashift/ashiftrt on a paradoxical subreg). Combine's other folds
 * decrement refs correctly and yield NO phantom — measured: the two-stage shift
 * `w = idx*2; &SpecialCam[w]` folds to a single `sll $2,$2,3`, emits the IDENTICAL
 * 41 insns as the baseline, and still gives vars=24.
 *
 * Combine can only prove the extension redundant when the narrow value is already
 * sign-extended in its register — i.e. it came from a NARROW LOAD (lh/lb extend on
 * load). Every other route a narrow value can take into a function was measured:
 *
 *     s16/u16/s8/u8 PARAMETER    -> promoted subreg; no extension pseudo   vars=24  (44 variants)
 *     s16 CALL RETURN            -> promoted subreg; no extension pseudo   vars=24
 *     truncated ARITHMETIC       -> extension NOT redundant; emits sll/sra  vars=24
 *     lhu / lbu (zero-extending) -> no phantom                              vars=24
 *     lh  LOAD  (sign-extending) -> PHANTOM                                 vars=32  <-- the only one
 *
 * => A phantom slot REQUIRES at least one `lh`/`lb` instruction in the function.
 *
 *    **func_80037540's target has SEVEN loads, ALL `lw`. Zero narrow loads.**
 *    And there is no room to add one: the target is 43 insns and the tail-free
 *    `s32 sp[6]` form below already emits 43 (s2: only the sp offsets differ).
 *
 * POSITIVE CONTROL (proving the negative is not a harness artifact): grafting the
 * faithful tslLineG5Init trigger into this function's exact shape DOES produce
 * vars=32 / frame 0x48 on top of a tail-free `s32 sp[6]` — at 56 insns instead of
 * 41, because it emits the lh/nor/and/andi/branches that carry the phantom. The
 * frame is reachable; it is not reachable for free.
 *
 * IN-BINARY CORROBORATION: of the 28 census functions reserving untouched locals,
 * 24 contain a narrow load. The 4 that do not are all non-witnesses — 2 are
 * AUTHORIZED CANONICAL-ASM (func_800520B8, func_8006BD28), and 2 are INCOMPLETE
 * queue items whose "phantom" is a hand-written cheat:
 *     func_800644FC    `s32 dummy_pad; __asm__ volatile("" : "=m"(dummy_pad));`  (+13 regfix rules)
 *     file_LoadSectors `s32 _pad[2];`  (src/ings.c:163 — the disputed family itself)
 * Not one cheat-free compiled-C phantom slot in BB2 lacks a narrow load.
 *
 * ---------------------------------------------------------------------------
 * VERDICT: branch (c) of the trichotomy is CLOSED. s4's trichotomy is RESTORED,
 * with its missing fourth branch now measured rather than overlooked. This
 * function is not a search problem — it is the policy question it was already
 * held on. Do NOT re-grind the phantom hunt.
 *
 * The body below is the tail-free `s32 sp[6]` form — the ONLY form with no
 * declared-but-unwritten byte. It scores 15 (43 insns, correct order, all 15
 * diffs are sp-relative offset shifts from frame 0x40 vs 0x48). Adding ANY
 * phantom inducer to it costs at least one instruction the target does not have.
 */
extern s32 func_800392B8(void);
extern void marionation_camera_Init_80037468(s32, s32 *, s32);

void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 sp[6];
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
