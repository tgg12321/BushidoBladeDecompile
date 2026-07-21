# Evidence bank — func_8007B844

- WIP rejected_form: {'form': 'Single-statement mask: mask = ((u32)&g_gpu_ot_end) & 0xFFFFFF; *ot = mask;', 'score': 7, 'reason': 'Merging the two `mask =` statements regresses sandbox 6 -> 7. The named-intermediate split (Lever B) is load-bearing; fusion loses the addr-register $v1 alignment that closes the addr-register diff. (Workflow round 1, 2026-06-02.)'}

- WIP rejected_form: {'form': 'Hoist mask init before call: `u32 mask = 0xFFFFFF;` at function entry, live across the vtable call.', 'score': 19, 'reason': 'Massive regression sandbox 6 -> 19. cc1 keeps mask in a callee-save reg across the call, inflating prologue/epilogue cost. The 0xFFFFFF materialization must happen AFTER the call. (Workflow round 1, 2026-06-02.)'}

- WIP rejected_form: {'form': 'Explicit return-value variable: `u32 *ret = ot; ...; *ret = mask; return ret;`', 'score': 6, 'reason': "Copy-prop folds ret back to ot; RTL is identical to the baseline. Sandbox 6, no progress and no detectable codegen change. Same outcome as documented in candidate.c's commentary. (Workflow round 1, 2026-06-02.)"}

- WIP rejected_form: {'form': 'Comma-operator return: `return *ot = mask, ot;` (collapse store + return into one expression)', 'score': 6, 'reason': "Comma operator gets flattened during RTL-gen; produces identical RTL to the baseline statement form. Sandbox 6, no semantic difference for cc1's optimizer. (Workflow round 1, 2026-06-02.)"}

- WIP rejected_form: {'form': 'Split mask and stored value: `u32 mask, val; mask = 0xFFFFFF; val = ((u32)&g_gpu_ot_end) & mask; *ot = val;`', 'score': 7, 'reason': 'Introducing a second-name val regresses 6 -> 7. The single-name mask reuse across the AND and the store is what holds the addr-register in $v1 matching target; renaming the stored value breaks that. (Workflow round 1, 2026-06-02.)'}

- WIP rejected_form: {'form': 'Sibling-asm cross-reference: gpu_ClearOTag (display.c:177, ALREADY matched) has the SAME tail C and emits return-staging-AFTER-store, mirroring our score-6 form.', 'score': None, 'reason': "Decisive sibling EVIDENCE (not a rejected form per se, recorded here for the next session): gpu_ClearOTag emits `lui/addiu D_8009BF30; and; sw $v0,0($s0); addu $v0,$s0,$zero` — exactly the score-6 shape we produce for B844, and gpu_ClearOTag is byte-matched. The same C tail compiles to a matching shape there. The discriminating factor is the function's preceding-body shape (single vtable call vs gpu_ClearOTag's loop), not the tail's C structure. Drives the new sibling-mimicry next_hypothesis. (Workflow round 1, 2026-06-02.)"}

- WIP rejected_form: {'form': 'conditional dead-store: `if (debug >= 2) { ...; new_var = ot; } ...; new_var = ot; *new_var = mask; return new_var;`', 'score': 0, 'build_sha1': '62efab4f73f992798c43e8c730aa43baa10bb4fa', 'reason': "FORBIDDEN per .claude/rules/no-new-park-categories.md + commit 44ef3df (find_dead_conditional_stores detector). Same family as Lever D (dead-param-assign, forbidden 2026-05-31). The inner conditional store is DCE'd; its only effect is to influence RTL pseudo numbering for sched.c priority. Permuter-found across ~36k iters; vetted by the worker per cheats-by-any-spelling and NOT surfaced as a candidate."}

- WIP rejected_form: {'form': "function-pointer return-type lie: declare the vtable entry's return type to coerce $v0 liveness through the call", 'score': None, 'reason': "Cheat-by-spelling — falsely declaring a function pointer's return type to manipulate GCC's value-flow analysis. The actual called function does not return that type; the declaration is a lie that exists solely for codegen control. Same family as inline-asm-policy expanded catalog. Not committed; tested only to rule out."}

