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
