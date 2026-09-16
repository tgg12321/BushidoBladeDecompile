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

## [s4] Chassis re-verify: candidate.c (s3 form, inlined base expr) reproduces sandbox score 130 exactly on this session's HEAD.
- mechanism: N/A - chassis re-verification measurement
- probe: Applied candidate.c to src/main.c, ran `sandbox _SsSndCrescendo --disable all`
- result: CONFIRMED exactly: score=130, build_insns=213, target_insns=200. Chassis is NOT stale vs the ledger.
- verdict: CONFIRMED

## [s4] Reusing the already-computed `base` pointer (instead of the SS_SCORE_FLAG macro's fresh re-derivation) at the 6 unk90-clear sites closes the gap or at least matches build_insns closer.
- mechanism: N/A - alternative C spelling of the clear-site address, measured not derived; tests H3's "fresh recompute is real" finding directly rather than by asm-read alone
- probe: Replaced all 6 `SS_SCORE_FLAG(a0, a1) &= ~0x10;` sites with `*(s32 *)(base + 0x98) &= ~0x10;`, measured via sandbox --disable all
- result: KILLED: score 132, build_insns 165 (target 200) - FEWER instructions than target, confirming BB2's asm genuinely re-derives the address fresh at these sites (H3), not merely a plausible-but-unconfirmed asm reading. Reverted; banked to rejected/base-pointer-reuse-s4.md.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main.c HEAD with the base-pointer-reuse variant substituted for the banked candidate.c body at all 6 clear sites, no FAKE constructs present, sandbox --disable all

## [s4] Sharing the a0 sign-extension sub-expression (`bank_off`) explicitly across `base`'s computation AND all 6 clear sites, while still performing the FULL address re-derivation fresh at each clear site (not reusing `base`), closes the 3-vs-2-insn shift gap and/or improves the score below 130.
- mechanism: suspected combine.c/local-alloc.c - sharing only the sign-extend sub-step while keeping every other part of the address fresh per-site was flagged in s3's frontier as "may not be independently expressible in C (CSE/combine decide sub-expression sharing)"; this session measured it directly rather than leaving it as a derived guess.
- probe: Declared `s32 bank_off = (s32)(a0 << 16) >> 14;` once at top; rewrote `base = ...` to use `bank_off`; expanded all 6 clear sites inline as `*(s32 *)((u8 *)&_ss_score + bank_off + (s16)a1 * 0xB0 + 0x98) &= ~0x10;` (same full formula as the SS_SCORE_FLAG macro, just with the shift factored through the shared local). Measured via sandbox --disable all. Also measured a variant with `bank_off` declared `s16` instead of `s32`.
- result: KILLED as a closing form on THIS session's weighted score (139, worse than banked 130) but NOTABLE: build_insns landed EXACTLY on target_insns (200 vs 200) for the s32-bank_off variant - the first time this ledger has measured a form with the correct raw instruction count. The s16-bank_off variant was worse on both axes (136/208). Reverted to the banked 130 form; both variants banked to rejected/shared-bank-off-s4.md with a flagged next-step: register-alloc modality should dump-read THIS insn-count-exact variant's `.greg`/`.lreg` allocation (not the 213-insn banked form) since the obstacle there is now purely register choice, not instruction count.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main.c HEAD with the shared-bank_off variant (both s32 and s16 typings) substituted for the banked candidate.c body, no FAKE constructs present, sandbox --disable all

## [s4, permuter modality] Chassis re-verify: the s4-recorded 139/200/200 shared-bank_off form (rejected/shared-bank-off-s4.md) reproduces on this session's HEAD, and the banked 130 candidate.c reproduces exactly after re-applying it.
- mechanism: N/A - chassis re-verification measurement
- probe: Applied the s32-bank_off variant to src/main.c, measured; then reverted to candidate.c, measured again.
- result: CONFIRMED: bank_off variant = score 139, build_insns 200 (target 200) exactly as banked. candidate.c (inlined single-expr form) = score 130, build_insns 213, exactly as banked. Chassis NOT stale.
- verdict: CONFIRMED

