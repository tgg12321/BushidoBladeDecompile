/* REJECTED s6 (forensics, 2026-08-23) — sandbox 15, IDENTICAL allocation to the
 * plain pa4-read chassis (out2 3 refs / live 42 / pri 714).
 *
 * Form: MID does `out2 = (s32 *)((u8 *)pa4 + 0x20); stptr = (s32) out2;`
 * (loop2 then uses stptr for both pointer sites).  The intent was an F1-style
 * combine-foldable ref-lift: two MID insns at flow time (+2 out2 refs), then
 * combine merges them into target's single `addiu s3,s7,32`.
 *
 * DUMP-MEASURED MECHANISM (tmp/grind/func_80041188/s5/e1/*):
 *   .rtl   insn167 `86 = 77+32`     insn170 `87 = 86`
 *   .cse   insn167 `87 = 77+32`     insn170 `86 = 87`   <-- cse1 DEST-SWAP
 *   .flow  insn170 GONE, out2 uses 3
 * cse.c's cse_insn rewrites the DESTINATION of the producing insn to the copy's
 * destination when the original dest dies in the copy, turning the chain into
 * `stptr = pa4+0x20; out2 = stptr;` — and out2 is then dead, so flow.c's
 * life-analysis dead-store elimination deletes the copy BEFORE reg_n_refs is
 * counted.  The lift is therefore uncounted, not just byte-free.
 * (Same mechanism family as split-staged-reinit-cse-destswap.c, now with the
 * pass and the exact insn rewrite named.) */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr = base;
    stptr += 0xFC;
    loop1:
    offset = (*tbl) * 6;
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
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = saved + 0x750;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr = (s32) out2;
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
    func_8004A348(buf, (s32 *) stptr);
    func_800523E0(pa4, (s32 *) stptr, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
