/* candidate.c -- func_80041188 (src/text1a_pre.c) -- s37 (forensics, 2026-08-27).
 *
 * SANDBOX `func_80041188 --disable all` = **0** at 132 build / 132 target insns,
 * rules_dropped 16, cheat_asm_stripped 0. This is a BYTE MATCH of the honest,
 * cheat-invisible build. It supersedes the previous candidate (goto chassis, floor 1,
 * owner-sanctioned split increment), which is banked under
 * rejected/prior-candidate-goto-chassis-floor1-superseded-by-s37-match.c.
 *
 * CHASSIS. This is s36's form P1r verbatim -- the REAL-LOOP chassis (loop1 a
 * note-marked `do { ... } while (i < 0x12);`, loop2 a note-free `goto` loop),
 * carrying target's honest block-2 `out3 = (s32 *)(((u8 *)a4) + 0x20);` and with the
 * `s32 *pa4 = a4;` param alias DELETED so the fifth parameter is used directly.
 * P1r measures sandbox 2 with ALL-TARGET callee-saved seats and ZERO constructs
 * (s36, evidence E-s36-2). Its allocation, unchanged here:
 *     out2 5 refs/41 live/pri 2439 -> $s6      a4 9/188/1436 -> $s7
 *     a3   5/99/1010  -> $fp                   out3 3/47/638 -> $s3
 *     tbl  7/47/2978  -> $s5                   i 11/97/3402 -> $s4
 *     stptr2 6/48/2500 -> $s0                  loop.c giv 9/40/6750 -> $s3
 *
 * WHAT S37 ADDED (the last two instructions). P1r's entire residual was
 * `addiu $t0,$zero,2` / `sh $t0,0x6($s3)` against target's `addiu $v0,$zero,0x2` /
 * `sh $v0,0x6($s3)` (asm/funcs/func_80041188.s:65-66) -- same position, same order,
 * one register NAME. s36 proved the cause end-to-end from the dumps: the `sh` of an
 * immediate expands to a fresh HImode temp; `loop.c` `move_movables` HOISTS that temp
 * out of note-marked loop1 (red.i.loop: "Insn 152: regno 121 (life 1), move-insn
 * savings 1  moved to 312"); hoisted it is live 92 insns across 7 calls with every
 * callee-saved seat already held by a target-hosted pseudo, so `global_alloc` leaves
 * it unallocated and reload REMATERIALISES the REG_EQUIV constant at the use -- right
 * position, but into `$t0`, the first legal reload register (MIPS defines no
 * REG_ALLOC_ORDER). Target's `$v0` is what LOCAL-ALLOC gives an UNHOISTED,
 * block-local, call-free temp -- exactly what the note-free loop2 already produces
 * and matches byte for byte (`sh $v0,0x6($s0)`, asm/funcs/func_80041188.s:119).
 *
 * s36 closed four dimensions of that hoist (threshold `61 >= 48` for every spelling;
 * five respellings A/C/E/F/G all identical at 2; gates (2)+(3) via form J = 37;
 * gate (1) `maybe_never` provably 0). It did NOT test the FIFTH: `scan_loop` only
 * builds a movable when `n_times_set` of the destination pseudo is 1
 * (loop.c `count_loop_regs_set`). A SECOND, non-consecutive set of the same pseudo
 * inside loop1 makes `n_times_set == 2`, so no movable is ever created and the
 * constant stays in loop1, where local-alloc seats it in `$v0` -- target's register.
 * The second set is DEAD and `flow.c` `propagate_block` deletes it before any bytes
 * are emitted, so the instruction count is unchanged at 132.
 *
 * MEASURED PROOF OF THE MECHANISM (tmp/grind/func_80041188/s37/K/):
 *   - red.i.loop: "Loop from 39 to 173: 49 real insns." and ZERO "moved to" lines
 *     (P1r's dump has exactly one, for this pseudo).
 *   - red.i.loop / red.i.cse2: `(insn 139 (set (reg/v:HI 90) (const_int 2)))`,
 *     `(insn 155 (set (mem:HI ...+6) (reg/v:HI 90)))`, `(insn 158 (set (reg/v:HI 90)
 *     (const_int 3)))` -- both sets alive, so `n_times_set == 2`.
 *   - red.i.flow: pseudo 90 has only TWO occurrences left (the live set + the store,
 *     carrying REG_DEAD); flow.c deleted insn 158.
 *
 * SPELLING BOUNDARY (all measured this session, banked in rejected/):
 *   - `two = two;` self-assign after the store  -> 2 (the expander elides the
 *     self-move, so there is never a second set).
 *   - `two = 2;` same-value re-store after the store -> 2 (cse1 runs BEFORE loop and
 *     deletes the redundant set, so loop.c still sees n_times_set == 1).
 *   - `two = (s16)(i + 1);` -> 77 at 135 insns (a dead store whose SOURCE is not a
 *     constant materialises real instructions; flow cannot delete the computation).
 *   - `two = (s16) i;` -> 0 at 132 (works, equivalent to the chosen form).
 *   The dead store must therefore be a plain constant DIFFERENT from the live one.
 *
 * CONSTRUCTS (both FAKE-annotated in the body, both in frozen sanctioned families):
 *   1. `s16 two;` + `two = 2;` -- constant-holder local
 *      (.claude/rules/named-local-fake-exception.md; byte-neutral on its own -- s36
 *      form C measures the same 2 as the bare literal).
 *   2. `two = 3;` -- dead store to a local, deleted by flow.c
 *      (.claude/rules/dead-store-fake-exception.md).
 * Self-vet: memory/grind/func_80041188/self_vet.md.
 *
 * INTEGRATION NOTE (surfaces this session may not touch): the function still carries
 * 16 regfix/asmfix rules plus a prologue_config entry from the rule era. The honest
 * sandbox is 0 with all of them DISABLED; retiring them (`engine retire func_80041188`)
 * is an operator step.
 */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 ents;
    s32 *out2;
    s32 *out3;
    s32 offset;
    u16 *p;
    s32 stptr2;
    s16 two;   /* FAKE: constant-holder carrying loop1's record-flag value; mechanism: gives loop.c's scan_loop a user pseudo it can count sets on. lever-exhaustion: memory/grind/func_80041188/hypotheses.md s36 (five literal/holder spellings A,C,E,F,G all measure 2) */
    ents = base + 0x94;
    out2 = (s32 *) (((u8 *) a4) + 0x20);
    do {
        offset = (*tbl) * 6;
        p = (u16 *) (offset + (s32) a1);
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        func_8004A348(buf, a4);
        tbl++;
        offset = offset + (s32) a2;
        p = (u16 *) offset;
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        func_8004A348(buf, out2);
        func_800523E0(a4, out2, a3, ents + i * 0x68 + 0x38);
        two = 2;
        *((s16 *) (ents + i * 0x68 + 6)) = two;
        two = 3; /* FAKE: dead store, never read; mechanism: loop.c count_loop_regs_set sees n_times_set == 2 so scan_loop builds no movable and move_movables cannot hoist the constant out of loop1 (flow.c propagate_block then deletes this store, zero emitted bytes). lever-exhaustion: memory/grind/func_80041188/hypotheses.md s32-s36 */
        i++;
    } while (i < 0x12);
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = ents + 0x750;
    out3 = (s32 *) (((u8 *) a4) + 0x20);
    loop2:
    func_80044DE4((s16 *) a1, (s16 *) a2, a3, stptr2 + 0x4C);
    a1 += 6;
    a2 += 6;
    buf[0] = *((u16 *) a1);
    a1 += 2;
    buf[1] = -(*((u16 *) a1));
    a1 += 2;
    buf[2] = -(*((u16 *) a1));
    a1 += 2;
    func_8004A348(buf, a4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    func_8004A348(buf, out3);
    func_800523E0(a4, out3, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
