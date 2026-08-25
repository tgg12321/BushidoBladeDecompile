/* s20 (structural). Y2a with `s32 *pa4 = a4;` demoted to a bare declaration and
`pa4 = a4;` written as a statement AFTER the a4-sourced wrapped out2 definition,
so the two pseudos are no longer merged by cse1 (the equivalence class is only
formed at the copy, i.e. after out2's definition has already referenced a4).
MEASURED: sandbox 12 at 133 build insns. ALLOCDBG: stptr 6/41=2926 $s3,
stptr2 6/48=2500 $s0, i 8/98=2448 $s4, tbl 5/48=2083 $s5, out2 4/43=1860 $s6,
pa4 6/93=1290 $s7, a3 4/100=800 $fp, out3 3/47 $s3.
THIS IS ALL-TARGET SEATS *TOGETHER WITH* target's block-2 `addiu $s3,$s7,0x20`
on the GOTO chassis -- the combination E-s16-5 called impossible there.
WHY DEAD (for now): keeping a4 and pa4 distinct materialises the copy.
objdump vs target: target loads the parameter straight into its callee-saved
seat (`lw $s7,0x58($sp)`), we load it into $v0 (a4 is a block-0-local quantity,
so local_alloc seats it in a call-clobbered register) and then emit
`addu $s7,$v0,$zero`; the extra insn plus its block-0 scheduling wake is the
whole 12. The axis is alive: make that copy free (a4 and pa4 sharing one hard
register, or the load landing directly in the global allocno) and this form
is distance 0.
*/
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4;
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 *out3;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    saved = base + 0x94;
    do { out2 = (s32 *) (((u8 *) a4) + 0x20); } while (0); /* FAKE: loop-note ref weighting, +1 flow reference on out2 */
    pa4 = a4;
    stptr = base + 0xFC;
        loop1:
    do { offset = (*tbl) * 6; } while (0); /* FAKE: loop-note ref weighting, +1 flow reference on tbl (4 -> 5) */
    p = (u16 *) (offset + (s32) a1);
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, pa4);
    tbl++;
    offset = offset + (s32) a2;
    p = (u16 *) offset;
    i++;
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, out2);
    func_800523E0(pa4, out2, a3, stptr + 0x38);
    do { *((s16 *) (stptr + 6)) = 2; } while (0); /* FAKE: loop-note ref weighting, +1 flow reference on stptr (5 -> 6) */
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = saved + 0x750;
    out3 = (s32 *) (((u8 *) pa4) + 0x20);
    loop2:
    func_80044DE4((s16 *) a1, (s16 *) a2, a3, stptr2 + 0x4C);
    a1 += 6;
    a2 += 6;
    buf[0] = *((u16 *) a1);
    a1 += 2;
    buf[1] = -(*((u16 *) a1));
    a1 += 2;
    buf[2] = -(*((u16 *) a1));
    a1 += 2;
    func_8004A348(buf, pa4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    func_8004A348(buf, out3);
    func_800523E0(pa4, out3, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
