/* func_80047A90 — MATCH FORM, session 6 (forensics), honest sandbox 0 (84/84, 0 rules).
 *
 * THIS FORM IS APPLIED IN src/sound.c (pure C, zero pins, zero rules).
 *
 * How s6 closed the last 6 (the loop-1 i<->judge 2-cycle) — full mechanism:
 * FINDREGDBG traces (instrumented cc1, tools/gcc-2.7.2/cc1) proved that in every
 * SEPARATE-counter landscape, find_reg(i) sees conflicts {v0,v1,a0,a1} only,
 * empty regs_someone_prefers, empty own prefs -> first-free ascending walk ->
 * $a2, UNCONDITIONALLY: no reachable source state masks both $a2 and $a3
 * (someone_prefers[i] can only come from full-prefs of later-allocated
 * CONFLICTING allocnos = only the judge base, whose sole set is
 * (set (reg) (symbol_ref)) which set_preference ignores). The s1-H2
 * "counter->t0" observation could not be reproduced in 4 nested-for spellings
 * (h2a-h2d dumps) and is presumed misread.
 * The escape is a LANDSCAPE change: a counter SHARED by both loops conflicts
 * with pa2($a2-holder), a3($a3-holder) and pt1, so once those allocate first
 * the walk itself forces the counter to $t0, and the loop-1 judge base (jb,
 * 2 refs, allocates last) lands $a2 sharing with the loop-2-only pa2. Priorities
 * that make the order work (ALLOCDBG-measured, floor_log2(refs)*refs/live):
 *   pa1 9/34=.794($a1) > pa2 7/32=.4375($a2) > a3 7/33=.424($a3)
 *   > i 9/67=.403($t0) > p59C 4/22($a1) > pt1 6/40=.300($t1)
 *   > pt2 6/41=.293($t2) > pt3 3/31($t3) > jb 2/48($a2).
 * Requirements encoded in the source:
 *   1. Loop 1 must be NOTE-FREE (goto-form, source pointer walk p558/p59C +
 *      hand-hoisted jb) so i's loop-1 refs are UNWEIGHTED: 9 total refs
 *      (4 loop-1 + 5 loop-2). A for-form loop 1 weights them to 12 -> prio
 *      .537 -> allocates before pa2 -> $a2 (measured, m1 dump). Goto-form
 *      pointer-walk loop 1 is byte-identical to the for-form's giv output
 *      (s1 fact: clean pointer-walk everywhere = same 84 insns).
 *   2. i = 1 reinit FIRST at the loop-2 init site (matches target emission
 *      order slots 30-33 naturally in this form — no wrap needed, unlike the
 *      separate-k s3 form).
 *   3. Three single-level do{}while(0) wraps (family do-while-zero-exception,
 *      owner 2026-07-06 final, FAKE-annotated inline): a3=0 and pa2=pt2 lift
 *      those two above i (without them: pa2 .375/a3 .364 < i .403 -> i takes
 *      $a2 — the s2 kill); pt3=pt1+0x11 restores pt1 > pt2 after the pa2
 *      wrap's weighted pt2-use lifted pt2 (without it: pt1/pt2 swap, score 8,
 *      m2 dump).
 *   4. Inner-loop tail duplicated into both if(i==8) arms (family
 *      duplicated-statement-into-arms, owner 2026-07-01, FAKE-annotated;
 *      byte-neutrality verified: cross-jump re-merges to the single shared
 *      tail, 84/84) — lifts pa1/pa2/a3 ref counts as in s2.
 * Verified this session: sandbox 0 twice (bare m3 + annotated final), all 84
 * instructions diffed 1:1 against target.txt including registers.
 * Dumps: tmp/grind/func_80047A90/s6/ (m1-m3, h2a-h2d, base6, dump.sh).
 */
void func_80047A90(void) {
    s32 i;
    s32 a3;
    s32 v1;
    s32 a0;
    s16 *jb;
    s32 *p558;
    s32 *p59C;
    s32 *pt2;
    s32 *pt1;
    s32 *pa1;
    s32 *pa2;
    s32 *pt3;
    s32 *temp;

    i = 0;
    jb = Judge;
    p558 = D_800EF558;
    p59C = D_800EF59C;
  loop1:
    i++;
    *p59C = ((s32)jb[*p558 & 0xFFF] * 0x271) >> 10;
    p59C++;
    *p558 += 0x12;
    p558++;
    if (i < 0x11)
        goto loop1;

    i = 1;
    pt2 = D_800EF59C;
    pt1 = D_800EF59C + 0x11;
  outer_loop:
    pa1 = pt1;
    do {
        /* FAKE: loop-note ref weighting lifts a3's allocno priority above
         * the shared counter i, seating a3 in $a3 and i in $t0 */
        a3 = 0;
    } while (0);
    do {
        /* FAKE: loop-note ref weighting lifts pa2 above the shared counter
         * i, seating pa2 in $a2 (shared with the loop-1 Judge base) */
        pa2 = pt2;
    } while (0);
    do {
        /* FAKE: loop-note ref weighting keeps pt1 ahead of pt2 in
         * allocation order (pt1->$t1, pt2->$t2) after the pa2 wrap's
         * weighted pt2 use lifted pt2 */
        pt3 = pt1 + 0x11;
    } while (0);
  inner_loop:
    a0 = 0x7D0 - (*pa1 - *pa2);
    if (a0 < 0) {
        v1 = (a0 + 0xF) >> 4;
    } else {
        v1 = a0 / 10;
    }
    *pa1 += v1;
    if (i == 8) {
        *(s32 *)((s8 *)g_snd_fade_curve + a3) = v1;
        pa1++;
        a3 += 4;
        pa2++;
    } else {
        /* FAKE: loop tail duplicated into both arms (cross-jump re-merges,
         * byte-neutral); reg_n_refs lift lands pa2->$a2, a3->$a3 */
        pa1++;
        a3 += 4;
        pa2++;
    }
    if ((s32)pa1 < (s32)pt3)
        goto inner_loop;
    pt2 += 0x11;
    i++;
    pt1 += 0x11;
    if (i < 9)
        goto outer_loop;

    temp = (s32 *)D_800A3820;
    D_800A3820 = (s32)(temp + 1);
    *temp = (s32)&D_800EF070;
}
