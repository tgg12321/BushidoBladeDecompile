/* ===========================================================================
 * DRAFT TRANSPLANT — func_800889D4  ( = PsyQ libspu spu.c `_spu_FwriteByIO` )
 * 0x800889D4 · src/main.c:1682 (currently INCLUDE_ASM) · dist 115 · 0 rules
 *
 * (a) psyz source: tmp/psyq_prov/psyz/decomp/src/libspu/spu.c:111-159
 *
 * (b) MAPPING DECISIONS.  psyz reaches the SPU register file through
 *     `SPUR(field)` / `SPUW(field,val)` macros over `union SpuUnion *_spu_RXX`.
 *     BB2 has NO such union and spells every SPU register access as
 *         *(volatile u16 *)(D_800A2CDC + <offset>)
 *     with `extern s32 D_800A2CDC;` — see the already-matched `_spu_init`
 *     (src/main.c:1590-1681), which does this ~30 times.  MATCHED THAT STYLE;
 *     no union is introduced.  Field offsets read off asm/funcs/func_800889D4.s:
 *
 *       psyz field        offset   BB2 spelling                       asm
 *       ----------------  -------  ---------------------------------  ------------
 *       trans_addr        0x1A6    *(volatile u16*)(D_800A2CDC+0x1A6)  sh 0x1A6
 *       trans_fifo        0x1A8    *(volatile u16*)(D_800A2CDC+0x1A8)  sh 0x1A8
 *       spucnt            0x1AA    *(volatile u16*)(D_800A2CDC+0x1AA)  lhu/sh 0x1AA
 *       spustat           0x1AE    *(volatile u16*)(D_800A2CDC+0x1AE)  lhu 0x1AE
 *       (_spu_init already uses 0x1AA and 0x1AE at src/main.c:1601,1607)
 *
 *       psyz global       BB2 global      addr         status
 *       ----------------  --------------  -----------  ----------------
 *       _spu_RXX          D_800A2CDC      0x800A2CDC   ALREADY DECLARED (main.c:74)
 *       _spu_tsa          D_800A2CF4 u16  0x800A2CF4   ALREADY DECLARED (main.c:77)
 *       _spu_Fw1ts()      _spu_Fw1ts()    —            ALREADY DECLARED (main.c:84)
 *       "SPU:T/O [%s]\n"  D_800163D8      0x800163D8   ALREADY DEFINED  (main.c:1583)
 *       "wait (wrdy H -> L)" D_800163F8   0x800163F8   ALREADY DEFINED  (main.c:1585)
 *       "wait (dmaf clear/W)" D_8001640C  0x8001640C   ALREADY DEFINED  (main.c:1586)
 *
 *       SPU_CTRL_MASK_SRAM_TRANSFER_MODE      = 0x30  -> andi 0xFFCF (asm 80088A68)
 *       SPU_CTRL_MASK_TRANSFER_MANUAL_WRITE   = 0x10  -> ori  0x10   (asm 80088A6C)
 *       (BB2 has no SPU_CTRL_* names; literals match main.c's existing style.)
 *
 * (c) ASM CROSS-CHECK (asm/funcs/func_800889D4.s, 115 insns, 4 back-edges =
 *     psyz's 4 loops; jal set = {_spu_Fw1ts, printf} = psyz's call set):
 *       - prologue reads spustat & 0x7FF into $s3 and stores _spu_tsa to
 *         0x1A6 BEFORE the first _spu_Fw1ts  (asm 800889D4-80088A10)
 *       - `beqz $s1` on size guards the whole while-loop (size is u32 but the
 *         chunk clamp is `sltiu $v0,$s1,0x41` = `size > 0x40 ? 0x40 : size`)
 *       - inner fifo loop steps i by 2 and cur_pos by 2 (`addiu $s2,$s2,0x2`)
 *       - both timeout loops are `sltiu $v0,$v1,0xF01`, i.e. psyz's
 *         `timeout > 0xF00` — the sweep's shared 0x400 / 0x7FF evidence
 *       - printf takes (&D_800163D8, &D_800163F8) then (&D_800163D8,
 *         &D_8001640C); the second one RETURNS (asm falls to epilogue), the
 *         first one BREAKS (asm 80088AC4 `j .L80088AEC`) — psyz has exactly
 *         this asymmetry (break vs return); keep it.
 *
 * (d) OPEN QUESTIONS FOR THE MEASURING SESSION
 *   1. `printf` is declared in main.c:83 as `extern void printf(s32 *, s32 *);`
 *      and the strings as `extern s32 D_800163D8;` (main.c:81) while ALSO being
 *      defined as `const char D_800163D8[16]` at main.c:1583.  The existing
 *      _spu_init call site spells it `printf(&D_800163D8, &D_800163E8)`.  This
 *      draft copies that spelling verbatim — if the compiler objects, fix it
 *      the way _spu_init already does, not by adding a new declaration.
 *   2. `spustat` is captured as u16 and compared against a `& 0x7FF` value; the
 *      asm does `andi $a1,$s3,0xFFFF` before the compare, which is the u16
 *      truncation — keep the u16 type, an s32 loses that instruction.
 *   3. `size` is u_long in psyz; the `> 0x40` test compiles to `sltiu` either
 *      way, but `while (size > 0)` on an unsigned is what produces the target's
 *      `bnez` rather than a `bgtz`.  Keep it unsigned.
 *   4. Replaces the INCLUDE_ASM at src/main.c:1682 ONLY.  The two INCLUDE_ASM
 *      lines that follow (_spu_FiDMA, _spu_Fr_) must stay immediately after it
 *      in address order — see the comment block at src/main.c:1683-1692, and
 *      note that comment's warning about not spelling the alabel symbol name
 *      anywhere in the file.
 * ======================================================================== */

