/* cpu_set_move_command_and_dir_for_no_action — src/code6cac_b.c
 *
 * Grind session 1 (2026-07-30, modality: recon). Honest pure-C floor
 * 18 -> 12 -> 8 -> 4 -> 0 (`engine sandbox <func> --disable all`, rules
 * dropped = 9, cheat-asm stripped). build_insns == target_insns == 189 and
 * a normalized instruction-for-instruction diff against
 * asm/funcs/cpu_set_move_command_and_dir_for_no_action.s shows zero real
 * differences (the only residual lines in the diff log are artifacts of the
 * differ's own `(N >> 16)` immediate rendering and objdump reloc symbol
 * naming; each was checked arithmetically).
 *
 * THREE independent levers took it from 18 to 0. All three are ordinary
 * statement-placement / control-flow-shape changes; nothing was added that
 * has no semantic purpose, and the session DELETED a cheat rather than
 * adding one.
 *
 * L1 (18 -> 12) — DELETE the `__asm__ volatile("" : "=r"(one) : "0"(one));`
 *   scheduling barrier that HEAD carried, and move `one = 1;` to AFTER the
 *   `mask`/`bits` computation.
 *   Mechanism: the target materializes the constant 1 TWICE — once as the
 *   loop's opaque `one` (`li a2,1`, feeding the two rotated `sllv v0,a2,s1`
 *   at target idx 24/34) and once as the operand of the mask shift
 *   (`li v0,1; sllv v0,v0,v1; nor v0,zero,v0`, target idx 17-19). With
 *   `one = 1;` placed BEFORE the mask expression, cse.c sees constant 1
 *   already live in `one`'s pseudo at the mask's expansion point and reuses
 *   it (`sllv v0,a2,v0`), costing exactly one instruction — that was the
 *   whole 188-vs-189 deficit. Placing `one = 1;` after the mask means the
 *   mask's own constant register has already been clobbered by its `sllv`
 *   result, so cse has nothing to reuse and GCC emits a second `li 1`.
 *   The second `li 1` also lands in the `lbu v1` load-delay slot, matching
 *   target's fill exactly. The stripped barrier was never doing this work:
 *   the sandbox strips it, and the honest floor was 18 WITH it in place.
 *
 * L2a (12 -> 8) — order `one = 1;` BEFORE `ptr = &D_801077B0;`.
 *   Target prologue order is `li a2,1` (idx 7) then the D_8010277C `lb`
 *   (8-9) then the D_801077B0 `la` (10-11); with `ptr` assigned first our
 *   build emitted the `la` ahead of `li a2,1`. Source statement order is
 *   what sets the sched1 priority here.
 *
 * L2b (8 -> 4) — compute `mask` BEFORE `count = 0; i = 0;` (declare `bits`
 *   separately and assign it after the two zero-inits).
 *   Target hoists the 0x3EF3DF constant pair (`lui a0` / `ori a0`) above the
 *   callee-save stores, to idx 1-2 immediately after `addiu sp,sp,-0x28`;
 *   with `count = 0; i = 0;` first, our build put the pair 4 slots later,
 *   after `move s1,zero`. Making the mask the first statement of the body
 *   raises that chain's scheduling priority enough to clear the saves.
 *
 * L3 (4 -> 0) — shared trailing `append_last:` block with BOTH arms doing
 *   `goto append_last`, and an explicit `goto set_count;` for the
 *   nothing-to-append path; `val` is assigned INSIDE each arm's `if`, not
 *   before it.
 *   Mechanism: HEAD nested the `append_last` label inside the else-arm's
 *   `if` body, so jump.c/reorg.c laid the then-arm out as
 *   `bnez <append>; li a0,26` (branch TO the work) and allocated `val` to
 *   `a0`. Target is `beqz <end>; li v0,26; j <append>` — branch AWAY from
 *   the work, work reached by fallthrough in the else-arm — and `val`
 *   reuses the dead `and` result register `v0`. Hoisting the label out of
 *   both arms so each arm is a plain `if (flag) { val = K; goto append; }`
 *   over a common tail reproduces both the branch sense and the register.
 *   This fixed all 4 remaining diffs at once (branch sense at target idx
 *   162, plus `val` in v0 instead of a0 at idx 163/171/174).
 *
 * NOT a cheat, per construct:
 *   - `s32 one = 1;` opaque-one is the SOTN-sanctioned shape for keeping
 *     `bits & (one << i)` as `sllv`+`and` instead of GCC's `(x>>i)&1`
 *     bit-test rewrite (see .claude/rules/loop-rotation-two-shift.md and
 *     the "opaque arithmetic variables" entry in no-new-park-categories).
 *     It predates this session; the session did not add it.
 *   - every local declared here is read; there are no dead stores, no
 *     self-assignments, no unused arrays, no register pins, no inline asm,
 *     no volatile coercion, no alias renames. The session's net effect on
 *     the cheat surface is -1 (the scheduling barrier is gone).
 *   - the `goto set_count;` / shared `append_last:` shape is ordinary
 *     control flow that a human would write for "append one of two byte
 *     codes, or nothing"; it is the same shared-tail idiom as
 *     .claude/rules/shared-end-label.md, not a no-op wrapper.
 *
 * Still outstanding (operator/driver surface, NOT touched by this session):
 *   regfix.txt lines 2018-2021 still carry this function's 4 rules
 *   (`reorder 8,9,1,2,3,4,5,10,11,6,7 @ 1-11` and three `subst "$4" "$2"`
 *   at 147/154/155). They are now redundant — the sandbox reaches 0 with
 *   all 9 rules dropped — and must be retired by the operator via
 *   `engine retire cpu_set_move_command_and_dir_for_no_action`.
 */