## [s4, permuter modality] Directed permuter campaign on a standalone workspace (tmp/perm_crescendo_s4, built from the s4 139/200/200 bank_off chassis) can find a novel sub-9589 (base) form approaching 0.
- mechanism: N/A - random-mutation search from an insn-count-exact base
- probe: Built a clean single-function permuter workspace (base.c = the bank_off chassis, target.o extracted from asm/funcs/_SsSndCrescendo.s at offset 0 via _prelude.inc) and launched `tools/permuter_campaign.py launch -j 4 --stop-on-zero`; waited in-turn across 3 windows (tools/permuter_campaign.py wait), ~165s wall time, 4687 iterations total, harvested + stopped.
- result: KILLED as a closing search: base_score 9589 (permuter's own weighted metric, a different scale from the sandbox's), best find across all 4687 iterations was 8332 (only ~13% improvement), with no trend toward 0 and no near-zero find in any of the 3 fresh-seed wait windows. The permuter's random mutation space on this ~200-insn function with a large base score does not converge in this budget; per the brief's own caveat, the permuter cannot express the N-way statement-sharing/pointer-caching transformations this residual actually needs (confirmed directly below via manual objdump diff, not just inferred).
- verdict: KILLED
- kill_scope: instance
- measured_on: standalone permuter workspace tmp/perm_crescendo_s4 (base.c = s4 bank_off chassis, no FAKE constructs), decomp-permuter random mode, 4687 iterations, 4 parallel jobs, stop-on-zero enabled

## [s4, permuter modality] Directly diffing the permuter workspace's own target.o disassembly against its base.o (the 139/200/200 bank_off chassis) reveals the EXACT register-reuse pattern target uses for the bank-index address, which is NOT simply "share bank_off": target materializes bank_off's pointer (`&_ss_score + bank_off`, held in one hard reg for the whole function, spilled to stack early) AND a1's ×0xB0 product (a SEPARATE hard reg, also spilled early) as two independently-cached components, recombined via one `addu` at the point `base` is first built — not the single fused expression our banked 130 form uses, and not literally reusing `base` itself (which H3 already confirmed is wrong).
- mechanism: N/A - direct evidence from objdump, not derived/guessed; see tmp/grind/_SsSndCrescendo/s4/target.dis lines 4-31 vs base_s4_bankoff.dis same range
- probe: mipsel-linux-gnu-objdump -d on both tmp/perm_crescendo_s4/target.o and base.o (bank_off chassis), diffed the disassembly listings instruction-by-instruction (opcodes only, addresses/hex stripped)
- result: CONFIRMED via direct read: target's prologue is `move a3,a0 / sll v0,a3,0x10 / lui v1,%hi(_ss_score) / addiu v1,v1,%lo(_ss_score) / sra v0,v0,0xe / sw s3,52(sp) / addu s3,v0,v1` (s3 = bank pointer, cached) followed later by the already-matched a1*0xB0 chain feeding `sw s2,48(sp) / sll s2,v0,0x4` (s2 = a1_off, cached), then `lw v1,0(s3); addu s0,v1,s2` builds `base` ONCE from the two cached components. This is NOT what our banked forms do: the 130-score form recomputes everything inline each occurrence; the 139-score bank_off form shares ONLY the sign-extend step, re-deriving the a1*0xB0 product fresh at the base computation (matches) but the base itself only once (matches structurally) - the divergence from target is specifically in HOW MANY separate hard registers are held live and WHERE the clear-site re-derivations draw their operands from.
- verdict: CONFIRMED

