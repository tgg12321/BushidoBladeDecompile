/* func_80041188 -- s19 (rederive) ALTERNATIVE CHASSIS. sandbox --disable all = 3,
 * 132 target / 132 build insns, ALL-TARGET callee-saved seats, and ZERO FAKE
 * constructs (no chain-extender, no wrap, no variable reuse, no dead code).
 *
 * WHAT IS NEW. s18's E-s18-1 rederived the memory layout as one 0x68-stride array
 * based at `base + 0x94` with `i` as the entry index. This body writes that layout
 * literally: loop1 is a real `do { ... } while (i < 0x12);` and the entry address is
 * written INLINE at its two use sites as `ents + i * 0x68 (+0x38 / +6)`. Consequences,
 * all measured this session:
 *   - loop.c strength-reduces the multiply into a giv walking pointer whose initial
 *     value folds to `addiu $s3,$v0,0xFC` and whose increment is `addiu $s3,$s3,0x68`
 *     in the branch delay slot -- i.e. target's own block-0 and loop1-tail insns;
 *   - the giv gets 9 references / live 40 = priority 6750 FOR FREE, which is the honest
 *     replacement for candidate.c's FAKE-annotated F1 chain-extender
 *     (`stptr = base; stptr += 0xFC;`), whose entire purpose was to buy `stptr` enough
 *     references to outrank `i`;
 *   - loop2's pointer initial value is `ents + 0x750`, which is exactly target's
 *     `lw $t0,0x18($sp)` + `addiu $s0,$t0,0x750` -- the block-0 spill of `ents` is
 *     explained by the layout instead of being hand-placed;
 *   - the address must NOT be named (see
 *     rejected/realloop-giv-uservar-copy-steals-s1-a3-spills.c): a user variable makes
 *     loop.c emit `ent = <giv>` every iteration, which costs a seat and spills a3.
 *
 * MEASURED ALLOCNO TABLE (tmp/grind/func_80041188/s19/W4/cc1.err):
 *   a1 17/99=6868 $s1 | a2 17/99=6868 $s2 | giv 9/40=6750 $s3 | i 11/97=3402 $s4 |
 *   tbl 7/47=2978 $s5 | out2 6/46=2608 $s6 | pa4 8/94=2553 $s7 | stptr2 6/48=2500 $s0 |
 *   a3 5/99=1010 $fp | out3 3/47 $s3 (shared) | ents spilled | const-2 pseudo spilled.
 *
 * RESIDUAL (3 insns, both causes identified):
 *   (a) 2 insns -- `addiu $t0,zero,2` / `sh $t0,6($s3)` where target has $v0. In a REAL
 *       loop, loop.c hoists the invariant `li 2` into the preheader (scan_loop,
 *       loop.c:686-700: the destination is a compiler pseudo, so movable test (2)
 *       passes; move_movables' threshold test at loop.c:1631 passes for any lifetime
 *       once the loop has calls), the hoisted pseudo is then live across the whole
 *       function (3 refs / live 92), global.c spills it, and reload rematerialises the
 *       constant into $t0 instead of the $v0 that local_alloc picks when the li is
 *       generated in place. This +2 is a fixed tax of the real-loop regime and it is
 *       what separates this body from candidate.c's floor of 1.
 *   (b) 1 insn -- the standing lock: `move $s3,$s6` (from `out3 = out2;`) where target
 *       has `addiu $s3,$s7,0x20`. Swapping in target's spelling swaps the out2/pa4
 *       seats (see rejected/realloop-Z0-out2-5refs-live41-one-reference-short.c).
 */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 ents;
    s32 *out2;
    s32 *out3;
    s32 offset;
    u16 *p;
    s32 stptr2;
    ents = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    do {
        offset = (*tbl) * 6;
        p = (u16 *) (offset + (s32) a1);
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        func_8004A348(buf, pa4);
        tbl++;
        offset = offset + (s32) a2;
        p = (u16 *) offset;
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        func_8004A348(buf, out2);
        func_800523E0(pa4, out2, a3, ents + i * 0x68 + 0x38);
        *((s16 *) (ents + i * 0x68 + 6)) = 2;
        i++;
    } while (i < 0x12);
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = ents + 0x750;
    out3 = out2;
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
