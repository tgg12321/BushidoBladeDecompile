/* func_800480C0 — s2 (structural, 2026-09-02) BEST POLICY-LEGAL FORM.
 * sandbox --disable all = 32 (73 scorable insns vs target 74); .frame $sp,56
 * (vars=0, regs=8, args=24) vs target .frame $sp,88 (vars=32, regs=8, args=24).
 *
 * WHY THIS AND NOT THE s1 BODY: the s1 handoff form (volatile u32 pre_pad[8])
 * was Judge-FAILed 2026-09-02 04:28 (docs/grind/decisions.md:20349) — the
 * unwritten-leading-pad family is a CLOSED per-function enumeration and
 * func_800480C0 is not in it, so the pad is banned here in any spelling. That
 * body is preserved at rejected/pad-judge-banned-2026-09-02.c (it is still the
 * bytes-proven form: full build SHA1 == oracle, s1/build.log).
 *
 * s2 correction to the ledger: the previously recorded honest floor of 20 was a
 * SANDBOX ARTIFACT, not a measurement. engine/cheats.py deletes the pad
 * declarator text but leaves the dangling `volatile` qualifier, which then binds
 * to the NEXT declaration — the stripped source in
 * tmp/sandbox/func_800480C0/src/text1b.c:160 reads `volatile` / `u32 *p;`, i.e.
 * it silently scores a `volatile u32 *p` body. The honest pad-free floor is 32.
 *
 * Residual at 32 (objdump diff, tmp/grind/func_800480C0/s2/v0_dis.txt): the
 * instruction STREAM matches the target one-for-one except (a) every sp-relative
 * offset (frame 0x38 vs 0x58) and (b) a callee-saved SEAT ROTATION — target
 * binds base_addr to $s2 and sx_arg2..5 to $s6/$s5/$s4/$s3; this build binds them
 * to $s6 and $s5/$s4/$s3/$s2. The rotation is NOT independent of the frame: with
 * the pad present the seats come out correct (SHA1 match), so the next attack is
 * the RA seat, not more frame spellings — classify with
 * tools/ra_solver/inverse_compose.py. */
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0;
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        s32 sx_arg4;
        s32 sx_arg5;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        sx_arg4 = arg4;
        sx_arg5 = arg5;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            func_800482C8(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          (s32)v0v + sx_arg5);
        } while ((count--) != 0);
    }
}
