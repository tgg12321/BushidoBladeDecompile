/* REJECTED -- s4 (2026-09-07, permuter modality).
 * Two OTHER spellings of the loop-1 index add, both measured NO-CHANGE against
 * the s3 body (31 normalized diffs each, versus 29 for the accepted
 * `((u8 *)p + i)[0x17]` form). Banked so a later session does not re-try them:
 *     ((u8 *)p)[i + 0x17] = *src;        -- commuting the operands in the
 *         subscript expression does NOT commute the emitted `addu`; combine
 *         canonicalises the PLUS before the operand order is fixed.
 *     (((u8 *)p) + 0x17)[i] = *src;      -- folding the constant into the
 *         pointer and subscripting by `i` also leaves `addu $v1, $a3, $t0`.
 * Only "pointer + index, then a CONSTANT subscript" flips the add to the
 * target's `addu $v1, $t0, $a3`. The file below carries the first of the two;
 * the second is the one-line variant noted above.
 */
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;

extern u8 D_80106A73;
extern s32 D_80106A58;
extern s32 *func_80077D00(void);

void func_80035280(void) {
    s32 *p;
    u8 *f;
    u8 *src;
    u8 *base;
    s32 i;
    s32 flags;
    s32 flags0;
    s32 flags1;
    s32 flags2;

    p = func_80077D00();
    i = 0;
    f = &D_80106A73;
    src = f - 3;
    flags = p[8];
    flags0 = (flags & ~1) | (src[3] & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (src[3] & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (src[3] & 4);
    p[8] = flags2;
    for (; i < 3; i++) {
        ((u8 *)p)[i + 0x17] = *src;
        ((u8 *)p)[i + 0x1D] = *src;
        src++;
    }
    base = (u8 *)&D_80106A58;
    for (i = 0; i < 3; i++) {
        ((u8 *)p)[i * 4 + 0x21] = *(s32 *)(base + i * 8 + 4) / 1800;
        ((u8 *)p)[i * 4 + 0x22] = (*(s32 *)(base + i * 8 + 4) / 30) % 60;
        ((u8 *)p)[i * 4 + 0x23] = (*(s32 *)(base + i * 8 + 4) % 30) * 100 / 30;
        ((u8 *)p)[i * 4 + 0x24] = base[i * 8];
    }
}
