/* REJECTED (s2 structural, 2026-07-23) — single u16 read + (s16) value cast.
 * sandbox --disable all: score 0, build_insns 107 == target 107.  *** FULL BYTE MATCH ***
 * This is the FIRST distance-0 pure-C form ever found for func_8001F938 (prior best was
 * floor 2 with build_insns 108, i.e. +1 insn, never a true match). It proves the function
 * IS byte-closable in pure C — but ONLY via the pre-banned signedness-split family.
 *
 * FRESH adversarial cheat-reviewer: FAIL (Tests 1/2/3/4/5). It is a third respelling of the
 * dual-typed same-address read the ledger's judge_constraints pre-banned "any spelling".
 * Verbatim tell (reviewer Test-1): `(X<<16)>>15` erases the low-16-bit signedness distinction,
 * so `raw_or_3 = u` (unsigned) and `raw_or_3 = (s16)u` produce byte-identical OUTPUT for every
 * field value; the u16 read exists only to hand GCC a second typed view so it re-materializes a
 * distinct lhu load. Codegen effect, not program-behavior effect. Sibling FAILs:
 * dual-type-probe-load.c (floor 2), guarded-dual-typed-read.c (floor 5).
 *
 * MECHANISM: ONE C memory dereference (u16, the field's faithful storage type per sibling
 * func_80027438) + a `(s16)u` value cast for the signed `<4` compare. GCC 2.7.2 materializes
 * the signed interpretation via a SECOND lh load (cheaper than in-register sll;sra; fills the
 * load-delay slot) while keeping lhu for the index -> exactly target's .L8001FA60:
 *     lh $v0,0x270 ; lhu $v1,0x270 ; slti $v0,$v0,4 ; bnez ; sll $v0,$v1,16 ; li $v1,3 ;
 *     sll $v0,$v1,16 ; sra $v0,$v0,15
 *
 * WHY NOT A CLEAN RETYPE (F2 remedy): +0x270 is a struct-offset access, not a named global,
 * so there is no header typedef to correct. AND the compare is genuinely SIGNED (target slti/lh)
 * while the index is unsigned — the field is inherently read two ways in target. A u16 field
 * signed-compared in C would emit sltiu, not slti; the `(s16)` cast is unavoidable to match. So
 * this is a genuine dual-view requirement, not a mistyped-global cleanup.
 *
 * DISPOSITION: subject of the s2 ruling-request. src/ kept at clean floor-8. Do NOT ship.
 */
    {
        s32 u = *((u16 *)(arg0 + 0x270));
        s32 raw_or_3;
        if (((s16)u) >= 4) {
            raw_or_3 = 3;
        } else {
            raw_or_3 = u;
        }
        idx = ((raw_or_3 << 16) >> 15);
    }
