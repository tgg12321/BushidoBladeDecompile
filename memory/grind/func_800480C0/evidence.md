# Evidence bank — func_800480C0

## s1 — recon (2026-09-02, chassis HEAD dd2808d5, -mel, 0 regfix/asmfix rules)

- **Shape.** 74-insn loop: `p = arg0 + (s16)arg1*4; p = arg0 + (*p & ~3);
  count = *p++; while (count--) { word, lhu*4 -> func_800482C8(arg0 + (word & ~3),
  a1v+arg2, a2v+arg3, a3v+arg4, v0v+arg5) }`. It is the 6-arg twin of COMPLETED-C
  `func_80047FBC` (4-arg, src/text1b.c:84) — identical prologue idiom (`addu $s0,$a0`;
  `addu $s2,$s0`), identical loop body, plus two extra stack args (`lw $v1,0x68($sp)`,
  `lw $a0,0x6C($sp)`) sign-extended into $s4/$s3 inside the `if`.
- **Frame forensics.** `.frame $sp,0x58`: outgoing args 0x00-0x17 (5-arg call; `sw $v0,0x10($sp)`
  is the only sp-relative store outside the register saves), vars 0x18-0x37 = 32 bytes with
  ZERO `sw`/`lw` anywhere in `asm/funcs/func_800480C0.s`, regs $s0-$s6+$ra at 0x38-0x57.
  Exactly the same allocated-but-untouched 32-byte region the three text1b siblings carry
  (`engine/volatile_cheats.py:753-767`: func_80047EE8, func_80047FBC, func_800481E8 all
  `("pre_pad", 8)`).
- **Attribution ladder** (`tmp/grind/func_800480C0/s1/ladder.txt`), all `sandbox --disable all`:
  | body | score |
  |---|---|
  | clean (no `arg0 = 0`; pad stripped by detector) | 32 (74/74) |
  | + `arg0 = 0; /* FAKE */` (pad still stripped) | **20** (74/74) — honest floor as the engine reads it today |
  | + `volatile u32 pre_pad[8]` honoured (real build) | **0** — full build SHA1 == oracle (`s1/build.log`) |
  All 20 residual insns at score 20 are sp-offset deltas only (`addiu $sp,-0x38` vs `-0x58`,
  8 saves, 8 restores, 2 stack-arg loads, final `addiu $sp`): the instruction STREAM is
  byte-identical (`s1/cand1_build.txt` vs target, diffed in-session).
- **Why the sandbox cannot read 0.** The volatile-cheat stripper removes every unused local
  array unless `_is_sanctioned_pad` finds an exact `(func, name, count)` row in
  `engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS` (line 746). `engine/` is on the
  scope-grant DENYLIST (`.claude/rules/integration-handoff-self-serve.md:63`,
  `tools/grinder/grindlib.py:402`) — owner-only. The siblings got their rows by owner ruling
  2026-08-20 (func_80047FBC) and the 2026-08-22 parked-but-proven audit (func_800481E8).
- **Canonical gate:** verdict C, distance 20. `scan_hand_coded --single`: LOW 1/8 (S4 only) —
  no canonical-asm signal; not wanted anyway, the C form is proven.
- **Duplicate leads:** `tmp/duplicates_leads.txt` has no entry for func_800480C0; scan S5
  reports no >0.5-jaccard sibling (the twin differs by the two stack args).
- **Retired chassis (2026-08-24 pin):** old pinned floor 36 came from a 33-rule regfix body
  with five register-asm pins (`retired-chassis-2026-08/body.c`); superseded entirely.

- [s1] target .frame 0x58: args 0x00-0x17, vars 0x18-0x37 with ZERO sw/lw in asm/funcs/func_800480C0.s, regs 0x38-0x57 - same layout as siblings func_80047EE8/func_80047FBC/func_800481E8 (engine/volatile_cheats.py:757-767)

