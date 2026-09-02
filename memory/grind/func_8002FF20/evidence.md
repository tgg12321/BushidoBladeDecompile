# Evidence bank — func_8002FF20

## s1 (2026-09-01, recon) — BYTES PROVEN ON MAIN

1. **Chassis baseline (this session).** With `INCLUDE_ASM` in place the driver could not measure
   a floor. Applying the s1 body (below) to `src/code6cac_b.c`: `canonical func_8002FF20` =
   ASM-PARTIAL, "11/99 insns canonical-asm (c2, ctc2, lwc2, mtc2, swc2)"; `sandbox func_8002FF20
   --disable all` = **score 0, 99/99, rules_dropped 0** (the sandbox scorer drops the two explicit
   cop2 load-delay `nop`s from both sides, hence 99 not 101 — target `.s` is 101 insns).
2. **Full-build oracle proof.** `& tools/wteng.ps1 main build` with the body in place:
   `sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want` — **MATCH**. Bytes are proven on main
   in the full-link context (the 2 explicit nops inside the gte_ldv0 island ARE required, exactly as
   func_800203B4 measured 2026-09-01: maspsx does not supply them in the full-build context).
3. **The pure-C head is byte-exact with zero coercion.** The head insns (.s L1-L59: identity
   matrix init + three RotMatrix* calls + func_8002EECC + MulMatrix0 + three pos-diff subtracts)
   and the tail (.s L85-L101: three `>>= 1` stores + epilogue) came straight off the retired-chassis
   body (`retired-chassis-2026-08/body.c`) with all 11 cheat constructs deleted (6 register pins,
   3 `move %0,%1` aliasing blocks, 2 free-standing `nop` blocks). No named-intermediate, no dead
   store, no FAKE construct anywhere. The tail `lw/sra/sw` interleave (brief's "secondary residual
   candidate") fell out of sched1 unaided once the pins stopped distorting pressure.
4. **Island spelling.** Four `__asm__ volatile` blocks, character-identical in shape to the
   func_8002FDB0 (`src/code6cac_b.c`, `inline_asm_canonical.txt:268`) and func_800203B4
   (`src/code6cac.c`, `inline_asm_canonical.txt:367`) authorized islands: gte_SetRotMatrix (11
   lines, operand `mat_local`, clobbers $12-$15), gte_ldv0 (9 lines incl. 2 nops, operand `vec`,
   clobbers $12-$14), `.word 0x4A486012` (MVMVA sf=1,rt,v0,none), gte_stlvnl (4 lines, operand
   `vec`, clobber $12). In-island GPR insns are exactly the SDK macro bodies' addressing preamble
   (`move $12,%0` + lw/lhu/sll/or + nops) — nothing else swallowed. Zero pins, zero aliasing
   blocks, zero barriers (cluster-rule check items 2 and 3 satisfied by construction).
5. **The brief's SECOND LEVER was correct and load-bearing.** Islands 2 and 3 share ONE named
   local `vec = (s32 *)((u8 *)arg0 + 0x2C)` as their `"r"` operand; cse.c materializes
   `addiu $v0,$s0,0x2C` once (.s L71) and island 3's `addu $t4,$v0,$zero` (.s L84) reuses $v0
   across the `.word` block. Not measured in isolation (the first form built matched), banked as the
   spelling that works; a session that respells the operand as two distinct expressions should
   expect a 102-insn build (extra `addiu`).
6. **Cluster-grant membership.** func_8002FF20 is a NAMED confirmed carrier of the 2026-09-01
   widened-anchor owner GRANT (`docs/grind/decisions.md:18082` record; membership line
   `docs/grind/decisions.md:18120`; `.claude/rules/cop2-addressing-preamble-cluster.md:156`).
   Its three `$t4` copy sources are `$v0` (.s L60, L72, L84) — non-`$aN`, i.e. covered only by the
   widened anchor, not by the original 2026-08-17 `$aN` literal. Splat tags 8 island insns
   `/* handwritten instruction */`; two unfilled cop2 load-delay nops (.s L80-L81).
7. **Hand-coded scanner (canonical-asm gate, rule-3 path).** `tools/scan_hand_coded.py --single
   func_8002FF20` = score 1/8, tier LOW, only S4 front-loads (5 loads in 8-insn window @ insn 59 —
   that IS the island). No S1/S2/S6. Same LOW tier as func_800203B4; the 2026-09-01 grant record
   explicitly names the LOW whole-function tier as the known GTE-wrapper-misroute artifact (the
   head is compiled C; the gate scores whole functions). Output: tmp/grind/func_8002FF20/s1/scan_hand_coded.txt.
8. **What remains is integration only, under the normal grind gates** (the grant's own execution
   clause: "their islands are covered when (and only when) their C bodies reach zero under the
   normal grind gates"): layer-1 cheat-reviewer -> Judge applying the cluster rule's 4-point
   mechanical check -> driver-written `inline_asm_canonical.txt` line + borderline entry
   ([[judge-sole-gate]] rule 3) -> `verify-oracle --rebuild` -> `queue done`. Zero codegen work remains.

Artifacts: tmp/grind/func_8002FF20/s1/{body.c, body_final.c, code6cac_b_sandbox0.o, bytecmp.py,
scan_hand_coded.txt, apply.py, finalize.py}.

## s1b (2026-09-02, recon — owner directive: grant rescan, cop2 cluster condition 3) — RE-MEASURED ON CURRENT CHASSIS, BYTES PROVEN

9. **Directive executed.** The queue directive "grant rescan 2026-09-02: cop2 cluster condition 3: SDK macro
   body is the template (owner Ruling A 2026-09-02)" was executed this session: the banked candidate body
   (identical to `tmp/grind/func_8002FF20/s1/body_final.c`) was restored into `src/code6cac_b.c` in place of
   the `INCLUDE_ASM` line (HEAD 63e6bcf6 chassis). `canonical func_8002FF20` = ASM-PARTIAL, "11/99 insns
   canonical-asm (c2, ctc2, lwc2, mtc2, swc2)". `sandbox func_8002FF20 --disable all` = **score 0, 99/99,
   rules_dropped 0** (the `cheat_asm_stripped: 33` field is FILE-wide — `engine/sandbox.py:68`
   `inlineasm.write_stripped(stem, ...)` counts every island in code6cac_b.c, not this function's; build_insns
   99 == target proves nothing of this body was stripped). `& tools/wteng.ps1 main build` = **sha1
   62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH**. No codegen change since s1; the chassis move
   (asm-until-matched migration) did not perturb the body.
10. **Ruling A conformance edits (comments only, bytes unchanged — sandbox re-run 0, 99/99).** Owner Ruling A
   (docs/grind/decisions.md "2026-09-02 — OWNER RULING", rule text
   `.claude/rules/cop2-addressing-preamble-cluster.md:163`) requires "every island comment must cite the
   macro name and header line". The s1 comments named island 2 `gte_ldv0` — WRONG macro: the PsyQ
   `gte_ldv0` body is two `lwc2`; the `lhu/lhu/sll/or` VX0/VY0 pack + `mtc2 $0` + `lwc2 $1` body is
   **`gte_ldlv0`** (PsyQ 4.5 `inline_c.h:101-110`, the exact cite the owner ruling and allowlist rows :373/:374
   use). Island 3's `.word 0x4A486012` is the `gte_rtv0()` macro. Comments now name gte_SetRotMatrix(r0),
   gte_ldlv0(r0) [inline_c.h:101-110], gte_rtv0(), gte_stlvnl(r0) and state "verbatim macro body".
   **Header-line caveat:** no copy of PsyQ `inline_c.h` exists on this machine (searched tools/, include/,
   docs/, tmp/sotn-decomp, tmp/psyq_prov/psyz, Desktop/sotn-decomp, WSL home — SOTN fetches the SDK at build
   time; bin/ holds only tarball hashes), so line numbers for gte_SetRotMatrix / gte_rtv0 / gte_stlvnl could
   NOT be verified locally and are cited by header + macro name only, exactly as the operator-written
   allowlist rows for func_8002E838 (:373) and func_80031890 (:374) do. The only line-numbered cite
   (gte_ldlv0 :101-110) is the one the owner ruling itself records.
11. **Grant-door status.** Registry row exists: `tools/grinder/owner_cluster_grants.txt:29` (func_8002FF20,
   widened anchor, owner grant 2026-09-01 decisions.md:18082, row per owner ruling 2026-09-02). Under Ruling C
   the Judge PASS path counts the four islands, finds no `inline_asm_canonical.txt` line, and executes the
   grant through the registry door, titling the commit COMPLETED-INLINE-ASM-CANONICAL. The prior
   borderline.md:360 REFUSAL is marked SPENT by the owner (borderline.md:556). Nothing remains for a grind
   session to do on this function except submit.

Artifacts: tmp/grind/func_8002FF20/s1b/{body.c, body_final.c, fix_comments.py, bank.py}. Candidate:
memory/grind/func_8002FF20/candidate.c (whole-file copy of src/code6cac_b.c with the body applied).
