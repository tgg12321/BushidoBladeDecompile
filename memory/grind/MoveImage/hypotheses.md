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

## Session 2 (structural, 2026-08-11)

### H4 — CONFIRMED AND CLOSED. "The residual is the BF28 store group being 4
### slots early; the rect[0] read must precede it."
Statement / mechanism: as written by session 1 — cc1 pins the `0x0($s0)`
rect[0] load behind the `%lo(D_8009BF28)` store via a memory dependence it
cannot disambiguate, so target's ordering means the original source read
rect[0] BEFORE storing BF28; session 1 believed probe G had shown that this
destroys the H2 delay-slot steal.
Probe: 12 forms on the read-ahead axis (sweep 2). Three plain-named-local
placements of `src = arg0[0];` ahead of the store (W1 immediately before it,
W2 first statement of the body, W3 after `packed`), two both-reads-ahead forms,
two variable-reuse forms, two store-order forms, and two walking-pointer forms.
Result: the three plain-local hoists reproduced probe G exactly — 21 / 47, the
steal lost. **The walking-pointer spelling (`rect = arg0; src = *rect++;`)
performs the SAME hoist and measures 2 / 49 — floor 7 -> 2 with the steal
intact.** The emitted `lw $a0,0x0($s0)` moves to target's slot 5 and the
rect[0] value lands in `$a0`, target's register.
Verdict: **CONFIRMED — and the "naive way destroys the steal" caveat was over-
general. It is true only of the plain-local spelling.**

### H5 — KILLED as a lever. "The primitive is one object, not three symbols;
### a different C view of that memory is the route to the ordering."
Mechanism as recorded by session 1 (base-register vs `%lo(SYM)` store forms).
Probe: every re-view that preserves target's store-form split was measured —
all three words through the primitive pointer (`bf24[0..2]`, 8/49), the BF24
word as a plain symbol store with the pointer kept only for the call arg
(12/49 and 14/48), a `u32 *` primitive pointer (2/49, inert), the pointer
post-incremented at its store (9/49), the call argument as `(s32)(bf24 - 2)`
(2/49, inert), and the rect side re-viewed as `s16 *` with an `arg0[2]/arg0[3]`
guard (2/49, inert).
Result: nothing beat the candidate; the forms that DID change the emitted
store shapes all moved AWAY from target. The `%lo` / `0x0($a1)` split target
uses is already exactly what the candidate emits.
Verdict: **KILLED as a lever for the residual.** (The underlying claim that the
three words are one object is still plausible and still un-disproved — it just
does not buy anything, because the C view that produces target's store forms is
the one we already have.)

### H6 — KILLED as a lever. "Copy the matched siblings' convention."
Probe: LoadImage / StoreImage were read (src/display.c:149-163) and their exact
convention transcribed onto MoveImage — one `u32 *` dev-table local, no `fn`
local, the dispatch written as a single inline cast expression
`((s32 (*)(u32, s32, s32, s32))v0[2])(v0[6], ...)` — plus a fully prototyped
function-pointer variant and a declaration-order variant.
Result: all 2 / 49 — byte-identical to the candidate. The sibling convention is
codegen-inert here.
Verdict: **KILLED as a lever.** (Useful negative: it means the candidate's
spelling is already convention-compatible, so no future session should spend
effort "matching house style" on this function.)

### H7 — the entire floor-2 residual, with the mechanism read from cc1's dump.
Statement: the residual is ONE scheduler pick. In the post-guard block cc1's
backward list scheduler reaches T-5 with ready list `80 (prio 3) 91 (prio 2)
97 (1) 95 (1)` and takes insn 80 (the `%lo(D_8009BF2C)` store); target takes
insn 91 (the `lw $a0,0x18($v1)` p[6] load). Everything else in the function is
byte-identical. Therefore target's build satisfies
`priority(91) >= priority(80)`.
Mechanism (measured, from `dumps/display.i.sched2` block 3, not inferred):
cc1 2.7.2 `priority()` is the longest TRUE-dependence path from the block
start; anti/output links have insn_cost 0 and cannot raise a priority.
`priority(80) = 3` because its value comes from insn 78 (the rect[1] load,
depth 2 — an unknown-base `0x4($s0)` load carries a true memory dep on the
preceding `%lo(D_8009BF28)` symbol store). `priority(91) = 2` because its only
true predecessor is insn 83 (the dev-table load, depth 1 — cc1 disambiguates
`%lo(D_8009BE6C)` from both symbol stores and from the `0x0($a1)` store, since
`$a1` carries a REG_EQUIV to `&D_8009BF24`).
What is already KILLED on this hypothesis:
  (a) LOWER priority(80) by making the rect[1] load depth-1, i.e. hoisting it
      above the BF28 store — 10 spellings measured, ALL build_insns 48 (the
      delay-slot steal dies). Banked in
      `rejected/rect1-read-hoisted-above-bf28-store.c`. At most ONE of the two
      rect reads can sit ahead of the BF28 store.
  (b) RAISE priority(91) by re-spelling the dispatch tail or the dev-table
      read — 29 forms, all inert at 2/49 (or worse). Banked in
      `rejected/tail-respellings-inert-at-floor-2.c`.
