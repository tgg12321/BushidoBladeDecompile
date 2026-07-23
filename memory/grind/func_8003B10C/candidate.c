/* func_8003B10C — SOLVED, sandbox --disable all = 0 (64/64 insns, byte-match).
 *
 * THE 0-PATH: the duplicated chassis (recompute-inline of arg0*1100 + e2-less
 * per-arm reads + the *3/*2 split that keeps arm-specific code after the ED6
 * read so jump2 does NOT cross-jump-merge it) reaches 0 once the ED6 value is
 * held in a SINGLE-SET (block-local) variable per arm.
 *
 * WHY THE BLOCK-LOCAL SPLIT CLOSES IT (s3 structural win): the dup13 residual
 * was a sched1 hoist — the ED6 load was pulled ABOVE the ED2 *3 mult, so ED2
 * was still live in v1 and ED6 was forced to a0 (tbl bumped to a1). Root cause:
 * a function-scope `v1` assigned in BOTH arms has reg_n_sets==2, so its ED6
 * load does NOT get GCC 2.7.2 sched.c adjust_priority/birthing_insn_p's
 * "single-set -> load me late" boost and schedules early. Declaring `v1` as a
 * block-local INSIDE each arm makes each a distinct single-set pseudo
 * (reg_n_sets==1); the ED6 load then gets the late-birthing priority and is
 * scheduled AFTER the *3 mult (into the region target fills with `sll v0,v0,1`
 * in the j delay slot), reusing the freed v1 -> tbl keeps a0, ED6 lands in v1.
 * Verified byte-for-byte in the sandbox disasm (if-arm: lh v1,ED2; lui/addiu
 * a0,E6A4; sll/addu v0=v1*3; lh v1,ED6; j; sll v0,v0,1).
 *
 * This is ordinary C lexical scoping of a real, used value — no pin, no asm,
 * no dead store, no volatile, no FAKE annotation. The prior floor-8 hoisted
 * family (61 insns, ED6 read once) is superseded; this is the 64-insn match.
 */
void func_8003B10C(s32 arg0) {
    s32 addr = (s32)0x80190800;
    s32 v0;
    u8 *tbl;

    gpu_EnableDisplay();
    EndADRSound();
    gnd_close_8004939C();

    func_800493E4(*(s16 *)((u8 *)&D_80101EDA + arg0 * 1100));

    if (D_800A38DC == 5) {
        s32 v1;
        tbl = &D_8008E6A4;
        v0 = *(s16 *)((u8 *)&D_80101ED2 + arg0 * 1100) * 3;
        v1 = *(s16 *)((u8 *)&D_80101ED6 + arg0 * 1100);
        v0 *= 2;
        func_800494D4(arg0, *(tbl + v0 + v1));
    } else {
        s32 v1;
        tbl = &D_8008E5CC;
        v0 = *(s16 *)((u8 *)&D_80101ED2 + arg0 * 1100) * 8;
        v1 = *(s16 *)((u8 *)&D_80101ED6 + arg0 * 1100);
        func_800494D4(arg0, *(tbl + v0 + v1));
    }
    func_80049584(addr);
}
