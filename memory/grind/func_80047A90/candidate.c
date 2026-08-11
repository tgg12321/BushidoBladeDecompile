/* func_80047A90 — BEST FORM, session 1 (recon), honest sandbox floor 14 (down from 20).
 *
 * THIS FORM IS ALREADY APPLIED IN src/sound.c (pure C, zero pins, zero rules).
 * The old pinned version (register asm("$N") ×10) was removed this session —
 * it was cheat-asm; sandbox strips it, so it never counted.
 *
 * Structure facts (all measured this session, see evidence.md):
 *  - Loop 1 MUST be a real for-loop with array indexing: loop.c then LICM-hoists
 *    the Judge base and strength-reduces D_800EF558[i]/D_800EF59C[i] into
 *    walking-pointer givs — this reproduces target's loop-1 shape exactly.
 *  - Loop 2 MUST be goto-form (no loop notes): a real for/do-while loop gets
 *    LICM of 0x7D0 / 0x66666667 / 8 constants out of the loop; target keeps
 *    them inline. Source-level pointer walk (pa1/pa2/pt1/pt2/pt3) is required.
 *  - Statement order "pt2; pt1; k = 1;" (counter init LAST) is load-bearing:
 *    it lengthens pt1/pt2 live ranges, sinking their allocno priority below
 *    a3off's, which lands pt1->t1, pt2->t2, pt3->t3, pa1->a1, pa2->a2 (all
 *    target). Other orders measured: k-first = 21, k-middle = 18.
 *
 * Remaining 14 diff = two 2-cycles:
 *   loop1: i(a2 vs t0) x judge-base(a3 vs a2)
 *   loop2: k(a3 vs t0) x a3off(t0 vs a3)
 * See hypotheses.md H-NEXT for the priority-arithmetic path to close both.
 */
void func_80047A90(void) {
    s32 i;
    s32 k;
    s32 a3;
    s32 v1;
    s32 a0;
    s32 *pt2;
    s32 *pt1;
    s32 *pa1;
    s32 *pa2;
    s32 *pt3;
    s32 *temp;

    for (i = 0; i < 0x11; i++) {
        D_800EF59C[i] = ((s32)Judge[D_800EF558[i] & 0xFFF] * 0x271) >> 10;
        D_800EF558[i] += 0x12;
    }

    pt2 = D_800EF59C;
    pt1 = D_800EF59C + 0x11;
    k = 1;
  outer_loop:
    pa1 = pt1;
    a3 = 0;
    pa2 = pt2;
    pt3 = pt1 + 0x11;
  inner_loop:
    a0 = 0x7D0 - (*pa1 - *pa2);
    if (a0 < 0) {
        v1 = (a0 + 0xF) >> 4;
    } else {
        v1 = a0 / 10;
    }
    *pa1 += v1;
    if (k == 8) {
        *(s32 *)((s8 *)g_snd_fade_curve + a3) = v1;
    }
    pa1++;
    a3 += 4;
    pa2++;
    if ((s32)pa1 < (s32)pt3)
        goto inner_loop;
    pt2 += 0x11;
    k++;
    pt1 += 0x11;
    if (k < 9)
        goto outer_loop;

    temp = (s32 *)D_800A3820;
    D_800A3820 = (s32)(temp + 1);
    *temp = (s32)&D_800EF070;
}
