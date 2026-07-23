# Hypothesis ledger — ang_hosei_80056FE8

## s1 recon (2026-07-23) — baseline map

Floor: sandbox 10, build_insns 42 vs target 43, canonical verdict C. Applied clean
candidate to src (removed HEAD's `register asm("$5")` pin + `asm volatile` barrier).

### Mechanism (confirmed from asm + ledger)
40-insn angle-correction leaf. The single missing insn is a load-delay NOP:
- TARGET: `base=a3*40` -> `$a1` (via `sll $a1,$v0,3` early); `var_v0` test load
  `lh $v0,0x5E($a0)` -> `$v0`; both live to the join. At `.L80057074`:
  `addu $a1,$a1,$v0` (partial) is FORCED before `lw $v0,0($a0)` (arg0 reload)
  because the reload would clobber live var_v0 in $v0 -> the following
  `lh $v0,0x40A($v0)` keeps a genuine load-delay `nop` = 43 insns.
- OURS: var_v0's test load claims `$a1`, base parks in `$a3`; partial-add
  `addu ?,$a3,$a1` has no conflict with the `$v0` reload, so the scheduler fills
  the lh's delay slot with it -> 42 insns (1 short).
- The wall is the FULL coupling: `var_v0->$v0 AND base->$a1 SIMULTANEOUSLY`. Every
  structural variant tried so far produces build 42 with DIFFERENT reg choices;
  the masked sandbox scores 10 for all of them (can't see the a1/a3 swap).

### Recon findings
- No sibling/duplicate analog: find_duplicates at threshold 0.85 AND 0.6 returns
  ZERO pairs for this function. `ang_hosei` (base name) is also INCOMPLETE and
  structurally unrelated; `ang_hosei_8003F62C` is a non-leaf with many calls —
  name-only kinship, not a code analog.
- m2c reference saved (tmp/grind/ang_hosei_80056FE8/s1/m2c_reference.c): m2c
  inlines base into the return, but target computes base EARLY. candidate's
  early-base ordering matches target; base-late was measured floor 27.

## Frontier (un-tried, mechanism-grounded — for next drill/permuter session)
1. RTL .greg dump (cc1 -da on the candidate's base.i): read `;; Register
   dispositions:` + conflict lists for the base pseudo and the var_v0 pseudo.
   NOT yet done (prior sessions only did objdump-tail comparison). Determines
   whether base->$a3 is a priority TIEBREAKER (C-lever reachable) or a real
   liveness conflict. This is the mandated pre-claim diagnostic.
2. Clean single-function-target permuter (--stop-on-zero + directed PERM_*),
   target.o built from asm/funcs + prelude at offset 0 so base score is the real
   weighted RA diff (per difficult-is-not-impossible §3). NOT yet run — prior
   sweeps were manual structural variants only, no permuter campaign.
3. Steer base into $a1 by shrinking var_v0's competing live range so the
   allocator's priority for the arg-reg-resident base rises (specific C form to
   derive from the .greg dump in #1).

## [s1] A completed sibling/duplicate function provides a known-C analog to seed the match.
- mechanism: find_duplicates.py opcode-class Levenshtein near-clone search over asm/funcs/.
- probe: Ran find_duplicates.py at threshold 0.85 and 0.6; grepped tmp/duplicates.txt for 80056FE8.
- result: Zero near-clone pairs at either threshold. ang_hosei/ang_hosei_8003F62C are name-only kin (latter is a non-leaf with jal chains).
- verdict: KILLED

## [s2] The {base->$a1, var_v0->$v0, partial-first} coupling is reachable by structural reassociation / declaration-order / type levers.
- mechanism: If some dataflow-preserving C form makes GCC keep the partial-add ahead of the *arg0 reload, var_v0 stays in $v0, base takes $a1, and the lh(+0x40A) load-delay nop survives (43 insns).
- probe: RTL diagnosis (.combine vs .sched vs .greg on a standalone mini unit) + 12 new forms (sweep.py 8 tail forms, sweep2.py 4 head forms) + 2 real-sandbox measurements.
- result: KILLED. .combine shows source order is ALREADY partial-first; sched1 hoists the reload+lookup chain above the partial-add because that chain is intrinsically one load longer (higher INSN_PRIORITY) for EVERY grouping of the final adds. Forms that force var_v0->$v0 (0x12C-into-partial, grouped) displace base to $a2 and fill both delay slots (build 41, score 14). s16-narrow adds a truncating sll (wrong bytes). Head decl-order/pointer-local/base-mul give the identical candidate alloc. The target fixpoint {a2=$a2,base=$a1,var_v0=$v0,partial-first} is not reachable by any structural transform.
- verdict: KILLED

## [s1] The 1-insn shortfall (42 vs 43) is a single load-delay NOP produced by a coupled register allocation {base->$a1, var_v0->$v0}.
- mechanism: Target keeps base=a3*40 in $a1 and var_v0 in $v0 across the join; the arg0 reload `lw $v0,0($a0)` would clobber live var_v0, forcing `addu $a1,$a1,$v0` before it, so `lh $v0,0x40A($v0)` keeps a genuine load-delay nop. Our build lands var_v0's test in $a1 and base in $a3, so the partial-add fills the lh delay slot instead.
- probe: Read asm/funcs/ang_hosei_80056FE8.s instruction-by-instruction; cross-checked against ledger's masked-invisible RA-swap finding; m2c reference confirmed dataflow (arg0 reload, early base).
- result: Confirmed: target's partial-add precedes the reload (43 insns); every prior structural variant produces 42 with different reg choices, masked-invisible at sandbox 10.
- verdict: CONFIRMED

## [s2] The {base->$a1, var_v0->$v0, partial-add-before-reload} coupling that yields the 43rd (load-delay nop) insn is reachable by structural reassociation / declaration-order / type-narrowing levers.
- mechanism: If a dataflow-preserving C form keeps the partial-add (base+var_v0) ahead of the *arg0 reload, var_v0 stays in $v0, the reload clobbers it forcing partial-first, base takes $a1, and the lh(+0x40A) load-delay nop survives.
- probe: Ran the mandated RTL diagnostic on a standalone mini unit (cc1 -da): compared .combine (pre-sched1), .sched (post-sched1), and .greg dispositions/conflicts; then swept 8 tail-reassociation forms (sweep.py) + 4 head-structural forms (sweep2.py) + 2 real-sandbox measurements.
- result: .combine shows source order is ALREADY partial-first (insn107 partial, then 110 reload, 114 lookup, 116 sum); sched1 HOISTS reload+lookup above the partial-add, manufacturing var_v0's hard conflict with $v0 (greg: pseudo 82 conflicts hard reg 2). The reload->lookup->sum chain is intrinsically one load longer than partial->sum, so INSN_PRIORITY(reload)>=INSN_PRIORITY(partial) for every grouping -> sched1 always hoists. Forms forcing var_v0->$v0 (0x12C-into-partial, grouped) displace base to $a2 (never $a1) and fill BOTH delay slots (build 41, masked score 14). s16-narrow injects a truncating sll (wrong bytes). Head decl-order/pointer-local/base-mul reproduce the candidate alloc exactly. Target fixpoint {a2=$a2,base=$a1,var_v0=$v0,partial-first} unreachable by structural transform.
- verdict: KILLED
