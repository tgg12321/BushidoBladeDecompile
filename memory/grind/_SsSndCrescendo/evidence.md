# Evidence — _SsSndCrescendo

## s1 (recon)

OBJECT MODEL: `_ss_score` MATCHES — the target asm's addressing
(`sll a0,16; sra ,14` == `(s16)a0*4`, then `addu` onto `%hi/%lo(_ss_score)`
base, then `lw` the resulting pointer, then `+ (s16)a1*0xB0`) is byte-for-byte
the SAME pattern already used, matched, and byte-proven at 10+ call sites in
this same TU (`src/main.c:318-345,370-372,583,609-642,809-855,1262-1285`,
e.g. the byte-matched `_SsSeqPlay` at main.c:341-366). `_ss_score` is
declared `extern s32 _ss_score;` (scalar handle, main.c:37/231) and accessed
via `(s32 *)&_ss_score` + byte-offset pointer arithmetic — this is the
established, ALREADY-PROVEN idiom for this global in this codebase. No
declaration-fix hypothesis applies; proceeding straight to codegen structure
is correct here. (No sandbox measurement needed for this conclusion — it's
a structural match against 10 already-oracle-verified call sites, not a
guess.)

## SOTN precedent found (STRONG — exact function name, not analog)

`tmp/sotn-decomp/src/main/psxsdk/libsnd/cres.c` contains a **matched PSX
`_SsSndCrescendo`** (SOTN's own decomp of the same Sony LIBSND `CRES`
module BB2 links — confirmed identity via `memory/closer/libsnd-hunt-report.md:70`,
`memory/closer/sony-naming-map.md:26`: "func_800841E0 -> _SsSndCrescendo
(LIBSND CRES) ; identity by call position in SsSeqCalledTbyT"). Full text
read this session (49 lines):

```c
void _SsSndCrescendo(s16 arg0, s16 arg1) {
    struct SeqStruct* score = &_ss_score[arg0][arg1];
    u16 voll, volr;
    score->unk98--;
    if (score->unk42 > 0) {
        if ((score->unk98 % score->unk42) == 0) {
            score->unk40--;
            if (score->unk40 >= 0) {
                SpuVmGetSeqVol(arg0|(arg1<<8), &voll, &volr);
                if ((voll+1) <= (voll+score->unk40))
                    SpuVmSetSeqVol(arg0|(arg1<<8), voll+1, volr+1, 0);
            } else {
                SpuVmSetSeqVol(arg0|(arg1<<8), 0x7F, 0x7F, 0);
                _ss_score[arg0][arg1].unk90 &= ~0x10;
            }
            if ((score->unk98==0)||(score->unk40==0))
                _ss_score[arg0][arg1].unk90 &= ~0x10;
        }
    } else if (score->unk42 < 0) {
        score->unk40 += score->unk42;
        if (score->unk40 >= 0) {
            SpuVmGetSeqVol(arg0|(arg1<<8), &voll, &volr);
            if (((voll-score->unk42)>=0x7F) && ((volr-score->unk42)>=0x7F))
                SpuVmSetSeqVol(arg0|(arg1<<8), 0x7F, 0x7F, 0);
            if (((score->unk94-score->unk98)*-score->unk42) < score->unk3E)
                SpuVmSetSeqVol(arg0|(arg1<<8), voll-score->unk42, volr-score->unk42, 0);
        } else {
            SpuVmSetSeqVol(arg0|(arg1<<8), 0x7F, 0x7F, 0);
            _ss_score[arg0][arg1].unk90 &= ~0x10;
        }
        if ((score->unk98==0)||(score->unk40==0))
            _ss_score[arg0][arg1].unk90 &= ~0x10;
    }
    SpuVmGetSeqVol(arg0|(arg1<<8), &score->unk78, &score->unk7A);
}
```

Caveat: [[psyq-reference-c-version-skew]] applies — BB2 links a DIFFERENT
PsyQ/LIBSND build than the sotn-decomp source tree (hunt report identifies
BB2's build as the "4.1 CRES/DECRES fix" — the changelog explicitly says
CRES/DECRES were *modified* going into 4.1), so this is a strong STRUCTURAL
guide, not a byte-for-byte transcript. Field offsets differ (BB2 struct
stride 0xB0 confirmed from asm; SOTN's `struct SeqStruct` field names are
its own reverse-engineering guesses, not proven offsets for BB2's build).

## BB2 asm field-offset mapping derived this session (from `asm/funcs/_SsSndCrescendo.s`, cross-read against cres.c control flow)

Channel struct base = `s0` = `&_ss_score[a0][a1]` (stride 0xB0, matches
census `memory/closer/*` + main.c precedent).

| BB2 offset | asm evidence | SOTN field (name only, offset unproven) | confidence |
|---|---|---|---|
| `0xA0` | decremented at function entry (`lw;addiu -1;sw`), sign-tested immediately after | `unk98` (SOTN also decrements this field first, line 9) | HIGH — same "decrement first" position |
| `0x4C` | `lh a2,0x4C(s0)`; `blez a2,.L80084318` branches pos/non-pos | `unk42` (SOTN branches `>0` / `<0` on this field right after the decrement) | HIGH |
| `0x4A` | `lhu v0,0x4A(s0); -1; sh back`, then sign-tested (`bltz -> .L8008441C`) inside BOTH the pos-42 and non-pos-42 arms | `unk40` (SOTN decrements/adds-to this field then tests `>= 0` in BOTH arms) | HIGH |
| `0x98` | `lw;andi -0x11(~0x10);sw` — clears bit 0x10, appears 3x (once per exit path) | `unk90` (SOTN clears `& ~0x10` in the same 3 positions: neg-40 branch, and the "unk98==0 \|\| unk40==0" tail check in each of the two 42-sign arms) | HIGH |

## Structural DIVERGENCE found (the actual frontier — not yet resolved)

The BB2 asm has an **outer early-exit that SOTN's cres.c does NOT have**:
after decrementing the `0xA0` field (SOTN's `unk98--`), asm branches
`bgez $v0, .L80084260` — i.e. if the POST-decrement value is still
`>= 0`, fall into the `0x4C` (unk42)-sign logic; but if it went NEGATIVE,
jump straight past ALL of the unk42/unk40 logic directly to the tail
(clear the `0x98` bit + the final `SpuVmGetSeqVol` call at label
`.L800844AC`/`.L800844C0`). SOTN's `cres.c` runs the `unk42`-sign checks
**unconditionally** every call, regardless of `unk98`'s sign after
decrement.

This is evidence the BB2/4.1 build's `_SsSndCrescendo` gained an extra
guard clause not present in whatever build sotn-decomp's `cres.c` reflects
— consistent with the hunt report's note that CRES was one of the modules
"fixed after 4.0" going into 4.1. This guard is the #1 thing to get right
before transcribing the rest; getting it wrong will cost ~150 instructions
of distance in the two skipped branches alone (0x4C/0x4A logic is ~half
the function body).

- [s1] OBJECT MODEL: _ss_score access pattern in the target asm is byte-identical to 10+ already-oracle-verified call sites in this TU (main.c:318-345,370-372,583,609-642,809-855,1262-1285, incl. byte-matched _SsSeqPlay) - no declaration-fix needed, extern s32 _ss_score; scalar-handle idiom is already proven here.

- [s1] tmp/sotn-decomp/src/main/psxsdk/libsnd/cres.c contains SOTN's own matched _SsSndCrescendo for the same Sony LIBSND CRES module (identity confirmed via memory/closer/libsnd-hunt-report.md:70 and memory/closer/sony-naming-map.md:26 by call-position in SsSeqCalledTbyT) - full 49-line reference read this session.

- [s1] BB2 links a different PsyQ/LIBSND build than sotn-decomp's tree (psyq-reference-c-version-skew) - hunt report explicitly notes CRES/DECRES were modules 'fixed after 4.0' going into the 4.1 build BB2 uses, so cres.c is a strong structural guide but not a verbatim transcript.

- [s1] Derived BB2 struct offset mapping (channel struct stride 0xB0, base = &_ss_score[a0][a1]) by cross-reading asm/funcs/_SsSndCrescendo.s against cres.c control flow: 0xA0=unk98 (outer decremented counter), 0x4C=unk42 (sign-branch field), 0x4A=unk40 (inner decremented/added field, sign-tested in both arms), 0x98=unk90 (bit 0x10 cleared in 3 places). Table with per-field asm evidence in evidence.md.

- [s1] Found a genuine STRUCTURAL divergence from the SOTN reference (not yet resolved): BB2 asm wraps the entire unk42-sign logic in an outer guard on the post-decrement sign of unk98 (0xA0) - `bgez v0,.L80084260` falls into the unk42 logic, but the negative case jumps straight past ALL of it to the tail. SOTN's cres.c runs the unk42-sign checks unconditionally regardless of unk98's sign. This guard, once confirmed and spelled, is expected to close roughly half the function's distance by itself since the skipped branches cover most of the body.

- [s1] Final SpuVmGetSeqVol-analog call in the tail writes directly into struct fields at s0+0x5C/0x5E (matches cres.c's final call writing &score->unk78/&score->unk7A) - confirmed by static asm read, not yet compiled.

- [s1] Open call-signature question: every jal to func_80087770 (the SpuVmSetSeqVol analog) passes a literal 1 as the 4th arg where cres.c's SpuVmSetSeqVol always passes literal 0 - func_80087770 is only a WEAK/unproven identity per the hunt report, so this needs checking against func_80087770's own body/other call sites before transcribing calls verbatim.

## s2 (structural) — full asm read, H1/H2/H3 all resolved, candidate written

Read the ENTIRE `asm/funcs/_SsSndCrescendo.s` (213 lines) instruction-by-
instruction and cross-mapped every branch/field-offset against cres.c.
func_80087770 is still its own INCLUDE_ASM (queue-unmatched), so H2's
"check its own body" probe was moot — the 4th-arg-is-1 fact is settled
directly from _SsSndCrescendo's own asm (systematic across all 6 call
sites, no exceptions), independent of func_80087770's identity/signature.

- **H1 CONFIRMED.** The outer `unk98`(0xA0) guard is real: asm decrements
  and stores `*(s32*)(base+0xA0)` UNCONDITONALLY (the store is in the
  branch's delay slot, executes either way), then branches on the
  POST-decrement sign. Negative -> jump straight to a fresh recompute of
  the channel address + unconditional `unk90 &= ~0x10` clear + the final
  tail call, skipping ALL unk42/unk40 logic AND skipping the
  "(unk98==0)||(unk40==0)" conditional check that gates the SAME clear on
  the positive path. Spelled as `if (--(*(s32*)(base+0xA0)) >= 0) {...}
  else { SS_SCORE_FLAG(a0,a1) &= ~0x10; }`.

- **H2 CONFIRMED**, systematic, no exceptions: all 6 `func_80087770` call
  sites in this function pass literal `1` as the 4th arg (BB2/4.1
  addiu $a3,$zero,0x1 at every site) vs cres.c's transcribed literal `0`.
  Spelled as literal `1` in every call in candidate.c.

- **H3 CONFIRMED — matches cres.c exactly, no divergence found.** The
  three `_ss_score[arg0][arg1].unk90 &= ~0x10;` sites in cres.c really do
  correspond to BB2 asm re-deriving the channel address FRESH from the
  bank-pointer slot (a NEW `lw` of the bank pointer + re-add the channel
  offset) rather than reusing the cached `s0`/`base` register — exactly
  where cres.c's source syntax switches from `score->` (the cached
  pointer) to `_ss_score[arg0][arg1]` (a fresh double-index) for these
  specific statements. This is NOT a codegen artifact to fight; it's a
  literal source-level re-index in the reference, and BB2's asm shows the
  identical re-derive-after-call pattern (GCC treats the intervening
  `jal func_80087770` as clobbering the cached value's provenance
  differently from a same-object reload — moot, since the SOURCE itself
  re-indexes). Spelled via the pre-existing `SS_SCORE_FLAG(i,j)` macro
  (main.c:267-268) which is EXACTLY this same "recompute base+0x98 fresh
  from _ss_score" pattern already used at 6+ other sites in this same TU
  (SsSeqCalledTbyT). Using the established macro instead of hand-rolling
  the cast expression is the right idiom-reuse call.

- **NEW divergence found (not previously in evidence.md): the unk42<0
  branch's FIRST inner call has an extra unk90 clear cres.c's transcribed
  text does NOT show.** In the `unk40>=0` sub-branch of the `unk42<0` arm,
  after `((voll-unk42)>=0x7F) && ((volr-unk42)>=0x7F)` guards a
  `func_80087770(key,0x7F,0x7F,1)` call, BB2's asm (`.L800843B8`'s lead-in,
  lines 117-123 of the .s file) immediately re-derives the channel address
  and clears `unk90 &= ~0x10` — a clear that is NOT present after the
  structurally-identical call in the `unk42>0` arm's own `unk40<0` else
  branch's sibling call (that one DOES have a clear, per cres.c and the asm
  both) but IS present here where cres.c's copied text shows only the bare
  call with no trailing clear. Read three times against the asm to be sure
  this isn't a mis-copy of cres.c; the asm is unambiguous (lines 117-123:
  `lw v0,0x0(s3); addu v0,s2,v0; lw v1,0x98(v0); addiu a0,zero,-0x11; and
  v1,v1,a0; sw v1,0x98(v0);` sits directly between this call and the
  following `.L800843B8:` label, which is NOT part of any other call's
  sequence). Most likely explanation: either cres.c's own transcription
  from its (different, pre-4.1) LIBSND build omitted this clear, or BB2's
  4.1 CRES fix added it. Spelled as present in candidate.c (matches the
  BYTES, which is what actually matters here — cres.c is a structural
  guide, not gospel, per the psyq-reference-c-version-skew caveat already
  on record).

