/* candidate - func_80019568 - s4 (session 3 of the grind, permuter modality, 2026-08-25)
 *  *** sandbox --disable all == 0, build_insns 141 == target_insns 141, re-measured
 *      THIS session on today's chassis with the body in place in src/code6cac.c ***
 *  *** SUBMITTED as candidate-ready.  The open ruling-request that blocked s3b was
 *      ANSWERED PASS on 2026-08-25 13:45 (docs/grind/decisions.md:11114): the
 *      default-initialised `s32 enable = 0;` flag is ordinary C needing no exception,
 *      and the per-arm write-out is duplicated-statement-into-arms (NOT
 *      named-local-fake-exception, which was the 12:52 layer-1 FAIL's mis-citation).
 *      The banned shape - uninitialised `s32 enable;` whose every in-arm assignment is
 *      a dead pass-through - is NOT this form and is not present. ***
 *
 * Floor history: s1 34 -> s2 20 -> s3 0 (layer-1 FAIL on citation) -> s3 (this session) 0.
 *
 * Structure (all mechanism-grounded in tools/gcc-2.7.2/loop.c, read not guessed):
 *  L1  per-iteration record pointers `u8 *rec = &packets[i*8]; s16 *o = &sp.output[i];`
 *      at the top of the loop body.  Collapses each scale group to ONE "add 0"
 *      DEST_REG giv leader; a lone leader has benefit 2 and loop.c:3804 subtracts
 *      add_cost(2)*biv_count(1) -> 0 -> loop.c:3824 "not worth while" -> ignore=1 ->
 *      all_reduced=0 -> the loop.c:4034 gate skips biv elimination, so counter `i`
 *      survives and the exit test stays `slti v0,t0,2`.  ORDINARY C.  (s2, H5)
 *      Layer-1 PASSED this construct on 2026-08-25 12:52.
 *  L2  `s32 enable = 0;` - the slot's enable word, defaulted at the top of the loop
 *      body, raised to 1 in the valid arm, written out in each arm.  RULED ORDINARY C
 *      for the flag itself; the per-arm write-out is filed under
 *      .claude/rules/duplicated-statement-into-arms.md with its FAKE annotation.
 *      Mechanism for why the alternatives lose: what keeps the `addiu $v0,$zero,1`
 *      inside the loop is the `n_times_set == 1 || consec_sets_invariant_p` test at
 *      loop.c:706-709 - the loop-top default and the in-arm override are two
 *      NON-CONSECUTIVE sets, so scan_loop never builds a movable.  Measured:
 *      single-set arm-scoped named local 8/142 (rejected/armscope-single-set-named-local-8.c),
 *      address-typed arm-scoped local 8/142 (rejected/blockscope-enable-pointer-8.c),
 *      single store after the join 21/136 (rejected/flag-store-after-join-21.c).
 *  L3  `s32 *p = &D_80102790;` read-modify-write in the tail: one `la` address kept
 *      in a register for the load and the store, matching target's
 *      `lui/addiu; lw 0(v0); sw 0(v0)`.  (s3, H12; pointer-rmw-global-sanctioned.md)
 *      Layer-1 PASSED this construct on 2026-08-25 12:52.
 *
 * NOTE FOR INTEGRATION: func_80019568 still carries 5 regfix rules calibrated to the
 * old rule-era body; they must be retired (operator/driver `retire`) for the full build.
 * Self-vet: memory/grind/func_80019568/self_vet.md (rewritten this session with the
 * corrected family citation).
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
    u8 *packets;
    s16 *output;
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
    packets = (u8 *)&sp.packets[0];
    sp.packets[0] = D_800FF580;
    sp.packets[1] = D_800FF584;
    sp.packets[2] = D_800FF5A4;
    sp.packets[3] = D_800FF5A8;
    do {
        u8 *rec = &packets[i * 8];
        s16 *o = &sp.output[i];
        s32 enable = 0;
        s32 bits;

        if (rec[0] == 0) {
            s32 voice2;

            o[0] = rec[1] >> 4;
            enable = 1;
            /* FAKE: the `o[2] = enable;` store is written into BOTH arms rather
             * than once after the join (family: duplicated-statement-into-arms,
             * .claude/rules/duplicated-statement-into-arms.md; owner ruling
             * 2026-08-25 13:45, docs/grind/decisions.md).  mechanism: loop.c scan_loop
             * (loop.c:695-716) only creates a movable for the `1`-holding
             * pseudo when it has a single set or consecutive sets; the
             * loop-top default plus this in-arm set are non-consecutive, so no
             * movable exists and the `addiu $v0,$zero,1` stays in the loop
             * filling target's lhu load-delay slot.
             * lever-exhaustion: memory/grind/func_80019568/hypotheses.md
             * H6/H10/H12 + s3 H14-H17 (bare literal 8/142, `bits` carrier
             * reuse 6/141, computed `enable = (rec[0] == 0)` 10/142,
             * single store after the join 21/136). */
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

    p = &D_80102790;
    old_mask = *p;
    *p = sp.voice_mask;
    D_80102794 = sp.voice_mask & ~old_mask;
    D_8010279C = ~sp.voice_mask;
    D_80102798 = ~sp.voice_mask & old_mask;
}
