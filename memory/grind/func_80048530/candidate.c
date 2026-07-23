/* func_80048530 — CLEAN pure-C form (no pins, no __asm__, no barriers).
 * sandbox --disable all = 10 (s1, 2026-07-23). NEW FLOOR: improved from the
 * prior clean floor of 12 / HEAD honest 11 by adopting the COMPLETED-C sibling
 * func_800483DC's base-routing idiom: mutate the arg0 parameter itself as the
 * walking pointer and hold `base = arg0` in a SEPARATE local for the final
 * `entry += base`. This forces the target's `move t0,v1` 2nd-copy-of-arg0 +
 * v1-as-walker routing (diffs #1/#2 of the WIP notes essentially resolved:
 * move a1,a3 now lands naturally in the beqz delay slot; move t0,v1 appears,
 * just scheduled after `lw v0` instead of before the sw's).
 *
 * RESIDUAL (score 10, 43 vs 47 insns):
 *   (a) 4-insn count gap: c,d compile to `lh` (the (s32)(s16)*(u16*) fold);
 *       target loads `lhu`+`sll`+`sra` (6 insns for c,d) like a,b.
 *   (b) scheduling: `move t0,v1` placed after `lw v0` vs before the `sw ra/sw
 *       s0` prologue stores in target.
 *
 * DO NOT re-try the "give c,d the lhu+sll+sra shape" fix in isolation — it
 * cascades to score 22 (walker v1->t0, base t0->t1 full register rename),
 * because the c/d sign-extend needs v0+v1 scratch which collides with the
 * v1-walker. Target REUSES the dead walker v1 as the d-scratch
 * (`lhu v0,0(v1); lhu v1,2(v1)`); our build won't. See
 * memory/grind/func_80048530/rejected/cd-signext-cascade22.c and
 * tmp/grind/func_80048530/s1/cd_signext_cascade22.txt. */
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base;
    s32 count;
    s32 entry;
    s32 a, b, c, d;
    s32 off;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    arg0 += off;
    count = *(s32 *)arg0;
    if (arg2 >= (u32)count) return -1;
    arg0 += 4;
    arg0 += arg2 * 0xC;
    entry = *(s32 *)arg0;
    arg0 += 4;
    a = (s32)*(u16 *)arg0;
    arg0 += 2;
    b = (s32)*(u16 *)arg0;
    arg0 += 2;
    c = (s32)(s16)*(u16 *)arg0;
    d = (s32)(s16)*(u16 *)(arg0 + 2);
    entry += base;
    func_800485EC(entry, arg3, (s16)a, (s16)b, c, d);
    return count;
}