## [s5, permuter modality] Caching BOTH bank_off AND a1_off as separate named C locals (following the objdump-diff finding above literally) closes the gap or improves on 130/139.
- mechanism: suspected combine.c/CSE - if C-level naming of a1_off mirrors target's register-level caching of the same component, GCC's CSE was hypothesized to reuse it exactly where target does
- probe: Declared both `s32 bank_off` and `s32 a1_off` at top, rewrote base and all clear sites to use both cached components (`*(s32*)((u8*)&_ss_score + bank_off + a1_off + 0x98) &= ~0x10;`), measured via sandbox --disable all
- result: KILLED: score 143, build_insns 184 - UNDER target_insns 200 for the first time this ledger (every prior variant landed AT OR ABOVE 200). Caching a1_off as a named C local makes GCC's CSE reuse it MORE aggressively than target's own asm does at the clear sites (target's visible s2/a1_off register at the top of the function is evidence of ONE local build of `base`, not a general a1_off-reuse idiom threaded through the whole body — the clear sites likely still re-derive fresh per H3). Reverted; banked to rejected/shared-bank-off-and-a1off-s5.md.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main.c HEAD with the bank_off+a1_off dual-cache variant substituted for the banked candidate.c body at all 6 clear sites plus base, no FAKE constructs present, sandbox --disable all

## [s3] Applying the banked s3 candidate.c body to src/main.c reproduces the ledger's recorded floor of 130 on the current chassis.
- mechanism: N/A - chassis re-verification measurement
- probe: Applied candidate.c to src/main.c, ran sandbox _SsSndCrescendo --disable all
- result: CONFIRMED exactly: score=130, build_insns=213, target_insns=200. Chassis is NOT stale vs the ledger.
- verdict: CONFIRMED

## [s3] Reusing the already-computed base pointer (instead of the SS_SCORE_FLAG macro's fresh re-derivation) at the 6 unk90-clear sites closes the gap or at least matches build_insns closer.
- mechanism: N/A - alternative C spelling of the clear-site address, measured not derived; directly tests H3's asm-read finding that BB2 re-derives the channel address fresh at these sites rather than reusing the cached register
- probe: Replaced all 6 SS_SCORE_FLAG(a0, a1) &= ~0x10; sites with *(s32 *)(base + 0x98) &= ~0x10;, measured via sandbox --disable all
- result: KILLED: score 132, build_insns 165 (target 200) - FEWER instructions than target, confirming BB2's asm genuinely re-derives the address fresh at these sites (H3), not merely a plausible-but-unconfirmed asm reading. Reverted; banked to rejected/base-pointer-reuse-s4.md.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main.c HEAD with the base-pointer-reuse variant substituted for the banked candidate.c body at all 6 clear sites, no FAKE constructs present, sandbox --disable all

## [s3] Sharing the a0 sign-extension sub-expression (bank_off) explicitly across base's computation AND all 6 clear sites, while still performing the full address re-derivation fresh at each clear site (not reusing base), closes the 3-vs-2-insn shift gap and/or improves the score below 130.
- mechanism: suspected combine.c/local-alloc.c - sharing only the sign-extend sub-step while keeping every other part of the address fresh per-site was flagged in s3's frontier as possibly not independently expressible in C since CSE/combine decide sub-expression sharing; this session measured it directly instead of leaving it as a derived guess
- probe: Declared s32 bank_off = (s32)(a0 << 16) >> 14; once at top; rewrote base = ... to use bank_off; expanded all 6 clear sites inline as *(s32 *)((u8 *)&_ss_score + bank_off + (s16)a1 * 0xB0 + 0x98) &= ~0x10; (same full formula as the SS_SCORE_FLAG macro, shift factored through the shared local). Measured via sandbox --disable all. Also measured an s16-typed bank_off variant.
- result: KILLED as a closing form on this session's weighted score (139, worse than banked 130) but NOTABLE: build_insns landed EXACTLY on target_insns (200 vs 200) for the s32-bank_off variant - the first time this ledger has measured a form with the correct raw instruction count. The s16-bank_off variant was worse on both axes (136/208). Reverted to the banked 130 form; both variants banked to rejected/shared-bank-off-s4.md flagging that register-alloc modality should dump-read this insn-count-exact variant's .greg/.lreg allocation instead of the 213-insn banked form, since its remaining obstacle is purely register choice, not instruction count.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main.c HEAD with the shared-bank_off variant (both s32 and s16 typings) substituted for the banked candidate.c body, no FAKE constructs present, sandbox --disable all

## [s4] The banked s3 candidate.c body and the s4-recorded 139/200/200 shared-bank_off form both reproduce their recorded sandbox scores exactly on this session's HEAD chassis.
- mechanism: N/A - chassis re-verification measurement
- probe: Applied each form to src/main.c in turn, ran sandbox _SsSndCrescendo --disable all
- result: CONFIRMED exactly: bank_off form = score 139, build_insns 200 (target 200); candidate.c = score 130, build_insns 213 (target 200). Chassis not stale vs ledger.
- verdict: CONFIRMED

## [s4] A directed permuter campaign on a clean standalone workspace (tmp/perm_crescendo_s4), seeded from the s4 139/200/200 bank_off chassis (the ledger's own recommended insn-count-exact base for register-alloc search), can find a novel form approaching score 0 on the permuter's own weighted metric.
- mechanism: N/A - random-mutation search from an insn-count-exact base
- probe: Built target.o from asm/funcs/_SsSndCrescendo.s at offset 0 (via a generic _prelude.inc) + base.o from the bank_off chassis; launched tools/permuter_campaign.py launch -j4 --stop-on-zero; waited in-turn across 3 windows (tools/permuter_campaign.py wait, ~165s wall, 4687 iterations); harvested + stopped.
- result: KILLED: base_score 9589, best find across all 4687 iterations was 8332 (~13% improvement, no trend toward 0, no near-zero find in any of 3 fresh-seed windows). The permuter's random mutation space does not converge on this residual in this budget - consistent with the brief's caveat that the permuter cannot express the N-way register-caching/sharing structure this function actually needs.
- verdict: ?

## [s4] Diffing the permuter workspace's own target.o disassembly against its base.o (the bank_off chassis) instruction-by-instruction reveals target's exact register-reuse pattern for the bank-index address computation: a dedicated hard register for the bank pointer (&_ss_score+bank_off) AND a separate dedicated hard register for a1's x0xB0 product, both spilled to the stack early and recombined via one addu at the point base is first built - not the single fused expression the banked 130 form uses, and not a naive reuse of base itself (already ruled out by H3/s4's base-pointer-reuse kill).
- mechanism: N/A - direct evidence from objdump read, not derived or guessed
- probe: mipsel-linux-gnu-objdump -d on both tmp/perm_crescendo_s4/target.o and base.o, diffed the disassembly opcode-by-opcode (addresses/hex stripped); artifacts copied to tmp/grind/_SsSndCrescendo/s4/{target,base_s4_bankoff}.dis
- result: CONFIRMED: target's prologue computes bank_off via (move a3,a0; sll; lui %hi(_ss_score); addiu %lo; sra; addu) into a hard reg (s3 in this build) saved to the stack, and separately the already-matched a1x0xB0 chain feeds another hard reg (s2) also saved to the stack; base is then built ONCE via lw+addu combining s3 and s2. This is new, directly-read evidence beyond H4's earlier combine.c dump finding - it names the SPECIFIC register-caching shape target uses, not just that a fold is blocked.
- verdict: CONFIRMED

## [s4] Literally mirroring target's 2-register-cache pattern in C - declaring BOTH a bank_off local AND an a1_off local, reused at every clear site - closes the gap or improves on the banked 130/139 forms.
- mechanism: suspected combine.c/CSE: naming a1_off as a C local was hypothesized to make GCC reuse it exactly where target's asm shows the cached a1_off register being used
- probe: Declared s32 bank_off and s32 a1_off at top of the function; rewrote base and all 6 clear sites to use *(s32*)((u8*)&_ss_score + bank_off + a1_off + 0x98) &= ~0x10; measured via sandbox --disable all
- result: KILLED: score 143, build_insns 184 - the first time any measured variant on this ledger landed UNDER target_insns (184 < 200; every prior variant landed at or above 200). Naming a1_off as a C local makes GCC's CSE reuse it MORE aggressively across all 6 clear sites than target's own asm actually does - target's visible cached a1_off register at the top of the function is evidence of ONE local build of base, not a general a1_off-reuse idiom threaded through the whole function body (the clear sites likely still re-derive the a1 offset fresh, consistent with H3's earlier confirmed fresh-recompute finding for the WHOLE clear-site address). Reverted; banked to rejected/shared-bank-off-and-a1off-s5.md.
- verdict: ?

