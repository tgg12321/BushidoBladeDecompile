---
name: techniques-audit-2026-06-02
description: "Systematic audit of every active rule in .claude/rules/ against the 2026-06-01 cheats-by-any-spelling policy. 3 rules flagged FORBIDDEN-BY-POLICY, 6 rules flagged BORDERLINE, 22 active rules clean. Affected COMPLETED-C functions enumerated. Confidence ~80%."
metadata:
  type: project
---

# Techniques audit — 2026-06-02

## Why this audit exists

The user reclassified the **dead-goto label-pad** technique (in
[[global-label-drift-sibling-cheat]]) as forbidden per the
[[no-new-park-categories]] "cheats by any spelling" policy. They asked for a
systematic sweep of every documented technique in `.claude/rules/` to flag
anything else that should also be considered a cheat under the same lens — to
avoid the "find out a week later we were farther out than we thought" pattern.

The audit applies the policy lens from [[no-new-park-categories]]:

- A technique is **FORBIDDEN-BY-POLICY** if its "fix" code construct has no
  semantic purpose, exists only to influence GCC's compilation, a human
  programmer would not naturally write it, and the justification references
  GCC internals (allocator priority, INSN_PRIORITY, label_num, LUID, pseudo
  numbering) as the mechanism.
- **BORDERLINE** = sits between the two; a defensible human-readable reading
  exists but the technique-as-applied is overtly codegen-driven.
- **LEGITIMATE** = real semantic structure, natural C, SOTN-acceptable.

## Phase 1 — enumeration

40 rule files in `.claude/rules/`. 6 already marked
`status: forbidden` / "ARCHIVED FORBIDDEN" in frontmatter or body (skipped):

- `dead-branch-scheduling.md` — ARCHIVED FORBIDDEN
- `dead-vars-local-array.md` — FORBIDDEN (2026-05-31, expanded 2026-06-01)
- `gte-3x3.md` — ARCHIVED FORBIDDEN
- `inline-move-aliasing.md` — ARCHIVED FORBIDDEN
- `scratchpad-gte.md` — ARCHIVED FORBIDDEN
- `strength-reduce-defeat.md` — ARCHIVED FORBIDDEN

`global-label-drift-sibling-cheat.md` is partly current — its `{lbl#N}` slot
mechanism is legit, and only the dead-goto-pad / DImode-chain SUB-techniques
are forbidden as of 2026-06-02. Listed as LEGITIMATE for the diagnostic /
`{lbl#N}` content; the forbidden sub-content is already correctly flagged.

## Phase 2 — per-rule evaluation (33 active rules)

### FORBIDDEN-BY-POLICY (3 rules)

#### 1. `goto-end-prologue-delay-slot.md` — FORBIDDEN

Teaches: rewrite `if (x == NULL) return 0;` as `s32 ret_val; if (x == NULL) {
ret_val = 0; goto end; } ... ret_val = …; end: return ret_val;`. The shared
`end:` label is explicitly documented as "load-bearing" so the body's final
return goes through it.

Why this is a cheat: the function has **one return value** and **one true
return path** semantically (the body computes it; the early-exit returns 0).
The `ret_val` accumulator + shared `end:` label do not encode any new program
behavior — `return 0;` and `return ret_val;` (with `ret_val=0`) produce
identical observable behavior. The structure exists **solely** to make
`reorg.c` fill the `bnez`'s delay slot with the prologue's `addiu` and the
`j`'s delay slot with the zero-assignment. The rule's justification cites
"`reorg.c` (delayed-branch pass)", "the delay-slot candidate", "GCC's `jump.c`
may try to thread the `goto end`" — pure GCC-internals reasoning. The
construct names itself a "lever". Identical-intent sibling to the now-forbidden
dead-goto label-pad and DImode chain. A human writing the function from its
spec would write `if (arg0 == NULL) return 0;` and let the compiler emit
whatever — not invent an accumulator variable just to bend reorg.c.

Caveat: a `goto end; end: cleanup;` pattern with REAL shared cleanup work
(closing handles, freeing memory, etc.) at `end:` is a legitimate C idiom and
this audit does NOT flag that — only the specific bare `end: return ret_val;`
with zero shared work is flagged.

#### 2. `param-local-alias-prologue-pair-flip.md` — FORBIDDEN

