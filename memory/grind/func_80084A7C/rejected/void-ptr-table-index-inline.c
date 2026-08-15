/* REJECTED (pre-session-1 WIP, found uncommitted 2026-08-14; owner-reviewed)
 * score 39 vs HEAD's 26 (disable=all, 146 vs 145 insns) -- REGRESSION.
 * Form: void** table handle + inlined `offset + (s32)tbl[a0] + 0x98` at every
 * flag site, dropping HEAD's `s32 *base_ptr` CSE handle and the a0<<16>>14
 * sign-extension idiom. Re-materialising tbl[a0] at each of the 8 flag sites
 * costs more than it saves; the cached base_ptr form is strictly better.
 * Do not re-derive.
 */
/* kengo:MED  |  sa_tan4/saTan4GaugeInit  |  66i */
void func_80084A7C(s16 a0, s16 a1) {
    void **tbl = (void **)&D_80106F28;
    s32 offset = a1 * 0xB0;
    u8 *base = (u8 *)((s32)tbl[a0] + offset);
    s32 val;
    u32 threshold;

    val = base[0x21] + 1;
    threshold = base[0x20];
    base[0x21] = val;

    if (threshold == 0) {
        *(s32 *)(base + 0x88) = 0;
        base[0x1C] = 0;
        *(s32 *)(base + 0x90) = 0;
        if (*(s32 *)(offset + (s32)tbl[a0] + 0x98) & 0x400) {
            *(s32 *)base = *(s32 *)(base + 0xC);
        } else {
            *(s32 *)base = *(s32 *)(base + 4);
        }
        return;
    }

    if ((u8)val < threshold) {
        *(s32 *)(base + 0x88) = 0;
        base[0x1C] = 0;
        *(s32 *)(base + 0x90) = 0;
        if (*(s32 *)(offset + (s32)tbl[a0] + 0x98) & 0x400) {
            *(s32 *)base = *(s32 *)(base + 0xC);
            *(s32 *)(base + 8) = *(s32 *)(base + 0xC);
        } else {
            *(s32 *)base = *(s32 *)(base + 4);
            *(s32 *)(base + 8) = *(s32 *)(base + 4);
        }
        return;
    }

    *(s32 *)(offset + (s32)tbl[a0] + 0x98) &= ~1;
    *(s32 *)(offset + (s32)tbl[a0] + 0x98) &= ~8;
    *(s32 *)(offset + (s32)tbl[a0] + 0x98) &= ~2;
    *(s32 *)(offset + (s32)tbl[a0] + 0x98) |= 0x200;
    *(s32 *)(offset + (s32)tbl[a0] + 0x98) |= 4;
    base[0x14] = 0;

    if (*(s32 *)(offset + (s32)tbl[a0] + 0x98) & 0x400) {
        *(s32 *)(base + 8) = *(s32 *)(base + 0xC);
    } else {
        *(s32 *)(base + 8) = *(s32 *)(base + 4);
    }

    if (base[0x22] != 0xFF) {
