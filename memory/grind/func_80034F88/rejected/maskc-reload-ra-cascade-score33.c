/*
 * REJECTED (s10, synthesis) — from x1_maskc_one_ptr.c
 *
 * s10 waves X/A2 — routing the MASK's stored value through the reused local `c`
 * (`c = *ptr & 0xF8; *ptr = c;`) so that block 1's `c = p[8] & 1;` kills it and
 * block 1's read becomes a real reload.  The mechanism WORKS: this is the only
 * non-volatile, zero-extra-instruction form measured that reaches the target's
 * full `lbu 5` count with an unconditional store.  It is nevertheless dead,
 * because carrying the mask through `c` extends that local's live range across
 * the call return and permutes the entire allocation (`p` lands in a2, the
 * condition in v1, the selected value in v1): 33 at 45 insns, and 33 on every
 * base structure it was crossed with — x1/x5/x6 33/45, x3 33/47, x2/x4/x8 33/49,
 * zz1 33/49, a1/a2/a3/a4 33/49, a8 33/49, a7 34/50.
 * Controls that keep the mask out of `c` all score 9-13 with `lbu 4`.
 * So block 1's reload and the target's register assignment are, on this chassis,
 * mutually exclusive — that is the whole of the residual 9 and the frontier for
 * the next session.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 val;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    c = *ptr & 0xF8;
    *ptr = c;

    c = p[8] & 1;
    val = *ptr;
    if (c) {
        c = val | 1;
    } else {
        c = val;
    }
    *ptr = c;

    c = p[8] & 2;
    val = *ptr;
    if (c) {
        c = val | 2;
    } else {
        c = val;
    }
    *ptr = c;

    c = p[8] & 4;
    val = *ptr;
    if (c) {
        c = val | 4;
    } else {
        c = val;
    }
    *ptr = c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
