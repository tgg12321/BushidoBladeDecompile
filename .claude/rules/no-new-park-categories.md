---
name: no-new-park-categories
paths: ["src/*.c", "engine/queue.py", "engine/cheats.py"]
description: "Anti-cheat policy: no new park categories, no build-time rewriting, auto-search finds are proposals; the pre-cleared FAKE shapes (examples under completion-bar item 3, owner ruling Q91)."
metadata:
  type: rules
  tier: blocking
---

# No cheat-tolerant categories; the pre-cleared FAKE shapes

Owner (2026-06-01): *"If it's a cheat, it will not be accepted. Full stop."* Bar: SOTN
standard: pure C, or canonical-body asm only for code that was originally hand-written asm.

## Forbidden

- **New park / "infrastructure" categories** retiring functions with a cheat or a gap
  (register-rotation walls, cross-jump walls, prologue save-order, RA plateaus). These are
  pure-C problems not yet solved ([[no-compiler-divergence]]).
- **Build-time assembly rewriting in ANY form** (the retired regfix/asmfix class): no
  output-transforming rule/config files, no stage between cc1 and the linker, no per-function
  Makefile/engine byte edits, no prebuilt `.o`/asm substitution for a function claimed as C.
- **Speculative rodata/link reorders** to force a match (evidence-based TU re-attribution is
  legitimate).

A stuck function gets more search, the canonical-asm grant path if hand-coded signals support
it ([[judge-sole-gate]]), or rotation ([[rotation-not-foreclosure]]).

## Cheats by any spelling

The detectors (`engine/volatile_cheats.py`, `engine/inlineasm.py`) catch literal forms; they
are a backstop, the standard is intent. Cheat signals: no semantic purpose (names like `pad`,
`buf`, `dummy`, `spill`, `slack`, `_unused`, `_tmp`); dead in the output while changing
decisions upstream of DCE; "necessary" only because removing it raises the score; justified
only by GCC internals. Since owner ruling Q91 such a construct is admissible only as a labelled
hack: `/* FAKE: <measured reason> */` at the site, nothing false asserted, simplest known form
([[completion-bar]] item 3). Unlabelled, or dressed up with a purpose it lacks, it is refused.

## Auto-search output is a PROPOSAL

Vet every permuter/sweep/enumerator closing form before surfacing it: (1) a catalog construct,
directly or by analogy? (2) code with no semantic purpose? (3) would a programmer write it from
a specification? (4) is it justified by GCC internals rather than program logic? Any yes ⇒
reject it unless it is kept as a labelled FAKE that meets [[completion-bar]] items 2-5 (and is
the simplest known form); bank the rest as rejected. Layer-2 is still mandatory.

## The pre-cleared FAKE shapes (owner ruling Q91: examples, not the boundary)

These shapes are known SOTN-accepted forms. Under [[completion-bar]], when an entry's
construct is used with no semantic purpose, its BLOCKING part is the `/* FAKE: ... */` label
with its measured reason, honesty (item 3) and the wall (item 2); its linked rule's other
prerequisites (exhaustion dossier, named GCC pass, frame proofs, symbol retirement) are
HYGIENE. No label where the construct is truthful: entry 12 (MMIO typing, per
[[mmio-volatile-type-level]]); entry 8's real merge (the Q63/Q73 admissions and second-view
exceptions keep the labels their own terms require: [[aggregate-merge-family]],
[[aggregate-declaration-views]]); entries 1, 4, 6 with a truthful reading (Ruling 1(3)). A construct not listed here is judged directly
on completion-bar item 3. "Standard prerequisites" below means: the label (when the construct
has no semantic purpose) and honesty (blocking); exhaustion, named mechanism (hygiene).

1. **Variable reuse for codegen control** — [[defeat-licm-hoist-var-reuse]].
2. **Opaque arithmetic variable** (`s32 one = 1;`) — [[loop-rotation-two-shift]]; never a
   dummy subscript/offset (Q22).
