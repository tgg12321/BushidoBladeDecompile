---
name: codegen-technique-index
paths: ["src/*.c", "include/*.h"]
description: "Symptom-keyed index of the on-demand codegen-technique rules. Technique rules no longer auto-load on src reads (token cost); when a symptom below matches your diff or rule cluster, Read .claude/rules/<slug>.md BEFORE grinding."
metadata:
  type: reference
---

# Codegen-technique index (rules are ON-DEMAND — Read the file when a symptom matches)

The technique rules in `.claude/rules/` do NOT auto-load anymore (they cost
~200k tokens/session; re-scoped 2026-06-11). This index is what auto-loads.
**Contract:** when a symptom below matches the diff shape, regfix cluster, or
construct you're facing, `Read .claude/rules/<slug>.md` FIRST — these rules
encode measured, often multi-session findings; re-deriving them wastes a
session. Never use a sanctioned-exception or forbidden-family construct
without reading its rule. (Policy rules — no-new-park-categories,
review-discipline-before-commit, inline-asm-policy, no-compiler-divergence,
difficult-is-not-impossible — still auto-load and are not listed here.)

## Register-allocation / register-rename diffs

- **register-alloc-pure-c** — tempted to add a `register asm("$N")` pin, or a reg-rename plateau; Step-0 anomaly diagnosis (`-da` greg dump) + Levers A/B/C; confirmed limits for cpu_side_move_dir_4 / marionation_Exec.
- **register-asm-pins** — pins are DIAGNOSTIC-ONLY (never committable); why GCC sometimes ignores them.
- **compare-operand-order-register** — `$X <-> $Y` rename cluster over paired comparisons → reverse the comparison's operand order.
- **call-return-if-result-reuse-v0** — mirror-image rename cluster (branch sense + if/else const loads + subu on one reg) → init the if-else result var from the call return so it lands in `$v0`.
- **restore-discarded-return-displaces-v0** — `$v0`→`$v1` subst on a post-call lw/sw pair → capture the return value the void impl discards.
- **drop-param-alias-local** — param→local alias pins → drop the alias so the param register frees up for reuse.
- **exit-path-return-set-cse-join** — shared finish label where a copy into `$v0` feeds an op → set the return value in EACH exit path, not at the join.
- **divmod-coalesce-reuse-var** (memory/reference/) — GCC's quotient→move→var divmod allocation.

## Cross-jump / merged-tail diffs (target has MORE instructions than you)

- **cross-jump-call-merge** — target has more call sites than your build (jump2 merged identical CALL suffixes) → vary the arg counts.
- **cross-jump-store-tail-merge** — target has more `sw GLOBAL` stores / `j END` tails → MIX the exit forms (distinct `goto endK` labels + one inline `return`); saEft00Add residual is a documented coupled fixpoint — read before re-attempting.
- **shared-end-label** — the INVERSE: per-case `return s2;` constant-folds and DROPS `s2 = N;` stores → restructure to `goto end; ... end: return s2;`.

## Scheduling / delay-slot diffs

- **fake-varargs-explicit-homing** — printf-wrapper arg-register homes are bulk pre-subu in your build but body-SCHEDULED in target (delay slot / copy-reg home) → the original was NOT `...`-variadic; write 4 named args + explicit stores through the va pointer (`ap[1]=a; ...`), pass `ap+1`.
- **switch-break-shared-return-sched-hoist** — per-case `return 0;` lets sched1 hoist the v0-set into a load-delay slot → `break;` + shared trailing `return 0;`.
- **loop-exit-work-inside-loop-sched-fence** — post-loop inits hoisted above a tail-store region → move the loop's exit work INSIDE the loop (`if (cond) continue; tail; break;`).
- **loop-note-fixes-delay-slot-steal** — a memory-clobber barrier that only blocks a delay-slot steal → write the loop as a real `while`/`do`.
- **loop-counter-fills-load-delay** (func_80045294, 2026-06-14) — accumulator loop `sum += *(p+off); i++; off+=0x10;` leaves a maspsx nop in the `lw` load-delay slot (your build hoists `i++` to the loop top) → split the load into a named temp and reorder so the address-advance comes BETWEEN load and use, `i++` just before the consume: `s32 val = *(p+off); off+=0x10; i++; sum+=val;`. cc1's first-pass scheduler then drops `i++` into the lw delay slot (no nop). NB: `i++` BEFORE `sum+=` matters — the 4 forms keeping `i++` ahead of the load all failed; the `off+=` between load and use is the lever. Pure C, no dead store.
- **walking-pointer-serializes-parallel-loads** — memory-clobber barriers OR per-load `register asm("$N")` pins between independent parallel-array element stores (`G0=r[0]+a[0]; ...` or `Gi=a0[i]`) → walk the array(s) with post-increment pointers (`*ap++`); the pointer dependence serializes the loads so GCC keeps the per-element lw/sw and stops stealing later loads into delay slots. An interleaved independent constant/global store whose `lui` GCC hoists too early: move that store PAST the loads so it schedules into the freed delay slots.
- **hoist-call-arg-local-flips-jal-delay** — fill_delay regfix cluster around a call → hoist the late-loaded arg into a local declared FIRST in the block.
- **store-before-jal** — arg saved into a callee-save between a table load and its call; ordering recipe.
- **defer-store-past-later-compute-into-jal-delay** — a `reorder` rule over a pre-call store cluster + the lone diff is a `sw` (global/field) emitted EARLY where target defers it into a following jal's delay slot → hoist the stored value into a local and move the `GLOBAL = val;` store statement AFTER a later independent compute; GCC then schedules the sw into the delay slot.
- **legitimate-volatile-interrupt-touched** (§Confirmed cases, sys_VSync 2026-06-12) — target block looks UNSCHEDULED (strictly source-ordered, genuine load-delay nop) around reads of an IRQ-touched counter while your build interleaves the chains → sched.c `read_dependence` needs BOTH reads volatile; check whether the second read's symbol qualifies for (or already has, under another C handle) a carve-out grant.
- **goto-end-prologue-delay-slot** — ARCHIVED/FORBIDDEN tombstone.
- **dead-branch-scheduling** — ARCHIVED/FORBIDDEN tombstone (manufactured dead-branch insns).

