/* s38 (synthesis, 2026-09-05) — the PUN-FREE equivalent of candidate.c.
 *
 * Identical to memory/grind/func_80034F88/candidate.c except that the trailing
 * copy loop is spelled `D_80106A70[i] = ...` instead of the declaration pun
 * `*(&D_80106A70 + i) = ...`.  It REQUIRES a one-line header change that is
 * outside a grind session's surface (INTEGRATION HANDOFF):
 *
 *     include/code6cac.h:472
 *     -  extern u8 D_80106A70;
 *     +  extern u8 D_80106A70[3];
 *
 * (D_80106A70 is the only symbol touched; `grep -rn D_80106A70 include src`
 * returns exactly the header line and this loop.)
 *
 * MEASURED s38 with that header change in place:
 *   `sandbox func_80034F88 --disable all` = score 10, build_insns 49 — and the
 *   objdump is BIT-IDENTICAL to the punned body's
 *   (`diff tmp/grind/func_80034F88/s38/b0.txt tmp/grind/func_80034F88/s38/a1.txt`
 *   empty).  So removing the pun is exactly codegen-neutral.
 *
 * WHY IT MATTERS: the dispatch brief's auto-scan flags the pun in candidate.c as
 * a layer-1 FAIL condition ("the sanctioned fix is at the DECLARATION").  Any
 * future candidate-ready for this function should be THIS body plus the header
 * line, not candidate.c.  candidate.c keeps the punned spelling only because it
 * is the form that compiles against HEAD's header unmodified.
 *
 * The source byte must stay `*((u8 *)p + i + 0x17)`: spelling it
 * `((u8 *)p)[i + 0x17]` measures 12 at 49 (banked as
 * rejected/s38-loop-source-byte-array-indexed-score12.c).
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    m = m & 0xF8;
    *q = m;

    {
        s32 c;

        c = p[8] & 1;
        if (c) {
            c = m | 1;
        } else {
            c = m;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 2;
        v = *q;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 4;
        v = *q;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        D_80106A70[i] = *((u8 *)p + i + 0x17);
    }
}
