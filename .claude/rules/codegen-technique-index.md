---
name: codegen-technique-index
paths: ["src/**/*.c", "include/*.h"]
description: "Symptom-keyed index of the on-demand codegen-technique rules. When a symptom matches your diff, Read .claude/rules/<slug>.md BEFORE grinding."
metadata:
  type: reference
---

# Codegen-technique index (rules are ON-DEMAND)

When a symptom matches, `Read .claude/rules/<slug>.md` first. Never use or judge a `(FAKE)`
(last resort, annotation mandatory) or `(FORBIDDEN)` construct without reading its rule.
Auto-loaded policy: completion-bar (the whole blocking tier, Q91), no-new-park-categories
(pre-cleared FAKE shapes), inline-asm-policy,
no-compiler-divergence, asm-until-matched. Read when relevant: ordinary-c-judge-decidable
(+ reused-local-one-role, reused-local-meaning-source, reused-local-necessity),
sotn-precedent-suffices, aggregate-merge-family, aggregate-declaration-views,
phantom-frame-pad-family, review-discipline-before-commit, judge-sole-gate,
rotation-not-foreclosure, decomp-loop.

## Register allocation / renames
- **register-alloc-pure-c** — reg diff or pin temptation → `-da` diagnosis; split / narrow type / precompute
- **compare-operand-order-register** — reg pair swapped across a compare → `local > GLOBAL`, not `GLOBAL < local`
- **call-return-if-result-reuse-v0** — select on a call result lands in $v1 → init result from the call
- **restore-discarded-return-displaces-v0** — post-call copy in $v0 vs $v1 → restore `return ret;`
- **drop-param-alias-local** — param→local alias keeps $a0 busy → use the param directly
- **exit-path-return-set-cse-join** — op after `move v0,sN` at a shared end → set the return in each exit
- **hoist-shared-arm-computation-defeats-copy-pref** — duplicated arm sum gets $a0 → hoist it after the if/else
- **param-reuse-base-copy-cse-canon** — param→callee-save copy + folded base copy → walk with the param
- **local-alloc-death-count-class-wall** — pure $v0<->$v1 swap, multi-load temp → read `.lreg` deaths first
- **reload-spill-reg-reveals-asm-clobbers** — reload scratch reg differs near an asm island → widen its clobbers
- **staged-value-reused-variable** (FAKE) — birthing boost misorders a load → stage via an existing dead local
- **named-local-fake-exception** (FAKE) — constant-holder / dead scalar local biases RA; no dummy subscripts
- **duplicated-statement-into-arms** (FAKE) — global-RA priority wall → duplicate a REAL statement into arms
- **dead-store-fake-exception** (FAKE) — last-resort RA/sched lever → annotated dead store to a local/param
- **pointer-alias-fake-exception** (FAKE) — redundant pointer handle to a global/param; pointer-RMW allowed
- **register-asm-pins** (FORBIDDEN) — `register T x asm("$N")` is diagnostic only

## Cross-jump / merged tails (target has MORE instructions)
- **cross-jump-call-merge** — more jalr sites in target → give each fn-ptr call its real arg count
- **cross-jump-store-tail-merge** — more `sw GLOBAL` tails → mix exit forms (goto endK + one return)
- **shared-end-label** — per-case `s2 = 0;` folded away → `goto end; end: return s2;`

## Scheduling / delay slots
- **sched-rank-class-tie-wall** — reorders can't move a load (equal priority) → refactor the expression tree
- **switch-break-shared-return-sched-hoist** — per-case `return 0;` misorders → `break;` + one shared return
- **loop-exit-work-inside-loop-sched-fence** — next loop's inits hoisted into a store region → exit work inside
- **loop-note-fixes-delay-slot-steal** — goto-loop branch target stolen into a delay slot → real while/do
- **walking-pointer-serializes-parallel-loads** — parallel loads hoisted into slots → post-increment pointers
- **hoist-call-arg-local-flips-jal-delay** — pre-call store misses the jal slot → hoist the arg into a local
- **store-before-jal** — target stores in the jal slot and reloads → store, then reload inside the call expr
- **defer-store-past-later-compute-into-jal-delay** — early sw, target in a later jal slot → store after compute
- **fake-varargs-explicit-homing** — printf wrapper homes body-scheduled → named args + explicit stores
- **legitimate-volatile-interrupt-touched** — volatile on game state or a local → IRQ test, SOTN cite or byte proof
- **mmio-volatile-type-level** — 0x1F801000-0x1F802FFF registers → volatile at declaration, any shape
- **do-while-zero-exception** (FAKE) — any codegen effect natural geometry can't reach → `do {} while (0)`
- *loop-counter-fills-load-delay* (no file) — nop in an accumulator `lw` slot → `val = *(p+off); off += 0x10; i++; sum += val;`

