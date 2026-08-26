/* func_80061250 — candidate. STATE BANNER (s2, 2026-08-26, session HEAD 95de1421):
 * this body was APPLIED to src/text1b.c:3271-3299 during session s2, replacing
 * `extern volatile u8 D_800F1159;` + `INCLUDE_ASM("asm/funcs", func_80061250);`.
 * On main at HEAD 95de1421 the committed representation is INCLUDE_ASM (the s2
 * working-tree edit is uncommitted pending driver verification); this file is the
 * ledger copy, not a claim about the committed tree.
 * MEASURED s2 (2026-08-26): `sandbox func_80061250 --disable all` == 0 (59/59).
 * MEASURED s1 re-run (2026-08-26, HEAD 351b99be): sandbox 0 AND full clean
 * `build` SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (oracle MATCH).
 * 0 regfix/asmfix rules, 0 cheat-asm, 0 volatile, 0 FAKE constructs.
 * Prior layer-1 FAIL (2026-08-26 14:56) was CITATION-ONLY: the construct was
 * verified legitimate; the self-vet's sibling line numbers had drifted. s2's
 * self_vet.md carries post-application re-grepped citations
 * (sibling func_8006156C body = src/text1b.c:3370-3397, flag test at 3375,
 * Match commit 450f69d1).
 * Body mirrors COMPLETED-C sibling func_8006156C (Match 450f69d1, Judge PASS
 * 2026-07-22 21:27): same flag-block base-offset spelling, same *p++ tail;
 * only data differs (flags [5]/[6] vs [1]/[2], codes 0x210009/A vs 0x210005/6,
 * mask 0xFF0060 vs 0xFF8080). */
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
