void func_800167EC(void) {
    s32 i;
    /* FAKE: pointer to the record, admitted on SOTN precedent (Q50, Q53; that function also
     * writes its global directly beside the pointer); mechanism: cse addresses the header
     * stores off rec's register and loop.c strength-reduces rec->times[i] into a pointer
     * copied from it, so unk_00 is stored at 0(base) and the loop walks base by 8;
     * exhaustion: direct only 19, rec only 10, times-array pointer 8, per-element pointer 10,
     * rec set after the header stores 15; memory/grind/func_80034708/evidence.md [s10] */
    FileRecord *rec = &D_80106A50; /* SOTN: src/st/st0/2A218.c:48 @db41b28 */

    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        rec->times[i].unk_0 = 0;
        rec->times[i].unk_1 = 0;
        rec->times[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
}