- **Sandbox measured: `sandbox --disable all` dropped from the s1 floor of
  200 (no C written) to 136** (build_insns 213 vs target 200) with the
  candidate in candidate.c. Confirms H1/H2/H3 + the field-offset table +
  the extra-clear finding are all real, substantial contributors — NOT
  fully closing, ~13 instructions of residual remain (see hypotheses.md
  H4, register-allocation class).

- **Sibling comparison performed**: `_SsSndPause` (main.c, already
  COMPLETED-C, sandbox score 0) uses the textually IDENTICAL
  `s32 shifted=a0<<16; s32*addr=(s32*)&_ss_score; s32*base_ptr=(s32*)((u8*)
  addr+(shifted>>14)); s32 offset=(s16)a1*0xB0;` idiom for the SAME
  `_ss_score` bank+channel addressing, and in ITS build this compiles to
  the FUSED 2-instruction form (`sll v0,a0,0x10; sra v0,v0,0xe` — shift by
  14 directly). In `_SsSndCrescendo`'s build the identical source shape
  compiles to 3 instructions (`sll;sra 0x10 [full 16]; sll 0x2`) instead.
  Confirmed via direct sandbox+objdump of both functions this session.
  The only structural difference between the two functions at the point of
  this computation is that `_SsSndPause` uses its post-shift `key`-like
  value exactly once (immediately, one call), while `_SsSndCrescendo`'s
  `key` stays live across up to 6 downstream call sites spanning the whole
  function body — i.e. this looks like a register-allocation / live-range
  driven codegen choice, not something the C SOURCE shape controls
  directly (three different `key` spellings measured this session, see
  rejected/key-type-variants-s2.md; none reproduced the fused 2-insn form).

