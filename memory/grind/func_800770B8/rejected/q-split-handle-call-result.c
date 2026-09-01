/* REJECTED s1 (2026-09-01): split-handle for the post-func_8006E49C stores.
 * Hypothesis: target stores 0x30/0x34 through $v0 (call result) while +4 goes
 * through $s1, so route them through a second local `q` holding the raw call
 * result. MEASURED: isolated on the floor-14 chassis -> score 28, build_insns
 * collapses 176 -> 171: cse forwards `q` (== the value just stored to
 * D_800A36A0) into the loop preheader and DELETES the pre-loop reload of
 * D_800A36A0 plus downstream reloads, restructuring the whole loop entry.
 * Combined runs: +mask+tail = 41; +row-pointer = 52. KILLED in this spelling.
 * The $17-vs-$2 handle residual (rows 35-36 of the s1 posdiff) needs a lever
 * that does NOT create a second C handle to the same value visible to cse.
 */
{
    s32 *prev = p_old;
    u8 *q = func_8006E49C(r, D_800A35D8);
    p_old = (s32 *)q;
    D_800A36A0 = (u8 *)p_old;
    *(s32 *)((u8 *)p_old + 4) = (s32)prev;
    *(s32 *)(q + 0x30) = 0;
    *(s16 *)(q + 0x34) = 0;
}
