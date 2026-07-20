/* REJECTED s3: split `count=*p++` into `count=*(base+off); p=(base+off+4)`
 * intending to lift base_addr reg_n_refs to 3 pre-loop. Score 3 (from
 * baseline 1) — the split folds all three base+off computes through $a0
 * (`addu a0,a0,v0` at insn #18), same or worse alias residual, and
 * introduces new diffs downstream. Not a winning shape. */
void InitHiraRmd_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    s32 buf[8];
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    s32 word_offset;
    p = (u32 *)arg0;
    base_addr = arg0;
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    word_offset = ((*p) >> 2) << 2;
    count = *(u32 *)(base_addr + word_offset);
    p = (u32 *)(base_addr + word_offset + 4);
    /* ... loop body identical to candidate ... */
    (void)buf;
}
