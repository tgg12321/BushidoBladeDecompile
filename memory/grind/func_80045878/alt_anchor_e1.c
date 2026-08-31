/* func_80045878 -- ALTERNATIVE BYTE-EXACT FORM (s10b, 2026-08-30, rederive modality).
 * sandbox --disable all = 0, build_insns 108 == target 108, rules_dropped 0.
 *
 * Structurally DIFFERENT from candidate.c in exactly one respect: the carrier
 * local `c` takes its earlier (non-block-local) mention from the FIRST-IF ELSE
 * ARM's argument value `0x1A88 + (s32)s1` instead of from the third `if`'s
 * `s1[3]` condition read.  The third `if` is therefore left as ordinary,
 * untouched C (`... && (s1[3] != -2)`).
 *
 * WHY IT STILL WORKS (mechanism, read out of the compiler source this session):
 * reg_basic_block[] is computed ONLY in flow.c's life_analysis (flow.c:2072 and
 * flow.c:2508 set REG_BLOCK_GLOBAL the moment a pseudo is referenced from a
 * second block) and is NEVER recomputed afterwards -- toplev.c runs
 * combine_instructions at 3004, schedule_insns at 3033, and only then
 * regclass()/local_alloc() at 3051-3052.  So the carrier's earlier reference has
 * to exist at FLOW time only; combine is free to fold it into the call argument
 * afterwards (which is why the earlier site still emits the single target insn
 * `addiu $a1, $s1, 0x1A88` and the carrier does not inherit an $a1 seat).
 * local-alloc.c:472 then skips the carrier (reg_basic_block < 0 == GLOBAL) and
 * global.c is free to seat the tail base in $v0 and the carrier in $v1.
 *
 * NOT SUBMITTED.  It carries the same construct the Judge FAILed on 2026-08-30
 * 21:30: a fresh local written more than once as an allocation carrier.  It is
 * banked because it proves the anchor SITE is not unique (two independent
 * sites reach 0) and because it isolates the carrier from the if-condition.
 */
void func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s16 *v0;
    s16 *s1;
    s32 s0;
    s32 c;
    v0 = (s16 *) func_8004574C(a0);
    if (v0 != 0) {
        s1 = (s16 *) ((s32 *) v0)[1];
    } else {
        s1 = (s16 *) func_800455AC(a0);
        c = 0x1A88 + ((s32) s1);
        func_80045600(a0, c);
        func_80045230(0);
        func_80045694(a0, (s32) (&func_80045AA4));
        s1[4] = -1;
        s1[3] = 0;
    }
    s3 = a0 + 3;
    if (func_8004574C(s3) != 0) {
        func_800400F8((s32) s1);
    }
    if (((func_8004574C(s3) != 0) && (s1[4] == a1)) && (s1[3] != (-2))) {
        s1[3] = 0;
    } else {
        *((s32 *) (((s32) s1) + 0x20)) = a2;
        s0 = (s32) func_800455AC(s3);
        *((s32 *) (((s32) s1) + 0x1C)) = s0;
        if (a2 != 0) {
            func_80044ED8(a1, a2);
        } else {
            func_80044ED8(a1, s0);
            s0 = s0 + ((((u32) ((s32 *) s0)[*((s32 *) s0)]) >> 2) << 2);
            func_80045230(s0);
        }
        func_80045600(s3, s0);
        func_80045694(s3, (s32) (&func_80045AA4));
        s1[3] = 1;
        *((s32 *) (((s32) s1) + 0x24)) = 0;
        *((s32 *) s1) = 0;
    }
    v0 = s1;
    c = a0 + 3;
    v0[11] = c;
    v0[2] = a0;
    v0[4] = a1;
    v0[10] = a0;
    v0[8] = a0;
    c = 0x8000;
    *((s32 *) (((s32) v0) + 0x18)) = c;
}
