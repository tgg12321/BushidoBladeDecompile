---
name: codegen-technique-index
paths: ["src/*.c", "include/*.h"]
description: "Symptom-keyed index of the on-demand codegen-technique rules. When a symptom matches your diff, Read .claude/rules/<slug>.md BEFORE grinding."
metadata:
  type: reference
---

# Codegen-technique index (rules are ON-DEMAND)

When a symptom below matches, `Read .claude/rules/<slug>.md` first. Never use or judge a
`(FAKE)` (sanctioned last-resort, annotation mandatory) or `(FORBIDDEN)` construct without
reading its rule. Policy that auto-loads on `src/*.c`: no-new-park-categories (the frozen
family list), inline-asm-policy, no-compiler-divergence, asm-until-matched. Read directly when
relevant: ordinary-c-judge-decidable, review-discipline-before-commit, judge-sole-gate,
rotation-not-foreclosure, decomp-loop.

## Register allocation / renames
- **register-alloc-pure-c** — register diff vs target or a pin temptation → `-da` diagnosis; block-local split / narrow type / precompute
- **compare-operand-order-register** — local/global reg pair swapped across a compare block → write `local > GLOBAL` not `GLOBAL < local`
- **call-return-if-result-reuse-v0** — 2-constant select on a call result lands in $v1, branch flipped → init result from the call, test ==0
- **restore-discarded-return-displaces-v0** — post-call global copy in $v0 vs target $v1 and a caller captures the return → restore `return ret;`
- **drop-param-alias-local** — param→local alias keeps $a0 busy so a short local misses it → drop the alias, use the param directly
- **exit-path-return-set-cse-join** — op after `move v0,sN` at a shared end reads $sN → set the return value in each exit path
- **hoist-shared-arm-computation-defeats-copy-pref** — duplicated arm sum gets $a0 via copy-pref → hoist it after the if/else (jump2 re-duplicates it)
- **param-reuse-base-copy-cse-canon** — target has a param→callee-save copy plus a base copy we fold → walk with the param itself, name the call arg early
- **local-alloc-death-count-class-wall** — pure $v0<->$v1 swap, multi-load temp vs constant → death-count class wall; diagnose with .lreg, not a priority lever
- **reload-spill-reg-reveals-asm-clobbers** — reload scratch reg differs next to an authorized asm island → widen that island's clobbers to $12-$15
- **staged-value-reused-variable** (FAKE) — once-set pseudo's sched birthing boost misorders a load → stage the value through an existing dead local
- **named-local-fake-exception** (FAKE) — constant-holder / dead scalar local biases RA → FAKE-annotated scalar local, last resort; no dummy subscripts (Q22)
- **duplicated-statement-into-arms** (FAKE) — global-RA priority wall / need a ref-lift → duplicate a REAL statement into arms, byte-neutral via cross-jump
- **dead-store-fake-exception** (FAKE) — last-resort RA/sched lever after exhaustion → annotated dead store/self-assign to a local/param
- **pointer-alias-fake-exception** (FAKE) — redundant pointer handle to a global/param changes codegen → FAKE-annotated alias, last resort; pointer-RMW allowed
- **register-asm-pins** (FORBIDDEN) — `register T x asm("$N")` → diagnostic only; strip it and find the C structure that picks the register

## Cross-jump / merged tails (target has MORE instructions)
- **cross-jump-call-merge** — target has more jalr sites than build → give each fn-ptr its real arg COUNT
- **cross-jump-store-tail-merge** — target has more `sw GLOBAL` error tails → mix exit forms (distinct goto endK + one inline return)
- **shared-end-label** — per-case `s2 = 0;` dropped by constant-fold → route cases through `goto end; end: return s2;`

## Scheduling / delay slots
- **sched-rank-class-tie-wall** — operand/decl reorders can't move a load (equal sched priority) → class-compare wall; refactor the expression tree
- **switch-break-shared-return-sched-hoist** — per-case `return 0;` gives wrong RMW register / flipped branch → `break;` + one shared `return 0;`
- **loop-exit-work-inside-loop-sched-fence** — next loop's inits hoisted into a post-loop store region → move exit work inside the loop
- **loop-note-fixes-delay-slot-steal** — single-insn op at a forward branch target stolen into its delay slot in a goto loop → write a real while/do loop
- **walking-pointer-serializes-parallel-loads** — parallel-array element loads hoisted into delay slots → post-increment walking pointers (needs intervening stores)
- **hoist-call-arg-local-flips-jal-delay** — pre-call store not in jal delay slot + load-delay nops → hoist the global arg into a local declared first in a block
- **store-before-jal** — target stores in the jal delay slot and reloads after → store in its own statement, reload from memory inside the call expression
- **defer-store-past-later-compute-into-jal-delay** — one sw early, target has it in a later jal delay slot → compute into a local, store after the later compute
- **fake-varargs-explicit-homing** — printf wrapper's arg homes are body-scheduled in target → named args + explicit stores through `&fmt`, not `...`
- **legitimate-volatile-interrupt-touched** — volatile on a game-state global or local → only with cited IRQ writer + use-site shape; locals via SOTN cite or byte proof
- **mmio-volatile-type-level** — access to 0x1F801000-0x1F802FFF registers → volatile at declaration, any shape, no FAKE needed
- *loop-counter-fills-load-delay* (no file) — accumulator loop leaves a nop in the `lw` delay slot → `s32 val = *(p+off); off += 0x10; i++; sum += val;`
- **do-while-zero-exception** (FAKE) — any codegen effect where natural geometry fails → `do { } while (0)` wrap, inline FAKE annotation

