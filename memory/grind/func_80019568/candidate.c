/* candidate - func_80019568 - s2 (2026-08-25)
 *
 * BEST STRUCTURALLY-CORRECT FORM: sandbox --disable all = 20, build_insns 141 == target 141.
 * (s1 best: 34 honest / 28 with a diagnostic non-submittable probe.)
 *
 * Two structural levers found this session, both mechanism-grounded in tools/gcc-2.7.2/loop.c:
 *
 *  L1 - per-iteration record pointers "u8 *p = &packets[i*8]; s16 *o = &sp.output[i];"
 *       declared at the TOP of the loop body.  This collapses the two i*8 and the three
 *       i*2 DEST_REG "add 0" givs into ONE each; a lone giv leader has benefit 2 and
 *       loop.c:3804 subtracts add_cost(=2)*biv_count(=1) -> 0 -> "giv of insn NN not worth
 *       while, 0 vs 51" -> v->ignore=1 -> all_reduced=0 -> the "all_reduced == 1" gate at
 *       loop.c:4034 skips biv elimination entirely, so counter i SURVIVES and the exit
 *       test stays "slti v0,t0,2" (target).  The ignored intermediates are dead once the
 *       add-reg givs reduce to the a2/a3 walkers, so they cost nothing.
 *       Register file now matches target exactly: i->t0, mask->t1, const4->t2, jtbl->t3,
 *       a2 = output walker (offsets 0/4), a3 = packet walker (offsets 0..3).
 *
 *  L2 - "voice" reused for both the shifted voice id and the constant 1
 *       (voice = p[1] >> 4; o[0] = voice; voice = 1; o[2] = voice;).  Two NON-consecutive
 *       sets of the same pseudo inside the loop make n_times_set==2 and
 *       consec_sets_invariant_p fail, so scan_loop never creates a movable for the 1
 *       (loop.c:703-708).  Target keeps "addiu v0,zero,1" inside the loop filling the lhu
 *       load-delay slot; without L2 it is hoisted ("Insn 74: regno 101 (life 1),
 *       move-insn savings 1 moved").
 *       NOTE FOR SUBMISSION: L2 is the variable-reuse family
 *       (.claude/rules/defeat-licm-hoist-var-reuse.md + staged-value-reused-variable.md)
 *       and would need a FAKE annotation + lever-exhaustion evidence, OR an honest
 *       replacement, before any candidate-ready.  L1 is ordinary C and needs neither.
 *
 * REMAINING RESIDUAL (20):
 *   R1 (~6)  v0<->v1 swap in the if-arm: target ties "voice" to the lbu temp
 *            (srl v0,v0,4; lhu v1,0(a2)); ours gets srl v1,v0,4 / lhu v0,0(a2).
 *            Pure local-alloc seat assignment; declaration-order swaps measured inert.
 *   R2 (~13) tail block: target materializes &D_80102790 (lui+addiu) and uses 0(v0) for
 *            BOTH the load and the store, storing early; ours emits two independent
 *            %hi/%lo accesses.  Statement reordering measured inert.  Sibling
 *            func_800194F4 proves the four words are plain scalars, so the "la" comes from
 *            an address-in-a-register producer (pointer-alias family territory).
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
        u8 *p = &packets[i * 8];
        s16 *o = &sp.output[i];
        s32 bits;

        if (p[0] == 0) {
            s32 voice2;
            s32 voice;

            voice = p[1] >> 4;
            o[0] = voice;
            voice = 1;
            o[2] = voice;
            voice2 = (s16)((u16)o[0] - 1);

            if ((u32)voice2 < 8) {
                switch (voice2) {
                case 4:
                case 6:
                    o[0] = 4;
                case 1:
                case 2:
                case 3:
                    bits = ~((p[2] << 8) | p[3]);
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
            o[2] = 0;
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

    old_mask = D_80102790;
    D_80102790 = sp.voice_mask;
    D_80102794 = sp.voice_mask & ~old_mask;
    D_8010279C = ~sp.voice_mask;
    D_80102798 = ~sp.voice_mask & old_mask;
}
