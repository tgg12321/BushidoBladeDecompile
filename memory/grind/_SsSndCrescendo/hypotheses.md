# Hypotheses — _SsSndCrescendo

## Resolved (s2, 2026-09-16 — all CONFIRMED, spelled in candidate.c)

### H1 — outer `unk98` (0xA0) guard — CONFIRMED
Full asm read confirms the outer guard exactly as stated originally:
`if (--(*(s32*)(base+0xA0)) >= 0) { <all unk42 logic + inner
(unk98==0||unk40==0) clear> } else { SS_SCORE_FLAG(a0,a1) &= ~0x10; }`.
Spelled in candidate.c. Real control-flow divergence from cres.c, not a
codegen artifact — confirmed by reading the negative-branch's jump target
(straight past ALL unk42/unk40 logic to a fresh-address unconditional
clear + the shared final-call tail).

### H2 — `func_80087770`'s 4th arg is literal `1` (BB2), not `0` (cres.c) — CONFIRMED
Systematic across all 6 call sites in this function's own asm (no
exceptions) — settled directly, func_80087770's own body/identity was not
needed (it's still INCLUDE_ASM). Spelled as literal `1` throughout
candidate.c.

### H3 — the 3 `_ss_score[a0][a1].unk90 &= ~0x10` clears — CONFIRMED, matches cres.c exactly
These really do re-index fresh from `_ss_score` (not the cached `base`
pointer) at exactly the 3 (now more — see the NEW finding below) places
cres.c's own source syntax switches from `score->` to `_ss_score[arg0]
[arg1]`. Spelled via the pre-existing `SS_SCORE_FLAG(i,j)` macro
(main.c:267-268), the proven idiom for this exact pattern elsewhere in the
TU. No separate addressing needed — same channel throughout, per the fresh
address computation matching the same bank+offset formula each time.

### NEW (found s2, not a numbered H — folded into H3's resolution) — extra unk90 clear cres.c's text omits
In the `unk42<0` arm's `unk40>=0` sub-branch, the FIRST inner
`func_80087770(key,0x7F,0x7F,1)` call (guarded by `(voll-unk42>=0x7F) &&
(volr-unk42>=0x7F)`) is followed by an unk90 clear in BB2's asm that
cres.c's transcribed text does not show after the structurally-analogous
call. Verified 3x against the raw asm bytes (lines 117-123 of
asm/funcs/_SsSndCrescendo.s); not a mis-copy. Spelled as present (matches
target bytes, which is authoritative over cres.c's structural-guide role
per psyq-reference-c-version-skew).

## Live frontier

### H4 (NEW, s2) — 1-instruction address-shift codegen difference is register-allocation class, not yet closed
**Statement:** the `(a0<<16)>>14` bank-index shift compiles to 3
instructions here (`sll 16; sra 16 [full]; sll 2`) vs the already-matched
sibling `_SsSndPause`'s 2 (`sll 16; sra 14`, fused) — using the textually
IDENTICAL `s32 shifted=a0<<16; ...; shifted>>14` source idiom in both.
Three `key`-variable spellings measured this session (s32 cached / s16
cached / inlined-per-call-site — see rejected/key-type-variants-s2.md);
s16-cached is best (score 136, build_insns 213 vs target 200) but none
fused the shift. The likely cause: `_SsSndPause`'s post-shift value is
used ONCE immediately; `_SsSndCrescendo`'s `key` stays live across ~6 call
sites spanning the whole function, changing GCC's register-allocation /
combine decision for how to derive the shifted value.
**Mechanism:** suspected `combine.c` / `local-alloc.c` — the extra
liveness of `key` likely changes which pseudo the shift's result gets
folded into, or blocks a 2-op combine that only fires when the shifted
value dies immediately. NOT confirmed via `-da` dump yet — the PASS
ATTRIBUTION step (dump.ps1) was not run this session because the
turn budget was spent on the full asm read + candidate write; this is the
correctly-sequenced next step, not a skip.
**Next probe:** `pwsh tools/grinder/dump.ps1 _SsSndCrescendo` with the
candidate.c body applied, read the `.combine` and `.lreg`/`.greg` dumps
for the pseudo carrying `(a0<<16)>>14` and see which pass decides the
3-insn vs 2-insn split; cross-reference against `_SsSndPause`'s own dump
for the same expression to see the ACTUAL rule GCC used to fuse there but
not here. This is squarely `register-alloc-pure-c`
(.claude/rules/codegen-technique-index.md) territory — register-alloc
modality is the right next assignment for this ledger.
**Status (s3, 2026-09-16): PASS ATTRIBUTION CONFIRMED via `.combine` dump
read** (`tmp/grind/_SsSndCrescendo/dumps/main.combine`, function starts
line 1741). Insns 16/17/19 are the top-of-function bank-index shift:
`insn16: reg83 = ashift(reg73[=a0], 16)`, `insn17: reg82 =
ashiftrt(reg83, 16)` — carrying a `REG_EQUAL (sign_extend:SI (reg/v:HI
72))` note, i.e. combine's own analysis already IDENTIFIES this pair as a
plain HImode-to-SImode sign-extend of a0 — then `insn19: reg85 =
ashift(reg82, 2)` is a SEPARATE insn multiplying that sign-extended value
by 4. This is 3 RTL insns (matching the 3-insn asm: sll 16; sra 16; sll
2), never combined into one `ashiftrt(x,14)` insn. Root cause confirmed:
reg82 (the sign-extended a0, hard-reg s5 in the final allocation — see
`.greg` dump `285 in 65` = the pseudo carrying this value is referenced
in 65 separate RTL positions) is **NOT single-use** — the `.greg` dump's
disassembly correlate shows `sll v0,s5,0x2` re-emitted 3 more times later
in the function body (offsets 0x7b4/0x854/0x8dc in the sandboxed .o),
one per `SS_SCORE_FLAG(a0,a1)` macro expansion (each macro invocation
re-derives `((s32*)&_ss_score)[a0]` fresh, per H3's confirmed "recompute
from _ss_score" pattern). Because reg82/s5 has multiple later consumers
independent of the final `<<2` step, `combine.c`'s single-pass 2-3-insn
merge window cannot retire it (retiring it would require proving it dead
after the fold, which it isn't) — so the `ashift(reg82,2)` step stays a
separate, unfoldable insn. This is EXACTLY what already-matched
`_SsSndPause` avoids: `_SsSndPause` uses its shifted value exactly once
(one dereference, no repeated macro-style re-index), so combine's
liveness check clears and the 3-insn chain folds to `ashiftrt(x,14)`
(2 insns) freely.
**Confirmed via measurement, not just dump-read:** collapsing the 4
separate `shifted`/`addr`/`base_ptr`/`offset` locals into ONE inline
expression (matching the already-matched `_SsSeqPlay`'s style,
main.c:399) dropped sandbox score 136 -> 130 this session (build_insns
UNCHANGED at 213 — see the "not yet explored" note below on why
build_insns itself doesn't move). An array-subscript rewrite of the same
base expression (`((s32*)&_ss_score)[a0]`) was ALSO measured and was
WORSE (143/233) — banked in
`rejected/array-index-base-expr-s3.md`. Neither reduces build_insns
because the underlying RTL fold-blocker (reg82's multi-use across the
SS_SCORE_FLAG sites) is a property of the WHOLE function's control flow,
not the syntax of one expression — matches the mechanism above exactly:
no single-expression rewrite of the FIRST use can fix a fold that's
blocked by later independent uses of the same intermediate value.
**KILL (instance, this chassis, s3):** none of the three key-variable
spellings (s16-cached / s32-cached / inline-per-call-site, all measured
s2, see `rejected/key-type-variants-s2.md`) NOR either of the two
base-address-expression spellings measured this session (inline manual
shift / array subscript) closes the 3-vs-2-insn bank-index-shift gap,
because the gap's true cause (per the `.combine` dump) is the SHARED
liveness of the sign-extended-a0 intermediate across all 4
SS_SCORE_FLAG-macro re-derivation sites in the SAME function, which no
local rewrite of the FIRST use site alone can change.
**Next probe (untried):** the fold only unblocks if the SS_SCORE_FLAG
re-derivation sites themselves stop independently re-deriving
`(a0<<16)>>14` from scratch — i.e. try CACHING the shifted bank-pointer
(`s32 *bank_ptr = (s32*)((u8*)&_ss_score+((a0<<16)>>14));`) and rewriting
the 4 `SS_SCORE_FLAG(a0,a1)` clear sites to dereference `*bank_ptr` (or
an equivalent shared pointer) instead of re-invoking the macro — this is
a genuinely different C shape (not yet measured) and is the correct next
register-alloc-modality probe. NOTE: H3's own finding was that BB2's
asm DOES recompute fresh at those 3-4 sites (not reuse `base`/`s0`) — so
this next probe needs to preserve that fresh-pointer-recompute BYTE
PATTERN while sharing only the a0-sign-extension sub-step, which may
not be independently expressible in C (GCC decides sub-expression
sharing via CSE, not the programmer) — flagged as the likely wall for
next session to confirm or refute with a direct sandbox measurement.

## Not yet explored

- Whether the remaining ~13-instruction gap (213 built vs 200 target) is
  ENTIRELY the H4 shift issue (which is only 1 instruction of raw delta)
  or whether there are other, still-unidentified register-allocation
  diffs stacked on top — H4's dump read will surface this; a full
  objdump-vs-target diff (not just the header 30-40 lines this session
  spot-checked) is the mechanical way to enumerate every remaining diff
  once H4's lever is found and applied.
- The exact register choice for `voll`/`volr` (stack temps at sp+0x10/
  0x12 in target) — not yet compared instruction-for-instruction against
  the candidate's build; likely fine (both use stack slots) but unverified
  past the first ~40 lines of disassembly.

## [s2] Applying the banked s2 candidate.c body to src/main.c reproduces the ledger's recorded floor of 136 on the current chassis.
- mechanism: N/A - chassis re-verification measurement
- probe: Applied candidate.c to src/main.c, ran `sandbox _SsSndCrescendo --disable all`
- result: CONFIRMED exactly: score=136, build_insns=213, target_insns=200. Chassis is NOT stale vs the ledger.
- verdict: CONFIRMED

## [s2] Collapsing the base-address computation from 4 separate locals (shifted/addr/base_ptr/offset) into one inline expression (matching the already-matched _SsSeqPlay's style, main.c:399) lowers the sandbox score below 136 on this chassis.
- mechanism: Fewer intermediate pseudos reduces weighted register-diff penalty in the sandbox score even though raw build_insns is unchanged (combine/RA effect, not instruction-count effect)
- probe: Rewrote the base = ... statement as base = (u8*)(*(s32*)((u8*)&_ss_score + ((s32)(a0<<16)>>14)) + (s16)a1*0xB0); measured via sandbox --disable all
- result: CONFIRMED: score dropped 136 -> 130 (build_insns unchanged at 213, target 200), measured twice (once before, once after reverting the array-index variant, both times reproducing 130 exactly).
- verdict: CONFIRMED

## [s2] Rewriting the base-address computation as an array subscript into _ss_score (((s32*)&_ss_score)[a0] + (s16)a1*0xB0) closes or improves on the 130-score inline-shift form on this chassis.
- mechanism: N/A - alternative C spelling of the same address computation, measured not derived
- probe: Rewrote base = ... using the array-subscript form, measured via sandbox --disable all
- result: KILLED: score 143, build_insns 233 (worse than both the 136 and 130 forms on every axis). Reverted; banked to rejected/array-index-base-expr-s3.md.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main.c HEAD with the array-subscript base-expr variant substituted for the banked candidate.c body, no FAKE constructs present, sandbox --disable all

## [s2] The 3-instruction (vs target/_SsSndPause's 2-instruction) codegen for the (a0<<16)>>14 bank-index shift is caused by combine.c being unable to fold the ashift(16)+ashiftrt(16)+ashift(2) chain into a single ashiftrt(14) insn, because the sign-extended-a0 intermediate (insn17's dest pseudo) has additional live consumers later in the function from the 4 separate SS_SCORE_FLAG(a0,a1) macro re-derivation sites, unlike _SsSndPause where the shifted value has exactly one consumer.
- mechanism: combine.c 2-3-insn local fold window requires the intermediate value to die at the fold point; it does not here because reg82 (sign-extended a0, hard-allocated to $s5) is re-read via sll v0,s5,0x2 at 3 further offsets (0x7b4/0x854/0x8dc in the sandboxed .o) corresponding to the SS_SCORE_FLAG macro's independent fresh re-derivations elsewhere in the function body
- probe: pwsh tools/grinder/dump.ps1 _SsSndCrescendo; read tmp/grind/_SsSndCrescendo/dumps/main.combine (function block starts line 1741, insns 16/17/19) and cross-referenced .greg dump pseudo-285/reg82 usage (65 references) against the sandboxed .o disassembly (mipsel-linux-gnu-objdump -d tmp/sandbox/_SsSndCrescendo/main.o), confirming 3 additional sll v0,s5,0x2 sites
- result: CONFIRMED via direct dump read: insn17 in main.combine carries a REG_EQUAL (sign_extend:SI (reg/v:HI 72)) note (combine recognizes the sign-extend semantically) but does not retire the insn because reg82/hard-reg s5 is referenced at 3 more disassembly offsets outside the fold window. This is a named-mechanism (combine.c local fold window vs multi-use liveness), not a guess.
- verdict: CONFIRMED
