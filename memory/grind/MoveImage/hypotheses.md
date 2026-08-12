# HYPOTHESES — MoveImage

## Session 1 (recon, 2026-08-11)

### H1 — KILLED. "The 2-instruction shortfall is a guard-SPELLING problem."
Statement: the missing `j <epilogue> / addiu $v0,$zero,-1` pair comes from the
second zero-check being emitted in the collapsed `beqz` sense, and re-spelling
the guard in C will flip it.
Mechanism: `jump.c:1764` / `reorg.c:3877` collapse `bnez L2 / j Lend / L2:`
into `beqz Lend`; the collapse is blocked when an active insn sits between the
conditional branch and the `j`, so a C spelling that keeps the `v0 = -1` block
inline between them should survive.
Probe: three spellings compiled and disassembled —
(A) `if (h != 0) { body; return call; } return -1;`
(B) `if (w == 0) goto err;` ... `err: return -1;` (mixed exit forms)
(C) `if (w == 0 || h == 0) return -1;`
Result: **all three byte-identical to baseline** — 47 insns, score 21, same
`beqz $v0,<epi> / li $v0,-1` twice. GCC normalizes if/goto/`||` guard spellings
to the same RTL here.
Verdict: **KILLED.** Do not spend another session re-spelling the guards.

### H2 — CONFIRMED. "The branch shape is decided by the BODY, not the guard."
Statement: the guard's emitted sense is a downstream consequence of which
instruction sits at the head of the post-guard basic block, because reorg's
eager delay-slot fill steals from the taken thread.
Mechanism: `reorg.c` `fill_eager_delay_slots` → `fill_slots_from_thread` takes
the first instruction of the TAKEN thread into the delay slot; taking from the
taken thread forces the branch to point at the body, which leaves the
`v0 = -1; j <epilogue>` block inline on the fall-through — target's shape.
Probe: hoist `D_8009BF28 = packed;` to be the first of the three primitive
stores, making `sll $v0,$s1,16` the head instruction of the body block.
Result: **floor 21 → 7, build_insns 47 → 49 (== target 49)**; the emitted guard
became `bnez $v0,.L720` (delay `sll $v0,$s1,16`) + `j <epi>` (delay
`addiu $v0,$zero,-1`), byte-identical to target. Converse probe G (a named
`src = arg0[0];` local placed before the BF28 store, so the load outranks the
`sll` at the block head) took it straight back to 47 insns / score 21.
Verdict: **CONFIRMED, and it is the load-bearing lever for this function.**

### H3 — KILLED. "Hoisting the g_gpu_dev_table read toward the top helps."
Statement: target loads `g_gpu_dev_table` into `$v1` early (slots 6-7 of the
body block), so moving `p = (s32 *)g_gpu_dev_table;` / `fn = p[2];` earlier in
the C should reproduce that.
Mechanism: source order sets LUID, which breaks priority ties in cc1's list
scheduler.
Probe: E (`p =` only, hoisted above the stores), H (both `p =` and `fn = p[2]`),
I (`p =` between the BF28 and BF24 stores).
Result: E → 9/49, H → **10/48** (loses an instruction again), I → 9/49. All
strictly worse than the 7/49 form; none moved the BF28 store group later.
Verdict: **KILLED as a standalone lever.** The dev-table load's position is not
independently steerable from the C; it moves as part of the store/load cluster.

---

## LIVE FRONTIER (for session 2+)

