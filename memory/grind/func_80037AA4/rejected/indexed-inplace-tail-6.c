/* REJECTED s1: sandbox 6 (24 insns, one extra). Same unguarded indexed base as
   indexed-t0-tail-reg-swap-11.c but with the in-place tail (var_v0 >>= 13):
   sum's tail refs drop to 2, flipping the loop allocation to TARGET regs
   (i=$5, sum=$4, p=$3, vars=8, 21 cc1 insns) — but the tail degrades:
   v0 lands in $3 (not $2; conflicts with the return-value pseudo at li 15
   because v0 lives past sra), the move is emitted standalone BEFORE bgez
   (CSE rewrites the compare onto v0), adds become in-place, sra dest $3 not
   $4. Target tail is provably the T0 spelling: move $2,$4 in bgez delay,
   addu $2,$4,8191, sra $4,$2,13 (quotient coalesced with sum's variable). */
typedef struct {
    s32 unk00[6];
    s32 unk18;
    s32 unk1C[3];
} CamEnt_37AA4;
s32 func_80037AA4(void) {
    s32 var_a1;
    s32 var_a2;
    s32 var_a0;
    s32 var_v0;

    var_a0 = 0;
    var_a2 = D_800A38C8;
    for (var_a1 = 0; var_a1 < var_a2; var_a1++) {
        var_a0 += ((CamEnt_37AA4 *)&D_80102810)[var_a1].unk18;
    }
    var_v0 = var_a0;
    if (var_v0 < 0) {
        var_v0 = var_a0 + 0x1FFF;
    }
    var_v0 >>= 0xD;
    return 0xF - var_v0;
}
