---
name: no-new-park-categories
paths: ["src/*.c", "engine/queue.py", "engine/cheats.py"]
description: "Anti-cheat policy and the single authority for the FROZEN SOTN-accepted family list: no new cheat-tolerant categories, cheats by any spelling are cheats, auto-search finds are proposals."
metadata:
  type: rules
---

# No cheat-tolerant categories; the frozen SOTN family list

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
it ([[judge-sole-gate]]), or rotation ([[rotation-not-foreclosure]]). A proposed frozen-list
extension is FAIL(CONSTRUCT) + a borderline entry.

## Cheats by any spelling

The detectors (`engine/volatile_cheats.py`, `engine/inlineasm.py`) catch literal forms; they
are a backstop, the standard is intent. Cheat signals: no semantic purpose (names like `pad`,
`buf`, `dummy`, `spill`, `slack`, `_unused`, `_tmp`); dead in the output while changing
decisions upstream of DCE; "necessary" only because removing it raises the score; justified
only by GCC internals. Outside the list below (and [[ordinary-c-judge-decidable]]) it is refused.

## Auto-search output is a PROPOSAL

Vet every permuter/sweep/enumerator closing form before surfacing it: (1) a catalog construct,
directly or by analogy? (2) code with no semantic purpose? (3) would a programmer write it from
a specification? (4) is it justified by GCC internals rather than program logic? Any yes ⇒
reject it yourself and bank it as rejected. Layer-2 is still mandatory.

## The FROZEN SOTN-accepted family list (owner-only to extend)

Every entry below is the whole list. Its prerequisites live in the linked rule and are
mandatory; "standard prerequisites" = documented lever exhaustion, a named GCC-pass mechanism,
the stated `/* FAKE: ... */` annotation, layer-1 + layer-2. Read the rule before using or
judging an entry; never generalize from one entry.

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
   no-semantic-purpose wrapper (`for(i=0;i<1;i++)`, `while(1){..break;}`, `if (1)` are not).
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
    exhaustion ledger. The `+= 2 / -= 1` respelling stays banned.
17. **F7 common store duplicated into both arms**: values real and required; annotation.
18. **Phantom-frame-slot volatile pad** (+ Q35 trailing sibling array) —
    [[phantom-frame-pad-family]].

Entries 1-7: 2026-06-02 research; 8: 2026-08-17; 9-14: 2026-07-01; 15-18: 2026-08-18 (SOTN
surveys @8bd7c777). Narrow per-function rulings (sprintf, vmNoiseOn) live in
[[ordinary-c-judge-decidable]].

**Refused (stand):** fabricated dead call site `if (0) { call(); }` (2026-08-17); F1
constant→local→local staging; F2 signedness-split dual read; F4 cross-symbol arithmetic
(except the two named Q63/Q73 cases in [[aggregate-merge-family]]); F5 union-constructor
CLOBBER; register pins; hardcoded-`$N` asm; build-time rewriting; alias renames; redundant
width casts.

## Owner ruling 2026-09-30 — SOTN precedent suffices

A construct verifiably present in matched PS1-build SOTN code is admissible on that citation,
even over an older refusal (Q55), with its family's paperwork still owed (Q53); manual-path
layer-2 only. Conditions and Q51/Q52: [[sotn-precedent-suffices]].

Related: [[ordinary-c-judge-decidable]] · [[review-discipline-before-commit]] ·
[[judge-sole-gate]] · [[register-alloc-pure-c]]
