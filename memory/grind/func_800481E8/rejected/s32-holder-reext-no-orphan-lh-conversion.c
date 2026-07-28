/* REJECTED (s2, structural): the entire "s32-holder + redundant (s16)
 * re-extension" spelling family CANNOT produce phantom frame slots here.
 *
 * Hypothesis was: hold a stream value in an s32 local (extended once at
 * load), then re-cast (s16) at use sites; combine deletes the redundant
 * re-extension chain post-flow -> stale-ref orphan -> vars>0.
 *
 * Measured (tmp/grind/func_800481E8/s2/probe.py, cc1 .frame vars=):
 *   V1 (a3w + re-ext at compare AND call arg)  vars=0, and the load
 *      becomes `lh` (combine folds (s16)(u16-load) into a sign-extending
 *      load) -- BYTE-BREAK: target requires lhu + live sll/sra.
 *   V2/V3 (single-site re-ext)                 vars=0, same lh conversion.
 *   V4 (holder, no re-ext)                     vars=0, lh conversion.
 *   V9 (holder + staged s16 view)              vars=0, lh conversion.
 *   V10 (all three signed values as holders)   vars=0.
 *   V5/V6/V7 controls: tree-folds / byte-identical alt spellings, vars=0.
 *
 * Root cause (bisect, tmp/grind/func_800481E8/s2/bisect2.py + mr/):
 * the stale-ref orphan (reload alter_reg, func_80037540 H14) fires ONLY
 * when combine deletes an emitted 2-insn sign-extension chain
 * (sll16/sra16 with an intermediate pseudo), and every deletion route is
 * VISIBLE in the emitted bytes: either the chain vanishes (high-bits-dead
 * consumer like `& 1`), or the lhu converts to lh, or extra consumer insns
 * (andi/branch) appear. func_800481E8's target keeps all 4 lhu loads and
 * all 4 sll/sra pairs live, and its semantics contain NO low-bit-only
 * consumer of any halfword value -> no honest spelling can host a deleted
 * extension -> no orphan -> vars stays 0. Confirmed quantitatively by a
 * 52,043-iteration instrumented permuter campaign: 0 clean forms above
 * vars=8, 0 clean forms near byte-match, all vars=32 forms volatile cheats.
 */
void func_800481E8_V1_representative(s32 arg0, s32 arg1)
{
    u32 *p;
    u32 *base;
    s32 count;
    s32 a0_for_call;
    p = (u32 *)arg0;
    base = p;
    arg0 = 0;
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)((s32)base + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        count--;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s32 a3w;   /* s32 holder */
            u16 v0v;
            unsigned int new_var2;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3w = (s16)(*((u16 *)p));   /* becomes `lh` -- byte-break */
            p = (u32 *)(((s32)p) + 2);
            v0v = *((u16 *)p);
            new_var2 = word >> 2;
            a0_for_call = (s32)base + (new_var2 << 2);
            p = (u32 *)(((s32)p) + 2);
            if ((s16)a3w < 0x280) {     /* redundant re-ext: folded, no orphan */
                v0v += 1;
            }
            efc_buki_draw_zanzou(a0_for_call,
                          (s32)a1v,
                          (s32)a2v,
                          (s32)(s16)a3w,  /* redundant re-ext: folded, no orphan */
                          (s32)(s16)v0v);
        } while ((count--) != 0);
    }
}