3. **Sub-word param read** `*(u16 *)&local` — [[narrow-stack-param-subword-offset]].
4. **Mixed exit forms** (`goto endK` + inline `return`) — [[cross-jump-store-tail-merge]].
5. **Duplicate read into branch arms** — [[split-read-defeats-hoist]].
6. **Named intermediate** ([[narrow-byte-args-packed-call]] hi/lo): a fresh named local for a
   sub-expression, via any GCC pass, ALL of: (1) once-written (any reads; multi-write only via
   [[ordinary-c-judge-decidable]] Rulings 5-12 or Q51, which then govern exclusively); (2) a
   real value from the target's bytes, only relocating where it is named (no-op copies are
   the dead-store family; param copies only via Ruling 12); (3) byte-neutral
   (`build_insns == target_insns`); (4) fresh, not a borrow ([[staged-value-reused-variable]]);
   (5) destination not live-pre-initialized; (6) standard prerequisites. No extra handles to
   one object.
7. **`do { } while (0)` wrap**, any codegen effect — [[do-while-zero-exception]]; the ONE
   pre-cleared wrapper. Other wrappers are judged on [[completion-bar]] item 3; `if (1) { }`
   is detector-stripped (`find_always_true_if_scaffolds`), so it cannot satisfy item 1.
8. **Per-word splat symbol → aggregate merge**, with its second-view exceptions —
   [[aggregate-merge-family]], [[aggregate-declaration-views]].
9. **Dead store / self-assign to a local or param** — [[dead-store-fake-exception]]
   (store-level deadness, Ruling 2).
10. **Constant-holder / dead scalar local** — [[named-local-fake-exception]].
11. **C-level pointer alias to a global** — [[pointer-alias-fake-exception]] (`asm("Sym")`
    renames stay forbidden).
12. **Type-level MMIO volatile** (0x1F801000-0x1F802FFF) — [[mmio-volatile-type-level]]; game
    state keeps [[legitimate-volatile-interrupt-touched]].
13. **Duplicated statement into arms** — [[duplicated-statement-into-arms]].
14. **Written-never-read local array** (target holds the stores) — [[dead-vars-local-array]].
15. **F3 compound-address duplication** across call arg-lists (`&base[i] + k` written at each
    argument instead of a pointer local): value consumed at each site; annotation at the site.
16. **F6 cancellation pair / redundant condition**: exactly `i++; i--;` adjacent, or an
    empty-if / redundant condition (`if (!i) { }`, `if (p && p)`); `!FAKE` annotation;
    exhaustion ledger (hygiene since Q91).
17. **F7 common store duplicated into both arms**: values real and required; annotation.
18. **Phantom-frame-slot volatile pad** (+ Q35 trailing sibling array) —
    [[phantom-frame-pad-family]].

Entries 1-7: 2026-06-02 research; 8: 2026-08-17; 9-14: 2026-07-01; 15-18: 2026-08-18 (SOTN
surveys @8bd7c777). Narrow per-function rulings (sprintf, vmNoiseOn) live in
[[ordinary-c-judge-decidable]].

**Refused (stand):** the wall ([[completion-bar]] item 2) — register pins; hardcoded-`$N` asm;
build-time rewriting; alias renames. Semantic lies and fabrications (items 3-4) — fabricated
dead call site `if (0) { call(); }` (2026-08-17); F4 cross-symbol arithmetic (one symbol's bytes
reached through another's address; the per-function Q63/Q73 admissions in
[[aggregate-merge-family]] stand); F5 union-constructor CLOBBER (an invented object model);
volatile outside its catalog.
**Re-judged under Q91** (formerly refused as no-semantic-purpose shapes outside the list): F1
constant→local→local staging, F2 signedness-split dual read, redundant width casts and the
`+= 2 / -= 1` respelling are judged on [[completion-bar]] items 2-5 — labelled, nothing false,
existing types used, simplest known form.

## Owner ruling 2026-09-30 — SOTN precedent suffices

A construct verifiably present in matched PS1-build SOTN code is admissible on that citation,
even over an older refusal (Q55): manual path only (never on a Judge PASS), `/* FAKE */`-
labelled, simplest known form, fresh layer-2 (Q53 as tiered by Q91: exhaustion and
byte-neutrality write-ups are hygiene). For constructs [[completion-bar]] item 3 already
admits, a citation is supporting evidence, not a requirement. Conditions:
[[sotn-precedent-suffices]] (1)-(4) and its `/* SOTN: */` tag.

Related: [[completion-bar]] · [[ordinary-c-judge-decidable]] · [[review-discipline-before-commit]] ·
[[judge-sole-gate]] · [[register-alloc-pure-c]]
