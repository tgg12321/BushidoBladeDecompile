/* REJECTED (s3, structural) — duplicate the partial-add (base+var_v0) into
 * every arm so it sits in a predecessor block (sched1 is basic-block-local in
 * GCC 2.7.2, so it cannot hoist the join-block reload above a partial-add that
 * lives in a predecessor). In the STANDALONE mini this produces the exact
 * target tail structure: partial -> reload -> nop -> lookup -> nop -> sum, with
 * var_v0 -> $v0 and the 2nd (43rd) load-delay nop present.
 *
 * WHY IT DIES IN FULL CONTEXT: sandbox score 16, build_insns 47 (5 OVER target).
 * cross-jump (jump.c find_cross_jump) cannot merge the three duplicated
 * `partial = base + var_v0` copies because `base` occupies DIFFERENT hard regs
 * per arm — objdump: `addu v1,a2,v0` (arm1), `addu v1,a2,v0` (arm2),
 * `addu v1,a3,v0` (arm3). Two copies use $a2, one uses $a3, so the byte
 * sequences differ and no suffix-merge fires. The duplication therefore
 * persists as 3 physical adds -> +5 insns. Register pinning base to one reg to
 * force the merge would be a cheat (register-asm pin). KILLED.
 * (vB, folding `+ base` into var_v0 inside each arm, is the same family and
 *  produces the same 3-copy divergence.) */
s32 ang_hosei_80056FE8(s32 arg0) {
    s32 a2 = *((s32 *) arg0);
    s32 a3 = *((u8 *) ((*((s32 *) (a2 + 0x58))) + 3));
    s32 base = a3 * 40;
    s32 var_v0;
    s32 partial;
    if ((*((u8 *) (a2 + 0xA3))) != 0xFF) {
        if ((var_v0 = *((s16 *) (arg0 + 0x5E))) == 0) {
            var_v0 = (*((u8 *) (((s32) (&D_8009A830)) + (*((s16 *) (a2 + 0xE)))))) * 2;
            partial = base + var_v0;
        } else {
            var_v0 = (*((s8 *) (((s32) (&D_8009A838)) + (*((s16 *) (a2 + 0xE)))))) * 8;
            partial = base + var_v0;
        }
    } else {
        var_v0 = (*((u8 *) (((s32) (&D_8009A840)) + (*((s16 *) (a2 + 0x14)))))) * 2;
        partial = base + var_v0;
    }
    return partial + (*((s16 *) ((*((s32 *) arg0)) + 0x40A))) + 0x12C;
}
