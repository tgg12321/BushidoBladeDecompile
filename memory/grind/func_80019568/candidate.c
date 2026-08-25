/* candidate - func_80019568 - s3 (2026-08-25)   ***  sandbox --disable all == 0  ***
 *
 * build_insns 141 == target_insns 141.  Floor history: s1 34 -> s2 20 -> s3 0.
 *
 * Structure (all mechanism-grounded in tools/gcc-2.7.2/loop.c, read not guessed):
 *  L1  per-iteration record pointers `u8 *rec = &pk[i*8]; s16 *o = &sp.output[i];`
 *      at the top of the loop body.  Collapses each scale group to ONE "add 0"
 *      DEST_REG giv leader; a lone leader has benefit 2 and loop.c:3804 subtracts
 *      add_cost(2)*biv_count(1) -> 0 -> loop.c:3824 "not worth while" -> ignore=1 ->
 *      all_reduced=0 -> the loop.c:4034 gate skips biv elimination, so counter `i`
 *      survives and the exit test stays `slti v0,t0,2`.  ORDINARY C.  (s2)
 *  L2  `enable` flag local, 1 in the valid arm / 0 in the invalid arm, stored inside
 *      each arm.  Two NON-consecutive sets => no scan_loop movable for the `li 1`
 *      (loop.c:702-716), so it stays in the loop filling the lhu load-delay slot,
 *      AND the v0/v1 seats in that block snap to target.  20 -> 12.  (s3, FAKE)
 *  L3  `s32 *p = &D_80102790;` read-modify-write in the tail: one `la` shared by the
 *      load and the store, matching target's `lui/addiu; lw 0(v0); sw 0(v0)`.
 *      12 -> 0.  (s3, FAKE; .claude/rules/pointer-rmw-global-sanctioned.md shape)
 *
 * Self-vet: memory/grind/func_80019568/self_vet.md
 * NOTE FOR INTEGRATION: func_80019568 still carries 5 regfix rules calibrated to the
 * old rule-era body; they must be retired (operator `retire`) for the full build.
 */
void func_80019568(s32 arg0) {
    struct {
        s16 output[4];
        s32 voice_mask;
        s32 unk_1C;
        s32 unk_20;
        s32 unk_24;
        s32 packets[4];
    } sp;
    u8 *pk;
    s32 i;
    s32 voice_mask;
    s32 old_mask;
    s32 *p;
    s16 *base_addr;
    s16 *dst1;
    s16 *dst0;
    s16 *src;

    voice_mask = 0;
    i = 0;
    pk = (u8 *)&sp.packets[0];
    sp.packets[0] = D_800FF580;
    sp.packets[1] = D_800FF584;
    sp.packets[2] = D_800FF5A4;
    sp.packets[3] = D_800FF5A8;
    do {
        u8 *rec = &pk[i * 8];
        s16 *o = &sp.output[i];
        s32 bits;
        /* FAKE: constant-holder flag local for output[i+2]; two non-consecutive sets
         * (1 in the valid arm, 0 in the invalid arm) stop scan_loop from creating a
         * movable for the `li 1`.  mechanism: loop.c:702-716 (movable requires
         * n_times_set==1 or consec_sets_invariant_p); the literal 4, which IS written
         * as a bare literal in two arms, is hoisted into $t2 in target, so target's
         * un-hoisted 1 proves the original held it in a variable.
         * lever-exhaustion: memory/grind/func_80019568/hypotheses.md H6/H10/H12 -
         * bare `o[2] = 1;` measures 8 (build 142, li hoisted); the variable-reuse
         * spelling measures 8 (build 141); this form measures 0. */
        s32 enable;

        if (rec[0] == 0) {
            s32 voice2;

            o[0] = rec[1] >> 4;
            enable = 1;
            o[2] = enable;
            voice2 = (s16)((u16)o[0] - 1);

            if ((u32)voice2 < 8) {
                switch (voice2) {
                case 4:
                case 6:
                    o[0] = 4;
                case 1:
                case 2:
                case 3:
                    bits = ~((rec[2] << 8) | rec[3]);
                    break;
                case 0:
                case 5:
                case 7:
                default:
                    bits = 0;
                    break;
                }
            } else {
                bits = 0;
            }
        } else {
            o[0] = 4;
            enable = 0;
            o[2] = enable;
            bits = 0;
        }

        voice_mask = ((u32)voice_mask >> 16) | (bits << 16);
        i++;
    } while (i < 2);

    sp.voice_mask = voice_mask;
    func_8001B138(&sp.voice_mask);

    if (D_800A3834 == 1 && arg0 == 0) {
        s32 voice_state = D_800A38DC;

        if ((u32)voice_state < 7) {
            switch (voice_state) {
            case 4:
            case 5:
                if (D_8010278E == 0) {
                    sp.voice_mask |= 0x08000800;
                }
            case 0:
            case 1:
            case 2:
            case 3:
            case 6:
                if (D_8010278C == 0) {
                    sp.voice_mask |= 0x08000800;
                }
                break;
            }
        }
    }

    func_8003A728((s32)&sp.output[0]);

    i = 0;
    base_addr = &D_80102788;
    dst1 = base_addr + 2;
    dst0 = base_addr;
    src = &sp.output[0];

    do {
        dst0[0] = src[0];
        dst0++;
        dst1[0] = src[2];
        src++;
        i++;
        dst1++;
    } while (i < 2);

    /* FAKE: read-modify-write handle for D_80102790 - one `la` address kept in a
     * register for both the load and the store instead of two independent
     * %hi/%lo symbol MEMs.  mechanism: address materialization / CSE shape at
     * expand time (a MEM whose address is a symbol_ref never gets its address
     * cse'd into a register on MIPS).  lever-exhaustion:
     * memory/grind/func_80019568/hypotheses.md H8 (aggregate KILLED), H9
     * (statement reordering INERT), H11 (direct multi-read of the global
     * measures 11). */
    p = &D_80102790;
    old_mask = *p;
    *p = sp.voice_mask;
    D_80102794 = sp.voice_mask & ~old_mask;
    D_8010279C = ~sp.voice_mask;
    D_80102798 = ~sp.voice_mask & old_mask;
}
