/* func_80075670 — SANDBOX 0 candidate (session 1, 2026-08-11).
 * This exact body is in place in src/text1b.c (line ~7099) and scored
 * distance 0 under `sandbox func_80075670 --disable all` (112/112 insns).
 * 9 regfix rename rules become dead and are retired by the driver.
 *
 * Key structure (why it matches): the previous form used ONE `base`
 * variable across the whole function; its unioned live range (idx 0-101)
 * tanked its global.c priority so it was allocated LAST ($a3), rotating
 * the bottom-half registers. The original had THREE separate pointer
 * locals: `base` (top half -> $v1), `work` (bottom half -> $a1, allocated
 * FIRST so the loop constants land 1->$a3, 2/3->$a2), and `q` (post-loop
 * block -> fresh $a3 reload shared by both the read `q + arg1 + 0x68` and
 * the write `q + ((arg1+1)&1) + 0x68`, giving the single `lw gp` +
 * `addu $a3,$a3,$v1` self-update). All three locals are live and
 * dereferenced — no dead code, no FAKE constructs.
 */
void func_80075670(s32 arg0, s32 arg1) {
    s16 i;
    u8 *base;
    u8 *work;
    u8 *q;
    s16 *p;

    base = D_800A36A0;
    if (*(s32 *)(base + 0x10) != 0) {
        return;
    }
    if ((func_800692C0(&arg0, arg1, (s16 *)(base + (arg1 * 4 + 0x40)), (&D_800A35D0) + (arg1 * 2)) >> 16) != 0) {
        *(s16 *)(D_800A36A0 + 0x34) = 0;
        *(u8 *)(D_800A36A0 + arg1 + 0x68) = *(u8 *)(D_800A36A0 + arg1 + 0x68) + 1;
        base = D_800A36A0;
        *(u8 *)(base + arg1 + 0x68) &= 1;
        func_8005C650(0, 0x7F, 0x7F);
    }
    work = D_800A36A0;
    *(s16 *)(work + arg1 * 2 + 0x3C) = *(u8 *)(work + arg1 + 0x68);
    if (arg0 & (0x40 << (arg1 * 16))) {
        for (i = 0; i < 2; i++) {
            ((s16 *)(work + i * 2))[0x38 / 2] = 0;
            ((s16 *)(work + i * 2))[0x10 / 2] = 1;
            ((s16 *)(work + i * 2))[0x18 / 2] = 2;
        }
        q = D_800A36A0;
        *(u8 *)(q + ((arg1 + 1) & 1) + 0x68) = (*(u8 *)(q + arg1 + 0x68) + 1) & 1;
        func_8005C650(1, 0x7F, 0x7F);
        return;
    }
    if (arg0 & (0x10 << (arg1 * 16))) {
        if (arg1 != 0) {
            p = (s16 *)(work + 0x14);
        } else {
            p = (s16 *)(work + 0x16);
        }
        if (*p == 1) {
            for (i = 0; i < 2; i++) {
                ((s16 *)(work + i * 2))[0x38 / 2] = 0;
                ((s16 *)(work + i * 2))[0x10 / 2] = 3;
                ((s16 *)(work + i * 2))[0x18 / 2] = 0;
            }
            func_8005C650(2, 0x7F, 0x7F);
        } else {
            func_8005C650(4, 0x7F, 0x7F);
        }
    }
}
