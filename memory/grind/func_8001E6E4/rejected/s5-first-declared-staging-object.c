/* REJECTED (s5, permuter modality) — "first-declared genuinely-used 8-byte
 * object" chassis family. These are the ONLY honest shapes that reproduce the
 * target's FULL frame layout (vars=80 AND `local` at sp+0x18, saves at
 * 0x60-0x6C), because GCC 2.7.2 allocates declared locals in DECLARATION
 * ORDER with the first-declared local getting the LOWEST sp offset. They are
 * rejected because a genuinely-used 8-byte object necessarily emits memory
 * traffic that the target does not have (target never touches sp+0x10..0x17).
 *
 * Measured (tmp/grind/func_8001E6E4/s5/screen.py, Makefile-mirror pipeline):
 *   pair_first     (Pair2 q declared first, staged into local.vx/vy):
 *                  frame 112, vars=80, 71 insns, objdump diff 14 insn pairs
 *                  (base honest wp chassis is 19 pairs) — best honest form yet
 *                  by raw diff, but the q stores/reloads replace target's two
 *                  scheduling nops + one lui/lw pair, so it is NOT the target.
 *   arr_first_used (s32 t[2] declared first, written then read): identical
 *                  metrics (frame 112, 71 insns, 14 pairs).
 *   rot4_first     (8-byte Rot4 aggregate copy declared first): frame 112 but
 *                  76 insns (aggregate copy materializes) — worse.
 * Two permuter campaigns seeded from these chassis (s5-pair-first: 25 min,
 * ~14k iters, best score 16 but that find is SEMANTICALLY INVALID — it
 * repoints `wp` at `q` so the first callee receives the wrong buffer;
 * s5-arr-first-used: 18 min, 40409 iters, best 108, flat across two windows)
 * found no honest score-0.
 */
void func_8001E6E4(s32 arg0) {
    Pair2 q;                 /* first-declared 8 bytes -> lands at sp+0x10 */
    CamWork *wp;
    CamWork local;           /* now at sp+0x18, exactly as target */
    s32 *s2;

    s2 = (s32 *)&D_800F5328;
    if ((u32)(arg0 - 0x555) >= 0x556U) {
        s2 = (s32 *)&D_800F6608;
    }
    q.a = s2[0] + D_800FF5C8;   /* <- these two stores are the divergence: */
    q.b = s2[1] + D_800FF5CC;   /*    target has no store into sp+0x10/14  */
    local.vx = q.a;
    local.vy = q.b;
    local.vz = s2[2] + D_800FF5D0;
    local.rx = *(u16 *)((u8 *)s2 + 0x10) + (u16)D_800FF5D8;
    wp = &local;
    local.ry = *(u16 *)((u8 *)s2 + 0x12) + (u16)D_800FF5DA;
    local.rz = *(u16 *)((u8 *)s2 + 0x14) + (u16)D_800FF5DC;
    local.dist = *(s32 *)((u8 *)s2 + 0x18) + D_800FF5E0;
    func_80046BF4((s32 *)wp, &local.rx, local.dist);
    {
        s32 *p20 = (s32 *)((u8 *)s2 + 0x20);
        func_8001A538((s32 *)&local, p20);
        func_80061064((s32 *)&local.rx, p20);
    }
    D_800A36B4 = (s32)s2;
}
