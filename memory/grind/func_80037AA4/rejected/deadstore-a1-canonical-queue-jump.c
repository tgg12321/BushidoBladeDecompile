/* s3 (2026-07-28): dead-store canonical steering — mechanically CONFIRMED,
   match-dead. A trailing dead a1-touch AFTER sum's last read flips cse's
   zero-class canonical to a1 (make_regs_eqv picks argmax regno_last_uid;
   reg_scan runs pre-flow so dead stores still count), so the guard slt
   charges a1 instead of sum. Measured: sra dst moved = charge moved. Dead as
   a lever, three ways:
   (1) beneficiary a1 queue-jumps: 10 refs/13 len -> pri 23076 > everyone ->
       a1 grabs $3 (sum=$5, p=$4 measured — worse than the swap).
   (2) the trailing touch needs uid AFTER sum's return-read, which forces
       staging the return through v0 — and staging RESTRUCTURES the tail
       (sra duplicated into both arms, bgez reads $2: the killed in-place
       family, n=22).
   (3) a fresh throwaway beneficiary z would dodge (1) but needs
       z.last_uid > the return-stmt read of the returned var — only
       unreachable code sits there, and jump1 deletes it before reg_scan.
   sum's last_uid is byte-locked maximal: T0 requires quotient-in-sum's-pseudo
   (sra $4) and the return statement always reads the result var last. */
s32 func_80037AA4(void) {
    s8 *var_v1;
    s32 var_a1;
    s32 var_a2;
    s32 var_a0;
    s32 var_v0;

    var_a1 = 0;
    var_a0 = 0;
    var_a2 = D_800A38C8;
    if (var_a1 < var_a2) {
        var_v1 = (s8 *)&D_80102810;
        do {
            var_v0 = *(s32 *)(var_v1 + 0x18);
            var_a1 += 1;
            var_a0 += var_v0;
            var_v1 += 0x28;
        } while (var_a1 < var_a2);
    }
    var_v0 = var_a0;
    if (var_a0 < 0) {
        var_v0 = var_a0 + 0x1FFF;
    }
    var_a0 = var_v0 >> 0xD;
    var_v0 = 0xF - var_a0;
    var_a1 += 1;
    return var_v0;
}
