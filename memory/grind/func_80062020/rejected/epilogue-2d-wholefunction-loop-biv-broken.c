/* s6 probe: WHOLE-FUNCTION 2D-array object model (loop AND epilogue).
   Motivated by the s6 forensic sweep: the consumer func_800620B8 addresses all
   three columns of one row with three independent %hi/%lo(sym)+index accesses
   and never forms a shared row base — the table's whole-program idiom. The 2D
   array `s32 tbl[N][3]` is the single-object C declaration that produces exactly
   that per-column symbol-folded form, and splat would name tbl / tbl+4 / tbl+8
   as D_800F1198 / D_800F119C / D_800F11A0. Never measured with the LOOP in this
   shape (s5 measured 2D only in the epilogue, over a three-symbol loop). */
void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    s32 (*tbl)[3] = (s32 (*)[3])&D_800F1198;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
        tbl[i][0] = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 4);
        i = i + 1;
        tbl[i - 1][1] = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 8);
        tbl[i - 1][2] = t;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    tbl[i][2] = 0;
    tbl[i][1] = 0;
    tbl[i][0] = 0;
}
