/* s8 REJECTED: score 18 at 138 insns. Naming the LoadImage source address in a
 * block-local `s32 src` declared BEFORE the rect stores (rather than inlining the
 * expression at the call) materialises the address three instructions early and the
 * scheduler cannot recover it. Distinct from s5's 'named address intermediate'
 * probes, which were declared AT the call and measured exactly inert. */
    s32 off = idx << 5;
    s32 src = (s32)((u8 *)&D_800A9A24 + off);
