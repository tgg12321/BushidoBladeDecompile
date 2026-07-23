/* REJECTED (s3, 2026-07-23): the off-first structural axis is DEAD.
 *
 * At floor 1 the sole residual is the walker-relocation add:
 *   build  addu $v1,$v1,$v0  (base-first)   vs   target addu $v1,$v0,$v1 (off-first)
 * The ONLY C reaching off-first WITH the correct v1-walker routing is the
 * commutative operand swap `arg0 = off + base` (rejected/offbase-operand-
 * shuffle-cheat.c, byte-0, or-tree-shape-shift FORBIDDEN). Every *non-swap*
 * spelling that tries to make `off` lead the add MISROUTES the walker and
 * regresses. Measured this session (candidate floor-1 base):
 *
 *   off += base; (off becomes the walker)          -> score 22  (walker -> $a1,
 *        `addu a1,a1,t0` off-first but wrong reg; c/d cascade lh/lhu)
 *   off += base; arg0 = off;                        -> score 22  (walker -> $t0)
 *   arg0 = ((s32*)arg0)[arg1] + arg0;  (mem-first)  -> score 22  (walker -> $t0)
 *   ((s32*)base)[arg1] + base  (fresh walker)       -> score 22
 *   arg0 = base + ((s32*)base)[arg1]  (mem inline)  -> score 20  (walker -> $t0)
 *   s32 w = base + off; ... walk w                  -> score 12/22 (base-first
 *        or cascade; copy coalesces, breaks v1 routing)
 *   arg0 = base + off  (natural, THIS candidate)    -> score  1  (base-first)
 *
 * Root cause (RTL + cc1psx): MIPS addu is 3-operand, so GCC picks operand order
 * by commutative canonicalization; for two plain pseudos (off, base) source
 * order controls, and only the swap `off + base` yields off-first. Making off a
 * MEM or an accumulator to raise its precedence moves the WALKER off $v1 (RA
 * gives the leading operand the walker reg), so off-first + v1-walker are
 * mutually exclusive except via the swap. cc1psx emits base-first from the
 * natural `base + off` too (n1.psx.s), so the original source itself used the
 * off-first order this project classifies as or-tree-shape-shift.
 *
 * Kept as the measured proof that the floor-1 residual has NO non-cheat closer
 * on the structural axis. Disposition: endgame-lock (owner-escalation) after a
 * permuter pass from the floor-1 base. */
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base, count, entry, a, b, c, d, off;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    off += base;            /* off-first order, but walker misroutes to $a1 -> score 22 */
    count = *(s32 *)off;
    off += 4;
    if (arg2 >= (u32)count) return -1;
    off += arg2 * 0xC;
    entry = *(s32 *)off;
    off += 4;
    a = (s32)*(u16 *)off;
    off += 2;
    b = (s32)*(u16 *)off;
    off += 2;
    c = (s32)*(u16 *)off;
    off += 2;
    d = (s32)*(u16 *)off;
    entry += base;
    func_800485EC(entry, arg3, (s16)a, (s16)b, (s16)c, (s16)d);
    return count;
}
