/* REJECTED [s1] 2026-08-25 — goto-spelled loop (rule-era shape): no
 * NOTE_INSN_LOOP_BEG, so count/st land swapped (count=s1, st=s0; target wants
 * s0/s1) and the beqz delay slot never fills (rule-era $16<->$17 + fill_delay
 * regfix existed to paper over exactly this). Measured score 22 on the
 * otherwise-B chassis; real do-while loops measured 11 same chassis.
 * Rejected shape (outer loop):
 */
loop_top:
        spu = D_800A3044;
        if ((*((volatile u16 *)(spu + 4))) & 0x38) goto cleanup_A;
        if ((*((volatile u16 *)(spu + 4))) & 2) goto copy_byte;
inner:
        /* ... */
        if (!((*((volatile u16 *)(D_800A3044 + 4))) & 2)) goto inner;
copy_byte:
        /* ... */
        if (st[2] != 0) goto loop_top;
