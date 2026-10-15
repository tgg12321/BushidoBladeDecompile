# Evidence bank — func_8004A4E0

## s1 (recon) — canonical gate verdict + OBJECT MODEL

`canonical func_8004A4E0` returns verdict **ASM-WHOLE** (`"reasons": ["whole-function
canonical-asm grant in inline_asm_canonical.txt"]`), citing `inline_asm_canonical.txt:199`.
That entry (text corrected 2026-09-29 per owner ruling Q40, commit 803d0fea1) is backed by a
full decisive audit at `docs/audits/func_8004A4E0-canonical-2026-09-29.md`: a custom
callee-saved-register ABI across `jal func_8004A940`/`func_8004A76C`/`func_8004A808` —
$s4/$s5/$s6/$fp/$s0 handed across calls in ways GCC 2.7.2 cannot emit from C (dead-looking
pre-call stores the callee actually reads; `3-$s5` the callee itself writes so C would fold
it to a constant; a ghost `$s3` save/restore the callee clobbers unsaved; a callee-advanced
`$s0` read back after return, which GCC assumes a callee-save can't do) — plus 2 trapping
`addi` (0x8004A5A0, 0x8004A67C, non-overflow-safe in C) and 4 `ctc2` (no C form). The audit's
own conclusion: C islands would have to cover every handoff/`jal` site with forbidden GPR
asm, so the remainder is the whole body. `scan_hand_coded.py --single` tier is LOW (0/8 — the
S7 heuristic checks callee-side unsaved callee-saves; this function is the *caller* and saves
everything), so the grant rests on the ABI evidence, not scanner tier.

`sandbox func_8004A4E0 --disable all` this session: `{"score": 163, "no_c_body": true,
"build_insns": 0, "cheat_asm_stripped": 71}` — current `src/main/3AB48.c` still carries this
function as raw `INCLUDE_ASM`; there is no C body to diff (`--diff` prints "no diff: function
is still whole-body INCLUDE_ASM").

OBJECT MODEL: all 7 flagged DATA MODEL symbols are real operands inside
`asm/funcs/func_8004A4E0.s` (grep-confirmed: `D_80095328` @107-108,152-153; `D_80095488`
@169-170; `D_80095508` @136-137; `D_800FF610` @20-21; `g_draw_queue` @15-16; `D_80103608`
@39-40; `jtbl_8001541C` @29-31; `jtbl_8001545C` @55-57). Every one is
**MISMATCH-unmeasured (why): no C statement in this function references any of these symbols
— the entire body is `INCLUDE_ASM`, so no header declaration/type change for any of them can
alter this function's emitted bytes.** This is confirmed mechanically by the sandbox
`no_c_body: true` above, not inferred. The declaration-fix hypothesis (brief's recon
default #1) is therefore inapplicable to *this* function as long as it stays whole-body asm;
it would only become live if a future session carved a C island out of the body (which the
2026-09-29 audit argues is impossible without forbidden GPR asm — see hypothesis H1 below).

- [s1] canonical func_8004A4E0 returns verdict ASM-WHOLE, reason 'whole-function canonical-asm grant in inline_asm_canonical.txt' (entry at inline_asm_canonical.txt:199).

- [s1] sandbox func_8004A4E0 --disable all this session: {"score": 163, "no_c_body": true, "build_insns": 0, "cheat_asm_stripped": 71}; --diff reports no diff is possible since the function is still whole-body INCLUDE_ASM.

- [s1] scan_hand_coded.py --single tier for this function is LOW (0/8) per the audit, because its S7 heuristic checks callee-side unsaved callee-saves and this function is the caller, saving everything — the grant rests on the cross-call ABI evidence in the 2026-09-29 audit, not scanner tier.

- [s1] All 7 flagged DATA MODEL symbols (D_80095328, D_80095488, D_80095508, D_800FF610, g_draw_queue, D_80103608, jtbl_8001541C, jtbl_8001545C) are grep-confirmed as real lui/addiu/lw operands inside asm/funcs/func_8004A4E0.s; none is reachable from a C statement in this function since the whole body is INCLUDE_ASM, so no header-declaration fix can change this function's score. OBJECT MODEL bank entry written in full to evidence.md with per-symbol line citations.

- [s1] The inline_asm_canonical.txt:199 entry text was corrected 2026-09-29 under owner ruling Q40 (commit 803d0fea1), post-dating the 2026-08-18 judge-sole-gate ruling, so the grant has already been owner-reviewed under current policy.

- [s2] canonical func_8004A4E0 this session returns verdict ASM-WHOLE, reason 'whole-function canonical-asm grant in inline_asm_canonical.txt', asm_insns=7, total=163 — unchanged from the ledger's prior recorded state.

- [s2] No docs/grind/decisions.md entry exists yet for func_8004A4E0 (checked via grep); not filed this session because my mandated modality is structural, not escalation, and the owner's standing auto-ruling (A) reserves the exhaustion determination to the driver's modality assignment.

- [s2] candidate.c and evidence.md already accurately describe the no-C-body state from s1 recon; no edits needed there this session beyond the hypotheses.md s2 re-confirmation entry.
