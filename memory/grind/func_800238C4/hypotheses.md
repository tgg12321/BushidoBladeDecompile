# Hypothesis ledger — func_800238C4

Floor: **3** (measured s1 2026-09-07, current chassis, 219/219 instructions).
Residual: a $v0/$v1 seat swap on `{lw parent, li 2}` feeding
`sh 2, 0x286(parent)` — the single C statement
`*((s16 *) ((*((u8 **) arg0)) + 0x286)) = 2;`.
Governing pass: local-alloc, `qty_compare_1` (tools/gcc-2.7.2/local-alloc.c:1660).
Measured quantity data: tmp/grind/func_800238C4/s1/qtydbg.txt:1237-1238.
Full derivation in evidence.md — do NOT re-derive it.

## KILLED

### K1 — pointer local at the use site (P1)
Naming the parent pointer in a local declared immediately before the store
(`s16 *parent = *(s16 **)arg0; parent[0x143] = 2;`) does not change the seating.
MEASURED score 3 (unchanged) and the residual asm is bit-identical.
Mechanism: expand order for `*p = 2` is fixed — `expand_assignment` evaluates the
LHS address before the RHS — so the pointer pseudo is still born one insn ahead of
the constant pseudo and still loses the qty priority sort.
Scope: INSTANCE (this form, floor-3 chassis, no FAKE constructs present).
Banked: rejected/pointer-local-at-use-site-noop-score3.c

### K2 — duplicated-statement-into-arms (P3)
Duplicating the parent store into both arms of `if (s1 < 0x400)` scores 9 (+6).
jump2/cross-jump does not re-merge the two copies, so the target's single
post-merge block `.L80023B90` (lw/li/j/sh-in-delay-slot) is destroyed outright.
The re-merge prong of the duplicated-statement family is measured FALSE here, so
the family cannot even be claimed for this residual.
Scope: INSTANCE (this form, floor-3 chassis, no FAKE constructs present).
Banked: rejected/duplicated-parent-store-into-arms-score9.c

### K3 — named constant intermediate `s16 two = 2;` (P7)
This was the direct spelling of the mechanism lever "materialise the constant
before the pointer load". MEASURED score 3 (unchanged), residual asm bit-identical.
It fails EARLIER than allocation: cse/combine constant-propagates `two` back into
the store and deletes the pseudo, so the `.lreg` RTL is unchanged (pointer born
suid 4, constant born suid 6, both dead at suid 8). A source-level constant holder
of a plain literal never survives to local-alloc here, in any integer width.
Scope: INSTANCE (this form, floor-3 chassis, no FAKE constructs present).
Banked: rejected/named-const-two-folded-noop-score3.c

### K0 (inherited, now explained) — parent pointer hoisted above the if/else
Score 6. Now attributed: the hoist makes the pointer live across basic blocks,
which takes it out of local-alloc and into global-alloc.
Banked: rejected/parent-ptr-hoisted-before-ifelse-score6.c

## LIVE FRONTIER (for the next session)

### H1 — make the constant's quantity born no later than the pointer's, with a value that survives cse/combine
Only two `qty_compare_1` inputs can invert the sort (evidence.md derives both):
invert birth order, or give the pointer 4 references. K3 killed the naive
birth-order spelling because a literal holder is folded away. The open question
is whether ANY source-level shape puts a live HImode/SImode pseudo holding 2 into
the stream ahead of `lw $v0, 0($s0)` while keeping the emitted instruction count
at 219. Note the tie-break trap: equal spans hand the seat to the EARLIER-born
quantity, so the constant must be born strictly earlier AND the pointer must then
be the shorter-lived quantity — that is the same condition, and it is satisfiable
in principle.
NEXT PROBE: dump `.cse`/`.combine` for the K3 build and find exactly which pass
deletes pseudo 212's holder; that names what the value must look like to survive
(and whether the surviving spelling is ordinary C or lands in a FAKE family, in
which case the correct outcome is a ruling-request, not a submission).

### H2 — reshape the statement so the store is not a same-block two-quantity race
The competition exists only because both pseudos live and die inside the
four-insn block at `.L80023B90`. Any shape that changes what else occupies that
block changes both spans. K2 (duplicate into arms) and K0 (hoist above the
if/else) are the two obvious reshapes and both are measured worse. Untried:
reshaping the SUCCESSOR side — the arm currently falls straight into
`j .L80023BE8`; moving the common-tail `*(s32 *)(arg0 + 0x74) = *(s32 *)(arg0 + 0xBC);`
store into this arm changes the block's insn population after the `sh` without
touching the two competing spans, and is a cheap measurement.
MECHANISM: block composition -> suid numbering -> qty_birth/qty_death inputs to
qty_compare_1.

### H3 — solver triage before more hand-probing
This residual is exactly the shape `tools/ra_solver` + `inverse_compose.py
classify` exist for: a two-quantity local-alloc seat assignment with a fully
measured priority computation. Feeding the block (2 quantities, refs 2/2, spans
4/2, class GENERAL_REGS, free set at block entry) to the solver should return
REACHABLE with a ranked C-lever vector, or FORECLOSED — either verdict is worth
more than another hand-spelled probe. This has NOT been run yet.

