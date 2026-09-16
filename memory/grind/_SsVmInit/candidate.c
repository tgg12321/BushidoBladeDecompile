/* _SsVmInit candidate — s4 (permuter modality; primarily manual hypothesis
 * testing this session, not an automated campaign — see s4 notes below).
 *
 * CHASSIS DISCONTINUITY (s4, load-bearing for every prior session's
 * conclusions): the CHASSIS CHECK at s4 dispatch found HEAD honest floor
 * mismatched the ledger's recorded floor 19. Applying this exact candidate
 * body (unchanged since s2/s3) to HEAD and measuring fresh gave
 * target_insns=200, build_insns=193, score=38 — NOT target_insns=174/score=19
 * as every prior ledger entry recorded. The target instruction COUNT itself
 * changed (174 -> 200), which cannot come from any C edit on our side — this
 * is a toolchain/asm/scoring-side change made between s3 and s4 (candidate
 * dates to s2/s3; no commits since then touched src/main.c for this
 * function). Every s2/s3 KILLED verdict recorded against the "floor 19"
 * chassis (u16-masking the per-voice loop compare/shift; hoisting the shared
 * 0xFF constant) is VOID against the current chassis and was RE-MEASURED
 * this session — see hypotheses.md s4 entries. Do not cite the s2/s3 kills
 * without re-verifying against a fresh sandbox run first.
 *
 * s4 RESULT: re-measuring from the fresh 38/200 baseline and re-testing
 * (not re-deriving from scratch) the same two families the s2/s3 kills had
 * ruled out — u16-typing the loop counter, and hoisting the shared 0xFF
 * constant — both CONFIRMED as wins on the new chassis (the opposite verdict
 * from s2/s3), plus three new structural fixes (branch-sense flip on the a0
 * clamp if/else, dropping the `maxVoice` cache local so `_SsVmMaxVoice` is
 * read directly at both use sites like target does, and moving `i = 0;`
 * earlier to before the `buf[1] = 0x60093;` field-init block) dropped the
 * score 38 -> 3 (target_insns 200 == build_insns 200 exactly, 0 source-level
 * hunks in `sandbox --diff`). This is the lowest floor ever recorded for
 * this function. See hypotheses.md s4 for each isolated measurement.
 *
 * REMAINING RESIDUAL AT s4 (score 3): `sandbox --diff` shows exactly ONE
 * register-allocation tie, 2 real (scored, operand-only) instructions —
 * the `(u8)a0 >= 0x18` clamp comparison/store — where target keeps the
 * masked parameter value in `$a0` (`andi a0,s1,0xff` / `sb a0,0(at)`) and
 * ours keeps it in `$v0` (`andi v0,s1,0xff` / `sb s1,0(at)`). Every ordinary
 * C respelling tried this session (explicit `u8` temp, self-truncating
 * `a0 = (u8)a0;`, swapping the if/else branch order back) measured flat or
 * WORSE — see hypotheses.md s4. A directed permuter campaign was ATTEMPTED
 * (tools/decomp-permuter/import.py against src/main.c + asm/funcs/_SsVmInit.s,
 * workspace at tmp/grind/_SsVmInit/s4/perm_ws/) but the imported base.c does
 * not compile standalone — main.c's pre-declaration-order duplicate extern
 * pattern (the same forward-declared-before-canonical-type shape s2 fixed
 * for THIS function's own body) produces `conflicting types for` errors
 * against OTHER main.c functions' declarations when the whole TU is pruned
 * and reassembled by the permuter's prune step, and maspsx then throws
 * `too many values to unpack` trying to assemble the resulting malformed
 * output. This blocked the campaign before any iteration ran; see
 * tmp/grind/_SsVmInit/s4/perm_ws/campaign.log and the compile.sh direct-run
 * output captured in hypotheses.md s4. NEXT SESSION: fixing the import
 * (likely `--no-prune`, or hand-trimming base.c to drop the conflicting
 * unrelated declarations) is the concrete next step before another permuter
 * attempt; hand-derivation of the register tie (read local-alloc.c's
 * `find_reg`/allocno priority for a parameter-homed pseudo vs a fresh v0
 * temp) is the alternative un-tried avenue.
 *
 * Apply verbatim in place of `INCLUDE_ASM("asm/funcs", _SsVmInit);` in
 * src/main.c. Several of these externs duplicate declarations already in
 * scope LATER in the same TU (main.c) with identical types — harmless
 * per C extern-redeclaration rules, and REQUIRED here because this
 * function sits textually BEFORE those later declarations.
 */