### H4 — the whole remaining residual: the BF28 store group is 4 slots early.
Statement: the entire floor-7 residual is one displacement — our
`lui $at,%hi(D_8009BF28)` + `sw $v0,%lo(D_8009BF28)($at)` pair is scheduled
BEFORE the `lw $a0,0x0($s0)` (rect[0]) read, the `g_gpu_dev_table` load and the
`addiu $a2,$zero,0x14`, whereas target schedules all three of those first. The
`$a0`-vs-`$v0` rename on the rect[0] read is a consequence of the same
displacement (target keeps `packed` live in `$v0` four instructions longer), so
one fix should close all 7.
Mechanism to investigate: cc1's `sched.c` treats a store to a SYMBOL_REF MEM
and a load through a REG base (`0x0($s0)`, a pointer parameter) as possibly
aliasing (`memrefs_conflict_p` cannot disambiguate a symbol against an unknown
pointer), so the rect[0] load is pinned BEHIND the BF28 store by a true/anti
dependence edge. Target has the load ahead of the store, which under that model
means the ORIGINAL source read `rect[0]` before storing BF28 — but probe G
showed the naive way of doing that (a named local assigned before the store)
destroys the delay-slot steal that H2 depends on. The next session's job is to
find the ordering that satisfies BOTH constraints at once.
Next probe: dump the RTL with the sandbox's exact cc1 flags
(`cc1 <flags> -da base.i`, dumps to `tmp/grind/MoveImage/s2/`) and read the
`.sched2` and `.dbr` dumps for the body block. Specifically confirm or refute
the alias-dependence story by checking whether a dependence edge exists between
the BF28 store and the `0x0($s0)` load in the sched2 dependency list. If the
edge exists, the ordering must come from source order and the search is over
statement permutations that keep `sll` at the block head (verify
`build_insns == 49` on EVERY probe, not just the score). If there is no edge,
this is a pure INSN_PRIORITY tie and the lever is dependency-chain length —
e.g. sinking the BF28 store by lengthening the chain that feeds it, or reading
`rect[0]` through an expression whose critical path outranks the store.

### H5 — the primitive may be one object, not three symbols.
Statement: D_8009BF24 / D_8009BF28 / D_8009BF2C plus the 8 bytes below
(`&D_8009BF24 - 8`) are one GPU move-primitive object; the original C very
likely declared a single struct/array and the splat per-word `D_` names are an
artifact ([[splat-symbol-names-are-not-evidence]]). A different C *view* of the
same memory (e.g. `s32 mv[5]` based at D_8009BF1C with `mv[2]`/`mv[3]`/`mv[4]`
writes, or a struct with named fields) would change which stores share a base
register and therefore the scheduler's dependence graph — plausibly the natural
route to H4's ordering.
Mechanism: cc1 emits `sw $x,%lo(SYM)($at)` for a standalone symbol store but
`sw $x,N($base)` for a struct-member store off a materialized base. Target uses
`%lo` form for BF28 and BF2C and the `0x0($a1)` base form for BF24, which
constrains the shape: BF24 must be reached through the same pointer that ends
up decremented by 8 in the `jalr` delay slot, while BF28/BF2C must be
standalone-symbol stores. Any re-view must preserve that split.
Next probe: check `bb2.ld` / `undefined_syms_auto.txt` / `symbol_addrs.txt` for
what actually lives at 0x8009BF1C-0x8009BF2F and whether any other function in
the tree touches those words (grep `8009BF1C|8009BF20|8009BF24|8009BF28|8009BF2C`
across `src/` and `asm/funcs/`); a second user of the same object would pin down
its real shape. Beware [[base-register-store-invisible-to-symbol-grep]] — also
grep for stores through a base register into the neighbourhood.

### H6 — sibling analogs in the same wrapper family.
Statement: `LoadImage` / `StoreImage` / `ClearImage` / `ClearImage2` at
src/display.c:129-163 are the same libgpu dev-table-dispatch shape and are
already matched; their statement order is direct evidence for the house style
the original author used, and `MoveImage` should be written to match it.
Mechanism: same TU, same compiler invocation, same call shape — a matched
sibling's ordering is the closest thing to ground truth about the original
source's conventions.
Next probe: diff the emitted schedule of `LoadImage`/`StoreImage` against their
C and confirm whether the dev-table read is written last (it is, in the current
matched source) and whether any sibling stores globals before the dispatch; then
apply the same convention to MoveImage's three stores. Also run
`tools/find_duplicates.py` / check `tmp/duplicates_leads.txt` for a
byte-similar sibling elsewhere in the tree.

---

## Constraints for every future session on this function
1. **Score alone is not enough — always report `build_insns`.** A form at
   score < 21 with `build_insns == 47` has lost the H2 delay-slot steal and is
   a dead end no matter how good the score looks.
2. `do { } while (0)` / `LABEL_OUTSIDE_LOOP_P` coercion is NOT authorized here.
   The branch shape was reached by ordinary statement order (H2); reaching for
   the carve-out would fail the lever-exhaustion prerequisite outright.