## [s5, enumerate modality] Chassis re-verify: the banked candidate.c (s3 inlined-single-expr form) reproduces sandbox score 130 exactly via the enum-sweep baseline comparison.
- mechanism: N/A - chassis re-verification via tools/sweep_variants.py baseline row
- probe: python3 tools/sweep_variants.py --func _SsSndCrescendo --file main --variants tmp/grind/_SsSndCrescendo/s5/enum --json; the fully-inlined enum spellings (v10/v12/v14/v15, equivalent to the banked form under decl-order/operand-swap) reproduced score 130, build_insns 213 exactly.
- result: CONFIRMED: 130/213/200, matching the ledger's last recorded floor. Chassis NOT stale.
- verdict: CONFIRMED

## [s5, enumerate modality] The base/key address-computation preamble's ENTIRE spelling space (decl-order x inline-or-keep for bank_off/a1_off x commutative-swap of the two products), holding the SS_SCORE_FLAG clear sites fixed at their existing fresh-macro-recompute form, contains no spelling that beats the banked 130-score form.
- mechanism: N/A - exhaustive systematic enumeration (tools/spelling_enum.py), not derived or guessed; the tool enumerates ALL def-before-use decl orderings x inline/keep combinations x commutative operand swaps for a marked region by construction (tools/spelling_enum.py:9)
- probe: Marked the preamble (bank_off, a1_off named locals feeding base = ...; key = a0|(a1<<8);) with ENUM-BEGIN/END in tmp/grind/_SsSndCrescendo/s5/enum_candidate.c; `python3 tools/spelling_enum.py --candidate tmp/grind/_SsSndCrescendo/s5/enum_candidate.c --out tmp/grind/_SsSndCrescendo/s5/enum` generated all 16 distinct spellings (2 named locals x 2 assigns, with swaps); swept all 16 with tools/sweep_variants.py --json in one call.
- result: KILLED as a closing route: full histogram (16/16 measured) = 6 variants at 130/213 (all fully-inlined-equivalent forms, ties the banked best), 4 at 133/213, 4 at 137/215, 2 at 139/215. No spelling in this exhaustively-enumerated space scores below 130 or reaches build_insns=200. Notably, keeping BOTH bank_off and a1_off as NAMED locals (v00/v02) also ties 130/213 as long as the SS_SCORE_FLAG clear sites are left untouched (fresh-recompute) - this is a NEW result (s4's same-looking "shared bank_off+a1_off" form scored 143/184 because it ALSO rewrote the 6 clear sites to reuse the cached locals; s5's enum form does not touch the clear sites at all). This isolates the naming/caching of bank_off+a1_off in the PREAMBLE ALONE as harmless-but-not-helpful, strengthening H3/s4's already-confirmed finding that the residual is specifically in what happens at the 6 clear sites, not in the preamble's own spelling.
- verdict: KILLED
- kill_scope: class
- measured_on: src/main.c HEAD, banked candidate.c body as baseline chassis, preamble region replaced by each of the 16 exhaustively-enumerated spellings in turn, no FAKE constructs present, sandbox --disable all
- predicate_cite: tools/spelling_enum.py:9

