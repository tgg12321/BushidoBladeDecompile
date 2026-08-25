/* candidate.c - func_80040D48 - s3 2026-08-25 - honest floor 0 (from 4)
 * MEASURED: `sandbox func_80040D48 --disable all` = score 0, 272/272 insns,
 * rules_dropped 34, with this exact body in src/text1a_pre.c on HEAD 51a9e464.
 *
 * s1 (34->24): the asm("s7") pin removed; the prologue entry/null-check/cast
 *     split; s5 carries both the s4+0x2C pointer and the Copy8-loop source;
 *     copyloop/copydone spelling of the Copy8 loop.
 * s2 (24->4): the Copy8 loop's list-push walker (s4+0x10D4) and the later
 *     s4+0x8B4 walker are ONE variable (a2p), which raises that pseudo's
 *     global.c priority above the a3p copy pointer's and fixes both seats.
 * s3 (4->0): two edits, and they only work together.
 *   (a) case-0 init loop spelled with an explicit label + `goto` back edge.
 *       Target's loop is provably NOT strength-reduced: its base register holds
 *       s3+0x68 (the induction variable's own initial value) with the LAST
 *       store at +0x14. loop.c cannot produce that for three dest-address givs
 *       off one biv - combine_givs always merges them onto the last-recorded
 *       giv (benefit 2+2+2 = 6, lifetime 1+1+1 = 3) and the "not worth while"
 *       test `v->lifetime * threshold * benefit < insn_count`
 *       (3 * 2*(3+n_non_fixed_regs) * (6 - add_cost*1) vs 28) can never fire.
 *       With no NOTE_INSN_LOOP_BEG the loop is invisible to loop.c, the three
 *       stores stay biv-relative at +0x10/+0x12/+0x14 and the single
 *       `addiu $a0,$a0,0x68` is emitted as an ordinary add - target exactly.
 *       (A `do { ... goto ... } while (0)` wrapper re-creates the notes and
 *       measures 4 again; a plain do/while measures 4; both banked.)
 *   (b) Losing that loop's notes also loses the counter's flow.c loop-depth
 *       ref weighting, so global.c then allocates the func_800417D0 walker
 *       BEFORE the counter and the whole callee-saved bank shifts up one seat
 *       (an extra $s8, 274 insns, score 111). In the original that walker and
 *       the arg4 pointer are ONE variable - target's $s1 holds arg4 through
 *       case 0 and then walks s3 by 0x68 - so `s1` carries both, restoring
 *       target's allocation order (counter -> $s0, s1 -> $s1) and closing the
 *       last 4 diffs.
 *   Also: the prologue reads the table entry into `s32 ent`, null-checks it and
 *   casts to `s4`. The one-variable spelling (`s4 = (u8*)D_800A9A10[a0];
 *   if (s4 == 0) return;`) measures 4 - banked in rejected/.
 * Apply: replace the func_80040D48 region in src/text1a_pre.c (incl. the
 * typedef/extern prelude) with this file.
 */
