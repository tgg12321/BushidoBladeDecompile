/* REJECTED AS A NET WIN (session 1) -- but the MECHANISM here is CONFIRMED
 * and is the most valuable single finding of the session. Read all of this
 * before touching the frame axis.
 *
 * WHAT IS CONFIRMED: a local aggregate of >= 4 bytes gets a stack slot from
 * GCC 2.7.2's assign_stack_local at expand time; get_frame_size() counts it
 * (vars= 4 -> MIPS_STACK_ALIGN rounds to 8), and when every access to it is
 * subsequently register-forwarded, the function emits `addiu $sp,$sp,-8` /
 * `addiu $sp,$sp,8` with ZERO $sp-relative traffic. That is EXACTLY
 * func_80086014's target frame signature, reproduced from pure C with no dead
 * declarations and no cheats.
 *
 * Measured signatures (cc1's own .frame comment; harness
 * tmp/grind/func_80086014/s1/{frame_probe.c,run_probe.sh}):
 *
 *   variant                                       insns  sp_traffic  vars
 *   ------------------------------------------------------------------------
 *   naive scalar baseline (candidate.c)             16        0        0
 *   union {s16 h[2]; s32 w;} field-wise (below)     21        0        8   <-- exact frame signature
 *   union {s16 h[2]; s32 w;} + aggregate assign     19        0        8   (but coalesces to ONE sw)
 *   struct {s16 f0,f1;} field-wise                  21        3        8
 *   s16 v[2] written + read field-wise              21        3        8
 *   struct {s16 f0,f1;} + separate flags local      20        2        8
 *   struct {s16 i,flags;} carrying index+flags      23        5        8
 *   union {s16 h; u8 b[2];} (only 2 bytes)          16        0        0
 *   union {u8 b[4]; s32 w;} for the flag only       22        0        0
 *   16-byte slot-struct pointer (no local agg)      18        0        0
 *
 * TWO SUB-FINDINGS worth keeping:
 *  (a) The aggregate must be >= 4 bytes. A 2-byte union gives vars= 0. So the
 *      slot is allocated per-aggregate by size, and 4 bytes is the threshold
 *      that rounds up to target's 8.
 *  (b) Zero stack traffic requires the aggregate collapse to ONE pseudo. The
 *      union's `s32 w` view does that (SImode pseudo -> forwarded, traffic 0).
 *      A plain 2-field struct accessed field-wise does NOT -- GCC keeps two
 *      HImode subregs and spills them (traffic 3). This is why the union
 *      variants are the only zero-traffic ones.
 *
 * WHY IT IS REJECTED AS A NET WIN: the same SImode collapse that buys zero
 * traffic also forces a mode-punning round-trip. cc1 materializes the whole
 * word and then re-extracts the halves, costing 4-5 instructions we do not
 * have room for -- we need to ADD exactly 2 (the frame pair), not 5. Emitted
 * for the form below (the extra insns marked):
 *
 *      andi  $7,$6,0xffff        <-- EXTRA (pack low half)
 *      sll   $2,$5,16            <-- EXTRA (pack high half)
 *      or    $7,$7,$2            <-- EXTRA (pack)
 *      move  $2,$0
 *      sll   $5,$4,16
 *      sra   $5,$5,16
 *      sll   $6,$5,4
 *      lbu   $3,D_800F65E0($5)
 *      sra   $4,$7,16            <-- EXTRA (unpack high half)
 *      sh    $7,D_80102A7A($6)
 *      sh    $4,D_80102A78($6)
 *      ori   $3,$3,0x0003
 *      sb    $3,D_800F65E0($5)
 *
 * Note it DOES keep the two separate `sh` stores at the right addresses, and
 * it DOES place the frame pair correctly (cc1 emits `subu $sp,$sp,8` into the
 * bnez delay slot; prologue_fix hoists it to the top, which is where target
 * has it, refilling the delay slot with the `move a3,a1` param-save). So
 * everything about this form is right except the pack/unpack tax.
 *
 * THE OPEN QUESTION THIS LEAVES (frontier H1): find a >= 4-byte local
 * aggregate that collapses to ONE forwarded pseudo (zero traffic, vars= 8)
 * WITHOUT a mode-punning round-trip -- i.e. an aggregate whose natural use in
 * this function is already SImode-shaped, so no packing is needed. Candidates
 * not yet tried: an aggregate whose members are s32/pointer rather than
 * paired s16 (nothing to pack); a 4-byte aggregate genuinely consumed by the
 * flag read/modify/write path instead of the value pair; or an aggregate that
 * models the 16-byte table slot itself and is READ from rather than written.
 *
 * DO NOT "solve" this by declaring an unused/unwritten aggregate. That is the
 * forbidden dead-local-array frame-coercion cheat ([[dead-vars-local-array]]);
 * the narrow 2026-07-01 carve-out requires the TARGET BYTES to contain
 * corresponding dead stores, and func_80086014's target contains NONE (zero
 * $sp traffic). The gate fails on its face here. The aggregate must be live
 * and load-bearing.
 *
 * The measured form (union, field-wise stores) -- vars=8, sp_traffic=0,
 * 21 insns, rejected for the 5-instruction pack/unpack tax:
 */
typedef union {
    s16 h[2];
    s32 w;
} PairU;

s32 func_80086014_REJECTED_union_field_wise(s16 idx, s16 x, s16 y)
{
    PairU p;

    if ((u16)idx < 0x18) {
        p.h[1] = y;
        p.h[0] = x;
        D_80102A7A[idx * 8] = p.h[1];
        D_80102A78[idx * 8] = p.h[0];
        D_800F65E0[idx] |= 3;
        return 0;
    }
    return -1;
}