## Optimizer-fold diffs (CSE / combine / LICM / strength-reduce)

- **defeat-licm-hoist-var-reuse** — GCC hoists a loop-invariant the target recomputes inline → reuse a scratch variable.
- **defeat-combine-symbol-fold** — displaced store/load folded into `%lo(sym+K)` addressing → pre-compute a displaced pointer.
- **hoist-flag-load-defeat-add-combine** — two `pN += K` adds combine-merged into one → hoist a flag-word load between them.
- **split-read-defeats-hoist** — shared read through a flag-selected base gets hoisted → duplicate the read into the branch arms (SOTN-sanctioned).
- **store-const-reload-cse** — store-then-reload folded to `li` → re-read the GLOBAL instead of caching a local.
- **explicit-rejection-set-defeats-range-fold** — subtract-then-unsigned-compare range exclusion vs target's explicit `!= K1 && != K2`.
- **loop-rotation-two-shift** — two `sllv` from loop rotation; opaque-`one` arithmetic (SOTN-sanctioned) replaces the initial-mask asm.
- **strength-reduce-defeat** — ARCHIVED/FORBIDDEN tombstone.
- **or-tree-shape-shift** — FORBIDDEN: parenthesization-axis mutations in associative/commutative expressions.

## Width / addressing / layout diffs

- **bitfield-direction-divergence** — bitfield access compiles to the OPPOSITE half of the word (srl vs sra direction): our fork allocates bitfields HIGH-first; header-level field-order is the fix surface.
- **halfword-index-srl-sra** — `subst srl→sra` on a halfword array index → direct byte-offset cast.
- **u16-global-lhu-lbu-low-byte** — `subst lhu→lbu` on a u16 global dispatch read → read the low byte explicitly.
- **narrow-stack-param-subword-offset** — sub-word stack-param load at offset ±2 → read the low half explicitly (SOTN-sanctioned cast).
- **narrow-byte-args-packed-call** — byte-packed-arg GPU/SPU call wrappers → declare the params `u8`.
- **param-reuse-base-copy-cse-canon** — list-walker carrying copy pins → param-reuse walker + early-named call arg.
- **switch-vs-ifchain-branch-sense** — branch-sense-swap rules on ONE case of a dispatch → write a real `switch`, not an if-goto chain.

## Forbidden families & narrow sanctions (READ THE RULE before using or judging)

