/* func_80062020 (src/text1b.c) — s14 (synthesis, 2026-09-03) — DISTANCE 0, ORACLE SHA1 MATCH,
 * DISPOSITION PENDING A RULING.  DO NOT COMMIT until the ruling question in the s14 outcome
 * JSON / evidence.md is answered.
 *
 * MEASURED THIS SESSION, this body pasted over `INCLUDE_ASM("asm/funcs", func_80062020);`
 * at src/text1b.c:3932:
 *     sandbox func_80062020 --disable all -> score 0, target_insns 38, build_insns 38,
 *         rules_dropped 0, cheat_asm_stripped 165
 *     verify-oracle -> ok: true, build_sha1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa
 *         == original_sha1_locked (full EXE byte-identical)
 * src/text1b.c was restored to HEAD immediately afterwards.
 *
 * WHAT IS NEW.  Every previous distance-0 form for this function reached the target's
 * second address materialisation with a DEAD statement (a re-assignment of the row pointer
 * to the value it already held) or with the same lvalue spelled two ways inside ONE function
 * body — both banned (state.json banned_constructs; Judge 2026-08-25, 2026-08-31; layer-1
 * 2026-08-30, 2026-08-31).  This body has NEITHER.  There is no dead statement anywhere, and
 * NO function body in it contains two spellings of one lvalue: `bb2_clr_pay` holds exactly one
 * address expression and uses it twice, `bb2_clr_flag` holds exactly one address expression and
 * uses it once, and `func_80062020` itself holds none.  No /* FAKE */ construct is present and
 * none is needed.
 *
 * MECHANISM (matches H-s13-8's law exactly).  flow.c:2102 gives combine a LOG_LINK only for a
 * def that dies at its single use in the block, so the target arrangement
 * DISP8 | DISP4 | LOSUM[D_800F1198] requires ONE address def with TWO uses plus a SECOND def of
 * the same value with ONE use.  GCC 2.7.2 inlines each `static __inline__` body with its own
 * address computation, and cse2 does not unify the two, so the required 2-use + 1-use pair
 * exists without any dead code.  s13 proved no UNIFORM spelling can produce that pair
 * (165 shapes); the helper boundary is the first construct measured that produces it honestly.
 *
 * THE OPEN QUESTION (why this is not submitted as candidate-ready).  A `static __inline__`
 * helper used once is not in the frozen SOTN-sanctioned family list and not in the forbidden
 * catalog — a first reach.  It passes T4 and T6 cleanly; T1/T2/T3 are arguable in both
 * directions (see the s14 outcome JSON's ruling_question).  In-repo precedent for the SYNTAX in
 * byte-matched code: src/main.c:1069 (`static inline void vmSetStartAddr`) and src/main.c:2187
 * (`static inline void _memcpy`).  No SOTN-master precedent: `docs/reference/sotn-construct-index.md`
 * has zero non-asm `inline` entries.
 *
 * VARIANT (also distance 0 this session, tmp/grind/func_80062020/s14/body_E2.c): a SINGLE helper
 * `bb2_clear_terminator(ofs)` with the payload written inline through a row pointer in the
 * caller.  It reads more naturally but it IS the banned statement sequence with a function
 * boundary inserted before the last store, so it is the WEAKER form to ask about; this
 * two-helper form is the one the question is about.
 */
static __inline__ void bb2_clr_pay(s32 ofs) {
    s32 *q = (s32 *)((u8 *)&D_800F1198 + ofs);
    q[2] = 0;
    q[1] = 0;
}

static __inline__ void bb2_clr_flag(s32 ofs) {
    *(s32 *)((u8 *)&D_800F1198 + ofs) = 0;
}

void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
        *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 4);
        i = i + 1;
        *(s32 *)((u8 *)&D_800F119C + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 8);
        *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    ofs = i + i;
    ofs = ofs + i;
    ofs = ofs << 2;
    bb2_clr_pay(ofs);
    bb2_clr_flag(ofs);
}