3. No dead stores, no named-constant holders, no pins, no volatile. Nothing in
   the current 7-floor form is FAKE and it must stay that way unless a genuine
   lever-exhaustion record exists.

## [s1] The 2-instruction shortfall (47 vs 49) is a guard-SPELLING problem: re-spelling the second zero-check in C will stop jump.c from collapsing `bnez L2 / j Lend / L2:` into `beqz Lend` and restore target's `j <epilogue> / addiu $v0,$zero,-1` pair.
- mechanism: tools/gcc-2.7.2/jump.c:1764-1784 collapses a conditional jump that jumps over an unconditional jump, guarded by `prev_active_insn (reallabelprev) == insn && no_labels_between_p (insn, reallabelprev) && simplejump_p (reallabelprev)`; the same peephole reappears in reorg.c:3877 and reorg.c:4085. An active insn (the `li $v0,-1`) left between the branch and the `j` blocks it.
- probe: Compiled and disassembled three guard spellings against the sandbox: (A) inverted second guard with the body inside `if (h != 0) { ... }` and a trailing `return -1;`; (B) mixed exit forms, `if (w == 0) goto err;` with a trailing `err: return -1;`; (C) short-circuit `if (w == 0 || h == 0) return -1;`.
- result: All three produced output BYTE-IDENTICAL to the baseline: score 21, build_insns 47, `beqz $v0,<epilogue>` + `li $v0,-1` emitted twice. cc1 normalizes if / goto / `||` guard forms to the same RTL before jump.c runs.
- verdict: KILLED

## [s1] The guard's emitted branch shape is not controlled by the guard's spelling at all — it is a downstream consequence of which instruction sits at the head of the post-guard basic block, because reorg's eager delay-slot fill steals from the TAKEN thread and stealing from the taken thread forces the branch to point at the body.
- mechanism: tools/gcc-2.7.2/reorg.c fill_eager_delay_slots -> fill_slots_from_thread takes the first instruction of the taken thread into the branch's delay slot. If that fill happens, the `v0 = -1; j <epilogue>` block stays inline on the fall-through, which is exactly target's `bnez $v0,.L720 (delay: sll) / j .L8007B770 (delay: addiu $v0,-1)` shape and supplies the 2 missing instructions.
- probe: Hoisted `D_8009BF28 = packed;` to be the FIRST of the three primitive stores, so the packed chain (sll -> andi -> or) is the top-priority work at the block head; then ran the converse probe G, inserting a named local `src = arg0[0];` before that store so the rect[0] load outranks the `sll` at the block head.
- result: Hoist: floor 21 -> 7 and build_insns 47 -> 49 (== target_insns 49); disassembly shows `bnez $v0,.L720` with `sll $v0,$s1,16` in the delay slot and `j <epilogue>` with `addiu $v0,$zero,-1` in its delay slot, byte-identical to target. Knock-on: `packed` stays live in $v0 four insns longer, so the rect[0] read moves toward target's $a0. Converse probe G regressed straight back to 21 / 47 insns, confirming the block-head instruction is the causal factor.
- verdict: CONFIRMED

## [s1] Target loads g_gpu_dev_table into $v1 early in the body block (slots 6-7), so hoisting `p = (s32 *)g_gpu_dev_table;` / `fn = p[2];` earlier in the C will reproduce that placement and push our too-early D_8009BF28 store group later.
- mechanism: Source order sets LUID, which breaks INSN_PRIORITY ties in cc1's sched.c list scheduler, so moving a read earlier in the C should move it earlier in the emitted schedule.
- probe: Three placements measured from the 7-floor form: E = `p =` hoisted above the three stores with `fn = p[2]` left below; H = both `p =` and `fn = p[2]` hoisted above the stores; I = `p =` placed between the D_8009BF28 store and the D_8009BF24 store.
- result: E -> score 9 / 49 insns; H -> score 10 / 48 insns (loses an instruction, i.e. breaks the delay-slot steal); I -> score 9 / 49 insns. All strictly worse than 7, and none moved the BF28 store group later in the schedule.
- verdict: KILLED