## [s5] Chassis re-verify: the banked candidate.c (s3 inlined-single-expr form) reproduces sandbox score 130 exactly on this session's HEAD.
- mechanism: N/A - chassis re-verification via the enum-sweep's fully-inlined variants
- probe: python3 tools/sweep_variants.py --func _SsSndCrescendo --file main --variants tmp/grind/_SsSndCrescendo/s5/enum --json
- result: CONFIRMED: score 130, build_insns 213, target_insns 200 - matching the ledger's last recorded floor exactly.
- verdict: CONFIRMED

## [s5] The base/key address-computation preamble's entire spelling space (decl-order x inline-or-keep for bank_off/a1_off x commutative operand swaps), holding the 6 SS_SCORE_FLAG clear sites fixed at their existing fresh-macro-recompute form, contains a spelling that beats the banked 130-score form.
- mechanism: N/A - exhaustive systematic enumeration via tools/spelling_enum.py, which enumerates ALL def-before-use decl orderings x inline/keep combinations x commutative operand swaps for a marked region by construction
- probe: Marked the preamble with ENUM-BEGIN/END in tmp/grind/_SsSndCrescendo/s5/enum_candidate.c; generated all 16 distinct spellings via tools/spelling_enum.py; swept all 16 in one tools/sweep_variants.py --json call.
- result: KILLED: full histogram (16/16 measured) = 6 variants at 130/213 (ties the banked best), 4 at 133/213, 4 at 137/215, 2 at 139/215. No spelling reaches below 130 or hits build_insns=200. New finding: naming BOTH bank_off and a1_off in the preamble alone (clear sites untouched) also ties 130 - the s4/earlier-s5 143/184 regression came specifically from additionally rewriting the 6 clear sites to reuse those locals, not from naming them in the preamble. This isolates the residual to the clear sites, not the preamble.
- verdict: KILLED
- kill_scope: class
- measured_on: src/main.c HEAD, banked candidate.c body as the fixed surrounding chassis, preamble region replaced by each of the 16 exhaustively-enumerated spellings in turn, no FAKE constructs present, sandbox --disable all
- predicate_cite: tools/spelling_enum.py:9

## s6 (synthesis, 2026-09-16)

## [s6] The banked 130 candidate.c reproduces its recorded floor on this session's HEAD chassis.
**Probe:** apply candidate.c to src/main.c, `sandbox _SsSndCrescendo --disable all`.
**Result:** 130 / build_insns 213 / target_insns 200 - exact. **CONFIRMED.**

## [s6] The 13-instruction surplus (213 built vs 200 target) is dominated by the H4 register-allocation residual and other allocation-class effects.
**Probe:** the never-run full objdump-vs-target diff of the whole function
(frontier item 2, carried unchanged from s2 through s5), via a
register/operand-canonicalising differ (tmp/grind/_SsSndCrescendo/s6/diff.py).
**Result:** FALSE, and this is the session's central finding. H4's shift issue
is worth ONE instruction. The other twelve are five independent SOURCE-LEVEL
divergences (cached field locals; s16-vs-u16 voll/volr; the bank/a1_off pair;
the `unk40 <= 0` tail disjunct; two blocks target shares that GCC will not
cross-jump from duplicated source). Fixing them measures 130 -> 10.
**KILLED** (kill_scope: instance - measured on the s6 chassis with no FAKE
constructs present; what is killed is the ledger's attribution of the
surplus, not any C form).

## [s6] Removing the cached `unk42`/`unk40`/`key` s16 locals in favour of direct struct field re-reads and an inline key expression at every call site lowers the score.
**Mechanism:** the intervening `jal`s clobber memory, so target reloads
`lh 0x4C($s0)` / `lh 0x4A($s0)` and rebuilds the key per block; caching them
in locals makes GCC keep them in callee-saved registers across the calls,
adding copies and spilling `key` to the frame (`sh $a0,0x18($sp)`).
**Probe:** rewrite the body with no field locals, measure.
**Result:** 130 -> 84 (213 -> 210 insns). **CONFIRMED.**

## [s6] voll/volr are u16 and the two computed func_80087770 calls pass (u16)-truncated arguments.
**Probe:** target's `lhu 0x10($sp)` / `andi $a1,$a1,0xFFFF` / `andi $a2,$a2,0xFFFF`
(0x800843F4..0x80084410); spelled as `u16 voll, volr;` + `(u16)(voll+1)` etc.
**Result:** part of the 84 measurement; removing the casts re-introduces the
missing `andi`s. **CONFIRMED.**

