# Evidence bank — func_800401CC

## Session 1 (recon, 2026-08-11)

### Baseline / routing
- `canonical`: verdict **C**, 78 insns, distance 20 — pure-C target.
- `sandbox --disable all` at session start: **20/78**. 3 regfix rules dropped
  (regfix.txt:9-11: `$2 <-> $3 @ 1-5`, `$2 <-> $3 @ 14-21`, `$6 <-> $7 @ 51-68`),
  4 cheat-asm stripped — the stripped cheat-asm is the `register ... asm("s7")`
  pins at src/text1a_pre.c:566/755 which belong to OTHER functions in this file,
  NOT func_800401CC. func_800401CC itself carries no inline asm and no pins.
- No duplicate/sibling lead: `tmp/duplicates_leads.txt` has no entry for this func.
- All 20 baseline diffs were pure register RENAMES in 3 regions exactly matching
  the 3 regfix swap rules; instruction order/opcodes were already byte-perfect.
- Build context: text1a_pre is one of the two GP_FILES — compiled with
  CC_FLAGS_GP (`-O2 -G8 ... -mel`), gp-relative access to D_800A3234/D_800A3378.

### Function shape (target asm/funcs/func_800401CC.s)
GPU packet writer: reads D_800A36AC parity → selects double-buffered packet
base (D_800A9830/D_800A9920 + parity*240, D_800A3378 = write cursor,
D_800A3234 = cached parity); if cursor not at the D_800A9920-side base, copies
a 6-halfword record from D_80094AF4[a1*6] into a stack buf (+0x80 x-offset if
a0), calls SetDrawMove, then links the packet into the OT at D_800A378C+0x3FFC
(classic 24-bit-pointer/8-bit-code OT insert) and advances the cursor by 0x18.
Param a2 is dead-on-entry and immediately overwritten (target does
`andi $a2, $v0, 1` clobbering the incoming a2) — keep the param-reuse spelling.

### Measured wins (floor 20 → 7)
1. **Staging-local removal** (20 → 7 together with nothing else): old C staged
   global reads through a reused `s32 v0;` (`v0 = D_800A36AC; a2 = v0 & 1;` and
   `v0 = D_800A3234;` before compare 2). That single multi-block pseudo went to
   global-alloc and ended in $3 while block-local temps took $2. Inline reads
   (`a2 = D_800A36AC & 1;` / compare against `D_800A9920 + D_800A3234 * 240`
   directly) make every read a short-lived local-alloc temp; temps take
   ascending regs and BOTH swap regions 1-5 and 14-21 match target exactly.
