#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"
#include "gte.h"

extern s16 Judge[];

extern s32 func_8005C2A8(s32 *, s16, s32);

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* --- Functions from text1b segment (0x80047ED0 - 0x80079A30) --- */

void func_80047ED0(s32 a0) {
    D_800A33D0 = (s16 *)((u8 *)D_800A33D0 + a0);
}

void func_80047EE8(s32 arg0, s32 arg1)
{
    /* FAKE: unwritten leading frame pad (phantom-frame-slot volatile pad local
     * family, owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md:390).
     * Mechanism: GCC 2.7.2 function.c assign_stack_local reserves the array slot at
     * RTL-expand from the source DECL and never reclaims frame_offset after DCE, so a
     * declared-but-untouched local aggregate reproduces target's allocated-but-unwritten
     * 32-byte vars region (.frame $sp,72 - args 0x00-0x17, vars 0x18-0x37, regs
     * 0x38-0x47; ZERO sw/lw in 0x18-0x37 - frame forensics in
     * memory/grind/func_80047EE8/evidence.md [s6]/[s7], cc1 size-pin puts the original
     * aggregate at 7-8 words). Lever-exhaustion: 9 structural .frame variants (s3),
     * ~26,500 permuter iters across two distinct basins (s4/s5), forensics (s6/s7),
     * rederive (s8/s9) - every honest producer measured inert; see hypotheses.md.
     * SOTN-master precedent: volatile u32 pad; // !FAKE: at src/st/sel/2C048.c:564
     * (docs/reference/sotn-construct-index.md:101); volatile u32 pad[4]; // FAKE at
     * src/st/sel/stream.c:80 (sotn-construct-index.md:103). */
    volatile u32 pre_pad[8];
    u32 *p;
    s32 saved;
    s16 new_var;
    s32 count;
    u32 v_off;
    unsigned int new_var2;
    p = (u32 *) arg0;
    saved = (s32) p;
    arg0 = 0; /* FAKE: dead store to a PARAM (dead-store-fake-exception family,
               * .claude/rules/dead-store-fake-exception.md). Mechanism: defeats cse2's
               * canonical-register substitution over the {arg0, p, saved} equivalence
               * class so the second pointer binds addu $s0,$s2,$v0 rather than $a0.
               * Lever-exhaustion: 6 pure spellings of this init chain measured dead on
               * this body at s2 (rejected/pure-*.c). */
    p = (u32 *) ((s32) p + (((s32) (arg1 << 16)) >> 14));
    v_off = *p;
    p = (u32 *) (saved + ((v_off >> 2) << 2));
    count = *(p++);
    if (count != 0)
    {
        count--;
        do
        {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            s32 first;
            word = *p;
            p = (u32 *) (((s32) p) + 4);
            a1v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a2v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a3v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            new_var = a1v;
            new_var2 = word >> 2;
            first = saved + (new_var2 << 2);
            v0v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            func_800482C8(first, new_var, a2v, a3v, v0v);
        }
        while ((count--) != 0);
    }
}
void func_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    volatile u32 pre_pad[8]; // !FAKE: phantom-frame-slot volatile filler (owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md): target reserves 32 locals bytes at sp+0x18..sp+0x37 that no instruction touches; mechanism: GCC 2.7.2 get_frame_size reserves declared locals
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: defeats cse2 canonical-reg substitution
                 that folds {reg 72 arg0, reg 78 p, reg 79 base_addr}
                 equivalence class at insn 36 - RTL-proven s6 */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            func_800482C8(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg2,
                          (s32)v0v + sx_arg3);
        } while ((count--) != 0);
    }
}
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    volatile u32 pre_pad[8]; // !FAKE: phantom-frame-slot volatile filler (owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md:422-434; grant row engine/volatile_cheats.py:779, commit 661c01ef): the target reserves 32 locals bytes at sp+0x18..sp+0x37 that no instruction in asm/funcs/func_800480C0.s reads or writes; mechanism: GCC 2.7.2 get_frame_size/expand_decl reserves declared locals (config/mips/mips.c:4443-4475); lever-exhaustion: memory/grind/func_800480C0/hypotheses.md s1-s23, 104 rejected forms, every referenced producer costs >=1 store (flow.c:1740-1741 never deletes the last store to a frame object)
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; // !FAKE: dead store to a PARAMETER (sanctioned dead-store family, .claude/rules/dead-store-fake-exception.md; identical construct in the matched sibling func_80047FBC at src/text1b.c:91). It defeats cse2's canonical-register substitution, which otherwise folds the {arg0, p, base_addr} equivalence class and emits one base copy instead of two; mechanism: GCC 2.7.2 cse.c canonical-reg substitution; lever-exhaustion: hypotheses.md s1-s3
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        s32 sx_arg4;
        s32 sx_arg5;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        sx_arg4 = arg4;
        sx_arg5 = arg5;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            func_800482C8(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          (s32)v0v + sx_arg5);
        } while ((count--) != 0);
    }
}
/* func_800481E8 — COMPLETED-C 2026-08-22 (commit 272e47c4, layer-2 PASS;
 * allowlist row granted per the parked-but-proven audit, ruling cbcfda04).
 * Two annotated constructs, both in sanctioned families:
 *  - `volatile u32 pre_pad[8];` — phantom-frame-slot volatile pad (owner
 *    ruling 2026-08-18), ARRAY form, first-decl, engine allowlist row in
 *    engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS.
 *  - `arg0 = 0;` — dead-store-fake-exception (dead store to a param), same
 *    lever Judge-PASSed on the in-file sibling func_80047EE8.
 * Full derivation: docs/grind/decisions.md 2026-08-20/22 entries + this
 * block's pre-completion history in git (272e47c4^). */
void func_800481E8(s32 arg0, s32 arg1)
{
    /* Pure C (s1 recon): the sibling InitHiraRmd_80047FBC prologue technique
     * transfers — base-copy staging + function-scope precompute makes GCC
     * stage arg0 through $s0 first ($s0=$a0; $s2=$s0), replacing the former
     * INLINE_MOVE_ALIASING __asm__ + $16 pin. The `arg0 = 0;` dead store is
     * load-bearing FAKE-family (dead-store-fake-exception, sibling
     * precedent in-file): without it GCC keeps arg0 live in $a0 and emits
     * `addu $s0,$a0,$v0` (measured, s1 probe); with it the second pointer
     * binds to base in $s2 — matching target.
     */
    /* FAKE: unwritten leading frame pad (phantom-frame-slot volatile pad local
     * family, owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md:390).
     * Mechanism: GCC 2.7.2 function.c assign_stack_local reserves the array slot at
     * RTL-expand from the source DECL and never reclaims frame_offset after DCE, so a
     * declared-but-untouched local aggregate reproduces target's allocated-but-unwritten
     * 32-byte vars region (.frame $sp,72 - args 0x00-0x17 incl the 5th-arg slot
     * sw $v0,0x10($sp); vars 0x18-0x37 with ZERO sw/lw; regs 0x38-0x47). Identical
     * shape and size to the two granted siblings in this file (func_80047EE8 /
     * func_80047FBC, owner ruling 2026-08-20). Lever-exhaustion: recon (s1),
     * structural entry-condition bisection + 11-variant .frame grid (s2), AND-gate
     * re-evaluation (s3), ~152k permuter iterations across three independent seeds
     * (s2/s4) - every honest producer measured inert; see hypotheses.md.
     * SOTN-master precedent: volatile u32 pad; // !FAKE: at src/st/sel/2C048.c:564
     * (docs/reference/sotn-construct-index.md:101); volatile u32 pad[4]; // FAKE at
     * src/st/sel/stream.c:80 (sotn-construct-index.md:103). */
    volatile u32 pre_pad[8];
    u32 *p;
    u32 *base;
    s32 count;
    s32 a0_for_call;
    p = (u32 *)arg0;
    base = p;
    arg0 = 0; /* FAKE: breaks $a0==base association, see block comment */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)((s32)base + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        count--;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            u16 v0v;
            unsigned int new_var2;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = *((u16 *)p);
            new_var2 = word >> 2;
            a0_for_call = (s32)base + (new_var2 << 2);  /* compute early (target sched) */
            p = (u32 *)(((s32)p) + 2);  /* always advance (target delay slot) */
            if ((s32)a3v < 0x280) {
                v0v += 1;
            }
            func_800482C8(a0_for_call,
                          (s32)a1v,
                          (s32)a2v,
                          (s32)a3v,
                          (s32)(s16)v0v);
        } while ((count--) != 0);
    }
}
void func_800482C8(u8 *arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4) {
    u16 arg4_lo = *(u16 *)&arg4;
    s16 rect[4];
    s16 buf[512];
    u8 *p_alt;
    s32 flags;
    u32 dim;
    u32 dim2;
    s32 head = arg0[0];
    arg0 += 4;
    if (head != 0x10) return;
    flags = *(s32 *)arg0 & 8;
    arg0 += 4;
    if (flags != 0) {
        p_alt = arg0;
        arg0 = p_alt + (((u32)*(u32 *)p_alt >> 2) << 2);
        p_alt += 4;
    }
    arg0 += 8;
    rect[0] = arg1;
    rect[1] = arg2;
    dim = *(u32 *)arg0;
    rect[3] = dim >> 16;
    rect[2] = dim;
    LoadImage(rect, (s32 *)(arg0 + 4));
    if (flags == 0) return;
    p_alt += 4;
    rect[0] = arg3;
    rect[1] = arg4_lo;
    dim2 = *(u32 *)p_alt;
    p_alt += 4;
    rect[3] = dim2 >> 16;
    rect[2] = dim2;
    if (func_800486FC() != 0) {
        math_GrayscaleRgb555((s32)p_alt, rect[2], (s32)buf);
        LoadImage(rect, (s32 *)buf);
        DrawSync(0);
        return;
    }
    LoadImage(rect, (s32 *)p_alt);
}


void func_800483DC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    s32 base;
    s32 count;
    s32 off;
    arg1 = (((s32)(arg1 << 16)) >> 14) + arg0;
    base = arg0;
    off = *(s32 *)arg1;
    arg0 += off;
    count = *(s32 *)arg0;
    arg0 += 4;
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            s32 entry;
            u32 dx_u;
            u32 dy_u;
            s32 dx;
            s32 dy;
            entry = base + *(s32 *)arg0;
            arg0 += 8;
            dx_u = *(u16 *)arg0;
            arg0 += 2;
            dy_u = *(u16 *)arg0;
            arg0 += 2;
            dx = ((s32)(dx_u << 16)) >> 16;
            dy = ((s32)(dy_u << 16)) >> 16;
            func_800484A0(entry, dx + sx_arg2, dy + sx_arg3);
        } while ((count--) != 0);
    }
}
void func_800484A0(u8 *arg0, s16 arg1, s16 arg2) {
    s16 rect[4];
    s16 buf[512];
    u32 dim;
    s32 flags;
    if (arg0[0] != 0x10) return;
    arg0 += 4;
    flags = *(s32 *)arg0;
    arg0 += 4;
    if ((flags & 8) == 0) return;
    arg0 += 8;
    rect[0] = arg1;
    rect[1] = arg2;
    dim = *(u32 *)arg0;
    arg0 += 4;
    rect[3] = dim >> 16;
    rect[2] = dim;
    if (func_800486FC() != 0) {
        math_GrayscaleRgb555((s32)arg0, rect[2], (s32)buf);
        LoadImage(rect, (s32)buf);
        return;
    }
    LoadImage(rect, (s32)arg0);
}
extern void func_800485EC();
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base, count, entry, a, b, c, d, off;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    /* FAKE: operand order chosen to match target (off + base, not base + off);
     * mechanism: RTL expansion's commutative-operand canonicalization
     * (expand_binop) keeps two equal-precedence pseudos in source order, and
     * no later pass (combine/sched) reorders the addu operands — so only the
     * off-first source spelling emits target's `addu $v1,$v0,$v1`;
     * lever-exhaustion: memory/grind/func_80048530/ s1-s4 — every natural
     * ordering and every non-swap off-first spelling measured dead
     * (natural base+off = 1 insn off; off+=base / mem-inline / fresh-walker
     * misroute the walker, scores 22/20/12; cc1psx also emits base-first from
     * the natural order); sanctioned by the 2026-08-20 owner ruling in
     * .claude/rules/or-tree-shape-shift.md (single justified target-matching
     * operand order). */
    arg0 = off + base;
    count = *(s32 *)arg0;
    arg0 += 4;
    if (arg2 >= (u32)count) return -1;
    arg0 += arg2 * 0xC;
    entry = *(s32 *)arg0;
    arg0 += 4;
    a = (s32)*(u16 *)arg0;
    arg0 += 2;
    b = (s32)*(u16 *)arg0;
    arg0 += 2;
    c = (s32)*(u16 *)arg0;
    arg0 += 2;
    d = (s32)*(u16 *)arg0;
    entry += base;
    func_800485EC(entry, arg3, (s16)a, (s16)b, (s16)c, (s16)d);
    return count;
}
typedef struct {
    /* 0x00 */ s16 mode;
    /* 0x02 */ s16 x;
    /* 0x04 */ s16 y;
    /* 0x06 */ s16 w;
    /* 0x08 */ s16 h;
    /* 0x0A */ s16 cx;
    /* 0x0C */ s16 cy;
    /* 0x0E */ s16 cw;
    /* 0x10 */ s16 ch;
    /* 0x12 */ u16 tpage;
    /* 0x14 */ u16 clut;
    /* 0x18 */ u32 *pixdata;
    /* 0x1C */ u32 *clutdata;
} TimHdr485;
extern u32 GetTPage(s32, s32, s32, s32);
extern u16 GetClut(s32, s32);
void func_800485EC(tim, spr, x, y, cx, cy)
u32 *tim;
TimHdr485 *spr;
s16 x, y;
u16 cx, cy;
{
    u32 flag;
    u32 *p;

    if (*(u8 *)tim++ == 0x10) {
        flag = *tim++;
        spr->mode = flag & 7;
        if (flag & 8) {
            u32 bnum;

            p = tim;
            bnum = *p;
            spr->cx = cx;
            spr->cy = cy;
            tim = p + (bnum >> 2);
            p += 2;
            spr->ch = ((u16 *)p)[1];
            spr->cw = *p++;
            spr->clutdata = p;
            spr->clut = GetClut(spr->cx, spr->cy);
        } else {
            spr->clut = 0;
        }
        p = tim + 2;
        /* !FAKE: cancellation pair (sanctioned family: semantically-null
         * fabricated statement pair, .claude/rules/no-new-park-categories.md:370-382,
         * owner ruling 2026-08-18, F6 ESTABLISHED — exact `i++; i--;` shape).
         * What: net-zero adjacent same-variable inc/dec of tim, byte-free
         * (survives cse1/cse2, then flow.c dead-store elimination deletes both:
         * NOTE_INSN_DELETED in .flow dump, tmp/grind/func_800485EC/dumps/).
         * Mechanism: cse.c fold_rtx PLUS-association (cse.c:5589-5666, applied
         * uncosted to addresses via find_best_addr, cse.c:2663) rewrites the
         * pixel-block reads onto tim whenever p's recorded equivalent
         * (plus tim 8) is valid; the pair bumps reg_tick(tim) so exp_equiv_p
         * invalidates that equivalence and the reads keep p as base, matching
         * target's addiu v1,s1,8 + lhu 2(v1)/lw 0(v1)/addiu v1,v1,4.
         * Lever-exhaustion: memory/grind/func_800485EC/hypotheses.md s1-s2 —
         * natural fresh-def folds (7), tim-walker misallocates to s1 (39),
         * live tim->pixdata routing cascades (22), def-in-arms leaves two
         * unmergeable addius (3), full-tail duplication into arms (16); the
         * cse.c mechanism proof shows every join-local p==tim+K chain folds. */
        tim++;
        tim--;
        spr->x = x;
        spr->y = y;
        spr->h = ((u16 *)p)[1];
        spr->w = *p++;
        spr->pixdata = p;
        spr->tpage = GetTPage(spr->mode, 0, spr->x & 0xFFC0, spr->y & 0xFF00);
    }
}
extern s16 g_color_mode;
s32 file_GetFlag0(void);
s16 func_800486FC(void) {
    if (file_GetFlag0()) {
        g_color_mode = 1;
    } else {
        g_color_mode = 0;
    }
    return g_color_mode;
}
extern s16 g_color_mode;
void func_80048744(s32 a0) {
    if (a0) {
        g_color_mode = 1;
    } else {
        g_color_mode = 0;
    }
}
void math_GrayscaleRgb555(u16 *arg0, s32 arg1, u16 *arg2) {
    s32 temp_a3;
    s32 temp_v1;
    s32 var_t0;
    u16 *var_t1;
    s32 temp_a1;
    var_t1 = arg0;
    var_t0 = arg1 - 1;
    if (var_t0 != -1) {
        do {
            temp_a1 = *var_t1;
            var_t1 += 1;
            var_t0 -= 1;
            temp_v1 = temp_a1 << 0x10;
            {
                s32 b;
                s32 g;
                s32 bt;
                temp_a3 = temp_a1 & 0x1F;
                temp_a3 = temp_a3 * 0x547;
                b = (temp_v1 >> 0x1A) & 0x1F;
                g = (temp_v1 >> 0xA) & 0xF800;
                bt = b * 0x2B8;
                temp_a3 = ((temp_a3 + g + bt) >> 0xC) & 0x1F;
            }
            *arg2 = (u16) ((((temp_a1 & (~0x7FFF)) + (temp_a3 << 0xA)) + (temp_a3 << 5)) + temp_a3);
            arg2 += 1;
        } while (var_t0 != (-1));
    }
}
s32 math_Grayscale3(s32 arg0, s32 arg1, s32 arg2) {
    arg0 = arg0 * 0x547;
    arg1 = arg1 << 11;
    arg2 = arg2 * 0x2B8;
    return (arg0 + arg1 + arg2) >> 12;
}

