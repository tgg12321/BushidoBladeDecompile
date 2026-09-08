/* func_8002D780 - grind candidate (s12 structural, 2026-09-08).  Honest sandbox floor
 * 2/202, build_insns == target_insns == 202, measured THIS session with these exact
 * edits in src/code6cac_b.c (`sandbox func_8002D780 --disable all` -> 2).
 *
 * WHY THIS BODY REPLACES THE s10/s11 ONE AT THE SAME SCORE.  The score is unchanged but
 * the RESIDUAL IS STRICTLY MORE TRACTABLE.  The previous body (kept verbatim as
 * memory/grind/func_8002D780/candidate_alt_s11_dzfirst.c) failed on the ax/dz pair that
 * straddles the DELAY SLOT of `bltz $v0,.L8002D964` -- a reorg.c decision that s11 proved
 * is the same INSN_LUID as the dz/dx register seat and pulls the opposite way.  This body
 * has the delay slot RIGHT, every register in the function RIGHT, and its only two
 * differing instructions are an ordinary adjacent transposition INSIDE block 7:
 *
 *   ours[96] subu v0,a2,a3 (az)      target[96] subu v1,t5,t1 (dx)
 *   ours[97] subu v1,t5,t1 (dx)      target[97] subu v0,a2,a3 (az)
 *   === 2 differing instructions ===   (tmp/grind/func_8002D780/s12/pairdiff_d1.txt)
 *
 * No delay slot is involved any more, so the s11 LUID contradiction is GONE: the last
 * question is a pure sched2 ready-list order between two independent subus that feed the
 * same mult.
 *
 * HOW THE THREE DECLARATIONS BUY THAT (measured, instrumented-cc1 QTYDBG tables in
 * tmp/grind/func_8002D780/s12/{a1,b1,c1}/stderr_full.txt).  local-alloc.c:1725-1758 ranks
 * block-7 quantities by floor_log2(refs)*refs*size/(death-birth)*10000 and breaks a tie by
 * quantity number (local-alloc.c:1684); whichever of dz/dx is seated FIRST takes $v1 and
 * the other takes $a0, and the target needs dz=$a0 / dx=$v1, i.e. dx seated first.
 *   `s32 ax = cx - x0;`  emits ax's subu FIRST, so reorg's fill_slots_from_thread takes IT
 *                        for the test-2 delay slot instead of dz's subu (target's slot).
 *                        On its own (variant a1) that costs the seats: dz and dx both land
 *                        at refs 3 / span 12 and tie, and the tie seats dz first (score 9,
 *                        rejected/axfirst-slot-right-seats-swapped-9.c).
 *   `s32 dz = z2 - z0;`  keeps the mult operand order `mult $a0,$v0` (dz * ax).
 *   `s32 az = cz - z0;`  is the tie-breaker.  It puts ONE extra RTL insn between dz's
 *                        definition and dx's definition, which lengthens dz's live range to
 *                        14 while leaving dx's at 12 (both endpoints shift together).  dx
 *                        then outranks dz (2500 > 2142), is seated first, takes $v1, and dz
 *                        takes $a0 -- the target's seats.  az is a REAL value consumed by
 *                        the second product; it costs no instruction because the subu it
 *                        names is one the target emits anyway.
 * Hoisting bz or bx instead does NOT work: sched1 sinks them back down to their (later)
 * consumers, so no span shift happens and the score stays 9
 * (rejected/bz-hoist-sinks-back-no-span-shift-9.c, .../bx-...-9.c).
 *
 * WHAT IS LEFT.  sched.c rank_for_schedule compares INSN_PRIORITY, then dependence class,
 * then INSN_LUID (sched.c:2464).  dx and az are independent, both feed mult2, so their
 * longest-path priorities are equal and the LUID decides -- and az's LUID is lower because
 * the declaration puts it earlier.  Any fix must either raise dx above az on the first two
 * comparisons or find a different zero-cost insn to sit between dz and dx.  tools/sched_solver
 * models both scheduler passes order-exactly and is the right instrument for that question.
 *
 * The rest of the body is unchanged from s5/s10 and every line of it is load-bearing; see
 * candidate_alt_s5_named_locals.c for the LZCS/LZCR island split, the `m` carrier and the
 * clobber footprint.  Summary of what must not change:
 *  - the two cop2 islands in the sqrt block must stay SPLIT;
 *  - `s32 m = dist;` must be declared BEFORE `s32 lzcr` and re-stored inside the
 *    `dist >= 0` arm (the annotated FAKE construct below; worth 4 insns, s11 ablation);
 *  - the three cop2 blocks are the owner-authorized canonical LZCS/LZCR + mvmva idiom
 *    (.claude/rules/cop2-addressing-preamble-cluster.md).
 *
 * FAMILY NOTE for whoever reaches 0: this body carries one annotated construct, the
 * same-value re-store of the local `m` (dead-store family,
 * .claude/rules/dead-store-fake-exception.md, in-TU byte-matched precedent at
 * src/code6cac_b.c:1244-1265).  It ALSO reintroduces three named difference locals in test
 * 3 (`ax`, `dz`, `az`).  `dz` is written once and read twice (an ordinary CSE variable);
 * `ax` and `az` are written once and read once, which is the named-intermediate shape
 * described in .claude/rules/narrow-byte-args-packed-call.md plus the 2026-08-17
 * clarification in .claude/rules/no-new-park-categories.md -- all three hold REAL values
 * that appear in the target bytes, but a candidate-ready session must decide whether the
 * once-read pair needs the named-intermediate FAKE annotation (six prongs) or is ordinary
 * C, and should file a ruling-request if the answer is not clean. */