Next probe (for a NON-structural modality — the structural axis is spent):
  1. FORENSICS: dump `.sched` / `.sched2` for a MATCHED sibling that has the
     same dev-table dispatch (LoadImage, StoreImage, ClearImage) and read the
     priority of ITS `p[N]` argument load relative to its neighbouring stores.
     If a matched sibling shows a dev-table load at depth >= 2, whatever gives
     it that depth is the missing structure.
  2. FORENSICS: instrument or read `sched.c`'s `rank_for_schedule` to check
     the class tie-break (data-dependent / anti-dependent / independent of
     `last_scheduled_insn`). At T-5 the last scheduled insn is 86, the `fn`
     load. If priority(91) can be made to TIE 80 at 3 rather than exceed it,
     the class tie-break should already favour 91 (independent of 86, whereas
     86 depends on 80) — so a +1 on 91 is sufficient, not a +2.
  3. PERMUTER: the search space left is narrow and well-characterised (get one
     more true-dependence edge into the dev-table load without hoisting the
     rect[1] read). Directed PERM_* over the body with the gate
     `build_insns == 49` is a reasonable next modality; any find must still be
     cheat-vetted (a dependence manufactured by a dead construct is a cheat,
     not a lever).

---

## SUPERSEDED FRONTIER (session 1 — kept for provenance)

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

## [s2] H4 (inherited): the floor-7 residual is the D_8009BF28 store group being scheduled four slots early, and closing it requires the rect[0] read to precede that store in RTL order - which session 1 believed was impossible because probe G showed it destroys the H2 delay-slot steal.
- mechanism: cc1 sched.c cannot disambiguate an unknown-base load (`0x0($s0)`, through a pointer parameter) from a SYMBOL_REF store, so memrefs_conflict_p yields a true memory dependence and the rect[0] load is pinned behind the D_8009BF28 store. Target has the load ahead of the store, so the original source read rect[0] first.
- probe: 12 forms on the read-ahead axis (tmp/grind/MoveImage/s2/sweep2.py), each measured with sandbox --disable all and gated on build_insns == 49: three plain-named-local placements of `src = arg0[0];` ahead of the store (immediately before it / first statement of the body / right after `packed`), two both-reads-ahead forms, two variable-reuse forms, two store-order forms, and two walking-pointer forms.
- result: The three plain-local hoists reproduced probe G exactly - 21 / 47, the delay-slot steal lost in every placement. The walking-pointer spelling `rect = arg0; src = *rect++;` performs the SAME hoist and measures 2 / 49: floor 7 -> 2 with build_insns still 49. The emitted `lw $a0,0x0($s0)` moves to target's slot 5 and the rect[0] value lands in $a0, target's register. So the plain-local and walking-pointer hoists are NOT equivalent, and session 1's caveat was over-general - it holds only for the plain-local spelling.
- verdict: CONFIRMED

## [s2] H5 (inherited): D_8009BF24/BF28/BF2C plus the 8 bytes below are one GPU move-primitive object, and a different C view of that memory (struct or array) is the natural route to target's ordering.
- mechanism: cc1 emits `sw $x,%lo(SYM)($at)` for a standalone symbol store but `sw $x,N($base)` for a member store off a materialized base; target uses the %lo form for BF28/BF2C and the `0x0($a1)` base form for BF24 only, so any re-view must preserve exactly that split.
- probe: Every re-view that can preserve the split was measured: all three words through the primitive pointer (bf24[0..2]); the BF24 word as a plain symbol store with the pointer kept only for the call argument (two spellings); a u32 * primitive pointer; the pointer post-incremented at its store; the call argument as `(s32)(bf24 - 2)`; and the rect side re-viewed as `s16 *` with an `arg0[2]/arg0[3]` guard (sweep1 V6/V7, sweep4 Z3/Z7/Z8/Z11, sweep6 B8).
- result: Nothing beat the candidate. bf24[0..2] 8/49; BF24-as-symbol-store 12/49 and 14/48; u32 * primitive pointer 2/49 (inert); `*bf24++` 9/49; `(s32)(bf24 - 2)` 2/49 (inert); s16 * rect parameter 2/49 (inert). Every form that actually changed the emitted store shapes moved AWAY from target - the %lo / 0x0($a1) split target uses is already exactly what the current form emits.
- verdict: KILLED

