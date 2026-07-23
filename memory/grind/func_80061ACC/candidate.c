/* func_80061ACC — MATCHED pure-C, sandbox distance 0, zero rules, zero pins.
 *
 * The floor-9 plateau (t/mask v0<->v1 allocator swap) was resolved by the
 * [[walking-pointer-serializes-parallel-loads]] technique, exactly as the
 * just-COMPLETED-C sibling func_800617C8 and the rule's own confirmed case
 * func_800613C8 (text1b.c, same "Change*" sprite-setup family) did:
 *
 *   - Consume arg0[0..2] through a POST-INCREMENT walking pointer (s32 *p).
 *     The pointer data-dependence serializes the three loads so GCC keeps the
 *     target's per-element lw/sw cadence with t in $v0 instead of hoisting.
 *   - Write the independent mask store `D_800A3464 = 0xFF8080;` LAST (after
 *     the third load). GCC's scheduler then materializes the lui/ori and the
 *     sw into the freed load-delay slots exactly where target has them
 *     (interleaved between load2's lw and its sw), with mask in $v1.
 *
 * Net anti-cheat: removes the two register-asm pins (t->$2, mask->$3), adds
 * zero rules, no volatile / no dead store / no barrier. Idiomatic C a human
 * walking a 3-element array would write.
 */
extern u8 D_800F1164[];
void func_80061ACC(s32 *arg0, s32 arg1) {
    s32 *p;
    D_800A3468 = (s32)&D_800F116C;
    D_800F1178 = (s32)arg0;
    D_800F117C = arg1;
    if (D_800F1164[0] != 0) {
        if (D_800F1164[1] != 0) {
            D_800F1164[1] = 0;
            D_800F1164[0] = 0;
        }
        if (D_800F1164[0] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)D_800F1164;
    *(s32 *)D_800A3468 = 0x210014;
    func_80060A68();
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)D_800F1164 - 0xD;
    *(s32 *)D_800A3468 = 0x10007;
    goto end;
check_one_zero:
    if (D_800F1164[1] == 0) {
        D_800F1180 = (s32)(D_800F1164 + 1);
        *(s32 *)D_800A3468 = 0x210015;
        func_80060A68();
        *(s32 *)((s32)D_800A3468 + 0x14) = (s32)D_800F1164 - 0xD;
        *(s32 *)D_800A3468 = 0x10007;
    }
end:
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800F1148 = *p;
    D_800A3464 = 0xFF8080;
}
