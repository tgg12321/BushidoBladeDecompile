/* =====================================================================
 * func_80056CB8 -- CANDIDATE, s72 (synthesis modality, 2026-09-16)
 * SANDBOX 0/204 (build_insns 204 == target_insns 204) on the s72 chassis.
 *
 * NOTE: this file is a CANDIDATE, not HEAD state. src/text1b.c on main
 * carries INCLUDE_ASM("asm/funcs", func_80056CB8) per asm-until-matched;
 * this body is applied to src/text1b.c per-session for measurement and was
 * left in place at the end of s72 for the driver's candidate-ready byte
 * verification.
 *
 * PREREQUISITES carried with the body (both required for the 0/204):
 *  (a) func_80053614 in src/text1b.c is declared `s32 func_80053614(...)`
 *      with `return func_80052D00(arg2, arg3);` instead of `void` + bare
 *      call (byte-neutral to that function, 0/32, re-verified s68) -- the
 *      caller consumes $v0 after both calls, so bytes decide the prototype
 *      (SOTN prototype-contradiction norm).
 *  (b) the TU-local scalar externs `extern u8 D_8009A820; / D_8009A821;`
 *      (formerly src/text1b.c ~line 2183, no C uses anywhere) are REMOVED
 *      in favour of the array externs below; the tables are indexed by
 *      i*2 and the scalar declarations would conflict in the same TU.
 *      src/text1b_b.c still carries the same unused scalar externs (out of
 *      this function's scope; a header-canonical cleanup for later).
 *
 * WHAT CLOSED THE 38/204 RESIDUAL (all measured this session, full detail
 * in hypotheses.md/evidence.md [s72]; the residual was several COUPLED
 * diffs, so several of these had been measured "worse" in isolation on
 * earlier chassis):
 *  1. `idx = i * 2; obj = arg0; flags = D_8009A821[idx] << 8;` -- the
 *     FIRST byte-table index named as a fresh once-written once-read local
 *     with one real statement between def and use. loop.c's giv-worth
 *     test (tools/gcc-2.7.2/loop.c:3823, reject iff
 *     lifetime*threshold*benefit < insn_count; threshold 31 with
 *     loop_has_call) rejected the combined i*2 giv pair at 124 vs 163
 *     because both givs had lifetime 1; lifetime 2 on one lifts the pair
 *     to 3*31*2 = 186 >= 163 and it is reduced into the $fp accumulator
 *     (`sll $fp,$v1,2` / `addu $at,$at,$fp` x2 / `addiu $fp,$fp,2`),
 *     exactly the target. Dump: tmp/grind/func_80056CB8/s72/life2.loopgiv
 *     ("giv at 131 reduced to (reg:SI 220)"). Naming BOTH lookups' index
 *     makes ONE giv with benefit 2 - add_cost 2 = 0, never reduced -- that
 *     is why every shared-index spelling in rejected/ was worse.
 *     FAKE-annotated: named-intermediate family
 *     (.claude/rules/no-new-park-categories.md:214 clarification).
 *  2. `for (i = start; i < start + 2; i++)` (no `limit` local): reproduces
 *     the target's `li $v0,1; beqz` entry guard AND the per-iteration
 *     `lw start; addiu $v0,$t3,2; slt` loop-end test. (s46 measured this
 *     form WORSE on the pre-$fp chassis; with step 1 it is the missing
 *     piece -- a chassis-relative kill, superseded.)
 *  3. `s32 work[2]` (was [4]): the target's first spill slot is at sp+0x60,
 *     overlapping where work[2..3] would sit -- the original work buffer is
 *     8 bytes. Frame 0xA8 and every spill/save offset now match.
 *  4. `flags |= func_80053614(...) << 1; flags += 1;` (split-init
 *     accumulation, ordinary C) -> `or $s0,$s0,$v0; addiu $s0,$s0,1`;
 *     `*(s8 *)(arg0 + i + 0x444)` -> `addu $v0,$s7,$s6` operand order.
 *  5. `if (flags == 3 && hit1[1] - y < 5) flags = 0; else if (flags == 4)`
 *     (was a nested if inside `== 3`): the target falls from the failed
 *     `< 5` test into the `== 4` compare and jumps over it
 *     (`j .L80056F94`) only after `flags = 0`.
 *  6. `if ((y - hit1[1] >= 0 ? y - hit1[1] : hit1[1] - y) >= 0x3E9)`:
 *     fold-const distributes the compare into the ?: arms, giving the
 *     target's `bltz / beqz -> L5 / j end` then-arm and `bnez -> end`
 *     else-arm (opposite branch senses, so cross-jump merges only the
 *     shared `li $s0,5`).
 *  7. `D_800F6608.w8` instead of a split-scalar `extern s32 D_800F6610;`
 *     (byte-identical, honest data model; text1b.c:1889 uses the same).
 * --------------------------------------------------------------------- */

extern s16 Judge;
extern s32 ratan2(s32, s32);
extern u8 D_8009A820[];
extern u8 D_8009A821[];

void func_80056CB8(s32 arg0) {
    s32 pt0[4];
    s32 pt1[4];
    s32 hit0[4];
    s32 hit1[4];
    s32 work[2];
    s32 start;
    s32 i;

    start = (*(u16 *)(arg0 + 0x3E8) & 3) * 2;
    for (i = start; i < start + 2; i++) {
        s32 obj;
        s32 flags;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;
        s32 idx;

        /* FAKE: idx names the byte-table index for the first lookup only, mechanism:
           loop.c strength_reduce giv-worth test (lifetime * threshold * benefit >= insn_count),
           lever-exhaustion: memory/grind/func_80056CB8/hypotheses.md [s72] + rejected/ */
        idx = i * 2;
        obj = arg0;
        flags = D_8009A821[idx] << 8;
        if ((flags & 0x1000) != 0) {
            obj = *(s32 *)arg0;
        }

        if (*(u16 *)(arg0 + 0x6A) == 0x13 || *(u16 *)(arg0 + 0x6A) == 6) {
            flags += *(s16 *)(obj + 0x1CA);
        } else {
            flags += ratan2(D_800F6608.w0 - *(s32 *)(obj + 0xF4),
                             D_800F6608.w8 - *(s32 *)(obj + 0xFC));
        }

        sin_p = &Judge + (flags & 0xFFF);
        scale = D_8009A820[i * 2] << 8;
        x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
        cos_p = &Judge + ((flags + 0x400) & 0xFFF);
        z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
        pt0[0] = *(s32 *)(obj + 0xB8);
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt0[2] = *(s32 *)(obj + 0xC0);
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt1[2] = z;

        flags = func_80053614(pt0, pt1, (s32)hit0, (s32)work, 0x1F8002B8);
        if (flags != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        pt0[0] = x;
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x834;
        pt0[2] = z;
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) + 0x1004;
        pt1[2] = z;

        flags |= func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8) << 1;
        flags += 1;
        if (flags == 3 && hit1[1] - *(s32 *)(obj + 0xBC) < 5) {
            flags = 0;
        } else if (flags == 4) {
            s32 dx = hit0[0] - *(s32 *)(obj + 0xB8);
            s32 dz = hit0[2] - *(s32 *)(obj + 0xC0);
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = *(s32 *)(obj + 0xBC);
                if ((y - hit1[1] >= 0 ? y - hit1[1] : hit1[1] - y) >= 0x3E9) {
                    flags = 5;
                }
            }
        }
        *(s8 *)(arg0 + i + 0x444) = (s8)flags;
    }
}