## [s6] Carrying the bank-slot pointer and the a1*0xB0 product as two ordinary C locals (target's $s3/$s2) lowers the score on the s6 chassis.
**Probe:** `s32 *bank = (s32*)((u8*)&_ss_score + ((s32)(a0<<16)>>14)); s32 a1_off = (s16)a1*0xB0;`
with the in-arm clear sites spelled `*(s32*)(a1_off + *bank + 0x98) &= ~0x10;`.
**Result:** 84 -> 69. **CONFIRMED - and this REVERSES the s4 and s5 instance
kills** (`shared-bank-off-s4.md`, `shared-bank-off-and-a1off-s5.md`), which
measured the same lever at 139/143 on the cached-field chassis. Those kills
were correct as instance results and are now superseded: the lever works, it
was being measured on top of a larger error.

## [s6] The tail check's second disjunct is `unk40 <= 0`, not cres.c's `== 0`.
**Probe:** target `lh $v0,0x4A($s0); bgtz $v0,.L800844C0` at 0x80084470 skips
the clear only when unk40 > 0; measured both spellings.
**Result:** 69 -> 68. **CONFIRMED.** Same class of BB2/4.1-build divergence
from the SOTN reference as H1 (outer guard) and H2 (literal-1 4th arg).

## [s6] GCC 2.7.2 cross-jumps the duplicated tail check / unk40<0 handler that cres.c's structure produces, so the source may keep them duplicated in both arms.
**Probe:** measured both spellings of each block (duplicated in both arms vs
shared via `goto tail;` / `goto neg40;` into single blocks before the common
final call), and read the two emitted copies side by side in the sandbox .o
disassembly.
**Result:** FALSE - the two copies are byte-identical, 21 instructions each,
both ending in a jump to the same label, and GCC leaves both. Duplicated tail
= 233 insns; duplicated neg40 handler = 212; both shared = 199. Target emits
exactly one copy of each (.L8008441C and .L80084458/.L8008447C).
**KILLED** (kill_scope: instance - measured on the s6 chassis, no FAKE
constructs present). The shared-goto form is the `cross-jump-store-tail-merge`
shape (.claude/rules/cross-jump-store-tail-merge.md), ordinary C.

## [s6] The five in-arm clear sites and the shared tail clear can use the SAME address spelling.
**Probe:** all-cached (bank/a1_off everywhere), all-re-derived (inline
expression everywhere), all-macro (SS_SCORE_FLAG everywhere), and the mixed
form target actually shows.
**Result:** FALSE. All-cached under-counts (186 vs 200 - the 14-insn re-derive
block at the tail is missing); all-re-derived over-counts (241); all-macro on
the s6 chassis measures 89/223. Only the mixed form (cached in the arms,
inline re-derive at the shared tail) lands on 199/200. **KILLED**
(kill_scope: instance - s6 chassis, no FAKE constructs present).

## Live frontier after s6

### H5 (NEW, s6) - the last instruction is a second live copy of param a0
**Statement:** target opens with `addu $a3, $a0, $zero` and later makes a
third copy `addu $s5, $a3, $zero` (and `addu $s4, $a1, $zero` for a1). Every
key build in a block with no preceding call reads `$a3`/`$a1`; every one
after a call reads `$s5`/`$s4`. Our build has only the `$s5`/`$s4` copies and
reads the incoming `$a0` directly, which is exactly the one missing
instruction (199 vs 200) and the cause of all six remaining register-name
diffs plus the two unfilled delay slots (target fills them with a reorg.c
peel of `sll $a0,$s4,8` from the final call's key build -
[[reorg-peel-is-not-a-source-statement]]).
**Mechanism:** unread. Candidate passes: local-alloc.c (two allocnos for one
parameter would need two source-level pseudos) or caller-save.c. DO NOT
GUESS - run `pwsh tools/grinder/dump.ps1 _SsSndCrescendo` on the banked s6
candidate and read `.greg`/`.lreg` for the a0 pseudo(s).
**Next probe (in order):** (1) the dump read above; (2) parameter typing -
try `void _SsSndCrescendo(s32 a0, s16 a1)` / both `s32`, updating the forward
decl at src/main.c:234 and the call site at main.c:285 (NOTE: this changes
SsSeqCalledTbyT's own codegen, so re-measure that function too before
banking); (3) whether the preamble scheduling diff (`sra $v0,$v0,14` sitting
one slot later in target, after the `lui`/`addiu` of %hi/%lo(_ss_score))
resolves on its own once the register split matches.

