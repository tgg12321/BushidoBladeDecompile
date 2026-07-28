/* REJECTED s1: sandbox 11 (23/23 insns). Unguarded indexed for-loop — GCC
   strength-reduces to the exact target walking-pointer loop AND allocates the
   phantom 8-byte frame (vars=8) with la after blez, lw 24(p), addu p,p,40 —
   ALL structure correct. But with the T0 tail (quotient back into var_a0),
   sum's 4 tail refs make global-alloc allocate sum BEFORE the compiler-
   generated giv: sum=$3/p=$4 (target: sum=$4/p=$3). Measured inert against
   the swap: all 24 decl orders, init orders (sum-first/n-first), while vs for,
   postinc-in-index, plain-add vs +=, u8-arith vs struct-index, fresh-quotient
   var (coalesces the move away, 20 insns), quotient-into-i (i=$3). The ONLY
   flip lever found (in-place tail on var_v0, sum tail refs 2) breaks the tail
   shape (score 6, see indexed-inplace-tail-6.c). */
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
    if (var_a0 < 0) {
        var_v0 = var_a0 + 0x1FFF;
    }
    var_a0 = var_v0 >> 0xD;
    return 0xF - var_a0;
}
