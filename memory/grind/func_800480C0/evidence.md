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

## s5 — synthesis (2026-09-02, chassis HEAD 118e147d, -mel, 0 regfix/asmfix rules)

- **Chassis re-measured: floor 20.** `sandbox func_800480C0 --disable all` with
  `memory/grind/func_800480C0/candidate.c` installed prints
  `{"score": 20, "target_insns": 74, "build_insns": 74}`. (HEAD itself, carrying
  `INCLUDE_ASM`, reads 74 / `no_c_body: true` — do not mistake that for the floor.)

- **THE UNIT OF A PHANTOM SLOT IS 8 BYTES, NOT 4. s3's "one 4-byte ST_REGS slot
  rounded up to vars=8" is corrected: one unallocated pseudo == 8 bytes of `vars`,
  exactly.** Measured tree-wide this session by cross-correlating cc1's own
  `# vars=` term with the instrumented cc1's `BB2_ALLOC_DEBUG` hardreg=-1 count,
  per function, over all 32 TUs (`tmp/grind/func_800480C0/s5/census3.py`,
  `s5/*.alloc.err`):
  | function | unalloc pseudos | untouched vars |
  |---|---|---|
  | func_80042874 (text1a_c) | 6 | 48 |
  | func_80041E10 (text1a_post) | 3 | 24 |
  | get_cs / get_ce (display), func_8003FECC (config), func_80038170 (code6cac_c_mid), func_80040594 (text1a_pre), SsSeqCalledTbyT / _SsSeqPlay / SpuSetCommonAttr (main) | 2 | 16 |
  | ~30 further functions | 1 | 8 |
  **Consequence for this function: the target's 32 untouched bytes are exactly FOUR
  unallocated pseudos (or one 32-byte aggregate), not "eight 4-byte slots". Every
  spelling in the ledger's 27-form ceiling produced ONE, i.e. 8 of the 32 bytes —
  the gap is 3 more phantoms, and the ledger's own arithmetic was off by 2x.**

- **s4 frontier item 1 (the precedent census) is ANSWERED, and it is a HIT, not the
  clean negative the frontier anticipated.** Instrument:
  `tmp/grind/func_800480C0/s5/census.sh` compiles all 32 `src/*.c` with the project
  cc1 to `s5/asm/*.s`; `census2.py` parses every `.frame ... # vars=/regs=/args=`
  line, computes the vars window `[args, args+vars)`, and reports functions with
  ZERO `N($sp)` traffic in that window AND zero uses of `$sp` as a source operand
  (the latter filter removes address-taken locals — without it the census
  false-positives on 21 functions such as func_8006C168). Result: **184 functions
  have vars>0; 80 have an untouched vars window; 59 of those never take a frame
  address.** Only 5 of the 59 carry a sanctioned pad row. So allocated-but-never-
  touched `vars` from ORDINARY C, with no pad aggregate and no spill, is common on
  main — 54 instances — and **eight of them reach 16 bytes (two phantoms) from
  ordinary C**. The pad is therefore NOT "the only known producer of untouched vars";
  it is the only known producer of *four at once* on a body of this shape.

- **The producer class is the same one the s3 `.lreg` read named, at multiplicity 2.**
  `tmp/grind/func_800480C0/s5/dumps/display.lreg`, function `get_cs`:
  `Register 85 used 2 times across 2 insns in block 1; ST_REGS or none.` and
  `Register 99 used 2 times across 2 insns in block 6; ST_REGS or none.` — two
  ST_REGS-classed compare residues, in two different basic blocks, from two
  ordinary `x < 0 ? 0 : (x > lim ? lim : x)` clamp ternaries (`src/display.c:556`).
  func_800480C0's single residue (s3: `Register 92 ... ST_REGS or none`) is the same
  animal. Multiplicity, not mechanism, is what this function lacks.

- **A SECOND, independent phantom producer exists and is inapplicable here: the
  DImode HILO scratch of `mult`/`div`.** `func_80041E10` (`src/text1a_post.c:465`) is
  five statements, a leaf, regs=0, args=0, and reserves `vars=24` with zero traffic —
  three `mult`s (from three `/255` magic-number divisions) leaving three unallocated
  8-byte DImode scratches. `func_80042874`/`func_80042A88` reach 6 phantoms / 48 bytes
  the same way. **`asm/funcs/func_800480C0.s` contains ZERO `mult`/`multu`/`div`/`divu`
  instructions**, so this producer cannot be spelled here without materialising
  instructions the byte-identical 74-insn stream does not have.

- **Tree-wide bound on the folded-compare producer: no function in this tree that
  contains no `mult`/`div` exceeds TWO phantoms (16 bytes).** Across the 59 census
  candidates, the zero-`mult`/`div` subset (45 functions) shows untouched-vars values
  of exactly {8, 16, 32}, and every 32 is one of the five sanctioned-pad functions
  (whose 32 bytes come from the declared array — `BB2_ALLOC_DEBUG` reports ZERO
  unallocated pseudos for func_80047EE8/func_80047FBC). So four folded-compare
  residues on a mult-free body has no in-tree precedent at all.

- [s5] One unallocated pseudo reserves exactly 8 bytes of `vars`, not 4: measured across all 32 TUs by correlating cc1's `# vars=` with the instrumented cc1's BB2_ALLOC_DEBUG hardreg=-1 count (6 phantoms->48 in func_80042874, 3->24 in func_80041E10, 2->16 in eight functions, 1->8 in ~30). The target's 32 untouched bytes are FOUR phantoms, and the ledger's 27-spelling ceiling of one phantom covers 8 of the 32 bytes.

- [s5] s4 frontier item 1 (precedent census) answered and it is a HIT: 59 functions on main carry an allocated-but-untouched vars window with no frame-address use, only 5 of which have a sanctioned pad row; eight ordinary-C functions reach 16 untouched bytes (two phantoms). Instruments: tmp/grind/func_800480C0/s5/{census.sh,census2.py,census3.py}.

- [s5] The two-phantom producer is the same ST_REGS-classed compare residue as this function's single one, at multiplicity 2 in distinct basic blocks: display.lreg get_cs `Register 85 ... block 1; ST_REGS or none` + `Register 99 ... block 6`, from two ordinary clamp ternaries at src/display.c:556.

- [s5] A second phantom producer exists — the DImode HILO scratch of mult/div, 8 bytes each (func_80041E10: three /255 divisions -> three mults -> vars=24 with zero traffic; func_80042874: 16 mults -> 6 phantoms -> vars=48). asm/funcs/func_800480C0.s contains zero mult/multu/div/divu, so it is inapplicable without materialising instructions.

- [s5] Tree-wide bound: among the 45 census candidates with no mult/div, untouched-vars takes only the values {8, 16, 32}, and every 32 belongs to one of the five sanctioned-pad functions (BB2_ALLOC_DEBUG reports ZERO unallocated pseudos for func_80047EE8/func_80047FBC, i.e. their 32 bytes are the declared array, not phantoms). Four folded-compare residues on a mult-free body has no precedent in this tree.

- [s5] Three further two-branch guard spellings (guards folded onto BOTH the entry test and the do-while backedge; the same with mixed signed/unsigned guards; a get_cs-style `(u32)g < K` range test on the entry) still report unalloc_pseudos = 1 / vars = 8, or 0 / 0 for the range form, and all three take a NINTH callee-saved register (regs= 9/0), moving every sp offset further from the target. Running total 30 measured structural forms.

- [s5] Chassis re-measured this session: HEAD (INCLUDE_ASM) reads 74 with no_c_body true; with memory/grind/func_800480C0/candidate.c installed, sandbox --disable all prints score 20, target_insns 74, build_insns 74. src/text1b.c was restored to HEAD afterwards.