extern s32 MarioCam_str[2];
extern s16 D_800F4E18;
extern s16 D_800F4E1C;
extern s8  D_800F4E35;
extern u16 D_800F1B10;
extern u16 D_800F1B12;
extern u16 D_801078D8;
extern u8  _svm_auto_kof_mode;
extern s16 kMaxPrograms;
extern u16 _svm_vab_count;
extern s16 D_800F4E20;
extern s8  D_800F4E22;
extern s16 D_800F4E24;
extern s16 D_800F4E36;
extern s16 D_800F4E38;
extern s16 D_800F4E3A;
extern s16 D_800F4E3C;
extern s16 D_800F4E3E;
extern s16 D_800F4E42;
extern s16 D_800F4E44;
extern s16 D_800F4E46;
extern s16 D_800F4E48;
extern s16 D_800F4E4A;
extern u16 D_8010280A;
extern u16 D_800F1B14;
extern u16 D_800F2B68;

void _SsVmInit(s32 a0) {
    s32 buf[16];
    u16 i;
    s32 offset;

    _spu_setInTransfer(0);
    D_800F66F8 = 0;
    SpuInitMalloc(0x20, MarioCam_str);

    i = 0;
    do {
        *(s16 *)((u8 *)D_80102A78 + i * 2) = 0;
        i++;
    } while (i < 0xC0);
    i = 0;
    do {
        D_800F65E0[i] = 0;
        i++;
    } while (i < 0x18);
    _svm_vab_count = 0;
    i = 0;
    do {
        _svm_vab_used[i] = 0;
        i++;
    } while (i < 0x10);

    if ((u8)a0 >= 0x18) {
        _SsVmMaxVoice = 0x18;
    } else {
        _SsVmMaxVoice = a0;
    }

    buf[1] = 0x60093;
    i = 0;
    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;

    if (_SsVmMaxVoice != 0) {
        s16 ff = 0xFF;
        do {
            s32 idx = i;
            offset = ((idx * 8 - idx) * 4 - idx) * 2;
            *(s16 *)((u8 *)&D_800F4E1A + offset) = 0x18;
            *(s16 *)((u8 *)D_800F4E28 + offset) = -1;
            *(s16 *)((u8 *)&D_800F4E18 + offset) = ff;
            *(s8  *)((u8 *)&D_800F4E35 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E1C + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E1E + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2A + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2C + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2E + offset) = ff;
            *(s16 *)((u8 *)&D_800F4E20 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E24 + offset) = 0;
            *(s8  *)((u8 *)&D_800F4E22 + offset) = 0x40;
            *(s16 *)((u8 *)&D_800F4E36 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E38 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E3A + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E3C + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E42 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E44 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E46 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E48 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E4A + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E3E + offset) = 0;
            offset = 1;
            buf[0] = offset << i;
            func_8008B488(buf);
            D_8010280A = i;
            _SsVmKeyOffNow(1);
            i = i + 1;
        } while (i < _SsVmMaxVoice);
    }

    _svm_rattr_plus_0x8 = 0x3FFF;
    _svm_rattr_plus_0xA = 0x3FFF;
    D_800F1B10 = 0;
    D_800F1B12 = 0;
    D_801078D8 = 0;
    D_800F1B14 = 0;
    D_800F2B68 = 0;
    _svm_rattr = 0;
    _svm_rattr_plus_0x4 = 0;
    _svm_auto_kof_mode = 0;
    _svm_stereo_mono = 0;
    kMaxPrograms = 0x80;
    _SsVmFlush();
}
