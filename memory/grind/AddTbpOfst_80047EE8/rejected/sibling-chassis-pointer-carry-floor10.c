/* AddTbpOfst_80047EE8 — s9 rederive REJECTED (co-optimal, does NOT beat floor).
 *
 * Sibling InitHiraRmd_80047FBC committed-chassis transplant: a STRUCTURALLY
 * DIFFERENT pure-C shape from the floor-10 candidate.c (which carries the base
 * as `s32 saved`) AND from s8's m2c arg0-live rederive (which scored 35). Here
 * the base is carried as a `u32 *base` pointer, arg0=0 /* FAKE */ breaks the
 * cse2 canonical-reg class, and the call's first-arg offset is computed inside
 * the loop body between the a2v and a3v reads (the sibling's exact placement).
 *
 * MEASURED s9 (2026-07-21): `& tools/wteng.ps1 main sandbox AddTbpOfst_80047EE8
 * --disable all` = score 10, 53/53 insns. Full objdump diff
 * (tmp/grind/AddTbpOfst_80047EE8/s9/build.norm vs target.norm): the body is
 * BYTE-IDENTICAL to target EXCEPT the 10 frame-offset insns — addiu sp,-0x28
 * (build) vs -0x48 (target) and the 8 save/restore offsets shifted by the
 * 32-byte phantom vars region. IDENTICAL residual to the floor-10 candidate.
 *
 * CONCLUSION: a THIRD structurally-distinct legit pure-C chassis converges on
 * the EXACT same frame-only residual. Not only does no structurally-different
 * shape ESCAPE the 32-byte phantom frame (s8), multiple distinct reaching-forms
 * hit the SAME invariant residual — the residual is chassis-independent and is
 * the s6/s7-named function.c source-DECL phantom (dead >=32-byte local array),
 * upstream of every C-structure lever. Co-optimal, does not lower the floor.
 * Rederive modality re-confirmed DEAD by a fresh lane s8 did not run.
 */
void AddTbpOfst_80047EE8(s32 arg0, s32 arg1)
{
    u32 *p;
    u32 *base;
    s32 count;
    s32 new_var;
    p = (u32 *)arg0;
    base = p;
    arg0 = 0; /* FAKE */
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
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = (s32)base + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            efc_buki_draw_zanzou(new_var, a1v, a2v, a3v, v0v);
        } while ((count--) != 0);
    }
}