- [s5] One unallocated pseudo reserves exactly 8 bytes of vars, not 4 (s3's model corrected): measured across all 32 TUs by correlating cc1's `# vars=` term with the instrumented cc1's BB2_ALLOC_DEBUG hardreg=-1 count - 6 phantoms->48 bytes (func_80042874), 3->24 (func_80041E10), 2->16 (eight functions), 1->8 (~30 functions). The target's 32 untouched bytes are FOUR phantoms.

- [s5] s4 frontier item 1 answered as a HIT: 184 functions have vars>0, 80 have an untouched vars window, 59 of those never take a frame address, and only 5 of the 59 carry a sanctioned pad row - so 54 ordinary-C COMPLETED-C bodies on main reserve untouched frame bytes with no pad aggregate and no spill.

- [s5] Eight ordinary-C functions on main reach 16 untouched bytes (two phantoms): get_cs, get_ce (src/display.c:556), func_8003FECC, func_80038170, func_80040594, SsSeqCalledTbyT, _SsSeqPlay, SpuSetCommonAttr. Same producer class as this function's single residue - display.lreg: 'Register 85 used 2 times across 2 insns in block 1; ST_REGS or none' and 'Register 99 ... block 6', from two ordinary clamp ternaries.

- [s5] A second phantom producer exists - the DImode HILO scratch of mult/div, 8 bytes each (func_80041E10: three /255 divisions -> three mults -> vars=24 untouched; func_80042874: 16 mults -> 6 phantoms -> vars=48). asm/funcs/func_800480C0.s contains zero mult/multu/div/divu, so it cannot be used here without materialising instructions.

- [s5] Tree-wide bound: among the 45 census candidates with no mult/div, untouched vars takes only {8, 16, 32}, and every 32 is a sanctioned-pad function whose BB2_ALLOC_DEBUG phantom count is ZERO (their 32 bytes are the declared array). Four folded-compare residues on a mult-free body has no in-tree precedent.

- [s5] Three further spellings folding guards onto both of this body's conditional branches (P entry+backedge guards, Q mixed signed/unsigned, R get_cs-style range test) still measure unalloc_pseudos=1 / vars=8 (P, Q) or 0 / 0 (R), and all three take a ninth callee-saved register, moving every sp offset further from the target. Running total 30 structural forms.

- [s5] Reusable instruments added: tmp/grind/func_800480C0/s5/census.sh (compile all 32 TUs), census2.py (untouched-vars-window census with the frame-address filter), census3.py (adds the mult/div correlation), alloc_tu.sh (per-TU BB2_ALLOC_DEBUG capture), dump2.sh (per-TU -da dumps via -dumpbase). They are function-agnostic and apply to any frame-residual grind.

## s6 — synthesis (2026-09-02, chassis HEAD ba593529, -mel, 0 regfix/asmfix rules)

- **Chassis re-measured: floor 20.** `sandbox func_800480C0 --disable all` with
  `memory/grind/func_800480C0/candidate.c` installed prints
  `{"score": 20, "target_insns": 74, "build_insns": 74}`. Bare HEAD (INCLUDE_ASM)
  prints 74 / `no_c_body: true`. src/text1b.c restored to HEAD afterwards.

- **THE PHANTOM-SLOT MECHANISM IS NOW NAMED AND DUMP-VERIFIED, not inferred.**
  s3/s5 described the producer as "an ST_REGS-classed compare residue". The dumps
  say something more specific and far more actionable: the phantom is an orphan
  `(insn N (use (reg:M P)))` planted by **combine.c's `distribute_notes`**
  (`tools/gcc-2.7.2/combine.c:10832-10841`). When combine rewrites or deletes the
  insn that DEFINED intermediate pseudo P, P's `REG_DEAD` note has no home; the
  backward scan at `combine.c:10757-10762` walks `prev_nonnote_insn` only while the
  predecessor is an `INSN`/`CALL_INSN`, so it stops at the block's leading
  JUMP_INSN or CODE_LABEL, and with `place == 0 && tem != 0` combine emits
  `(use P)` right after that jump/label. P then has NO set and NO constraint-bearing
  reference, so `regclass` records nothing for it and the `.lreg` report prints the
  default `ST_REGS or none`; `find_reg` cannot seat it, `alter_reg` pays it a stack
  slot, and `assign_stack_local` bumps `frame_offset` by 8. Zero instructions are
  emitted for the `use` itself.
  Worked exhibit (`tmp/grind/func_800480C0/s5/dumps/display.{flow,combine}`, get_cs):
  flow has `insn 22 (set (reg:HI 83) (mem:HI D_8009BE78))`,
  `insn 23 (set (reg:SI 85) (ashift (subreg:SI (reg:HI 83)) 16))`,
  `insn 24 (set (reg:SI 84) (ashiftrt (reg:SI 85) 16))` carrying `REG_DEAD 85`;
  combine turns 22 into a DELETED note, rewrites 23 to
  `(set (reg:SI 84) (sign_extend:SI (mem:HI ...)))`, turns 24 into
  `(set (reg:HI 83) (subreg:HI (reg:SI 84) 0))`, and plants
  `insn 143 (use (reg:SI 85))` immediately after `jump_insn 18`.

- **New instrument: the orphan-USE census.** `tmp/grind/func_800480C0/s6/count_uses.py`
  counts pseudo-numbered `(insn ... (use (reg:M P)))` patterns per function in a
  `.combine` dump (the hard-reg return-value `(use (reg/i:SI 2 $2))` is excluded by
  the `P >= 64` filter). Run over six TUs (`tmp/grind/func_800480C0/s6/dumps/*.combine`
  plus s5's display dumps) and correlated against s5's `BB2_ALLOC_DEBUG hardreg=-1`
  counts:

  | function | orphan USEs | phantoms |
  |---|---|---|
  | SetDrawEnv, SetDrawEnv2 (display) | 3 | 3 |
  | get_cs, get_ce (display), func_80040594 (text1a_pre), func_80038170 (code6cac_c_mid), SpuSetCommonAttr, SsSeqCalledTbyT (main) | 2 | 2 |
  | func_800400B0, func_800400F8 (config), func_80039320, SsVabOpenHeadWithMode, func_80086014, func_80086130 | 1 | 1 |
  | func_80042874, func_80042A88 (text1a_c) | 0 | 6 |
  | func_8003FECC, SpuSetReverbModeParam, _SsSeqPlay | 1 | 2 |

  Orphan-USE count is an EXACT predictor of phantom count for every function whose
  phantoms are combine residues; the mismatches are the second (mult/div DImode-HILO)
  producer s5 already identified, either alone (0 USEs / 6 phantoms) or mixed
  (1 USE / 2 phantoms). Call these **class A** (combine orphan USE) and
  **class B** (everything else, incl. the mult/div HILO scratch).

- **s5's tree-wide bound "no mult-free function exceeds TWO phantoms" is REFUTED by
  counterexample.** `SetDrawEnv` (`src/display.c:360`) and its twin `SetDrawEnv2`
  (`src/display.c:436`) are mult-free, ordinary C, COMPLETED-C on main, and each
  carries THREE orphan USEs / THREE phantoms:
  `tmp/grind/func_800480C0/s5/asm/display.s:1243` reads
  `.frame $sp,64,$31   # vars= 32, regs= 3/0, args= 16` — 24 bytes of phantom plus
  8 bytes for the live `u16 buf[4]`. s5's bound was an artifact of its own census
  FILTER: the untouched-vars-window census only admits functions with ZERO `N($sp)`
  traffic in the vars window, which silently excludes every function that also has a
  live stack local. Three phantoms from ordinary C on a mult-free body therefore has
  an in-tree precedent; the ceiling this function is fighting is not 2. The three
  SetDrawEnv orphans are pseudos 128/137/140, all from the same
  `s16 loc; ... if (SHORT_GLOBAL - 1 < loc) loc2 = SHORT_GLOBAL - 1;` clamp idiom
  that produces get_cs's two (`src/display.c:392-400`).

- **This body's own phantom is class B, and class A has never fired here.** Re-measured
  on the current chassis with the s6 probe: `rejected/phantom-guard-vars8-ceiling.c`
  gives `vars= 8, unalloc=1, orphanUSE=0` and
  `rejected/two-branch-guards-still-one-phantom.c` gives
  `vars= 8, regs= 9/0, unalloc=1, orphanUSE=0`. Both s4/s5 instance kills still hold
  on HEAD ba593529 — and the new column shows WHY the 30-form ceiling never moved:
  every spelling tried in s2-s5 was reaching for class B, whose measured multiplicity
  on this body is 1, while the only producer that reaches 3 anywhere in this tree is
  class A.

- **Four new spellings, all class-A-negative** (`tmp/grind/func_800480C0/s6/bodies/`,
  each measured `vars= 0, unalloc=0, orphanUSE=0`):
  `B_s32params.c` (arg2..arg5 declared `s32` with explicit `(s16)` casts at the
  sign-extend site — the shape the target's `lw 0x68($sp)` + `sll/sra` suggests),
  `C_hi_intermediates.c` (four block-scope `s16` intermediates feeding the four
  `sx_argN`), `D_fnscope_hi.c` (the same four intermediates hoisted to function
  scope), `G_splitshift_loop.c` (`(((u32)word >> 1) >> 1) << 2` at the loop head, a
  split shift intended to leave a foldable intermediate at a block head; GCC folds
  constant-shift-of-constant-shift before combine ever sees two insns, so no
  intermediate insn exists to orphan).

- [s6] Chassis re-measured: sandbox --disable all with candidate.c installed prints score 20 / target 74 / build 74 on HEAD ba593529; bare HEAD prints 74 with no_c_body true.

- [s6] The phantom slot is an orphan `(insn (use (reg P)))` planted by combine.c distribute_notes (tools/gcc-2.7.2/combine.c:10832-10841) when a REG_DEAD note for an intermediate pseudo whose defining insn combine rewrote or deleted has no home and the backward scan (combine.c:10757-10762) stops at the block's leading jump/label. The pseudo then has no set and no class-bearing reference, regclass leaves its printed class at the default 'ST_REGS or none', find_reg cannot seat it, and alter_reg pays 8 bytes of vars for zero emitted instructions.

- [s6] New instrument tmp/grind/func_800480C0/s6/count_uses.py counts pseudo-numbered orphan USE insns per function in a .combine dump; across six TUs it is an exact predictor of the BB2_ALLOC_DEBUG hardreg=-1 count for every function whose phantoms are combine residues (class A), the residual mismatches being the mult/div DImode-HILO producer (class B).

- [s6] s5's tree-wide bound 'no mult-free function exceeds two phantoms' is refuted by counterexample: SetDrawEnv (src/display.c:360) and SetDrawEnv2 (src/display.c:436) are mult-free ordinary C, COMPLETED-C on main, and each carry three orphan USEs / three phantoms with .frame $sp,64 # vars= 32 (tmp/grind/func_800480C0/s5/asm/display.s:1243). s5's bound was an artifact of its census filter, which drops any function that also has live stack traffic.

- [s6] func_800480C0's own known phantom is class B, not class A: rejected/phantom-guard-vars8-ceiling.c and rejected/two-branch-guards-still-one-phantom.c both re-measure unalloc=1 with orphanUSE=0 on HEAD ba593529, so the 30-form ceiling recorded in s2-s5 is a ceiling on class B only; class A has never been triggered on this body.

- [s6] Four new spellings measured negative for class A (all vars=0 / unalloc=0 / orphanUSE=0): s32-typed arg2..arg5 with explicit (s16) casts, block-scope s16 intermediates for the four sign-extends, the same intermediates at function scope, and a split shift ((word>>1)>>1)<<2 at the loop head (GCC folds constant-shift-of-constant-shift before combine, so no intermediate insn ever exists to orphan).

- [s6] Reusable instruments: tmp/grind/func_800480C0/s6/probe.sh (git-checkout-clean install, cpp, instrumented cc1 frame/unalloc census, -da combine dump, orphan-USE count — one line per body), runall.sh, dumpall.sh, count_uses.py. probe.sh restores src/text1b.c from HEAD before and after every measurement, which the s3 probe did not.

- [s6] Chassis re-measured this session: sandbox func_800480C0 --disable all with memory/grind/func_800480C0/candidate.c installed prints score 20, target_insns 74, build_insns 74 on HEAD ba593529; bare HEAD (INCLUDE_ASM) prints 74 with no_c_body true. src/text1b.c restored to HEAD afterwards.

- [s6] The phantom slot is an orphan (insn (use (reg P))) planted by combine.c distribute_notes (tools/gcc-2.7.2/combine.c:10832-10841) when a REG_DEAD note for an intermediate pseudo whose defining insn combine rewrote or deleted has no home and the backward scan (combine.c:10757-10762) stops at the block's leading jump/label; the pseudo then has no set and no class-bearing reference, regclass leaves its printed class at the default 'ST_REGS or none', find_reg cannot seat it, and alter_reg pays 8 bytes of vars for zero emitted instructions.

- [s6] Worked dump exhibit for the mechanism (get_cs, src/display.c:556): display.flow has insn 22 (set (reg:HI 83) (mem:HI D_8009BE78)), insn 23 (set (reg:SI 85) (ashift (subreg:SI (reg:HI 83)) 16)), insn 24 (set (reg:SI 84) (ashiftrt (reg:SI 85) 16)) with REG_DEAD 85; display.combine deletes 22, rewrites 23 to (set (reg:SI 84) (sign_extend:SI (mem:HI ...))), turns 24 into (set (reg:HI 83) (subreg:HI (reg:SI 84) 0)), and plants insn 143 (use (reg:SI 85)) immediately after jump_insn 18.

- [s6] New instrument tmp/grind/func_800480C0/s6/count_uses.py counts pseudo-numbered orphan USE insns per function in a .combine dump (the hard-reg return-value use is excluded by a P >= 64 filter); across six TUs it is an exact predictor of the BB2_ALLOC_DEBUG hardreg=-1 count for every function whose phantoms are combine residues.

- [s6] Two independent phantom producer classes are now separated by measurement: class A = combine orphan USE (SetDrawEnv/SetDrawEnv2 3, get_cs/get_ce/func_80040594/func_80038170/SpuSetCommonAttr/SsSeqCalledTbyT 2, six functions 1), class B = everything else including the mult/div DImode-HILO scratch (func_80042874 and func_80042A88 at 6 phantoms with 0 orphan USEs).

- [s6] s5's tree-wide bound 'among functions containing no mult/div none exceeds two phantoms' is refuted by counterexample: SetDrawEnv (src/display.c:360) and SetDrawEnv2 (src/display.c:436) are mult-free ordinary C, COMPLETED-C on main, and each carry three orphan USEs and three phantoms with .frame $sp,64 # vars= 32, regs= 3/0, args= 16 (tmp/grind/func_800480C0/s5/asm/display.s:1243) - 24 phantom bytes plus 8 for the live u16 buf[4]. The bound was an artifact of s5's census filter, which drops any function that also owns live stack traffic.

- [s6] func_800480C0's own known phantom is class B, not class A: rejected/phantom-guard-vars8-ceiling.c re-measures vars=8 / unalloc=1 / orphanUSE=0 and rejected/two-branch-guards-still-one-phantom.c re-measures vars=8 / regs=9 / unalloc=1 / orphanUSE=0 on HEAD ba593529. The 30-form ceiling recorded across s2-s5 therefore bounds class B only; the combine-orphan producer that reaches 3 in this tree has never been triggered on this body.

- [s6] Four new spellings measured class-A-negative (all vars=0 / unalloc=0 / orphanUSE=0): s32-typed arg2..arg5 with explicit (s16) casts, block-scope s16 intermediates for the four sign-extends, the same intermediates at function scope, and a split shift ((word>>1)>>1)<<2 at the loop head. mips1 has no register sign-extend pattern so the ashift/ashiftrt intermediate is genuinely live, and GCC folds constant-shift-of-constant-shift before combine so no intermediate insn ever exists there to orphan.

- [s6] Reusable instruments added: tmp/grind/func_800480C0/s6/probe.sh (git-checkout-clean install, cpp, instrumented cc1 frame + BB2_ALLOC_DEBUG unalloc census, -da combine dump, orphan-USE count - one line per body), runall.sh, dumpall.sh, count_uses.py. probe.sh restores src/text1b.c from HEAD before and after every measurement, which the s3 probe did not, removing the header-accumulation hazard of repeated installs.

## s7 - solver (2026-09-02, chassis HEAD 7e18adc2)

- [s7] CHASSIS RE-MEASURED. With memory/grind/func_800480C0/candidate.c installed in src/text1b.c, `sandbox func_800480C0 --disable all` prints `"score": 20, "target_insns": 74, "build_insns": 74`. The ledger's floor of 20 reproduces exactly on the current chassis; nothing in the s3-s6 chain has drifted.

- [s7] THE SOLVER AXIS IS CLOSED FOR THIS FUNCTION, AND THE CLASSIFIER SAYS SO IN ITS OWN WORDS. `tools/ra_solver/inverse_compose.py classify text1b func_800480C0` REFUSES to run on an INCLUDE_ASM-represented function - it prints that the text-stream classifier would report a "FICTITIOUS PRE-RA verdict" because text1b.tgt.s is absent - and directs the caller to the object-level entry points. `python3 tools/ra_solver/goal_from_tgt.py classify text1b func_800480C0` (run under WSL; it needs mipsel-linux-gnu-objdump and fails with WinError 2 from Windows Python) returns `FIRST DIVERGENCE: PRE-RA` with `next tool: none - the residual is upstream of every model`. It lists the complete shape delta: `addiu #,#,-56` vs `addiu #,#,-88`, eight `sw` and three `lw` at offsets shifted by exactly 32, and the two incoming stack-argument loads at 44/48 vs the target's 104/108. Every register assignment and every instruction position already agrees. There is no RA seat and no scheduler order to solve for.

- [s7] THE ORPHAN PLANT SITE IS ALWAYS A JUMP_INSN OR CODE_LABEL, AND ORPHANS CHAIN AT ONE SITE. Census over the six -da .combine dumps already banked (s5/dumps + s6/dumps): 31 orphan `(use (reg))` insns across 21 functions. Every one is planted immediately after a JUMP_INSN or a CODE_LABEL. func_80040594 (src/text1a_pre.c) plants TWO at the same head - insn 565 `(use (reg:SI 198))` after `code_label 457 ("done_cases")`, then insn 564 `(use (reg:SI 187))` after insn 565. So per-site multiplicity is not 1, and func_800480C0's two usable block heads (the fallthrough after `beqz $s1`, and .L80048144) do not by themselves bound class A below the four phantoms the residual needs. Instruments: tmp/grind/func_800480C0/s7/plantsites.py, s7/plantkind.py.

- [s7] THE STACKING PRODUCER'S RTL SHAPE IS AN ARITHMETIC ADDRESS, NOT A COPY. func_80040594's two orphans both come from `(set (reg:SI T) (plus:SI (reg idx4) (reg base)))` immediately followed by `(set X (mem/s:SI (reg:SI T)))` (text1a_pre.flow insns 480/482/488 and 509/511/513). combine merges the pair into `(set X (mem (plus idx4 base)))` - visible in text1a_pre.combine as `(mem/s:SI (plus:SI (reg:SI 186) ...))` - which deletes T's def, leaves T with no reference in the block, and strands its REG_DEAD note at the block head. The C shape is an ordinary `base[i]` read whose address value dies at the deref.

- [s7] THE TARGET'S NARROW LOADS ARE `lhu` + `sll 16` + `sra 16`, NOT `lh`. asm/funcs/func_800480C0.s at .L80048144 emits `lhu $a1,0($s0)` / `lhu $a2` / `lhu $a3` / `lhu $v0` and four separate `sll 16` / `sra 16` pairs. The HImode sign-extend fold that produces class-A orphans in checkRECT, SetDrawEnv and SetDrawEnv2 (`movhi` + `ashift` + `ashiftrt` collapsed to one `extendhisi2_internal`) is therefore ABSENT from the target's own instruction stream. Any spelling that induces it changes bytes. This closes the single most exact-looking fit for the residual - four narrow loads, four phantoms needed - on evidence from the target listing rather than by analogy.

- [s7] Three further spellings measured class-A-negative (`vars= 0, regs= 8/0, unalloc=0, orphanUSE=0`): the four loop loads respelled through `s16 *` (a `*(s16 *)p` deref expands directly to `sign_extend:SI (mem:HI)`, one insn, no intermediate to delete); four single-use `u16 *` copy temps in the loop body; the same plus a `u32 *` copy temp for the word load. Plain register copies are propagated away by cse/cse2 before combine ever sees them. Banked at rejected/s7-s16ptr-direct-loads-fold-clean.c, rejected/s7-single-use-ptr-copy-temps-no-orphan.c, rejected/s7-five-ptr-copy-temps-no-orphan.c. Running total of measured structural forms on this body: 38.

- [s7] func_800480C0 owns exactly ONE arithmetic-address-then-deref site - the entry-block `p = (u32 *)(base_addr + (((*p) >> 2) << 2)); count = *(p++);` - and its address pseudo does not die at the deref because `p` is carried live into the do-while. Making a second, dying copy of that address is a semantically redundant recomputation with no observable effect, i.e. the forbidden no-semantic-purpose class, not a lever. Class A therefore has at most one honest site on this body, and the arithmetic requiring four phantoms cannot be closed by class A alone.

- [s7] Reusable instruments: tmp/grind/func_800480C0/s7/plantsites.py (per-function orphan count grouped by plant site, flags shared sites), s7/plantkind.py (resolves each plant site to its insn kind), s7/run.sh (batch wrapper around s6/probe.sh; note that s6/probe.sh must be invoked through WSL - run from Git Bash it silently reports insns=0). Both census scripts take a glob and work on any .combine dump set.

- [s7] CHASSIS RE-MEASURED: with memory/grind/func_800480C0/candidate.c installed in src/text1b.c, `sandbox func_800480C0 --disable all` prints score 20, target_insns 74, build_insns 74. The ledger floor of 20 reproduces exactly on HEAD 7e18adc2; nothing in the s3-s6 chain has drifted.

- [s7] TOOLING FACT for future solver sessions: `tools/ra_solver/inverse_compose.py classify` REFUSES to run on an INCLUDE_ASM-represented function and says so explicitly (it would otherwise print a fictitious PRE-RA verdict from the absent text1b.tgt.s). The working entry point is `python3 tools/ra_solver/goal_from_tgt.py classify text1b func_800480C0`, and it must be run under WSL - Windows Python dies with WinError 2 because engine/score.py shells out to mipsel-linux-gnu-objdump.

- [s7] SOLVER VERDICT: FIRST DIVERGENCE PRE-RA, `next tool: none - the residual is upstream of every model`. The complete listed delta is addiu -56 vs -88, eight sw and three lw shifted by exactly 32, and the two incoming stack-argument lw at 44/48 vs 104/108. Registers and instruction order already agree on all 74 instructions.

- [s7] ORPHAN PLANT-SITE CENSUS (six -da .combine dumps, 21 functions, 31 orphan USEs): every orphan `(use (reg))` is planted immediately after a JUMP_INSN or a CODE_LABEL - never after an ordinary insn - and orphans CHAIN at a single site. func_80040594 (src/text1a_pre.c) plants insn 565 `(use (reg:SI 198))` after `code_label 457 ("done_cases")` and then insn 564 `(use (reg:SI 187))` after insn 565. Per-site multiplicity is therefore not 1.

- [s7] THE STACKING PRODUCER'S RTL SHAPE IS AN ARITHMETIC ADDRESS, NOT A COPY: func_80040594's two orphans are both `(set (reg:SI T) (plus:SI (reg idx4) (reg base)))` immediately followed by `(set X (mem/s:SI (reg:SI T)))`; combine merges the pair into `(set X (mem (plus idx4 base)))` (visible in text1a_pre.combine as `(mem/s:SI (plus:SI (reg:SI 186) ...))`), deleting T's def. The C shape is an ordinary `base[i]` read whose address value dies at the deref.

- [s7] THE TARGET'S NARROW LOADS ARE `lhu` + `sll 16` + `sra 16`, NEVER `lh`: asm/funcs/func_800480C0.s at .L80048144 emits four `lhu` and four separate sll/sra pairs. The HImode sign-extend fold that produces class-A orphans in checkRECT, SetDrawEnv and SetDrawEnv2 is therefore absent from the target's own stream, so the single most exact-looking fit for the residual (four narrow loads, four phantoms needed) is closed on target-listing evidence rather than by analogy.

- [s7] Three further spellings measured class-A-negative (vars=0, unalloc=0, orphanUSE=0): four loop loads through `s16 *`; four single-use `u16 *` copy temps; the same plus a `u32 *` copy temp for the word load. Running total of measured structural forms on this body: 38.

- [s7] func_800480C0 owns exactly ONE arithmetic-address-then-deref site (the entry-block `p = base_addr + ((*p>>2)<<2)` followed by `count = *(p++)`), and its address pseudo does not die at the deref because `p` is carried live into the loop. Class A therefore has at most one honest site on this body, which is short of the four phantoms the 32-byte residual requires.

- [s7] Reusable instruments added: tmp/grind/func_800480C0/s7/plantsites.py (per-function orphan count grouped by plant site, flags shared sites), s7/plantkind.py (resolves each plant site to its insn kind), s7/run.sh (batch wrapper around s6/probe.sh). NOTE: s6/probe.sh must be invoked through WSL - run from Git Bash it silently reports insns=0.

- [s7] candidate.c now carries a MIGRATION BANNER stating that func_800480C0 is committed on main as INCLUDE_ASM and that every 'measured on main' claim in its headers means 'measured with this body installed', clearing the dispatch-time STALE HEAD CLAIMS consistency warning.

## s8 — forensics (2026-09-02, chassis HEAD 28583e8e)

- [s8] CHASSIS RE-MEASURED. `memory/grind/func_800480C0/candidate.c` installed over the
  INCLUDE_ASM line, `sandbox func_800480C0 --disable all` prints
  `"score": 20, "target_insns": 74, "build_insns": 74`. The ledger floor of 20 reproduces
  on HEAD 28583e8e. src/text1b.c restored to HEAD afterwards (`git status` clean apart
  from metrics/events.jsonl).

- [s8] MANDATED KILL RE-AUDIT — FAKE ABLATION. `python3 tools/fake_ablate.py --func
  func_800480C0 --file text1b --candidate memory/grind/func_800480C0/candidate.c` finds
  exactly ONE ablatable FAKE unit (the annotated `arg0 = 0;` dead param store) and scores
  the full grid: keep-all = **20** (74 insns), drop-1 = **32** (73 insns). The FAKE is
  strictly load-bearing and it is NOT masking a lever — removing it costs an instruction
  and 12 score. Every s2–s7 kill measured with the store present therefore stands on its
  own terms; there is no func_8002EA24-style carrier-occupies-the-pseudo hazard here,
  because the store's target pseudo ($a0/arg0) is not the residual's carrier (the residual
  is the frame size, and the two streams already agree on all 74 instructions).

- [s8] THE FRONTIER-2 "CALLER-PROVIDED 32-BYTE RECORD" READING IS NEGATIVE, MEASURED FROM
  THE CALL SITES. Every caller of the four text1b siblings passes SCALARS only:
  `src/text1a_post.c:277/279/284/286` and `src/text1a_pre.c:268/271/275/278` call
  `func_800480C0(sec, 0, 0x80, 0, -0x140, 0xF0)` / `func_80047EE8(sec, 0)` /
  `func_80047FBC(sec, 0, 0x80, 0)`; `src/text1a_c2.c:161` calls
  `func_800481E8((s32)fp_ptr, 0)`; the asm callers `func_80040594.s:125/144/152/172`,
  `func_80041988.s:47/61/68`, `func_80045B68.s:149`, `func_800460E4.s:219` show the same
  shape. The callee `func_800482C8` (src/text1b.c:209) takes a `u8 *` into a TIM/image data
  blob and owns its own locals (`s16 rect[4]`, `s16 buf[512]`) — it never reads a
  caller-supplied 32-byte stack area. So the 32 bytes are NOT an aggregate the original
  built on the stack and passed by address; if the original declared a 32-byte object, it
  was declared LOCAL and never referenced.

- [s8] THE FOUR SIBLINGS' TARGET FRAMES ARE IDENTICAL IN THE UNTOUCHED WINDOW — the
  strongest declaration-signature evidence banked so far. `tmp/grind/func_800480C0/s8/
  framecensus.py` over the shipped listings gives:

  | target | frame | callee-saved base | sp-slots used below it | untouched |
  |---|---|---|---|---|
  | func_80047EE8 | 0x48 | 0x38 | 0x10 only | 0x14–0x37 |
  | func_80047FBC | 0x50 | 0x38 | 0x10 only | 0x14–0x37 |
  | func_800480C0 | 0x58 | 0x38 | 0x10 only | 0x14–0x37 |
  | func_800481E8 | 0x48 | 0x38 | 0x10 only | 0x14–0x37 |

  All four reserve args = 0x18 (only the 5th-argument slot 0x10 is ever written) and the
  SAME 32-byte untouched vars region 0x18–0x37; the frame totals differ only by how many
  callee-saved registers each body needs (0x48/0x50/0x58/0x48). The four bodies differ
  structurally — 2, 4, 6 and 2 parameters, and func_800481E8 carries an extra
  `if (a3v < 0x280)` clamp — yet the reserved vars is bit-for-bit the same size at the same
  offset. Phantom residue scales with EXPRESSION SHAPE (the tree census below shows counts
  of 0/1/2/3 tracking each body's fold sites), so a shape-invariant 32 bytes across four
  structurally different siblings is the signature of one shared source DECLARATION, not of
  allocation residue. Three of the four already carry owner-granted `volatile u32
  pre_pad[8];` rows (engine/volatile_cheats.py:757-767, owner rulings 2026-08-20 / 08-22).

- [s8] FILTER-FREE TREE-WIDE PHANTOM CENSUS — FOUR PHANTOMS IS ATTESTED NOWHERE ON A
  MULT-FREE BODY. s5's bound came from a census that FILTERED OUT any function with live
  stack traffic (refuted in s6); this one filters nothing. All 32 `src/*.c` TUs recompiled
  with the instrumented cc1 (`tmp/grind/func_800480C0/s8/allcensus.sh`), 1096 functions
  with a `.ent`/`.end` pair, phantom counted as `hardreg=-1 AND livelen<=2`
  (`tmp/grind/func_800480C0/s8/phantom_census_v2.py`):
    * ALL functions: {0: 1043, 1: 39, 2: 7, 3: 5, 6: 2}
    * mult-free:     {0: 980, 1: 30, 2: 7, 3: 3}
    * the only bodies above 3 are func_80042874 / func_80042A88 (src/text1a_c.c), both
      mult/div (class-B DImode HILO), at 6 phantoms / vars=48.
    * the mult-free 3s are SetDrawEnv, SetDrawEnv2 (src/display.c) and func_80041AC8
      (src/text1a_post.c), all vars=32 — 24 phantom bytes plus 8 bytes of live local.
  func_800480C0's target stream contains no mult/div, so the class-B producer that reaches
  6 cannot be spelled here byte-neutrally, and the mult-free ceiling measured across the
  whole tree is 3 — one short of the four the 32-byte region needs.

- [s8] CENSUS METHOD CORRECTION (worth carrying forward): `hardreg=-1` alone does NOT mean
  phantom. v1 of the census reported func_80060E38 (src/text1b.c:3317) as a NINE-phantom
  mult-free body; it is a nine-SPILL body — 32 scratchpad-address constants live at once,
  `nrefs=2 livelen=57..65 pri≈300`, and its frame bytes are genuinely written. Phantoms
  carry the get_cs/SetDrawEnv signature `nrefs=2 livelen=2 pri=10000`. Both scripts are
  banked (`phantom_census_v1_spill_conflated.py`, `phantom_census_v2.py`) so the next
  session can see the discriminator rather than re-derive it.

- [s8] DIVIDE/MULTIPLY SPELLING OF THE POWER-OF-TWO SCALES PLANTS NO PHANTOM AND IS
  BYTE-NEUTRAL. New attack (not in the 38 forms s2–s7 measured): the two `((x) >> 2) << 2`
  sites and the `(arg1 << 16) >> 14` scale respelled as ordinary unsigned `/ 4 * 4` and
  `* 4`, on the theory that expand_divmod's constant-power-of-two path might leave a
  foldable intermediate for combine to delete. Measured with
  `tmp/grind/func_800480C0/s8/run.sh` (s6 probe: instrumented cc1 frame + BB2_ALLOC_DEBUG +
  orphan-USE count on a -da .combine dump):
    * `A_divmul_both.c`   → `.frame $sp,56 # vars= 0, regs= 8/0, args= 24`, unalloc=0, orphanUSE=0
    * `B_divmul_arg1mul.c`→ same
    * `C_divmul_loop.c`   → same
  and `sandbox func_800480C0 --disable all` on A prints score 20 / 74 / 74 — i.e. the
  division spelling is codegen-IDENTICAL to the shift spelling (GCC 2.7.2 folds the
  constant power-of-two divide at expand time, before combine ever runs) and contributes
  nothing. Banked at rejected/s8-divmul-spelling-no-phantom.c and
  rejected/s8-divmul-arg1-scale-no-phantom.c. Running total of measured structural forms on
  this body: 41.

- [s8] THE CLASS-A CONSTANT-OFFSET RESPELLING IS CLOSED ON TARGET-LISTING EVIDENCE. The
  class-A producer (s7) is `T = base + CONST; X = *T` with T dying, which combine folds
  into the mem's displacement — so the honest C shape that would plant four orphans here is
  reading the four halfwords at constant offsets (`*(u16*)((s32)p + 4)`, `+6`, `+8`, `+10`)
  and advancing `p` once. The target's own loop at `.L80048144` (asm/funcs/func_800480C0.s)
  emits `lhu $aN, 0x0($s0)` FOUR TIMES with a separate `addiu $s0,$s0,0x2` between each —
  every load at displacement zero off a pointer that is live across the whole loop. A
  constant-offset respelling therefore changes the emitted displacements and the addiu
  count, i.e. it cannot be byte-neutral. Combined with s7's finding that the body owns
  exactly one arithmetic-address-then-deref site, class A is capped at ≤1 here by the
  target listing itself, not by analogy.

- [s8] Chassis re-measured: with memory/grind/func_800480C0/candidate.c installed, sandbox func_800480C0 --disable all prints score 20, target_insns 74, build_insns 74 on HEAD 28583e8e; src/text1b.c restored to HEAD afterwards.

- [s8] FAKE ablation grid (tools/fake_ablate.py): the candidate has exactly ONE ablatable FAKE unit, the annotated `arg0 = 0;` dead param store; keep-all scores 20 at 74 insns, drop-1 scores 32 at 73 insns. Load-bearing, and not masking any lever.

- [s8] All four text1b siblings' SHIPPED frames reserve the identical untouched window 0x18-0x37 (args=0x18, only the 5th-arg slot 0x10 written; callee-saved base 0x38), with frame totals 0x48/0x50/0x58/0x48 differing only by register count - despite 2/4/6/2 parameters and structurally different bodies (func_800481E8 carries an extra clamp). Instrument: tmp/grind/func_800480C0/s8/framecensus.py.

- [s8] Filter-free tree-wide phantom census (all 32 TUs, 1096 functions, instrumented cc1): mult-free phantom distribution {0:980, 1:30, 2:7, 3:3}; all-functions {0:1043, 1:39, 2:7, 3:5, 6:2}. The only bodies above 3 phantoms are func_80042874 and func_80042A88 (src/text1a_c.c), both mult/div at 6 phantoms / vars=48. Four phantoms on a mult-free body is attested nowhere in the tree.

- [s8] Census-method correction for future sessions: hardreg=-1 alone does NOT mean phantom. func_80060E38 (src/text1b.c:3317) has nine hardreg=-1 pseudos, but they are genuine register-pressure SPILLS (nrefs=2, livelen=57-65, pri~300) whose frame bytes are written. Phantoms carry the get_cs/SetDrawEnv signature nrefs=2 livelen=2 pri=10000; the discriminator is livelen<=2. Both scripts banked (phantom_census_v1_spill_conflated.py vs phantom_census_v2.py).

- [s8] The three mult-free 3-phantom bodies are SetDrawEnv, SetDrawEnv2 (src/display.c) and func_80041AC8 (src/text1a_post.c); the first two are the clamp idiom already characterised in s6, the third has never been read and is the only unexamined phantom producer in the tree.

- [s8] Callers of the four siblings pass scalars only (src/text1a_post.c:277-286, src/text1a_pre.c:268-278, src/text1a_c2.c:161; asm callers func_80040594/func_80041988/func_80045B68/func_800460E4), and callee func_800482C8 (src/text1b.c:209) reads a u8* image blob and owns its own s16 rect[4] / s16 buf[512]. No 32-byte record is passed by address anywhere in the cluster.

- [s8] Unsigned /4 *4 spelling of both `>>2 <<2` sites (and *4 for the arg1 scale) is codegen-identical to the shifts - sandbox 20 / 74 / 74, vars=0, unalloc=0, orphanUSE=0 - because GCC 2.7.2 folds constant power-of-two division at expand, before combine.

- [s8] The target loop at .L80048144 loads all four halfwords at displacement 0x0 off $s0 with a separate addiu $s0,$s0,0x2 after each, so the class-A constant-offset shape (dying base+CONST address folded into a mem displacement) does not appear in the shipped stream and cannot be introduced byte-neutrally.

- [s8] Three of the four siblings (func_80047EE8, func_80047FBC, func_800481E8) are COMPLETED-C on main carrying `volatile u32 pre_pad[8];` with owner-granted rows at engine/volatile_cheats.py:757-767 (rulings 2026-08-20 and 2026-08-22); func_800480C0 is the fourth member of the same shipped frame family and the only one without a row (Judge FAIL 2026-09-02 04:28, docs/grind/decisions.md:20349).

## s9 — forensics (2026-09-02, chassis HEAD 0c7f30e4)

- [s9] CHASSIS RE-MEASURED. `memory/grind/func_800480C0/candidate.c` installed over the
  INCLUDE_ASM line, `sandbox func_800480C0 --disable all` prints `"score": 20,
  "target_insns": 74, "build_insns": 74` on HEAD 0c7f30e4. src/text1b.c restored to HEAD
  afterwards (`git status` clean apart from metrics/events.jsonl). The floor of 20 has now
  reproduced on four consecutive chassis (7e18adc2, 28583e8e, 0c7f30e4 and s3's).

- [s9] FRONTIER ITEM 1 IS CLOSED NEGATIVE: THE FAMILY HAS EXACTLY FOUR MEMBERS. s8's
  frontier proposed surveying the binary for a fifth/sixth batch-loader sibling that might
  WRITE the 0x18-0x37 window and so name the object honestly. `grep -l 'func_800482C8'
  asm/funcs/*.s src/*.c` returns exactly five files: the four known siblings
  (func_80047EE8, func_80047FBC, func_800480C0, func_800481E8), plus func_800482C8's own
  listing and src/text1b.c. There is no fifth caller anywhere in the shipped binary, so
  there is no live sibling to name the 32-byte object. The "is the object LIVE in some
  variant of the routine" question has no further body to ask it of.

- [s9] FRONTIER ITEM 2 IS CLOSED NEGATIVE: func_80041AC8 IS NOT A THIRD PRODUCER CLASS —
  IT IS THE SAME COMBINE ASHIFT-DELETION AS SetDrawEnv. s8 flagged func_80041AC8
  (src/text1a_post.c:298) as the one mult-free 3-phantom body never read. Dumped with the
  instrumented cc1 (`tmp/grind/func_800480C0/s9/dumps/text1a_post.*`, dump.sh) and
  cross-read against display.c (`s9/dumps/display.*`, dump2.sh):

  | body | orphan pseudos (BB2_ALLOC_DEBUG hardreg=-1) | .cse RTL that defines them |
  |---|---|---|
  | func_80041AC8 | 115, 105, 85 | insns 171/106/26: `(set (reg:SI P) (ashift:SI (subreg:SI (reg:HI Q) 0) (const_int 16)))` |
  | SetDrawEnv | 140, 137, 128 | insns 217/209/166: byte-for-byte the same pattern |

  In every one of the six cases the NEXT insn is `(set (reg:SI Q') (ashiftrt:SI (reg:SI P)
  (const_int 16)))` carrying `REG_EQUAL (sign_extend:SI (reg:HI Q))`, and in the .combine
  dump P survives only as the orphan `(insn (use (reg:SI P)))` that s6 attributed to
  distribute_notes (combine.c:10832-10841). So this tree has ONE class-A producer, not two
  or three, and its RTL signature is fixed: **combine deletes the `ashift` half of a
  shift-pair sign-extension of an HImode PSEUDO**.

- [s9] THE PRODUCER'S PRECONDITION, NAMED AND BYTE-VISIBLE. Combine can only delete that
  `ashift` because it substitutes a MEMORY equivalent for the HImode pseudo and re-forms
  the sign extension as a signed narrow load. The emitted proof is in both bodies:
  func_80041AC8 ships `lh $2,0($16)` next to `lhu $3,0($16)` (source reads `*var_s0` twice,
  once cast to u16 and once compared signed); SetDrawEnv ships `lh $5,22($sp)` and
  `lh $2,D_8009BE78` against values it had just stored into its stack local. So the
  producer needs (a) an HImode value with a MEMORY home (stack local or global), and (b)
  that home reachable at the shift site — i.e. `can_combine_p`'s `use_crosses_set_p (src,
  INSN_CUID (insn))` guard at tools/gcc-2.7.2/combine.c:917 must not fire, which means no
  intervening set of the address registers.

  func_800480C0's shipped 74-insn stream (asm/funcs/func_800480C0.s) satisfies NEITHER.
  It contains no `lh` at all; its four halfword values come from `lhu $aN,0x0($s0)` with an
  `addiu $s0,$s0,0x2` on the base register BETWEEN each load and its `sll/sra` pair (which
  is exactly why those pairs survive into the shipped bytes instead of folding to `lh`),
  and its four s16 parameters arrive in $a2/$a3/$v1/$a0 with no memory home at all. Any
  spelling that gives a halfword a memory home or removes the intervening `addiu` changes
  the emitted stream — the constant-offset variant was already killed in s8 on displacement
  and addiu-count grounds.

- [s9] FOUR NEW SPELLINGS MEASURED, ALL CLASS-A-NEGATIVE (running total of measured
  structural forms on this body: 45). Instrument: s6/probe.sh (instrumented-cc1 frame +
  BB2_ALLOC_DEBUG unalloc count + s6/count_uses.py orphan-USE count on a -da .combine),
  runner tmp/grind/func_800480C0/s9/run.sh, bodies at s9/bodies/:
    * CTRL_candidate           -> `vars= 0, regs= 8/0, args= 24`, unalloc=0, orphanUSE=0
    * A_s32_immediate_sx       -> identical. The four halfword reads spelled as IMMEDIATE
      s32 sign-extends (`s32 a1v = (s16)*(u16*)p;` instead of an s16 local) do NOT fire the
      fold: the `addiu` still crosses the mem, combine.c:917 still refuses.
    * B_arg1_chain_2x2 / C_arg1_chain_2x2x1 / D_arg1_mul4_after_sx -> identical. Extending
      the arg1 scale into a longer combine-foldable chain (`((s16)arg1*2)*2`, `*2*2*1`,
      `(s16)arg1*4`) plants NO orphan: combine's shift-merge rewrites i3 in place (that is
      how the shipped `sll $a1,16; sra $a1,14` arises) and the intermediate's REG_DEAD note
      finds a home, so distribute_notes never plants a USE. Chain length is not the lever.
  Banked: rejected/s9-s32-immediate-sign-extend-no-orphan.c,
  rejected/s9-arg1-foldable-chain-extension-no-orphan.c.

- [s9] Chassis re-measured: with memory/grind/func_800480C0/candidate.c installed, `sandbox func_800480C0 --disable all` prints score 20, target_insns 74, build_insns 74 on HEAD 0c7f30e4. Floor 20 has now reproduced on four consecutive chassis. src/text1b.c restored to HEAD afterwards.

- [s9] The func_800482C8 caller family has exactly four members. grep over asm/funcs/*.s and src/*.c returns only func_80047EE8, func_80047FBC, func_800480C0, func_800481E8 plus the callee's own listing and src/text1b.c - so s8's frontier-1 probe (find a fifth sibling that WRITES the untouched window) has no candidate to run on.

- [s9] This tree has ONE class-A phantom producer, not several. func_80041AC8's three orphans (pseudos 115/105/85, .cse insns 171/106/26) and SetDrawEnv's three (140/137/128, .cse insns 217/209/166) share the identical RTL signature: `(set (reg:SI P) (ashift:SI (subreg:SI (reg:HI Q) 0) (const_int 16)))` followed by an `ashiftrt ... 16` carrying REG_EQUAL (sign_extend:SI (reg:HI Q)), with P surviving in .combine only as the orphan `(insn (use (reg:SI P)))`.

- [s9] The producer's precondition is an HImode value with a MEMORY home: combine deletes the ashift by substituting the mem back and re-forming the extension as a signed narrow load, so each orphan is paid for with an emitted `lh` (func_80041AC8: `lh $2,0($16)` beside `lhu $3,0($16)`; SetDrawEnv: `lh $5,22($sp)`, `lh $2,D_8009BE78`). The substitution is gated by can_combine_p's use_crosses_set_p at tools/gcc-2.7.2/combine.c:917.

- [s9] func_800480C0's shipped 74-insn stream contains zero `lh`, and every one of its four `lhu $aN,0x0($s0)` loads has `addiu $s0,$s0,0x2` on the base register between the load and its `sll/sra` pair. That intervening set is exactly what makes use_crosses_set_p refuse - and is why the sll/sra pairs survive into the target bytes instead of folding to `lh`. The target listing is therefore direct evidence that the class-A fold did not fire in the original either.

- [s9] Four new spellings measured negative on the s6 probe harness (running total 45 structural forms on this body): immediate s32 sign-extends of the four u16 loads, and three combine-foldable chain extensions of the arg1 scale. All print vars=0, unalloc=0, orphanUSE=0, identical to the control.

- [s9] Combine's shift-MERGE (which produced the shipped `sll $a1,16; sra $a1,14` from a sign extension plus a `*4`) rewrites i3 in place and orphans nothing; only the MEM-substitution shape reaches distribute_notes' no-home path. The two combine behaviours must not be conflated when proposing chain-length levers.

## s10 (rederive, 2026-09-02, chassis HEAD a0198d09)

Floor re-measured: `sandbox func_800480C0 --disable all` -> `"score": 20`,
target_insns 74, build_insns 74, with `memory/grind/func_800480C0/candidate.c`
installed over the `INCLUDE_ASM("asm/funcs", func_800480C0);` line in
src/text1b.c. Unchanged from s3-s9; the chassis moved 0c7f30e4 -> a0198d09
(ledger commits only) and the number did not.

New instrument: `tmp/grind/func_800480C0/s10/probe.sh`. It is s6/probe.sh with
two defects fixed that mattered for this session: (a) the s6 script's insn
counter used `grep -E '^[ \t]+[a-z]'`, where `\t` inside a bracket expression is
a literal backslash-t, so it always reported insns=0; (b) when run from Git Bash
on the Windows host rather than through WSL, the `cd` to the /mnt path fails, cc1
never runs, and awk silently reports the frame line of the PREVIOUS run's
`probe.s`. Every s10 measurement was taken through `bash tools/wsl.sh`, and
probe.sh now deletes probe.s/fn.s before each run and prints CC1-FAIL if cc1
produced nothing.

### 1. Fresh m2c decompile (rederive axis 1)

`python3 tools/m2c/m2c.py --target mipsel-gcc-c -f func_800480C0 --valid-syntax
asm/funcs/func_800480C0.s` reproduces the control flow this ledger has carried
since s1: the two-step base computation, `if (count != 0)` guard, do-while loop
with a moving cursor and five-argument call. The only deltas are cosmetic - m2c
types arg4/arg5 as s32 with `(s16)` casts at the use sites, and it renders the
loop tail as `var_s1 -= 1; while (var_s1 != 0)` rather than the target's
test-then-decrement `while ((count--) != 0)`. m2c invents no local and no
aggregate, so it offers no producer for the 32 untouched `vars` bytes. Spelled
out as a compilable chassis (b1) it measures
`.frame $sp,56 # vars= 0, regs= 8/0, args= 24`, insns=71, unalloc=0 - one insn
SHORT of the current body, because using `arg0` directly at both base sites
removes the `move $18,$16` the target ships at 0x800480D0.

### 2. Sibling transplant (rederive axis 2) - the decisive result

func_80047FBC (src/text1b.c:82) is the same routine with four parameters instead
of six and it is COMPLETED-C on main today. Read side by side with candidate.c
the two bodies are line-for-line the same program: `base_addr = arg0; p =
(u32 *)arg0;`, the annotated `arg0 = 0;` dead param store with the same cse2
justification, `p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));`, the
`(((*p) >> 2) << 2)` re-base, `count = *(p++);`, the `if (count != 0)` guard with
hoisted `sx_argN` widenings, and the in-loop word + four halfword cursor reads
feeding `func_800482C8`. func_80047EE8 (src/text1b.c:35) is the same body again
with the widening inlined at the call.

The ONE structural difference between the accepted siblings and this candidate is
the leading `volatile u32 pre_pad[8];`. All three siblings hold owner grants for
it - `"func_80047EE8"` and `"func_80047FBC"` from the 2026-08-20 ruling and
`"func_800481E8"` from the 2026-08-22 "parked-but-proven audit" ruling, at
engine/volatile_cheats.py:757-767. func_800480C0 is the fourth member of the same
family in the same translation unit with the same 0x18-0x37 window, and it is not
in the enumeration; the 2026-09-02 04:28 Judge FAIL
(docs/grind/decisions.md:20349) turned on exactly that - "the unwritten-leading-pad
family is a CLOSED per-function enumeration, and func_800480C0 is not in it".

The consequence for THIS modality is concrete rather than rhetorical: the rederive
mandate is to find a structurally different C shape, and the transplant axis shows
the shape that reaches the target is already on main three times over, differing
from what is banked here by one declaration that this function may not carry.

### 3. Structural respellings (rederive axis 3) - ten new forms

All measured with s10/probe.sh; all banked under
memory/grind/func_800480C0/rejected/ with an s10 header.

| form | .frame | insns | unalloc |
|---|---|---|---|
| candidate.c (baseline) | vars=0 regs=8 args=24 | 72 | 0 |
| b1 fresh-m2c chassis | vars=0 regs=8 args=24 | 71 | 0 |
| b2 u8* byte-cursor chassis | vars=0 regs=8 args=24 | 72 | 0 |
| b3 `long long` loop counter | vars=0 regs=10/0 args=24 | 86 | 0 |
| b4 `unsigned long long` scale intermediate | vars=0 regs=8 args=24 | 76 | 0 |
| b5 12-byte struct-record chassis | vars=0 regs=9/0 args=24 | 63 | 0 |
| b6 for-index chassis | vars=0 regs=9/0 args=24 | 62 | 0 |
| b7 sub-word read of the 2 stack-passed s16 params | vars=0 regs=8 args=24 | 68 | 0 |
| b8 sub-word read of all 4 s16 params | vars=0 regs=8 args=24 | 68 | 0 |
| b9 s16 round-trip temporaries | vars=0 regs=8 args=24 | 72 | 0 |
| candidate.c minus the FAKE | vars=0 regs=8 args=24 | 72 | 0 |

Running total of measured spellings on this body: 55.

### 4. s9 frontier item 1 (DImode phantom route) - CLOSED

There are exactly two places a `long long` can enter this body without inventing
a value: the loop counter and the two `(word >> 2) << 2` scale expressions.

- b3 makes `count` a `long long`. cc1 emits genuine DImode compare/decrement code
  (insns 72 -> 86) and grows the callee-saved set to `regs= 10/0`, i.e. the DImode
  pseudo is ALLOCATED and pays registers. unalloc stays 0, vars stays 0.
- b4 routes both scale sites through `unsigned long long acc` whose high word is
  provably zero and never read. cc1 lowers the whole thing to SImode, adding four
  insns (72 -> 76) with vars=0 and unalloc=0. The dead high word does not survive
  to allocation as an unallocated DImode pseudo.

So the s8 observation that the only six-phantom bodies in the tree
(func_80042874/func_80042A88) are mult/div bodies whose phantoms are DImode HILO
residue does NOT generalise into a route here: on this body a DImode value either
gets a register pair or gets lowered away, and neither outcome reserves `vars`.

### 5. s9 frontier item 3 (stack-passed s16 args as a class-A site) - CLOSED

The prediction was that arg4/arg5, which arrive in memory at 0x68/0x6C($sp) with
no intervening write to their slots, would let `use_crosses_set_p`
(tools/gcc-2.7.2/combine.c:917) pass where it blocks on the four `lhu` sites, so
combine could substitute the incoming stack MEM and delete an intermediate.

It does exactly that, and it is a byte LOSS. b7 declares arg4/arg5 as s32 and
reads them `*(s16 *)&argN` (the narrow-stack-param-subword-offset family,
ordinary C, no annotation needed). insns drop 72 -> 68: both
`lw $v1,0x68($sp); sll $v0,$v1,16; sra $s4,$v0,16` triples collapse into a single
signed halfword load. unalloc stays 0 - the substitution rewrites i3 in place, so
no REG_DEAD note is left homeless and distribute_notes plants no orphan USE. b8
extends the same reading to all four s16 params with the identical result. b9
keeps the emitted triple intact by round-tripping through s16 temporaries and
plants no orphan either.

The class-A producer is therefore not merely blocked on this body (s9's finding
for the four `lhu` sites); where it CAN fire it deletes target instructions
instead of reserving frame. Both directions are now measured.

### 6. Mandated FAKE re-audit

`tools/fake_ablate.py --func func_800480C0 --file text1b --candidate
memory/grind/func_800480C0/candidate.c` returned `ERR` for both keep-all and
drop-1 variants on this chassis (a tool-side failure, not a scoring result), so
the ablation was done by hand: candidate.c with the `arg0 = 0;` line physically
deleted, run through s10/probe.sh
(rejected/s10-candidate-minus-fake-reseats-registers.c).

Result: insns stays 72 and vars/unalloc stay 0, but the seating changes -
`sw $22,48($sp); move $22,$16` replaces `sw $18,32($sp); move $18,$16`, the second
base use binds `$4` instead of `$18` (`addu $16,$4,$2` vs `addu $16,$18,$2`), and
the prologue store order shifts. The FAKE is load-bearing for the register match
and masks no phantom lever: with it gone the frame is still empty. s8's verdict
holds on the current chassis.

- [s10] Floor re-measured on the current chassis: `sandbox func_800480C0 --disable all` prints score 20, target_insns 74, build_insns 74, with candidate.c installed over the INCLUDE_ASM line. Unchanged s3->s10 across chassis 7e18adc2 -> 28583e8e -> 0c7f30e4 -> a0198d09.

- [s10] Running total of measured spellings on this body is now 55 (45 inherited + 10 this session). Every one of the 55 reports vars= 0; the target needs vars= 32.

- [s10] Fresh m2c (tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax) reproduces the banked control flow exactly; its only deltas are s32-typed arg4/arg5 with casts at the use sites and a post-decrement loop tail. It invents no local and no aggregate, so it offers no producer for the 32 untouched vars bytes.

- [s10] The class-A combine substitution DOES fire on the two stack-passed s16 params (insns 72 -> 68, both lw+sll+sra triples folding to lh) and still yields unalloc=0 - so class A is a byte cost on this body wherever it is reachable, not a phantom source.

- [s10] A DImode value in this body is either allocated to a register pair (b3: regs= 10/0, insns 86) or lowered to SImode (b4: insns 76). Neither outcome reserves vars, so s8's DImode-HILO observation on func_80042874/func_80042A88 does not generalise into a route here.

- [s10] Abandoning the moving cursor is byte-fatal in the wrong direction: the struct-record chassis emits 63 insns and the for-index chassis 62, against the target's 74, because base+offset addressing collapses the four addiu increments the target ships.

- [s10] TOOLING DEFECT FIXED, and it matters for reading s6-s9 numbers: tmp/grind/func_800480C0/s6/probe.sh counts insns with grep -E '^[ \t]+[a-z]', where \t inside a bracket expression is a literal backslash-t, so it always reported insns=0; and when that script is run from Git Bash on the Windows host instead of through WSL, its cd to the /mnt path fails, cc1 never runs, and awk reports the frame line of the PREVIOUS run's probe.s. tmp/grind/func_800480C0/s10/probe.sh fixes both (deletes probe.s/fn.s first, prints CC1-FAIL if cc1 produced nothing) and every s10 measurement was taken through bash tools/wsl.sh.

- [s10] src/text1b.c is byte-clean at HEAD at session end; every probe restored it via git checkout before and after each compile.

## s11 (escalation, 2026-09-02, chassis HEAD 37f9ecdb) — DISPOSITION SESSION

E-s11-1. **Floor re-measured on the current chassis: 20, unchanged.**
`sandbox func_800480C0 --disable all` with `memory/grind/func_800480C0/candidate.c`
installed over the `INCLUDE_ASM("asm/funcs", func_800480C0);` line reads
`{"score": 20, "target_insns": 74, "build_insns": 74, "scorable": true,
"rules_dropped": 0, "cheat_asm_stripped": 166}`. (The 166 stripped bytes are
pre-existing file-scope cheat-asm elsewhere in text1b.c, not this function.)
src/text1b.c was restored to its HEAD INCLUDE_ASM state immediately after the
measurement; the working tree ends this session clean apart from ledger files
and the engine-written metrics/events.jsonl.

E-s11-2. **Mandated FAKE re-audit re-run on this chassis.**
`tools/fake_ablate.py --func func_800480C0 --file text1b --candidate
memory/grind/func_800480C0/candidate.c` (`tmp/grind/func_800480C0/s11/fake_ablate.txt`)
finds exactly ONE FAKE unit in the candidate — the annotated `arg0 = 0;` dead
param store at candidate.c L206. keep-all = score 20 / 74 build insns;
drop-1 = score 32 / 73 build insns. So the FAKE is load-bearing for the
instruction stream (it buys the 74th insn and the `$a0`/`base_addr` value split)
and it does NOT sit on a pseudo that a phantom-slot lever would otherwise
occupy. This re-confirms s8's and s10's ablation verdicts under the current
chassis and closes the KILL RE-AUDIT the driver mandated: no banked instance
kill was measured with a FAKE carrier masking its target, because there is only
one FAKE and its ablation moves the floor the wrong way (20 -> 32).
Note the `volatile u32 pre_pad[8];` declaration is NOT counted as a FAKE unit by
the ablator — the volatile-cheat stripper removes it before scoring, which is
exactly why the honest floor reads 20 and not 0.

E-s11-3. **Endgame gate (a), canonical-asm: FAILS.**
`python3 tools/scan_hand_coded.py --single func_800480C0`
(`tmp/grind/func_800480C0/s11/scan_hand_coded.txt`) gives
`HAND_CODED: tier=LOW score=1/8 (74 insns), Reason: no strong hand-coded
indicators`. Signal detail: S1 multu pacing — 0 multu/mflo pairs; S2 empty
branch — none; S3 no spills — 74 insns / 9 spills / 13 distinct regs (i.e. it
DOES spill, which is compiler behaviour); S4 front loads — the only hit
(4 loads in an 8-insn window @ insn 32); S5 cluster — no high-similarity sibling
above jaccard 0.5; S6 BIOS jumptable — no; S7 unsaved $sN — all callee-saves are
saved; S8 redundant mask — none. None of the STRONG signals (S1/S2/S6) fire, so
there is no canonical-asm grant path. This is consistent with everything else in
the ledger: m2c rederives the control flow (s10 axis 1) and a COMPLETED-C
sibling with the same body exists on main.

E-s11-4. **Endgame gate (b), SOTN-master precedent: the CONSTRUCT is exhibited,
the FAMILY is still closed.** Grepping `docs/reference/sotn-construct-index.md`
(machine-generated at sotn-decomp master `aa535002`, 1911 files) for leading
volatile pad declarations returns three PSX (untagged, GCC 2.7.2) hits:
`:103` -> `src/st/sel/stream.c:80` `volatile u32 pad[4]; // FAKE`;
`:84`/`:620` -> `src/st/e_background_bushes_trees.h:160`
`volatile char pad[8]; //! FAKE`; `:101` -> `src/st/sel/2C048.c:564`
`volatile u32 pad; // !FAKE:` (scalar). Honest limit of this evidence: the index
records the DECLARATION site only, so it establishes that SOTN master ships a
volatile leading pad as a match hack but does NOT resolve whether those specific
pads are subsequently written — i.e. it does not, on its own, evidence the
*unwritten* sub-case that this function needs. And per the index header, a hit
"is not, by itself, a BB2 authorization."

E-s11-5. **Why gate (b) passing on the construct still does not unblock this
function.** BB2's carve-out is not a predicate over shapes; it is a CLOSED
per-function enumeration. `.claude/rules/no-new-park-categories.md:340-342`:
"applies to func_8001E404, func_8001E6E4, func_8003CF84 ONLY; any further use
requires a fresh owner ruling". `engine/volatile_cheats.py:744`: "Any extension
requires a fresh owner ruling". All five later rows in
`_SANCTIONED_UNWRITTEN_PADS` (`engine/volatile_cheats.py:746-769` — func_80047EE8,
func_80047FBC, func_800481E8, func_80049A2C, func_80041688) carry an inline dated
OWNER RULING citation (2026-08-20 / 2026-08-22); none was added by a Judge, a
driver scope grant, or a session. A sixth row is therefore a family extension,
which under the 2026-08-31 ordinary-c-judge-decidable ruling is FAIL(CONSTRUCT),
not an escalation — exactly what the 2026-09-02 04:28 Judge ruled at
decisions.md:20349.

E-s11-6. **Disposition filed.** `docs/grind/decisions.md:20353` —
`## 2026-09-02 — func_800480C0 (src/text1b.c) — RESOLVED BY STANDING RULING
(2026-07-27): FORECLOSED`. It carries both gates' evidence, the ten-session /
seven-modality exhaustion table, the ~50k-iteration permuter result, the 29+2
kills and 31 rejected forms, the s10 sibling-transplant identification of the
residual as a source DECLARATION rather than an unfound spelling, and three
re-activation triggers (owner enumeration row; a general class grant; a
toolchain finding that plants four distribute_notes orphans at one block head
from ordinary live C on a body with narrow lhu+sll+sra parameter loads). Because
gate (b) produced an in-hand precedent citation, the driver borderline-logs the
record for owner batch review; the disposition is still the silent foreclosure.

- [s11] Floor re-measured this session on chassis HEAD 37f9ecdb: sandbox func_800480C0 --disable all with candidate.c installed = score 20, target_insns 74, build_insns 74, rules_dropped 0 - unchanged since s3, so the driver's exhaustion call rests on a live number, not a stale ledger entry.

- [s11] The 20 residual insns are all sp-offset deltas: build frame 0x38 vs target frame 0x58. The 74-instruction stream is otherwise byte-identical. The gap is a 32-byte allocated-but-never-touched vars window at sp+0x18..0x37, with ZERO sw/lw in that range anywhere in asm/funcs/func_800480C0.s.

- [s11] Mandated FAKE re-audit (tools/fake_ablate.py, tmp/grind/func_800480C0/s11/fake_ablate.txt): exactly one FAKE unit in the candidate (arg0 = 0;). keep-all = 20 / 74 insns; drop-1 = 32 / 73 insns. The FAKE is load-bearing for the stream and masks no phantom-slot lever, re-confirming s8's and s10's ablation verdicts on the current chassis.

- [s11] Endgame gate (a) FAILS: scan_hand_coded --single func_800480C0 = tier LOW, score 1/8, only S4 (front loads) firing; none of the STRONG signals S1/S2/S6 fire. No canonical-asm grant path.

- [s11] Endgame gate (b) PASSES on the construct: SOTN PSX master ships volatile leading pads as match hacks - docs/reference/sotn-construct-index.md:103 (src/st/sel/stream.c:80, volatile u32 pad[4]; // FAKE), :84 (src/st/e_background_bushes_trees.h:160, volatile char pad[8]; //! FAKE), :101 (src/st/sel/2C048.c:564, scalar form). The index records declaration sites only, so the unwritten sub-case is not itself established by it.

- [s11] Gate (b) does not unblock the function because BB2's carve-out is a CLOSED per-function enumeration, not a shape predicate: .claude/rules/no-new-park-categories.md:340-342 ('applies to func_8001E404, func_8001E6E4, func_8003CF84 ONLY; any further use requires a fresh owner ruling') and engine/volatile_cheats.py:744 ('Any extension requires a fresh owner ruling'). All five later rows carry inline dated owner-ruling citations.

- [s11] The residual is a source DECLARATION, not an unfound spelling: s10's sibling transplant showed func_80047FBC (src/text1b.c:82) is this same routine with four parameters, COMPLETED-C on main, line-for-line identical to candidate.c apart from its granted volatile u32 pre_pad[8]; (engine/volatile_cheats.py:757-758). func_80047EE8 and func_800481E8 carry the identical grant. func_800480C0 is the fourth family member, same TU, same untouched window, no row.

- [s11] Exhaustion: ten sessions, floor FLAT at 20 since s3, across seven distinct modalities (recon, structural, permuter, synthesis, solver, forensics, rederive) - including ~50k permuter iterations over 2 chassis with --stack-diffs (s4), the combine.c distribute_notes orphan-USE producer identification (s6, combine.c:10832-10841), a solver classify that closed its own axis PRE-RA with 'next tool: none' (s7), a filter-free census of all 1096 compiled functions (s8), and 55 structural respellings every one of which measured vars=0 / unalloc=0 (s10). 29 instance kills + 2 class kills, 31 rejected forms banked.

- [s11] Bytes remain proven for the granted form: with the enumeration row present the sandbox reads 0 and the full clean-driver build SHA1s to 62efab4f73f992798c43e8c730aa43baa10bb4fa (tmp/grind/func_800480C0/s1/build.log). The function integrates unchanged the moment an owner ruling adds the row.

- [s11] Disposition record filed this session at docs/grind/decisions.md:20353 - '## 2026-09-02 - func_800480C0 (src/text1b.c) - RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED'. It is a proof-of-foreclosure record, not a decision packet and not a re-filing of the INTEGRATION HANDOFF at decisions.md:20282 (Judge FAIL at decisions.md:20349).

- [s11] src/text1b.c was restored to its HEAD INCLUDE_ASM("asm/funcs", func_800480C0); state after the measurement; the session's diff touches only docs/grind/decisions.md, memory/grind/func_800480C0/{candidate.c,evidence.md,hypotheses.md}, tmp/, and the engine-written metrics/events.jsonl.

- [operator 2026-09-02] owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): re-activated — ledger floor 20 > ENDGAME_LOCK_MAX_FLOOR=5, so the 2026-07-27 standing ruling was never its subject; the ladder runs a second full cycle (20 flat sessions, >= 6 modalities) before any disposition. All standing banned_constructs remain in force. exhaustion_base=11
