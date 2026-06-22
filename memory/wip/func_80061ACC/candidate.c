/* func_80061ACC — best pure-C floor (masked-distance 9, no rules, no pins).
 *
 * HEAD form pins t->$2 and mask->$3 (cheat per [[register-asm-pins]]). With
 * pins stripped, GCC's natural allocator emits the SAME instruction schedule
 * as target byte-for-byte EXCEPT the v0/v1 registers are swapped on every
 * load/mask insn (9 such insns -> masked Levenshtein = 9). 8 pure-C variants
 * explored this session (decl order, block-local scope, mask split high|low,
 * mask early, mask top-of-fn initialiser, inline-arg0[] no t, u8 *p alias)
 * all land at >= floor 9 (two worse: no-t -> 11, p-alias -> 33).
 *
 * Sibling 80061710 already saved the same shape with PASS cheat-review (see
 * memory/wip/func_80061710/candidate.c). This function is identical family.
 * Saved so the next agent resumes from the clean no-pin form, not HEAD.
 */
extern u8 D_800F1164[];
void func_80061ACC(s32 *arg0, s32 arg1) {
    s32 t;
    s32 mask;
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
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0xFF8080;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