### H6 (carried, s6) - permuter is now viable for the first time
**Statement:** at 199/200 insns with a purely register/scheduling residual,
a directed permuter campaign on the s6 chassis has a real gradient, unlike
the s4 campaign (which ran on a 139-score chassis carrying five unfixed
source-level errors and plateaued 8332-9589 over 4687 iterations).
**Next probe:** rebuild a single-function permuter workspace from the s6
candidate and run a fresh-seed campaign per [[permuter-fresh-seed-discipline]].

## [s6] The banked s3/s5 candidate.c reproduces its recorded floor of 130 on this session's HEAD chassis.
- mechanism: chassis re-verification (mandated before spending any banked conclusion)
- probe: apply memory/grind/_SsSndCrescendo/candidate.c to src/main.c; `wteng main sandbox _SsSndCrescendo --disable all`
- result: score 130, build_insns 213, target_insns 200 - exactly as banked. Chassis stable.
- verdict: CONFIRMED

## [s6] The 13-instruction surplus of the 130-score form (213 built vs 200 target) is dominated by the H4 register-allocation residual and other allocation-class effects.
- mechanism: ledger attribution carried s2->s5: combine.c fold refusal on the sign-extended-a0 intermediate, then local-alloc/global.c seat choice at the clear sites
- probe: ran the full objdump-vs-target diff of the whole function (frontier item 2, carried forward unchanged from s2 through s5 and never executed) with a register/operand-canonicalising differ, tmp/grind/_SsSndCrescendo/s6/diff.py
- result: False. H4's shift issue is worth ONE instruction. The other twelve are five independent source-level divergences: (a) s2 introduced cached s16 locals for unk42/unk40/key that target does not have - target re-reads those fields after every call and rebuilds the key per block; (b) voll/volr are u16 with (u16)-truncated call arguments, not s16; (c) the bank-slot pointer and the a1*0xB0 product are real source locals (target's $s3/$s2); (d) the tail check's second disjunct is `unk40 <= 0`, not cres.c's `== 0`; (e) the two arms share their unk40<0 handler and their tail check, which GCC will not produce from duplicated source. Correcting all five measures 130 -> 84 -> 69 -> 68 -> 50 -> 10.
- verdict: KILLED
- kill_scope: instance
- measured_on: s6 chassis = src/main.c HEAD with each successive s6 variant substituted for the banked 130 body; no FAKE constructs present in any variant

## [s6] Removing the cached unk42/unk40/key s16 locals in favour of direct struct field re-reads and an inline key expression at every call site lowers the score below the banked 130.
- mechanism: the intervening jal clobbers memory, so target reloads lh 0x4C($s0)/lh 0x4A($s0) and rebuilds the key per block; caching them forces GCC to hold them in callee-saved registers across the calls and to spill key to the frame (sh $a0,0x18($sp))
- probe: tmp/grind/_SsSndCrescendo/s6/v1.c - same control flow as the banked form, no field locals, key spelled (s16)(a0|(a1<<8)) at each of the 8 use sites; sandbox
- result: 130 -> 84 (build_insns 213 -> 210). The largest single contributor this session, and it voids the framing of the s2 key-type kill: the problem was never which type `key` had, it was that `key` was a variable at all.
- verdict: CONFIRMED

## [s6] Carrying the bank-slot pointer and the a1*0xB0 product as two ordinary C locals (target's $s3 and $s2), with the in-arm clear sites reloading *bank and re-adding a1_off, lowers the score on the s6 chassis.
- mechanism: target holds &_ss_score+((a0<<16)>>14) in $s3 and (s16)a1*0xB0 in $s2 for the whole function and does lw 0x0($s3) + addu $s2 at each clear site; cse.c does not manufacture those shared pseudos from repeated inline expressions (measured: all-inline = 241 insns), so they have to be source locals
- probe: tmp/grind/_SsSndCrescendo/s6/v3.c and v6.c; sandbox
- result: 84 -> 69 (then 68 with the <=0 fix). This REVERSES the s4 and s5 instance kills (rejected/shared-bank-off-s4.md, rejected/shared-bank-off-and-a1off-s5.md) which measured the same lever at 139/143 - correct as instance results, but measured on top of the cached-field error that dominated the residual.
- verdict: CONFIRMED

## [s6] The tail check's second disjunct is `unk40 <= 0` rather than the SOTN reference's `unk40 == 0`.
- mechanism: source-level semantic divergence in BB2's 4.1 LIBSND build, same class as the already-confirmed H1 outer guard and H2 literal-1 4th argument
- probe: target asm/funcs/_SsSndCrescendo.s:174 `bgtz $v0, .L800844C0` (skip the clear only when unk40 > 0) vs our `bnez`; measured both spellings
- result: 69 -> 68, and the branch mnemonic now matches. Confirmed by bytes, not by reference text.
- verdict: CONFIRMED

