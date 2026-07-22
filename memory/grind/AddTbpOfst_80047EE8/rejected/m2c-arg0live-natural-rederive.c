/* REJECTED — s8 rederive (fresh m2c structural shape). sandbox --disable all
 * = 35, build_insns 52 (target 53). WORSE than the floor-10 candidate and one
 * instruction short.
 *
 * Provenance: `python3 tools/m2c/m2c.py --valid-syntax -f AddTbpOfst_80047EE8
 * asm/funcs/AddTbpOfst_80047EE8.s` (tmp/grind/AddTbpOfst_80047EE8/s8/m2c_fresh.c),
 * transcribed faithfully into project idioms. This is m2c's independent
 * reconstruction directly from the TARGET bytes.
 *
 * Two rederive facts this shape establishes:
 *  1. The natural/independent reconstruction keeps arg0 LIVE as the base
 *     throughout (`efc_buki_draw_zanzou(arg0 + ((temp_a0>>2)<<2), ...)`), so
 *     GCC copy-props base==arg0 and emits the second pointer bind from $a0
 *     (not the target's $s2). This is exactly the arg0-live basin the s1/s2
 *     FAKE-arg0 lever exists to flip; without the lever it is WORSE (35) and
 *     52 insns, corroborating the s2 finding that the FAKE store is the only
 *     construct that reaches the $s2 binding on this body.
 *  2. m2c declares NO local aggregate / NO frame array. An independent
 *     decompiler working from the target bytes produces a body with vars=0 —
 *     the target's 32-byte phantom vars region (0x18-0x37, zero sw/lw) has NO
 *     semantic correlate. This independently re-confirms the s6/s7 forensic
 *     mechanism (function.c assign_stack_local from a source-level dead
 *     local-array DECL, reproducible only by the forbidden
 *     dead-vars-local-array; no struct substitute; WRITTEN carve-out
 *     inapplicable).
 */
void AddTbpOfst_80047EE8(s32 arg0, s32 arg1)
{
    u32 *temp_s0;
    s32 temp_s1;
    s32 var_s1;
    u32 *var_s0;
    temp_s0 = (u32 *) (arg0 + (((*(u32 *) (arg0 + (((s32) (arg1 << 16)) >> 14))) >> 2) << 2));
    temp_s1 = *temp_s0;
    var_s0 = temp_s0 + 1;
    if (temp_s1 != 0)
    {
        var_s1 = temp_s1 - 1;
        do
        {
            u32 temp_a0;
            u16 temp_a1;
            u16 temp_a2;
            u16 temp_a3;
            u16 temp_v0;
            u32 *temp_s0_2;
            u32 *temp_s0_3;
            u32 *temp_s0_4;
            temp_a0 = *var_s0;
            temp_s0_2 = (u32 *) (((s32) var_s0) + 4);
            temp_a1 = *((u16 *) temp_s0_2);
            temp_s0_3 = (u32 *) (((s32) temp_s0_2) + 2);
            temp_a2 = *((u16 *) temp_s0_3);
            temp_s0_4 = (u32 *) (((s32) temp_s0_3) + 2);
            temp_a3 = *((u16 *) temp_s0_4);
            temp_v0 = *((u16 *) (((s32) temp_s0_4) + 2));
            var_s0 = (u32 *) (((s32) temp_s0_4) + 2 + 2);
            efc_buki_draw_zanzou(arg0 + ((temp_a0 >> 2) << 2), (s16) temp_a1, (s16) temp_a2, (s16) temp_a3, (s32) (s16) temp_v0);
            var_s1 -= 1;
        }
        while (var_s1 != 0);
    }
}
