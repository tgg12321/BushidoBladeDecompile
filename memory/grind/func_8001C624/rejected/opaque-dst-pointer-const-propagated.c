/* REJECTED (s2) — opaque copy-destination pointer.  KILLED: score 61, byte-for
 * byte the SAME output as the plain wrapper-free form (v1).  Fully inert.
 *
 * THE IDEA:
 *   sched.c's canon_rtx resolves a pseudo to a symbol only via reg_known_value,
 *   which init_alias_analysis populates ONLY for pseudos with exactly one set.
 *   If the block-copies' destination address lived in a pointer variable ASSIGNED
 *   MORE THAN ONCE, reg_n_sets > 1 => reg_known_value unset => canon_rtx returns
 *   the REG => memrefs_conflict_p falls through to its default `return 1` =>
 *   the copy would conflict with EVERY memory op => a total, honest fence, with
 *   every global keeping its own symbol (so no reloc respelling, and the sandbox
 *   could still score it 0).  That would have been the clean closing form.
 *
 * WHY IT IS DEAD:
 *   GCC 2.7.2 const-propagates the pointer away long before sched1.  `dst =
 *   &D_80101F90; *(Blk16 *)dst = ...;` is folded by cse/copy-prop back into
 *   `(mem:BLK (reg <- symbol_ref D_80101F90))` — a fresh, SINGLE-set pseudo per
 *   copy (RTL dump confirms: pseudos 88/89, 92/93, 96/97, one set each).  So
 *   reg_known_value IS populated, canon_rtx DOES resolve to the symbol, and the
 *   alias picture is byte-identical to the plain form.  The emitted code is
 *   identical to v1 (score 61, same hoist of copy2 above the D_80101FB4/FB8
 *   stores that feed it).
 *
 *   Do not retry this by adding more assignments to the pointer, aliasing two
 *   pointers, or laundering the address through a union — cse/copy-prop folds
 *   every constant-valued spelling, and any spelling it CANNOT fold would be an
 *   opacity construct with no semantic purpose (i.e. a cheat by the same test
 *   that rejected region-cast-alias-review-is-cheat.c).
 */
void func_8001C624(void) {
    typedef struct { s32 a, b, c, d; } Blk16;
    typedef struct { s32 a, b, c; } Blk12;
    s32 local[3];
    s32 x;
    s32 y;
    s32 z;
    s32 i;
    void *dst;

    i = 0x36;
    func_80021D10(0, &((s32 *)&D_80101EC8)[i], (s32)D_800A38E0);
    func_80021D10(1, local, (s32)D_800A38E0);
    D_80101FB0 = 0;
    x = ((s32 *)&D_80101EC8)[0x36];
    y = D_80101FA4;
    z = D_80101FA8;
    D_80101FB4 = -0x384;
    D_80101FB8 = 0;
    D_80101FBC = x;
    D_80101FC0 = y - 0x384;
    D_80101FC4 = z;
    D_80101F80 = x;
    D_80101F84 = y;
    D_80101F88 = z;
    dst = &D_80101F90;
    *((Blk16 *)dst) = *((Blk16 *)&D_80101F80);
    dst = &D_801020C0;
    *((Blk12 *)dst) = *((Blk12 *)&D_80101FB0);
    D_80101FCC = 0;
    D_80101FD0 = 0;
    D_80101FD4 = 0;
    dst = &D_80102114;
    *((Blk16 *)dst) = *((Blk16 *)&D_80101FCC);
    local[0] = local[0], local[1] = local[1] - 0x384, local[2] = local[2];
    D_80101FDC = 0;
    D_80101FE0 = 0;
    D_80101FE4 = 0;
    D_80101FEC = 0;
    D_80101FF0 = 0;
    D_80101FF4 = 0;
    D_80101FFC = 0;
    D_80102000 = 0;
    D_80102004 = 0;
    D_8010200C = 0;
    D_80102014 = 0;
    D_80102018 = 0;
    D_8010201A = 0;
    D_80102016 = 0;
    D_80102010 = D_80101F84;
    func_8003FFE0(0);
}
