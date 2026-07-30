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
