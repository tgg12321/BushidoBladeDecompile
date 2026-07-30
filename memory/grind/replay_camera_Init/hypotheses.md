# Hypothesis ledger — replay_camera_Init

## s1 (recon, 2026-07-30)

### H1 — The 2-insn `D_80101E70` reload gap is ENTIRELY the pre-existing `extern volatile`. **CONFIRMED**
**Statement.** Of the 3 instructions by which our build falls short of target
(36 vs 39), exactly 2 are the `lui $v1,%hi(D_80101E70)` / `lw $v1,%lo(D_80101E70)($v1)`
re-load that target emits immediately after storing the same global. Those 2
instructions are produced ONLY by the `volatile` qualifier on the declaration
`extern volatile s32 D_80101E70;` at `src/code6cac_b2_post.c:45`, which the
cheat-invisible sandbox STRIPS before scoring. The honest floor of 17 is
therefore measured in a regime where the reload cannot exist at all.

**Mechanism (named, GCC 2.7.2 source).** `cse.c:7308-7361` ("Now insert the
destinations into their equivalence classes") inserts the store's destination
MEM into the hash table with the stored value as its equivalence class, so the
later read of the same MEM folds to the register that was stored — store-to-load
forwarding. A `volatile` MEM sets `do_not_record` during hashing, so
`sets[i].src_elt == 0` and the guard at `cse.c:7329` (`|| sets[i].src_elt == 0
... continue;`) skips the insertion entirely; the later read then has nothing to
fold against and survives as a real load all the way to codegen.

**Probe / measurement.**
- `cc1 -da` on the unstripped tree: the initial `.rtl` dump already carries
  `(set (mem/v:SI (symbol_ref "D_80101E70")) (reg 81))` and
  `(set (reg 85) (mem/v:SI (symbol_ref "D_80101E70")))` — the `/v` volatile flag
  — and BOTH mentions survive unchanged through jump / cse / loop / cse2 / flow /
  combine / sched / lreg / greg. The load is never folded when volatile is present.
- `grep D_80101E70 tmp/sandbox/replay_camera_Init/src/code6cac_b2_post.c` →
  `extern s32 D_80101E70;` (volatile removed); `grep -c volatile` on the sandbox
  copy = 0, on the real source = 2. The sandbox demonstrably strips it.
- `sandbox --disable all` = **17**, build_insns 36 (no reload).
  `sandbox --disable all --keep-cheat-asm` = **14**, build_insns 38 (reload present,
  emitted at exactly target's position).

**Consequence.** 3 of the 17 honest points and 2 of the 3 missing instructions are
gated on a policy question, not on a C-structure search. The volatile at line 45
predates this grind; it is not something a prior session added to close this
function, but the sandbox classifies it as a coercion cheat regardless.

### H2 — A plain `s16 *s0 = &D_80101E62;` alias can substitute for the volatile by invalidating CSE's memory table. **KILLED**
**Statement.** Target's `sh $a0, 0($t0)` store to `D_80101E62` goes through a
register-held address rather than `%lo(sym)($at)`. If that store's address were
opaque to CSE, `note_mem_written` would flag the insn as writing all of memory,
killing the `D_80101E70` equivalence and producing the reload WITHOUT volatile.

**Mechanism.** `cse.c:7539-7578` `note_mem_written`: for a MEM store whose address
satisfies `cse_rtx_addr_varies_p`, and which is neither `MEM_IN_STRUCT_P` nor a
PLUS-form (array-element) address, it sets `writes_ptr->all = 1`, which drives
`invalidate_memory` and discards every memory equivalence in the table — including
the `D_80101E70` store entry recorded earlier in the same basic block.

**Probe.** Applied `s16 *s0 = &D_80101E62;` with `*s0 != 0` and `*s0 = 2`,
sandboxed `--disable all`, and disassembled the object.

**Result.** Score **17**, build_insns **36** — byte-for-byte the same shortfall.
No reload appeared. The alias DOES reproduce target's address shape (`lui $a2;
addiu $a2,$a2,%lo; lh $v0,0($a2)` … `sh $v1,0($a2)`, i.e. one materialized address
reused on both sides of the branch, matching target's `$t0`), but it does NOT
invalidate memory. Reason: `cse_insn` canonicalizes and `fold_rtx`-folds the
destination MEM's address before `note_mem_written` inspects it, and the address
pseudo carries `(expr_list:REG_EQUIV (symbol_ref "D_80101E62"))` (visible in the
`.greg` dump, insn 13), so the address folds to a constant symbol_ref and
`cse_rtx_addr_varies_p` is false. Any C-level pointer whose initializer GCC can
constant-fold will fail the same way. **The alias is dead as a reload lever.**

### H3 — The alias is score-inert. **KILLED (prior session's conclusion overturned)**
**Statement.** The s0 prior-session note records "removed the alias entirely —
distance UNCHANGED at 17, so the alias was not load-bearing."

**Probe.** Re-measured the alias in BOTH regimes rather than only the stripped one:

| form | `--disable all` | `--disable all --keep-cheat-asm` |
|---|---|---|
| no alias (candidate.c) | 17 / 36 insns | **14** / 38 insns |
| with `s16 *s0` alias    | 17 / 36 insns | **13** / 38 insns |

**Result.** The alias is inert ONLY in the reload-absent (volatile-stripped)
regime. Once the reload exists it is worth **−1**. The prior session measured it
in the one regime where it could not show an effect and generalised. This does
NOT make the alias committable (it is still an unannotated
pointer-alias-fake-exception shape and was reviewer-FAILed on that ground), but it
means the alias must be re-evaluated, not assumed useless, by any session that
resolves H1.

### H4 — Extra declared parameters shift the register allocation to target's `$a3`/`$t0`. **KILLED**
**Statement.** Our RA gives the three global allocnos `$a0/$a1/$a2`; target uses
`$a0/$a3/$t0` — a uniform +2 shift, implying hard regs `$a1` and `$a2` were
excluded (conflicting) for the allocno in target's compile. If the original
function had additional declared parameters, those incoming argument registers
would be live at entry and would conflict with any allocno born there.

**Probe.** Changed the signature to `replay_camera_Init(s32 a0, s32 a1, s32 a2)`
leaving `a2` unused; `sandbox --disable all`.

**Result.** Score **17**, build_insns **36** — completely unchanged. `flow` deletes
the dead parameter-copy insn before `global.c` builds its conflict graph, so a
declared-but-unused parameter contributes no conflict and cannot shift the
allocation. Adding *used* parameters is not available (the in-file caller
`special_camera_check_pos_outside_ground_80036E34` passes exactly two).

## Live frontier (for s2)

1. **Decide the volatile.** Run the two-prong
   `legitimate-volatile-interrupt-touched` test on `D_80101E70`: census every
   writer of the global across `src/` and the PsyQ callback/IRQ surface. If an
   identifiable asynchronous (VSync / cdrom callback / root-counter) writer
   exists, the existing declaration is a sanctioned carve-out, the honest floor
   legitimately drops 17 → 14, and the remaining gap is a 1-insn RA problem.
   If no such writer exists, the volatile is a coercion the operator must
   eventually retire, and the reload must come from somewhere else — which,
   given H2, means no C-level pointer alias will do it.
2. **The `addu $a3,$a1,$zero` RA shift.** `.greg` (this session, banked in
   `tmp/grind/replay_camera_Init/s1/rtl/`) shows exactly 3 global allocnos:
   72 (preferences 4) → `$a0`, 73 (preferences 5) → `$a1`, 75 (no preference) →
   `$a2`. MIPS defines no `REG_ALLOC_ORDER` in this tree (grep over
   `tools/gcc-2.7.2/` finds it only in ChangeLogs), so `global.c` `find_reg`
   masks conflicts and preferences (`global.c:1093`) and then takes the lowest
   free hard reg. To land on `$a3` the allocno for the `a1` value must conflict
   with BOTH `$a1` and `$a2`. Find a C shape that makes the `a1`-valued pseudo
   live across a region where those two hard registers are still live. H4 kills
   the dead-parameter route; the untried routes are shapes that keep the
   incoming `$a1` hard reg live past the pseudo's birth.
3. **Permuter has never been run on this function** (the s0 session explicitly
   deprioritized the infra setup). With the corrected clean single-function
   `target.o` recipe from `difficult-is-not-impossible.md` §3, a directed run
   seeded from candidate.c is genuinely unexplored search space.

## [s1] The two missing instructions of the D_80101E70 re-load (lui $v1,%hi + lw $v1,%lo) exist in target ONLY because of the pre-existing `extern volatile s32 D_80101E70;` at src/code6cac_b2_post.c:45, which the cheat-invisible sandbox strips before scoring — so the honest floor of 17 is measured in a regime where that reload cannot exist at all.
- mechanism: GCC 2.7.2 cse.c:7308-7361 inserts a store's destination MEM into the equivalence table with the stored value's class, so a later read of the same MEM folds to the stored register (store-to-load forwarding). A volatile MEM sets do_not_record during hashing, leaving sets[i].src_elt == 0, and the guard at cse.c:7329 skips the insertion entirely; the later read then has no equivalence to fold against and survives as a real load to codegen.
- probe: cc1 -da over the unstripped tree (artifacts in tmp/grind/replay_camera_Init/s1/rtl/): the initial .rtl already carries (mem/v:SI (symbol_ref "D_80101E70")) on BOTH the store and the read, and both survive unchanged through jump/cse/loop/cse2/flow/combine/sched/lreg/greg. Confirmed the sandbox strips it: tmp/sandbox/replay_camera_Init/src/code6cac_b2_post.c:45 reads `extern s32 D_80101E70;` and grep -c volatile on the sandbox copy is 0 vs 2 on the real file. Then measured both regimes: `sandbox --disable all` = 17 / 36 insns (no reload) vs `sandbox --disable all --keep-cheat-asm` = 14 / 38 insns (reload emitted at exactly target's position).
- result: CONFIRMED — 3 of the 17 honest points and 2 of the 3 missing instructions are gated on the volatile policy question, not on a C-structure search. The volatile predates this grind (it is not something a prior session added to close this function), but the sandbox classifies it as a coercion regardless.
- verdict: CONFIRMED

## [s1] A plain C pointer alias `s16 *s0 = &D_80101E62;` (matching target's register-held store address `sh $a0, 0($t0)`) will make CSE treat that store as writing an unknown address, invalidating all memory equivalences and producing the D_80101E70 reload WITHOUT the volatile.
- mechanism: cse.c:7539-7578 note_mem_written sets writes_ptr->all = 1 — which drives invalidate_memory and discards every memory equivalence in the table — for a MEM store whose address satisfies cse_rtx_addr_varies_p and which is neither MEM_IN_STRUCT_P nor a PLUS-form (array-element) address.
- probe: Applied the alias with `*s0 != 0` and `*s0 = 2`, ran `sandbox --disable all`, and disassembled the resulting object.
- result: KILLED — score 17, build_insns 36, byte-for-byte the same shortfall; no reload anywhere in the object. The alias DOES reproduce target's address shape (lui $a2; addiu $a2,$a2,%lo; lh $v0,0($a2) ... sh $v1,0($a2): one materialized address reused across the branch, matching target's $t0) but it does not invalidate memory, because cse_insn canonicalizes and fold_rtx-folds the destination address before note_mem_written sees it and the address pseudo carries REG_EQUIV (symbol_ref "D_80101E62") (.greg insn 13), so the address folds to a constant and does not vary. This closes the WHOLE family: any C pointer whose initializer GCC can constant-fold fails identically.
- verdict: KILLED

## [s1] s0's ledger conclusion that the `s16 *s0 = &D_80101E62;` alias is not load-bearing and can be dropped for free is correct in general.
- mechanism: s0 measured the alias only with `--disable all`, i.e. in the volatile-stripped regime where the D_80101E70 reload is structurally absent and the alias's register/scheduling effect has nothing to act on.
- probe: Re-measured the alias in BOTH regimes: alias-free = 17 / 36 insns (--disable all) and 14 / 38 insns (--keep-cheat-asm); with-alias = 17 / 36 insns and 13 / 38 insns.
- result: KILLED (s0's generalisation overturned) — the alias is inert only in the reload-absent regime; once the reload exists it is worth -1. This does not make it committable (it is still an unannotated pointer-alias-fake-exception and was reviewer-FAILed in s0 on that ground), but it must be re-measured rather than assumed useless by any session that resolves the volatile question.
- verdict: KILLED

## [s1] Declaring additional parameters makes more incoming argument hard registers live at function entry, creating the conflicts that would shift RA from our $a0/$a1/$a2 to target's $a0/$a3/$t0 and thereby materialise the missing `addu $a3,$a1,$zero` parameter copy.
- mechanism: global.c find_reg masks an allocno's conflicts and preferences (global.c:1093) and then takes the lowest-numbered free hard reg; this tree defines no REG_ALLOC_ORDER for MIPS (grep over tools/gcc-2.7.2/ hits only ChangeLogs), so the default 0..31 order gives our 4/5/6. Target's 4/7/8 is a uniform +2 shift, implying hard regs $a1 and $a2 were genuinely excluded for the a1-valued allocno.
- probe: Changed the signature to replay_camera_Init(s32 a0, s32 a1, s32 a2) leaving a2 unused; ran `sandbox --disable all`.
- result: KILLED — score 17, build_insns 36, completely unchanged. flow deletes the dead parameter-copy insn before global.c builds its conflict graph, so a declared-but-unused parameter contributes no conflict. Adding a *used* extra parameter is not available: the in-file caller special_camera_check_pos_outside_ground_80036E34 passes exactly two arguments.
- verdict: KILLED

## s2 (structural, 2026-07-30)

### H5 — `D_80101E70` qualifies for the legitimate-volatile-interrupt-touched carve-out (prong 1: an identifiable asynchronous IRQ/callback writer exists). **KILLED — NEGATIVE CENSUS, CLOSED GATE**
**Statement.** s1 left prong (1) of the two-prong test open. If some IRQ /
callback / VSync handler asynchronously writes `D_80101E70`, the pre-existing
`extern volatile s32 D_80101E70;` at `src/code6cac_b2_post.c:45` is a sanctioned
declaration, the honest floor legitimately drops 17 -> 14, and the residual is a
1-instruction RA problem.

**Probe.** Full-tree census: every mention of `D_80101E70` across `src/`, then
every callback-registration surface reachable from this file.

**Result (KILLED).** `D_80101E70` has exactly TWO writers in the entire tree,
both synchronous and both in this file:
  - `src/code6cac_b2_post.c:263` — `replay_camera_Init` itself (the store under study)
  - `src/code6cac_b2_post.c:329` — `func_80036FD4`, called synchronously from
    `src/code6cac_b2_post.c:362` and `src/code6cac_c2.c:327`

The remaining seven mentions are the `extern` declarations in the six
`code6cac*` translation units plus `code6cac_b2_pre.c:48`. NO handler writes it.
Every callback registration in the tree installs a DIFFERENT address:
`cdrom_SetCallbackA((s32)&D_80080014)` / `cdrom_SetCallbackB((s32)&D_8008003C)`
(`src/display.c:3743-3744`), `cdrom_SetCallbackB((s32)&D_80082050)`
(`src/system.c:1146`), `spu_SetCallback((s32)&g_snd_irq_data)`
(`src/main.c:1617`); the other `cdrom_SetCallback*` calls pass `0` or the saved
`D_800A14F4` / `D_800A14F8` handles. `marionation_camera_Init_80036064`
(`src/code6cac_b2_post.c:205`) — the replay-camera path the s1 frontier
suspected — calls `cdrom_SetCallbackB(0)`, i.e. it DEREGISTERS.

**Consequence.** Prong (1) FAILS. Per the owner's standing auto-ruling, a census
that came back negative is a FAILED gate, not an open question: the volatile at
line 45 is NOT carve-out-eligible, the sandbox is right to strip it, and the
honest floor of 17 stands. The 2-instruction `D_80101E70` reload has no
sanctioned source. Combined with s1's H2 (which closed the entire "C pointer
alias invalidates CSE" family), and with the fact that target's own assembly
contains NO call and NO varying-address store between the `D_80101E70` store and
its reload, there is currently no known GCC 2.7.2 mechanism by which a
non-volatile store-then-read of the same global survives cse_insn's
store-to-load forwarding. This does NOT make the function un-matchable — the
driver has not declared exhaustion and the permuter axis is still entirely
unrun — but it does mean the reload is not reachable on the structural axis.

### H6 — Statement order / declaration order / type narrowing can move the honest floor below 17. **KILLED**
**Statement.** The structural modality's own axis: reorder statements, move
declarations, narrow types, re-associate expressions, invert the guard.

**Probe.** Built a sweep harness (`tmp/grind/replay_camera_Init/s2/sweep.py`)
that swaps the function body for each variant in `variants/`, runs the sandbox,
and restores the file. Eight forms, measured in BOTH regimes (s1's lesson: never
generalise from the stripped regime alone).

| variant | `--disable all` | `--keep-cheat-asm` |
|---|---|---|
| v01 target execution order | 18 / 37 | 23 / 40 |
| v02 reload into a named local | 18 / 37 | 23 / 40 |
| v03 `D_80101E60 = a0` after the loads | 22 / 38 | 24 / 40 |
| v04 inverted guard (`if (== 0) { ... return 1; } return 0;`) | 23 / 37 | 27 / 39 |
| v05 pointer locals for both loads | 18 / 37 | 23 / 40 |
| **v06 == candidate.c (`D_80101E7C = a1` between the loads)** | **17 / 36** | **14 / 38** |
| v07 `D_80101E7C = a1` last | 19 / 37 | 23 / 40 |
| v08 `(s16)` narrowing for `sval` | 18 / 37 | 23 / 40 |

**Result (KILLED).** candidate.c is a strict local optimum; every neighbour on
the ordering axis is worse in both regimes. Notably the "natural" spelling that
writes the statements in target's own execution order is WORSE (18 / 23), which
is why the odd placement of `D_80101E7C = a1;` between the two loads must not be
tidied up. The ordering axis is measured dead. Banked:
`memory/grind/replay_camera_Init/rejected/structural-order-sweep-no-gain.c`;
raw numbers in `tmp/grind/replay_camera_Init/s2/sweep_results.json` and
`sweep_results_keepcheat.json`.

### H7 — The missing `addu $a3,$a1,$zero` is produced when another live value occupies hard reg `$a1` across the parameter-home copy's live range. **CONFIRMED (mechanism), but unreachable with this function's honest value set**
**Statement.** s1 left the a1 -> $a3 shift as the open structural question. The
copy insn `(set (reg 73) (reg 5))` survives as a real instruction precisely when
allocno 73 does not get hard reg 5.

**Mechanism (named, GCC 2.7.2 source, line-cited).**
  - `.greg` for the current candidate (regenerated this session, banked at
    `tmp/grind/replay_camera_Init/s2/rtl/base.i.greg`) shows only TWO global
    allocnos — not s1's three, because s1 measured with the pointer alias in
    place: `;; 72 conflicts: 72 73 2 5 29` / `;; 72 preferences: 4` (the a0-home
    pseudo, which DOES conflict with $a1 because $a1 is still live when it is
    born) and `;; 73 conflicts: 72 73 2 3 29` / `;; 73 preferences: 5` (the
    a1-home pseudo). `Register dispositions: 72 in 4  73 in 5` — 73 takes its
    preference.
  - `global.c` `prune_preferences` (the same-size
    `AND_COMPL_HARD_REG_SET (temp, hard_reg_full_preferences[allocno]);` clause,
    global.c:893-895) explicitly refuses to put into `regs_someone_prefers[A]`
    any register that A itself prefers. Since 73's full preferences are {5},
    **$a1 can never be excluded from allocno 73 by the preference machinery.**
  - `global.c` `find_reg` seeds `regs_used_so_far` with ALL `call_used_regs`
    (global.c:352-355), so the pass-0 "never allocate a register for the first
    time in pass 0" restriction never protects $a1..$a3 / $t0..; pass 0 simply
    takes the lowest hard reg not in conflicts ∪ regs_someone_prefers. No
    `REG_ALLOC_ORDER` is defined for MIPS in this tree.
  - Therefore the ONLY route to denying 73 hard reg 5 is a genuine entry in
    `hard_reg_conflicts[73]`, i.e. $a1 must be OCCUPIED by an overlapping live
    value.

**Probe.** Diagnostic form with three extra simultaneously-live block-local
temporaries spanning the `D_80101E7C = a1;` store (semantics deliberately
changed; never committable). Sandboxed and disassembled.

**Result (CONFIRMED mechanism / KILLED as a route).** The home copy materialises
exactly as predicted: the object contains `3b0: move v1,a1` at the top of the
function — the same insn target places in the `bnez` delay slot — while a local
temp took $a1 (`3e4: lw a1,8(at)`). Score 26 / 49 insns. So the mechanism is
real and now proven, not inferred. But it is dead as a route for two independent
reasons: (1) the extra temporaries have no semantic purpose, making any such
form a dead-value cheat by any spelling; (2) even so, the copy lands in $v1
(hard reg 3), NOT target's $a3 (hard reg 7) — reaching 7 requires hard regs
2, 3, 4, 5, 6 to all be unavailable at that allocno's allocation point, i.e.
FIVE values live across the a1-store region, where this function's honest value
set (sval / cam_val / ec_val) supplies three. Banked:
`memory/grind/replay_camera_Init/rejected/extra-live-temps-forces-copy-but-is-dead-value-cheat.c`.

## Live frontier (for s3)

1. **Permuter — still never run on this function** (s0 deprioritized the infra,
   s1 was recon, s2 was structural). This is now the single largest unexplored
   axis. Build a clean single-function `target.o` from
   `asm/funcs/replay_camera_Init.s` + `tools/decomp-permuter/prelude.inc` (drop
   the `.set gp=64` line for r3000) so the function sits at offset 0 like
   `base.o`, per difficult-is-not-impossible.md §3; seed from candidate.c;
   launch with `tools/permuter_campaign.py`, wait IN-TURN with
   `permuter_campaign.py wait --dir <ws>`, and `harvest --stop` before writing
   the outcome.
2. **Re-examine the provenance of the `$a3` value.** Every structural attack so
   far assumes `addu $a3,$a1,$zero` is the ordinary parameter home for a
   two-parameter leaf. s2's RA analysis shows that assumption forces a
   contradiction: in a 2-parameter leaf whose only reference to $a1 is its own
   home copy, `find_reg` gives the pseudo hard reg 5 deterministically, yet
   target has 7 AND leaves both 5 and 6 unused everywhere in the body. The
   caller (`special_camera_check_pos_outside_ground_80036E34`, asm read this
   session) sets up only $a0/$a1 before the `jal` and moves its own $a2/$a3 into
   $s1/$s2, so a wider declared signature is not supported from the call side
   either. Something about the original compile's allocno set differs from ours
   in a way neither s1 nor s2 has identified — m2c the target and diff its
   allocno structure against ours rather than assuming our two-allocno shape.
3. **The `$t0` address cache for `D_80101E62`** remains the one known -1 (14 ->
   13 in the reload regime, s1). It is a `pointer-alias-fake-exception` shape,
   which IS a sanctioned last-resort family — but it requires documented
   lever-exhaustion + a named GCC-pass mechanism + a `/* FAKE */` annotation,
   and s0's reviewer FAILed it for having none of those. After s2's kills the
   exhaustion record is materially stronger; a future session that has also run
   the permuter could re-propose it properly annotated.

## [s2] D_80101E70 qualifies for the legitimate-volatile-interrupt-touched carve-out, i.e. prong (1) holds and an identifiable asynchronous IRQ/callback writer of the global exists — in which case the pre-existing `extern volatile s32 D_80101E70;` at src/code6cac_b2_post.c:45 is sanctioned, the honest floor legitimately drops 17 -> 14, and the residual is a 1-instruction RA problem.
- mechanism: The two-prong test in .claude/rules/legitimate-volatile-interrupt-touched.md. Prong (2) (the use site demonstrably requires CSE-defeat) was already proven by s1's 14-vs-17 measurement; prong (1) requires a named asynchronous writer reachable through the PsyQ callback surface (cdrom_SetCallbackA/B, spu_SetCallback, sys_VSync, root-counter handlers).
- probe: Full-tree census of every mention of D_80101E70 across src/, cross-referenced against every callback-registration call site in the tree (cdrom_SetCallbackA/B, spu_SetCallback, SetCallback-family greps).
- result: KILLED — NEGATIVE CENSUS. D_80101E70 has exactly two writers, both synchronous and both in this file: replay_camera_Init itself (src/code6cac_b2_post.c:263) and func_80036FD4 (src/code6cac_b2_post.c:329), the latter called synchronously from src/code6cac_b2_post.c:362 and src/code6cac_c2.c:327. All seven other mentions are plain `extern` declarations in the code6cac* TUs. Every callback registration in the tree installs a DIFFERENT address: &D_80080014 / &D_8008003C (src/display.c:3743-3744), &D_80082050 (src/system.c:1146), &g_snd_irq_data (src/main.c:1617); the remaining cdrom_SetCallback* calls pass 0 or the saved D_800A14F4 / D_800A14F8 handles. marionation_camera_Init_80036064 (src/code6cac_b2_post.c:205) — the replay-camera path the s1 frontier specifically suspected — calls cdrom_SetCallbackB(0), i.e. it DEREGISTERS. Prong (1) fails; the volatile at line 45 is not carve-out-eligible, the sandbox is right to strip it, and the honest floor of 17 stands. Per the owner's standing auto-ruling a negative census is a FAILED gate, not an open question.
- verdict: KILLED

## [s2] Statement order, declaration order, type narrowing, guard inversion or statement re-association — the structural modality's own axis — can move the honest floor below 17.
- mechanism: The codegen-technique-index structural catalog: block-local var splits, declaration order, type narrowing, statement re-association. Each changes cc1's LUID ordering, the first-pass scheduler's launch priorities, and the live ranges that local-alloc and global.c see.
- probe: Built a sweep harness (tmp/grind/replay_camera_Init/s2/sweep.py) that swaps the function body for each variant file, runs `sandbox --disable all`, and restores src. Eight forms measured in BOTH regimes (s1's lesson: never generalise from the volatile-stripped regime alone): target execution order; reload into a named local; D_80101E60 store moved after the loads; inverted guard; pointer locals for both loads; candidate.c; D_80101E7C store last; (s16) narrowing for sval.
- result: KILLED — no form beats candidate.c. Stripped / with-reload scores: v01 target-order 18/23, v02 reload-local 18/23, v03 E60-after-loads 22/24, v04 inverted-guard 23/27, v05 pointer-locals 18/23, v06 (== candidate.c) 17/14, v07 E7C-last 19/23, v08 s16-narrowing 18/23. candidate.c is a strict local optimum in both regimes. The counter-intuitive result: writing the statements in TARGET'S OWN execution order is worse in both regimes than candidate.c's odd placement of `D_80101E7C = a1;` between the two loads — that placement is load-bearing and must not be tidied.
- verdict: KILLED

## [s2] The missing `addu $a3,$a1,$zero` parameter-home copy materialises as a real instruction when another live value occupies hard reg $a1 across the copy's live range — and a pure-C shape can supply that occupancy.
- mechanism: GCC 2.7.2 has no coalescing pass: the home copy `(set (reg 73) (reg 5))` survives to codegen unless allocno 73 gets hard reg 5, in which case it becomes a self-move and is deleted. global.c prune_preferences (global.c:893-895) explicitly refuses to place into regs_someone_prefers[A] any register A itself prefers (the same-size AND_COMPL_HARD_REG_SET on hard_reg_full_preferences), and 73's full preferences are exactly {5}; find_reg seeds regs_used_so_far with ALL call_used_regs (global.c:352-355) so the pass-0 'never allocate a register for the first time' rule never protects $a1..$a3 / $t0..; and no REG_ALLOC_ORDER is defined for MIPS in this tree, so find_reg takes the lowest free hard reg. Hence the only route to denying 73 hard reg 5 is a genuine hard_reg_conflicts entry — i.e. $a1 occupied by an overlapping live value.
- probe: Regenerated the cc1 -da dumps for the current candidate (tmp/grind/replay_camera_Init/s2/rtl/) and read the allocno sets directly; then built a DIAGNOSTIC form with three extra simultaneously-live block-local temporaries spanning the `D_80101E7C = a1;` store (semantics deliberately changed, never committable), sandboxed it and disassembled the object.
- result: CONFIRMED as a mechanism, KILLED as a route. The diagnostic produced the copy exactly as predicted — `3b0: move v1,a1` at the top of the function, the same insn target places in the bnez delay slot, while a local temp took $a1 (`3e4: lw a1,8(at)`); score 26 / 49 insns. Dead for two independent reasons: (1) the extra temporaries have no semantic purpose, making any such form a dead-value cheat by any spelling; (2) even so, the copy lands in $v1 (hard reg 3), NOT target's $a3 (hard reg 7) — reaching 7 needs hard regs 2,3,4,5,6 all unavailable at that allocno's allocation point, i.e. FIVE values live across the a1-store region, where this function's honest value set (sval / cam_val / ec_val) supplies three.
- verdict: CONFIRMED

## s3 (structural / permuter, 2026-07-30)

### H8 — A decomp-permuter campaign over the honest (cheat-invisible) regime finds a structural lever four sessions of manual derivation did not. **CONFIRMED — FLOOR 17 -> 13**
**Statement.** The permuter had never been run on this function. s2 declared the
ordering axis a strict local optimum from an 8-form hand sweep; a randomised
plus directed PERM_* search over statement order, temporary introduction and
expression spelling samples a much wider structural space.

**Probe.** Built a CLEAN single-function workspace per
difficult-is-not-impossible.md section 3 (`tmp/grind/replay_camera_Init/s3/ws*/`):
`target.o` assembled from `asm/funcs/replay_camera_Init.s` + the permuter
prelude with `.set gp=64` removed (r3000) so the function sits at offset 0 like
`base.o`; `compile.sh` reproduces the Makefile pipeline for code6cac_b2_post
(CC_FLAGS / prologue_fix / MASPSX_FLAGS / multu_pad, NO regfix/asmfix) and then
extracts only replay_camera_Init's region, so the score is the real per-function
diff (validated at base 36 insns vs target 39, base_score 1295). `base.c` is
preprocessed from the SANDBOX copy of the TU, i.e. the volatile-stripped
cheat-invisible source, so the search runs in exactly the regime the grind
scores. Three campaigns, ~33k iterations total, each re-seeded from the best
sandbox-measured form.

**Result (CONFIRMED).** The first campaign's best find (`ws/output-725-1`)
introduced `s32 *new_var = &D_80101E70; ... new_var2 = *new_var;` — a
pointer-mediated RE-READ of the global. De-permuted and sandboxed, that form
scores **15 / 38 insns**; reduced to its minimal delta from the s2 candidate
(candidate.c + the pointer read only) it scores **14 / 38**; adding the
analogous `s16 *pe62 = &D_80101E62;` gives **13 / 38**. The honest floor moved
17 -> 13 and build_insns 36 -> 38 for the first time in the grind.

**Cross-check that isolates the lever.** `v_b` — the permuter's statement
reordering WITHOUT the pointer, reading `D_80101E70` directly — scores 19 / 35:
the reload is gone and the score is worse than the s2 baseline. The pointer, not
the reordering, is what produces the two missing instructions.

### H9 — The D_80101E70 reload is reachable in pure C without `volatile`. **CONFIRMED (s1 H2's family kill was too broad)**
**Statement.** s2 concluded "there is currently no known GCC 2.7.2 mechanism by
which a non-volatile store-then-read of the same global survives cse_insn's
store-to-load forwarding." That is now false.

**Mechanism (named, GCC 2.7.2 source).** `cse.c:7308-7361` inserts the store's
destination MEM into the equivalence table keyed by the stored value, and the
later read folds to that register only if `exp_equiv_p` matches the two rtxes.
Reading through a pointer local makes the read `(mem (reg))` while the recorded
entry is `(mem (symbol_ref "D_80101E70"))` — they do not match, so no forwarding
happens and the `lui`/`lw` pair survives to codegen at exactly target's
position. s1's H2 tested a DIFFERENT and much stronger claim (that the alias
would invalidate ALL memory equivalences through `note_mem_written`), correctly
killed it because the address constant-folds, and then generalised the kill to
"any C pointer whose initializer GCC can constant-fold fails identically". That
generalisation was wrong: the working mechanism needs no invalidation at all,
only a non-matching read rtx — and it is the READ that must go through the
pointer, whereas H2 aliased the STORE, and to a different global (D_80101E62).

**Probe / measurement.** `sandbox --disable all`; forms banked in
`tmp/grind/replay_camera_Init/s3/variants/`, numbers in
`tmp/grind/replay_camera_Init/s3/sweep_results.json`:

| form | score | build insns |
|---|---|---|
| s0-s2 candidate (no pointers) | 17 | 36 |
| + `s32 *pe70` re-read (v_a) | 14 | 38 |
| + `s16 *pe62` guard/store alias (v_d) | **13** | **38** |
| v_d with `D_80101E7C = a1` moved late (v_g) | 13 | 38 |
| permuter ws3/output-465 de-permuted (v_h) | 13 | 38 |
| permuter ws/output-725 de-permuted (p725) | 15 | 38 |
| permuter ws2/output-625 de-permuted (v_c) | 15 | 38 |
| pointer-free reordering, direct read (v_b) | 19 | 35 |
| both pointers + target's own order (v_f) | 19 | **39** |

**Consequence.** The construct is a `pointer-alias-fake-exception` and needs a
fresh layer-2 cheat-reviewer before acceptance; candidate.c now carries the full
three-part justification (lever exhaustion / named mechanism / `/* FAKE */`)
that s0's proposal was FAILed for lacking. It is NOT the construct s0 proposed:
s0 aliased only D_80101E62 and never touched the reload.

### H10 — Target's statement order becomes correct once the reload exists. **CONFIRMED as an instruction-count match, KILLED as a score improvement**
**Statement.** s2's v01 ("target execution order") scored 18/37 and was rejected
— but s2 measured it in a regime with no reload at all. With the pointers
present, target's order (both loads, both stores, the re-read, and only THEN
`D_80101E7C = a1;`) keeps the incoming `a1` live to the very end.

**Probe.** `v_f_target_order.c`, sandboxed and disassembled against the clean
single-function target.

**Result.** **39 instructions — exactly target's count** — and the object
contains `move a2,a1` in the `bnez` delay slot: the parameter-home copy that s1
H4 and s2 H7 both failed to materialise, at target's exact position, with the
branch displacements (`bnez ...,90` / `j ...,94`) matching target too. It sits
in `$a2` where target has `$a3`, the guard address sits in `$a3` where target
has `$t0`, and the four post-load stores are emitted in a different order, so
the score is 19 rather than 13. **The a3 copy is no longer a
missing-instruction problem; it is a register-naming problem.** Banked:
`rejected/target-order-gets-39-insns-but-wrong-regs.c`.

## Live frontier (for s4)

1. **Close the last register-naming gap from v_f (score 19 / 39 insns).** This
   is the closest form ever produced: right instruction count, right branch
   displacements, the `move` in the delay slot. What remains is (a) the home
   copy in `$a2` instead of `$a3`, (b) the guard address in `$a3` instead of
   `$t0`, (c) the order of the four post-load stores. The diff is reproducible
   via `tmp/grind/replay_camera_Init/s3/setup_ws3.sh` (apply a form to src/, run
   `sandbox`, re-preprocess the sandbox copy, compile, objdump-diff). v_d and
   v_f differ by only three statement positions — hybridise one statement at a
   time and sandbox each step.
2. **Re-run the permuter seeded from v_f, not from v_d.** All three s3 campaigns
   were seeded from a 38-instruction form, so every mutation started from a body
   missing an instruction. A campaign seeded from the 39-instruction v_f
   searches purely over register allocation and store order, which is exactly
   what is left. NOTE: the permuter's weighted score does NOT track the sandbox
   score here (ws2 base 860 == sandbox 14; a permuter-625 find == sandbox 15) —
   treat permuter output only as a source of structural leads and sandbox each.
3. **Get the reviewer verdict early.** candidate.c's two pointers are the only
   thing between the current 13 and a committable form. If a fresh
   cheat-reviewer FAILs the annotated construct, the whole s3 gain reverts to 17
   and the frontier changes completely — so obtain that verdict before spending
   a session on the register-naming residue.

## [s3] A decomp-permuter campaign over the honest cheat-invisible regime — never run on this function through three sessions — finds a structural lever that manual derivation did not.
- mechanism: Random plus directed PERM_* mutation over statement order, temporary introduction and expression spelling explores a structural space that s2's hand-built 8-form ordering sweep only sampled. Run against a CLEAN single-function target.o (asm/funcs/replay_camera_Init.s + the permuter prelude with `.set gp=64` dropped for r3000) so the function sits at offset 0 like base.o and the score is the real per-function diff rather than ~340k of address noise (difficult-is-not-impossible.md section 3).
- probe: Built tmp/grind/replay_camera_Init/s3/ws, ws2, ws3: compile.sh reproduces the Makefile pipeline for code6cac_b2_post (CC_FLAGS, prologue_fix, MASPSX_FLAGS, multu_pad; NO regfix/asmfix) and extracts only replay_camera_Init's region; base.c is preprocessed from the SANDBOX copy of the TU so the search runs in the volatile-stripped regime the grind actually scores. Validated at base 36 insns vs target 39 (base_score 1295). Three campaigns, ~33k iterations, each re-seeded from the best sandbox-measured form; every find de-permuted by hand and re-measured with `sandbox --disable all`.
- result: CONFIRMED — the honest floor moved 17 -> 13 and build_insns 36 -> 38, the first movement in the grind. The lever came from ws/output-725-1, which introduced a pointer-mediated re-read of D_80101E70; reduced to its minimal delta from the s2 candidate it is worth -3, and the analogous pointer for D_80101E62 a further -1.
- verdict: CONFIRMED

## [s3] The 2-instruction D_80101E70 reload is reachable in pure C without the `volatile` qualifier — s1's H2 kill was generalised too broadly and s2's 'no known GCC 2.7.2 mechanism' conclusion is false.
- mechanism: cse.c:7308-7361 records a store's destination MEM in the equivalence table keyed by the stored value, and a later read folds to that register only if exp_equiv_p MATCHES the two rtxes. Reading through a pointer local makes the read (mem (reg)) while the recorded entry is (mem (symbol_ref "D_80101E70")); they do not match, so no store-to-load forwarding happens and the lui/lw pair survives to codegen at exactly target's position. s1's H2 tested the much stronger claim that a pointer would invalidate ALL memory equivalences via note_mem_written, correctly killed it (the address constant-folds so nothing is invalidated), and then generalised to 'any C pointer whose initializer GCC can constant-fold fails identically'. The working mechanism needs no invalidation at all, only a non-matching READ rtx — and H2 aliased the STORE, and to a different global (D_80101E62), never the read of D_80101E70.
- probe: sandbox --disable all over nine forms (tmp/grind/replay_camera_Init/s3/variants/, results in sweep_results.json): s0-s2 candidate 17/36; + pointer re-read of D_80101E70 (v_a) 14/38; + pointer for D_80101E62 (v_d) 13/38; v_g late-a1-store 13/38; v_h 13/38; permuter p725 15/38; permuter p625 15/38; v_f target-order 19/39. Control: the permuter's reordering WITHOUT the pointer, reading the global directly (v_b), scores 19/35 — the reload is absent and the form is worse than the s2 baseline, so the pointer and not the reordering is the lever.
- result: CONFIRMED — floor 17 -> 13. The construct is a pointer-alias-fake-exception and must clear a fresh layer-2 cheat-reviewer; candidate.c now carries the three prerequisites s0's FAILed proposal lacked (documented lever exhaustion, the named cse.c mechanism above, and inline /* FAKE */ annotations).
- verdict: CONFIRMED

## [s3] Target's own statement order — both loads, both stores, the re-read, and only THEN `D_80101E7C = a1;` — materialises the missing `addu $a3,$a1,$zero` parameter-home copy once the reload exists.
- mechanism: Deferring the only use of the incoming a1 to the end of the function keeps its home pseudo live across the whole body, so the copy cannot become a self-move and be deleted (GCC 2.7.2 has no coalescing pass; the copy survives unless the allocno gets hard reg 5). s2 measured target's order in a regime with NO reload at all, where the body was three instructions shorter and the liveness picture different — which is why s2 recorded target-order as strictly worse.
- probe: v_f_target_order.c = the two pointers + target's execution order; sandboxed and disassembled against the clean single-function target.o.
- result: CONFIRMED as an instruction-count match, KILLED as a score improvement. The object has exactly 39 instructions — target's count — and contains `move a2,a1` in the bnez delay slot, the very insn s1 H4 and s2 H7 failed to produce, at target's exact position, with branch displacements (bnez ...,90 / j ...,94) matching target. But the copy lands in $a2 where target has $a3, the guard address lands in $a3 where target has $t0, and the four post-load stores are emitted in a different order, so the score is 19 vs v_d's 13. The a3 copy is no longer a missing-instruction problem; it is a register-naming problem. Banked: rejected/target-order-gets-39-insns-but-wrong-regs.c.
- verdict: CONFIRMED

## [s3] A decomp-permuter campaign over the honest cheat-invisible regime - never run on this function through three sessions - finds a structural lever that manual derivation did not.
- mechanism: Random plus directed PERM_* mutation over statement order, temporary introduction and expression spelling explores a structural space that s2's hand-built 8-form ordering sweep only sampled. Run against a CLEAN single-function target.o (asm/funcs/replay_camera_Init.s + the permuter prelude with `.set gp=64` dropped for r3000) so the function sits at offset 0 like base.o and the score is the real per-function diff rather than ~340k of address noise (difficult-is-not-impossible.md section 3).
- probe: Built tmp/grind/replay_camera_Init/s3/ws, ws2, ws3: compile.sh reproduces the Makefile pipeline for code6cac_b2_post (CC_FLAGS, prologue_fix, MASPSX_FLAGS, multu_pad; NO regfix/asmfix) and extracts only replay_camera_Init's region; base.c is preprocessed from the SANDBOX copy of the TU so the search runs in the volatile-stripped regime the grind actually scores. Validated at base 36 insns vs target 39 (base_score 1295). Three campaigns, ~33k iterations, each re-seeded from the best sandbox-measured form; every find de-permuted by hand and re-measured with `sandbox --disable all`. All three campaigns harvested and stopped before the session ended (pid_alive false, `permuter_campaign.py status` shows 0 alive).
- result: The honest floor moved 17 -> 13 and build_insns 36 -> 38, the first movement in the grind. The lever came from ws/output-725-1, which introduced a pointer-mediated re-read of D_80101E70; reduced to its minimal delta from the s2 candidate it is worth -3, and the analogous pointer for D_80101E62 a further -1.
- verdict: CONFIRMED

## [s3] The 2-instruction D_80101E70 reload is reachable in pure C without the `volatile` qualifier - s1's H2 kill was generalised too broadly and s2's 'no known GCC 2.7.2 mechanism' conclusion is false.
- mechanism: cse.c:7308-7361 records a store's destination MEM in the equivalence table keyed by the stored value, and a later read folds to that register only if exp_equiv_p MATCHES the two rtxes. Reading through a pointer local makes the read (mem (reg)) while the recorded entry is (mem (symbol_ref "D_80101E70")); they do not match, so no store-to-load forwarding happens and the lui/lw pair survives to codegen at exactly target's position. s1's H2 tested the much stronger claim that a pointer would invalidate ALL memory equivalences via note_mem_written, correctly killed it (the address constant-folds so nothing is invalidated), and then generalised to 'any C pointer whose initializer GCC can constant-fold fails identically'. The working mechanism needs no invalidation at all, only a non-matching READ rtx - and H2 aliased the STORE, and to a different global (D_80101E62), never the read of D_80101E70.
- probe: sandbox --disable all over nine forms (tmp/grind/replay_camera_Init/s3/variants/, numbers in sweep_results.json): s0-s2 candidate 17/36; + pointer re-read of D_80101E70 (v_a) 14/38; + pointer for D_80101E62 (v_d) 13/38; v_g late-a1-store 13/38; v_h 13/38; permuter p725 15/38; permuter p625 15/38; v_f target-order 19/39. Control: the permuter's reordering WITHOUT the pointer, reading the global directly (v_b), scores 19/35.
- result: Floor 17 -> 13. The control proves the pointer and not the reordering is the lever. The construct is a pointer-alias-fake-exception and must clear a fresh layer-2 cheat-reviewer; candidate.c now carries the three prerequisites s0's FAILed proposal lacked (documented lever exhaustion, the named cse.c mechanism, and inline /* FAKE */ annotations).
- verdict: CONFIRMED

## [s3] Target's own statement order - both loads, both stores, the re-read, and only THEN `D_80101E7C = a1;` - materialises the missing `addu $a3,$a1,$zero` parameter-home copy once the reload exists.
- mechanism: Deferring the only use of the incoming a1 to the end of the function keeps its home pseudo live across the whole body, so the copy cannot become a self-move and be deleted (GCC 2.7.2 has no coalescing pass; the copy survives unless the allocno gets hard reg 5). s2 measured target's order in a regime with NO reload at all, where the body was three instructions shorter and the liveness picture different - which is why s2 recorded target-order as strictly worse (18/37).
- probe: v_f_target_order.c = the two pointers + target's execution order; sandboxed and disassembled against the clean single-function target.o (tmp/grind/replay_camera_Init/s3/chk/).
- result: CONFIRMED as an instruction-count match, KILLED as a score improvement. The object has exactly 39 instructions - target's count - and contains `move a2,a1` in the bnez delay slot, the very insn s1 H4 and s2 H7 failed to produce, at target's exact position, with branch displacements (bnez ...,90 / j ...,94) matching target. But the copy lands in $a2 where target has $a3, the guard address lands in $a3 where target has $t0, and the four post-load stores are emitted in a different order, so the score is 19 vs v_d's 13. The a3 copy is no longer a missing-instruction problem; it is a register-naming problem.
- verdict: CONFIRMED

## s4 (permuter, 2026-07-30)

### H11 — Seeding a permuter campaign from the 39-instruction v_f closes the register-allocation residue. **KILLED**
**Statement.** s3's frontier item 2: all three s3 campaigns were seeded from
38-instruction bodies, so every mutation started from a structurally incomplete
body and the search spent itself re-finding the missing instruction. A campaign
seeded from the 39-instruction `v_f` (target's exact instruction count, the
`move` already in the `bnez` delay slot) would search purely over register
allocation and store order — exactly what is left.

**Probe.** Built `tmp/grind/replay_camera_Init/s4/ws4` with the validated s3
recipe (`setup_ws4.sh`: clean single-function `target.o` at offset 0,
Makefile-faithful `compile.sh`, `base.c` preprocessed from the CHEAT-INVISIBLE
sandbox copy) after applying `v_f_target_order.c` to `src/`; validated at
`base insns: 39  target: 39`. Launched via
`tools/permuter_campaign.py launch --label s4-vf-39insn -j 6 --stop-on-zero`,
waited IN-TURN, ran ~23,100 iterations producing 160 output dirs, then
`harvest --stop`. Every structurally distinct find was de-permuted by hand and
re-measured with `sandbox --disable all`.

**Result (KILLED).** No find improves the floor; the campaign's finds are
uniformly WORSE than the 13 floor, and the better the permuter score the worse
the sandbox score:

| find | permuter score | sandbox score / insns |
|---|---|---|
| `output-235-1` (campaign best) | 235 | **21** / 38 |
| `output-315-1` | 315 | 15 / 37 |
| `output-360-1` (staged `sval` reuse) | 360 | 14 / 38 |
| `output-385-1` (`*pe62 = 2` before the re-read) | 385 | 15 / 38 |
| seed `v_f` itself | — | 19 / 39 |
| `candidate.c` (the floor) | 650 (measured as the ws5 base) | **13** / 38 |

Note that the two lowest-scoring finds (235, 315) both work by SINKING the
`s32 *pe70 = &D_80101E70;` initialiser below the read, i.e. they read an
UNINITIALISED pointer. That is not legal C and is not committable; both were
measured only as diagnostics.

### H12 — The decomp-permuter's weighted objective is ANTI-CORRELATED with the sandbox score on this function. **CONFIRMED**
**Statement.** s3 recorded the weaker observation that "the permuter score does
not track the sandbox score here". The s4 data is stronger and has a named
cause: the two scores move in OPPOSITE directions.

**Mechanism (named).** The permuter's default scorer (see
`memory/reference/scoring-systems.md`) is a weighted penalty —
**registers × 5**, reorderings × 60, insertions/deletions × 100 — whereas the
engine sandbox counts every differing instruction at full weight. This
function's ENTIRE remaining residue is register naming (s2/s3: the values are
instruction-exact; only `$t0` vs `$a2`/`$a3`, `$a3` vs `$a2`, `$a0` vs `$a1`
differ). The permuter therefore prices the only defect we still have at 5
points each while happily paying 100-point structural changes to shave them —
so it climbs toward bodies with more wrong instructions and slightly fewer
wrong registers. `output-235-1` is the extreme case: best permuter score in the
campaign (235, versus the 650 the score-13 `candidate.c` chassis measures at)
and the WORST sandbox score measured anywhere in this grind (21).

**Probe.** Launched a second campaign (`ws5`) seeded from `candidate.c` — the
actual score-13 floor — and read its reported `base_score`: **650**. The
permuter rates the true floor form as nearly three times worse than a find that
sandbox-measures at 21.

**Consequence (bank this — it is the modality-level lesson).** The permuter is
a STRUCTURE generator for this function, not an optimiser: s3's win came from a
find that introduced a NOVEL CONSTRUCT (the pointer-mediated re-read), not from
a low permuter score. Any future permuter session here must (a) ignore the
permuter's ranking entirely, (b) triage finds by novel-construct content, and
(c) sandbox each one. Chasing the lowest permuter score actively walks away
from the floor. The scorer cannot be reweighted from a grind session
(`tools/` is outside the allowed surface), so this limitation is permanent for
this pipeline.

### H13 — Reseeding the campaign from the score-13 `candidate.c` chassis yields a novel construct the v_f basin could not. **KILLED**
**Probe.** `ws5` (`setup_ws5.sh`, label `s4-candidate13-reseed`, -j 6,
--stop-on-zero), base validated at 38 insns vs target 39, permuter
`base_score` **650**. ~44,900 iterations, harvested and STOPPED in-turn.

**Result (KILLED).** Best permuter score 295 (late) / 380 (mid-run); nothing
approaches the floor. De-permuted and sandbox-measured:

| find | permuter score | sandbox / insns | construct |
|---|---|---|---|
| `output-295-1` | 295 | 16 / 37 | both loads hoisted out of the block, `cam_val` staged through `sval` (also semantically broken: it indexes `D_8008EC38` by `cam_val`) |
| `output-380-1` | 380 | (see H16) | `short a0` parameter narrowing |
| `output-425-1` | 425 | — | `sval = 13` constant staging + the uninitialised-`pe70` trick again |

### H16 — Narrowing the first parameter to `short a0` supplies the missing 39th instruction. **KILLED — it is the WRONG instruction**
**Statement.** The one structurally novel idea in either campaign was
`replay_camera_Init(short a0, s32 a1)` (ws5 `output-380-1`). `D_80101E60` is
written with a halfword `sh` in target, so a 16-bit first parameter is a
plausible real signature, and it would change the a0-home pseudo's live range.

**Probe.** `variants/q1_short_a0.c` = `candidate.c` with the single change
`s32 a0` -> `short a0`; `sandbox --disable all` then `diffform.sh`.

**Result (KILLED).** Score **14 / 39 insns** — it does reach target's
instruction count, and unlike the v_f family it keeps the re-read AFTER the
E70 store (semantically faithful), and it costs only +1 over the floor rather
than +4. But the 39th instruction is **`move v1,a0`** — a home copy of the
*first* parameter created by the truncation — not target's `move a3,a1`. And
target's own asm disproves the narrowing outright: target stores the parameter
with `sh a0,0(at)`, i.e. straight from the incoming register with NO truncating
copy, which is exactly what a non-narrowed `s32` parameter produces. The
narrowing is therefore both wrong-instruction and contradicted by target.
Banked: `rejected/short-a0-narrowing-wrong-home-copy.c`.

### H14 — The `addu $a3,$a1,$zero` parameter-home copy appears exactly when the two loads are made ADJACENT — not because of target's statement order. **CONFIRMED**
**Statement.** s3 attributed v_f's 39th instruction to "target's own statement
order … keeps the incoming a1 live to the very end". That attribution is wrong.
A ten-form hybridisation sweep between `v_d` (= `candidate.c`, 13 / 38) and
`v_f` (19 / 39), moving ONE statement at a time, isolates the real cause: the
copy appears in every form whose two loads are textually adjacent and in NO
form where a store separates them, independently of where `D_80101E7C = a1;`
sits.

**Mechanism.** Adjacent loads make `cam_val` and `ec_val` simultaneously live,
so one of them takes hard reg `$a1`; per s2's H7 (proven there only with
semantically dead temporaries) the `(set (reg 73) (reg 5))` home copy then
cannot become a self-move and survives to codegen. s4 supplies that `$a1`
occupancy from the function's OWN honest values — no dead temporaries — which
is exactly the route s2 recorded as closed.

**Probe.** `tmp/grind/replay_camera_Init/s4/variants/h1..h10`, swept with
`tmp/grind/replay_camera_Init/s4/sweep.py`:

| form | sandbox / insns |
|---|---|
| h1 v_f order, E6C store BETWEEN the loads | 13 / **38** |
| h2 v_f order, `D_80101E7C = a1` BETWEEN the loads | 13 / **38** |
| h3 v_d tail order, loads ADJACENT | 19 / **39** |
| h4 v_f, `D_80101E68 = 0` before the re-read | 19 / 39 |
| h5 v_f, `*pe62 = 2` before `D_80101E7C = a1` | 18 / 39 |
| h6 v_f, `D_80101E7C = a1` last | 19 / 39 |
| h7 v_f, EC38 load issued first | 18 / 39 |
| **h8 = h5 + EC38 load issued first** | **17** / 39 |
| h9 h5, `ec_val` declared before `cam_val` | 18 / 39 |
| h10 v_f, E70 stored before E6C | 19 / 39 |

**Consequence.** The 39-instruction family is a real, honest family (h8 is its
best member at 17), but it is still 4 points ABOVE the 38-instruction floor:
buying the missing instruction costs more in register naming than it saves.
h8's disassembly does fix one naming defect the s3 ledger did not know was
movable — issuing the `D_8008EC38` load FIRST puts `cam_val` in `$v1`, matching
target — but the guard address stays in `$a3` (target `$t0`), the home copy in
`$a2` (target `$a3`), and `ec_val` in `$a1` (target `$a0`).

### H15 — The 39-instruction (v_f / h8) family is SEMANTICALLY DIVERGENT from target and must not be pursued as a match. **CONFIRMED — this closes the whole family**
**Statement.** Independent of its score, the v_f/h8 family emits the
`D_80101E70` re-read BEFORE the store it is supposed to observe.

**Probe.** `tmp/grind/replay_camera_Init/s4/diffform.sh` (re-preprocess the
sandbox copy, compile with the ws4 pipeline, objdump-diff against target).
h8's object emits, in order: `lw v1` / `lw a1` (the two loads), `sw v1` (E6C),
`lui a0; lw a0,0(a0)` (**the re-read**), `li v1,2`, `sh v1,0(a3)`, … and only
near the very end `lui at; sw a1,0(at)` — the store of `ec_val` into
`D_80101E70`. Target's order is the opposite and is the only correct one:
`sw v1` (E6C), `sw a0` (E70), `lui v1; lw v1,0(v1)` (re-read), `li a0,2`, …

**Result.** In the v_f/h8 family `D_80101E78` is computed from the STALE value
of `D_80101E70`, not from `ec_val`. The scheduler is free to sink the store
past the load precisely because the pointer read `(mem (reg))` and the store
`(mem (symbol_ref))` do not alias to GCC 2.7.2's disambiguator — the same
property that makes the re-read survive at all. `candidate.c` does NOT have
this defect: its object emits `sw v1` (the E70 store) and only then
`lui v1; lw v1,0(v1)`, so it computes the value target computes. Any future
session that moves the re-read earlier in the statement order must re-check the
EMITTED order, not just the score.

## Live frontier (for s5)

1. **The register residue is a uniform DOWNWARD shift, and the unexplained
   occupant of `$a1` / `$a2` is the last open fact.** Target uses `$v1`, `$a0`,
   `$a3`, `$t0` and leaves `$a1` and `$a2` unused in the whole body; our best
   38-insn form uses `$v1` and `$a2`, our best 39-insn form `$v1`, `$a1`, `$a2`,
   `$a3`. Every allocation of ours sits one or two hard registers BELOW
   target's. s2 proved `find_reg` takes the lowest free hard reg (no
   `REG_ALLOC_ORDER` for MIPS in this tree), so target's compile carried genuine
   `hard_reg_conflicts` on `$a1` AND `$a2` at both allocation points, which
   nothing in the honest value set supplies. This is a PROVENANCE question, not
   an ordering question — the ordering axis is now measured dead in BOTH the
   38- and the 39-instruction families, and the permuter is structurally unable
   to help (H12). Next modality should be forensics / rederive: is the original
   `replay_camera_Init` the tail of a larger function, does it inline a callee,
   or does some neighbouring construct keep `$a1`/`$a2` live at entry?
2. **Do not re-open the 39-instruction family.** H15 shows its extra
   instruction is bought with a semantic divergence (stale re-read) and H14
   shows it costs +4; H16 shows the one semantically-clean route into 39 insns
   emits the wrong copy and is contradicted by target's `sh a0,0(at)`. The
   38-instruction family is the right family.
3. **`candidate.c`'s two `/* FAKE */` pointer aliases still carry NO layer-2
   cheat-reviewer verdict** (s3 could not obtain one; s4 was constrained from
   spawning agents). Highest-leverage open question in the ledger: a FAIL
   reverts the floor from 13 to 17 and changes the entire frontier. One further
   fact for that review, discovered in s4: the alias's non-aliasing property is
   load-bearing in BOTH directions — it is what makes the re-read survive, and
   it is also what lets the scheduler sink the store past the load in
   neighbouring forms (H15). `candidate.c` itself is unaffected.

## [s4] Seeding a permuter campaign from the 39-instruction v_f form (rather than a 38-instruction one) closes the register-allocation residue, because every mutation then starts from a structurally complete body and the search is purely over register allocation and store order.
- mechanism: s3's frontier item 2. All three s3 campaigns were seeded from 38-instruction bodies, so the search spent itself re-finding the missing instruction. v_f already has target's exact 39 instructions with the `move` in the bnez delay slot, so a campaign seeded there searches only what remains.
- probe: Built tmp/grind/replay_camera_Init/s4/ws4 with the validated s3 recipe (setup_ws4.sh: clean single-function target.o at offset 0, Makefile-faithful compile.sh, base.c preprocessed from the CHEAT-INVISIBLE sandbox copy) after applying v_f_target_order.c to src/; validated at 'base insns: 39  target: 39'. Launched via tools/permuter_campaign.py launch --label s4-vf-39insn -j 6 --stop-on-zero, waited IN-TURN, ~23,100 iterations / 160 output dirs, then harvest --stop. De-permuted every structurally distinct find by hand and re-measured each with `sandbox --disable all`.
- result: No find improves the floor and the finds are uniformly worse than 13: output-235-1 (campaign best, permuter 235) sandboxes at 21/38; output-315-1 at 15/37; output-360-1 at 14/38; output-385-1 at 15/38; the v_f seed itself is 19/39. The two lowest-permuter-score finds work by sinking the `s32 *pe70 = &D_80101E70;` initialiser below the read, i.e. they read an UNINITIALISED pointer — not legal C, measured as diagnostics only.
- verdict: KILLED

## [s4] The decomp-permuter's weighted objective is ANTI-correlated with the engine sandbox score on this function, so campaign ranking must be ignored entirely and finds triaged only for novel constructs.
- mechanism: The permuter's default scorer (memory/reference/scoring-systems.md) is registers x 5, reorderings x 60, insertions/deletions x 100; the engine sandbox counts every differing instruction at full weight. This function's ENTIRE remaining residue is register naming (s2/s3: the values are instruction-exact; only $t0 vs $a2/$a3, $a3 vs $a2, $a0 vs $a1 differ). The permuter therefore prices our only defect at 5 points each and pays 100-point structural damage to shave it, climbing toward bodies with more wrong instructions and marginally fewer wrong registers.
- probe: Launched a second campaign (ws5) seeded from candidate.c — the actual score-13 floor — and read its reported base_score: 650. Compared against the ws4 campaign's best find output-235-1 (permuter 235), which sandbox-measures at 21. Cross-checked with ws5's own best find output-295-1 (permuter 295) at sandbox 16/37.
- result: The permuter rates the true floor form (sandbox 13) at 650, i.e. nearly three times worse than a find that sandbox-measures at 21 — the worst number in this grind. The relationship is not merely 'not a reliable proxy' (s3's wording) but inverted. The scorer cannot be reweighted from a grind session (tools/ is outside the allowed surface), so this is permanent for this pipeline: the permuter is a STRUCTURE generator here (s3's win came from a novel construct, not a low score), never an optimiser.
- verdict: CONFIRMED

## [s4] Reseeding a campaign from the score-13 candidate.c chassis yields a structurally novel construct that the v_f basin could not.
- mechanism: Different basin, different neighbourhood: the 38-instruction chassis has the correct store/re-read ordering and a different live-range picture, so PERM_* mutation samples different structures.
- probe: ws5 (setup_ws5.sh, label s4-candidate13-reseed, -j 6, --stop-on-zero), base validated at 38 insns vs target 39, permuter base_score 650; ~44,900 iterations; waited in-turn via a looped `permuter_campaign.py wait`; harvest --stop. De-permuted and sandboxed the best finds.
- result: Best permuter scores 295 and 380; nothing approaches the floor. output-295-1 -> 16/37 (both loads hoisted out of the block plus cam_val staged through sval; also semantically broken, it indexes D_8008EC38 by cam_val). output-380-1's only novel idea was the `short a0` parameter narrowing (see the next hypothesis). output-425-1 was the uninitialised-pe70 trick again plus constant staging.
- verdict: KILLED

## [s4] Narrowing the first parameter to `short a0` supplies the missing 39th instruction cleanly.
- mechanism: D_80101E60 is written with a halfword `sh` in target, so a 16-bit first parameter is a plausible real signature, and the truncation changes the a0-home pseudo's live range and therefore the conflict graph global.c sees.
- probe: variants/q1_short_a0.c = candidate.c with the single token change `s32 a0` -> `short a0`; `sandbox --disable all`, then disassembled with tmp/grind/replay_camera_Init/s4/diffform.sh against the clean single-function target.
- result: 14 / 39 insns — it does reach target's instruction count, and unlike the v_f family it keeps the re-read AFTER the E70 store (semantically faithful), at only +1 over the floor. Dead anyway: the 39th instruction is `move v1,a0`, a truncation home copy of the FIRST parameter, not target's `move a3,a1`; and target's own asm disproves the narrowing outright — it stores the parameter with `sh a0,0(at)` straight from the incoming register with no truncating copy, which is what an un-narrowed s32 parameter emits. Banked: rejected/short-a0-narrowing-wrong-home-copy.c.
- verdict: KILLED

## [s4] v_f's 39th instruction (the a1 parameter-home copy) is produced by target's statement order keeping the incoming a1 live to the end, as s3 recorded.
- mechanism: s3's attribution. Tested by hybridising v_d (= candidate.c, 13/38) toward v_f (19/39) one statement position at a time — the exact probe s3's frontier item 1 specified.
- probe: Ten forms (tmp/grind/replay_camera_Init/s4/variants/h1..h10) swept with s4/sweep.py: h1 v_f order with the E6C store BETWEEN the loads 13/38; h2 v_f order with D_80101E7C=a1 BETWEEN the loads 13/38; h3 v_d tail order with the loads ADJACENT 19/39; h4 19/39; h5 (*pe62=2 before D_80101E7C=a1) 18/39; h6 19/39; h7 (EC38 load first) 18/39; h8 (h5 + EC38 load first) 17/39; h9 18/39; h10 19/39.
- result: s3's attribution is WRONG and is corrected in the ledger. The copy appears in every form whose two loads are textually ADJACENT and in no form where a store separates them, regardless of where D_80101E7C=a1 sits. Mechanism: adjacent loads make cam_val and ec_val simultaneously live so one takes hard reg $a1, and the (set (reg 73) (reg 5)) home copy can no longer become a self-move — s2's H7 mechanism, but supplied here by the function's OWN honest values with no dead temporaries, which is the route s2 recorded as closed. Best member of the family is h8 at 17/39, still 4 points above the 38-instruction floor; issuing the D_8008EC38 load FIRST is worth -2 and puts cam_val in $v1, matching target.
- verdict: CONFIRMED

## [s4] The 39-instruction (v_f / h8) family is a viable route to the match and should be pursued.
- mechanism: It has target's exact instruction count, the move in the bnez delay slot, and matching branch displacements — s3's live frontier item 1.
- probe: tmp/grind/replay_camera_Init/s4/diffform.sh on h8 (re-preprocess the cheat-invisible sandbox copy, compile with the ws4 pipeline, objdump-diff against target).
- result: KILLED on semantics, independently of score. h8's object emits, in order: lw v1 / lw a1 (the two loads), sw v1 (E6C), `lui a0; lw a0,0(a0)` — THE RE-READ — li v1,2, sh v1,0(a3), ... and only near the very end `lui at; sw a1,0(at)`, the store of ec_val into D_80101E70. So D_80101E78 is computed from the STALE value of D_80101E70; target stores first and re-reads after. GCC 2.7.2 may sink the store past the load precisely because (mem (reg)) and (mem (symbol_ref)) do not alias to its disambiguator — the same property that makes the re-read survive at all. candidate.c does NOT have this defect (E70 store, then `lui v1; lw v1,0(v1)`), so the floor form is semantically faithful. Any future session moving the re-read earlier must re-check the EMITTED order, not just the score.
- verdict: CONFIRMED
