/* func_80042ED8 — s2 candidate, sandbox 0 + byte-identical (verified
   tmp/grind/func_80042ED8/s2/check_bytes.sh). Replaces the judge-flagged
   t0-t3 partial-read-ahead transcription with a save-then-swap shape:
   natural names, natural per-swap tail order; the ONLY matching-motivated
   choice is grouping the three saves (annotated). Layer-1 cheat-reviewer
   FAILed it as a re-spelling of the flagged family — ruling requested;
   see tmp/grind/func_80042ED8/s2/layer1_verdict.md + outcome JSON. */
void func_80042ED8(u16 *a0) {
    /* FAKE: statement staging (2026-07-06 ALLOWED list) — saving one
       side of all three pairs up front seats x/y/z in $a1/$v1/$v0 for
       the whole body with the scratch reloads sharing $a2, and keeps
       the load-delay nop at +0x18 unfilled, as in the target. */
    u16 t, x, y, z;
    y = a0[1];
    x = a0[2];
    z = a0[5];
    t = a0[6];
    a0[2] = t;
    a0[6] = x;
    t = a0[3];
    a0[1] = t;
    a0[3] = y;
    t = a0[7];
    a0[5] = t;
    a0[7] = z;
}