void func_800889D4(u8 *addr, u32 size) {
    u16 spustat;
    s32 num_to_trans;
    u16 *cur_pos;
    s32 spustat_cur;
    s32 i;
    u16 cnt;
    u32 timeout;

    cur_pos = (u16 *)addr;
    spustat = *(volatile u16 *)(D_800A2CDC + 0x1AE) & 0x7FF;
    *(volatile u16 *)(D_800A2CDC + 0x1A6) = D_800A2CF4;
    _spu_Fw1ts();

    while (size > 0) {
        num_to_trans = (size > 0x40) ? 0x40 : size;
        for (i = 0; i < num_to_trans; i += 2) {
            *(volatile u16 *)(D_800A2CDC + 0x1A8) = *cur_pos++;
        }
        cnt = *(volatile u16 *)(D_800A2CDC + 0x1AA);
        cnt &= ~0x30;
        cnt |= 0x10;
        *(volatile u16 *)(D_800A2CDC + 0x1AA) = cnt;
        _spu_Fw1ts();
        timeout = 0;
        while (*(volatile u16 *)(D_800A2CDC + 0x1AE) & 0x400) {
            timeout++;
            if (timeout > 0xF00) {
                printf(&D_800163D8, &D_800163F8);
                break;
            }
        }
        _spu_Fw1ts();
        _spu_Fw1ts();
        size -= num_to_trans;
    }

    cnt = *(volatile u16 *)(D_800A2CDC + 0x1AA);
    cnt &= ~0x30;
    *(volatile u16 *)(D_800A2CDC + 0x1AA) = cnt;
    timeout = 0;
    spustat_cur = *(volatile u16 *)(D_800A2CDC + 0x1AE) & 0x7FF;
    while (spustat_cur != spustat) {
        timeout++;
        if (timeout > 0xF00) {
            printf(&D_800163D8, &D_8001640C);
            return;
        }
        spustat_cur = *(volatile u16 *)(D_800A2CDC + 0x1AE) & 0x7FF;
    }
}
