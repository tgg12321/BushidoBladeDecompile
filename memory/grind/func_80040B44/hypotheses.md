# Hypothesis ledger — func_80040B44

## Frontier after s1 (recon; floor 32, none measured yet — s1 was baseline+coupling)

### H1-ORDER — per-site addend order closes the 5 commutative addu diffs
STATEMENT: Rewriting the address arithmetic with m2c's per-site addend order
(index-first at the &seen[a3] site, the if-arm slot site, both
`arg0 + (t2<<2) + 0x1A34` sites, and the else-arm `a2` site; base-first ONLY at
the `*t4 = t7 + a3*0x68` store site) makes GCC emit target's mixed
`addu` operand orders, killing diffs at target insns 32, 42, 44, 55, 82.
MECHANISM: GCC 2.7.2 expand preserves source addend order for PLUS when
neither operand is constant; target+m2c agree the original mixed orders
per site. Plain C spelling — no family/FAKE needed.
PROBE: edit the 5 expressions in src, `sandbox --disable all`, read
sidebyside. Watch for or-tree-shape-shift rule (forbidden parenthesization
mutations) — addend ORDER of a two-term add is not parenthesization, but read
`.claude/rules/or-tree-shape-shift.md` before writing the vet.
STATUS: UNTESTED.

### H2-FRESH — fresh per-loop locals fix the a0/a1/v0 init-loop seats
STATEMENT: Using fresh locals per init loop (loop1: cnt1/p1; loop2: cnt2/p2,
per m2c var_a0/var_a1 + var_a0_2/var_v0) instead of the committed body's
reused `i`/`ptr` flips the counter into $a0 and puts loop-2's pointer in $v0,
killing diffs at target insns 6–19 (rules 159–167 class).
MECHANISM: reusing one pseudo across both loops merges liveness and changes
local-alloc seat assignment (reg_n_refs/death counts); fresh single-loop
pseudos allocate in first-use order → a0 (counter first), a1/v0. The committed
reuse is a rule-era shape; m2c shows the original had fresh vars.
PROBE: rewrite the two init loops with fresh locals, measure. If the seats
don't flip, read the .lreg dump (`pwsh tools/grinder/dump.ps1 func_80040B44`)
for death counts before iterating — local-alloc-death-count-class-wall rule.
STATUS: UNTESTED.

### H3-DEADDEC — honest producer for the dead `t3 -= 2` (target insn 89)
STATEMENT: The original source kept a post-loop pointer compensation
(`addiu t3,t3,-2`) that our chassis writes but GCC DCEs. An honest C shape
exists in which the decrement's result is (or appears) live.
MECHANISM CANDIDATES (in probe order):
  (a) m2c's single-advance read shape (`a3 = *t3; a0_val = *(u16*)((u8*)t3+2);
      t3 += 4;` per iteration, exit compensation `t3 -= 2;`) — different
      dataflow may leave the compensation live into the final bookkeeping;
  (b) the loop-exit-work-inside-loop shape (compensation inside the loop on
      the exit path before break);
  (c) if both measure dead: this is dead-store-to-LOCAL territory —
      the sanctioned family is dead-store-fake-exception
      (.claude/rules/dead-store-fake-exception.md, LOCALS/PARAMS, FAKE
      annotation, lever-exhaustion first). A FAKE self-assign/read that keeps
      t3 live is a LAST resort after (a)/(b) measured; do not reach for it
      first. The current rule @175 insert_after is the forbidden
      lost-codegen-insert class and can never be the answer.
NOTE: flow.c deletes dead register writes; the question is what read kept it
live for cc1psx=GCC2.7.2 in the original. Since the SAME compiler builds our
tree, some honest spelling must keep it — find it, don't fake it first.
STATUS: UNTESTED.

## Secondary (not yet frontier-ranked)

- H4-SCHED: target insns 26–28 order (`li t8,0xffff` BEFORE `addiu t4` and
  `addiu t3,2`). Both constants (t9=1, t8=0xffff) materialize together at the
  loop preheader in target. May fall out of H2/H3 restructures for free;
  else candidates: named u16 sentinel local read before the pointer inits,
  or statement order in the preheader block. Rule @177 is the current fake.
- H5-ONE: `s32 one = 1;` in the committed body is the forbidden-catalog
  spelling ("opaque variable to defeat single-bit transform"). Measure
  removing it (plain `*a1 = 1;`): target insn 25 `li t9,1` is a hoisted
  constant; GCC may hoist it naturally (loop.c constant hoist) without the
  holder. If removal is byte-neutral, delete it; if not, the honest producer
  question joins H4 (both loop-preheader constant materializations).
- t2/t3 seat swap (rule @157 class): likely COUPLED to H2/H5 outcomes (seat
  assignment cascades from the prologue). Re-measure after H1/H2 land; only
  then dump .lreg/.greg and treat as its own wall.

## [s1] The migration byte-coupling is positional: INCLUDE_ASM(macro-inc) emission lands at the TU's .text start, not the function's mid-TU source position
- mechanism: file-scope asm include emitted ahead of compiled functions; B44 is 10th of 15 in text1a_pre.c
- probe: reproduced migration state exactly (sha e29f91cc == recorded); image diff: one 2788B range 0x800401CC-0x80040CB0 with B44's prologue at TU start and func_800401CC's signature at +372B; 11 scattered 1-2B diffs are %hi/%lo addends to shifted sibling symbols; tree restored, oracle MATCH re-verified
- result: coupling fully characterized; COMPLETED-C (compiled C in source position) has no coupling — solving the function is the clean exit
- verdict: CONFIRMED

## [s1] The 32-insn honest diff decomposes into 5 classes: a0/a1 init-loop seats, t2/t3 main-loop seats, 5 commutative addu operand orders, one 3-insn preheader schedule, and one DCE'd dead decrement (addiu t3,t3,-2)
- mechanism: side-by-side disassembly of honest sandbox build vs oracle bytes
- probe: tmp/grind/func_80040B44/s1/sidebyside.txt (93 insns, 41 marked incl. branch-addend false positives, 32 scoring)
- result: map banked in evidence.md with per-class rule attribution (rules 157-177)
- verdict: CONFIRMED