Teaches: `void func(s32 *out, Rect *r) { Rect *_r = r; s32 *_out = out; /*
rest of body uses _r / _out */ }` to flip the prologue save+def pair order
by declaration LUID manipulation.

Why this is a cheat: `_r` and `_out` are literal renames of `r` and `out` —
identical lvalues, identical types, no extra information conveyed. The
function's behavior is byte-identical with or without the aliases. The
declaration order exists **solely** to manipulate cc1's `expand_function_start`
LUID assignment so `save_restore_insns` emits the prologue HIGH-to-LOW pair
order matching target. The rule's mechanism section is the textbook cheat
signal: "the C declaration order of the aliases controls the LUID of their
`move` RTL insns" — describing what it does without referencing program logic,
only RTL emission order. A human programmer writing this function from a
spec would write `void func(s32 *out, Rect *r) { /* use out, r directly */ }`
— they would not capture parameters into same-typed locals with leading
underscores for prologue scheduling.

#### 3. `global-label-drift-sibling-cheat.md` (dead-goto label-pad +
DImode-chain sub-techniques) — already FORBIDDEN (2026-06-02)

Already correctly flagged in the rule file body itself. Listed here for
completeness; no new action needed. The `{lbl#N}` slot mechanism in the same
file is LEGITIMATE.

### BORDERLINE (6 rules)

#### 4. `defeat-licm-hoist-var-reuse.md` — BORDERLINE-LEANING-FORBIDDEN

Teaches: reuse one C variable `tmp` for **two unrelated values** (one a real
loop-variant computation that gets stored, the other `limit - 1` for the
loop bound) to make GCC's loop.c see the pseudo as multi-set, so it's not a
movable, so the `limit-1` isn't hoisted.

Why borderline: the example confirmed case (`func_8003DBE4`) explicitly says
"reuse `tmp` for the `*pal` value and `limit-1` to defeat the loop-invariant
hoist". The variable reuse has no semantic motivation — the two values are
unrelated. A human writing the function would naturally use two different
variables OR write `if (i == limit - 1)` directly. The rule reads "**make the
invariant's pseudo MULTI-SET**" as the lever. Justification cites
`loop.c line ~705` (`n_times_set==1` test) and `loop.c line ~715`
(`may_trap_p` exclusion) — pure GCC-internals reasoning.

Why not outright forbidden: variable reuse for a SCRATCH role is a real C
idiom — programmers DO reuse variables for unrelated temporaries to save
declaration noise. The line is whether the reuse expresses intent ("this is
my scratch slot") or coerces ("this defeats loop.c's movable detection").
The rule's framing puts it squarely on the coercion side. **Recommendation:
forbid the specific "reuse `tmp` for unrelated values to defeat hoist"
guidance; keep the `func_8003DBE4` match's other levers (dead local array
is already forbidden, redundant dead store is already forbidden as Lever D)
— that function may need re-derivation.**

#### 5. `loop-rotation-two-shift.md` — BORDERLINE (the `s32 one = 1;` opaque
variable)

Teaches two things:
- Use a real `for` loop (legit — natural C control flow)
- Declare `s32 one = 1;` instead of writing `1 << i` directly, to defeat
  GCC's `(arg0 & (1 << i)) → (arg0 >> i) & 1` transform

