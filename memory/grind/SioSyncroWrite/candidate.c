/* candidate.c — SioSyncroWrite (formerly func_8008C1E8), grind session s4 (2026-08-19)
 *
 * HONEST FLOOR WITH THIS BODY IN PLACE IN src/main.c: sandbox --disable all = 0
 * (target_insns 159, build_insns 159, rules_dropped 1, cheat_asm_stripped 66).
 * Zero register pins, zero inline asm, zero rules touched, src/main.c only.
 *
 * === What s4 changed, and why it worked ===============================
 * s1-s3 left a single missing `addiu $v0,$v0,%lo(D_800F1AF4)` at the OUTER-LOOP
 * BOTTOM exit test: ours folded to `lui;lw`, target has the un-folded
 * `lui;addiu;lw`. s3 proved (loop.c:688-701) that a named pointer local in a
 * `while (cond)` loop can never produce it, because scan_loop hoists the
 * address set out of the loop, and the hoisted pseudo then takes a callee-save.
 *
 * s4 ran the permuter (mandated modality) on the s3 chassis: 63.6k iterations,
 * ZERO finds — the basin is dry (banked). A second, DIRECTED campaign with
 * PERM_GENERAL alternatives spelled out at exactly that test produced the
 * structural insight the random search could not: the cast form
 * `*(volatile s32 *)&D_800F1AF4 != 0` un-folds BOTH copies of the test, because
 * expand_end_loop duplicates a `while` condition verbatim to the loop top. In
 * TARGET the top copy is FOLDED (`lui;lw`) and only the bottom copy is
 * un-folded. Two copies of one `while` condition cannot differ — therefore
 * target's two tests are NOT one duplicated `while` condition.
 *
 * That kills the whole `while (D_800F1AF4 != 0) { ... }` chassis that s1-s3
 * were grinding, and replaces it with:
 *
 *     if (D_800F1AF4 == 0) goto done;    <- plain global read, FOLDS  (top)
 *     for (;;) {
 *         ... body ...
 *         { volatile s32 *remaining = &D_800F1AF4;
 *           if (*remaining == 0) break; }   <- pointer alias, UN-FOLDS (bottom)
 *     }
 *
 * The block scope is load-bearing: the alias dies at the closing brace, so its
 * live range never crosses the DeliverEvent/callback region and it is never a
 * candidate for a callee-save register (that was s3's rejected form, score 6);
 * and because the loop is `for (;;)` there is no condition for scan_loop to
 * duplicate to the top, so the top test keeps its folded, direct-global form.
 * Five equivalent loop shapes were measured and ALL match at 159/0 diff lines
 * (for(;;)+break, while(1)+break, for(;;)+goto done, do{}while(1)+break,
 * do{}while(stmt-expr)) — see hypotheses.md [s4-H2]; `for (;;)` is the one
 * banked here as the most ordinary C.
 *
 * === s4's second cleanup: the discarded read-backs are natural C ========
 * s2 spelled target's `sw` -> `lw` read-back pairs as `st[1] += 1; st[1];`
 * (a bare discarded volatile read — a construct a reader would question).
 * s4 measured that plain `st[1]++;` / `st[2]--;` emit exactly the same
 * load/modify/store/load, because GCC 2.7.2 re-loads a volatile lvalue after a
 * discarded-result post-increment. The bare read statements are GONE from the
 * body; the target really does contain those two `lw`s (asm/funcs/
 * SioSyncroWrite.s:107,111 — `lw $v0,0x4($s3)` / `lw $v0,0x8($s3)`).
 * Dropping them entirely costs 2 insns (157i) — they are load-bearing and
 * semantically real, not padding. See hypotheses.md [s4-H3].
 *
 * === Every pointer alias in the body is measured load-bearing ==========
 * s4 swept the direct-global form of each one (hypotheses.md [s4-M1..M5]):
 *   drop p_ae2   -> 158i, 27 differing lines
 *   drop p_af8   -> 158i, 23
 *   drop p_af4b  -> 158i, 29
 *   drop p_af4   -> 158i,  5
 *   drop st      -> 158i, 51
 * so each carries a /* FAKE */ annotation naming the pass (combine.c symbol
 * fold) and the exhaustion record, per pointer-alias-fake-exception.md.
 * `flag` / `st` are NOT annotated: they are the LIBCOMB control-block BASE
 * pointer, which target itself holds in $s3 and indexes at 0x4/0x8/0xC
 * (asm/funcs/SioSyncroWrite.s:106-115) — genuine program structure, not a
 * redundant second handle.
 *
 * === OPEN INTEGRATION HANDOFF (not a blocker; bytes are already 0) ======
 * `volatile s32 *flag = &D_800F1AEC;` adds a volatile qualifier the file-scope
 * `extern s32 D_800F1AEC;` does not carry. The natural spelling is
 * `extern volatile s32 D_800F1AEC;`, and s4 re-measured on THIS chassis that
 * it is score-neutral (159i, 0 diff lines — hypotheses.md [s4-M6]); s3 also
 * measured it score-neutral for the struct-sharing sibling SioAnsyncWrite.
 * Applying it needs a volatile_extern_allowlist.txt entry (two-prong evidence
 * already complete in evidence.md: IRQ writer HandleSio `sw $zero,0x0($a0)`
 * @0x8008CCD4, block base $a0=&D_800F1AEC loaded @0x8008CC78), and that file
 * is OUT OF SCOPE for a grind candidate (tools/grinder/grind.ps1:540 — only
 * src/main.c is allowed, tools/grinder/scope_allow.txt has no entry for this
 * function). Operator steps are listed in the s4 outcome JSON and evidence.md.
 *
 * Depends on file-scope decls in src/main.c (~3427-3435):
 *   extern volatile u16 D_800F1AE2; extern s32 (*D_800F1AE8)(s32, s32);
 *   extern s16 D_800A3074[4]; extern s32 D_800F1AEC;
 *   extern volatile s32 D_800F1AF0, D_800F1AF4, D_800F1AF8;
 *   extern s32 D_800A3044;
 */