## [s6] GCC 2.7.2 cross-jumps the duplicated unk40<0 handler and the duplicated tail check that the SOTN reference's per-arm structure produces, so those statements may stay written out in both arms.
- mechanism: jump.c cross-jumping of identical block tails ending in a jump to the same label
- probe: measured duplicated vs goto-shared spellings of both blocks (tmp/grind/_SsSndCrescendo/s6/v8.c, v9.c, v10.c, v11.c) and read the two emitted copies side by side in the sandbox .o disassembly
- result: False. The two tail copies are byte-identical, 21 instructions each, both ending in a jump to the same label, and GCC leaves both in place. Duplicated tail = 233 insns; duplicated unk40<0 handler = 212; both shared via `goto tail;`/`goto neg40;` = 199. Target emits exactly one copy of each (.L8008441C and .L80084458/.L8008447C). The shared-goto form is the ordinary-C cross-jump-store-tail-merge shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: s6 chassis (direct field reads + bank/a1_off locals + u16 voll/volr) with each block's two spellings substituted; no FAKE constructs present

## [s6] The five in-arm clear sites and the shared tail clear can use one and the same address spelling.
- mechanism: whether cse.c/local-alloc keeps the cached pseudos live across the join point at the shared tail label
- probe: measured all-cached (v6), all-inline-re-derive (v7), all-SS_SCORE_FLAG-macro (v12), and the mixed form (v11)
- result: False. All-cached under-counts by exactly the 14-instruction re-derive block target emits at .L8008447C (186 vs 200); all-inline over-counts (241); all-macro measures 89/223. Only the mixed form - cached bank/a1_off inside the arms, full inline re-derive at the shared tail - lands on 199/200, which is what target's own asm shows.
- verdict: KILLED
- kill_scope: instance
- measured_on: s6 chassis, each spelling substituted for the clear sites of the banked s6 body; no FAKE constructs present

## s7 (synthesis, 2026-09-16) - MATCHED

## [s7] Chassis re-verify: the banked s6 candidate reproduces score 10 (199/200) on this session's HEAD.
- verdict: CONFIRMED
- result: measured 10 / build_insns 199 / target_insns 200.

## [s7] The score-10 residual is a CSE substitution at the goto-shared `unk40 < 0` block (parameter pseudo 72/$s5 vs the raw SImode copy 73/$a3), not a register-allocation problem, and it resolves when that block is duplicated into both arms instead of shared via goto.
- mechanism: cse.c starts a fresh basic block at a multi-predecessor label and loses the `72 == subreg(73)` equivalence established by assign_parms; global.c then coalesces 73 onto $a0 because its conflict set lacks hard reg 4 (read from .lreg/.greg).
- verdict: CONFIRMED
- result: the duplicated forms emit `addu $a3,$a0,$zero` and read $a3 at all three pre-call key builds, matching target.

## [s7] GCC 2.7.2's post-reload cross_jump merges the duplicated handler/tail blocks when and only when the two copies are register-identical; s6's "GCC will not cross-jump these" kill was chassis-relative to the a1_off local.
- verdict: CONFIRMED
- result: 212 insns with a1_off present (one copy uses pseudo 78/$s1, the other 93/$s2); 200 insns without it.

## [s7] Every SINGLE-EXPRESSION spelling of the bank-table address preamble leaves the `la $v1,_ss_score` insn on the wrong side of the folded ashiftrt:14.
- mechanism: combine.c try_combine emits the ashiftrt:16 + ashift:2 fold at the later insn's slot; sched.c rank_for_schedule preserves RTL/LUID order; fold() moves the constant ADDR_EXPR to operand 1 of a pointer sum.
- verdict: KILLED
- kill_scope: instance
- measured_on: s7 chassis (the score-0 candidate.c body with only the bank preamble substituted), no FAKE constructs present in the substituted preambles
- result: 13 spellings measured, all score 2 / 200 insns - see rejected/single-expression-preamble-s7.md.

## [s7] Splitting the preamble into `bank_no = a0;` then `score_tbl = (s32 *)&_ss_score;` then `bank = score_tbl + bank_no;` places the address insn between the two halves of the index shift and closes the function.
- verdict: CONFIRMED
- result: score 0, build_insns 200 == target_insns 200.

## Live frontier after s7
None - the function is MATCHED at the honest cheat-free distance 0. The
remaining work is review: the two preamble locals are claimed under the
pointer-alias and named-intermediate sanctioned families with FAKE
annotations (see self_vet.md). If layer-1 or the Judge rejects either family
for this shape, the next session's frontier is to find an ordinary-C
statement that naturally materialises the _ss_score address between the
ashift:16 and the scale; the only structural candidate left is a different
declaration of _ss_score itself (`extern s32 _ss_score[];` or a 2-D table
type), which changes the other ~10 already-matched call sites in this TU and
must be measured TU-wide before it is spent.
