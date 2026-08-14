/* func_800174F4 - MATCHING FORM as of grind session 6 (forensics).
 * Honest sandbox distance (`sandbox func_800174F4 --disable all`): **0**
 * with build_insns == target_insns == 136 and rules_dropped == 1, i.e. the
 * sole surviving regfix rule (`func_800174F4: $3 <-> $5 @ 27-41`,
 * regfix.txt:11) is DISABLED in that measurement and the bytes still match -
 * the rule is now redundant and is an operator-side retire.
 *
 * Floor history: s1 25 -> 14, s2 -> 8, s3 8, s4 -> 2, s5 2, s6 -> 0.
 *
 * ==================== WHAT SESSION 6 ADDED ====================
 * The last 2 points were the `jal rand` delay slot: target fills it with
 * `move s0,zero` (the loop counter's `i = 0;`) and leaves a nop in the guard
 * branch's slot; every form before this session had the mirror. Sessions 2-5
 * proved the tension: reorg.c can only fill a CALL_INSN's slot from the
 * BACKWARD search, so `i = 0;` must sit in the basic block that ends with the
 * call (i.e. before `v0 = rand();`) - and that placement takes the counter
 * allocno to live_length 13 / priority 6153, losing $s0 to `h` (9333) in
 * global.c:allocno_compare, which costs 8 points of register naming.
 *
 * Session 6 broke the tension with the ONE surface s5 left open (F7): flow.c
 * weights every reference by loop depth (`reg_n_refs[regno] += loop_depth`,
 * flow.c:2081/2329/2515/2725, loop_depth driven by NOTE_INSN_LOOP_BEG at
 * :1385/:1401/:1447). Our loop is goto-formed, so the front end emits no loop
 * notes and every counter ref weighs 1 (n_refs 4). Two `do { ... } while (0);`
 * wraps - a sanctioned family, see .claude/rules/do-while-zero-exception.md -
 * emit real loop notes and re-weight exactly the counter's own references:
 *   - `do { i = 0; } while (0);`   -> the init ref weighs 2
 *   - the whole loop body wrapped  -> the two `i++` refs weigh 2 each
 * n_refs 4 -> 7 at the unchanged live_length 13 gives priority
 * floor_log2(7)*7/13*10000 = 10769 > `h`'s 9333, so the counter is allocated
 * first and takes $s0(16) while `h` takes $s1(17) - target's assignment - with
 * the init still sitting pre-call where reorg.c needs it.
 *
 * MEASURED, all on this base (see evidence.md for the tables):
 *   wrap `i++` only              n_refs 6, pri 9230 -> LOSES by 1% (score 11;
 *                                the wrap's end label also lands between the
 *                                call and `i++`, which costs the jal slot)
 *   wrap `i = 0;` only           n_refs 5, pri 7692 -> loses (score 8)
 *   both, `i++` wrapped alone    n_refs 7, pri 10769 -> registers WIN, but the
 *                                label costs the loop's delay slot (score 4)
 *   both, WHOLE BODY wrapped     n_refs 7, pri 10769 -> **score 0**
 * The body-spanning wrap is load-bearing for the second reason, not the first:
 * a `do { i++; } while (0);` on its own puts a CODE_LABEL between the call and
 * the increment, and reorg.c's backward search stops at labels, so
 * `addiu s0,s0,1` can no longer reach the func_8005D554 delay slot.
 *
 * ==================== INHERITED, STILL LOAD-BEARING ====================
 * - `mode` (s4) holds the switch selector AND the case-20 limit; that second
 *   live range is what puts regs 3 and 4 into the selector allocno's exclusion
 *   union so global.c:find_reg walks to $a1(5). Sanctioned "variable reuse for
 *   codegen control". It also deletes the old `a1_val` local outright.
 * - The goto-form loop with an explicit entry guard is required (the natural
 *   `for`/`while` forms score 39 at 135 insns - loop rotation emits a
 *   two-instruction entry test where target has a single `beqz $s1`).
 * - `h` is ONE variable deliberately reused for 0xF0 / the loop limit / the
 *   D_800A37A8[] table value; splitting any role costs 3 instructions.
 * - `i++` after the call statement (s1 H3); `if (h == 0) break;` and
 *   `if (h != 0) { ... }` are equivalent spellings.
 *
 * 100% pure C. Zero pins, zero inline asm, zero dead code, zero rules needed.
 * Self-vet: memory/grind/func_800174F4/self_vet.md.
 */
