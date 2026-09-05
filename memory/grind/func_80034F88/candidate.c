/* s48 (forensics, 2026-09-05) -- CHASSIS CHANGED.  This body replaces the
 * s39-s47 "mask store duplicated into the arms of if (c0)" chassis.  Same
 * honest floor (score 10, 49 insns) but a strictly closer SHAPE: it is the
 * first body in this ledger whose blocks 0/1 reproduce the target's
 * instruction ORDER exactly.
 *
 *   target                          this body (str1)
 *   lui   $v1,%hi(D_80106A73)       lui   $a0,%hi(D_80106A73)
 *   addiu $v1                       addiu $a0
 *   lbu   $a0,0($v1)                lbu   $v1,0($a0)
 *   addu  $a1,$v0,$zero             move  $a1,$v0
 *   andi  $a0,0xf8                  andi  $v1,0xf8
 *   sb    $a0,0($v1)                sb    $v1,0($a0)
 *   lw    $v0,0x20($a1)             lw    $v0,0x20($a1)
 *   lbu   $a0,0($v1)                nop            <-- cse'd away
 *   andi  $v0,1                     andi  $v0,1
 *   bnez  $v0                       bnez  $v0
 *    ori  $v0,$a0,1                  ori  $v0,$v1,1
 *   addu  $v0,$a0,$zero             move  $v0,$v1
 *   lui   $a0 ; addiu $a0           sb    $v0,0($a0)
 *   sb    $v0,0($v1)                lui   $a0 ; addiu $a0
 *
 * Blocks 2, 3, the copy loop and the epilogue are byte-identical to the
 * target.  The whole residual is (a) the missing block-1 reload and (b) the
 * $a0/$v1 seat exchange in blocks 0/1 (which is also what forces the block-2
 * lui/addiu pair after the block-1 sb instead of before it).
 *
 * WHY THIS CHASSIS AND NOT THE s47 ONE.  s39-s47 bought the surviving reload
 * with an EBB break (`if (c0) { *q = m; } else { *q = m; }`), which puts the
 * mask store in the arms; at sched1 time BB0 then loses the
 * lbu -> andi -> sb memory-dependence chain and sched1 emits the p[8] load
 * before the mask load.  Making the mask a PLAIN straight-line statement
 * restores the target's order for free.  The reload is then cse'd away
 * (cse.c store-forwarding, .cse dump), which costs 1 nop but no order.
 *
 * WHAT THE RESIDUAL IS, EXACTLY (measured this session, not inferred):
 *   - local_extract --suggest: BB0 holds two block-local quantities, the mask
 *     value (birth 6, death 10, 4 refs) and the p[8] value (birth 14, death
 *     20).  They no longer overlap (s47's overlap was an artefact of the
 *     arms chassis).
 *   - find_free_reg (local-alloc.c:2169-2247) builds `used` = fixed_reg_set
 *     | regs_live_at[birth..death) | ~GR_REGS.  Hard 2 is in it, because the
 *     call return $v0 is still live over [6,10) (the `move $a1,$v0` copy is
 *     scheduled AFTER the mask lbu -- exactly as in the target).  MIPS
 *     defines no REG_ALLOC_ORDER, so the scan is numeric from 0 and the mask
 *     value takes hard 3.
 *   - That puts hard 3 into the blocks-0/1 address allocno's conflict row
 *     (.greg: `;; 75 conflicts: 72 75 78 79 83 84 88 89 2 3 29`), so
 *     global.c find_reg seats the address at hard 4 = $a0.  The target seats
 *     it at hard 3 = $v1.
 *   - THE ONE LEVER LEFT: make hard 3 unavailable (or hard 2 available) over
 *     BB0 insn range [6,10) at local-alloc time.  Everything else in this
 *     function is already exact.
 *
 * Ordinary C: one declared pointer object `q`, three re-assignments, no FAKE
 * construct, no pun other than the pre-existing `*(&D_80106A70 + i)` loop
 * line (a DECLARATION-level issue, unchanged from every prior candidate).
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q & 0xF8;
    *q = m;

    {
        s32 v;
        s32 c;

        v = *q;
        c = p[8] & 1;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        v = *q;
        c = p[8] & 2;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        v = *q;
        c = p[8] & 4;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
