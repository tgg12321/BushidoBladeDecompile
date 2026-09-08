/* CD_datasync - SESSION 62 (structural).  SCORE 0 / 91.  MATCHED.
 *
 * Full build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (verify-oracle,
 * this session, with this body in src/system.c).
 *
 * WHAT CLOSED IT.  s60/s61 transplanted CD_ready's matched do_timeout window
 * and then spent thirteen forms hunting for a MULTI-SET carrier for chain A's
 * address, because sched.c:2505 birthing_insn_p boosts a single-set dest and
 * the boost was believed to be what mis-ordered the window.  s61's own model
 * consequence was already the answer and was read the wrong way round: in the
 * target the chain-A address sits in $a0 while every tail value sits in
 * $v0/$v1, so the original had NO spanning carrier.  The correct move was
 * therefore not to add a set but to REMOVE THE ADDRESS PSEUDO ENTIRELY -
 * subscript the table directly in the printf call (`tbl_125c[t0]`) instead of
 * staging the address through a named `pA`/`aA`/`dma` local.  With no address
 * intermediate there is no boostable addu dest at all: expand emits the
 * subscript inside the call sequence, and the window comes out byte-exact with
 * chain A in $a0 and the second table read in $v1.
 *
 * The same removal applies to the fifth (stack) argument: `arg5 = *pB` with a
 * `pB` address local was also unnecessary; `tbl_125c[tb]` inlined into the
 * call is byte-identical.  Both address locals AND the arg5 value local are
 * gone.  What remains is strictly smaller than every form measured in
 * sessions 2-61.
 *
 * WHY THE SEARCH MISSED IT FOR 60 SESSIONS.  Every chassis since s2 carried a
 * named address intermediate as a fixed assumption (it is what m2c emits and
 * what CD_ready ships), so the whole search space was "which pseudo do we
 * stage the address through" - a space in which 8 is the floor on the
 * single-set chassis and 3 on every multi-set one.  Removing the stage was
 * never in the space.  s62 also measured that on the single-set chassis the
 * window is completely insensitive to declaration order, statement order
 * inside the wrap, and type narrowing (nine forms, all 8) - which is the
 * signature of "the pseudo itself is the problem", not its ordering.
 *
 * EVERY REMAINING CONSTRUCT IS LOAD-BEARING (s62 ablation table, each measured
 * by removing exactly one thing from this body):
 *     do-while(0) wrap removed .................... 13 / 91
 *     pp alias removed (direct D_800F19C0) ......... 4 / 91
 *     tbl_11dc alias removed ...................... 18 / 88
 *     tbl_125c alias removed ...................... 27 / 89
 *     idx_1494 alias removed ...................... 20 / 90
 *     t0 moved inside the wrap ..................... 4 / 91
 *     t0 inlined into the call ..................... 12 / 91
 *     t0, tb both inlined .......................... 13 / 91
 * Each surviving construct carries its FAKE annotation with the named GCC pass
 * and the lever-exhaustion pointer, per the owner's three-prong policy.
 */
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    volatile u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC; /* FAKE: pointer alias (second handle) to the libcd command-name table per pointer-alias-fake-exception (owner ruling 2026-07-01), mechanism: global.c seats the base in $s3 across the poll loop as the target does (asm/funcs/CD_datasync.s:11-12); lever-exhaustion: s62 ablation A4 (direct D_800A11DC[] subscript) = 18/88, plus the 61-session ledger in memory/grind/CD_datasync/hypotheses.md */
    idx_1494 = &g_cd_status_a; /* FAKE: pointer alias (second handle) to the libcd Intr status block per pointer-alias-fake-exception (owner ruling 2026-07-01); the volatile is the TU's own declaration `extern volatile u8 g_cd_status_a;` (src/system.c, on main since 7e182728; ground truth `static volatile CD_intr Intr`, memory/closer/libcd-identity.md:28) - no cast, no local qualifier; mechanism: global.c seats the base in $s1 across the poll loop as the target does (asm/funcs/CD_datasync.s:13-14); lever-exhaustion: s62 ablation A5 (direct (&g_cd_status_a)[n] subscript) = 20/90 */
    tbl_125c = D_800A125C; /* FAKE: pointer alias (second handle) to the CD_intstr table per pointer-alias-fake-exception (owner ruling 2026-07-01), mechanism: global.c seats the base in $s0 across the poll loop as the target does (asm/funcs/CD_datasync.s:15-16); lever-exhaustion: s62 ablation A3 (direct D_800A125C[] subscript) = 27/89 */
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

loop:
    v0 = VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    puts(&g_str_cd_timeout);
    {
        s32 t0;
        s32 tb;
        void **pp;
        t0 = idx_1494[0]; /* FAKE: named intermediate for the sync byte, placed BEFORE the wrap (loop depth 1), mechanism: flow.c loop_depth-weighted reg_n_refs feeds local-alloc.c qty_compare - the depth-1 mention leaves the merged chain-A quantity below the second table read's priority, so the chain takes $a0 and the value $v1 exactly as at asm/funcs/CD_datasync.s:50/56/60/65; lever-exhaustion: s62 probe B3 (same read placed inside the wrap) = 4/91, s62 probe B1 (inlined into the call) = 12/91, s60 in-place spelling = 15 */
        do { /* FAKE: do-while(0) wrap per do-while-zero-exception (owner ruling 2026-07-06), mechanism: sched.c:2081 loop-note barrier on the first insn inside (the ready-byte load) orders the sync-byte load ahead of it and every later register-argument load after it, and flow.c loop_depth ref weighting seats tbl_125c in $s0; lever-exhaustion: s62 ablation A2 (wrap removed) = 13/91, s61 ablation 2 -> 12 */
            tb = idx_1494[1]; /* FAKE: named intermediate for the ready byte (fresh, once-written, once-read, real value = lbu $v0,1($s1) at asm/funcs/CD_datasync.s:51), mechanism: expand argument staging keeps the fifth (stack) argument's chain out of the call sequence so the sw lands at slot 63; lever-exhaustion: s62 probe B1 (tb inlined into the call) = 12/91, s62 probe A6 (all intermediates inlined) = 13/91 */
            pp = &D_800F19C0; /* FAKE: pointer alias (second handle) to the alarm callback slot per pointer-alias-fake-exception (owner ruling 2026-07-01, the `Type* t = &g_Thing;` redundant-second-handle shape), mechanism: calls.c:1652-1664 expand_call precomputes a register argument whose rtx_cost > 2 into a pseudo inside a loop (preserve_subexpressions_p), whereas `*pp` is a cheap mem(reg) that stays in the call sequence and cse folds the alias back to the target's `lui $a1 / lw $a1` at asm/funcs/CD_datasync.s:52-53; lever-exhaustion: s62 ablation A1 (direct D_800F19C0 read) = 4/91 */
            printf(&D_800161C8, *pp, tbl_11dc[D_800A11D5], tbl_125c[t0], tbl_125c[tb]);
            CD_flush();
        } while (0);
    }
    v0 = -1;
    goto check;

success:
    v0 = 0;

check:
    if (v0 != 0) {
        return -1;
    }
    if (*D_800A14C0 & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
