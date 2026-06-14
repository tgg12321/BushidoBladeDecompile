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
- **walking-pointer-serializes-parallel-loads** — memory-clobber barriers between independent parallel-array element stores (`G0=r[0]+a[0]; barrier; G1=r[1]+a[1]; ...`) → walk both arrays with post-increment pointers (`*rp++ + *ap++`); the pointer dependence serializes the loads so GCC keeps the per-element lw/lw/nop/addu/sw and stops stealing later loads into delay slots.
- **hoist-call-arg-local-flips-jal-delay** — fill_delay regfix cluster around a call → hoist the late-loaded arg into a local declared FIRST in the block.
- **store-before-jal** — arg saved into a callee-save between a table load and its call; ordering recipe.
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

- **dead-vars-local-array** — FORBIDDEN: frame coercion via unused arrays/scalars.
- **lost-codegen-insert-cheat** — FORBIDDEN: `insert_after "addu $rN,$0,$zero"` regfix (restore what const-prop ate) → fix via shared-end-label.
- **inline-asm-injection** — FORBIDDEN: hardcoded-`$N` single-instruction `__asm__`; also `asm("Sym")` alias renames.
- **inline-move-aliasing** — ARCHIVED/FORBIDDEN tombstone (placeholder-move recipe).
- **param-local-alias-prologue-pair-flip** — ARCHIVED/FORBIDDEN tombstone.
- **do-while-zero-exception** — the ONE sanctioned no-semantic-purpose wrapper; strict prerequisites; READ BEFORE using `do {...} while (0);`.
- **legitimate-volatile-interrupt-touched** — narrow `extern volatile` carve-out (IRQ-touched globals only); two-prong test; READ BEFORE adding any volatile. NB (func_80078B04, 2026-06-12): `*(volatile T *)(computed MMIO address)` is a DIFFERENT, legitimate category (hardware-register access; detector distinguishes it from `&D_xxx` casts) — a single-read of an MMIO-pointer table plateauing 1-2 regs off target allocation is the symptom that the original access was volatile.
- **pointer-rmw-global-sanctioned** — narrow zero-displacement pointer-RMW spelling sanction.
- **proven-spelling-class-reconstruction** — same-bytes respelling exception; ALL conditions must hold.

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

## Registering a NEW technique rule

Per the loop's step 6: add the rule file with `paths:
[".claude/rules/<slug>.md"]` (on-demand) AND add a one-line symptom entry to
the section above that fits. Only enforcement-critical POLICY rules get a
broad `src/*.c` glob — never technique recipes. New-technique-family rule
docs additionally need layer-2 review + user sign-off per
[[review-discipline-before-commit]].