- WIP rejected_form: {'form': 'ot[0] = mask; (round 2: array-index store at idx [0] instead of dereference)', 'score': 6, 'reason': "Semantically and syntactically equivalent to *ot=mask; lowers to identical RTL. Sandbox 6 (no change). Documents an attempted re-spelling of the existing form — fails to differentiate the store insn's base-register allocation. Not a new lever, just a confirmation that surface syntax doesn't reach the underlying RTL choice. (Workflow round 2, 2026-06-02.)"}

- WIP rejected_form: {'form': 'u32 *out_ptr; ... out_ptr = ot; *out_ptr = mask; return out_ptr; (round 2: post-call pointer rebinding via separately-named local)', 'score': 6, 'reason': "Copy-prop folds out_ptr back to ot before RTL gen; emitted RTL is identical to baseline. Sandbox 6 (no change). Same family as the prior round's `u32 *ret = ot;` rebinding, but spelled with declaration FIRST and assignment AFTER the call. Confirms that variable name and assignment timing don't defeat copy-prop. (Workflow round 2, 2026-06-02.)"}

- WIP rejected_form: {'form': 'u32 addr; ... mask = 0xFFFFFF; addr = (u32)&g_gpu_ot_end; *ot = addr & mask; (round 2: split-name mask + addr as separate locals, AND in store expression)', 'score': 7, 'reason': "Regresses 6 -> 7. The named-intermediate `mask = ((u32)&g_gpu_ot_end) & mask;` form is load-bearing for addr-register $v1 alignment; splitting mask AND addr into two locals with the AND fused into the store expression breaks Lever B's effect. Essentially the val-split form under a different name, confirming the named-intermediate constraint is tight. (Workflow round 2, 2026-06-02.)"}

- WIP rejected_form: {'form': 'u32 *p; ... p = ot + 0; *p = mask; return p; (round 2: pointer-arithmetic identity to attempt defeat of copy-prop)', 'score': 6, 'reason': "Combine simplifies `ot + 0` to `ot` before flow.c; p is then copy-prop-folded to ot like all other rebinding forms. Sandbox 6 (no change). Confirms that algebraic identities GCC's combine can fold are not viable carriers for surviving rebinding — any new lever in this family must use an expression combine CANNOT fold to ot. (Workflow round 2, 2026-06-02.)"}

- WIP rejected_form: {'form': 'u32 *ret; ... ret = ot; *ret = mask; return ret; (round 3: declaration FIRST, assignment after both calls)', 'score': 6, 'reason': "Copy-prop folds ret to ot. Sandbox 6 (no change). Same lowering as the prior `u32 *ret = ot;` and `u32 *out_ptr;` forms; round-3 confirms that the declaration-vs-assignment ORDER doesn't matter for copy-prop. RTL gen sees ret as a copyable pseudo regardless of where the assignment statement sits. (Workflow round 3, 2026-06-03.)"}

- WIP rejected_form: {'form': 'mask = (u32)ot; return (u32 *)mask; (round 3: SOTN-allowed variable reuse — reuse `mask` for return value per defeat-licm-hoist-var-reuse)', 'score': 7, 'reason': 'Regresses 6 -> 7. The SOTN-allowed variable-reuse technique requires a loop-invariant hoist to defeat (where multi-set defeats single-set movable). func_8007B844 has NO loop — no hoist mechanism exists. Reusing mask just breaks the named-intermediate alignment that Lever B established (sandbox 7). Confirms the SOTN variable-reuse technique is context-specific to loop-invariant defeat, not a general codegen lever. (Workflow round 3, 2026-06-03.)'}

- WIP rejected_form: {'form': 'volatile u32 mask; (round 3: volatile qualifier on the LOCAL mask, not the global)', 'score': None, 'reason': "Rejected at design time per cheats-by-any-spelling intent test. A volatile-qualified local variable holding a constant 0xFFFFFF mask has ZERO semantic purpose — there's no concurrent access, no signal handler, no memory-mapped hardware. Same family as the inline `volatile T *` cast cheat catalog entry, just spelled with a local declaration qualifier. NOT MEASURED — vetted out before sandbox. (Workflow round 3, 2026-06-03.)"}