void func_80048864(s32 mode, s32 sx, s32 sy, s32 w, s32 mr, s32 mg, s32 mb, s32 dx, s32 dy) {
    u16 buf[256];
    u16 out[256];
    s16 rect[4];
    u16 *src;
    u16 *dst;
    s32 i;
    u16 p;
    s32 r, g, b, a;

    DrawSync(0);
    rect[0] = sx;
    rect[1] = sy;
    rect[2] = w;
    rect[3] = 1;
    StoreImage(rect, buf);
    DrawSync(0);
    src = buf;
    dst = out;
    for (i = 0; i < w; i++) {
        p = *src;
        if (p == 0) {
            *dst++ = *src++;
            continue;
        }
        r = (p & 0x1F) << 3;
        g = ((p >> 5) & 0x1F) << 3;
        b = ((p >> 10) & 0x1F) << 3;
        a = p & 0x8000;
        src++;
        switch (mode) {
        case 0:
            r = (r * mr) >> 15;
            g = (g * mg) >> 15;
            b = (b * mb) >> 15;
            break;
        case 1:
            r = r * 0x547;
            g = g << 11;
            b = b * 0x2B8;
            r = (r + g + b) >> 15;
            r = (r * mr) >> 12;
            g = (r * mg) >> 12;
            b = (r * mb) >> 12;
            break;
        }
        r &= 0x1F;
        g &= 0x1F;
        b &= 0x1F;
        *dst++ = a | r | (g << 5) | (b << 10);
    }
    rect[0] = dx;
    rect[1] = dy;
    LoadImage(rect, out);
    DrawSync(0);
}
void func_80048A7C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    func_80048864(0, arg0, arg1, arg2, arg3, arg4, arg5, arg0, arg1);
}
extern s32 *func_800467B8(s32); /* corrected to the definition (src/sound.c:134) — owner ruling 2026-08-24, escalation packet func_80048AD0 */
extern s32 func_800468B0(s32);
extern u8 D_80099BCC;
extern s32 D_800A33E0;
extern s32 D_800A33E4;
extern s32 func_8004153C(s32);
s32 func_80048AD0(s32 arg0) {
    s32 temp_v0;
    s32 sound;
    s32 idx;
    u8 *base;
    s32 delta;
    u8 *p;
    u8 *q;

    temp_v0 = func_8004153C(arg0);
    if (temp_v0 == 0) return 0;
    idx = *(s16 *)(temp_v0 + 8);
    D_800A33E0 = arg0;
    sound = (&D_80099BCC)[idx];
    if (sound == 0xFF) return 0;
    base = (u8 *)func_800467B8(sound);
    p = base + ((*(u32 *)(base + 8) >> 2) << 2);
    delta = (s32)(p - base);
    D_800A33E4 = (s32)p;
    q = p + 0xA;
    /* FAKE: the record counter reuses `sound` rather than a fresh local.
       snd_LoadBgm's argument copy gives `sound` a hard-reg $a0 preference;
       global.c expand_preferences propagates it to the counter, which stops
       prune_preferences making the counter yield $a0 to `delta`. With a
       separate counter the pair allocates $a2/$a0 instead of target's
       $a0/$a2. Measured exhaustion: ~60 variants over 8 sweeps + kills in
       grind s1/s2 — see memory/grind/func_80048AD0/evidence.md. */
    for (sound = 0; sound < 0x11; sound++) {
        *(s16 *)(q - 8 + sound * 0x68) = sound;
        *(s16 *)(q - 6 + sound * 0x68) = 9;
        p[sound * 0x68] = 0xF;
        *(s8 *)(q - 9 + sound * 0x68) = 0;
        *(s16 *)(q + sound * 0x68) = (s16)arg0;
    }
    func_800468B0(delta + 0x6E8);
    return 1;
}
extern s32 D_800A33E4;
void func_80048B8C(s32 a0) {
    D_800A33E4 += a0;
}
extern void *game_GetPlayerData();
extern VECTOR *ApplyMatrix(MATRIX *, SVECTOR *, VECTOR *);
extern void math_RotMatrixZYX(SVECTOR *, MATRIX *);
extern void gte_MulMatrix0ClearTrans(MATRIX *, MATRIX *, MATRIX *);
extern s32 ClearOTagR(s32, s32);
extern s32 D_800A36AC;
extern s32 D_800A378C;
extern s32 D_800A3820;
extern s32 g_gpu_ot256_ptr;
extern u8 g_gpu_ot256_db[];
extern s16 D_80099C14[];

void func_80048BA4(s32 arg0, s32 arg1, s32 arg2) {
    MATRIX mtx;
    SVECTOR rot;
    s32 index;
    s32 scale;
    s32 old;
    s16 *indices;
    s32 *ot;
    MATRIX **player;
    u8 *prim;

    player = game_GetPlayerData(D_800A33E0);
    if (player == 0) {
        return;
    }
    if (arg1 >= 6) {
        arg1 = -1;
    }

    rot.vx = 0x1770;
    rot.vy = 0;
    rot.vz = 0;
    scale = 0x1770;
    rot.vz = ((s32)Judge[arg0 & 0xFFF] * scale) >> 12;
    rot.vx = ((s32)Judge[(arg0 + 0x400) & 0xFFF] * scale) >> 12;
    prim = (u8 *)D_800A33E4;
    ApplyMatrix(player[0], &rot, (VECTOR *)D_800FF558.t);
    D_800FF558.t[0] += player[0]->t[0];
    D_800FF558.t[1] += player[1]->t[1];
    D_800FF558.t[2] += player[2]->t[2];

    rot.vx = 0;
    rot.vy = 0xC00 - arg0;
    rot.vz = 0;
    indices = D_80099C14;
    math_RotMatrixZYX(&rot, &mtx);
    gte_MulMatrix0ClearTrans(player[0], &mtx, &mtx);
    D_800FF558.m[0][0] = mtx.m[0][0];
    D_800FF558.m[0][1] = mtx.m[1][0];
    D_800FF558.m[0][2] = mtx.m[2][0];
    D_800FF558.m[1][0] = mtx.m[0][1];
    D_800FF558.m[1][1] = mtx.m[1][1];
    D_800FF558.m[1][2] = mtx.m[2][1];
    D_800FF558.m[2][0] = mtx.m[0][2];
    D_800FF558.m[2][1] = mtx.m[1][2];
    D_800FF558.m[2][2] = mtx.m[2][2];

    goto test_index;
copy_index:
        *(MATRIX *)(prim + 0x18) = *player[index];
        ot = (s32 *)D_800A3820;
        D_800A3820 = (s32)(ot + 1);
        *ot = (s32)prim;
        prim += 0x68;
test_index:
    index = *indices;
    indices++;
    if (index >= 0) {
        goto copy_index;
    }
    if (arg1 >= 0) {
        *(MATRIX *)(prim + 0x18) = *player[18];
        ot = (s32 *)D_800A3820;
        *(s16 *)(prim + 2) = arg1 + 0xF;
        D_800A3820 = (s32)(ot + 1);
        *ot = (s32)prim;
        prim += 0x68;
    }
    if (arg2 != 0) {
        *(MATRIX *)(prim + 0x18) = *player[19];
        ot = (s32 *)D_800A3820;
        *(s16 *)(prim + 2) = 0x15;
        D_800A3820 = (s32)(ot + 1);
        *ot = (s32)prim;
    }

    g_gpu_ot256_ptr = (s32)(g_gpu_ot256_db + ((D_800A36AC & 1) << 10));
    ClearOTagR(g_gpu_ot256_ptr, 0x100);
    old = *(s32 *)D_800A378C;
    *(s32 *)D_800A378C = (g_gpu_ot256_ptr + 0x3FC) & 0xFFFFFF;
    *(s32 *)g_gpu_ot256_ptr = old;
}
extern u8 D_800EF848[];
extern u16 D_80099C34[];
extern void func_80052C10(void);
void func_80048F58(s32 a0, s32 a1) {
    s32 i;
    u16 *src;
    u16 *dst;
    u8 *base;
    if (a1 > 0) {
        func_80052C10();
    }
    base = D_800EF848 + a1 * 308;
    *(u32 *)base = 0;
    src = (u16 *)(D_80099C34 + a0 * 7);
    dst = (u16 *)(base + 0x124);
    i = 0;
    do {
        *dst = *src;
        src++;
        i++;
        dst++;
    } while (i < 7);
}





INCLUDE_ASM("asm/funcs", func_80048FFC);
extern s16 D_800EF9F2;
extern s16 D_800EF9F4;
extern s16 D_800A33EA;
extern s16 D_800A33E8;
extern s32 D_800A33EC;
void func_8004939C(void) {
    s16 val = -1;
    s32 i = 0x39;
    s16 *p = &D_800EF9F2;
    do {
        *p = val;
        i--;
        p--;
    } while (i >= 0);
    D_800EF9F4 = -2;
    D_800A33EA = -1;
    D_800A33E8 = -1;
    D_800A33EC = -1;
}

extern u8 D_80099CC8[];
extern u8 D_80099CC9[];
extern s32 D_800A33EC;
extern s16 D_800EF980[];
void func_800493E4(s32 arg0) {
    u8 temp_v1;
    s32 idx;

    D_800EF980[arg0] = 1;
    if (D_800A33EC == -1) {
        if (arg0 >= 0x33) {
            D_800A33EC = 1;
        } else {
            D_800A33EC = 0;
        }
    } else {
        if (D_800A33EC == 0 && !(arg0 < 0x33)) {
            func_80052C10();
        }
        if (D_800A33EC == 1 && arg0 < 0x33) {
            func_80052C10();
        }
    }
    idx = arg0 * 2;
    temp_v1 = D_80099CC8[idx];
    if (temp_v1 != 0xFF) {
        D_800EF980[temp_v1] = 1;
        /* FAKE: loop notes keep the D_80099CC9 lbu below the first sh (target has
           the unfilled load-delay nop) and keep the shared 1 cached in $v1 */
        do { } while (0);
        D_800EF980[D_80099CC9[idx]] = 1;
    }
}
extern s32 D_800A33EC;
extern s16 D_800A33E8;

void func_800494D4(s32 idx, s32 val) {
    s32 cond;
    if (D_800A33EC == 0) {
        cond = val < 16;
    } else {
        cond = val < 8;
    }
    if (!cond) {
        func_80052C10();
    }
    if (((u32)idx) >= 2U) {
        func_80052C10();
    }
    (&D_800A33E8)[idx] = (s16)val;
}
s32 func_8004954C(s32 arg0, s32 arg1, s32 arg2)
{
    s32 sum = 0;
    s32 i;
    for (i = 0; i < arg1; i++) {
        /* FAKE: do-while(0) loop-note ref weighting flips the sum/i allocno
           priority so sum seats in $v1 and i in $a3 (matches target). */
        do { sum += arg0; arg0 -= 1; } while (0);
    }
    return sum + (arg2 - arg1);
}
extern s16 D_80099C50[];
extern s16 D_800EF980[];
extern s32 D_800A33EC;
extern s16 D_800A33E8;
extern s16 D_800A33EA;
extern s32 D_800A324C;
extern s32 func_8004954C(s32, s32, s32);
extern s32 func_80046020();
extern void func_80045B68(s32, s32, s16 *, s32);
extern s32 func_8003E120();
void func_80049584(s32 arg0) {
    s16 *dst;
    s16 *src;
    s16 *p;
    /* FAKE: `i` carries both the two loop counters and the computed total,
       mechanism: global.c allocno allocation — only a pseudo that crosses a
       CALL is eligible for a call-saved hard reg, so sharing one variable is
       what puts the loop counter in $s0 (target); with a separate `total` the
       counter takes a call-clobbered reg and 12 insns diverge.
       lever-exhaustion: memory/grind/func_80049584/hypotheses.md (H1/H3). */
    s32 i;
    s32 unchanged;
    s32 rank;
    s32 step;
    s32 lo;
    s32 hi;

    unchanged = 1;
    i = 0;
    dst = D_80099C50;
    src = D_800EF980;
    do {
        s16 v = *src;
        if ((v >= 0) != ((*dst) >= 0)) {
            unchanged = 0;
        }
        *dst = v;
        dst++;
        i++;
        src++;
    } while (i < 0x3A);
    rank = 0;
    i = 0;
    p = D_800EF980;
    do {
        if ((*p) >= 0) {
            *p = (s16) rank;
            rank++;
        }
        i++;
        p++;
    } while (i < 0x3A);
    step = 8;
    if (D_800A33EC == 0) {
        step = 0x10;
    }
    hi = D_800A33E8;
    if (hi == -1) {
        lo = D_800A33EA;
        if (lo == hi) {
            i = 0x24;
            if (D_800A33EC == 0) {
                i = 0x88;
            }
            goto end;
        }
        hi = lo;
    } else {
        lo = hi;
        if ((D_800A33EA != (-1)) && (hi != D_800A33EA)) {
            if (hi < D_800A33EA) {
                hi = D_800A33EA;
            } else {
                lo = D_800A33EA;
            }
        }
    }
    i = func_8004954C(step, lo, hi);
end:
    if (D_800A324C != i) {
        D_800A324C = i;
        unchanged = 0;
    }
    if (unchanged == 0) {
        func_80046020();
        func_80045B68(D_800A33EC, i, D_800EF980, arg0);
        func_8003E120();
    }
}
void func_80049710(void) {
}

