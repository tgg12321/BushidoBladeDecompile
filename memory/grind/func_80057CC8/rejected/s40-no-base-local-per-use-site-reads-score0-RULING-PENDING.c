/* s40 (2026-08-27, forensics) — RULING-PENDING, NOT a submission.
 *
 * MEASURED THIS SESSION on the live chassis: `sandbox func_80057CC8 --disable all`
 * -> score 0, target_insns 111, build_insns 111, rules_dropped 0.  The form BYTE-
 * MATCHES today, i.e. the 2026-07-20 owner ruling / Judge bans are the only thing
 * between this function and COMPLETED-C.
 *
 * WHAT IT IS: there is NO cached vertex-table base local at all.  Not two locals,
 * not one local plus an inline reload — zero.  Every vertex-table access is spelled
 * at its point of use.  That is banned_constructs entry 5 ("No pointer/base local
 * declared; ... computed independently at each of the two ratan2 call sites") in the
 * cast spelling rather than the struct spelling, so this file is banked for the
 * ruling question only and must not be submitted until that ban is re-ruled.
 *
 * WHY s40 RE-OPENED IT (the forensic result — full detail in evidence.md s40):
 *   - The C source of this form contains FIVE reads of *(s16 **)(arg0 + 4).  The
 *     front end emits five (`.jump` dump: 5 `(mem:SI (plus:SI (reg ...) (const_int 4)))`).
 *     cse1 folds them to TWO (`.cse`, `.loop`, `.combine`: 2) — the number the target
 *     has.  The "duplication" is therefore not authored; it is cse1's output.
 *   - Isolated 4-way probe (tmp/grind/func_80057CC8/s40/probe.c/.s) names the pass and
 *     the deciding predicate exactly:
 *         pA  two source reads, extern call between            -> 2 lw   (target's shape)
 *         pB  the same two source reads, NO call between       -> 1 lw   (cse1 folds)
 *         pC  one read cached in a local, call between uses    -> 1 lw   (our 16-form)
 *         pD  two source reads, call between, `s16 *const *`   -> 1 lw   (cse1 folds)
 *     .jump has 2 base loads in pA/pB/pD; .cse has 2/1/1.  The fold is cse1's, and the
 *     ONLY thing that stops it in pA is the intervening non-const CALL_INSN:
 *     cse.c:1948 marks a MEM without RTX_UNCHANGING_P as hash_arg_in_memory, and
 *     cse.c:7241-7246 (`if (! CONST_CALL_P (insn)) invalidate_memory (&everything);`)
 *     purges every such element at the call.
 *   - Consequence for cheat-checklist T1: this form and the cached-local form are NOT
 *     the same program.  The cached-local form asserts that ratan2 cannot write
 *     ((s16 **)arg0)[1]; C does not guarantee that, and pD shows the compiler folds the
 *     reads the moment it is told so.  The construct has an observable effect.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s16 *p;
    s32 pi;
    u16 cy;

    prev_idx = arg1 - 1;
    cx = *(u16 *)((s32)(*(s16 **)(arg0 + 4)) + arg1 * 4 + 0);
    cy = *(u16 *)((s32)(*(s16 **)(arg0 + 4)) + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2((*(s16 **)(arg0 + 4))[pi * 2] - (s16) cx,
                      (*(s16 **)(arg0 + 4))[pi * 2 + 1] - (s16) cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = ratan2(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
