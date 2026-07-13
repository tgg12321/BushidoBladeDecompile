/* REJECTED — sandbox --disable all = 33, build 89 insns vs target 87.
 *
 * The idea (good, and the MECHANISM IS REAL — it just over-fires): rewrite the
 * goto-loop as a real `for` so the loop gets NOTE_INSN_LOOP, and declare the
 * -1 sentinel in the loop BODY. LICM then hoists the loop-invariant `li -1` to
 * the preheader — i.e. before the loop, exactly where the target materializes
 * it — with NO function-scope constant-holder in the C at all.
 *
 * Confirmed in the disassembly (tmp/grind/func_80030BA8/s2/forloop.txt): LICM
 * DID hoist `li s3,-1` to the preheader (insn 10).
 *
 * WHY IT'S DEAD: loop notes enable LICM for *every* invariant at once, and it
 * also hoisted:
 *     lui  t0,0xf ; ori t0,t0,0x423f     <- the 0xF423F range constant
 *     addiu a2,s1,50                     <- a base-address computation
 * out of the loop. The target REQUIRES the lui/ori to sit INSIDE the loop, in
 * the mult/mflo latency window (asm lines 47-49). Hoisting it out is the +2
 * insns and most of the 33. It also swapped the $s0/$s1 roles.
 *
 * You cannot have it both ways: the goto-loop's suppression of LICM is what
 * keeps 0xF423F in the mult window, and that same suppression is what forces
 * the -1 to be a function-scope local. Same for `while(1){...break;}` — any
 * real loop construct emits the loop notes and re-triggers this.
 */
s32 func_80030BA8(u8 *arg0) {
    s32 i;
    u8 *p = (u8 *)&D_80106A7A;
    s32 old_val;

    for (i = 0; i < 12; i++, p += 0x64) {
        u16 val = *(u16 *)p;
        s32 sval;
        s32 empty_slot = -1;          /* LICM hoists this to the preheader... */
        if ((unsigned)(val - 0x12) < 12u) { continue; }
        sval = (s16)val;
        if (sval == empty_slot) { continue; }
        if (*(s32 *)(p + 0x4E) != 0) { continue; }
        {
            s32 bc = *(s32 *)(arg0 + 0xBC);
            s32 pos2e = *(s32 *)(p + 0x2E);
            if (bc - 0x64 >= pos2e) { continue; }
            if (pos2e >= bc + 0x64) { continue; }
        }
        {
            s32 dx = *(s32 *)(arg0 + 0xF4) - *(s32 *)(p + 0x2A);
            s32 dz = *(s32 *)(arg0 + 0xFC) - *(s32 *)(p + 0x32);
            s32 range_sq = 0xF423F;   /* ...but it hoists THIS one too. Fatal. */
            if (dx * dx + dz * dz > range_sq) { continue; }
        }
        if (func_80030B10(arg0, sval) == 0) { return -1; }
        old_val = (s32)(*(s16 *)p);
        *(s16 *)p = (s16)empty_slot;
        if (old_val == 0xE) {
            s32 a0val = D_800A36F2 ^ 0xE;
            func_80032854(a0val != 0, 0x2F, arg0 + 0xF4, 0);
        } else {
            func_80032854(*(s16 *)(arg0 + 4), 0x11, arg0 + 0xF4, 0);
        }
        return old_val;
    }
    return -1;
}
