/*
 * func_80034F88 — best form as of grind session s1 (recon).
 * Honest sandbox floor (`sandbox func_80034F88 --disable all`): 23
 *   (previous committed form scored 24 and additionally carried three
 *    `asm volatile("" ::: "memory")` scheduling barriers, which this form
 *    removes outright — they are forbidden cheats and were score-inert.)
 *
 * Build: 50 insns vs target 49.
 *
 * WHY THE MIXED POINTER/SYMBOL SPELLING IS HERE — AND WHY IT IS NOT A
 * FINISHED FORM.  The single measured lever this session was that GCC
 * 2.7.2's cse.c forwards a QImode store into a following load of the SAME
 * memory only when both accesses hash to the same address rtx.  Writing the
 * `&= 0xF8` through the pointer `ptr` and reading back through the plain
 * symbol `D_80106A73` makes the two address expressions differ, so the read
 * survives as a real `lbu` — which is what the target does.  That single
 * surviving reload is worth one point (24 -> 23).
 *
 * The construct is NOT proposed as a closing form: `ptr` has exactly one use
 * here, which is precisely the "why is this here?" smell the cheat checklist
 * flags (a one-use pointer alias to a global whose only effect is on GCC's
 * CSE table).  It is checked in as the best MEASURED form so the next
 * session inherits the floor, not as something to submit.  See
 * hypotheses.md — the real open question is how the target gets a
 * *same-base-register* load/store pair with no forwarding, which no spelling
 * measured this session reproduces.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 val;
    u8 val2;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    val = D_80106A73;
    val2 = val | 1;
    if (!(p[8] & 1)) {
        val2 = val;
    }
    D_80106A73 = val2;

    val = D_80106A73;
    val2 = val | 2;
    if (!(p[8] & 2)) {
        val2 = val;
    }
    D_80106A73 = val2;

    val = D_80106A73;
    val2 = val | 4;
    if (!(p[8] & 4)) {
        val2 = val;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + 0x17 + i);
    }
}