- **dead-vars-local-array** — FORBIDDEN: frame coercion via unused arrays/scalars. **CARVE-OUT 2026-07-01:** a WRITTEN-never-read local array IS sanctioned (SOTN dra/62DEC.c sp70[4] ×2) when the target bytes contain the dead stores — last-resort, FAKE-annotated, dual-reviewed. Symptom: target has dead `sw` stores into a frame the clean C never allocates.
- **lost-codegen-insert-cheat** — FORBIDDEN: `insert_after "addu $rN,$0,$zero"` regfix (restore what const-prop ate) → fix via shared-end-label.
- **inline-asm-injection** — FORBIDDEN: hardcoded-`$N` single-instruction `__asm__`; also `asm("Sym")` alias renames.
- **inline-move-aliasing** — ARCHIVED/FORBIDDEN tombstone (placeholder-move recipe).
- **param-local-alias-prologue-pair-flip** — ARCHIVED/FORBIDDEN tombstone.
- **do-while-zero-exception** — the ONE sanctioned no-semantic-purpose wrapper; strict prerequisites; READ BEFORE using `do {...} while (0);`.
- **legitimate-volatile-interrupt-touched** — narrow `extern volatile` carve-out (IRQ-touched globals only); two-prong test; READ BEFORE adding any volatile. NB (func_80078B04, 2026-06-12): `*(volatile T *)(computed MMIO address)` is a DIFFERENT, legitimate category (hardware-register access; detector distinguishes it from `&D_xxx` casts) — a single-read of an MMIO-pointer table plateauing 1-2 regs off target allocation is the symptom that the original access was volatile.
- **pointer-rmw-global-sanctioned** — narrow zero-displacement pointer-RMW spelling sanction.
- **proven-spelling-class-reconstruction** — same-bytes respelling exception; ALL conditions must hold.
- **dead-store-fake-exception** — SANCTIONED 2026-07-01 (last-resort): dead store / self-assign to a LOCAL or PARAM, `/* FAKE */`-annotated, after documented lever-exhaustion; READ BEFORE writing one. Un-annotated = still forbidden; pins still forbidden. **Confirmed closure:** target keeps an unfolded 0/1 diamond (`bnez; li v0,1` delay; `move v0,zero`) that your build folds to `return X;` → dead `ret = 1;` INSIDE the else arm (two-set arm breaks jump.c's store-flag single-set precondition); detached placement does NOT work (func_80078EC0).
- **named-local-fake-exception** — SANCTIONED 2026-07-01 (last-resort): constant-holder locals across calls + dead SCALAR local decls biasing RA, `/* FAKE */`-annotated; arrays/frame coercion still forbidden.
- **pointer-alias-fake-exception** — SANCTIONED 2026-07-01 (last-resort): C-level local pointer alias / typed re-view of a global, `/* FAKE */`-annotated; `asm("Sym")` alias-RENAMES still forbidden.
- **mmio-volatile-type-level** — RULING 2026-07-01: volatile on hardware-MMIO-range (0x1F801000-0x1F802FFF) declarations is legitimate TYPE-LEVEL semantics — all shapes incl. single-read probes; no shape test, no FAKE annotation; game-state globals keep the two-prong gate.
- **duplicated-statement-into-arms** — SANCTIONED 2026-07-01: a REAL statement duplicated into 2+ arms (vs label-sharing), incl. when cross-jump re-merges it to identical bytes and the effect is a reg_n_refs priority lift; label placement steers merge direction. Prereqs: byte-neutrality verified + exhaustion + FAKE annotation. **The proven byte-free ref-lift for global-RA priority walls** (motion_SetMotion pins retired; dead stores measured INERT for this — flow deletes before counting).

## Engine / pipeline gotchas (also fire on their own narrow paths)

- **sandbox-zero-retire-fails** — sandbox 0 but `retire` rolls back: masked-0 hides a real register diff; keep editing.
- **prologue-fix-redundant-reorder** — a single prologue `reorder` regfix may just be undoing prologue_fix.
- **global-label-drift-sibling-cheat** — a pure-C retire that changes the `.L` label count can break a LATER sibling's hardcoded-label rule.
- **canonical-asm-authorization-recipe** — writing whole-body `__asm__("glabel ...")` (user-authorized only).
- **canonical-gate-distance-not-evidence** — a big distance is NOT evidence for ASM routing.
- **jtbl-rodata-split-infrastructure** — historical rodata-split jump-table carve-out (cluster resolved 2026-06-09).
- **maspsx-label-nop-gate** — single `__asm__("nop")` before a load in a loop → per-function maspsx gate, don't park.
- **maspsx-noreorder-stripping** — glabel-form asm MUST duplicate TAB-form `.set` directives with SPACE-form.
- **compiler-flags-canonical** — do not flag-hunt; flags were settled project-wide 2026-05-20.
- **permuter-directives** — bridge from a technique-index rule to PERM_* macros (decomp-permuter manual mutations); use when a rule lists 2+ alternative spellings, when you need to sweep candidate registers / orderings, or after you've measurably lowered the floor and want exhaustive cross-product validation. Closing forms are PROPOSALS and must clear the cheat-vetting checklist + layer-2 cheat-reviewer.

## Registering a NEW technique rule

Per the loop's step 6: add the rule file with `paths:
[".claude/rules/<slug>.md"]` (on-demand) AND add a one-line symptom entry to
the section above that fits. Only enforcement-critical POLICY rules get a
broad `src/*.c` glob — never technique recipes. New-technique-family rule
docs additionally need layer-2 review + user sign-off per
[[review-discipline-before-commit]].