Why borderline: the for-loop part is fine. The `s32 one = 1;` is a textbook
opaque-variable cheat: no semantic purpose (it equals literal 1), exists
**only** to keep GCC's combine pass from rewriting the bit-test, and the
rule says so explicitly ("GCC can't prove `one` is a single bit, so it keeps
the `sllv`+`and` mask form"). A human writes `1 << i`, not `one << i` with
an indirected literal 1. Naming pattern "one" announces coercion intent.

**Recommendation: forbid the `s32 one = 1;` opaque-variable sub-technique;
keep the for-loop guidance. The 3 affected committed functions
(`func_8008ACD0`, `text1a.c:443`, `code6cac_c.c:254`, `code6cac_b2.c:309`,
`code6cac_b2.c:326`, `main.c:2501`) may need re-derivation.**

#### 6. `narrow-stack-param-subword-offset.md` — BORDERLINE

Teaches: declare a `u16` parameter as `s32` and read its low half via
`u16 arg4_lo = *(u16 *)&arg4;`.

Why borderline: the technique IS faithful to a real ABI quirk (the GCC fork
right-justifies narrow stack params; cc1psx left-justified). But the C the
agent writes (`s32 arg4` + `*(u16 *)&arg4`) is NOT the C the original was
written from — the original was `u16 arg4` and the calling convention
delivered it at offset 0. The agent's `s32 arg4 + low-half cast` is a
reverse-engineered workaround for the FORK'S divergence from cc1psx — and
[[no-compiler-divergence]] is the standing policy. The justification
explicitly references the FORK's `FUNCTION_ARG_PADDING` behavior.

Why not outright forbidden: the technique IS semantically correct (the
caller passes a promoted int either way; the callee reads the low half as
specified). And the cheats-by-any-spelling test asks "would a human
programmer write this from the function's spec?" — here, a SOTN-quality
reconstruction WOULD note "the param is a u16 but the fork delivers it at
offset 2; mark it for canonical-asm or accept this workaround." The
question is whether the workaround IS the cheat or merely a faithful
encoding.

Recommendation: **escalate the policy question to the user — does the
narrow-stack-param ABI divergence count as a fork-vs-cc1psx wall (in which
case `s32 + low-half cast` is a cheat per `no-compiler-divergence`) or
a legitimate ABI re-expression (in which case it's borderline-legitimate)?**

#### 7. `cross-jump-store-tail-merge.md` — BORDERLINE-LEANING-FORBIDDEN

Teaches: mix exit forms (some `goto endK`, some inline `return D_GLOBAL;`)
so the block endings are not rtx_equal, defeating GCC's cross-jump.

Why borderline: each individual exit form is real C — both `return X;` and
`goto end;` with shared epilogue are normal idioms. But the technique tells
the agent to **deliberately mix** them across error paths that semantically
do identical things, purely to defeat cross-jump. A human writing the
original would either write all-inline or all-goto, not a contrived mix.
The rule says "match TARGET's per-tail register usage" as the criterion
for which exit gets which form — purely codegen-driven, not semantic.

Recommendation: **flag as borderline-leaning-forbidden, request user
policy decision.** The `saEft00Add` match would need re-derivation if
forbidden.

#### 8. `split-read-defeats-hoist.md` items #1 + #2 — BORDERLINE

Teaches: duplicate a shared post-flag read into both branch arms, indexing
the symbol DIRECTLY in the known arm. Item #3 (volatile coercion) already
forbidden.

Why borderline: a human MIGHT write `if (flag) use SYMBOL_A else use base`
in some cases — there is a semantic reading ("when we know which it is,
use it by name"). But duplicating the EXACT same arithmetic into both arms,
distinguished only by the base, is contrived. The justification is
explicitly "pins the offset computations inside their branch, so GCC can't
hoist them across the switch."

Recommendation: **flag as borderline; check whether `coli_HitPauseKatana_2`
(already noted as needing re-evaluation due to forbidden volatile lever #3)
also relies on the duplicate-read structure for matching.**

#### 9. `narrow-byte-args-packed-call.md` `func_8007B564` two-locals form —
BORDERLINE

The main `u8` parameter declaration is LEGITIMATE (faithful ABI
reconstruction). The `func_8007B564` extension that splits `hi = arg3<<16;
lo = (arg2<<8) | 0x80000000;` into two locals "for the priority/LUID
balance" is the coercion form. A SOTN reviewer might accept "hi/lo
expression naming" but the justification (LUID-tie-break manipulation) is
the cheat signal.

Recommendation: **soften the rule — remove the LUID rationale and the
two-locals split as a technique; if the natural single-expression form
doesn't reach pure-C close, park `func_8007B564`. The `u8`-typing main
lever stays.**

### LEGITIMATE (24 rules — clean)

These are all clean — natural C semantics, no GCC-internals justification,
or pure tooling/diagnostic rules that don't coerce code:

- `canonical-asm-retirement` (not in .claude/rules/, in memory/) — workflow
- `community-standard` (memory/) — policy
- `compiler-flags-canonical` — fact-stating ("don't flag-hunt")
- `completion-standard` (memory/) — policy
- `compiler-patch-low-roi` (memory/) — fact
- `cross-jump-call-merge` — recover the real arg count from target (legit RE)
- `debugging-discipline` (memory/) — workflow
- `difficult-is-not-impossible` — anti-quit policy
- `file-editing-wsl` (memory/) — toolchain workflow
- `gte-wrapper-misroute-park` — existing carve-out (no C form)
- `halfword-index-srl-sra` — `*(s16*)((u8*)&Tbl + offset)` is a real C
  idiom for "byte offset into halfword array"
- `hand-coded-asm-recognition` (memory/) — recognition rule
- `inline-asm-allowed` (memory/) — policy
- `inline-asm-policy` — policy (the catalog itself)
- `inline-asm-injection` — policy (declares the cheat)
- `jtbl-rodata-split-infrastructure` — existing carve-out
- `lost-codegen-insert-cheat` — policy (declares the cheat)
- `loop-note-fixes-delay-slot-steal` — rewrite goto-poll-loop as
  `while(1)/break` (natural C)
- `maspsx-label-nop-gate` — per-function allowlist for assembler blind spot
- `maspsx-noreorder-stripping` — assembler tooling rule
- `no-compiler-divergence` — policy
- `no-new-park-categories` — policy (the parent of this audit's lens)
- `packed-multiply-cluster` — S8 hand-coded signal recognition + legit u64
  multiply for non-hand-coded variants
- `programmatic-tools-first` (memory/) — workflow
- `prologue-fix-redundant-reorder` — remove redundant tooling cruft, no
  code change
- `register-asm-pins` — DIAGNOSTIC-ONLY (the rule itself forbids the pin
  in committed code)
- `register-alloc-pure-c` Levers A (block-local var split) and B (narrow
  integer type) — **legitimate**; Lever C (loop-local precompute) —
  legitimate IF the precompute has semantic purpose, **borderline** if
  it's a phantom local; Lever D (dead stores) — already FORBIDDEN. The
  "chain extender" technique
  (`idx_1494 = (u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C)`)
  surfaced in sessions 5-10 is **FORBIDDEN-BY-POLICY** — it is the same
  shape as the alias rename: a fold-to-zero-bytes RTL trick whose only
  purpose is to bump a pseudo's reference count. **Recommendation: add a
  ❌ FORBIDDEN section for the "combine-fold chain extender" sub-technique
  similar to how Lever D was retroactively forbidden.** The `permuter`
  found `cpu_side_move_dir_4`'s chain at score 16; per the policy that
  finding is now a cheat-class, not a path.
- `sandbox-zero-retire-fails` — diagnostic workflow rule
- `scoring-systems` (memory/) — reference
- `shared-end-label` — borderline-legitimate (the `goto end; ...
  end: return s2;` shape has a real semantic reading "every case sets s2;
  then return s2"; the justification cites constant-fold defeat but the
  resulting code IS readable C)
- `store-before-jal` — borderline-legitimate (expression nesting IS a
  real C structuring decision, even if the goal is delay-slot fill)
- `store-const-reload-cse` — read the global directly at the test (real C)
- `switch-vs-ifchain-branch-sense` — replace if-goto chain with real
  `switch` (natural C)
- `tool-error-handling` (memory/) — tool standard
- `token-efficiency` (memory/) — workflow
- `u16-global-lhu-lbu-low-byte` — `*(u8 *)&G` for explicit low-byte read
  (real C semantic — "I want the low byte specifically")
- `compare-operand-order-register` — borderline-legitimate. Writing
  `local > GLOBAL` vs `GLOBAL < local` is a stylistic choice that has
  real readability difference. The choice has zero semantic impact
  (mathematically equivalent), but operand ordering is a normal coding
  decision. Justification cites RTL-emission order — codegen-coupling
  signal — but the code itself stays clean. **Keep as legitimate;
  the line gets blurry if the rule is invoked dozens of times across
  a function purely to flip register assignment.**

## Phase 3 — affected COMPLETED-C functions

### Directly observable in current `src/*.c`

| Pattern | Function | File | Status |
|---|---|---|---|
| `goto end; end: return ret_val;` (goto-end-prologue-delay-slot) | `func_8007C97C` | display.c | COMPLETED-C — see commit `bc346de2c6` |
| `Rect *_r = r; s32 *_out = out;` (param-local-alias) | `func_8007C2A0`, `func_8007C4B8` | display.c | COMPLETED-C — commit `af10dc8d14` |
| `goto label_pad; label_pad: goto end;` (dead-goto pad) | `func_8007B3A8` | display.c:101 | COMPLETED-C — commit `02a0a965b6` |
| `goto done_label_pad; done_label_pad:` (dead-goto pad) | `tslPolyF4Init` | system.c:280 | already flagged ([[thorough-cheat-audit-2026-06-02]]) |
| `u16 arg4_lo = *(u16 *)&arg4;` (narrow-stack-param) | `efc_buki_draw_zanzou` | text1b.c:257 | COMPLETED-C — commit `4051a7a934` |
| `s32 one = 1;` (loop-rotation opaque var) | `func_8008ACD0` and ~5 sibling sites | main.c, text1a.c, code6cac_c.c, code6cac_b2.c (×2), code6cac_b2.c | COMPLETED-C — commits `48e83bf275`, `8df5f6fee5`, `e450e06972`, plus older ones |

### Inferred from git history (technique cited in commit messages)

- **`defeat-licm-hoist-var-reuse`** — `func_8003DBE4` (commits `9e722985e4`,
  `030a187d18`, `8bc42f2ed8`). Already noted in
  [[thorough-cheat-audit-2026-06-02]] as needing re-evaluation under the
  expanded cheat catalog due to its dead local array (Lever C/D family).
  This audit adds the loop.c-defeat sub-technique to the cleanup list.
- **`shared-end-label`** — `func_80077B30` (commits `60d8f7687e`,
  `0f206e5934`). BORDERLINE; if user accepts the "every case sets s2,
  then return s2" shape as natural C, no re-derivation needed.
- **`cross-jump-store-tail-merge`** — `saEft00Add` (commits `c322684233`,
  `624ff03e6d`). Currently uses the mixed-exit-form technique. If user
  forbids, the function needs re-derivation; if user accepts, OK.
- **`split-read-defeats-hoist`** — `coli_HitPauseKatana_2` already on the
  re-evaluation list (the volatile lever #3 was already forbidden); items
  #1 + #2 add to that.
- **`store-before-jal`** — `calc_fc_frame_800203B4` (commits `a502eb4f22`,
  `3a97f7a044`). LEGITIMATE per this audit (expression structuring is real
  C), but worth noting it relies on the technique.
- **`switch-vs-ifchain-branch-sense`** — `func_8001EFA0` (commit
  `64642bda13`). LEGITIMATE — `switch` is the natural C form.
- **`u16-global-lhu-lbu-low-byte`** — `func_8006EC0C`, `func_800550E8`,
  `func_8003504C`. LEGITIMATE per this audit.
- **`narrow-byte-args-packed-call`** — `func_8007B4D0` (LEGITIMATE for the
  `u8` typing main lever), `func_8007B564` (BORDERLINE for the hi/lo split
  sub-technique).
- **`halfword-index-srl-sra`** — `func_8001BAE4` (commits `d1eab127cf`,
  `e3c284bbc3`). LEGITIMATE per this audit.
- **`store-const-reload-cse`** — `func_8001E404` (commit `168dbe6ae7`).
  LEGITIMATE per this audit.
- **`loop-note-fixes-delay-slot-steal`** — `func_8003A450` (commit
  `81c8dd2128`). LEGITIMATE per this audit.
- **`compare-operand-order-register`** — `func_8007B3A8` (commit
  `02a0a965b6`). LEGITIMATE per this audit (but the SAME commit also
  applied the now-forbidden dead-goto label-pad — flagged separately).
- **`packed-multiply-cluster`** — `func_8007EDBC` (commit `2db22e18b8`).
  LEGITIMATE (routed to canonical-asm via S8 signal).

## Phase 4 — recommendations

### Rules to archive as FORBIDDEN (3)

1. **`goto-end-prologue-delay-slot.md`** — archive with "ARCHIVED FORBIDDEN
   (2026-06-02)" header per the standing process; the construct has no
   semantic purpose and the justification is pure GCC-internals (reorg.c
   delay-slot fill). Add `func_8007C97C` to the re-derivation list.
2. **`param-local-alias-prologue-pair-flip.md`** — archive with "ARCHIVED
   FORBIDDEN (2026-06-02)" header; the local-alias rename has no semantic
   purpose and the justification is pure GCC-internals (LUID, RTL emission
   order, save_restore_insns). Add `func_8007C2A0` and `func_8007C4B8` to
   the re-derivation list.
3. **`register-alloc-pure-c.md`** — add a ❌ FORBIDDEN section for the
   "combine-fold chain extender"
   (`x = (u8*)y + ((s32)&SYMBOL_A - (s32)SYMBOL_B)` pattern that bumps a
   pseudo's reference count via combine-foldable pointer arithmetic).
   The `cpu_side_move_dir_4` session-8 lever (D_800F19C0 store-via-tbl_125c
   chain) is the worked example, and it has the same signature as the
   alias-rename cheat — zero emitted bytes, RTL-pass-only effect, bumps
   refs to manipulate global.c allocno priority. **Note: this technique
   was never committed (the session-8 source change was reverted because
   it shifted maspsx indices), so there is no COMPLETED-C cleanup. The
   work is to prevent future agents from using it.**

### Rules needing user policy escalation (3)

4. **`defeat-licm-hoist-var-reuse.md`** — flag the `tmp = limit - 1;`
   variable-reuse-to-defeat-loop.c-movable as forbidden. Keep
   `func_8003DBE4` on the re-derivation list (already there per
   `dead-vars-local-array` Lever C/D update).
5. **`loop-rotation-two-shift.md`** — flag the `s32 one = 1;`
   opaque-variable sub-technique as forbidden. Re-derive the ~6 affected
   sites if user concurs.
6. **`cross-jump-store-tail-merge.md`** — flag the deliberately-mixed
   exit forms as forbidden. Re-derive `saEft00Add` if user concurs.

### Rules requiring clarifying edits (3)

7. **`narrow-stack-param-subword-offset.md`** — user decision on whether
   the `s32 arg4 + *(u16*)&arg4` workaround for the fork's stack-param
   right-justification is acceptable ABI re-expression or a
   no-compiler-divergence cheat. Only 1 function affected
   (`efc_buki_draw_zanzou`).
8. **`narrow-byte-args-packed-call.md`** — remove the LUID-rationale +
   two-locals split sub-technique (`func_8007B564` extension). Keep the
   `u8` typing main lever.
9. **`split-read-defeats-hoist.md`** — flag items #1 + #2 as borderline;
   the rule's status depends on user judgment whether
   `coli_HitPauseKatana_2` needed structural duplication or is
   pure-C-reachable with a single read.

### Rules confirmed clean (rest)

The remaining ~22 active rules are clean: real C semantics, no
GCC-internals-as-mechanism justification, and SOTN-acceptable C structure.

## Confidence assessment

**~80%.** I read all 33 active rules; the policy lens was applied uniformly.
The largest risk is the BORDERLINE bucket: defeat-licm-hoist-var-reuse,
loop-rotation-two-shift (opaque `one`), and cross-jump-store-tail-merge
mixed-exit are clear codegen-only constructs by the lens, but the user has
historically accepted some of these as "documented technique" — so my
verdict differs from the project's prior practice. The user should
re-read these three rules with the lens applied and confirm.

Lower-risk: the 3 forbidden items
(`goto-end-prologue-delay-slot`,
`param-local-alias-prologue-pair-flip`, the chain-extender in
`register-alloc-pure-c`) are essentially identical-in-shape to the
already-forbidden dead-goto pad and DImode chain — the same intent in
different spellings. High confidence on those.

Hidden-coverage risk: I cross-referenced commits and live source for the
syntactic patterns, but the audit is per-RULE, not per-function. A function
that uses a TECHNIQUE without the rule's signature pattern (e.g. a `goto
end;` shape that happens to fall out of natural C without the explicit
`ret_val` accumulator) would not be flagged. The
[[thorough-cheat-audit-2026-06-02]] is the per-function counterpart.

## Related

- [[no-new-park-categories]] — the parent policy this audit applies
- [[thorough-cheat-audit-2026-06-02]] — sibling audit, per-function instead
  of per-technique; together they cover the leakage surface
- [[completion-standard]] — what COMPLETED-C is allowed to contain
- [[inline-asm-policy]] — the expanded cheat catalog
- [[difficult-is-not-impossible]] — the anti-quit policy that the
  forbidden levers were originally introduced to satisfy; removing them
  pushes more functions back to "unfinished work," NOT "impossible"