s32 SioSyncroWrite(u8 *arg0, s32 arg1) {
    volatile s32 *flag = &D_800F1AEC;
    s32 retries;
    s32 pkt_len;
    s32 i;
    s32 (*cb)(s32, s32);

    retries = 0;
    if (*flag != 0) return -1;
    {
        /* FAKE: redundant second handle to D_800F1AE2, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: memory/grind/SioSyncroWrite/hypotheses.md [s4-M1] (direct-global form measured 158i/mismatch) */
        volatile u16 *p_ae2 = &D_800F1AE2;
        u32 mode;
        mode = *p_ae2;
        pkt_len = *(s16 *)((s32)D_800A3074 + ((mode & 0x300) >> 7));
    }
    D_800F1AF4 = arg1;
    D_800F1AF0 = (s32)arg0;
    i = 0;
    if (D_800F1AF4 == 0) goto done;
    for (;;) {
        volatile s32 *st = flag;

        while ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 5) != 5) {
            cb = D_800F1AE8;
            if (cb != 0) {
                s32 prev = retries;
                retries += 1;
                if (cb(2, prev) == 0) {
                    DeliverEvent(0xF000000B, 0x100);
                    goto done;
                }
            }
        }
        if (i == 0) {
            D_800F1AF8 = (*((volatile u16 *)(((s32)D_800A3044) + 4))) & 0x80;
        }
        *((u8 *)D_800A3044) = *((u8 *)D_800F1AF0);
        st[1]++;
        i += 1;
        st[2]--;
        if (i == pkt_len) {
            if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == st[3]) {
                /* FAKE: redundant second handle to D_800F1AF8, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: hypotheses.md [s4-M2] (direct-global form measured 158i/mismatch) */
                volatile s32 *p_af8 = &D_800F1AF8;
                do {
                    cb = D_800F1AE8;
                    if (cb != 0) {
                        s32 prev = retries;
                        retries += 1;
                        if (cb(2, prev) == 0) {
                            /* FAKE: redundant second handle to D_800F1AF4, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: hypotheses.md [s4-M3] (direct-global form measured 158i/mismatch) */
                            volatile s32 *p_af4b = &D_800F1AF4;
                            DeliverEvent(0xF000000B, 0x100);
                            return (arg1 - *p_af4b) - 1;
                        }
                    }
                } while ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == *p_af8);
            }
            i = 0;
        }
        {
            /* FAKE: redundant second handle to D_800F1AF4, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: hypotheses.md [s4-H1/H2] (plain-global and function-scope-pointer forms both measured negative) */
            volatile s32 *remaining = &D_800F1AF4;
            if (*remaining == 0) break;
        }
    }

done:
    {
        /* FAKE: redundant second handle to D_800F1AF4, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: hypotheses.md [s4-M4] (direct-global form measured 158i/mismatch) */
        volatile s32 *p_af4 = &D_800F1AF4;
        return arg1 - *p_af4;
    }
}