INCLUDE_ASM("asm/funcs", func_80049718);
extern u8 D_80099CC8[];
extern s16 D_80099D3C[];
extern u8 *D_800A38B4;
extern s16 D_800EF980[];
extern void func_800417D0(s32 *);
/* func_80049A2C - session s10 (rederive) INTEGRATION-HANDOFF FORM - BYTES PROVEN.
 *
 * FULL DRIVER BUILD with this body applied over src/text1b.c:868's
 * INCLUDE_ASM gives SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle
 * (tmp/grind/func_80049A2C/s10/build_P1_oracle_match.log). Object-level
 * word diff vs asm/funcs/func_80049A2C.s: 0 real diffs of 126 instructions
 * (relocation fields masked; tmp/grind/func_80049A2C/s10/bytediff_P1.log).
 * cc1 frame: .frame $sp,48 # vars= 8, regs= 5/0 - target's exact signature,
 * the first time in ten sessions vars=8 and regs=5/0 have coexisted.
 *
 * The ONLY non-ordinary construct is the first declaration:
 *   volatile u32 pre_pad[2]; // !FAKE ...
 * - the phantom-frame-slot volatile pad family, owner ruling 2026-08-18
 * (.claude/rules/no-new-park-categories.md:390): ARRAY form, first-decl
 * position, no (void) shim, volatile-qualified, FAKE-annotated. SOTN-master
 * PSX precedent: docs/reference/sotn-construct-index.md L620/L626/L627
 * (volatile char pad[8] //! FAKE; volatile u32 pad; volatile u32 pad[4]).
 * Working integration precedent: the 2026-08-20 OWNER RULING granting
 * ("pre_pad", 8) rows to text1b.c siblings func_80047EE8 / func_80047FBC,
 * and the same-day func_800481E8 INTEGRATION HANDOFF.
 *
 * Lever-exhaustion (why the pad is unavoidable, measured not argued):
 * s7-s9 proved target's +8 vars region is a phantom slot REACHABLE from
 * ordinary C only via a combine-orphaned pseudo (reload1.c:2404 alter_reg),
 * and s9's exclusion law shows the only fold-capable symbol (D_80099D3C)
 * cannot host it: the fold that creates the orphan shortens the arg1 index
 * chain, flips sched1's hoist, and costs a SIXTH callee-saved register
 * (target saves five). D_800EF980/D_80099CC8 are single-index (CSE merges
 * every respelling). s10 measured the five remaining non-array carriers
 * (H-S9C a-e: vehicle+0x50C, prev-obj across the jal, ot+4 hoist, temp_v1*2
 * across the beq, a1_val+1 intermediate) - all vars=0. The function is
 * loopless, so no back-edge carrier exists. No honest producer of the slot
 * is compatible with target's instruction stream; the sanctioned pad is the
 * documented FAKE carve-out the 2026-07-19/20 Judge constraints anticipated.
 *
 * Sandbox note: scores 12 (frame delta) until the operator adds
 *   "func_80049A2C": frozenset({("pre_pad", 2)}),
 * to engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS (owner-class
 * surface). With the pad honoured the build is byte-identical (SHA1 proof
 * above). Prior Judge-FAILed constructs (dummy[2], new_var4, empty if,
 * inline-assign, new_var3 holder) are all retired from this body.
 * Full record: memory/grind/func_80049A2C/evidence.md + hypotheses.md [s10].
 */