2. **Store-last statement order** (structural): `D_800A3378 = (s32)(pkt + 6);`
   must come AFTER `ot[0x3FFC/4] = ...`. Mechanism: GCC 2.7.2 cannot
   disambiguate `sw D_800A3378` (symbol) vs `lw 0x3FFC($ot)` (pointer). If the
   store precedes the load in RTL, sched treats it as a true dependence and
   pins `sw` before `lw`; if the load precedes, it's an anti-dependence and the
   scheduler floats the `sw` down between the two tail ANDs — which is exactly
   target's insn 70-77 order (`sw pkt; addiu; lw ot; and pkt&FFFFFF;
   sw D_800A3378; and ot&FF000000; or; sw ot`). With this order the whole tail
   is structure-identical; only the $6/$7 mask assignment differs.

### Remaining 7/78 — the $6/$7 mask swap, mechanism MEASURED
Target: 0xFFFFFF (lui+ori) in **$6/a2**, 0xFF000000 (single lui) in **$7/a3**.
Ours: reversed. Instrumented cc1 (`tools/gcc-2.7.2/cc1`, BB2_QTY_DEBUG=1;
NOT build/cc1 which lacks the hooks — but build/cc1 output is byte-identical
so either compiles the same) on a solo .i gives, for the tail block (blk=5),
in the canonical floor-7 form:

    qty reg=FFFFFF  birth=18 death=48 refs=3 got=$7   (li+ori: born 4 luids early)
    qty reg=FF000000 birth=22 death=46 refs=3 got=$6  (single li; dies first)

- local-alloc order is `qty_compare_1` (local-alloc.c:1660): priority =
  `floor_log2(refs) * refs * size / (death - birth)`, HIGHER first; tie →
  lower qty number (birth order). Both masks have refs=3, size=4 → priority is
  purely 1/length. FF000000 is shorter-lived in every variant tried → allocated
  first → takes $6 (find_free_reg ascending; $2-$5 occupied by longer temps).
- sched1 (which runs BEFORE local-alloc, so its luids are what qty_compare
  sees) ALWAYS hoists the 0xFFFFFF li+ori chain first among the constant loads
  (longest-chain priority), so birth_FFFFFF = birth_FF000000 - 4 in every
  variant measured. The two tail ANDs always end up adjacent (death gap 2) at
  sched1 output no matter the source order of their statements. Net:
  len(FFFFFF) = len(FF000000) + 2 always → FF000000 always wins $6.
- For target's assignment, the original RTL must have satisfied
  len(FFFFFF) <= len(FF000000), i.e. death gap >= birth gap (4), OR
  refs(FFFFFF) > refs(FF000000). With refs equal and deaths adjacent this is
  impossible — see hypotheses.md for the surviving directions.
- Analytically KILLED alternative: "FF000000 allocated first but takes $7".
  Impossible: FF000000's live range (22-46) is a strict subset of FFFFFF's
  (18-48), so any hard reg conflicting FF000000 also conflicts FFFFFF; the
  first-allocated of the two always gets $6.
- Also measured: target stmt2's `or` writes into the ot-word register (v0),
  proving the original operand order was `(ot & 0xFF000000) | (pkt & 0xFFFFFF)`
  — the flipped order makes GCC reuse pkt's reg as dest (`or $4,$4,$2`) and
  restructures the addiu/sw placement (+5). The current source's stmt2 operand
  order is CORRECT; do not re-flip it.

### Artifacts (tmp/grind/func_800401CC/s1/)
- `dump.sh` — regenerates solo.i from current src + runs instrumented cc1 with
  BB2_QTY_DEBUG, prints blk=5 QTYDBG lines + tail asm. Reuse next session.
- `solo.err` / `solo.s` — QTYDBG dump + asm of the final floor-7 form.
- `base.i.lreg` etc. — full -da dump set of an intermediate (two-mask-local)
  variant; mask pseudos there are 80/81, insns 183/184/146.
- CPP line for solo.i: boundary = line of `void func_80040304` in full.i
  (function is first in the TU; solo compile is warning-clean enough and its
  codegen for this function is byte-identical to the sandbox build's).

- [s1] canonical: verdict C, 78 insns; baseline sandbox --disable all = 20; all 20 diffs are register renames matching the 3 regfix swap rules exactly

- [s1] cheat_asm_stripped:4 belongs to OTHER functions in text1a_pre.c (asm("s7") pins at lines 566/755); func_800401CC itself is pin/asm-free

- [s1] Floor 7 reproduced and left applied in src/text1a_pre.c; candidate.c banked

- [s1] Remaining 7 diffs = $6<->$7 over tail masks (regfix @51-68): target has 0xFFFFFF in $6, 0xFF000000 in $7; ours reversed; insn order already byte-perfect

- [s1] QTYDBG (instrumented tools/gcc-2.7.2/cc1, BB2_QTY_DEBUG=1): FFFFFF birth=18 death=48 refs=3 got=$7; FF000000 birth=22 death=46 refs=3 got=$6 in the canonical form; analytic kill: FF000000's range is a strict subset, so first-allocated always gets $6 -> only path is FFFFFF allocated first (needs refs advantage or death gap >= 4)

- [s1] Full mechanism notes + artifact pointers in memory/grind/func_800401CC/evidence.md

## Session 2 (structural, 2026-08-11) — floor 7 → 0 (SANDBOX ZERO)

- [s2] Session-start gotcha: src/text1a_pre.c was back at the OLD 20-form (s1's
  src edits were not retained); re-applying candidate.c restored floor 7 before
  any probing. Future sessions: always verify the floor matches the ledger
  before interpreting a probe.
- [s2] P1 (a2 = 0xFFFFFF param-reuse): score 12. QTYDBG: FF000000 local qty
  (birth 22, refs 3) still took $6; a2's global allocno lost its $6 param
  preference to it (local-alloc runs BEFORE global-alloc) and landed $7,
  renaming the whole head parity cluster (andi/beq/sw) $6→$7 = 4 new diffs.
- [s2] P2 (a2 = 0xFF000000 param-reuse): score 7 REDISTRIBUTED — the tail ANDs
  matched target for the first time (FFFFFF, now the only local mask qty, took
  $6; the global holder was pushed to $7 = target for FF000000), but the head
  broke the same way (4) plus the constant-load emission order flipped (3).
  P2 is the probe that identified the winning mechanism.
- [s2] THE CLOSER: the FF000000 holder must be a multi-set GLOBAL pseudo with a
  $7 copy preference and NO head hard-reg commitment. That variable already
  exists: `v`, the texture V coordinate — SetDrawMove's 4th arg, copied into
  $7 at the call (sll/sra 16), dead afterward. Widened s16→s32 (head bytes
  unchanged: lhu + (s16) call casts identical) and staged `v = 0xFF000000;`
  after the call: score 2 (head intact, ALL registers target-correct; only the
  li cluster order wrong: ours li $7; li+ori $6, target li+ori $6; li $7).
- [s2] Emission-order fix: naming the low mask as a separate local set FIRST
  (`lowmask = 0xFFFFFF; v = 0xFF000000;`) restored the target order → 0/78.
  With inline 0xFFFFFF the FFFFFF li+ori was NOT hoisted above v's li by
  sched (unlike the all-local floor-7 form where it was); the explicit
  earlier set biases LUID order and sched keeps it. (Mechanism note: in the
  all-local form sched1 hoisted the FFFFFF chain by priority; with v global
  the priorities evidently tie or invert — not fully instrumented, the fix is
  measured not modeled.)
- [s2] Final form: sandbox 0/78, edits in place in src/text1a_pre.c, FAKE
  annotation on the staged assignment, self_vet.md written. QTYDBG of the
  closing form: FFFFFF qty birth=20 death=48 refs=3 got=$6; v not in local
  pool (global). Artifacts: tmp/grind/func_800401CC/s2/{dump.sh,solo.s,
  solo.err,diff.sh}.
