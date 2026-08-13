/* REJECTED (session 9, escalation modality) — RMW-window-only alias split.
 *
 * This is the ONE probe the s8 forced-decomposition theorem left inductive:
 * premise (3) said "every pseudo carrying a proper subset of the twelve
 * arg0 buffer accesses must be live across a call, and therefore costs a
 * fourth callee-save." That premise rested on four measured splits, not on
 * an exhaustive enumeration, and the target contains exactly one call-free
 * window per block — the tail RMW pair
 *     lw $v0,0x14($sN); addiu $v0,$v0,0xC; sw $v0,0x14($sN)
 * A split confined to that window would need no callee-save at all.
 *
 * FORM: a block-scoped PURE alias of the parameter (`p = arg0;` / `q = arg0;`)
 * created after AddPrim and dead by the end of the block, carrying only the
 * two RMW references. Pure alias (not `&arg0[5]`) so the 0x14 displacement
 * is preserved.
 *
 * RESULT — KILLED, and it closes premise (3) by construction rather than by
 * enumeration. cse DELETES a call-free pure alias outright: the post-RA
 * allocno table is byte-identical to the base —
 *     pseudo 72 (arg0) nrefs=13 livelen=98 pri=3979 -> $s0
 *     pseudo 73 (walk) nrefs=5  livelen=91 pri=1098 -> $s1
 * 116 insns, 38 diff lines: exactly the base's numbers, i.e. the alias never
 * reaches register allocation and moves ZERO references off the parameter.
 *
 * Combined with s8(a) (a CALL-CROSSING pure alias survives cse and becomes a
 * fourth call-crossing pseudo at 7 refs / pri 2978 that takes $s0 itself,
 * 119 insns / frame 0x60), the alias axis is now closed at BOTH endpoints:
 * an alias that is cheap enough to avoid a callee-save is deleted before RA,
 * and an alias that survives to RA is expensive enough to need one. There is
 * no middle. Premise (3) of the forced-decomposition theorem is therefore
 * deductive, not inductive.
 *
 * Measured with tmp/grind/func_80078654/s5/eval.sh (instrumented cc1
 * BB2_ALLOC_DEBUG allocno table + full-pipeline objdump diff against the s4
 * rig's target.o); log at tmp/grind/func_80078654/s5/r1/alloc.log.
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
        s32 *p;
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
        p = arg0;
        p[5] = p[5] + 0xC;
    }
    s.cd_flag = 0;
    goto check;
loop:
    {
    s32 *q;
    s.a = var_s0[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    s.c = arg0[3];
    arg0[3] = func_8007352C(&s.a);
    SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
    AddPrim(D_800A374C + (s.f * 4), arg0[5]);
    var_s0++;
    q = arg0;
    q[5] = q[5] + 0xC;
    }
check:
    if (var_s0[1] != -1) goto loop;
}