void func_80049A2C(s32 arg0, s32 arg1, s32 arg2) {
    volatile u32 pre_pad[2]; // !FAKE: phantom-frame-slot volatile filler (owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md): target reserves 8 locals bytes at sp+0x10..sp+0x17 that no instruction touches; mechanism: GCC 2.7.2 get_frame_size reserves declared locals
    u8 *new_var6;
    u8 *new_var5;
    s16 *new_var7;
    u8 temp_v1;
    u8 *new_var8;
    s16 *p_anim;
    s16 new_var2;
    s16 *src;
    u8 *obj;
    u8 *vehicle;
    s16 a1_val;
    u8 *ot;

    new_var6 = D_80099CC8;
    {
        u8 *p = new_var6 + (arg0 * 2);
        temp_v1 = p[arg2];
    }
    if (temp_v1 == 0xFF) {
        return;
    }
    new_var8 = (u8 *) D_800EF980;
    p_anim = (s16 *) (new_var8 + (temp_v1 * 2));
    if ((*p_anim) < 0) {
        func_80052C10();
    }
    vehicle = (u8 *) func_8004153C(arg1 >> 1);
    obj = D_800A38B4;
    obj[0] = 0;
    obj[1] = 0;
    a1_val = (*p_anim) * 2;
    *((s16 *) (obj + 4)) = 6;
    *((s16 *) (obj + 8)) = 0;
    *((s16 *) (obj + 0xA)) = 4;
    *((s16 *) (obj + 2)) = a1_val;
    src = &D_80099D3C[(arg1 & 1) * 6];
    *((s32 *) (obj + 0x4C)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((s32 *) (obj + 0x50)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((s32 *) (obj + 0x54)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((u16 *) (obj + 0x10)) = (u16) (*src);
    src++;
    *((u16 *) (obj + 0x12)) = (u16) (*src);
    new_var2 = src[1];
    *((s32 *) (obj + 0xC)) = (s32) (vehicle + 0x50C);
    *((s16 *) (obj + 6)) = 0;
    *((u16 *) (obj + 0x14)) = (u16) new_var2;
    func_800417D0((s32 *) obj);
    ot = (u8 *)D_800A3820;
    D_800A3820 = (s32)(ot + 4);
    *((u8 **) ot) = obj;
    obj += 0x68;
    new_var5 = obj + 0xA;
    a1_val = (*p_anim) * 2;
    obj[0] = 3;
    *((s32 *) (obj + 0xC)) = (s32) (obj - 0x68);
    obj[1] = 0;
    new_var7 = (s16 *) (obj + 6);
    *((s16 *) (obj + 8)) = 0;
    *new_var7 = 1;
    *((s16 *) new_var5) = 0;
    *((s16 *) (obj + 4)) = 6;
    *((s16 *) (obj + 2)) = (s16) (a1_val + 1);
    *((s32 *) (obj + 0x58)) = (s32) (*((s16 *) (vehicle + 0x1A84)));
    ot = (u8 *)D_800A3820;
    D_800A3820 = (s32)(ot + 4);
    *((u8 **) ot) = obj;
    D_800A38B4 = obj + 0x68;
}
s32 func_80049C24(s32 arg0, s32 arg1) {
    s32 count;
    s32 temp_v0;
    s32 temp_a2;
    s32 var_s7;
    s32 v0;
    s32 var_fp;
    s32 var_s5;
    s32 var_s6;
    s32 var_s4;
    s32 var_s0;
    s32 var_s2;
    s32 var_s1;
    s32 var_s3;
    s32 v1;
    s32 hdr;
    s32 a0_arg;

    count = *(s32 *)arg0;
    var_s3 = arg1;
    temp_v0 = *(s32 *)(arg0 + (count * 4) + 4);
    temp_a2 = *(s32 *)(arg0 + 8);
    var_s7 = arg0;
    var_s7 += temp_v0;
    v0 = *(s32 *)(arg0 + 4);
    var_fp = arg0 + v0;
    var_s5 = temp_a2 - v0;

    if (count >= 2) {
        var_s6 = arg0 + temp_a2;
        var_s4 = *(s32 *)(arg0 + 0xC) - temp_a2;
    } else {
        var_s6 = 0;
        var_s4 = 0;
    }

    var_s0 = D_800A33E8;
    var_s2 = D_800A33EA;
    var_s1 = var_s3 + 0xC;

    if (var_s0 == -1) {
        if (var_s2 == var_s0) {
            var_s0 = var_s2;
        } else {
            var_s2 = 0;
        }
    } else if (var_s2 == -1) {
        var_s0 = 0;
    } else if (var_s0 == var_s2) {
        var_s0 = 0;
        var_s2 = 0;
    } else if (var_s0 < var_s2) {
        var_s0 = 0;
        var_s2 = 1;
    } else if (var_s2 < var_s0) {
        var_s0 = 1;
        var_s2 = 0;
    } else {
        func_80052C10();
    }

    hdr = ~var_s0;
    v1 = var_s3;
    var_s3 += 4;
    hdr = (u32)hdr >> 31;
    if (var_s2 >= 0) {
        hdr += 1;
    }
    *(s32 *)v1 = hdr;

    if (var_s0 >= 0) {
        *(s32 *)var_s3 = 2;
        var_s3 += 4;
        if (var_s0 == 0) {
            func_800520B8(var_fp, var_s1, var_s5);
            a0_arg = var_s1 + var_s5;
        } else {
            func_800520B8(var_s6, var_s1, var_s4);
            a0_arg = var_s1 + var_s4;
        }
        func_80045230(a0_arg);
        var_s1 += func_8005C2A8(var_s1, 2, var_s7);
    }

    if (var_s2 >= 0) {
        *(s32 *)var_s3 = 5;
        if (var_s2 == 0) {
            func_800520B8(var_fp, var_s1, var_s5);
            a0_arg = var_s1 + var_s5;
        } else {
            func_800520B8(var_s6, var_s1, var_s4);
            a0_arg = var_s1 + var_s4;
        }
        func_80045230(a0_arg);
        var_s1 += func_8005C2A8(var_s1, 5, var_s7);
    }
    return var_s1;
}
extern s16 D_80099CC2;
extern s32 D_800A324C;
void func_80049E1C(void) {
    s16 val = -1;
    s32 i = 0x39;
    s16 *p = &D_80099CC2;
    do {
        *p = val;
        i--;
        p--;
    } while (i >= 0);
    D_800A324C = -1;
}
extern s32 func_800418D0();

extern void *D_800A3708;
extern void *D_800A370C;
void func_80049E4C(void) {
    Unk80101DF0Record *p1 = &D_80101DF0;
    Unk80101DF0Record *p2 = &D_800FF638;
    p1->unk0 = 0x64;
    D_80101DF0.unk1 = 0;
    D_80101DF0.xf.rot.vx = 0;
    D_80101DF0.xf.rot.vy = 0;
    D_80101DF0.xf.rot.vz = 0;
    D_80101DF0.work.t[0] = 0;
    D_80101DF0.work.t[1] = 0;
    D_80101DF0.work.t[2] = 0;
    D_80101DF0.unkC = 0;
    D_80101DF0.unk8 = 5;
    func_800418D0(p1);
    p2->unk0 = 0x65;
    D_800FF638.unk1 = 0;
    D_800FF638.xf.rot.vx = 0;
    D_800FF638.xf.rot.vy = 0;
    D_800FF638.xf.rot.vz = 0;
    D_800FF638.work.t[0] = 0;
    D_800FF638.work.t[1] = 0;
    D_800FF638.work.t[2] = 0;
    D_800FF638.unkC = 0;
    D_800FF638.unk8 = 2;
    func_800418D0(p2);
    D_800A3708 = p1;
    D_800A370C = p2;
}
extern u8 D_800153F0;
extern u8 D_800F62E0;
extern s32 g_gte_color_matrix_data;
extern u8 g_gte_back_color_r;
extern u8 g_gte_back_color_g;
extern u8 g_gte_back_color_b;
extern void func_8004A09C(s32, u16 *);
extern void SetColorMatrix(s32 *);
extern void SetBackColor(s32, s32, s32);

void func_80049F4C(void) {
    s32 sp10[11];
    s32 i;
    u8 *base;
    __builtin_memcpy(sp10, &D_800153F0, 44);
    i = 0;
    base = &D_800F62E0;
    do {
        func_8004A09C((s32)base, (u16 *)sp10);
        i++;
        base += 0x60;
    } while (i < 8);
    SetColorMatrix(&g_gte_color_matrix_data);
    SetBackColor(g_gte_back_color_r, g_gte_back_color_g, g_gte_back_color_b);
}
void func_8004A09C(s32 arg0, u16 *arg1) {
    *(s16 *)(arg0 + 0x38) = *arg1++;
    *(s16 *)(arg0 + 0x3E) = *arg1++;
    *(s16 *)(arg0 + 0x44) = *arg1++;
    *(s16 *)(arg0 + 0x3A) = *arg1++;
    *(s16 *)(arg0 + 0x40) = *arg1++;
    *(s16 *)(arg0 + 0x46) = *arg1++;
    *(s16 *)(arg0 + 0x3C) = *arg1++;
    *(s16 *)(arg0 + 0x42) = *arg1++;
    {
        s16 v48 = *arg1++;
        *(s16 *)(arg0 + 0x18) = 0;
        *(s16 *)(arg0 + 0x1A) = 0;
        *(s16 *)(arg0 + 0x1C) = 0;
        *(s16 *)(arg0 + 0x1E) = 0;
        *(s16 *)(arg0 + 0x20) = 0;
        *(s16 *)(arg0 + 0x22) = 0;
        *(s16 *)(arg0 + 0x24) = 0;
        *(s16 *)(arg0 + 0x26) = 0;
        *(s16 *)(arg0 + 0x28) = 0;
        *(s16 *)(arg0 + 0x48) = v48;
    }
    *(s16 *)(arg0 + 0x00) = *arg1++;
    *(s16 *)(arg0 + 0x02) = *arg1++;
    *(s16 *)(arg0 + 0x04) = *arg1++;
    *(s16 *)(arg0 + 0x08) = *arg1++;
    *(s16 *)(arg0 + 0x0A) = *arg1++;
    *(s16 *)(arg0 + 0x0C) = *arg1++;
    *(s16 *)(arg0 + 0x10) = *arg1++;
    *(s16 *)(arg0 + 0x12) = *arg1++;
    *(s16 *)(arg0 + 0x14) = *arg1++;
    func_8004A1FC();
    *(s8 *)(arg0 + 0x58) = *arg1++;
    *(s8 *)(arg0 + 0x59) = *arg1++;
    *(s8 *)(arg0 + 0x5A) = *arg1;
    *(s16 *)(arg0 + 0x5C) = *(arg1 + 1);
}
extern s32 rcos();
extern s32 rsin();
void func_8004A1FC(arg0) s16 *arg0; {
    s16 i;
    s16 *p;
    s16 *out;
    s16 c0;
    s32 t;

    i = 0;
    do {
        p = arg0 + (s32)i * 4;
        if (p[2] != 0) {
            c0 = rcos(p[0]);
            t = ((rsin(p[1]) * c0) >> 12) * arg0[0x2E];
            out = arg0 + (s32)i * 3 + 12;
            out[0] = -t >> 12;
            t = rsin(p[0]) * arg0[0x2E];
            out[1] = t >> 12;
            t = ((rcos(p[1]) * c0) >> 12) * arg0[0x2E];
            out[2] = -t >> 12;
        } else {
            out = arg0 + (s32)i * 3 + 12;
            out[0] = 0;
            out[1] = 0;
            out[2] = 0;
        }
        i++;
    } while (i < 3);
}
INCLUDE_ASM("asm/funcs", math_RotMatrixZYX);
INCLUDE_ASM("asm/funcs", func_8004A4E0);
INCLUDE_ASM("asm/funcs", func_8004A76C);
INCLUDE_ASM("asm/funcs", func_8004A808);
void func_8004A938(void) {
}
INCLUDE_ASM("asm/funcs", func_8004A940);
INCLUDE_ASM("asm/funcs", func_8004BB68);
INCLUDE_ASM("asm/funcs", func_8004BCC0);
INCLUDE_ASM("asm/funcs", func_8004C1F4);
INCLUDE_ASM("asm/funcs", func_8004C388);
PAD_NOPS_1; /* padding after func_8004C388 */
INCLUDE_ASM("asm/funcs", func_8004C404);
INCLUDE_ASM("asm/funcs", func_8004C994);
INCLUDE_ASM("asm/funcs", func_8004CB8C);
INCLUDE_ASM("asm/funcs", func_8004CDB0);
INCLUDE_ASM("asm/funcs", func_8004CFE0);
INCLUDE_ASM("asm/funcs", func_8004D244);
INCLUDE_ASM("asm/funcs", func_8004D424);
INCLUDE_ASM("asm/funcs", func_8004D634);
INCLUDE_ASM("asm/funcs", func_8004D838);
INCLUDE_ASM("asm/funcs", func_8004DA74);
INCLUDE_ASM("asm/funcs", func_8004DDB4);
PAD_NOPS_1; /* padding after func_8004DDB4 */
void func_8004E564(void) {
}
void func_8004E56C(void) {
}
INCLUDE_ASM("asm/funcs", func_8004E574);
INCLUDE_ASM("asm/funcs", func_8004E7E4);
INCLUDE_ASM("asm/funcs", func_8004EAC8);
INCLUDE_ASM("asm/funcs", func_8004ECC8);
INCLUDE_ASM("asm/funcs", func_8004EF10);
INCLUDE_ASM("asm/funcs", func_8004F0FC);
INCLUDE_ASM("asm/funcs", func_8004F314);
INCLUDE_ASM("asm/funcs", func_8004F53C);
INCLUDE_ASM("asm/funcs", func_8004F798);
INCLUDE_ASM("asm/funcs", func_8004F970);
INCLUDE_ASM("asm/funcs", func_8004FB74);
INCLUDE_ASM("asm/funcs", func_8004FD40);
INCLUDE_ASM("asm/funcs", func_8004FF40);
INCLUDE_ASM("asm/funcs", func_80050120);
INCLUDE_ASM("asm/funcs", func_80050334);
INCLUDE_ASM("asm/funcs", func_80050538);
INCLUDE_ASM("asm/funcs", func_80050774);
INCLUDE_ASM("asm/funcs", func_80050908);
INCLUDE_ASM("asm/funcs", func_80050AB8);
INCLUDE_ASM("asm/funcs", func_80050C68);
INCLUDE_ASM("asm/funcs", func_80050E60);
INCLUDE_ASM("asm/funcs", func_80051010);
INCLUDE_ASM("asm/funcs", func_80051208);
INCLUDE_ASM("asm/funcs", func_800513B0);
INCLUDE_ASM("asm/funcs", func_800515AC);
INCLUDE_ASM("asm/funcs", func_80051754);
INCLUDE_ASM("asm/funcs", func_80051944);
INCLUDE_ASM("asm/funcs", func_80051B04);
INCLUDE_ASM("asm/funcs", func_80051D08);
INCLUDE_ASM("asm/funcs", func_80051ED4);
INCLUDE_ASM("asm/funcs", func_800520B8);
INCLUDE_ASM("asm/funcs", func_800523E0);
INCLUDE_ASM("asm/funcs", func_800525D8);
/* func_800526A0: hand-coded asm in original PSY-Q source.
 * Evidence (see memory/feedback_hand_coded_asm_recognition.md):
 *   - 5 trapping arithmetic ops (add/addi/sub) GCC 2.7.2 cannot
 *     emit from pure C (opcode 0x20/0x22 vs natural 0x21/0x23)
 *   - Dead delay-slot init: addiu $t0,$zero,0x1F before beqz,
 *     overwritten before use in fall-through path
 *   - multi_jr_ra: 3 separate jr $ra blocks (small/large/zero
 *     cases), no shared epilogue (GCC -O2 always CSEs)
 *   - GTE LZCS/LZCR fast leading-zero-count math primitive
 *   - Classifier auto-verdict: permanently_blocked:handwritten_overflow_op
 *   - Pure-C+§6.1 attempt reached 27/30 insns; remaining 3 are
 *     GCC-impossible structural patterns.
 * User-authorized 2026-05-16. */
INCLUDE_ASM("asm/funcs", math_SquareRoot0);
PAD_NOPS_2; /* padding after func_800526A0 */
/* func_80052720: GTE sqr tail-call wrapper — mtc2 IR1-3 -> sqr -> sum
 * MAC1-3 into $a0 -> frameless `j func_800526A0` tail-call.
 * Hand-written asm: trapping `add` ops (GCC 2.7.2 emits addu), mfc2
 * results land in $t0/$t1/$t2 (natural cc1 allocation picks $v0/$v1/$a0),
 * hand-scheduled GTE pipeline nops, and no sibling-call TCO exists in
 * GCC 2.7.2 for the frameless j. Tail-call variant of the authorized
 * sibling func_80052754 below. Canonical-asm; see inline_asm_canonical.txt.
 * User-authorized 2026-06-12. */
INCLUDE_ASM("asm/funcs", math_Length3D);
/* GTE sqr (squared-vector-length) leaf wrapper: mtc2 IR1-3 -> sqr -> sum MAC1-3.
 * Hand-written asm — mfc2 results land in $t0/$t1/$t2, which natural cc1
 * register allocation cannot pick (GCC chooses $v0/$v1/$a0). Canonical-asm;
 * see inline_asm_canonical.txt. */
INCLUDE_ASM("asm/funcs", gte_SumSquares3);
INCLUDE_ASM("asm/funcs", math_LerpSVector);
INCLUDE_ASM("asm/funcs", math_LerpMatrix3x3);
/* func_80052930: LIBGTE 3x3-mvmva matrix x s16-packed-vector transform leaf.
 * 5x lw <- *a0 -> ctc2 $0-$4 (packed R matrix) + ctc2 $zero to $5-$7 (zero
 * translation), 5x lw <- *a1 packed to s16 pairs via a hand-held
 * `lui $t9,0xFFFF` mask, three mvmva 1,0,0,0,0 cycles whose packing for cycle
 * N+1 is computed inside cycle N's GTE latency window, mfc2 $9/$10/$11 drained
 * between, 9x sh to *a2 with the last IN the jr-ra delay slot (0x80052A1C).
 * Zero general-purpose computation on the mfc2 outputs. GCC 2.7.2 cannot fill
 * a delay slot with asm (reorg.c stop_search_p halts at ASM_INPUT) and the
 * per-cycle mask re-materialization + latency interleave are hand-scheduling,
 * so the bytes are unreachable from any C. Last member of the text1b.c LIBGTE
 * leaf run (siblings func_80052A20/A88/B00/B44/B7C, authorized 2026-08-06).
 * Canonical-asm; see inline_asm_canonical.txt. Owner-authorized 2026-08-11. */
INCLUDE_ASM("asm/funcs", gte_MulMatrix0ClearTrans);
INCLUDE_ASM("asm/funcs", gte_SetMatrixRotTransIR);
INCLUDE_ASM("asm/funcs", gte_SetMatrixRotTransIRVec);
INCLUDE_ASM("asm/funcs", gte_SetRotTransMatrix);
/* func_80052B44 = LIBGTE-style SetRotMatrix + zero-translation. Loads a packed
 * 3x3 rotation matrix (5 s32 words) from *a0 into cop2 controls CR0-CR4, then
 * zeroes the translation vector CR5-CR7 (TRX/TRY/TRZ), the last ctc2 in the
 * jr-ra delay slot. All cop2 + mechanical load packaging; hand-written GTE asm
 * (prologue instruction-identical to canonical-body func_8007ED6C, display.c).
 * Canonical-body authorized 2026-07-27 (judge PASS, docs/grind/decisions.md). */
INCLUDE_ASM("asm/funcs", gte_SetRotMatrixClearTrans);
INCLUDE_ASM("asm/funcs", func_80052B7C);
/* func_80052BE4: GTE far-color read wrapper — cfc2 RFC/GFC/BFC (cop2 ctrl
 * 21/22/23) -> srl 4 -> sb to *a0[0..2]. Hand-written asm: cfc2 results land
 * in $t0/$t1/$t2 (natural cc1 allocation picks $v0/$v1/$a1), and the jr $ra
 * delay slot holds a canonical nop where GCC's reorg would fill the last sb.
 * Canonical-asm; see inline_asm_canonical.txt. User-authorized 2026-06-12. */
INCLUDE_ASM("asm/funcs", gte_ReadFarColor);
INCLUDE_ASM("asm/funcs", func_80052C10);
PAD_NOPS_1; /* padding after InitFadePanel */
INCLUDE_ASM("asm/funcs", func_80052C28);
INCLUDE_ASM("asm/funcs", func_80052C4C);
INCLUDE_ASM("asm/funcs", gte_ReadIR1IR2Sra2);
PAD_NOPS_3; /* padding after func_80052CD4 */
extern s32 D_800A33F4;
extern s32 func_80053694(s32 *, s16 *);

typedef struct {
    s16 x;
    s16 z;
} Cell_80052D00;

typedef struct {
    s32 unk0;
    s16 unk4;
    s16 unk6;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
    s32 unk20;
    s32 unk24;
    s32 unk28;
    s32 unk2C;
    s32 unk30;
    s32 unk34;
    s32 unk38;
    s32 unk3C;
    s32 unk40;
    s32 unk44;
    s16 unk48;
    s16 unk4A;
    s16 unk4C;
    s16 unk4E;
    s16 unk50;
    s16 unk52;
    s16 unk54;
    s16 unk56;
    s16 unk58;
    s16 unk5A;
    s32 (*unk5C)(s32, s32);
    s32 unk60;
    s32 unk64;
    s32 unk68;
    s32 unk6C;
    s32 unk70;
    s32 unk74;
    s32 unk78;
    s32 unk7C;
    s32 unk80;
    s32 unk84;
    Cell_80052D00 unk88;
    Cell_80052D00 unk8C;
    s16 unk90;
    u8 unk92[0xA];
    s32 unk9C;
    s32 unkA0;
    s32 unkA4;
    s32 unkA8;
    s32 unkAC;
    s32 unkB0;
    s32 unkB4;
    s32 unkB8;
    s32 unkBC;
    s32 unkC0;
    s32 unkC4;
    s32 unkC8;
    s32 unkCC;
    s32 unkD0;
    s32 unkD4;
    s32 unkD8;
    s32 unkDC;
    s32 unkE0;
    s32 unkE4;
    s32 unkE8;
} Work_80053E9C;

#define W ((Work_80053E9C *)D_800A33F4)

/* Walks the 32x32 grid of 2000-unit cells (origin -32000) along the XZ
 * segment from the start point (+0x8/+0x10) to the end point (+0x18/+0x20),
 * DDA style: +0x88/+0x8C are the current and end cells, +0x70/+0x74 the
 * major/minor extents (swapped when Z dominates), +0x7C the 4.12 slope and
 * +0x90 the major-axis steps left. Each cell crossed goes to the per-cell
 * test at +0x5C (func_80053E9C or func_80053754) until one reports a hit;
 * func_80053694 then reads the result back out. */
s32 func_80052D00(s32 arg0, s32 arg1) {
    s32 xdir;
    s32 zdir;
    s32 swapped;

    W->unk60 = W->unk8 + 32000;
    W->unk64 = W->unk10 + 32000;
    W->unk68 = W->unk18 + 32000;
    W->unk0 = 0x7FFFFFFF;
    W->unk6C = W->unk20 + 32000;
    W->unk70 = W->unk68 - W->unk60;
    W->unk88.x = W->unk60 / 2000;
    W->unk88.z = W->unk64 / 2000;
    W->unk8C.x = W->unk68 / 2000;
    W->unk8C.z = W->unk6C / 2000;
    W->unk74 = W->unk6C - W->unk64;
    if (W->unk88.x == W->unk8C.x && W->unk88.z == W->unk8C.z) {
        if (W->unk8 == W->unk18 && W->unkC == W->unk1C && W->unk10 == W->unk20) {
            return 0;
        }
        W->unk5C(W->unk88.x, W->unk88.z);
    } else {
        W->unk80 = W->unk88.x * 2000 + 1000;
        W->unk84 = W->unk88.z * 2000 + 1000;
        W->unk60 -= W->unk80;
        W->unk64 -= W->unk84;
        W->unk68 -= W->unk80;
        W->unk6C -= W->unk84;
        if (W->unk70 < 0) {
            xdir = -1;
            W->unk70 = -W->unk70;
            W->unk60 = -W->unk60;
            W->unk68 = -W->unk68;
        } else {
            xdir = 1;
        }
        if (W->unk74 < 0) {
            zdir = -1;
            W->unk74 = -W->unk74;
            W->unk64 = -W->unk64;
            W->unk6C = -W->unk6C;
        } else {
            zdir = 1;
        }
        if (W->unk74 > W->unk70) {
            swapped = 1;
            W->unk80 = W->unk70;
            W->unk70 = W->unk74;
            W->unk74 = W->unk80;
            W->unk80 = W->unk60;
            W->unk60 = W->unk64;
            W->unk64 = W->unk80;
            W->unk80 = W->unk68;
            W->unk68 = W->unk6C;
            W->unk6C = W->unk80;
        } else {
            swapped = 0;
        }
        W->unk68 += 1000;
        W->unk60 += 1000;
        W->unk64 += 1000;
        W->unk90 = W->unk68 / 2000 - W->unk60 / 2000 + 1;
        W->unk7C = (W->unk74 << 12) / W->unk70;
        W->unk64 -= (W->unk60 * W->unk7C) >> 12;
        W->unk78 = (W->unk7C * 2000) >> 12;
        while (--W->unk90 != -1) {
            if ((W->unk80 = W->unk5C(W->unk88.x, W->unk88.z)) != 0) {
                break;
            }
            W->unk64 %= 2000;
            W->unk64 += W->unk78;
            if (W->unk90 == 0) {
                break;
            }
            if (W->unk64 > 2000) {
                if (swapped) {
                    if (xdir < 0) {
                        W->unk88.x--;
                    } else {
                        W->unk88.x++;
                    }
                } else {
                    if (zdir < 0) {
                        W->unk88.z--;
                    } else {
                        W->unk88.z++;
                    }
                }
                if ((W->unk80 = W->unk5C(W->unk88.x, W->unk88.z)) != 0) {
                    break;
                }
            }
            if (swapped) {
                if (zdir < 0) {
                    W->unk88.z--;
                } else {
                    W->unk88.z++;
                }
            } else {
                if (xdir < 0) {
                    W->unk88.x--;
                } else {
                    W->unk88.x++;
                }
            }
        }
        if (W->unk80 == 0 && (W->unk88.x != W->unk8C.x || W->unk88.z != W->unk8C.z)) {
            W->unk5C(W->unk8C.x, W->unk8C.z);
        }
    }
    return func_80053694((s32 *)arg0, (s16 *)arg1);
}
extern s32 gte_SumSquares3(s32, s32, s32);
extern s32 func_80052D00(s32, s32);
extern s32 func_80053754();
extern s32 func_80053E9C();
extern u8 D_800EFA00;
extern u8 D_800EF9F8;
extern s32 D_800A33F4;
typedef struct { s32 a, b, c, d; } _S16_53304;
void func_80053304(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3) {
    u8 *p;
    s32 a, b, c;
    s32 hi0, hi1, hi2;
    D_800A33F4 = (s32)&D_800EF9F8;
    *(_S16_53304 *)&D_800EFA00 = *(_S16_53304 *)arg0;
    *(_S16_53304 *)((u8 *)D_800A33F4 + 0x18) = *(_S16_53304 *)arg1;
    if (gte_SumSquares3(
            *(s32 *)((u8 *)D_800A33F4 + 0x18) - *(s32 *)((u8 *)D_800A33F4 + 0x8),
            *(s32 *)((u8 *)D_800A33F4 + 0x1C) - *(s32 *)((u8 *)D_800A33F4 + 0xC),
            *(s32 *)((u8 *)D_800A33F4 + 0x20) - *(s32 *)((u8 *)D_800A33F4 + 0x10)) <= 0x9C3F) {
        p = (u8 *)D_800A33F4;
        hi0 = *(s32 *)(p + 0x18);
        a = *(s32 *)(p + 0x8);
        hi1 = *(s32 *)(p + 0x1C);
        b = *(s32 *)(p + 0xC);
        *(s32 *)(p + 0x5C) = (s32)func_80053754;
        hi2 = *(s32 *)(p + 0x20);
        *(s32 *)(p + 0x8)  = a - ((hi0 - a) << 1);
        *(s32 *)(p + 0xC)  = b - ((hi1 - b) << 1);
        c = *(s32 *)(p + 0x10);
        *(s32 *)(p + 0x10) = c - ((hi2 - c) << 1);
    } else {
        *(s32 *)((u8 *)D_800A33F4 + 0x5C) = (s32)func_80053E9C;
    }
    func_80052D00(arg2, arg3);
}
extern s32 gte_SumSquares3(s32, s32, s32);
extern s32 func_80052D00(s32, s32);
extern s32 func_80053754();
extern s32 func_80053E9C();

typedef struct { s32 a, b, c, d; } _S16_5344C;
void func_8005344C(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 arg4) {
    u8 *p;
    s32 a, b, c;
    s32 hi0, hi1, hi2;
    D_800A33F4 = (u8 *)arg4;
    *(_S16_5344C *)((u8 *)D_800A33F4 + 8) = *(_S16_5344C *)arg0;
    *(_S16_5344C *)((u8 *)D_800A33F4 + 0x18) = *(_S16_5344C *)arg1;
    if (gte_SumSquares3(
            *(s32 *)((u8 *)D_800A33F4 + 0x18) - *(s32 *)((u8 *)D_800A33F4 + 0x8),
            *(s32 *)((u8 *)D_800A33F4 + 0x1C) - *(s32 *)((u8 *)D_800A33F4 + 0xC),
            *(s32 *)((u8 *)D_800A33F4 + 0x20) - *(s32 *)((u8 *)D_800A33F4 + 0x10)) <= 0x9C3F) {
        p = (u8 *)D_800A33F4;
        hi0 = *(s32 *)(p + 0x18);
        a = *(s32 *)(p + 0x8);
        hi1 = *(s32 *)(p + 0x1C);
        b = *(s32 *)(p + 0xC);
        *(s32 *)(p + 0x5C) = (s32)func_80053754;
        hi2 = *(s32 *)(p + 0x20);
        *(s32 *)(p + 0x8)  = a - ((hi0 - a) << 1);
        *(s32 *)(p + 0xC)  = b - ((hi1 - b) << 1);
        c = *(s32 *)(p + 0x10);
        *(s32 *)(p + 0x10) = c - ((hi2 - c) << 1);
    } else {
        *(s32 *)((u8 *)D_800A33F4 + 0x5C) = (s32)func_80053E9C;
    }
    func_80052D00(arg2, arg3);
}
extern s32 func_80052D00(s32, s32);
extern s32 func_80053E9C();
extern u8 D_800EFA00;
extern u8 D_800EF9F8;

typedef struct { s32 a, b, c, d; } _S16_53584;
void func_80053584(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3) {
    D_800A33F4 = (s32)&D_800EF9F8;
    *(_S16_53584 *)&D_800EFA00 = *(_S16_53584 *)arg0;
    *(_S16_53584 *)((u8 *)D_800A33F4 + 0x18) = *(_S16_53584 *)arg1;
    *(s32 *)((u8 *)D_800A33F4 + 0x5C) = (s32)func_80053E9C;
    func_80052D00(arg2, arg3);
}
typedef struct { s32 a, b, c, d; } _S16_53614;
s32 func_80053614(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 arg4) {
    D_800A33F4 = arg4;
    *(_S16_53614 *)((u8 *)D_800A33F4 + 8) = *(_S16_53614 *)arg0;
    *(_S16_53614 *)((u8 *)D_800A33F4 + 0x18) = *(_S16_53614 *)arg1;
    *(s32 *)((u8 *)D_800A33F4 + 0x5C) = (s32)func_80053E9C;
    return func_80052D00(arg2, arg3);
}

extern u16 D_800A33F8;
s32 func_80053694(s32 *arg0, s16 *arg1) {
    u8 *p = D_800A33F4;
    s32 t;
    if (*(s32 *)(p + 0) != 0x7FFFFFFF) {
        t = (*(s16 *)(p + 0x48) * 0x7D0) - 0x7D00;
        arg0[0] = *(s32 *)(p + 0x38) + t;
        arg0[1] = *(s32 *)(p + 0x3C);
        t = (*(s16 *)(p + 0x4A) * 0x7D0) - 0x7D00;
        arg0[2] = *(s32 *)(p + 0x40) + t;
        arg1[0] = *(s32 *)(p + 0x28) >> 2;
        arg1[1] = *(s32 *)(p + 0x2C) >> 2;
        arg1[2] = *(s32 *)(p + 0x30) >> 2;
        D_800A33F8 = *(u16 *)(p + 4);
        return 1;
    }
    return 0;
}
extern s32 D_800A33F0;
extern s32 D_800A33F4;
extern s32 gte_SumSquares3(s32, s32, s32);
extern void func_80052C4C(s32, s32, s32, s32);
extern void gte_ReadIR1IR2Sra2(s32 *, s32 *);

s32 func_80053754(s32 arg0, s32 arg1) {
    s32 n;
    s32 data;
    s32 count;
    s32 x;
    s32 z;
    s32 y;
    s16 hdr;

    if (arg0 < 0 || arg1 < 0 || arg0 >= 32 || arg1 >= 32) {
        return 0;
    }
    W->unkE0 = ((u16 *)D_800A33F0)[arg1 * 32 + arg0];
    if (W->unkE0 == 0xFFFF) {
        return 0;
    }
    data = D_800A33F0 + W->unkE0;
    x = arg0 * 2000 - 32000;
    z = arg1 * 2000 - 32000;
    W->unk4C = W->unk8 - x;
    W->unk4E = W->unkC;
    W->unk50 = W->unk10 - z;
    W->unk54 = W->unk18 - x;
    W->unk56 = W->unk1C;
    W->unk58 = W->unk20 - z;

    count = *(s16 *)data;
    data += 2;
    while (--count != -1) {
        W->unkD0 = *(s16 *)data;
        data += 2;
        W->unkD4 = *(s16 *)data;
        data += 2;
        W->unkD8 = *(s16 *)data;
        data += 2;
        W->unkDC = *(u16 *)data;
        data += 2;
        W->unkDC = (*(s16 *)data << 16) | W->unkDC;
        data += 2;
        W->unkE4 = W->unkD0 * W->unk4C + W->unkD4 * W->unk4E + W->unkD8 * W->unk50 + W->unkDC;
        W->unkE8 = W->unkD0 * W->unk54 + W->unkD4 * W->unk56 + W->unkD8 * W->unk58 + W->unkDC;

        W->unkE4 = (W->unkE4 < 0 ? -1 : 1) * ((W->unkE4 < 0 ? -W->unkE4 : W->unkE4) >> 10);
        W->unkE8 = (W->unkE8 < 0 ? -1 : 1) * ((W->unkE8 < 0 ? -W->unkE8 : W->unkE8) >> 10);

        if (W->unkE4 >= 0 && W->unkE8 < 0) {
            W->unkE0 = W->unkE4 - W->unkE8;
            W->unkE4 *= 2;
            W->unkA8 = (W->unk54 - W->unk4C) * W->unkE4 / W->unkE0;
            W->unkAC = (W->unk56 - W->unk4E) * W->unkE4 / W->unkE0;
            W->unkB0 = (W->unk58 - W->unk50) * W->unkE4 / W->unkE0;
            W->unkA8 = (W->unkA8 < 0 ? -1 : 1) * ((W->unkA8 >= 0 ? W->unkA8 + 1 : -W->unkA8 + 1) >> 1);
            W->unkAC = (W->unkAC < 0 ? -1 : 1) * ((W->unkAC >= 0 ? W->unkAC + 1 : -W->unkAC + 1) >> 1);
            W->unkB0 = (W->unkB0 < 0 ? -1 : 1) * ((W->unkB0 >= 0 ? W->unkB0 + 1 : -W->unkB0 + 1) >> 1);
            W->unkA8 += W->unk4C;
            W->unkAC += W->unk4E;
            W->unkB0 += W->unk50;
            W->unkE0 = *(s16 *)data;
            data += 2;
            W->unkE4 = *(s16 *)data;
            data += 2;
            W->unkE8 = *(s16 *)data;
            data += 2;
            W->unk9C = *(s16 *)data;
            data += 2;
            W->unkA0 = *(s16 *)data;
            data += 2;
            W->unkA4 = *(s16 *)data;
            data += 2;
            W->unkC4 = ((W->unkA8 - W->unkE0) * W->unk9C + (W->unkAC - W->unkE4) * W->unkA0 + (W->unkB0 - W->unkE8) * W->unkA4) >> 14;
            W->unk9C = *(s16 *)data;
            data += 2;
            W->unkA0 = *(s16 *)data;
            data += 2;
            W->unkA4 = *(s16 *)data;
            data += 2;
            W->unkC8 = ((W->unkA8 - W->unkE0) * W->unk9C + (W->unkAC - W->unkE4) * W->unkA0 + (W->unkB0 - W->unkE8) * W->unkA4) >> 14;
            hdr = *(u16 *)data;
            data += 2;
            n = hdr;
            W->unkCC = n >> 8;
            n &= 0xFF;
            W->unkB4 = *(s16 *)data;
            data += 2;
            y = *(s16 *)data;
            data += 2;
            W->unkE0 = 1;
            W->unkB8 = y;
            while (--n != -1) {
                W->unkBC = *(s16 *)data;
                data += 2;
                W->unkC0 = *(s16 *)data;
                data += 2;
                if ((W->unkC4 - W->unkB4) * (W->unkC0 - W->unkB8)
                    - (W->unkC8 - W->unkB8) * (W->unkBC - W->unkB4) > 0) {
                    W->unkE0 = 0;
                    break;
                }
                W->unkB4 = W->unkBC;
                W->unkB8 = W->unkC0;
            }
            if (n > 0) {
                data += n * 4;
            }
            if (W->unkE0 != 0) {
                if ((W->unkE0 = gte_SumSquares3(W->unkA8 - W->unk4C, W->unkAC - W->unk4E, W->unkB0 - W->unk50)) < W->unk0) {
                    W->unk48 = arg0;
                    W->unk4A = arg1;
                    W->unk38 = W->unkA8;
                    W->unk3C = W->unkAC;
                    W->unk40 = W->unkB0;
                    W->unk28 = W->unkD0;
                    W->unk2C = W->unkD4;
                    W->unk30 = W->unkD8;
                    W->unk34 = W->unkDC;
                    W->unk0 = W->unkE0;
                    W->unk4 = W->unkCC;
                }
            }
        } else {
            data += 18;
            n = *(s16 *)data;
            data += 2;
            n &= 0xFF;
            data += (n + 1) * 4;
        }
    }
    return W->unk0 != 0x7FFFFFFF;
}

s32 func_80053E9C(s32 arg0, s32 arg1) {
    s32 n;
    s32 data;
    s32 count;
    s32 x;
    s32 z;
    s32 y;
    s16 hdr;

    if (arg0 < 0 || arg1 < 0 || arg0 >= 32 || arg1 >= 32) {
        return 0;
    }
    W->unkE0 = ((u16 *)D_800A33F0)[arg1 * 32 + arg0];
    if (W->unkE0 == 0xFFFF) {
        return 0;
    }
    data = D_800A33F0 + W->unkE0;
    x = arg0 * 2000 - 32000;
    z = arg1 * 2000 - 32000;
    W->unk4C = W->unk8 - x;
    W->unk4E = W->unkC;
    W->unk50 = W->unk10 - z;
    W->unk54 = W->unk18 - x;
    W->unk56 = W->unk1C;
    W->unk58 = W->unk20 - z;

    count = *(s16 *)data;
    data += 2;
    while (--count != -1) {
        W->unkD0 = *(s16 *)data;
        data += 2;
        W->unkD4 = *(s16 *)data;
        data += 2;
        W->unkD8 = *(s16 *)data;
        data += 2;
        W->unkDC = *(u16 *)data;
        data += 2;
        W->unkDC = (*(s16 *)data << 16) | W->unkDC;
        data += 2;
        W->unkE4 = W->unkD0 * W->unk4C + W->unkD4 * W->unk4E + W->unkD8 * W->unk50 + W->unkDC;
        W->unkE8 = W->unkD0 * W->unk54 + W->unkD4 * W->unk56 + W->unkD8 * W->unk58 + W->unkDC;

        W->unkE4 = (W->unkE4 < 0 ? -1 : 1) * ((W->unkE4 < 0 ? -W->unkE4 : W->unkE4) >> 14);
        W->unkE8 = (W->unkE8 < 0 ? -1 : 1) * ((W->unkE8 < 0 ? -W->unkE8 : W->unkE8) >> 14);

        if (W->unkE4 >= 0 && W->unkE8 < 0) {
            W->unkE0 = W->unkE4 - W->unkE8;
            W->unkA8 = (W->unk54 - W->unk4C) * W->unkE4 / W->unkE0 + W->unk4C;
            W->unkAC = (W->unk56 - W->unk4E) * W->unkE4 / W->unkE0 + W->unk4E;
            W->unkB0 = (W->unk58 - W->unk50) * W->unkE4 / W->unkE0 + W->unk50;
            func_80052C4C(data, W->unkA8, W->unkAC, W->unkB0);
            data += 18;
            hdr = *(u16 *)data;
            data += 2;
            n = hdr;
            W->unkCC = n >> 8;
            n &= 0xFF;
            W->unkB4 = *(s16 *)data;
            data += 2;
            y = *(s16 *)data;
            data += 2;
            W->unkE0 = 1;
            W->unkB8 = y;
            gte_ReadIR1IR2Sra2(&W->unkC4, &W->unkC8);
            while (--n != -1) {
                W->unkBC = *(s16 *)data;
                data += 2;
                W->unkC0 = *(s16 *)data;
                data += 2;
                if ((W->unkC4 - W->unkB4) * (W->unkC0 - W->unkB8)
                    - (W->unkC8 - W->unkB8) * (W->unkBC - W->unkB4) > 0) {
                    W->unkE0 = 0;
                    break;
                }
                W->unkB4 = W->unkBC;
                W->unkB8 = W->unkC0;
            }
            if (n > 0) {
                data += n * 4;
            }
            if (W->unkE0 != 0) {
                if ((W->unkE0 = gte_SumSquares3(W->unkA8 - W->unk4C, W->unkAC - W->unk4E, W->unkB0 - W->unk50)) < W->unk0) {
                    W->unk48 = arg0;
                    W->unk4A = arg1;
                    W->unk38 = W->unkA8;
                    W->unk3C = W->unkAC;
                    W->unk40 = W->unkB0;
                    W->unk28 = W->unkD0;
                    W->unk2C = W->unkD4;
                    W->unk30 = W->unkD8;
                    W->unk34 = W->unkDC;
                    W->unk0 = W->unkE0;
                    W->unk4 = W->unkCC;
                }
            }
        } else {
            data += 18;
            n = *(s16 *)data;
            data += 2;
            n &= 0xFF;
            data += (n + 1) * 4;
        }
    }
    return W->unk0 != 0x7FFFFFFF;
}

#undef W
extern s32 D_800A33F0;
void func_80054410(s32 a0) {
    D_800A33F0 = a0;
}
void func_8005441C(s32 a0) {
    D_800A33F0 += a0;
}

s16 func_80054434(void) {
    return D_800A33F8;
}
INCLUDE_ASM("asm/funcs", func_80054440);
INCLUDE_ASM("asm/funcs", func_800545F4);
extern s32 D_800A3770;
extern const char D_80015840[];
extern s32 func_80044FA0(s32, s32);
extern s32 func_80045080(s32);
extern void func_80046914(void);
extern s32 *func_800469C4(s32);
extern s16 *stage_GetDataPtr(void);
extern s32 stage_GetId(void);

extern void func_8003FFC4(s32);
extern void func_8003F218(s32);
extern s32 math_FovToScreenDist(s32);
extern void SetGeomScreen(s32);
extern void gpu_ResetGraphMode1(void);
extern void game_StageCleanup(s32, s32);
extern void func_8004659C(s32);
s32 func_80054604(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6) {
    /* FAKE: second C handle to the global ctrl block (pointer-alias family);
       mechanism: expand/cse address materialisation -- the pointer local seats
       %hi/%lo(D_800EFAE8) in one callee-saved base register ($s1) for the whole
       body, whereas the direct D_800EFAE8.field form re-materialises the address
       per extended basic block; lever-exhaustion: direct-global form measured 82
       vs 26 (memory/grind/func_80054604/evidence.md s1,
       rejected/direct-global-no-pointer-local-82.c). */
    Unk800EFAE8Ctrl *s = &D_800EFAE8;
    s32 id = a0 + 0x131;
    s32 ret;
    s16 *t;
    s32 p;
    s32 v;
    s32 n;

    if (a6 != 0) {
        ret = func_80044FA0(id, a6);
        D_800EFAE8.unk2C = a6;
    } else {
        if (func_80045080(id) < 0) {
            func_80046914();
            printf(D_80015840);
        }
        D_800EFAE8.unk2C = (s32)func_800469C4(id);
        ret = 0;
    }
    p = s->unk2C;
    s->unk4 = *(s32 *)(*(s32 *)(p + 4) + p);
    p = s->unk2C;
    s->unk2 = *(u16 *)(*(s32 *)(p + 8) + p);
    s->unk0 = 0;
    t = stage_GetDataPtr();
    t += stage_GetId() * 24 + a1 * 6;
    s->unkC = *t++;
    s->unk10 = *t++;
    s->unk14 = *t++;
    s->unk1C = 0;
    s->unk20 = 0;
    s->unk44[0] = a2;
    s->unk44[1] = a3;
    s->unk48[0] = a4;
    s->unk48[1] = a5;
    s->unk1E = (((s->unk4 >> 8) & 0x7F) << 14) / 360;
    if (s->unk4 >= 0) {
        s->unk44[0] = -1;
    }
    if (!(s->unk4 & 0x40000000)) {
        s->unk44[1] = -1;
    }
    v = func_8004153C(0);
    if (v != 0) {
        func_8003FFC4(v);
    }
    v = func_8004153C(1);
    if (v != 0) {
        func_8003FFC4(v);
    }
    s->unk8 = a1;
    func_8003F218(0);
    SetGeomScreen(math_FovToScreenDist(0x2D));
    if (s->unk4 & 0x3F) {
        n = (s->unk4 & 0x3F) - 1;
        if (a6 != 0) {
            a6 += ret;
            game_StageCleanup(n, a6);
        } else {
            gpu_ResetGraphMode1();
            game_StageCleanup(n, (s32)&D_800A3770);
        }
    }
    if (s->unk4 & 0x8000) {
        func_8004659C(-1);
    }
    return ret;
}
extern s16 InfoPosYTbl1[];
void func_80054884(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7) {
    func_80054604(InfoPosYTbl1[a0] + a1 - 0x131, a2, a3, a4, a5, a6, a7);
}
void DrawSync(s32);
void func_8004659C(s32);
void func_80046A60(void);
void func_800548DC(void) {
    DrawSync(0);
    func_8004659C(-1);
    func_80046A60();
}
INCLUDE_ASM("asm/funcs", func_8005490C);
extern u32 D_80102C00;
extern u16 D_800A38D6;
extern s32 g_gpu_ot_ptr;
extern s32 D_800A3808;
extern s32 D_800A378C;
extern s32 func_8005490C(void);
extern void func_800444E0(void);
s32 func_80054F68(void) {
    s32 v3;
    s32 s0;
    D_800A3820 = &D_80102C00;
    v3 = g_gpu_ot_ptr;
    D_800A38D6 = D_800A38D6 + 1;
    D_800A3808 = v3;
    D_800A378C = v3 + 0x10;
    s0 = func_8005490C();
    func_800444E0();
    return s0;
}
void func_80054FDC(s32 a0) {
    s32 *p = &D_800EFAE8.unk2C;
    *p = a0 + *p;
    D_800EFAE8.unk30 = a0 + D_800EFAE8.unk30;
    if (D_800EFAE8.unk34[0]) {
        D_800EFAE8.unk34[0] = a0 + D_800EFAE8.unk34[0];
    }
    if (D_800EFAE8.unk34[1]) {
        D_800EFAE8.unk34[1] = a0 + D_800EFAE8.unk34[1];
    }
    if (D_800EFAE8.unk3C[0]) {
        D_800EFAE8.unk3C[0] = a0 + D_800EFAE8.unk3C[0];
    }
    if (D_800EFAE8.unk3C[1]) {
        D_800EFAE8.unk3C[1] = a0 + D_800EFAE8.unk3C[1];
    }
}
s32* func_8005507C(void) {
    return (s32 *)D_800EFAE8.unk24;
}
s32* func_8005508C(void) {
    return D_80101DF0.xf.mat.t;
}
void func_8005509C(s32 arg0)
{
  s32 i;
  u8 *p = ((u8 *) (&D_80101EC8)) + (arg0 * 0x44C);
  i = 0;
  do
  {
    p[i * 2 + 0x415] = 0;
    p[i * 2 + 0x414] = 0;
  }
  while ((++i) < 8);
}
void func_800550E8(s32 arg0) {
    s32 i;
    u8 *p = (u8 *)&D_80101EC8 + arg0 * 0x44C;
    i = 0;
    do {
        p[i * 2 + 0x415] = p[i * 2 + 0x415] >> 1;
    } while (++i < 8);
}
extern u32 file_GetFlag1(void);
extern s32 rand(void);
void func_80055138(s32 arg0, u16 *arg1, u16 *arg2) {
    u8 *p = (u8 *)&D_80101EC8 + arg0 * 0x44C;
    u8 *src;
    u8 *pair;
    u8 (*row)[4];
    u8 base;
    /* idx counts two loops: the eight bytes cleared at 0x444, then the two
     * players (0 = this record, 1 = the opponent's). Admitted under Ruling 11
     * (.claude/rules/ordinary-c-judge-decidable.md); allocator-dump proof in
     * memory/grind/func_80055138/ruling11.md. */
    s32 idx;
    s32 sec;
    u8 *rec;
    u16 *cursor;
    u8 *list;
    s32 chr;
    s32 bit;
    s32 lo, hi1, hi2;
    /* temp holds six values in turn; each is read before temp is written again:
     * case 2's level D_800A37D2 / 5; case 2's practice level D_800A37D2 / 3
     * (0 once it reaches 3); case 3's row in D_8009A9B4; a move entry's
     * byte-assembled character mask; the entry's stat bytes e[1] and e[2].
     * Admitted under Ruling 11 (.claude/rules/ordinary-c-judge-decidable.md);
     * allocator-dump proof in memory/grind/func_80055138/ruling11.md. */
    s32 temp;
    u32 cat;
    s32 lo_val, hi1_val, hi2_val;
    u8 *other;

    p[0x443] = *(u16 *)(p + 0xA);
    *(s16 *)(p + 0x438) = *(u16 *)(p + 8);
    switch (D_800A38DC) {
    case 1:
        *(s16 *)(p + 0x438) = (D_800A3783 - 1) / 5 * 0x300 + 0x400;
        if (*(s16 *)(p + 0x438) > 0xD00) {
            *(s16 *)(p + 0x438) = 0xD00;
        }
        break;
    case 0:
        if (D_800A3680 == D_800A3671) {
            func_8005509C(*(s16 *)(p + 4));
        }
        if (D_80099D88[p[0x443]].flags & 0x300) {
            row = D_8009A8C4[*(s16 *)(p + 0x86)];
            src = row[D_800A37A0];
            p[0x424] = src[0];
            p[0x3F6] = src[1];
        }
        break;
    case 2:
        if (D_800A389A) {
            temp = D_800A37D2 / 5;
            *(s16 *)(p + 0x438) = temp * 0x180 + 0x280;
            if (*(s16 *)(p + 0x438) > 0x1000) {
                *(s16 *)(p + 0x438) = 0x1000;
            }
            if ((u8)(D_800A37D2 % 5) == 0) {
                func_8005509C(*(s16 *)(p + 4));
            }
        } else {
            temp = D_800A37D2 / 3;
            if (temp >= 3) {
                D_800A37D2 = 0;
                temp = 0;
            }
            p[0x443] = 0x19;
            *(s16 *)(p + 0x1C) = (temp + 2) << 10;
            *(s16 *)(p + 0x438) = 0;
            p[0x424] = 0;
            p[0x3F6] = 0x3C - temp * 15;
        }
        break;
    case 3:
        p[0x443] = cpu_practice_honmokuroku_data_tbl[D_800A38E2 - 1][0] + 0x1B;
        base = D_800A38E2 / 10;
        *(s16 *)(p + 0x438) = base * 16 + 0x80;
        if (D_80099D88[p[0x443]].flags & 0x3000) {
            *(s16 *)(p + 0x438) = base * 16 + 0x180;
        }
        if (D_80099D88[p[0x443]].flags & 0x4000) {
            *(s16 *)(p + 0x438) += 0x200;
        }
        if ((D_800A38E2 - 1) % 10 == 0) {
            func_8005509C(*(s16 *)(p + 4));
        }
        temp = (u8)(D_800A38E2 / 10) * 2;
        if ((u8)(D_800A38E2 % 10) == 0) {
            temp--;
        }
        pair = D_8009A9B4[temp];
        p[0x424] = pair[0];
        p[0x3F6] = pair[1];
        break;
    }
    if (file_GetFlag1() && D_800A38DC != 3) {
        *(s16 *)(p + 0x438) = *(s16 *)(p + 0x438) * 11 >> 4;
    }
    if (!(D_80099D88[p[0x443]].flags & 0xFF00)) {
        p[0x424] = 0x11 - (*(s16 *)(p + 0x438) >> 8);
    }
    *(s16 *)(p + 0x39A) = 0x8000 / *(s16 *)(p + 0x1C);
    p[0x3BD] = 0x10 - (*(s16 *)(p + 0x438) >> 8);
    if (D_80099D88[p[0x443]].flags & 0x100) {
        D_80099D88[p[0x443]].unk3 = (rand() & 3) + 1;
    }
    for (idx = 0; idx < 8U; idx++) {
        (p + idx)[0x444] = 0;
    }
    *(u16 **)(p + 0x3A4) = arg1;
    for (idx = 0; idx < 2; idx++) {
        if (idx) {
            rec = *(u8 **)p;
            cursor = arg2;
            list = (u8 *)arg2;
            chr = *(s16 *)(rec + 0xA);
        } else {
            rec = p;
            cursor = arg1;
            list = (u8 *)arg1;
            chr = p[0x443];
        }
        *(s16 *)(rec + 0x40A) = (*(s16 *)(rec + 0x1A) - 0x1000) * 225 >> 11;
        for (sec = 0; sec < 3; sec++) {
            bit = 1 << chr;
            lo = 0xFFFF;
            hi2 = 0;
            hi1 = 0;
            if (idx == 0) {
                *(u16 **)(p + 0x3A8 + sec * 4) = cursor;
            }
            while (*cursor != 0) {
                u8 *e = list + *cursor;
                if (e[4] == 0x40) {
                    temp = (e[8] << 24) | (e[7] << 16) | (e[6] << 8) | e[5];
                    if (!(temp & bit)) {
                        goto next;
                    }
                }
                if (e[1] != 0 && e[1] != 0xFF) {
                    temp = e[1];
                    if (temp < lo) {
                        lo = temp;
                    }
                }
                if (e[2] != 0 && e[2] != 0xFF) {
                    temp = e[2];
                    if (hi1 < temp) {
                        hi1 = temp;
                    }
                    cat = e[0] & 7;
                    if (hi2 < temp && (e[3] & 0xF) * 4 < 0x10 && (cat < 2 || cat == 7)) {
                        hi2 = temp;
                    }
                }
            next:
                cursor++;
            }
            if (*(s16 *)(rec + 0xE) >= 6) {
                lo_val = 0;
                hi2_val = 0x7530;
                hi1_val = 0x7530;
            } else {
                /* FAKE: the shared base (rec's 0x40A halfword + 100) is staged
                 * through hi2_val, whose own value (base + hi2 * 40) is
                 * completed below; staged-value-reused-variable. Mechanism: a
                 * separate base local lives in one basic block, so
                 * local-alloc.c combine_regs ties it to the dying lh result
                 * (lh v1; addiu v1,v1,100); hi2_val is set in both arms and
                 * read after the join, so it is global-allocated and untied
                 * (target: lh v0; addiu v1,v0,100). Lever exhaustion:
                 * memory/grind/func_80055138/ruling11.md. */
                hi2_val = *(s16 *)(rec + 0x40A) + 100;
                lo_val = hi2_val + lo * 40;
                hi1_val = hi2_val + hi1 * 40;
                hi2_val += hi2 * 40;
            }
            cursor++;
            ((s16 *)(rec + 0x3F8))[sec] = lo_val;
            ((s16 *)(rec + 0x3FE))[sec] = hi1_val;
            ((s16 *)(rec + 0x404))[sec] = hi2_val;
        }
    }
    other = *(u8 **)p;
    *(s8 *)(p + 0x40D) = -1;
    *(s8 *)(p + 0x40C) = -1;
    *(s16 *)(p + 0x428) = -1;
    p[0x425] = 0;
    p[0x426] = 0;
    *(s32 *)(p + 0x3B4) = 0;
    *(s32 *)(p + 0x3E0) = 0;
    *(s32 *)(p + 0x3DC) = 0;
    *(s32 *)(p + 0x3D8) = 0;
    other[0x440] = 0;
    p[0x440] = 0;
    other[0x441] = 0;
    p[0x441] = 0;
    *(s16 *)(other + 0x43C) = 0;
    *(s16 *)(other + 0x43A) = 0;
    *(s16 *)(p + 0x43C) = 0;
    *(s16 *)(p + 0x43A) = 0;
    p[0x362] = 0;
    p[0x39D] = 0;
    p[0x3F5] = 0;
    p[0x3F4] = 0;
    p[0x3F3] = 0;
    p[0x3F2] = 0;
    *(s16 *)(p + 0x3EE) = 0;
    *(s16 *)(p + 0x3F0) = 0;
    *(s16 *)(p + 0x3E8) = 0;
    p[0x39C] = 0;
    *(s32 *)(p + 0x394) = 0;
    *(s16 *)(p + 0x398) = 0;
    *(s32 *)(p + 0x3C4) = 0;
    *(s16 *)(p + 0x3C2) = 0;
    p[0x3C1] = 0;
    p[0x3C0] = 0;
    p[0x440] = 0;
    *(s32 *)(p + 0x430) = 0;
    *(s32 *)(p + 0x3E4) = -1;
}

s32 func_80055948(u8 *arg0) {
    u8 *p;
    u8 ctr;
    s32 t;
    s32 mask;
    s32 dx, dy, dz;

    ctr = arg0[0x3B8];
    p = *(u8 **)(arg0 + 0x3B4);
    if (ctr != 0) {
        if (*(s16 *)(arg0 + 0x46) == 0) {
            arg0[0x3B8] = ctr - 1;
        }
        goto ret_3c8;
    }
    if (arg0[0x3BC] != 0) {
        goto check_loop;
    }
    {
        u8 idx = arg0[0x443];
        if (idx == 22) goto check_loop;
        if ((D_80099D88[idx].flags & 0xBF00) != 0) goto check_loop;
        {
            s32 limit;
            if (*(s32 *)(arg0 + 0x430) & 0x200) {
                limit = (*(s16 *)(arg0 + 0x43C) < 0x801);
            } else {
                limit = (*(s16 *)(arg0 + 0x43C) < 0x401);
            }
            if (limit == 0) goto reset_ret_neg1;
        }
        if (*(s32 *)(arg0 + 0x430) & 0x800) goto reset_ret_neg1;
        if ((u32)(arg0[0x425] - 1) < 2U) goto reset_ret_neg1;
        if (arg0[0x442] != 0) goto reset_ret_neg1;
        {
            u8 *other = *(u8 **)arg0;
            if (*(u16 *)(other + 0x6A) == 0x2D) goto reset_ret_neg1;
            dx = *(s32 *)(other + 0xF4) - *(s16 *)(arg0 + 0x40E);
            dy = *(s32 *)(other + 0xFC) - *(s16 *)(arg0 + 0x410);
            dz = *(s16 *)(arg0 + 0x412);
            if ((dz * dz) >= ((dx * dx) + (dy * dy))) goto loop;
        }
    }
    goto reset_ret_neg1;
check_loop:
    if (arg0[0x3BC] != 1) goto loop;
    if (*(s32 *)(arg0 + 0x430) & 0x800) {
        goto loop;
    }
reset_ret_neg1:
    *(s32 *)(arg0 + 0x3B4) = 0;
    return -1;
sentinel_reset:
    *(s32 *)(arg0 + 0x3B4) = 0;
    goto ret_3c8;
loop:
    while (1) {
        t = *p;
        p += 1;
        if (t & 0x80) {
            if (t == 0x80) goto sentinel_reset;
            *(s32 *)(arg0 + 0x3B4) = (s32)p;
            arg0[0x3B8] = (t & 0x7F) - 1;
            goto ret_3c8;
        }
        mask = 1 << (t & 0xF);
        if (t & 0x10) {
            *(s32 *)(arg0 + 0x3C8) &= ~mask;
        } else {
            *(s32 *)(arg0 + 0x3C8) |= mask;
        }
    }
ret_3c8:
    return *(s32 *)(arg0 + 0x3C8);
}
void func_80055B44(u8 *a0, s32 a1, s32 a2, s32 a3) {
    *(s32 *)(a0 + 0x3B4) = a1;
    a0[0x3BC] = (u8)a2;
    a0[0x3B8] = (u8)a3;
    *(s32 *)(a0 + 0x3C8) = 0;
    *(s32 *)(a0 + 0x3CC) = -1;
}
INCLUDE_ASM("asm/funcs", func_80055B60);
extern s32 ratan2(s32, s32);
extern u8 D_8009A820[];
extern u8 D_8009A821[];

void func_80056CB8(s32 arg0) {
    s32 pt0[4];
    s32 pt1[4];
    s32 hit0[4];
    s32 hit1[4];
    s32 work[2];
    s32 start;
    s32 i;

    start = (*(u16 *)(arg0 + 0x3E8) & 3) * 2;
    for (i = start; i < start + 2; i++) {
        s32 obj;
        s32 flags;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;
        s32 idx;

        /* FAKE: idx names the byte-table index for the first lookup only, mechanism:
           loop.c strength_reduce giv-worth test (lifetime * threshold * benefit >= insn_count),
           lever-exhaustion: memory/grind/func_80056CB8/hypotheses.md [s72] + rejected/ */
        idx = i * 2;
        obj = arg0;
        flags = D_8009A821[idx] << 8;
        if ((flags & 0x1000) != 0) {
            obj = *(s32 *)arg0;
        }

        if (*(u16 *)(arg0 + 0x6A) == 0x13 || *(u16 *)(arg0 + 0x6A) == 6) {
            flags += *(s16 *)(obj + 0x1CA);
        } else {
            flags += ratan2(D_800F6608.w0 - *(s32 *)(obj + 0xF4),
                             D_800F6608.w8 - *(s32 *)(obj + 0xFC));
        }

        sin_p = &Judge[flags & 0xFFF];
        scale = D_8009A820[i * 2] << 8;
        x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
        cos_p = &Judge[(flags + 0x400) & 0xFFF];
        z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
        pt0[0] = *(s32 *)(obj + 0xB8);
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt0[2] = *(s32 *)(obj + 0xC0);
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt1[2] = z;

        flags = func_80053614(pt0, pt1, (s32)hit0, (s32)work, 0x1F8002B8);
        if (flags != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        pt0[0] = x;
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x834;
        pt0[2] = z;
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) + 0x1004;
        pt1[2] = z;

        flags |= func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8) << 1;
        flags += 1;
        if (flags == 3 && hit1[1] - *(s32 *)(obj + 0xBC) < 5) {
            flags = 0;
        } else if (flags == 4) {
            s32 dx = hit0[0] - *(s32 *)(obj + 0xB8);
            s32 dz = hit0[2] - *(s32 *)(obj + 0xC0);
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = *(s32 *)(obj + 0xBC);
                if ((y - hit1[1] >= 0 ? y - hit1[1] : hit1[1] - y) >= 0x3E9) {
                    flags = 5;
                }
            }
        }
        *(s8 *)(arg0 + i + 0x444) = (s8)flags;
    }
}
#undef sp18
#undef sp1C
#undef sp20
#undef sp28
#undef sp2C
#undef sp30
#undef sp38
#undef sp3C
#undef sp40
#undef sp48
#undef sp4C
#undef sp50
#undef sp58
#undef sp5C
#undef sp60
#undef sp68
#undef sp70
#undef sp78
extern u8 D_8009A830;
extern s8 D_8009A838;
extern u8 D_8009A840;
/* ang_hosei_80056FE8 / func_80056FE8 -- angle-correction table lookup.
 *
 * MATCH-HACK FAMILY: duplicated-statement-into-arms
 * (.claude/rules/duplicated-statement-into-arms.md; frozen SOTN entry
 *  .claude/rules/no-new-park-categories.md:285-293).
 * The single real statement "add this arm's angle adjustment into `base`" is
 * written once PER DISPATCH ARM instead of being cached in a temp and added
 * once after the join. Each copy is REAL on its path (prereq 1) and the copies
 * are re-merged byte-neutrally by post-reload cross-jumping (prereq 2).
 *
 * Lever-exhaustion (prereq 3): memory/grind/func_80056FE8/hypotheses.md +
 * evidence.md, sessions s1-s7b -- structural reassociation (21 forms, s2/s3),
 * permuter (4 chassis, ~150k iters, s4/s5), copy-preference (s6: $a1 has no
 * ABI anchor in this 1-argument leaf, set_preference/global.c:1591 cannot
 * create one), scheduling wrappers (s6: do-while(0) at 3 placements never
 * shrinks base's live length), live-range shortening (s6: backfires -- raises
 * priority), register pins (s6: reschedule to 41 insns, 2 load-delay nops
 * lost). All measured dead.
 */
s32 func_80056FE8(s32 arg0) {
    s32 a2 = *((s32 *) arg0);
    s32 a3 = *((u8 *) ((*((s32 *) (a2 + 0x58))) + 3));
    s32 base = a3 * 40;
    /* FAKE: `base += <arm value>` duplicated into all three dispatch arms
     * instead of a post-join combine; mechanism: GCC 2.7.2 jump2 post-reload
     * cross-jumping tail-merges the three copies into the target's single join
     * `addu $a1,$a1,$v0` (byte-neutral, build 43 == target 43), while the
     * reference-count lift flow.c records (reg_n_refs 4 -> 8) raises `base`'s
     * global.c allocno priority above the struct pointer's so global_alloc
     * colours `base` first (.greg `;; 3 regs to allocate: 77 73 72` ->
     * `77 in 5  73 in 6`) -- which the post-join spelling provably cannot
     * (`;; 4 regs to allocate: 82 73 77 72` -> `73 in 5  77 in 6`);
     * lever-exhaustion: see the ladder in this function's preamble comment and
     * memory/grind/func_80056FE8/hypotheses.md s1-s7b. */
    if ((*((u8 *) (a2 + 0xA3))) != 0xFF) {
        if ((*((s16 *) (arg0 + 0x5E))) == 0) {
            /* FAKE: duplicated copy (see above) */
            base += (*((u8 *) (((s32) (&D_8009A830)) + (*((s16 *) (a2 + 0xE)))))) * 2;
        } else {
            /* FAKE: duplicated copy (see above) */
            base += (*((s8 *) (((s32) (&D_8009A838)) + (*((s16 *) (a2 + 0xE)))))) * 8;
        }
    } else {
        /* FAKE: duplicated copy (see above) */
        base += (*((u8 *) (((s32) (&D_8009A840)) + (*((s16 *) (a2 + 0x14)))))) * 2;
    }
    return base + (*((s16 *) ((*((s32 *) arg0)) + 0x40A))) + 0x12C;
}
extern s32 ratan2(s32, s32);
extern s32 func_800233AC(void *, s32 *);
extern s32 D_8009AA50[];

s32 func_80057094(void *arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 sp10;
    s32 temp_s0;
    s32 temp_v0;
    s32 temp_v1;
    s32 var_v0;

    temp_s0 = ratan2(D_800F6608.w0 - *(s32 *)((s32)arg0 + 0xF4), D_800F6608.w8 - *(s32 *)((s32)arg0 + 0xFC));
    var_v0 = temp_s0 - ratan2(arg1 - *(s32 *)((s32)arg0 + 0xF4), arg2 - *(s32 *)((s32)arg0 + 0xFC));
    var_v0 -= 0x100;
    temp_v0 = (s32)var_v0 >> 9;
    temp_v1 = temp_v0 & 7;
    sp10 = temp_v1;
    if ((arg3 == 0) && !(temp_v0 & 1)) {
        if (*(u16 *)((s32)arg0 + 0x3E8) & 0x10) {
            sp10 = temp_v1 + 1;
            if (sp10 >= 8) {
                sp10 = 0;
            }
        } else {
            sp10 = temp_v1 - 1;
            if (sp10 < 0) {
                sp10 = 7;
            }
        }
    }
    var_v0 = D_8009AA50[sp10 & 7];
    if (arg3 == 1) {
        var_v0 |= 4;
    }
    if (func_800233AC(arg0, &sp10) != 0) {
        var_v0 |= 8;
    }
    return var_v0;
}
typedef struct { s32 x, y, z, w; } Vec4_571C0;
extern s32 rand(void);

s32 func_800571C0(s32 obj) {
    Vec4_571C0 probe;
    Vec4_571C0 top;
    Vec4_571C0 left;
    Vec4_571C0 right;
    s32 hit[4];
    s16 work[4];
    s32 ret;
    s8 nl;
    s8 nr;
    u8 goL;
    u8 goR;
    s32 ang;
    s32 rad;
    s32 a;
    s32 p;
    s32 dx;
    s32 dz;
    s32 x;
    s32 e;
    s32 z;

    nr = 0;
    nl = 0;
    goR = 1;
    goL = 1;
    ret = 0;
    left.x = *(s32 *)(obj + 0xB8);
    left.y = *(s32 *)(obj + 0xBC) - 5;
    rad = D_800A387C + 800;
    left.z = *(s32 *)(obj + 0xC0);
    right = left;
    for (ang = 0x200; ang <= 0x800; ang += 0x200) {
        if (goL) {
            p = *(s32 *)obj;
            a = *(s16 *)(p + 0x1D8) + ang;
            goL = 0;
            dx = rad * Judge[a & 0xFFF];
            dz = rad * Judge[(a + 0x400) & 0xFFF];
            x = *(s32 *)(p + 0xB8) + (dx >> 12);
            z = *(s32 *)(p + 0xC0) + (dz >> 12);
            probe.x = x;
            probe.y = *(s32 *)(obj + 0xBC) - 5;
            probe.z = z;
            top.x = x;
            top.y = *(s32 *)(obj + 0xBC) + 5;
            top.z = z;
            if (func_80053614(&probe.x, &top.x, (s32)hit, (s32)work, 0x1F8002B8) != 0) {
                goL = func_80053614(&left.x, &probe.x, (s32)hit, (s32)work, 0x1F8002B8) == 0;
            }
            if (goL) {
                left = probe;
                nl++;
            }
        }
        if (goR) {
            p = *(s32 *)obj;
            a = *(s16 *)(p + 0x1D8) - ang;
            goR = 0;
            dx = rad * Judge[a & 0xFFF];
            dz = rad * Judge[(a + 0x400) & 0xFFF];
            x = *(s32 *)(p + 0xB8) + (dx >> 12);
            z = *(s32 *)(p + 0xC0) + (dz >> 12);
            probe.x = x;
            probe.y = *(s32 *)(obj + 0xBC) - 5;
            probe.z = z;
            top.x = x;
            top.y = *(s32 *)(obj + 0xBC) + 5;
            top.z = z;
            if (func_80053614(&probe.x, &top.x, (s32)hit, (s32)work, 0x1F8002B8) != 0) {
                goR = func_80053614(&right.x, &probe.x, (s32)hit, (s32)work, 0x1F8002B8) == 0;
            }
            if (goR) {
                right = probe;
                nr++;
            }
        }
    }
    if (nl != 0 || nr != 0) {
        if (nl == nr) {
            if (rand() & 1) {
                nl = 0;
            } else {
                nr = 0;
            }
        }
        if (nl < nr) {
            nl = nr;
            nr = 0;
        } else {
            nr = 1;
        }
        ret = nl--;
        for (ang = 0x200; nl >= 0; nl--, ang += 0x200) {
            s32 base = *(s16 *)(*(s32 *)obj + 0x1D8);
            if (nr != 0) {
                a = base + ang;
            } else {
                a = base - ang;
            }
            e = obj + nl * 6;
            *(s16 *)(e + 0x364) = *(s32 *)(*(s32 *)obj + 0xB8) + ((D_800A387C * Judge[a & 0xFFF]) >> 12);
            *(s16 *)(e + 0x366) = *(s32 *)(*(s32 *)obj + 0xC0) + ((D_800A387C * Judge[(a + 0x400) & 0xFFF]) >> 12);
            *(u8 *)(e + 0x368) = 2;
        }
        *(s16 *)(obj + 0x398) = 0;
        *(s16 *)(obj + 0x3A0) = *(s16 *)(obj + 0x364);
        *(s16 *)(obj + 0x3A2) = *(s16 *)(obj + 0x366);
        *(s16 *)(obj + 0x39E) = *(u8 *)(obj + 0x368);
    }
    return ret;
}
s32 func_8005763C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 *arg8, s32 *arg9) {
    s32 slope1;
    s32 x1;
    s32 dx1;
    s32 dy1;
    s32 dx2;
    s32 dy2;
    s32 slope2;
    s32 intercept1;
    s32 intersection_x;
    s32 y;

    x1 = arg0;
    x1 >>= 3;
    arg2 >>= 3;
    arg1 >>= 3;
    arg3 >>= 3;
    dx1 = arg2 - x1;
    dy1 = arg3 - arg1;
    arg4 >>= 3;
    arg6 >>= 3;
    arg5 >>= 3;
    arg7 >>= 3;
    dx2 = arg6 - arg4;
    dy2 = arg7 - arg5;
    if ((arg4 == arg6) && (arg1 == arg3)) {
        *arg8 = arg6;
        *arg9 = arg3;
        goto check_bounds;
    }
    if ((x1 == arg2) && (arg5 == arg7)) {
        *arg8 = arg2;
        *arg9 = arg7;
        goto check_bounds;
    }
    if ((arg4 == arg6) && (x1 != arg2)) {
        *arg8 = arg6;
        *arg9 = arg1 + (dy1 * (arg6 - x1)) / dx1;
        goto check_bounds;
    }
    if ((x1 == arg2) && (arg4 != arg6)) {
        *arg8 = x1;
        *arg9 = arg5 + (dy2 * (x1 - arg4)) / dx2;
        goto check_bounds;
    }
    if ((arg1 == arg3) && (arg5 != arg7)) {
        *arg9 = arg3;
        *arg8 = arg4 + (dx2 * (arg3 - arg5)) / dy2;
        goto check_bounds;
    }
    if ((arg5 == arg7) && (arg1 != arg3)) {
        *arg9 = arg5;
        *arg8 = x1 + (dx1 * (arg5 - arg1)) / dy1;
        goto check_bounds;
    }
    if (arg2 == x1) {
        return 0;
    }
    {
        if (arg6 != arg4) {
            slope1 = (dy1 << 7) / dx1;
            slope2 = (dy2 << 7) / dx2;
            if (slope1 != slope2) {
                intercept1 = (((arg1 * arg2) - (arg3 * x1)) << 7) / dx1;
                intersection_x = (((((arg5 * arg6) - (arg7 * arg4)) << 7) / dx2) - intercept1) / (slope1 - slope2);
                *arg8 = intersection_x;
                *arg9 = ((intersection_x * slope1) + intercept1) >> 7;
check_bounds:
                if ((((*arg8 - x1) >= -50) || (((*arg8 - arg2) < -50) == 0)) &&
                    (((x1 - *arg8) >= -50) || (((arg2 - *arg8) < -50) == 0)) &&
                    ((y = *arg9, (((arg1 - y) < -50) == 0)) || (((arg3 - y) < -50) == 0)) &&
                    (((y - arg1) >= -50) || (((y - arg3) < -50) == 0)) &&
                    (((*arg8 - arg4) >= -50) || (((*arg8 - arg6) < -50) == 0)) &&
                    (((arg4 - *arg8) >= -50) || (((arg6 - *arg8) < -50) == 0)) &&
                    (((arg5 - y) >= -50) || (((arg7 - y) < -50) == 0))) {
                    if ((y - arg5) >= -50) {
                        goto intersection_found;
                    }
                    if ((y - arg7) >= -50) {
                        goto intersection_found;
                    }
                }
                goto no_intersection;
            }
        }
no_intersection:
        return 0;
    }
intersection_found:
    *arg8 = *arg8 << 3;
    *arg9 = *arg9 << 3;
    return 1;
}
extern s32 func_8005763C(s32, s32, s32, s32, s32, s32, s32, s32, s32 *, s32 *);
extern s32 SquareRoot0(s32);