- WIP rejected_form: {'form': 'direct vtable cast (round 4): `((void(**)(u32 *,s32))g_gpu_dev_table)[11](ot, n);` — collapses the `u32 *v0 = g_gpu_dev_table; ((void(*)(u32 *,s32))v0[11])(ot, n);` two-statement form into one expression', 'score': 6, 'reason': "Combine folds the cast-then-index-deref-call chain to identical RTL as the explicit-`v0` intermediate. Same number of pseudos at the post-call boundary, same callee-save allocation for `ot` ($s0), same return-staging chain depth. Sandbox 6, no change. Confirms that dispatch-load surface syntax (intermediate local vs inline cast) does not affect the post-call dataflow shape that drives sched.c's priority decision. (Round 4, 2026-06-04, HEAD 57c65477.)"}

- WIP rejected_form: {'form': "SOTN-research lever 1a (round 5, 2026-06-04): `ot[n-1] = mask;` — write terminator to the LAST slot rather than slot 0, per tmp/sotn_research_func_8007B844.md's first ranked suggestion", 'score': 9, 'build_insns': 40, 'target_insns': 38, 'reason': "Regresses 6 -> 9. Requires `(n-1)*4` offset arithmetic: `sll $tmp,$n,2; addu $tmp,$tmp,$ot; addiu $tmp,$tmp,-4` (+2 insns). Also semantically writes to byte offset (n-1)*4, not 0 — target asm writes `sw $v1, 0x0($v0)` (verified via asm/funcs/func_8007B844.s line containing `000043AC`). Cannot byte-match target by construction. The research file's hypothesis that 'this changes which pseudo cc1 must schedule' is correct surface-wise but the store base still depends on $ot (the input), NOT $v0 (the return value), so the required REG_DEP_TRUE edge between return-staging and store-base pseudos is not created. Confirms SOTN research's analytical hypothesis but rules out lever as a viable closure."}

- WIP rejected_form: {'form': 'SOTN-research lever 1b (round 5, 2026-06-04): `ot[n] = mask;` — write terminator to slot n rather than slot 0', 'score': 8, 'build_insns': 40, 'target_insns': 38, 'reason': "Regresses 6 -> 8. Same semantic mismatch as lever 1a (target writes offset 0), with one fewer offset-arithmetic insn (no -4 addiu): `sll $tmp,$n,2; addu $tmp,$tmp,$ot` (+1 insn). Same mechanistic failure (store base depends on $ot, not $v0). Confirms research file's per-variant ranking (1b cheaper than 1a) but neither closes the priority gap."}

- WIP rejected_form: {'form': 'SOTN-research lever 2 (round 5, 2026-06-04, REJECTED AT DESIGN TIME): degenerate `do { *ot = mask; } while (--_n);` with `_n = 1;` (or literal `do { *ot = mask; } while (0);`)', 'score': None, 'reason': "Rejected on POLICY grounds without measurement. The `do { } while (0);` literal form is sanctioned ONLY by `.claude/rules/do-while-zero-exception.md` for the LABEL_OUTSIDE_LOOP_P / reorg.c invert-jump peephole mechanism, NOT for sched.c's INSN_PRIORITY chain depth (the documented mechanism here). The rule explicitly states 'Using it to bend other GCC passes reopens the slippery-slope this rule exists to close.' The non-literal `while (--_n);` form either unrolls to identical baseline RTL (no priority effect, single-iter known constant) or emits a back-branch (regression); the introduced `_n = 1;` variable has no semantic purpose (would fail the cheat-reviewer's family check) — same intent as the dead-conditional-store / unused-local-array family. Cheat-reviewer would FAIL the form."}

