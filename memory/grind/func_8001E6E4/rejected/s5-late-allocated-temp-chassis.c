/* REJECTED (s5, permuter modality) — the whole "expansion-temp / late-declared
 * object" family (this was H1, the ledger's last live frontier axis).
 *
 * MECHANISM KILL: in GCC 2.7.2 the frame's vars area is handed out in
 * ALLOCATION ORDER, lowest sp offset first, and declared locals of the
 * function's outer block are allocated (expand_decl) BEFORE any statement is
 * expanded. Consequently anything created later — an inner-block declaration,
 * an expansion-time aggregate temp, a reload/alter_reg spill slot — lands
 * ABOVE `local`, growing the frame at the TOP while leaving `local` at
 * sp+0x10. The target needs `local` at sp+0x18, i.e. 8 bytes reserved BELOW
 * it. Therefore no expansion-temp mechanism can ever produce the target
 * layout, and H1's -da expansion-temp census cannot succeed.
 *
 * Controls measured this session (tmp/grind/func_8001E6E4/s5/screen.py):
 *   arr_first_dead  (dead s32 t[2] declared FIRST = the committed pre_pad
 *                    cheat shape)  -> frame 112, 71 insns, objdump diff 0
 *   arr_last_dead   (same dead array declared LAST)
 *                                  -> frame 112 (vars=80!) but `local` stays
 *                                     at sp+0x10 -> 9 differing insn pairs
 *   scope_pair      (8-byte aggregate in a disjoint INNER block, genuinely
 *                    used)         -> frame 112, q allocated at sp+0x58,
 *                                     `local` still at sp+0x10, 21 pairs
 *   agg_rot4 / agg_vec3 (aggregate copy temps declared after `local`)
 *                                  -> frame 112 / 120, +5 insns, `local`
 *                                     unmoved
 *   agg_member / agg_whole (aggregate assignment into a member / whole-struct
 *                    copy)         -> frame stays 104 (no separate temp at
 *                                     all; GCC copies member-wise in place)
 *
 * Reading: reaching vars=80 is NOT sufficient and never was — four separate
 * constructs reach it. The binding constraint is that the 8 bytes must be
 * allocated BEFORE `local` AND emit zero instructions, which in GCC 2.7.2
 * only an unused array (forbidden family), an unwritten volatile scalar
 * (forbidden family), or a wider declared TYPE for the work buffer itself
 * (the s1 pad_lead form, cheat-reviewer FAILed for want of evidence) can do.
 */
void func_8001E6E4_scope_pair_variant(s32 arg0) {
    CamWork *wp;
    CamWork local;              /* stays at sp+0x10 no matter what follows */
    s32 *s2;

    s2 = (s32 *)&D_800F5328;
    if ((u32)(arg0 - 0x555) >= 0x556U) {
        s2 = (s32 *)&D_800F6608;
    }
    {
        Pair2 q;                /* inner-scope 8 bytes -> allocated at sp+0x58 */
        q.a = s2[0] + D_800FF5C8;
        q.b = s2[1] + D_800FF5CC;
        local.vx = q.a;
        local.vy = q.b;
    }
    local.vz = s2[2] + D_800FF5D0;
    local.rx = *(u16 *)((u8 *)s2 + 0x10) + (u16)D_800FF5D8;
    wp = &local;
    local.ry = *(u16 *)((u8 *)s2 + 0x12) + (u16)D_800FF5DA;
    local.rz = *(u16 *)((u8 *)s2 + 0x14) + (u16)D_800FF5DC;
    local.dist = *(s32 *)((u8 *)s2 + 0x18) + D_800FF5E0;
    func_80046BF4((s32 *)wp, &local.rx, local.dist);
    {
        s32 *p20 = (s32 *)((u8 *)s2 + 0x20);
        func_8001A538((s32 *)&local, p20);
        func_80061064((s32 *)&local.rx, p20);
    }
    D_800A36B4 = (s32)s2;
}