void func_800174F4(void) {
    u8 sp18[8];
    u8 sp20[0x68];
    s32 env;
    unsigned short h;
    s32 prim;
    s32 mask;
    s32 mode;

    prim = (s32)(&D_800F33D8);
    if (g_disp_enable == DISP_DISABLED) {
        return;
    }
    env = (s32)sp20;
    h = 0xF0;
    mask = D_800A36AC & 1;
    mask = -mask;
    SetDefDrawEnv((u8 *)env, 0, mask & 0xF0, 0x280, h);
    sp20[0x18] = 0;
    PutDrawEnv((u8 *)env);
    D_800A374C = sp18;
    ClearOTagR(sp18, 2);
    mode = g_disp_enable;
    switch (mode) {
    case 1:
    case 2:
        prim = (s32)func_8005D46C((u8 *)prim);
        if (g_disp_fade != 0) {
            s32 v0;
            s32 i;
            /* FAKE: do-while(0) wrap, mechanism: flow.c loop-note reference
             * weighting (reg_n_refs += loop_depth) feeding
             * global.c:allocno_compare, lever-exhaustion: memory/grind/
             * func_800174F4/hypotheses.md K5/K10/K11/K12/H-S4-2/H-S5-1/H-S5-2.
             * Effect: seats the loop counter in $s0 and h in $s1 while the
             * counter is initialised before rand(). */
            do { i = 0; } while (0);
            v0 = rand();
            v0 &= 3;
            h = v0 + 4;
            if (h == 0) {
                break;
            }
        inner_loop:
            /* FAKE: do-while(0) wrap, mechanism: flow.c loop-note reference
             * weighting (reg_n_refs += loop_depth) feeding
             * global.c:allocno_compare, lever-exhaustion: as above.
             * Effect: the companion wrap for the counter's in-loop refs; it
             * must span the whole body so no code label lands between the
             * call and `i++` (that placement costs reorg.c the jal delay
             * slot). */
            do {
                prim = (s32)func_8005D554((u8 *)prim, g_disp_enable);
                i++;
            } while (0);
            if (i >= h) {
                break;
            }
            goto inner_loop;
        }
        else if ((rand() & 7) == 0) {
            func_8005D554((u8 *)prim, g_disp_enable);
        }
        break;
    case 10:
        func_8005E54C(D_800A3784, (u8 *)prim, 0);
        break;
    case 20:
        mode = D_800A37A0;
    {
        u8 a2_val = D_800A38F8;
        s32 a0_val = a2_val & 0xFF;
        s32 div_result;
        s32 counter;
        if (((u32)mode) < (u32)a0_val) {
            break;
        }
        div_result = h / (mode + 1);
        counter = D_800A37C0 + 1;
        D_800A37C0 = counter;
        if (div_result >= counter) {
            break;
        }
        if (a0_val == mode) {
            D_800A38F8 = a2_val + 1;
        } else {
            u8 new_val = a2_val + 1;
            D_800A38F8 = new_val;
            D_800A37C0 = 0;
            h = D_800A37A8[a0_val];
            if ((new_val & 0xFF) == mode) {
                h |= 0x8000;
            }
            func_80060414(h, (u8 *)prim, 0);
        }
        break;
    }
    }
    DrawOTag((u8 *)(D_800A374C + 4));
    DrawSync(0);
}