s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        s32 *vin;
        s32 *vout;
        *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
        *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
        *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
        vin = (s32 *)(obj + 0xF8);
        __asm__ volatile(
            "addu $t4, %0, $zero\n"
            "lwc2 $0, 0($t4)\n"
            "lwc2 $1, 4($t4)\n"
            "nop\n"
            "nop\n"
            ".word 0x4A486012"
            : : "r"(vin) : "$12", "memory");
        vout = (s32 *)(obj + 0x100);
        __asm__ volatile(
            "addu $t4, %0, $zero\n"
            "swc2 $25, 0($t4)\n"
            "swc2 $26, 4($t4)\n"
            "swc2 $27, 8($t4)"
            : : "r"(vout) : "$12", "memory");
    }

    {
        s32 y = *(s32 *)(obj + 0x108);
        if (y < -threshold || threshold < y) return 0;
    }

    {
        s32 x0 = *(s32 *)(obj + 0xA8);
        s32 x2 = *(s32 *)(obj + 0xB8);
        s32 z0 = *(s32 *)(obj + 0xAC);
        s32 z2 = *(s32 *)(obj + 0xBC);
        s32 cx = (x0 + x2) / 3;
        s32 cz = (z0 + z2) / 3;
        s32 px = *(s32 *)(obj + 0x100);
        s32 pz = *(s32 *)(obj + 0x104);
        s32 kc = z0 * cx - x0 * cz;
        s32 kp = z0 * px - x0 * pz;

        if ((kc ^ kp) >= 0) {
            kc = z2 * cx - x2 * cz;
            kp = z2 * px - x2 * pz;
            if ((kc ^ kp) >= 0) {
                s32 ax = cx - x0;
                s32 dz = z2 - z0;
                s32 az = cz - z0;
                kc = ax * dz - (x2 - x0) * az;
                kp = dz * (px - x0) - (x2 - x0) * (pz - z0);
                if ((kc ^ kp) >= 0)
                    return 1;
            }
        }
    }

    {
        s32 y = *(s32 *)(obj + 0x108);
        s32 sp_var;
        s32 dist = r_sq - y * y;
        s32 sqrt_val;
        s32 *p118;
        s32 *p124;
        s32 *p10C;

        if ((u32)dist < 0x400) {
            sqrt_val = (u32)*((&D_8008D118) + dist) >> 3;
        } else {
            s32 m = dist;
            s32 lzcr = 0;
            if (dist >= 0) {
                /* FAKE: same-value re-store of the local `m`, mechanism: cse.c make_regs_eqv
                 * (a single-definition copy is folded; a second definition keeps the pseudo
                 * multiply-defined so the copy survives into local-alloc), lever-exhaustion:
                 * memory/grind/func_8002D780/hypotheses.md s1-s5, 14 copy spellings dead. */
                m = dist;
                __asm__ volatile(
                    "addu $t4, %0, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop"
                    : : "r"(m) : "$12");
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(&sp_var) : "$12", "$13", "$14", "$15");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *((&D_8008D118) + ((u32)m >> shift));
                sqrt_val = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        p118 = (s32 *)(obj + 0x118);
        p124 = (s32 *)(obj + 0x124);
        p118[0] = *(s32 *)(obj + 0xA8) - *(s32 *)(obj + 0x100);
        p118[1] = *(s32 *)(obj + 0xAC) - *(s32 *)(obj + 0x104);
        p124[0] = *(s32 *)(obj + 0xB8) - *(s32 *)(obj + 0x100);
        p124[1] = *(s32 *)(obj + 0xBC) - *(s32 *)(obj + 0x104);
        if (func_8002D518(sqrt_val, dist, p118, p124) != 0) return 1;

        p10C = (s32 *)(obj + 0x10C);
        p10C[0] = -*(s32 *)(obj + 0x100);
        p10C[1] = -*(s32 *)(obj + 0x104);
        if (func_8002D518(sqrt_val, dist, p10C, p118) != 0) return 1;

        if (func_8002D518(sqrt_val, dist, p10C, p124) != 0) return 1;
        return 0;
    }
}