## Optimizer folds (CSE / combine / LICM)
- **defeat-licm-hoist-var-reuse** (FAKE) — target recomputes an invariant inline but GCC hoists it → reuse one var for a used variant + the invariant
- **defeat-combine-symbol-fold** — displaced access emits lui+sym-K form, delay slot lost → pre-compute the displaced pointer into a local before a call
- **hoist-flag-load-defeat-add-combine** — two `p += K` merged into one addiu → hoist the test's flag load into a local before the first add
- **split-read-defeats-hoist** — register-rename plateau from offsets hoisted across a switch → duplicate the read into the flag arms, symbol direct
- **store-const-reload-cse** — ours `li N` where target reloads the global after storing N → drop the saved-local reload, re-read the global
- **explicit-rejection-set-defeats-range-fold** — u16 range exclusion: wrong reg / andi missing → write `a != K && a != K+1`
- **cse-block-extension-controls-fold-span** — CSE merges/forwards across a join the target keeps separate → real if/else (jump+BARRIER arm)
- **loop-rotation-two-shift** — target has the loop shift twice (peeled + bottom delay slot) → natural for-loop + opaque `one`
- **or-tree-shape-shift** (FORBIDDEN) — enumerated operand orders in an assoc/comm expression → natural order only; one FAKE mechanism-derived order
- **proven-spelling-class-reconstruction** — same-bytes respelling needed → only with mechanism proof the original used another spelling class

## Width / addressing / layout / declarations
- **split-scalars-hide-aggregate** — adjacent D_ scalars written via pointer local / `p - N` / wrong pointer home → declare the real array/struct (aggregate merge, no-new-park-categories)
- **header-type-correction-from-use-sites** — signedness casts compensating a mistyped global → fix the one header extern under the 4 prongs
- **phantom-slot-frame-lever** — target frame reserves untouched 8/16/24 bytes → unallocated-pseudo producers + `.frame` gradient (diagnosis, not a sanction)
- **dead-vars-local-array** (FAKE) — unused-array/(void)& frame coercion is FORBIDDEN → only the written-never-read / pad / Q35 sibling carve-outs
- **halfword-index-srl-sra** — masked s16[] index emits srl, target sra → index a byte pointer with the byte offset directly
- **u16-global-lhu-lbu-low-byte** — ours lhu vs target lbu on a u16 global → read `*(u8 *)&G` at the dispatch site
- **narrow-stack-param-subword-offset** — narrow 5th+ stack param loaded at slot+2 vs target +0 → declare it s32, read `*(u16 *)&arg`
- **narrow-byte-args-packed-call** — byte-masked args packed into one call slot → declare them `u8`; const-OR pack: named `hi`/`lo`, hi first
- **switch-vs-ifchain-branch-sense** — one dispatch case has inverted bne/beq → rewrite the if-goto chain as a real switch

## Asm / build infrastructure
- **canonical-asm-authorization-recipe** — writing a whole-body `__asm__("glabel ...")` canonical form (authorized routes only)
- **canonical-gate-distance-not-evidence** — a big distance is NOT evidence for ASM routing
- **cop2-addressing-preamble-cluster** — `addu $t4,$aN,$zero` feeding cop2 ops in the 0x8001-0x8003 band → the owner cluster ruling's conditions
- **packed-multiply-cluster** — display.c packed Q12 multiply → S8 redundant mask means canonical asm; else the u64 multiply recipe
- **fork-divergence-inline-asm** — our cc1 SIGSEGVs on the C cc1psx compiled → region-only asm island after the 4-gate evidence
- **maspsx-gate-lists** / **maspsx-noreorder-stripping** / **per-file-gp-model** / **rodata-object-alignment** — assembler gates, glabel `.set` duplication, gp model, rodata alignment
- **compiler-flags-canonical** — "maybe it's the flags" → no; flags are frozen; GP_FILES only by the -G8 proof
- **permuter-directives** — plateau or a rule offering 2+ spellings → PERM_* macros via permuter_campaign.py; vet every find

## Retired / forbidden (no file; history at tag `pre-slim-2026-10-01`)
Never revive: inline-move-aliasing (`move %0,%1` + pins), inline-asm-injection (hardcoded `$N`;
now in inline-asm-policy), strength-reduce-defeat (`negu` asm), goto-end-prologue-delay-slot
(`ret_val` + `goto end` accumulator), dead-branch-scheduling, param-local-alias reversed-pair
renames, the gte-3x3 and scratchpad-gte pin/barrier recipes, chained-accumulation (superseded by
Q17), bitfield-order flipping (moot since `-mel`), the jtbl rodata-split carve-out (resolved).

## Registering a NEW technique rule
Add `.claude/rules/<slug>.md` with `paths: [".claude/rules/<slug>.md"]` and one line here. A new
technique family needs its own layer-2 and a landed owner ruling, never inside the match commit
([[review-discipline-before-commit]]).
