# Evidence bank — func_8001F938

## s1 recon (2026-07-23) — floor 11 -> 5 measured with clean pure-C levers
- HEAD honest floor CONFIRMED 11 (sandbox --disable all, 107==107, 13 rules).
- Instruction-level diff (build vs target) localizes the entire gap to TWO regions:
  KIND area (+0x6A: target has redundant `lhu $a1; andi $v1,$a1,0xFFFF` + uses
  $5/$3; HEAD reads straight into $3) and the +0x270 ternary.
- kind-split lever `u32 kind_full=lhu; kind=kind_full&0xFFFF` -> floor 6 (matches
  the whole KIND region). BORDERLINE construct, needs cheat-reviewer.
- kind-split + branch-sense flip (`if(probe>=4)raw_or_3=3; else raw_or_3=(u16)*p;`)
  -> floor 5. The branch-flip is unambiguously clean C.
- ROOT CAUSE of residual 5: target's ternary emits TWO unconditional same-address
  loads `lh $v0,0x270; lhu $v1,0x270`. GCC 2.7.2 CSE always merges same-address
  HImode reads to ONE load + in-register signedness materialization. Verified both
  orders: s16-first -> lh+move+andi; u16-first -> lhu+sll;sra (floor 10, worse).
  The 2nd `lhu` is UNREACHABLE in pure C -> irreducible >=1-insn ternary gap; the
  other residuals ($v0-vs-$v1 reg, andi-vs-sll) are all coupled to it.
- Fields at +0x6A/+0x270 are ordinary game-state entity-struct fields (entity base
  D_80101EC8, stride 0x44C, per sibling func_8001EEB4) — NOT MMIO/volatile.
- GRINDABLE (floor dropped this session) -> NOT owner-gated. Next: cheat-review the
  kind-split (H1); if clean, banked floor is 5-6.


- WIP rejected_form: {'slug': 'dual-type-probe-load', 'file': 'rejected/dual-type-probe-load.c', 'floor': 2, 'build_insns': 108, 'reviewer_verdict': 'FAIL', 'construct': 's32 probe = *((s16*)(arg0+0x270)); s32 v = *((u16*)(arg0+0x270)); if (probe>=4) v=3; t=v<<16;', 'why': 'Unconditional dual-typed read at same address — codegen-steering with no semantic purpose. Cheat-reviewer default-FAIL.'}

- == imported from memory/wip notes.md ==
# func_8001F938 (code6cac.c) — WIP, NEEDS-DECISION, dual-typed-read question

## TL;DR (2026-06-15)
Comprehensive 41-variant sweep this session. Discovered the pure-C lever set
that drops the honest sandbox floor from HEAD's 11 to **2** (with build_insns
108, off by 1 from target's 107) and to **4** with matched insn count. The
floor-2 candidate (saved in `rejected/dual-type-probe-load.c`) was **FAILED by
the adversarial cheat-reviewer** — the deciding construct is an unconditional
dual-typed read of the SAME address (`s16 probe` + `u16 v` at +0x270), judged
as codegen-steering with no semantic purpose. HEAD's pure-C body ALSO contains
a dual-typed read at +0x270, but in a guarded-ternary form. **Whether that
construct (already in HEAD) is sanctioned is the open question this session
surfaces.**

## What works
Two pure-C levers reduce the honest distance meaningfully:

1. **kind-split** — `u32 kind_full = lhu; u32 kind = kind_full & 0xFFFFU;`
   provokes target's `lhu $a1, 0x6A($a0); andi $v1, $a1, 0xFFFF; ...` shape
   (target's redundant andi after lhu). Drops 5 distance vs HEAD when applied.
   Reviewer noted this as codegen-steering but secondary; treated as dual-use
   with split-init-accumulation.

2. **preload + conditional override + uniform shift** —
   `s32 v = *((u16*)(+0x270)); if (probe>=4) v=3; t = v<<16;` provokes target's
   `addiu $v1,$zero,3; sll $v0,$v1,16` else-branch shape (preventing GCC's
   lui-fold of `3 << 16`). Reuses `v` across both arms so the constant doesn't
   fold. Drops another 2-3 distance.

Combined: floor 2 with bi=108.

## The reviewer FAIL
Reviewer's deciding evidence:
> `s32 probe = *((s16 *)(arg0 + 0x270)); s32 v = *((u16 *)(arg0 + 0x270));` —
> Both load the same address with different sign types. `probe` is used only
> for the `>= 4` branch guard; `v` is used for the subsequent shift arithmetic.
> A single variable with one load would produce identical program semantics.
> The split exists to produce distinct RTL nodes (signed vs. unsigned memory
> access) to influence GCC's instruction selection. Not on the SOTN-accepted
> list. Test 1 (semantic purpose) and Test 2 (human-programmer) both fail.

Verdict: FAIL. Saved to `rejected/dual-type-probe-load.c`.

## The wrinkle — HEAD already has it
HEAD's pure-C body for the saturating ternary is:
```c
s32 probe = *((s16 *)(arg0 + 0x270));
s32 raw_or_3 = (probe < 4) ? (s32)*((u16 *)(arg0 + 0x270)) : 3;
idx = ((raw_or_3 << 16) >> 15);
```
This is **also** a dual-typed read at +0x270 — just in a guarded-ternary form
(the `(u16)` read only executes when `probe < 4`). The reviewer flagged the
unconditional split-into-separate-variables form but did not opine on the
HEAD guarded form. If the construct is fundamentally a cheat, HEAD's pure-C
body is ALSO suspect — the 13 regfix rules just paper over GCC's actual emission
of that source. If the construct is sanctioned (since it's already in main
and this function was "retired" with it 2026-05-14), then the v36 form can be
shipped as candidate.c with the 13 rules retired.

## Levers tried that DIDN'T help

- **Single-read variant** (`s32 v = *((s16*)(+0x270)); if (v >= 4) v = 3; t =
  v<<16;`) — scored 8, bi=104. Removes too many insns; target really has both
  the lh and the lhu.
- **Volatile-qualified u16 read** — scored 5. Volatile changes load semantics
  and emits extra work elsewhere.
- **Two-pointer alias** (`const s16 *p_s; const u16 *p_u; *p_s; *p_u;`) —
  same score as the single-pointer form. GCC's 2.7.2 CSE still merges.
- **Memory union at +0x270** — scored 4 (no better than v30); union didn't
  unlock additional savings.
- **Shift inside each if/else branch** with const else (no preload) — scored
  4 or 5; the `else` branch's `3 << 16` always folds to `lui`.

## Resume steps
1. **READ THIS FIRST.** The deciding question is the dual-typed read.
2. If user sanctions the dual-typed-read pattern for this function (it's
   already in HEAD), apply `rejected/dual-type-probe-load.c` to
   `src/code6cac.c`'s `func_8001F938`, retire the 13 regfix rules with `retire
   func_8001F938`, and verify oracle. The candidate has bi=108 (1 extra insn:
   the `andi $v1,$v0,0xFFFF` that materializes u16 from the s16-loaded $v0
   because GCC CSE'd the two reads). That extra insn is the LAST remaining
   gap; without it bytes would match. New lever idea: try a memory clobber
   that forces a second load — but that itself may be a cheat.
3. If user rejects the dual-typed read, this function is structurally not
   pure-C-closable under GCC 2.7.2 (CSE merges the two reads) and warrants
   either parking-with-evidence or canonical-asm authorization for the ternary
   region.

## Pointers
- `rejected/dual-type-probe-load.c` — the floor-2 form + full reviewer rationale
- `.claude/rules/inline-asm-policy.md`, `.claude/rules/codegen-technique-index.md`
- `memory/feedback/split-init-accumulation-sanctioned.md` — the closest
  sanctioned analogue
- 27-commit history: `git log --all -- src/code6cac.c | grep F938` (and the
  2026-05-14 "retire ... un-pin stabilizer" commits that left these rules).


- [s1] HEAD honest floor confirmed 11 (sandbox --disable all, 107==107 insns, 13 rules dropped).

- [s1] Instruction-diff localizes the entire 11-gap to two regions: KIND (+0x6A) and the +0x270 saturating-ternary.

- [s1] kind-split lever alone: floor 6; kind-split + branch-flip (clean): floor 5, build_insns 108.

- [s1] Residual 5 after clean levers all trace to ONE cause: target's ternary has two unconditional loads `lh $v0,0x270; lhu $v1,0x270`; GCC 2.7.2 CSE always keeps ONE same-address HImode load and materializes the other signedness in-register (sign->zero via andi, zero->sign via sll;sra).

- [s1] Verified both read orders: s16-first -> lh+move+andi; u16-first -> lhu+sll;sra (floor 10). No pure-C spelling produced two loads.

- [s1] The prior floor-2 form (rejected/dual-type-probe-load.c, reviewer-FAILED) also did NOT produce two loads; its win over floor-5 was register/branch shape from the unconditional preload steering.

- [s1] Fields +0x6A/+0x270 are ordinary game-state entity-struct fields (entity base D_80101EC8, stride 0x44C, per sibling func_8001EEB4) - not MMIO/volatile.

- [s1] Function is GRINDABLE (floor dropped this session) - NOT owner-gated.

## s2 structural (2026-07-23) — floor-5 was NOT clean; honest clean floor is 8; dual-read -> ruling
- [s2] Applied candidate (kind-split + branch-flip + guarded dual-typed read): sandbox floor 5, build_insns 108 CONFIRMED.
- [s2] Instruction diff pins the ENTIRE residual to .L8001FA60 (+0x270 clamp/index). All other regions byte-match. Coupled fixpoint: target's `lhu $v1,0x270` (2nd same-address load, also fills lh's load-delay slot) is the SINGLE lever; if emitted, all 5 residual insns collapse (nop, move, andi->sll, li v0->v1, sll reg). Build CSE-merges to one `lh` + `move`+`andi`.
- [s2] union { s16 s; u16 u; } at +0x270: floor 5, still ONE load. GCC 2.7.2 CSE merges union same-offset dual-typed reads too. KILLED (frontier item 2 union sub-probe).
- [s2] CLEAN single-read form (else arm reuses `probe`, no re-read): floor 8, build_insns 105. This is the honest floor using ONLY reviewer-PASSED constructs. The s1 "floor 5 clean levers" was mischaracterized — 5 always relied on the dual-typed read in the else arm.
- [s2] Fresh adversarial cheat-reviewer: kind-split = PASS (split-init family); guarded dual-typed read = FAIL (Tests 1/3/5, no semantic purpose — s16 and u16 provably equal in-range; only justification is "GCC CSE would merge"). Reviewer routes CSE-defeat-via-dual-typed-access to the OWNER as a NEW borderline-construct family needing SOTN evidence + ruling. (tmp/grind/func_8001F938/s2/cheat_reviewer_verdict.txt)
- [s2] Frontier item 2 EXHAUSTED: no semantically-purposeful pure-C construct keeps two loads. The two target loads are adjacent with NO intervening op (no legit memory-invalidation); +0x270 is an ordinary u16 game-state accumulator (func_80027438 `*(u16*)(a0+0x270)+=a2`), no dual-view field/union member/MMIO. Every two-load source is either CSE-merged (union/alias/single-read) or the reviewer-FAILED dual-typed read.
- [s2] => Frontier item 3 active: the target PROVABLY contains a construct (adjacent lh+lhu same address) the reviewer's Test-1 rejects. RULING-REQUEST emitted. src/ left at the CLEAN floor-8 form (no cheat in tree). candidate.c = clean floor-8 form.

## s2 structural (2026-07-23, DECISIVE update) — DISTANCE-0 FORM FOUND; structural fold-defeat KILLED
- [s2] Re-derived the WHOLE gap from full disasm (independent, not handoff): src at HEAD=floor 11; clean floor-8 form (kind-split+branch-flip+single-read) build_insns 105 confirmed. Entire residual is .L8001FA60 (+0x270). Region .L8001FA98 (+0x27E, the SIBLING index) ALREADY byte-matches with `sll,1` because its value is a SUM (vv0+vv1) GCC can't range-bound -> no fold. Region .L8001FA60 folds `(x<<16)>>15`->`<<1` because its value is a single sign-extended s16 (GCC knows the high bits).
- [s2] KEY PROOF: `(X<<16)>>15` keeps ONLY the low 16 bits before shifting, so it is signedness-INSENSITIVE to its result — s16 and u16 reads of +0x270 give IDENTICAL idx for ALL values (not just in-range). Confirms the dual read has ZERO observable effect; it only changes which load GCC keeps. Independently corroborates the reviewer's cheat verdict.
- [s2] STRUCTURAL fold-defeat probes (NEW, both KILLED): P1 `idx = raw_or_3 * 2` (literal, vs the <<16>>15 idiom) -> floor 8, build 105 (fold is spelling-invariant). P2b redundant register mask `raw_or_3 = probe & 0xFFFF` -> floor 8, build 105 — GCC ELIMINATES the mask (subsumed by <<16) then folds on probe's sign-extension knowledge. So NO register-level construct can force the opaque shape; only a second TYPED memory view of the field works. This closes the "redundant register mask" spelling that s1/s2 never tested (they only tested memory-level dual reads).
- [s2] *** DISTANCE-0 FORM FOUND *** (first ever; prior best was floor 2 / bi 108, never a true match): single u16 read + `(s16)` value cast — `s32 u=*(u16*)(a0+0x270); if((s16)u>=4)raw=3; else raw=u; idx=(raw<<16)>>15;` -> sandbox score 0, build_insns 107 == target. Disasm confirms GCC emits `lh $v0,0x270; lhu $v1,0x270; slti; sll $v1,16; li v1,3; sll v1,16; sra 15` = target .L8001FA60 byte-for-byte. This is ONE C dereference + a value cast, not two dereferences. PROVES the function is pure-C byte-closable.
- [s2] Field type: sibling clean func_80027438 (code6cac_b.c:234) accesses +0x270/+0x272/+0x26C/+0x26E as `*(u16*)`. BUT func_8001F938's target reads +0x26E/+0x272 as `lh` (signed) and indexes `sum*2`=`<<1` cleanly; only +0x270 is dual-read. +0x270 is a struct-OFFSET access (no named global / no header typedef to correct — the F2 "retype globals" remedy does NOT apply). Target's compare is signed (slti/lh); a u16 field signed-compared in C emits sltiu, so the `(s16)` cast is unavoidable -> genuine dual-view requirement, not a mistyped-global cleanup.
- [s2] Fresh adversarial cheat-reviewer on the distance-0 form: FAIL (Tests 1/2/3/4/5). "Third respelling of the same signedness-split CSE-defeat construct pre-banned in judge_constraints; F2 SOTN census (2026-07-01) already came back NOT ESTABLISHED; no ruling has landed." next_action = owner ruling-request OR canonical-asm authorization for the region. (verdict inline in outcome; form saved rejected/signed-cast-single-read.c)
- [s2] CONCLUSION: structural is EXHAUSTED for reaching < floor 8 cleanly. The +0x270 region inherently needs signed-compare + unsigned-index of ONE field; a single C value cannot be simultaneously known-signed (slti) and unsigned-opaque (no fold) without giving GCC two typed views = the reviewer-FAILED signedness-split. Not a search gap — a dichotomy. => RULING-REQUEST: a distance-0 pure-C form provably exists but only in the pre-banned F2 family (census NOT ESTABLISHED); owner must rule sanction-family / canonical-asm-authorize-region / keep-INCOMPLETE.