## s3 (structural, 2026-09-16) — chassis re-verify + H4 pass attribution + inline-address measurement

Chassis re-verified: applied candidate.c (s2 form, 4 separate locals) to
src/main.c and ran `sandbox _SsSndCrescendo --disable all`. Score
reproduced EXACTLY as banked: **136** (build_insns 213, target_insns
200) — the s2 floor was NOT stale.

**Pass attribution (the mandated next step from s2's H4, now done):**
read `tmp/grind/_SsSndCrescendo/dumps/main.combine` (generated via
`pwsh tools/grinder/dump.ps1 _SsSndCrescendo` with candidate.c applied).
Function's combine block starts at line 1741. Confirmed the 3-vs-2-insn
bank-index-shift gap is caused by combine.c's inability to fold
`ashift(x,16)` + `ashiftrt(t,16)` + `ashift(u,2)` into a single
`ashiftrt(x,14)` insn BECAUSE the intermediate sign-extended-a0 value
(insn17's dest, reg82, carrying a `REG_EQUAL (sign_extend:SI ...)` note —
combine recognizes the semantic but still can't retire the insn) has
multiple LATER independent consumers: the .greg dump shows this same
pseudo (hard-allocated to $s5) re-read via `sll v0,s5,0x2` at 3 more
offsets in the disassembly (0x7b4/0x854/0x8dc), one per
`SS_SCORE_FLAG(a0,a1)` macro expansion elsewhere in the function body
(H3's confirmed "fresh recompute from _ss_score" pattern re-derives
`(a0<<16)>>14` from scratch at each of those sites too). Combine's local
2-3-insn window can only fold a chain when the intermediate dies at the
fold point; here it doesn't, so the fold is refused. This is the
mechanism _SsSndPause (matched, fused 2-insn form) does NOT hit, because
its shifted value has exactly one consumer with no repeated macro-style
re-derivation elsewhere in that (much shorter) function.

**Measured attempt to fix via C-source rewrite of the FIRST use site
only:** collapsing the top-of-function base-address computation from 4
separate locals (shifted/addr/base_ptr/offset) into ONE inline
expression, styled after the already-matched _SsSeqPlay (main.c:399,
`base = (u8*)(*(s32*)((u8*)&_ss_score + ((s32)(a0<<16)>>14)) + (s16)a1*0xB0);`),
dropped the sandbox score **136 -> 130** (build_insns UNCHANGED at 213 —
the drop is fewer weighted register-diff penalties, not fewer raw
instructions; consistent with the pass-attribution finding that the
FIRST use site's own spelling cannot fix a fold blocked by LATER,
independent uses of the same intermediate). Banked as the new
candidate.c body. A second variant, writing the base expression as an
array subscript (`((s32*)&_ss_score)[a0]`) instead of the manual
shift-chain, measured WORSE (143/233 — see
rejected/array-index-base-expr-s3.md).

**KILL (instance, this chassis, s3):** neither of the two
base-address-expression spellings measured this session closes the
3-vs-2-insn gap, and per the pass-attribution finding this is EXPECTED
(not just an unlucky miss) — the true blocker is the shared liveness of
the sign-extended-a0 intermediate across ALL FOUR SS_SCORE_FLAG-macro
re-derivation sites in the function, not the syntax of the first
`base = ...` statement alone.

**Next probe (untried, register-alloc modality):** share the shifted
bank-pointer explicitly across all 4 re-derivation sites (a single
cached `s32 *bank_ptr` dereferenced at each SS_SCORE_FLAG-equivalent
site) instead of re-invoking the macro fresh each time — genuinely
different C shape, not yet measured. Caveat: H3 already confirmed BB2's
own asm DOES re-derive the pointer fresh (not reuse base) at those
sites, so this probe must preserve that byte-level fresh-pointer
re-derivation while only sharing the a0-sign-extension sub-step — which
may not be independently controllable from C (CSE/combine decide
sub-expression sharing, not the programmer). Flagged for next session to
confirm or refute with a direct measurement, not further dump-reading.

- [s2] Chassis re-verify: candidate.c (s2 form) reproduces sandbox score 136 exactly on this session's HEAD - the ledger's floor was not stale.

- [s2] Inlining the base-address computation to match _SsSeqPlay's single-expression style (main.c:399) drops the score to 130 (build_insns unchanged 213 vs target 200) - banked as the new candidate.c.

- [s2] An array-subscript rewrite of the same base expression measures worse (143/233) - banked to rejected/array-index-base-expr-s3.md, do not re-propose.

- [s2] H4's pass attribution is now DONE (was the #1 open item from s2): tmp/grind/_SsSndCrescendo/dumps/main.combine confirms the 3-vs-2-insn bank-index-shift gap is a combine.c local-fold-window refusal caused by the sign-extended-a0 intermediate having later independent consumers from the 4 SS_SCORE_FLAG macro re-derivation sites in the function body.

- [s2] Neither measured base-expression spelling can close the gap because the blocker is structural (multiple LATER independent uses of the shared intermediate across the whole function), not the syntax of the FIRST use site - the correct next lever is sharing the intermediate itself across the 4 re-derivation sites, not respelling the top-of-function computation.
