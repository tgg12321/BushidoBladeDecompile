/* func_80061250 — candidate. STATE BANNER (s4, 2026-08-26, recon; HEAD 2b38d6f6):
 * main's committed representation at the START of this session was
 * INCLUDE_ASM("asm/funcs", func_80061250) (asm-until-matched). This file is the
 * LEDGER copy of the form that is APPLIED IN THE WORKING TREE at s4's end
 * (src/text1b.c:3271-3298) and left there for the driver to verify and commit —
 * it is NOT a claim that main already carries it.
 *
 * s4 executed the integration handoff the Judge dispositioned on 2026-08-26
 * (docs/grind/decisions.md:14520; driver scope grant at
 * tools/grinder/scope_allow.txt:34 = src/text1b.c, src/text1b_b.c,
 * undefined_syms_auto.txt). The full staged change is THREE edits, which the
 * Judge's binding constraint (decisions.md:14543) requires to land together:
 *   1. this body replaces the INCLUDE_ASM + the dead `extern volatile u8
 *      D_800F1159;` above it in src/text1b.c;
 *   2. the dead `extern volatile u8 D_800F1159;` in src/text1b_b.c:591 is deleted;
 *   3. the splat symbol line `D_800F1159 = 0x800F1159;` is deleted from
 *      undefined_syms_auto.txt:516,
 * so exactly ONE C handle survives for bytes 0x800F1159-0x800F115A.
 *
 * SPELLING: the object is declared as what the naming census says it is — a byte
 * buffer (`extern u8 D_800F1154[];`, census named_syms.txt:827
 * g_text1b_glyph_buf_b) — and indexed normally. This is the committed house form
 * of this TU (src/text1b.c:3557+3563 func_800619F0; :3505 func_800618B4 over
 * D_800F1152; :3589 over D_800F1164). The banned single-byte pointer-pun
 * `(&D_800F1154)[5]` does not appear anywhere in this body.
 *
 * MEASURED s4 with all three edits in place:
 *   sandbox func_80061250 --disable all → 0 (59/59, rules_dropped 0)
 *   full clean build → SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle
 *
 * 0 regfix/asmfix rules, 0 cheat-asm, 0 volatile, 0 FAKE constructs. Body mirrors
 * COMPLETED-C sibling func_8006156C (src/text1b.c:3370-3397, Match 450f69d1, Judge
 * PASS 2026-07-22): same control shape, same *p++ tail; only the data differs
 * (flags [5]/[6] vs [1]/[2], codes 0x210009/A vs 0x210005/6, mask 0xFF0060 vs
 * 0xFF8080). Self-vet: memory/grind/func_80061250/self_vet.md. */
void func_80061250(s32 *arg0) {
    extern u8 D_800F1154[];
    s32 *v1 = (s32 *)&D_800F116C;
    s32 *p;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if (D_800F1154[5] != 0) {
        if (D_800F1154[6] != 0) {
            D_800F1154[6] = 0;
            D_800F1154[5] = 0;
        }
        if (D_800F1154[5] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)&D_800F1154[5];
    *(s32 *)D_800A3468 = 0x210009;
    goto end;
check_one_zero:
    if (D_800F1154[6] == 0) {
        D_800F1180 = (s32)&D_800F1154[6];
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