s32 func_80057ACC(s32 arg0, u8 *arg1, s32 arg2, s32 arg3) {
    s32 sp28;
    s32 sp2C;
    s32 best;
    s16 i;
    s16 j;
    s16 k;
    s16 n;
    u8 *poly;
    s32 dx;
    s32 dy;
    s32 d;

    best = 100000;
    for (i = 0; i < arg1[0]; i++) {
        poly = (u8 *)(*(s32 *)(arg1 + 4) + i * 8);
        n = poly[3];
        if (poly[0] & 0x80) {
            n = poly[3] - 1;
        }
        for (j = 0; j < n; j++) {
            k = j + 1;
            if (!(k < poly[3])) {
                k = 0;
            }
            if (func_8005763C(*(s32 *)(arg0 + 0xF4), *(s32 *)(arg0 + 0xFC), arg2, arg3,
                              *(s16 *)(*(s32 *)(poly + 4) + j * 4),
                              *(s16 *)(*(s32 *)(poly + 4) + j * 4 + 2),
                              *(s16 *)(*(s32 *)(poly + 4) + k * 4),
                              *(s16 *)(*(s32 *)(poly + 4) + k * 4 + 2),
                              &sp28, &sp2C) != 0) {
                dx = sp28 - *(s32 *)(arg0 + 0xF4);
                dy = sp2C - *(s32 *)(arg0 + 0xFC);
                d = SquareRoot0(dx * dx + dy * dy);
                if (d < best) {
                    best = d;
                    *(u8 *)(arg0 + 0x360) = i;
                    *(u8 *)(arg0 + 0x361) = j;
                }
            }
        }
    }
    return best;
}
extern s32 ratan2(s32, s32);
/* Per-vertex neighbour-angle midpoint: computes the outward bisector direction at
 * vertex arg1 of the polygon whose vertex table hangs off arg0[4], and writes the
 * offset point into *arg2 / *arg3.
 *
 * FAKE: the vertex-table base expression *(s16 **)(arg0 + 4) is written out at each
 * of its five use sites rather than bound to one pointer local (F3
 * compound-address duplication across call arg-lists, .claude/rules/no-new-park-categories.md:377,
 * owner ruling 2026-08-18; re-adjudication granted for this function by owner ruling
 * 6b of the 2026-08-30 escalation batch, docs/grind/decisions.md:14685).
 * mechanism: cse1 (cse.c:1948 hash_arg_in_memory / cse.c:7241-7246
 * `if (! CONST_CALL_P (insn)) invalidate_memory (&everything);`) folds the five
 * front-end loads down to the target's two, the intervening ratan2 CALL_INSN being
 * the only thing that stops the fold; a single cached local instead asserts the
 * call cannot write ((s16 **)arg0)[1], which C does not guarantee and which folds
 * to one load (s40 probe pA/pB/pC/pD, tmp/grind/func_80057CC8/s40/probe.c).
 * lever-exhaustion: memory/grind/func_80057CC8/hypotheses.md (46 sessions, 133
 * rejected forms, three ban-compliant regimes foreclosed in closed form at honest
 * floor 16; evidence.md s40-s45).
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 *p;
    s32 pi;
    u16 cy;

    prev_idx = arg1 - 1;
    cx = *(u16 *)((s32)(*(s16 **)(arg0 + 4)) + arg1 * 4 + 0);
    cy = *(u16 *)((s32)(*(s16 **)(arg0 + 4)) + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2((*(s16 **)(arg0 + 4))[pi * 2] - (s16) cx,
                      (*(s16 **)(arg0 + 4))[pi * 2 + 1] - (s16) cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = ratan2(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        /* FAKE: `base` and `half` are fresh once-written/once-read named
         * intermediates for the antipode of ang_prev and half the angular gap
         * (named-intermediate family, .claude/rules/no-new-park-categories.md:204
         * + the 2026-08-17 clarification at :208-229; both values are real and
         * appear in the target's own bytes, build_insns == target_insns == 111).
         * mechanism: local-alloc.c block_alloc -- they become BLOCK-LOCAL allocnos
         * (pseudos 82 and 83, "in block 5", tmp/grind/func_80057CC8/dumps/text1b.lreg
         * at the func_80057CC8 heading) that local-alloc seats before global.c runs;
         * collapsing them into one expression instead yields a single combine-folded
         * tree whose scratch is allocated globally and measures score 6.
         * lever-exhaustion: memory/grind/func_80057CC8/hypotheses.md (46 sessions);
         * both collapse spellings banked in
         * rejected/s46-collapse-base-half-splitinit-score6.c. */
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)Judge[ang_mid & 0xFFF]) >> 12);
    *arg3 = cy + ((scale * (s32)Judge[((s16)ang_mid + 0x400) & 0xFFF]) >> 12);
}

