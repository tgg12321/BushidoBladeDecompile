/* REJECTED (session 11) — loop-note wrap at a NON-SELF-FILLING site.
 *
 * WHY IT IS DEAD: the reference lift itself WORKS (walk pseudo 73 reaches
 * nrefs 13-14 and takes $s0, parameter pseudo 72 takes $s1 — the callee-save
 * inversion the ledger chased for ten sessions), but a NOTE_INSN_LOOP_BEG/END
 * pair is a hard scheduling boundary for sched2, so the wrapped statement
 * loses the neighbour insn that would have filled an adjacent delay slot and
 * the build comes out ONE instruction long (117 vs the target's 116, the extra
 * insn being a load-delay `#nop`). Measured this session at every
 * non-self-filling site (chassis + eval outputs in tmp/grind/func_80078654/s11
 * and tmp/grind/func_80078654/s5/s11_*):
 *   w2   var_s0++ wrapped, depth 4            -> 117 insns,  5 difflines, sandbox 2
 *   w13  ditto, increment moved after arg0[5] -> 117 insns,  5 difflines
 *   w18  increment + loop test wrapped, d3    -> 117 insns,  5 difflines
 *   w6   loop-top read alone, depth 8         -> 117 insns,  7 difflines
 *   w8   loop-top read d4 + loop test d4      -> 117 insns,  7 difflines
 *   w7   walk init wrapped, depth 8           -> 117 insns, 33 difflines
 *   w10/w22-w24 increment hoisted into the read region -> 117-118 insns
 *   w17  increment wrapped at depth 1 ONLY    -> 117 insns (cost is intrinsic
 *        to the note pair, not to the nesting depth — this variant does not
 *        even lift the references far enough to flip the allocation)
 *
 * THE FIX (see candidate.c, sandbox 0): make the wrapped region SELF-FILLING —
 * extend it to `s.a = var_s0[0]; s.b = s.a + 0xC; s.h = -D_800A3608;` so the
 * `s.h` global load sits inside the region and fills the walk read's load-delay
 * slot from within. That form builds 116 == 116 with zero differing insns.
 *
 * Representative form below: the depth-4 wrap around `var_s0++` (w2), which is
 * the cheapest of the dead family — honest sandbox floor 2, the whole residual
 * being the one unfilled load-delay slot at the loop tail.
 */
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;

extern s32 func_8006E480(s32, s32);
extern s32 func_8007352C(s32 *);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 D_800A3608;
extern s32 *D_800A3610;
extern s32 D_800A374C;

typedef struct {
    s32 a;
    s32 b;
    s32 c;
    s32 d;
    s32 e;
    s32 f;
    s32 g;
    s32 h;
    s32 i;
    s32 j;
    u8 cd_flag;
    u8 r;
    u8 g_;
    u8 b_;
} S78654;

void func_80078654(s32 *arg0) {
    S78654 s;
    s32 *var_s0;
    s32 zero;

    zero = 0;
    s.f = 2;
    s.cd_flag = 0;
    s.e = 0;
    s.g = 0;
    s.a = D_800A3610[0xF];
    s.h = 0;
    s.b = s.a + 0xC;
    var_s0 = D_800A3610 + 5;
    if (D_800A3608 >= 0xAAA) {
        if (D_800A3608 >= 0xB04) {
            s16 sv;
            s.cd_flag = 1;
            sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
            if (sv < 0) {
                sv = 0;
            }
            s.r = (s.g_ = (s.b_ = (u8) sv));
        }
        s.c = arg0[3];
        arg0[3] = func_8007352C(&s.a);
        SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
        AddPrim(D_800A374C + (s.f * 4), arg0[5]);
        arg0[5] = arg0[5] + 0xC;
    }
    s.cd_flag = 0;
    goto check;
loop:
    s.a = var_s0[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    s.c = arg0[3];
    arg0[3] = func_8007352C(&s.a);
    SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
    AddPrim(D_800A374C + (s.f * 4), arg0[5]);
    do { do { do { do { var_s0++; } while (0); } while (0); } while (0); } while (0);
    arg0[5] = arg0[5] + 0xC;
check:
    if (var_s0[1] != -1) goto loop;
}