## [s2] H6 (inherited): the matched siblings LoadImage / StoreImage / ClearImage are the same dev-table-dispatch shape in the same TU, so their statement order is direct evidence for the house convention and MoveImage should be written to match it.
- mechanism: Same TU, same compiler invocation, same call shape - a byte-matched sibling's ordering is the closest available ground truth about the original source's conventions.
- probe: Read LoadImage/StoreImage (src/display.c:149-163) and transcribed their exact convention onto MoveImage: one `u32 *` dev-table local, no `fn` local, dispatch as a single inline cast expression `((s32 (*)(u32, s32, s32, s32))v0[2])(v0[6], ...)`; plus a fully prototyped function-pointer variant and a declaration-order variant (sweep1 V13/V14, sweep4 Z1/Z2, sweep3 Y3).
- result: All measured exactly 2 / 49 - byte-identical to the candidate. The sibling convention is codegen-inert for this function. Useful negative: the candidate's spelling is already convention-compatible, so no future session should spend effort matching house style here.
- verdict: KILLED

## [s2] H7a: the floor-2 residual can be closed by LOWERING the priority of the D_8009BF2C store (cc1 insn 80, priority 3) so the p[6] load (insn 91, priority 2) wins the T-5 scheduler pick - which requires making the rect[1] load depth-1 by hoisting it above the D_8009BF28 store.
- mechanism: cc1 2.7.2's sched.c is a BACKWARD list scheduler picking the highest INSN_PRIORITY ready insn, and priority() is the longest TRUE-dependence path from the block start (anti/output links have insn_cost 0 and cannot raise a priority). priority(80) = 3 because its value comes from insn 78, the rect[1] load, which is depth 2 - an unknown-base `0x4($s0)` load carries a true memory dependence on the preceding %lo(D_8009BF28) symbol store. Removing that dependence makes 78 depth 1 and 80 depth 2, tying 91 and handing it the rank_for_schedule class tie-break (91 is independent of the last-scheduled insn 86; 80 is what 86 depends on).
- probe: 10 spellings that place the rect[1] read ahead of the BF28 store, all measured with sandbox --disable all (sweep5 A1/A2/A3/A4/A5/A7/A8/A9, sweep2 W4/W5): both reads walked ahead, walked twice, only rect[1] hoisted, both ahead with the BF24 store last, the read placed between the two stores, a backwards-walking pointer, both reads hoisted above the pointer materialization, and the plain-local equivalents.
- result: ALL ten measured build_insns 48 (scores 10-20) - i.e. every one of them loses the H2 delay-slot steal and drops an instruction below target's 49. At most ONE of the two rect reads can sit ahead of the D_8009BF28 store while the `sll` keeps the head of the post-guard block, and the rect[0] read is already using that slot. The lever and the constraint are mutually exclusive on the statement-order axis.
- verdict: KILLED

## [s2] H7b: the floor-2 residual can be closed by RAISING the priority of the p[6] load through the dispatch tail - re-spelling how the dev-table pointer, the function pointer and the call arguments are written.
- mechanism: priority(91) = 2 because the p[6] load's only true predecessor is insn 83, the dev-table load, which is depth 1: cc1 disambiguates %lo(D_8009BE6C) from both symbol stores AND from the `0x0($a1)` store (because $a1 carries a REG_EQUIV to &D_8009BF24). Its anti-dependence on insn 75 (shared $a0) contributes cost 0. Any source change that puts one more true-dependence edge into the dev-table load lifts 91 to 3.
- probe: 29 forms across sweeps 3, 4 and 6: inline dispatch; p[6] staged into a local before and after `fn`; `(s32)bf24 - 8` staged into a local; `*(p + 6)`; `(*fn)(...)`; the call result staged into a local and returned; the dev table read twice; `p = g_gpu_dev_table + 6; fn = p[-4]; fn(*p, ...)`; a second pointer `q = p + 6`; the dev-table read placed before the stores, between the stores, and as the first statement of the body; u32 * / prototyped / s32-param / u16-param type views; and declaration-order permutations.
- result: 26 of the 29 measured EXACTLY 2 / 49 - byte-identical output, the p[6] load's slot never moved. The three that differed were all worse: the two-pointer split 4/49, the `+6` base form 4/49, and `p` AND `fn` both hoisted above the stores 13/48 (steal lost). The dispatch tail and the type view cannot reach the residual.
- verdict: KILLED