## Optimizer folds (CSE / combine / LICM)
- **defeat-licm-hoist-var-reuse** (FAKE) — target recomputes an invariant GCC hoists → reuse one variable
- **defeat-combine-symbol-fold** — `%lo(sym+K)` fold loses a slot → pre-compute the displaced pointer
- **hoist-flag-load-defeat-add-combine** — two `p += K` merged → hoist the flag load between them
- **split-read-defeats-hoist** — offsets hoisted across a switch → duplicate the read into the arms
- **store-const-reload-cse** — ours `li N`, target reloads the global → re-read the global
- **explicit-rejection-set-defeats-range-fold** — u16 range exclusion → `a != K && a != K+1`
- **cse-block-extension-controls-fold-span** — CSE forwards across a join → real if/else with jump+BARRIER arm
- **loop-rotation-two-shift** — loop shift appears twice → natural for-loop + opaque `one`
- **or-tree-shape-shift** (FORBIDDEN) — enumerating operand orders of assoc/comm expressions
- **proven-spelling-class-reconstruction** — same-bytes respelling only with mechanism proof

## Width / addressing / layout / declarations
- **split-scalars-hide-aggregate** — adjacent D_ scalars via pointer / `p - N` → declare the real aggregate
- **header-type-correction-from-use-sites** — casts compensating a mistyped global → fix the extern (4 prongs)
- **phantom-slot-frame-lever** — target frame has untouched 8/16/24 bytes → producers + `.frame` gradient
- **dead-vars-local-array** (FAKE) — unused-array frame coercion FORBIDDEN; only its narrow carve-outs
- **halfword-index-srl-sra** — masked s16[] index srl vs sra → byte pointer + byte offset
- **u16-global-lhu-lbu-low-byte** — lhu vs lbu on a u16 global → read `*(u8 *)&G`
- **narrow-stack-param-subword-offset** — stack param at slot+2 vs +0 → declare s32, read `*(u16 *)&arg`
- **narrow-byte-args-packed-call** — byte-packed call args → `u8` params; named `hi`/`lo`, hi first
- **switch-vs-ifchain-branch-sense** — one case's branch sense inverted → real switch

## Asm / build infrastructure
- **canonical-asm-authorization-recipe** — whole-body `glabel` canonical form (authorized routes only)
- **canonical-gate-distance-not-evidence** — a big distance is NOT evidence for ASM routing
- **cop2-addressing-preamble-cluster** — `addu $t4,$aN,$zero` feeding cop2 → owner cluster conditions
- **packed-multiply-cluster** — display.c packed Q12 multiply → S8 mask = asm; else the u64 recipe
- **fork-divergence-inline-asm** — our cc1 crashes on C cc1psx compiled → region island, 4-gate evidence
- **maspsx-gate-lists**, **maspsx-noreorder-stripping**, **per-file-gp-model**, **rodata-object-alignment** — build gates
- **compiler-flags-canonical** — flags are frozen; GP_FILES only by the -G8 proof
- **permuter-directives** — plateau or 2+ candidate spellings → PERM_* macros; vet every find

## Retired / forbidden (history at tag `pre-slim-2026-10-01`)
Never revive: placeholder `move %0,%1` + pins, hardcoded-`$N` injection, `negu` strength-reduce
asm, the `ret_val` + `goto end` prologue trick, dead-branch scheduling, reversed param-alias
pair renames, the gte-3x3 / scratchpad-gte pin+barrier recipes, chained-accumulation (Q17),
bitfield-order flipping (moot since `-mel`), the jtbl rodata-split carve-out.

New technique rule: on-demand `paths:` (its own file) plus one line here.
