# Evidence bank — func_8001B138

- Audit diagnosis (regressions.md): Two codegen-control constructs fail the human-programmer test (test #2) and the semantic-purpose test (test #1): (1) a typed s16 intermediate used to clamp a u16 global — `{ s16 tmp = -0x1C00; g_file_vram_timer = tmp; }` — has no semantic difference from `g_file_vram_timer = -0x1C00;` and the SOTN named-intermediate sanction does not cover typed-literal coercion; (2) an explicit `(s32)` cast on a 32-bit AND mask — `& (s32)0xFFFEFFFE` — changes the RTL type of the expression (SI vs USI) without changing the MIPS `and` instruction or the runtime result, and a human programmer would write `& ~0x10001` or the unsigned literal without a signed cast. Both constructs change how GCC 2.7.2 sees the type of a computation in order to influence scheduling or RTL analysis, with no observable semantic effect. Next action: a worker must produce a clean pure-C form — replace `{ s16 tmp = -0x1C00; g_file_vram_timer = tmp; }` with `g_file_vram_timer = -0x1C00;` (or the natural u16 equivalent) and `& (s32)0xFFFEFFFE` with `& ~0x10001` (or `& 0xFFFEFFFE` without cast), then verify sandbox distance is 0 and SHA1 == oracle before re-submitting.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct 1 at src/code6cac.c:1041: { s16 tmp = -0x1C00; g_file_vram_timer = tmp; } inside if ((s16)g_file_vram_timer < -0x1C00). Target clamp (build/bb2.elf 0x8001b208-0x8001b224): lh v0; slti v0,v0,-7168; beqz skip (delay: li v0,-7168 = 0x2402e400 ADDIU form); sh v0,g_file_vram_timer

- [s1] [fable-blitz 2026-07-07] Why the tmp is probably load-bearing: g_file_vram_timer is extern u16 (include/system.h:10). Plain 'g_file_vram_timer = -0x1C00;' folds to unsigned-short cst 58368; expand materializes it in SImode as GEN_INT(0xE400) -> ori (0x3402e400), a 1-insn encoding diff vs target's addiu; the s16 tmp keeps the cst as -7168 -> addiu. The positive clamp 'g_file_vram_timer = 0x7400;' (src/code6cac.c:1044, plain spelling) matches at 0x8001b23c because positive 29696 fits addiu either way - explains why only the negative clamp got the tmp

- [s1] [fable-blitz 2026-07-07] Clean-lever shape evidence: every compare of this global already reads through (s16) value casts (src/code6cac.c:1040,1043,1050 -> lh at 0x8001b208/0x8001b228/0x8001b258), so a signed STORE view *(s16 *)&g_file_vram_timer = -0x1C00; is the same-kind type-pun (narrow sub-word access family, SOTN-sanctioned on the read side) and yields an HImode-signed store with cst -7168 -> addiu

- [s1] [fable-blitz 2026-07-07] Flagged construct 2 at src/code6cac.c:1057: *arg0 = *arg0 & (s32)0xFFFEFFFE. Target (0x8001b27c-0x8001b290): lui v1,0xfffe; lw v0,0(a0); ori v1,0xfffe; and; jr (delay: sw). In SImode RTL all three spellings - (s32)0xFFFEFFFE, bare 0xFFFEFFFE, ~0x10001 - canonicalize to const_int -65538 and force the identical lui/ori materialization (doesn't fit any 16-bit immediate); the cast is inert-likely with high confidence

- [s1] [fable-blitz 2026-07-07] Supporting style evidence: the sibling mask at src/code6cac.c:1046 (*arg0 &= ~0xB -> li v1,-12; and at 0x8001b24c) is already spelled with plain ~, so ~0x10001 is the consistent clean spelling for line 1057

- [s1] [fable-blitz 2026-07-07] Rest of function verified against target: seven zero-init stores (0x8001b138-0x8001b174), per-if fresh *arg0 reloads (lw at 0x8001b194/0x8001b1b0/0x8001b1dc/0x8001b248), lhu RMW for the +=/-= 0x4CC sites (0x8001b1c8/0x8001b1f4 - confirms u16 declared type is right), and the v>>4 rounding block (0x8001b258-0x8001b278) all follow from the existing clean source lines

- [s2] [2026-07-12] Applied the Judge's mandated clean forms: `g_file_vram_timer = -0x1C00;` (plain assignment, no tmp) and `*arg0 = *arg0 & ~0x10001;` (mask, no cast). Measured sandbox --disable all = 1 (87/87 insns, one encoding-only diff). This is the CLEAN FLOOR for this function under the Judge's constraints - confirmed by direct measurement, not assumed.

- [s2] [2026-07-12] Confirmed mechanism precisely: `(u16)(s16)-0x1C00` cast-on-literal and `0xE400` unsigned-hex literal both also score 1 (same ori encoding) - the front-end folds ANY literal RHS converted through the u16 lvalue's type to the unsigned CST 58368 regardless of how the literal is spelled/cast. Only a separately-typed VARIABLE (its own tree-level INTEGER_CST fixed at declaration, e.g. `s16 tmp` or `s32 lo`) escapes the target-type reconversion and keeps -7168, which materializes as addiu. Tested s32 lo confirms this generalizes beyond s16 - and sandbox score is 0 with it - but it is squarely the typed-literal-holder family the Judge forbade (same intent, different width). Banked as rejected/s32-typed-literal-holder-flagged-by-judge.c.

- [s2] [2026-07-12] Structural else-if merge of the two clamp checks measured KILLED: score jumps to 11 (build_insns 82 vs target 87) - target performs the positive-bound check UNCONDITIONALLY after the negative clamp (two independent `if`s), not a mutually-exclusive else-if. Confirms the existing two-if structure is correct and must not be touched.

- [s2] Clean src (plain g_file_vram_timer = -0x1C00; assignment + *arg0 & ~0x10001; mask, no s16 tmp, no pointer-cast pun, no signed cast on the mask) measures sandbox --disable all score=1, target_insns=87, build_insns=87 - a single encoding-only diff (addiu vs ori) with everything else byte-matched.

- [s2] The mask fix (& ~0x10001 replacing & (s32)0xFFFEFFFE) is confirmed inert/correct on its own - contributes 0 to the residual score.

- [s2] The residual byte is fully isolated to the negative-clamp store's constant-materialization instruction; no other part of the function is affected by any of this session's edits.

- [s2] No literal-RHS spelling (direct, cast-on-literal, or alternate hex base) changes the encoding - the u16 lvalue's type always wins the fold.

- [s2] Only a separately-typed named variable used purely to hold the clamp literal escapes the reconversion, and that construct is the exact family already ruled a cheat by the Judge (generalizes across s16/s32, so width is not an escape hatch).

- [s3] [2026-07-12, structural modality] Applied s2's clean floor to src (plain `g_file_vram_timer = -0x1C00;` + `*arg0 = *arg0 & ~0x10001;`), re-confirmed baseline sandbox --disable all = score 1, 87/87 insns — matches s2's recorded floor exactly.

- [s3] File-scope `static const s16 D_VRAM_TIMER_MIN = -0x1C00;` used as the clamp-store RHS scores 1 (NOT 0). Verified via objdump: the store still assembles to `li v0,0xe400` (raw `3402e400` = `ori $v0,$zero,0xe400`), identical to the plain-literal form, vs target's `addiu $v0,$zero,-0x1C00` at asm/funcs/func_8001B138.s:61. This KILLS the s2/hypotheses.md open frontier item speculating that storage-duration/linkage (file-scope vs block-local) might let a named constant escape the u16-target-type reconversion. It does not — only a block-local variable declaration escapes it; a file-scope const object is folded through the same reconversion path as a bare literal.

- [s3] Arithmetic-expression RHS `g_file_vram_timer = 0 - 0x1C00;` scores 1 — GCC 2.7.2 constant-folds the subtraction to the identical INTEGER_CST as the bare literal before the target-type conversion, so it is not a distinct code path.

- [s3] Enum-constant RHS (`enum { VRAM_TIMER_MIN = -0x1C00 };` then `g_file_vram_timer = VRAM_TIMER_MIN;`) scores 1 — an enumerator's CONST_DECL substitutes its INTEGER_CST at reference time exactly like a macro/literal, so it does not create a separately-typed tree node the way a local variable declaration does.

- [s3] Conclusion: the ONLY construct in this function's search space that reaches the target's `addiu` encoding is a block-scoped variable declaration used solely to hold the clamp literal (any integer width, per s2) — which is exactly the family the Judge already forbade. No storage-duration, linkage, or literal-spelling variant escapes it. src/code6cac.c restored to the s2 clean floor=1 form at session end.

- [s3] Re-confirmed s2's clean floor: plain `g_file_vram_timer = -0x1C00;` + `*arg0 = *arg0 & ~0x10001;` measures sandbox --disable all score=1, 87/87 insns.

- [s3] Verified via direct objdump of the sandbox .o: the residual diff is exactly one instruction, the negative-clamp store, which assembles to `ori $v0,$zero,0xe400` (raw 3402e400) in our build vs target's `addiu $v0,$zero,-0x1C00` at asm/funcs/func_8001B138.s:61.

- [s3] File-scope static const, arithmetic-expression RHS, and enum-constant RHS all produce the identical ori encoding as the bare literal - none of them create the separately-typed tree node that a block-local variable declaration does.

- [s3] This closes both open frontier items from s2/hypotheses.md: the file-scope-const idea is now measured (not just speculated) to fail, and no other named/derived-constant spelling escapes the reconversion either.

- [s3] The only construct in this function's search space that reaches addiu is a block-scoped variable declaration used solely to hold the clamp literal (any integer width, per s2) - exactly the family the Judge already forbade. No storage-duration, linkage, or literal-spelling variant escapes it.
