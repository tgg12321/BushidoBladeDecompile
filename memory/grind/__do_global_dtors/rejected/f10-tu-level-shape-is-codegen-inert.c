/*
 * REJECTED (session 8, rederive modality) — frontier F10 KILLED.
 *
 * CLAIM UNDER TEST. Every form measured in sessions 1-7 varies the BODY of
 * motion_Close inside the existing ings2.c. F10 held that a WHOLE-TU /
 * file-level change — the function's signature, whether its body arrives by
 * inlining a static helper shared with its sibling func_80083794, its position
 * in the TU — might move one of the three surviving residual mechanisms
 * (H1 frame, F5 $v0-vs-$t0, F7a/F7b prologue order + delay slot).
 *
 * PROBE. tmp/grind/motion_Close/s8/tusweep.py replaced the ENTIRE two-function
 * region of src/ings2.c per variant (both motion_Close and func_80083794),
 * measured each with `sandbox motion_Close --disable all` and read the
 * instrumented cc1's per-allocno table (BB2_ALLOC_DEBUG=1). Nine variants;
 * tmp/grind/motion_Close/s8/depthsweep.py then crossed the two inlined chassis
 * with do-while(0) wrap depths 0/1/2/3.
 *
 * RESULT — the TU-level axis is CODEGEN-INERT for this function.
 *
 *   v0_base13              13   (control, the banked candidate)
 *   v3_shared_inline_both  13   both functions written as ONE parameterised
 *                               `static __inline__ ctor_walk(void)` body
 *   v7_order_swap          13   motion_Close defined BEFORE func_80083794
 *   v2_inline_helper_count 16   helper takes count as a parameter
 *   v1_inline_helper_pc    20   helper takes (p, count) as parameters
 *   v9_helper_noinline     23   plain `static` helper, NOT inlined: 15 insns,
 *                               frame 24 (= 16 args + ra), the call moved out
 *
 * The three 13s are not merely equal in score — the allocno tables are
 * IDENTICAL (p pseudo nrefs=10 / live_length=8 / pri=37500 -> $s0; count
 * nrefs=8 / live_length=7 / pri=34285 -> $s1) and so are the emitted
 * instructions. The wrap-depth cross confirms the equivalence is exact, not
 * coincidental: on BOTH inlined chassis the depth ladder reproduces the
 * direct-body ladder value for value —
 *
 *   depth 0: p 7/8 = 17500,  count 8/7 = 34285  -> score 20
 *   depth 1: p 8/8 = 30000,  count 8/7 = 34285  -> score 20
 *   depth 2: p 9/8 = 33750,  count 8/7 = 34285  -> score 20
 *   depth 3: p 10/8 = 37500, count 8/7 = 34285  -> score 13
 *
 * i.e. GCC 2.7.2 inlines the helper BEFORE flow.c counts references, so an
 * inlined body and a written-out body are the same input to every pass that
 * matters here. F4b (reach 13 with wrap depth < 3) is therefore NOT helped by
 * the inlined chassis either — depth 3 remains necessary and minimal.
 *
 * CONSEQUENCE. The whole "TU-level rather than body-level" direction collapses
 * into the body-level search already exhausted across seven sessions. The only
 * TU-level change that alters motion_Close's codegen at all is one that removes
 * the call from motion_Close entirely (v9), which costs 10 instructions.
 *
 * The signature half of F10 is killed separately and more sharply — see
 * rejected/f7a-arg-copy-does-not-flip-save-order.c.
 */

/* v3_shared_inline_both — the structurally DIFFERENT shape that measures
   byte-identical to the banked candidate. Kept here as the record that the
   rederive produced a genuinely different C shape and it changed nothing. */

static __inline__ void ctor_walk(void) {
    s32 count;
    void (**p)(void);

    do { do { do { p = &D_8008D070; } while (0); } while (0); } while (0);
    count = (s32)&D_00000000;
    if (count != 0) {
        do {
            void (*f)(void) = *p;
            p++;
            f();
            count--;
        } while (count != 0);
    }
}

void func_80083794(void) {
    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        ctor_walk();
    }
}

void motion_Close(void) {
    if (D_800A2668 != 0) {
        ctor_walk();
    }
}