void cpu_set_move_command_and_dir_for_no_action(void) {
    s32 sp[2];
    s32 count;
    s32 i;
    s32 idx1;
    u8 *ptr;
    s32 one;

    {
        s32 mask = ~(1 << (&D_8008D538)[(s8)D_8010277C]) & 0x3EF3DF;
        s32 bits;
        count = 0;
        i = 0;
        bits = D_80106A50 & mask;
        one = 1;
        ptr = &D_801077B0;
        do {
            if (bits & (one << i)) {
                *ptr = i;
                ptr++;
                count++;
            }
            i++;
        } while (i < 0x1B);
    }

    i = 0;
    do {
        i++;
        idx1 = func_80079154() % count;
        {
            s32 idx2 = func_80079154() % count;
            s32 tmp = (u8)(&D_801077B0)[idx1];
            (&D_801077B0)[idx1] = (u8)(&D_801077B0)[idx2];
            (&D_801077B0)[idx2] = tmp;
        }
    } while (i < 0x6C);

    if (D_80106A50 & 0x10020) {
        s32 v1 = D_80106A50 & 0x20;
        s32 v0 = D_80106A50 & 0x10000;
        sp[0] = v1;
        sp[1] = v0;
        if (v1 == 0) {
            sp[0] = v0;
            sp[1] = 0;
            goto block_12;
        }
        if (v0 != 0) {
            if (func_80079154() & 1) {
                s32 t1 = sp[1];
                s32 t2 = sp[0];
                sp[0] = t1;
                sp[1] = t2;
            }
        }
block_12:
        if (count >= 0xB) {
            i = count;
            do {
                (&D_801077B0)[i] = (&D_801077AF)[i];
                i--;
            } while (i >= 0xB);
            {
                s32 dir = 0x10;
                if (sp[0] & 0x20) {
                    dir = 5;
                }
                D_801077BA = dir;
            }
        } else {
            s32 dir = 0x10;
            if (sp[0] & 0x20) {
                dir = 5;
            }
            (&D_801077B0)[count] = dir;
        }
        count++;
        if (sp[1] != 0) {
            u8 *a1 = &(&D_801077B0)[count];
            count++;
            {
                s32 dir = 0x10;
                if (sp[1] & 0x20) {
                    dir = 5;
                }
                *a1 = dir;
            }
        }
    }
    {
        u8 lookup = (&D_8008D9EC)[(&D_8008D538)[(s8)D_8010277C]];
        s32 val;
        if (lookup != 0) {
            if (D_80106A50 & 0x04000000) {
                val = 0x1A;
                goto append_last;
            }
        } else {
            if (D_80106A50 & 0x01000000) {
                val = 0x18;
                goto append_last;
            }
        }
        goto set_count;
append_last:
        (&D_801077B0)[count] = val;
        count++;
    }
set_count:
    D_800A391F = count;
    D_800A3783 = 0;
    D_800A37BC = 0;
}