- [s1] sandbox strips volatile pre_pad unless _SANCTIONED_UNWRITTEN_PADS (engine/volatile_cheats.py:746) has an exact row; engine/ is scope-grant denylisted (.claude/rules/integration-handoff-self-serve.md:63) - owner-only

- [s1] scan_hand_coded --single func_800480C0: LOW 1/8 (S4 only) - no canonical-asm signal

- [s1] retired 2026-08 chassis floor 36 (33 regfix rules, 5 register pins) is superseded; ledger floor now 20 (honest as the engine reads it) / 0 (bytes proven)

## s2 — structural (2026-09-02, chassis HEAD c8e81f83, -mel, 0 regfix/asmfix rules)

- **The s1 "honest floor 20" was a SANDBOX ARTIFACT and is retired.** The
  volatile-cheat stripper deletes the *declarator text* of `volatile u32
  pre_pad[8];` but leaves the dangling `volatile` qualifier, which then binds to
  the next declaration. The stripped source it actually scores
  (`tmp/sandbox/func_800480C0/src/text1b.c:160-161`) reads `volatile` /
  `u32 *p;` — i.e. the 20 was measured on a `volatile u32 *p` body that no
  session ever wrote. Confirmed by measuring the same body with the pad removed
  from source: 32. The **honest pad-free floor is 32** (73 scorable insns vs
  target 74). A plain non-volatile `u32 pre_pad[8]` also scores 32 (the stripper
  removes it cleanly, no dangling qualifier).
