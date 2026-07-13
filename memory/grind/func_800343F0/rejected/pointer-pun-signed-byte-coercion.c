/* REJECTED — byte-correct but a CHEAT (Judge-flagged, re-audit patrol 2026-07).
 *
 * This was HEAD's form (committed 38101f31, 2026-03-28). It reaches sandbox 0,
 * so it is NOT rejected on bytes — it is rejected on policy, permanently.
 *
 *     extern u8 D_80102787;                       // include/code6cac.h:409
 *     ...
 *     s32 val_87 = *(s8 *)&D_80102787;            // src/code6cac_b.c:3952
 *
 * Why it is a cheat: take the address of a global, cast it to a different
 * pointer type, read through it — solely to coerce GCC's instruction selection
 * (lb instead of lbu + sll 24 + sra 24). Same structural intent as the banned
 * `*(volatile T *)&G` family. It leaves the header DECLARATION LYING (u8 when
 * the original was signed) and fixes the lie at one use site instead of at the
 * ground truth. Fails prongs (c) and (d) of header-type-correction-from-use-sites.
 *
 * Do NOT re-propose. The clean replacement (header retype + plain read) is in
 * ../candidate.c and also reaches distance 0 — the pun buys nothing.
 */

void func_800343F0(void) {
    s8 val_85 = (s8)D_80102785;
    s8 val_86 = (s8)D_80102786;
    s8 val_84 = (s8)D_80102784;
    s32 val_87 = *(s8 *)&D_80102787;   /* <-- the cheat */

    D_800A36F6 = 0;
    D_800A38DC = val_85;
    D_800A38BA = val_86;
    D_800A3140 = val_87;
    D_800A36A4 = val_84;
    player_SetCharId(0, 0);
    player_SetCharId(1, 0);
    D_800A376A = 0;
    D_800A376B = 0;
    D_800A380C = 0;
    D_800A38D4 = 2;
    D_800A37D3 = 0;
    D_800A37D2 = 0;
}
