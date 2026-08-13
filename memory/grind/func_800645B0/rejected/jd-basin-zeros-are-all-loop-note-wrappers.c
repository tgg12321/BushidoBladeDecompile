/* REJECTED (grind session 9c, PERMUTER modality, 2026-08-13) — NOT proposed.
 *
 * WHAT WAS RUN.  A fresh decomp-permuter campaign on a brand-new offset-0
 * workspace built from the JD chassis (memory/grind/func_800645B0/
 * chassis_jd_inline_index_arith.c), i.e. the "inline index arithmetic, no
 * idx2/wid locals" body that measures 3 / 78 with the sandbox and 260 with the
 * permuter's weighted scorer.  Workspace tmp/grind/func_800645B0/s9c/ws_jd,
 * built by s9c/mkchassis.py + s9c/mkws.sh (full-TU cpp of a src/text1b.c copy,
 * honest compile.sh pipeline, session-4 offset-0 target.o).  Validated at
 * base 78 insns vs target 78 with exactly the three known JD diffs
 * (`li v1,1` / `addu s0,s3,a0` ordering at 11-13 and the copy at 65-66).
 *
 * WHAT IT FOUND.  The basin yields score 0 almost immediately and repeatedly:
 * the first zero landed 20.6 s after launch (iteration 390); a relaunch without
 * --stop-on-zero produced ELEVEN score-0 finds in 23,456 iterations / 22.8 min.
 * The eleven collapse to six distinct function bodies (s9c/zdiff.py).
 *
 * WHY EVERY ONE IS DEAD.  All six distinct zero bodies close the residual with
 * the same construct: an empty statement-level LOOP NOTE at the inner-loop top,
 * in one of three spellings —
 *
 *     do { idx = i + j; } while (0);            (0-1..0-4, 0-8, 0-10)
 *     idx = i + j;  do { } while (0);           (0-5, 0-6, 0-9, 0-11)
 *     if (1) { <the whole function body> }      (0-7, plus the same do-while)
 *
 * — with the remaining differences between them being pure noise the permuter
 * added and the scorer ignores (`val |= mask` vs `val = val | mask`, an
 * `(unsigned long long)` cast on the `i` initialiser, and in 0-10 a
 * store-then-read-back respelling of the halfword store that is not even
 * semantics-preserving in spirit).
 *
 * That construct is already banked as REJECTED for this function
 * (rejected/permuter-bare-do-while0-wrapper-outside-carveout.c, session 5, from
 * the CA chassis): `do { ... } while (0);` is sanctioned ONLY for the
 * LABEL_OUTSIDE_LOOP_P / reorg.c `relax_delay_slots` interaction
 * (.claude/rules/do-while-zero-exception.md), and the mechanism here is NOT
 * that — it is the NOTE_INSN_LOOP_BEG boundary changing where cc1's first-pass
 * scheduler may move the const-1 set relative to the index `addu`, i.e. the
 * same scheduling-tie steer this function's BANNED `j += 1;` relocation used.
 * `if (1) { ... }` is in the forbidden-family catalog verbatim
 * (.claude/rules/no-new-park-categories.md).
 *
 * THE VALUE OF THE RUN (this is why it is banked rather than discarded):
 *
 *  1. It REPLICATES the session-5 CA-chassis result on a structurally
 *     unrelated chassis with a different pseudo set.  Two independent basins,
 *     ~92k + ~23k iterations, and the ONLY zero-scoring mechanism either one
 *     ever finds is the loop note.  n = 11 zeros, 6 distinct bodies, 100 %
 *     loop-note.  The permuter axis on JD is measured: it does not contain a
 *     policy-clean closing form.
 *  2. It PROVES the JD chassis is exactly one loop note away from byte-exact.
 *     Everything else about JD — every register, the *3 sum's commutative
 *     operand order, the instruction count — is already the target's.  So the
 *     whole function now reduces to a single question: what SEMANTIC C
 *     construct emits a loop note (or otherwise denies sched.c's
 *     birthing_insn_p lift) between `idx = i + j;` and `val = 1;` at zero
 *     instruction cost?
 *  3. It narrows that question by one measured answer — see
 *     rejected/block-scope-decls-are-codegen-inert.c: a lexical BLOCK boundary
 *     (declaration at point of use, or bare braces) does NOT substitute.  Only
 *     a loop-statement note does.
 *
 * Artifacts: tmp/grind/func_800645B0/s9c/ws_jd/{campaign.log,campaign_meta.json,
 * output-0-*}, s9c/zdiff.py.
 */
#if 0
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            do { idx = i + j; } while (0);      /* <-- the whole find, six ways */
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                /* ... JD body verbatim: inline ((idx<<1)+idx)<<2 word stores,
                   (idx<<1) halfword store, occupancy OR, break ... */
            }
        }
    }
#endif