- **Frame gradient instrument** (`tmp/frame_probe.sh func_800480C0 text1b`,
  cpp | cc1 -mel, reads cc1's own `# vars=`): separates "wrong frame" from
  "wrong codegen" and is ~15 s per spelling. Baseline honest body:
  `.frame $sp,56 # vars= 0, regs= 8/0, args= 24`. Target:
  `.frame $sp,88 # vars= 32, regs= 8, args= 24`. **regs and args already match;
  the entire frame gap is vars=32.**
- **`volatile` is NOT load-bearing for the frame slot.** A plain unused
  `u32 pre_pad[8]` compiles to vars=32 exactly like the volatile one
  (`tmp/grind/func_800480C0/s2/frame_ladder.txt`). The volatile in the pad family
  buys engine allowlist eligibility, not codegen — so there is no "non-volatile
  respelling" escape from the ban; the plain form is the forbidden
  dead-vars-local-array family (`.claude/rules/dead-vars-local-array.md`).
- **Phantom-slot ceiling on this body is 8 bytes, not 32.** 13 distinct honest
  structural spellings measured (`tmp/grind/func_800480C0/s2/frame_ladder.txt`):
  the folded loop-guard compare in the func_8003D9A0 spelling
  (`guard = count - 1; if (guard != -1)`) and every derivative of it produce
  `vars= 8` and nothing more. Rotated-while (func_8003DBE4 shape), hoisted inner
  temporaries, HImode narrow accumulators, named pointer intermediates, unsigned
  count, added register pressure, 4 stacked independent guards, per-stack-arg
  guards, nested guards — all measured `vars= 0` or `vars= 8`. No spelling
  reached 16, let alone 32.
- **The 8-byte phantom slot is genuinely zero-cost**
  (`tmp/grind/func_800480C0/s2/v9.s`): the vars region 0x18-0x1F carries no
  `sw`/`lw`; the register saves start at 0x20. The lever is real; it simply
  saturates one quarter of the way to the target, and the best spelling carrying
  it scores 36 — worse than the vars=0 body's 32.
- **The residual at 32 is frame + a callee-saved SEAT ROTATION, and the two are
  coupled.** objdump of the honest build
  (`tmp/grind/func_800480C0/s2/v0_dis.txt`) matches the target
  instruction-for-instruction except (a) every sp-relative offset and (b) the
  callee-saved bindings: the target holds base_addr in `$s2` and sx_arg2..5 in
  `$s6/$s5/$s4/$s3`; the honest build holds them in `$s6` and `$s5/$s4/$s3/$s2`.
  Since the pad-carrying build byte-matches the oracle, adding the 32 frame bytes
  ALSO fixes the seats — the seat rotation is downstream of `get_frame_size`, not
  an independent residual. Attacking seats and frame separately is therefore the
  wrong decomposition, and it explains why s1 read the frame gap as "20 sp-offset
  deltas only": that 20 was measured on the accidental volatile-pointer body,
  which happens to seat correctly.

- [s2] honest pad-free floor = 32; the ledger's 20 was a stripper artifact (dangling `volatile` binds to `u32 *p`) - tmp/sandbox/func_800480C0/src/text1b.c:160

- [s2] target frame decomposition: vars=32, regs=8, args=24; the honest build already matches regs and args exactly - the entire gap is vars

- [s2] 13 structural spellings measured; phantom-slot production saturates at vars=8 (zero-cost, verified no sw/lw in the slot) - tmp/grind/func_800480C0/s2/frame_ladder.txt

- [s2] plain non-volatile unused u32 pre_pad[8] also gives vars=32, so `volatile` is not the codegen agent in the pad family

- [s2] honest build vs target differ by sp offsets AND a callee-saved seat rotation (base_addr $s6 vs target $s2); the pad build gets both right, so frame and seats are one coupled residual

- [s2] Honest pad-free floor of func_800480C0 is 32 (73 scorable insns vs target 74); the ledger's 20 is retired as a sandbox-stripper artifact - engine/cheats.py deletes the pad declarator text but leaves a dangling `volatile` that binds to `u32 *p` (tmp/sandbox/func_800480C0/src/text1b.c:160-161).

- [s2] Target frame decomposes as vars=32, regs=8, args=24 (.frame $sp,88). The honest build already matches regs and args exactly (.frame $sp,56 # vars= 0, regs= 8/0, args= 24) - the entire frame gap is the 32 vars bytes.

- [s2] A plain non-volatile unused `u32 pre_pad[8]` compiles to the same vars=32, so `volatile` buys engine-allowlist eligibility, not codegen; there is no non-volatile respelling that escapes the pad ban without landing in the dead-vars-local-array family.

- [s2] 13 honest structural spellings measured via the cc1 `# vars=` gradient saturate at vars=8; the 8-byte slot is verified zero-cost (no sw/lw in 0x18-0x1F, tmp/grind/func_800480C0/s2/v9.s) but its best sandbox score is 36 vs the baseline 32.

- [s2] The honest build's residual is sp offsets PLUS a callee-saved seat rotation (base_addr $s6 vs target $s2; sx_arg2..5 $s5/$s4/$s3/$s2 vs target $s6/$s5/$s4/$s3), and the pad-carrying build fixes both at once - frame and seats are coupled through get_frame_size.

- [s2] Reusable instrument for this function: tmp/frame_probe.sh func_800480C0 text1b prints cc1's own `# vars=` in ~15 s, separating 'wrong frame' from 'wrong codegen' far more cheaply than the sandbox score.

- [s2] The s1 bytes-proven body (volatile pre_pad[8] + arg0=0, full build SHA1 == oracle) is preserved at memory/grind/func_800480C0/rejected/pad-judge-banned-2026-09-02.c; it remains unusable under the Judge's 2026-09-02 04:28 ruling until an owner grant adds the engine/volatile_cheats.py row.

## s3 — structural (2026-09-02, chassis HEAD f3ba5f78, -mel, 0 regfix/asmfix rules)

- **THE HONEST FLOOR IS 20, NOT 32 — s2's floor was itself a stripper artifact.**
  `engine/volatile_cheats.py::find_dead_param_assigns` removes an **un-annotated**
  `arg0 = 0;` from the scored TU, and `engine/test_engine.py:1094-1116` pins exactly
  that contract ("annotated dead param assign bypassed" / "un-annotated dead param
  assign still flagged"). Every s1 and s2 body wrote the store bare, so the sandbox
  scored a body with the store deleted. Adding the annotation the dead-store family
  requires anyway (`.claude/rules/dead-store-fake-exception.md`) and changing nothing
  else moves the score **32 -> 20** (74 build insns vs 74 target, up from 73).
  Measured this session, both directions, on `tmp/grind/func_800480C0/s3/body_v0a_fake.c`.
- **THERE IS NO CALLEE-SAVED SEAT ROTATION.** s2 recorded base_addr in $s6 against
  the target's $s2 and built two of its three frontier items (ra_solver classify,
  sched_solver base-copy emission order) on that reading. The rotation exists ONLY in
  the stripped sandbox build — it is the downstream consequence of the deleted dead
  store. With the store honoured, cc1 emits `move $18,$16` ($s2 = $s0) and seats
  sx_arg2..5 in $s6/$s5/$s4/$s3, i.e. the target's exact bindings
  (`tmp/grind/func_800480C0/s3/probe.s`). The instrumented cc1's `BB2_ALLOC_DEBUG`
  order confirms it directly (`tmp/grind/func_800480C0/s3/alloc.err`): pseudo 83
  (base_addr, nrefs=4, livelen=52) is allocated third and takes hardreg 18 = $s2.
  **The residual at 20 is purely the frame size** — the instruction stream is
  byte-identical to the target and the 20 differing insns are exactly the 20
  sp-relative operands (1 `addiu $sp`, 8 `sw`, 2 incoming stack-arg `lw`, 8 `lw`,
  1 `addiu $sp`). Both s2 frontier items are therefore closed as chasing a
  non-divergence.
- **New instrument: unallocated-pseudo census.** `tmp/grind/func_800480C0/s3/probe_body.sh
  <body.c>` installs a body, runs the INSTRUMENTED cc1 (`tools/gcc-2.7.2/cc1`) with
  `BB2_ALLOC_DEBUG=1`, and prints the `.frame` line **plus the count of allocnos with
  hardreg=-1**. That is strictly more informative than s2's `# vars=` gradient: it says
  HOW MANY phantom slots a spelling produced, not just the rounded total. ~20 s per
  spelling.
- **The phantom producer is a single ST_REGS-class compare residue, and it caps at
  one.** The `.lreg` dump for the vars=8 spelling
  (`tmp/grind/func_800480C0/dumps/text1b.lreg`) names it: `Register 92 used 2 times
  across 2 insns in block 0; dies in 0 places; ST_REGS or none`. Its preferred class is
  the MIPS condition-register class, so `find_reg` cannot seat it in GR_REGS and
  `alter_reg` gives it a 4-byte stack slot which `assign_stack_local` rounds the frame
  up to 8 for. Eight further spellings this session (for-index, guard+explicit
  remainder, guard+pointer guard, guard with function-scope sign-extends, dual
  independent folded guards, named loop-tail intermediate, break-form infinite loop,
  chained guard-of-guard) every one produced **exactly 0 or 1** unallocated pseudo —
  never 2. Combined with s2's 13 spellings that is 21 measured structural forms with a
  hard ceiling of one 4-byte phantom (vars=8) against the 32 bytes needed.
- **The args-side decomposition is closed.** `args + vars = 0x38` is all the binary
  fixes (saves start at 0x38), so `(args=24, vars=32)` and `(args=56, vars=0)` are
  indistinguishable layouts — s2's third frontier item was a real idea. But
  `current_function_outgoing_args_size` on this body is `MIPS_STACK_ALIGN(20) = 24` in
  all 21 measured spellings, and the only mechanism that raises it is a call with a
  wider argument list. The single call `func_800482C8(...)` takes 5 words; fabricating
  a wider or never-executed call is the **fabricated dead call site** family REFUSED by
  owner ruling 2026-08-17 (`.claude/rules/no-new-park-categories.md:357-366`). All
  three s2 frontier items are now closed.

- [s3] honest floor is 20 (74/74), not 32: the sandbox strips an UN-ANNOTATED `arg0 = 0;` dead param store (engine/volatile_cheats.py find_dead_param_assigns; contract pinned at engine/test_engine.py:1094-1116). Adding the mandatory FAKE annotation and nothing else moves 32 -> 20.

- [s3] s2's callee-saved seat rotation does not exist in an honestly-annotated build: cc1 emits base_addr in $s2 and sx_arg2..5 in $s6/$s5/$s4/$s3, identical to target (BB2_ALLOC_DEBUG: pseudo 83 -> hardreg 18). The rotation was an artifact of the stripped dead store; s2 frontier items 1 and 2 are closed.

- [s3] With the annotation present the residual is purely frame size: the instruction stream is byte-identical to the target and the 20 differing insns are exactly the 20 sp-relative operands.

- [s3] The phantom-slot producer on this body is one ST_REGS-classed compare residue (text1b.lreg: 'Register 92 ... ST_REGS or none'), worth a 4-byte alter_reg slot that rounds the frame to vars=8; 8 new spellings (21 cumulative with s2) all produced 0 or 1 such pseudo, never 2.

- [s3] New instrument tmp/grind/func_800480C0/s3/probe_body.sh prints .frame AND the count of hardreg=-1 allocnos from the instrumented cc1's BB2_ALLOC_DEBUG - a phantom-slot census rather than a rounded total.

- [s3] The (args=56, vars=0) alternative frame decomposition is layout-equivalent to (args=24, vars=32) but requires a 14-word call; the only way to widen current_function_outgoing_args_size here is the fabricated-dead-call-site family REFUSED 2026-08-17 (.claude/rules/no-new-park-categories.md:357-366). s2 frontier item 3 closed.

- [s3] The honest floor of func_800480C0 is 20, not 32: engine/volatile_cheats.py::find_dead_param_assigns strips an UN-ANNOTATED `arg0 = 0;` out of the scored TU (contract pinned at engine/test_engine.py:1094-1116). Adding the FAKE annotation the dead-store family already mandates, and nothing else, moves the sandbox score 32 -> 20 and build_insns 73 -> 74.

- [s3] There is no callee-saved seat rotation on this function. With the dead store honoured, cc1 seats base_addr in $s2 and sx_arg2..5 in $s6/$s5/$s4/$s3 - the target's exact bindings (BB2_ALLOC_DEBUG: pseudo 83 -> hardreg 18). s2's rotation was an artifact of the stripped store.

- [s3] At floor 20 the instruction stream is byte-identical to the target; the 20 differing insns are exactly the 20 sp-relative operands, so the entire residual is the 32-byte `vars` frame delta.

- [s3] The phantom-slot producer on this body is a single ST_REGS-classed compare residue (tmp/grind/func_800480C0/dumps/text1b.lreg 'Register 92 ... ST_REGS or none') worth a 4-byte alter_reg slot that rounds the frame to vars=8. 21 cumulative structural spellings (13 in s2, 8 in s3) never produced a second one.

- [s3] New reusable instrument: tmp/grind/func_800480C0/s3/probe_body.sh installs a body, runs the instrumented cc1 with BB2_ALLOC_DEBUG=1 and prints .frame plus the count of allocnos with hardreg=-1 - a phantom-slot census rather than a rounded total, ~20 s per spelling.

- [s3] All three s2 frontier items are closed: no seat divergence for ra_solver, no base-copy emission-order divergence for sched_solver, and the args-side widening reduces to the REFUSED fabricated-dead-call-site family.

- [s3] A general engine gotcha worth carrying to other functions: any grind body relying on a dead store to a parameter MUST carry the /* FAKE: ... */ annotation or the sandbox silently scores a different program and the recorded floor is inflated.

## s4 — permuter (2026-09-02)

- **Chassis re-measured: floor 20.** `sandbox func_800480C0 --disable all` with
  memory/grind/func_800480C0/candidate.c installed prints `{"score": 20, "target_insns": 74,
  "build_insns": 74}`. The s3 correction (annotated dead param store => 20, not 32) is
  reproducible on HEAD 8395e8f4.
- **Isolated-TU permuter workspace reproduces the full-TU divergence exactly.** A one-function
  TU (common.h + the candidate body, `func_800482C8` left implicitly declared as in the real
  TU) compiles to 74 insns whose only differences from asm/funcs/func_800480C0.s are the 20
  sp-relative operands. Builder: tmp/grind/func_800480C0/s4/mk_ws.sh (reusable for any body).
- **Permuter campaign A** (candidate chassis, base_score 586, 39.4k iterations, `--stack-diffs`):
  best find 442 = 74 insns / 20 objdump diffs / frame 64 — equal to the baseline residual, not
  better. **Campaign B** (vars=8 guard chassis, base_score 857, 10.6k iterations, 39 finds):
  best 583 = 25 objdump diffs / frame 72. Largest frame reached by either campaign is 72, and
  every frame-72 form carries MORE differing insns than the vars=0 baseline's 20. Both
  campaigns harvested with --stop; `permuter_campaign.py status` reports 0 live campaigns.
- **Slot WIDTH is not an axis here.** Four DImode (`long long`) guard spellings do not widen the
  phantom slot; they delete it (vars=0, unalloc_pseudos=0), because the DImode compare residue
  gets allocated. s3 frontier item 1 closed.
- **Phantom multiplicity still capped at one** across six further spellings placing folded
  guards in distinct basic blocks (including inside the loop), against different constants,
  with pointer and 64-bit-multiply carriers. Running total: 27 measured structural forms,
  none above vars=8.
- **The residual is definitionally a get_frame_size() question.** In GCC 2.7.2's mips.c
  compute_frame_size, `total_size = var_size + args_size + gp_reg_rounded` on a
  mips1 / -mno-abicalls build: extra_size is 0 (mips.c:4464), pretend_args_size is added only
  under ABI_64BIT && mips_isa >= 3 (mips.c:4530-4531), and the register-save offsets are
  `args_size + extra_size + var_size + gp_reg_size - 4` (mips.c:4548-4550) so enlarging the
  save area instead of `vars` moves every sp offset the wrong way (confirmed by the frame-72
  9-register spelling). Only 32 bytes of stack LOCALS can close this gap.

- [s4] Chassis re-measured this session: sandbox func_800480C0 --disable all on memory/grind/func_800480C0/candidate.c prints score 20, target_insns 74, build_insns 74 - the s3 floor correction reproduces on HEAD 8395e8f4.

- [s4] An isolated one-function permuter TU (common.h + the candidate body, func_800482C8 left implicitly declared as in the real TU) reproduces the full-TU divergence exactly: 74 insns, differences confined to the 20 sp-relative operands. Reusable builder: tmp/grind/func_800480C0/s4/mk_ws.sh.

- [s4] Permuter campaign A (candidate chassis): base_score 586, 39.4k iterations, best find 442 which is 74 insns / 20 objdump diffs / frame 64 - no honest improvement over the baseline.

- [s4] Permuter campaign B (vars=8 guard chassis): base_score 857, 10.6k iterations, 39 finds, best 583 = 25 objdump diffs / frame 72. Frame 72 is reached by taking a 9th callee-saved register, not by vars=16.

- [s4] Both campaigns harvested with --stop; permuter_campaign.py status reports 0 live campaigns and 0 stale registry entries.

- [s4] DImode carriers delete the phantom slot rather than widening it (vars=0, unalloc_pseudos=0 in all four s64 spellings), while the banked s3 forms still measure vars=8 / unalloc_pseudos=1 in the same batch - the instrument is live.

- [s4] 27 structural spellings across s2/s3/s4 now cap at one unallocated pseudo and vars=8, one quarter of the 32 bytes the target reserves.

- [s4] mips.c compute_frame_size pins total_size = var_size + args_size + gp_reg_rounded on this build (extra_size 0 at mips.c:4464, pretend args excluded at mips.c:4530-4531), so only get_frame_size() locals can close the +32.
