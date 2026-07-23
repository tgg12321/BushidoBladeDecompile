/* REJECTED (s2, 2026-07-23) — score 10, does NOT defeat the lh-fold.
 * Attempt: load c,d as u16 (zero_extend) then sign-extend IN PLACE via a
 * reassignment statement (single reused pseudo), hoping to force
 * lhu+sll+sra instead of the lh fold WITHOUT the symmetric-form RA cascade.
 *
 * RESULT: GCC's combine RE-FOLDS `sign_extend(subreg(zero_extend(mem(reg))))`
 * back into a single sign-extending load (lh) for the offset-0 c and the
 * offset-2 d alike, because c,d remain single-use. Disassembly:
 *   lh v0,0(v1); lh v1,2(v1)   <- identical to the floor-10 candidate.
 * So the RA stays perfect (walker v1 reused for d) but we're still 4 insns
 * short (lh x2 vs target's lhu+sll+sra x2).
 *
 * CONCLUSION: the lh-fold is un-defeatable by any single-use c/d spelling.
 * combine collapses every zero-extend+shift back to lh. Only the SYMMETRIC
 * form ((s16) at the CALL, creating a second pseudo) prevents the fold — but
 * that reverses c/d evaluation order and evicts the walker v1->t0 (score 22,
 * see cd-signext-cascade22.c). See evidence.md s2. */
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base;
    s32 count;
    s32 entry;
    s32 a, b, c, d;
    s32 off;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    arg0 += off;
    count = *(s32 *)arg0;
    if (arg2 >= (u32)count) return -1;
    arg0 += 4;
    arg0 += arg2 * 0xC;
    entry = *(s32 *)arg0;
    arg0 += 4;
    a = (s32)*(u16 *)arg0;
    arg0 += 2;
    b = (s32)*(u16 *)arg0;
    arg0 += 2;
    c = *(u16 *)arg0;
    d = *(u16 *)(arg0 + 2);
    c = (c << 16) >> 16;
    d = (d << 16) >> 16;
    entry += base;
    func_800485EC(entry, arg3, (s16)a, (s16)b, c, d);
    return count;
}