## [s1] Naming the parent pointer in a local declared immediately before the store (s16 *parent = *(s16 **)arg0; parent[0x143] = 2;) changes the $v0/$v1 seating on the {lw parent, li 2, sh} triple.
- mechanism: expand_assignment evaluates the LHS address before the RHS, so a named pointer local cannot change which pseudo is born first; local-alloc's qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1660) then still ranks the shorter-lived const-2 quantity ahead of the pointer quantity.
- probe: Applied the form to src/code6cac.c over the banked candidate and ran `sandbox func_800238C4 --disable all`, then re-ran the normalized objdump-vs-target diff (tmp/grind/func_800238C4/s1/adiff2.py).
- result: Score 3, unchanged, and the residual instructions are bit-identical to the candidate's (idx 179/180/182 still swapped). A measured no-op, not merely unproven. Banked at rejected/pointer-local-at-use-site-noop-score3.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-07 chassis, asm-until-matched, 0 rules, banked candidate.c body, no FAKE constructs present

## [s1] Duplicating the parent store *(s16 *)(*(u8 **)arg0 + 0x286) = 2; into both arms of the if (s1 < 0x400) if/else re-merges at cross-jump and reproduces the target's post-merge block.
- mechanism: duplicated-statement-into-arms relies on jump2/cross-jump tail-merging the duplicated statement back out; if it merges, the block population and therefore the local-alloc suid spans of the two competing quantities change.
- probe: Applied the duplicated form over the banked candidate and ran `sandbox func_800238C4 --disable all`.
- result: Score 9, i.e. six worse than the floor of 3. Cross-jump does not re-merge the copies and the target's single post-merge block .L80023B90 (lw / li / j / sh-in-delay-slot) is destroyed. The re-merge prong of that family is measured false here, so the family cannot be claimed for this residual at all. Banked at rejected/duplicated-parent-store-into-arms-score9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-07 chassis, asm-until-matched, 0 rules, banked candidate.c body, no FAKE constructs present

## [s1] A named constant intermediate (s16 two = 2; immediately before the store, then storing two) materialises the constant's pseudo ahead of the parent-pointer load and inverts the local-alloc priority sort.
- mechanism: qty_compare_1 priority is floor_log2(n_refs)*n_refs*size/(death-birth); making the constant born earlier than the pointer would make the POINTER the short-lived quantity (pri 10000 vs 5000) and hand it the first free register, with sched2 free afterwards to hoist the lw back ahead of the li to cover the load-delay slot, which is the target's emission order.
- probe: Applied the form over the banked candidate, ran `sandbox func_800238C4 --disable all`, and re-ran the normalized objdump-vs-target diff.
- result: Score 3, unchanged, residual asm bit-identical. The form fails EARLIER than allocation: cse/combine constant-propagates `two` back into the store and deletes the pseudo, so the .lreg RTL is untouched (pointer born suid 4, constant born suid 6, both dead at suid 8 - confirmed against the BB2_QTY_DEBUG capture). A source-level holder of a plain literal does not survive to local-alloc here in any integer width. Banked at rejected/named-const-two-folded-noop-score3.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-07 chassis, asm-until-matched, 0 rules, banked candidate.c body, no FAKE constructs present

---

# s2 (2026-09-07, structural) — CLOSED AT 0

## CONFIRMED — a third quantity in the block, supplied by duplicating the common tail
The seat swap is decided by local-alloc's hand-rolled quantity sort
(tools/gcc-2.7.2/local-alloc.c:1539-1563), not by the qsort/qty_compare_1 path. With the
block holding only two quantities the sort is one comparison and the shorter-lived constant
always wins $v0. Duplicating the common tail `*(s32 *)(arg0 + 0x74) = *(s32 *)(arg0 + 0xBC);`
plus its `goto skip_74;` into the first arm puts a THIRD quantity (the 0xBC load, refs 2,
span 2, pri 10000) in the block after the store; the case-3 arm of the sort compares literal
quantity numbers rather than qty_order slots, so its third comparison undoes the first
exchange and leaves the parent pointer at qty_order[0]. Pointer -> $v0, constant -> $v1.
jump2 cross-jump re-merges the duplicated copy, so the emitted count stays at 219.
MEASURED: sandbox --disable all = 0, 219/219, 0 rules; verify-oracle ok, build SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle.
Family: duplicated-statement-into-arms (2026-08-06 control-transfer-tail scope),
FAKE-annotated. Vet in self_vet.md.

## KILLED — K4: source-level ordering of the constant ahead of the pointer load
s1's H1 lever (a). It fails at the SCHEDULER, one pass earlier than s1 supposed, and
independently of the cse/combine folding that killed K3's spelling: in the standalone
reproduction at tmp/grind/func_800238C4/s2/probe/ a deliberately non-foldable value emitted
ahead of the load is REORDERED behind it by sched1, because the MIPS memory unit gives a load
ready-delay 2 (tools/gcc-2.7.2/config/mips/mips.md:157-159) against 1 for an arith def and
rank_for_schedule (tools/gcc-2.7.2/sched.c:2408) sorts on INSN_PRIORITY before anything else.
Scope: INSTANCE (this block shape — one load and one arith def feeding one store —
on the floor-3 chassis, no FAKE constructs present).

## KILLED — K5: re-spelling the store's address or value expression
Eighteen spellings measured in the standalone harness (batch.py, batch2.py, batch3.py):
pointer local, `(short *)` cast local, `q[0x143]` index, two-step offset, struct member,
uintptr round-trip, alias local, split offset, enum constant, `1 + 1`, `register` storage
class, unsigned width, `int` width, assignment-as-value, assignment chain, comma sequence,
and a preceding-statement variant. Every one produced the same quantity table (pointer
refs 2 birth 2 death 6, constant refs 2 birth 4 death 6) and the same `lw $3 / li $2 / sh $2`
seating. Address and value spelling do not move this seat.
Scope: INSTANCE (these eighteen forms, standalone reproduction of the floor-3 block,
no FAKE constructs present).
