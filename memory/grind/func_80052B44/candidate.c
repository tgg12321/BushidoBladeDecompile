/* func_80052B44 — PROPOSED canonical-body finish (CONTINGENT on ruling-request s1).
 * Do NOT apply to src/text1b.c until the Judge/owner authorizes canonical-body
 * (COMPLETED-INLINE-ASM-CANONICAL) + the inline_asm_canonical.txt entry lands.
 *
 * Identity: LIBGTE-style SetRotMatrix + zero-translation. Loads a packed 3x3
 * rotation matrix (5 s32 words) from *a0 into GTE controls CR0-CR4, then zeroes
 * the translation vector CR5-CR7 (TRX/TRY/TRZ), the last ctc2 in the jr-ra
 * delay slot. All cop2 + mechanical load packaging; zero general-purpose
 * computation. Prologue is instruction-identical to authorized canonical-body
 * func_8007ED6C (verbatim-linked Sony PsyQ 4.0 LIBGTE object, display.c).
 *
 * Replaces HEAD's 5 register-asm pins ($8-$12) + 1 fill_delay regfix rule.
 * Spelling follows func_8007ED6C (src/display.c:2257-2284): decimal memory
 * offsets (maspsx parses 0x0($a0) as base-10), .set directives duplicated in
 * TAB and SPACE form per maspsx-noreorder-stripping.
 */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052B44\n"
    "    lw     $t0, 0($a0)\n"
    "    lw     $t1, 4($a0)\n"
    "    lw     $t2, 8($a0)\n"
    "    lw     $t3, 12($a0)\n"
    "    lw     $t4, 16($a0)\n"
    "    ctc2   $t0, $0\n"
    "    ctc2   $t1, $1\n"
    "    ctc2   $t2, $2\n"
    "    ctc2   $t3, $3\n"
    "    ctc2   $t4, $4\n"
    "    ctc2   $zero, $5\n"
    "    ctc2   $zero, $6\n"
    "    jr     $ra\n"
    "    ctc2   $zero, $7\n"
    "endlabel func_80052B44\n"
);