INCLUDE_ASM("asm/funcs", func_80057E84);
/* func_80058580's three switch tables (0x8001585C..0x800158B4): the rodata of this TU,
 * transcribed from src/text1a_b_pre_rodata.c while the function is INCLUDE_ASM
 * (rodata-object-alignment ruling 2026-09-30; docs/grind/rodata-align-2026-09-30.md s8). */
/* jtbl_8001585C: 10 words (40B) @ 0x8001585C */
const u32 jtbl_8001585C[10] = {
    0x80059DD0,
    0x80059DE8,
    0x80059E00,
    0x80059E18,
    0x80059E30,
    0x80059E7C,
    0x80059EC4,
    0x80059F10,
    0x80059F5C,
    0x00000000,
};

/* jtbl_80015884: 6 words (24B) @ 0x80015884 */
const u32 jtbl_80015884[6] = {
    0x8005A190,
    0x8005A238,
    0x8005A190,
    0x8005A238,
    0x8005A2C0,
    0x00000000,
};

/* jtbl_8001589C: 6 words (24B) @ 0x8001589C */
const u32 jtbl_8001589C[6] = {
    0x8005A418,
    0x8005A430,
    0x8005A458,
    0x8005A480,
    0x8005A4A8,
    0x8005A4B4,
};

INCLUDE_ASM("asm/funcs", func_80058580);
