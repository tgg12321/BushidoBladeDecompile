/* func_80061250 — candidate, APPLIED to src/text1b.c (s1 re-run, 2026-08-26).
 * PROVEN THIS SESSION at HEAD 351b99be: `sandbox func_80061250 --disable all` == 0
 * (59/59 insns) AND full clean `build` SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa
 * (oracle MATCH); 0 regfix/asmfix rules, 0 cheat-asm, 0 volatile, 0 FAKE constructs.
 * NOTE: the earlier discarded s1's claim that the sandbox floor was artifact-locked at 2
 * (named-symbol reloc addend D_800F1154+5 vs target D_800F1159+0) did NOT reproduce —
 * the scorer prints 0 with this exact body. Do not re-derive that analysis.
 * Body mirrors COMPLETED-C sibling func_8006156C (src/text1b.c, Judge PASS
 * 2026-07-22 21:27): same flag-block base-offset spelling, same *p++ tail. */
extern u8 D_800F1154;
void func_80061250(s32 *arg0) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 *p;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if ((&D_800F1154)[5] != 0) {
        if ((&D_800F1154)[6] != 0) {
            (&D_800F1154)[6] = 0;
            (&D_800F1154)[5] = 0;
        }
        if ((&D_800F1154)[5] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)((&D_800F1154) + 5);
    *(s32 *)D_800A3468 = 0x210009;
    goto end;
check_one_zero:
    if ((&D_800F1154)[6] == 0) {
        D_800F1180 = (s32)((&D_800F1154) + 6);
        *v1 = 0x21000A;
    }
end:
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800F1148 = *p;
    D_800A3464 = 0xFF0060;
}