typedef void (*FuncPtr_40D48)(s16 *, s16 *);
typedef struct { s32 a, b, c, d, e, f, g, h; } Copy8_40D48;
extern s32 D_800A9A10[];
extern s32 D_80094CFC[];
extern s32 D_800A3820;
extern FuncPtr_40D48 D_800F66A0[];
extern void func_800417D0(s32 *);
extern void func_800400B0(s32 *, s32);
extern void func_8003F62C(s32 *);
extern void func_800420E8(s32, s32);
void func_80040D48(s32 a0, s32 a1, s32 *a2, s16 *a3, s16 *arg4, s32 arg5) {
    u8 *s4;
    u8 *s5;
    u8 *s3;
    u8 *s2;
    s32 s0;
    s16 *s1;
    s32 ent;

    ent = D_800A9A10[a0];
    if (ent == 0) {
        return;
    }
    s4 = (u8 *)ent;

    *(s16 *)(s4 + 0x3C) = a3[0];
    *(s16 *)(s4 + 0x3E) = a3[1];
    *(s16 *)(s4 + 0x40) = a3[2];

    *(s32 *)(s4 + 0x78) = a2[0];
    s5 = s4 + 0x2C;
    *(s32 *)(s4 + 0x7C) = a2[1];
    s3 = s4 + 0x94;
    *(s32 *)(s4 + 0x80) = a2[2];

    s2 = s4 + 0x7E4;

    switch (a1) {
    case 0: {
        s32 *tbl;
        u8 *a4p;
        u8 *p;
        FuncPtr_40D48 *s0_fn;
        s0 = 1;
        tbl = D_80094CFC;
        s1 = arg4;
        a4p = s3 + 0x68;

        *(s32 *)(s3 + 0x4C) = 0;
        *(s32 *)(s3 + 0x50) = 0;
        *(s32 *)(s3 + 0x54) = 0;
        *(s16 *)(s3 + 0x10) = 0;
        *(s16 *)(s3 + 0x12) = 0;
        *(s16 *)(s3 + 0x14) = 0;

        // Label + backward goto, not a do/while: target's store pointer here is
        // the induction variable itself (base s3+0x68, stores at +0x10/+0x12/
        // +0x14, one addiu 0x68). With loop notes, loop.c's combine_givs merges
        // the three dest-address givs onto the last one (benefit 6, lifetime 3)
        // and the "not worth while" test can never fire, so it always
        // strength-reduces to base s3+124 with stores at -4/-2/0 (measured
        // score 4; a do{...}while(0) wrap around this goto measures 4 too).
        {
            s32 idx;
        initloop:
            idx = *tbl;
            *(s16 *)(a4p + 0x10) = *(u16 *)((u8 *)s1 + idx * 6);
            idx = *tbl;
            *(s16 *)(a4p + 0x12) = -(s16)*(u16 *)((u8 *)s1 + idx * 6 + 2);
            idx = *tbl;
            s0++;
            tbl++;
            *(s16 *)(a4p + 0x14) = -(s16)*(u16 *)((u8 *)s1 + idx * 6 + 4);
            a4p += 0x68;
            if (s0 < 0x12) goto initloop;
        }

        s0 = 0x11;
        p = s3 + 0x6E8;
        do {
            *(s16 *)(p + 6) = 0;
            s0--;
            p -= 0x68;
        } while (s0 >= 0);

        *(s32 *)(s2 + 0x4C) = *(s16 *)((u8 *)s1 + 0x6C);
        *(s32 *)(s2 + 0x50) = -(s32)*(s16 *)((u8 *)s1 + 0x6E);
        *(s32 *)(s2 + 0x54) = -(s32)*(s16 *)((u8 *)s1 + 0x70);
        *(s16 *)(s2 + 0x10) = *(u16 *)((u8 *)s1 + 0x72);
        s0_fn = D_800F66A0;
        *(s16 *)(s2 + 0x12) = -(s16)*(u16 *)((u8 *)s1 + 0x74);
        *(s16 *)(s2 + 0x14) = -(s16)*(u16 *)((u8 *)s1 + 0x76);

        (*s0_fn)((s16 *)(s2 + 0x10), (s16 *)(s2 + 0x38));

        *(s32 *)(s2 + 0xB4) = *(s16 *)((u8 *)s1 + 0x78);
        *(s32 *)(s2 + 0xB8) = -(s32)*(s16 *)((u8 *)s1 + 0x7A);
        *(s32 *)(s2 + 0xBC) = -(s32)*(s16 *)((u8 *)s1 + 0x7C);
        *(s16 *)(s2 + 0x78) = *(u16 *)((u8 *)s1 + 0x7E);
        *(s16 *)(s2 + 0x7A) = -(s16)*(u16 *)((u8 *)s1 + 0x80);
        *(s16 *)(s2 + 0x7C) = -(s16)*(u16 *)((u8 *)s1 + 0x82);

        (*s0_fn)((s16 *)(s2 + 0x78), (s16 *)(s2 + 0xA0));
        break;
    }
    case 1:
        *(s16 *)(s3 + 0x10) = 0;
        *(s16 *)(s3 + 0x12) = 0;
        *(s16 *)(s3 + 0x14) = 0;
        *(s16 *)(s3 + 0x06) = 0;
        *(s32 *)(s3 + 0x4C) = 0;
        *(s32 *)(s3 + 0x54) = 0;
        break;
    case 2: break;
    case 3: break;
    case 4: break;
    case 5: break;
    case 6: break;
    }

    {
        s32 scaled;
        // s1 (the arg4 pointer above) carries this walker too: target's $s1
        // holds arg4 through case 0 and is then redefined by addu $s1,$s3,$zero
        // for this walk. A separate local for it takes a ninth callee-saved
        // register and shifts the whole bank (measured 274 insns, score 111).
        scaled = (*(s32 *)(s3 + 0x50) * *(s16 *)(s4 + 0x12)) >> 12;
        s0 = 0;
        s1 = (s16 *)s3;
        *(s32 *)(s3 + 0x50) = scaled;
        *(s16 *)(s5 + 6) = 0;
        do {
            func_800417D0((s32 *)s1);
            s0++;
            s1 = (s16 *)((u8 *)s1 + 0x68);
        } while (s0 < 0x12);
    }

    s0 = 1;
    *s3 = 0xA;
    *(s32 *)(s3 + 0x58) = (s32)(s4 + 0x18F4);
    {
        s32 *list;
        u8 *a4p;
        list = (s32 *)D_800A3820;
        a4p = s3 + 0x68;
        *(s16 *)(s3 + 2) = 0;
        D_800A3820 = (s32)(list + 1);
        *list = (s32)s3;

        do {
            if (*(s16 *)(a4p + 2) >= 0) {
                s32 *list2;
                list2 = (s32 *)D_800A3820;
                D_800A3820 = (s32)(list2 + 1);
                *list2 = (s32)a4p;
            }
            s0++;
            a4p += 0x68;
        } while (s0 < 0x12);
    }

    {
        u8 *a2p;
        u8 *a3p;
        a2p = s4 + 0x10D4;
        a3p = s4 + 0x10EC;
        // Label + backward goto: a while/for spelling gives loop.c notes, the
        // loop rotates to a bottom test and cse folds the first-iteration load
        // to an s4-relative one (measured 273 insns, score 7).
    copyloop:
        {
            s5 = *(u8 **)(a3p + 0x40);
            if (s5 == 0) goto copydone;

            *(Copy8_40D48 *)a3p = *(Copy8_40D48 *)(s5 + 0x18);

            {
                s32 *list3;
                list3 = (s32 *)D_800A3820;
                a3p += 0x68;
                D_800A3820 = (s32)(list3 + 1);
                *list3 = (s32)a2p;
            }
            a2p += 0x68;
            goto copyloop;
        }
    copydone:;

        a2p = s4 + 0x8B4;
        if (*(s16 *)(s4 + 0x8B6) != -1) {
            do {
                s32 *list4;
                list4 = (s32 *)D_800A3820;
                D_800A3820 = (s32)(list4 + 1);
                *list4 = (s32)a2p;
                a2p += 0x68;
            } while (*(s16 *)(a2p + 2) != -1);
        }
    }

    func_800404A0((s16 *)(s4 + 0x8B4), arg5);
    *(s16 *)(s4 + 0x1A84) = (s16)arg5;
    func_800400B0((s32 *)s4, arg5);
    func_8003F62C((s32 *)s4);
    func_800420E8(a0, (s32)(s3 + 0x2C));
}
