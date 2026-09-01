/* candidate.c — func_800770B8 (src/text1b.c) — s2 structural, 2026-09-01
 * Honest floor THIS form, measured s2: sandbox --disable all = 10 (174/175 insns).
 * (s1 form measured 14 on the same chassis at the start of s2; s2 dropped it 14 -> 12 -> 10.)
 * 100% ordinary C, zero cheats, zero annotations, no FAKE-annotated constructs.
 *
 * s2 additions over the s1 form (both ordinary C, both measured):
 *   1. Tail store grouping: the `*(s32 *)D_800A36A0 = arg1;` and
 *      `*(s8 *)(D_800A36A0 + 0x65) = 0;` stores share ONE read of the global
 *      through a local `u8 *q` (14 -> 12). Without it GCC reloads D_800A36A0
 *      between the two stores (the pointer store may alias the global), giving
 *      2 extra insns (lw + load-delay nop) that the target does not have.
 *      Target's grouping is exactly {+0, +0x65} | {+0x67} | {lbu +0x67, sb +0x66},
 *      so only THIS pair is grouped; s1's K2 (grouping the whole tail) is still dead.
 *   2. `a2 = 0;` moved from the outer-loop preheader + outer-loop tail into the
 *      FIRST statement of the outer loop body (12 -> 10). This sinks the
 *      `addu $a2,$zero,$zero` from sched1 slot 30 to slot 37 (after the
 *      0x30/0x34 store cluster) exactly as target, and drops the duplicate
 *      trailing `a2 = 0;`. Rows 30-34 and 37 are now byte-clean.
 *
 * NOTE: applying this body also requires the two caller-side edits (already applied
 * in src/text1b.c by this session): the prototype near the caller becomes
 * `s32 func_800770B8(s32, s32, s32);` and the call site passes
 * `(s32)&D_8009BD24` (was `(s32 *)&D_8009BD24`). Byte-neutral for the caller.
 *
 * Residual at floor 10 (4 classes, all mapped — see evidence.md s2):
 *   A. prologue rows 7-12: `sw $ra` / `sw $s1` save order + ClearOTagR a1-first
 *      arg evaluation + `addiu $s1,$s0,0x58` placement (sched1 block-0 order).
 *   B. rows 35-36: the 0x30/0x34 stores go through $s1 (p_old) in ours, through
 *      $v0 (the raw call-result pseudo) in target. Second-handle spellings KILLED
 *      twice (K1 s1, K4 s2).
 *   C. rows 62-64: p_6a/p_7e base addu dest register (local-alloc coalesce).
 *   D. row 104: reorg delay-slot fill choice — mechanism PROVEN in s2, see
 *      evidence.md; the target build's fall-through steal was blocked.
 */
s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {
    u16 sp[2];
    s32 *p_old;
    s32 r;
    s16 t0;
    s16 a2;

    p_old = (s32 *)(arg0 + 0x58);
    sp[0] = 0;
    sp[1] = 0;
    ClearOTagR(D_800A374C, 0x1008);
    D_800A35D8 = arg0;
    snd_StopAll();
    func_8006E950(6, p_old);
    r = func_80076FF8(p_old);
    {
        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        *(s32 *)((u8 *)p_old + 4) = (s32)prev;
        *(s32 *)(D_800A36A0 + 0x30) = 0;
        *(s16 *)(D_800A36A0 + 0x34) = 0;
    }
    t0 = 0;
    do {
        u8 *base = D_800A36A0;
        u8 *ptr;
        a2 = 0;
        ptr = (u8 *)((t0 * 2) + (s32)base);
        *(s16 *)(ptr + 0x10) = 0;
        *(s16 *)(ptr + 0x8) = 0;
        *(s16 *)(ptr + 0xC) = 0;
        *(s16 *)(ptr + 0x14) = 0;
        *(s16 *)(ptr + 0x3C) = 0;
        ptr = (u8 *)&D_800A35D0;
        ptr = (t0 * 4) + ptr;
        *(s16 *)(ptr + 2) = 0;
        *(s16 *)(ptr + 0) = 0;
        ptr = base + (t0 * 4);
        *(s16 *)(ptr + 0x42) = 0;
        *(s16 *)(ptr + 0x40) = 0;
        *(u8 *)(base + t0 + 0x68) = (u8)t0;
        {
            s16 *p_6a = (s16 *)(D_800A36A0 + 0x6A + (t0 * 10));
            s16 *p_7e = (s16 *)(D_800A36A0 + 0x7E + (t0 * 10));
            do {
                p_6a[a2] = -1;
                p_7e[a2] = 0;
                a2 = (s16)(a2 + 1);
            } while (a2 < 5);
        }
        a2 = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x5C) = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x60) = 5;
        for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) {
            s16 idx = (s16)(a2 + (t0 * 10));
            s32 mask = 1 << idx;
            (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] & 0xF2);
            if ((arg2 & mask) != 0) {
                (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] | 1);
                sp[t0] += 1;
            }
        }
        t0 = (s16)(t0 + 1);
    } while (t0 < 2);
    {
        u8 *p = D_800A36A0;
        *(s32 *)(p + 0x20) = 0;
        *(s32 *)(p + 0x1C) = 0;
        if ((s16)sp[0] < (s16)sp[1]) {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[0] - 3);
        } else {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[1] - 3);
        }
    }
    if (*(u8 *)(D_800A36A0 + 0x64) >= 3) {
        *(u8 *)(D_800A36A0 + 0x64) = 2;
    }
    {
        u8 *q = D_800A36A0;
        *(s32 *)q = arg1;
        *(s8 *)(q + 0x65) = 0;
    }
    *(u8 *)(D_800A36A0 + 0x67) = 1;
    *(u8 *)(D_800A36A0 + 0x66) = (&D_8009BD21)[*(u8 *)(D_800A36A0 + 0x67) * 2];
    D_800A35DC = 1;
    return 1;
}
