/* func_80030580 - best form after session 2. sandbox --disable all = 2 (frame only).
 * Body is byte-identical to the target (pairdiff: only `addiu sp,-8` vs target `-24`).
 *
 * THE RESIDUAL, TYPED (s2): the target's 24 locals bytes are THREE 8-byte combine
 * REG_DEAD-orphan spill slots, not a BLKmode stack temp. Corpus census over 20 src
 * files (tmp/grind/func_80030580/s2/census2.py) found that every leaf function
 * (args=0, regs=0, zero sp accesses) with vars>=16 is built only from `ctx=spill_new`
 * 8-byte slots - display.c's matched get_cs/get_ce are the 2-slot witnesses, each
 * getting one slot per DISTINCT global folded into `lh %lo(SYM)(at)` across a
 * CODE_LABEL. This form produces exactly one such slot, from the SECOND Judge lookup.
 *
 * SESSION 3 SUPERSEDES THAT RECIPE. The generative law, measured by adding and removing
 * whole lookup sites (s3 variants j0read/j1only/base/j3read/j4read):
 *     slots = max(0, number of source-level indexed `(&Judge)[...]` sites - 1)
 * 0/1/2/3/4 sites -> vars 0/0/8/16/24. So the target's 24 bytes correspond to FOUR
 * source-level lookup sites, even though the target's bytes contain exactly two
 * `lui %hi(Judge)` / `lh %lo(Judge)($at)` pairs. The generator is specifically the
 * array-INDEXED fold `(mem (plus (reg idx) (symbol_ref)))`; constant-address global
 * reads add nothing (s3 otherglob/otherglob2). s2's "one slot per DISTINCT global
 * across a CODE_LABEL" reading is refuted by text1a_post:func_80041E10
 * (src/text1a_post.c:465): vars=24, regs=0, args=0, sp_acc=0, three orphan slots, and
 * no branch or label anywhere in its body.
 *
 * The frame IS reachable: s3's dupXZ (four sites) measures vars=24 with three slots -
 * but the duplicates are emitted (+199 fdiff). The open problem is a byte-neutral 3rd
 * and 4th site: CSE deletes a redundant duplicate pseudo and all (dupread, fdiff 0,
 * vars 8), and the only byte-removing pass after combine is jump2 cross-jumping, so the
 * duplicate's merged code has to coincide with insns the target already emits. Cheapest
 * measured duplicate: arms2 (+17 fdiff, vars=16).
 *
 * Additional s3 body-neutral composables: block-scope `extern` redeclarations of any of
 * the three globals; `*(s16 *)((s32)&Judge + (idx * 2))`; `% 0x1000` and `& ~0xF000` /
 * `& ~(-0x1000)` mask spellings; a `(u16)` cast on the first index; a redundant second
 * `&D_8008E194 + arg1 * 7` local; and `judgearr` - declaring `extern s16 Judge[];`
 * TU-wide and indexing `Judge[i]`, which leaves the REST of code6cac_b.c byte-identical
 * (TUdiff = 0).
 *
 * Load-bearing shapes (measured, s1+s2): for-loop with `i++, obj += 0x64` in the header
 * (i++ BEFORE the pointer bump); if/else-if chain (not switch) with the case-1 and
 * case-3 bodies written identically (cross-jumped); vel x,y,z stores then the two +=
 * passes in x,y,z order; `tbl` hoisted into a pointer local (indexing the global
 * directly costs 64 fdiff lines).
 *
 * Body-neutral and free to compose with any future lever (all measured fdiff 0, vars 8
 * in s2): the sibling func_80032064 hand-spelled `/32`; `(s32)*(&Judge + i)`; any
 * declaration order of obj/src/tbl/i; nested ifs instead of `&&`; a while-form loop;
 * `s32 *src` with word indices; an `s32 ang` local for the first index; ARRAY_REF /
 * function-scope-struct COMPONENT_REF / union spellings of the pos->old copy.
 */
s32 *func_80030580(s32 *arg0, s32 arg1) {
    u8 *obj;
    u8 *src = (u8 *)arg0;
    s16 *tbl;
    s32 i;

    obj = (u8 *)&D_80106A78;
    for (i = 0; i < 12; i++, obj += 0x64) {
        if (*(s16 *)(obj + 2) == -1 && *(u8 *)(obj + 0xA) == 0xFF) break;
    }
    *(u8 *)(obj + 0xA) = i;
    *(s16 *)(obj + 2) = arg1;
    *(u8 *)(obj + 7) = 0;
    *(u8 *)(obj + 8) = 0;
    *(u8 *)(obj + 4) = 1;
    *(u8 *)(obj + 6) = *(u16 *)(src + 4);
    *(s32 *)(obj + 0x2C) = *(s32 *)(src + 0xF4);
    *(s32 *)(obj + 0x30) = *(s32 *)(src + 0xF8) - *(s16 *)(src + 0x1A) / 32;
    *(s32 *)(obj + 0x34) = *(s32 *)(src + 0xFC);
    tbl = &D_8008E194 + arg1 * 7;
    *(s32 *)(obj + 0x44) = ((&Judge)[*(u16 *)(src + 0x1CA) & 0xFFF] * tbl[2]) >> 12;
    *(s32 *)(obj + 0x48) = tbl[3];
    *(s32 *)(obj + 0x4C) = ((&Judge)[(*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF] * tbl[2]) >> 12;
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44);
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48);
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C);
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44) / 2;
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48) / 2;
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C) / 2;
    *(Vec3i *)(obj + 0x38) = *(Vec3i *)(obj + 0x2C);
    *(s16 *)(obj + 0x54) = 0;
    *(u16 *)(obj + 0x56) = *(u16 *)(src + 0x1CA);
    *(s16 *)(obj + 0x58) = 0;
    if (tbl[0] == 1) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else if (tbl[0] == 2) {
        *(u16 *)(obj + 0x5C) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    } else if (tbl[0] == 3) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else {
        *(s16 *)(obj + 0x5C) = 0;
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    }
    *(s32 *)(obj + 0x50) = 1;
    *(u8 *)(obj + 5) = 0;
    *(s16 *)obj = 0;
    return (s32 *)obj;
}