- WIP rejected_form: {'form': 'v2-research Angle 1 (round 7, 2026-06-05): `g_gpu_debug_func(&D_80015F98, &ot, n)` — make `ot` addressable by passing its address to the varargs debug helper', 'score': 19, 'build_insns': 37, 'target_insns': 38, 'reason': "Regresses 6 -> 19. Mechanism worked partially as v2 research predicted: addressable `ot` is given a stack home, the `move v0,s0` return-staging insn becomes unnecessary (-1 insn vs target 38), but the schedule of every other insn cascades differently and the masked diff explodes. ALSO fails cheats-by-any-spelling intent check: passing `&ot` (a u32**) to a debug helper expecting `ot` (u32*) is a semantic mismatch with no human-programmer justification — sole purpose is forcing addressability for codegen control, same family intent as `(void)&local;` from .claude/rules/dead-vars-local-array.md scalar variant (spelled inside a function call argument instead of a void-cast statement). Cheat-reviewer would FAIL the form. Measured to confirm v2 research's analytical hypothesis (mostly regresses) and to rule out lever empirically."}

- WIP rejected_form: {'form': 'Post-increment + arithmetic rebinding (round 7, 2026-06-05): `u32 *ret = ot; *ret++ = mask; return ret - 1;`', 'score': 6, 'reason': "Combine folds `*ret++ = mask; return ret - 1;` to identical RTL as baseline `*ot = mask; return ot;`. The `ret` pseudo is copy-prop-folded to `ot`, then combine simplifies the +1/-1 pointer arithmetic identity. Sandbox 6 (no change). Joins the rejected-forms family of pointer-rebinding variants (round 1 `u32 *ret = ot`, round 2 `out_ptr = ot`, round 2 `p = ot + 0`, round 3 `u32 *ret declaration-first`) — every form that introduces a fresh ptr pseudo equal to ot ultimately folds. Confirms even non-trivial arithmetic identities (post-increment + -1 offset) are within combine's simplification reach."}

- WIP rejected_form: {'form': "Reverse named-intermediate order (round 10, 2026-06-05): `mask = (u32)&g_gpu_ot_end; mask = mask & 0xFFFFFF; *ot = mask;` — addr loaded first, then masked, vs Lever B's const-first form `mask = 0xFFFFFF; mask = ((u32)&g_gpu_ot_end) & mask; *ot = mask;`", 'score': 7, 'reason': "Regresses 6 -> 7. The Lever B form requires the CONST to be staged into the mask pseudo FIRST, then the address-load to AND-fold into the SAME pseudo — this is what aligns the addr-register to target's $v1. Reversing the order makes cc1 stage the address-load into a fresh pseudo (later renamed via the second assignment), causing the addr to land in a different register and breaking the alignment. Confirms even superficial 2-statement reordering of the same operands breaks Lever B's specific operand evaluation chain; the const-first form is uniquely correct among the variants in this neighbourhood. Joins the score-7 rejected-forms family (single-statement mask fuse, val-split, split-name addr-then-AND-in-store) — all share the broken-Lever-B-alignment failure mode. (Round 10, 2026-06-05, HEAD a5b2d97a.)"}

- WIP rejected_form: {'form': "Fused-AND-into-store with retained mask init (round 11, 2026-06-06): `u32 mask; mask = 0xFFFFFF; *ot = ((u32)&g_gpu_ot_end) & mask; return ot;` — keeps Lever B's first statement (`mask = 0xFFFFFF;`) but eliminates the second rebinding (`mask = ((u32)&g_gpu_ot_end) & mask;`) by inlining the AND into the store RHS", 'score': 7, 'reason': "Regresses 6 -> 7. Mechanism: removing the rebinding statement removes the named-pseudo target for the AND result. cc1 allocates the AND result to a fresh pseudo (not `mask`'s register), which breaks the addr-register $v1 alignment that Lever B established. Distinct from rejected_forms[0] (single-statement mask fuse fuses BOTH mask= statements into one expression `mask = addr & 0xFFFFFF;`); this form keeps the first init statement intact but eliminates the rebinding. Confirms that the named-intermediate REBINDING (not just the named-intermediate's existence) is what holds Lever B's specific addr-register alignment. The mask-init statement is necessary but NOT sufficient — the rebinding into mask's pseudo is ALSO load-bearing. Together with the round-10 finding, pins Lever B's structure as TIGHTLY constrained: exactly 3 statements (mask=CONST; mask=ADDR&mask; *ot=mask;) with no fusion, no reordering, no rebinding-elimination tolerated. Joins the score-7 rejected-forms cluster. (Round 11, 2026-06-06, HEAD d4872471.)"}

- s1 (grind recon, 2026-07-21): **Function identified as PsyQ libgpu `ClearOTagR`** (sys.c v1.129 — the linked-libgpu CVS id in AGENTS.md). gpu_ClearOTag (display.c:194, byte-matched) is `ClearOTag`; func_8007B844 is its DMA-dispatch twin — dev-table slot 11 (offset 0x2C) is the device `otc` (OT-clear-by-DMA) entry; D_80015F98 is the "ClearOTagR(%08x,%d)..." debug string; g_gpu_ot_end is `Gpu_ot_tag`. The widely-known PsyQ source spells the terminator store as a SINGLE const-first statement `ot[0] = 0x00ffffff & (u_long)&Gpu_ot_tag;` and the dispatch as a struct-field call `Gpu_dev->otc(ot, n)`. This gives the original-source shape axis for future probes (struct-typed device table).

- s1 measured probe: const-first SINGLE-statement store `*ot = 0xFFFFFF & (u32)&g_gpu_ot_end;` (the literal leaked-PsyQ spelling) = sandbox **7** (KILLED — regresses like the addr-first single-statement form; operand order does not rescue statement fusion; Lever B's named-intermediate split stays load-bearing).

- s1 measured probe: Lever B with AND operands swapped (`mask = 0xFFFFFF; mask = mask & (u32)&g_gpu_ot_end;`) = sandbox **6** (neutral — cc1 canonicalizes the commutative AND, so second-statement operand order is score-invariant; kills the whole operand-order axis inside Lever B).

- s1 duplicate scan: `tools/find_duplicates.py --threshold 0.75` over asm/funcs — ZERO pairs involving func_8007B844 (artifacts tmp/grind/func_8007B844/s1/dups_all.txt, dups_leads.txt). The near-clone axis is dead; gpu_ClearOTag (structurally different: loop body) remains the only informative sibling and is already exploited.

- s1 target-asm note (re-verified): the store base in target is `$v0` (`sw $v1, 0x0($v0)` after `addu $v0, $s0, $zero`) — the store consumes the return-staging register, which IS the chain-depth mechanism the ledger documents; mask materializes in `$a0` (`lui/ori $a0`), the argument register freed after the second jalr.

- == imported from memory/wip notes.md ==
# func_8007B844 — WIP resume notes (current-state; history in git)

## TL;DR

- **Floor 6** (HEAD floor 7), build_insns 38 == target. Lever: named-intermediate
  `u32 mask;` (Lever B from `register-alloc-pure-c.md`). STABLE across 15 sessions,
  55+ structural variants, ~50k permuter iters — re-confirmed 2026-06-14.
- **Mechanism (instrumented, decisive):** target needs the return-staging
  `move $v0, $s0` (ot→v0) scheduled at idx 23 **BEFORE** the mask/addr chain
  (idx 24-26). cc1 `sched.c` gives return-staging `INSN_PRIORITY = 1` (chain depth
  to `jr ra` is 1); the mask/addr chain (`lui→addiu→and→sw`) has priority 4. To flip
  the schedule, the return-staging pseudo's chain depth must reach ≥4. The only
  pure-C way is for a later insn (the store) to **consume the return-value register
  `$v0`** — which needs GCC to keep `ret_val` a SEPARATE pseudo from `ot` after
  copy-prop. **No pure-C SOTN-allowed construct prevents that copy-prop:** every
  `u32 *p = ot; *p = ...; return p;` form folds to `ot`.
- **Sibling evidence (key):** `gpu_ClearOTag` (byte-matched, display.c:177) has the
  IDENTICAL final-statement C tail and ALSO emits return-staging-AFTER-store at the
  score-6 emission order. So **score-6 IS a legitimate matching emission shape** for
  this C tail; the discriminator is the function's preceding BODY shape (single
  vtable call here vs ClearOTag's loop), not the tail's C structure.

## Resume

1. Apply `candidate.c` to `src/display.c` (replace HEAD's func_8007B844 body).
2. Confirm floor: `& tools/eng.ps1 sandbox func_8007B844 --disable all` → `"score": 6`.
3. Do NOT re-derive the `rejected_forms` (meta.json) — 23 forms already ruled out.

## What's left (the 6-diff residual)

6 register-rotation + scheduling diffs in the final mask/addr/return-staging block:
- cc1 picks `$v0` for the mask; target uses `$a0`.
- return-staging emits AFTER the store; target emits it BEFORE (the priority flip above).
These cascade from the single sched.c priority decision; they are NOT independent.

## FORBIDDEN closing forms (do NOT reach for — preserved in rejected/)

- `rejected/conditional_dead_store.c` — Lever D family (conditional dead-store to
  manipulate $v0 liveness). FORBIDDEN per 2026-06-02 cheat catalog.
- `rejected/fnptr_return_type_lie.c` — return-type-lie. FORBIDDEN.

## Untried resume avenues (for the next session — change MODALITY)

1. **Instrumented `BB2_SCHED_DEBUG` on the score-6 form** to read the return-staging
   pseudo's `REG_DEP_TRUE` predecessor list directly — the closing lever must extend
   that chain WITHOUT emitting bytes (combine-foldable) AND without coercing RA into
   the no-new-park-categories family.
2. **Fresh SOTN borderline research** for a community pattern that affects sched.c
   return-staging priority specifically (none found in 15 sessions so far).
3. **Sibling cross-ref of other thin GPU vtable-dispatch wrappers** in display.c:
   does any match pure-C while emitting return-staging BEFORE the store? If so, what
   natural body structure raised its return-staging chain depth?

## Status

INCOMPLETE in queue. Honest floor 6 stable; no pure-C path found across 15 sessions.
Not a GTE/canonical-asm candidate (ordinary C; the gap is a cc1 sched.c priority tie).
No new park category requested (per [[no-new-park-categories]]).


- [s1] func_8007B844 IS PsyQ libgpu ClearOTagR (sys.c v1.129): gpu_ClearOTag=ClearOTag twin, dev-table slot 11 (offset 0x2C)=device otc DMA-clear entry, D_80015F98=debug format string, g_gpu_ot_end=Gpu_ot_tag; known PsyQ source spells the tail `ot[0] = 0x00ffffff & (u_long)&Gpu_ot_tag;` and dispatches via struct field `Gpu_dev->otc(ot,n)`

- [s1] Baseline re-confirmed this session: candidate.c (Lever B) applied to src/display.c -> sandbox --disable all = 6, build_insns 38 == target 38, canonical verdict C

- [s1] Target-asm mechanism re-verified: store base is $v0 (`addu $v0,$s0,$zero` then `sw $v1,0($v0)`) — the store consumes the return-staging register; mask materializes in $a0 (freed arg register)

- [s1] Const-first single-statement store = 7 (killed); Lever B AND-operand swap = 6 (neutral); operand-order axis around the AND is fully dead

- [s1] Duplicate scan (opcode-class Levenshtein, threshold 0.75): no near-clones of func_8007B844 anywhere in asm/funcs

- [s2] Frontier F1 KILLED: struct-typed device-table dispatch (faithful GpuDevice struct, otc fn-ptr field at offset 0x2C) = sandbox 6 in BOTH spellings (typed local `dev->otc(ot,n)` AND direct `((GpuDevice*)g_gpu_dev_table)->otc(ot,n)`) — identical to untyped v0[11]; the dispatch-load surface type never reaches the post-call pseudo landscape.

- [s2] Two-local addr-rebind AND (`mask=0xFFFFFF; addr=(u32)&g_gpu_ot_end; addr=addr&mask; *ot=addr;` — the only unmeasured shape mirroring target's `and $v1,$v1,$a0` dest-choice) = 7; decl-order swap (addr first) also = 7. Disassembly (tmp/grind/func_8007B844/s2/): the STORED pseudo always seizes $v0 (first free in REG_ALLOC_ORDER across the tail), the other local gets $v1; declaration order is RA-invariant. No 2-local permutation can reproduce target's mask-in-$a0 without $v0 being occupied across the tail — RA-side confirmation of the sched cascade diagnosis.

- [s2] Pseudo-numbering/placement axis dead: block-local tail decl (`{ u32 mask; ... }` after the call) = 6; fn-scope dispatch-pointer decl (`u32 *dev;` hoisted, gpu_SendPacket spelling) = 6; s32 retype of mask = 6. All neutral — the plateau is insensitive to pseudo creation order/timing and tree-type signedness.

- [s2] Duplicated-tail-into-debug-arm ref-lift (sanctioned duplicated-statement-into-arms shape) = 22, build_insns 44: jump2 cross-jump does NOT re-merge the duplicated dispatch+tail suffix (+6 insns) — not byte-neutral, fails the sanction prerequisite AND regresses. The technique is inapplicable to this single-flow function.

- [s2] Structural modality is now EXHAUSTED for this function: every catalog structural lever (var splits, decl order/placement, type retype, statement re-association, dispatch surface type, arm duplication) is measured dead around the floor-6 plateau. Remaining live axes are non-structural: F2 (instrumented sched dump forensics) and F3 (cross-project ClearOTagR research).

- [s2] Floor 6 re-confirmed at session start and end with candidate.c applied to src/display.c (build_insns 38 == target 38); src left in the candidate (Lever B) form

- [s2] Disassembly of the score-6 build banked (tmp/grind/func_8007B844/s2/build_form_c.dis): our AND dest is the mask pseudo in $v0 (`and v0,v1,v0; sw v0,0(s0); move v0,s0` last) vs target `and v1,v1,a0; sw v1,0(v0)` with return-staging early — the full 6-diff residual is visible as one coupled cascade

- [s2] RA-side confirmation of the sched diagnosis: in every 2-local tail split the stored pseudo takes $v0 because the folded return only hard-uses $v0 at the very end; target's mask-in-$a0 requires $v0 live across the tail, i.e. the return-staging schedule flip

- [s2] Structural modality is exhausted: dispatch surface type, var splits, decl order/placement/timing, type retype, statement re-association (killed s1), and arm duplication are all measured dead around floor 6

- [s3] s3 baseline re-confirmed start AND end: candidate.c (Lever B) in src/display.c -> sandbox --disable all = 6, build_insns 38 == target 38; src left in candidate form

- [s3] Structural axis is now exhaustively MEASURED (s1+s2+s3 combined): debug arm (local split 6), dispatch load shape (typed struct 6, index cast 6, byte-offset re-association 6), tail statement forms (1/2/3-statement, both AND dests, both operand orders), decl order/placement/timing, s32 retype, control flow (goto-end 6, arm duplication 22), cross-call placement (hoist before both calls 19, before dispatch only 20). Every neutral form lowers to identical RTL; every non-neutral form regresses

- [s3] New Lever B refinement from P4: a named addr intermediate is tolerated (copy-prop-transparent, score 6) when the AND destination is mask; it breaks the alignment (7) only when the AND destination is addr — the load-bearing constraint is the AND-dest pseudo, not the local count

- [s3] P5 extends the round-1 hoist kill: 0xFFFFFF materialization must occur strictly AFTER the LAST call (live-across-one-call = 20/build 40, live-across-both = 19); any pre-call constant staging is dead by prologue construction

- [s3] Target-asm re-verified this session: store base $v0 is the return-staging register (addu $v0,$s0 BEFORE lui/addiu $v1/and/sw), mask materializes as lui/ori into $a0, debug load is lbu (g_gpu_debug_level correctly u8 in include/gpu.h)
