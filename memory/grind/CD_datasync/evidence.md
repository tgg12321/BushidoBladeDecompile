> **ALIAS NOTE (2026-08-24):** this ledger's function is `CD_datasync` (formerly `saEft01Init` — decisions.md rulings and interior prose may use either name; any `memory/grind/saEft01Init/` path is this dir).

# Evidence bank — saEft01Init

Function: `saEft01Init` in `src/system.c` (target `asm/funcs/saEft01Init.s`,
0x80081BB0, 91 instructions). Queue verdict **C**; honest pure-C distance
(`sandbox --disable all`) **18**; 15 regfix rules currently paper over it.

## Floor history

| session | modality | floor | note |
|---|---|---|---|
| 1 | recon | 18 → 18 | baseline established; cluster-A RA mechanism identified and confirmed |
| 2 | structural | 18 → 18 | **cluster A SOLVED** (target's exact `$s0-$s3` map, only 4 callee-saves); residual moved wholesale into the tail block layout; 92 build insns vs 91 |

## Shape of the gap (session 1)

`build_insns == target_insns == 91`. There is **no insertion/deletion
component** — the entire 18 is register allocation + instruction scheduling.
The 15 regfix rules split cleanly into the same two clusters:

* **Cluster A — callee-save rotation** (`regfix.txt:97-103` + `115-119`):
  `$16 <-> $18`, `$16 <-> $17`, plus six `subst` rules rewriting the
  prologue/epilogue save/restore stack offsets and one `reorder 72,70,71`.
  A single 3-way rename.
* **Cluster B — the `debug_printf` argument block** (`regfix.txt:105-113`):
  `$2 <-> $3` / `$2 <-> $4` renames over maspsx indices 35-48 plus
  `reorder 36,35,37,38,39,40,44,43,41,45,46,47,48,42`.

## Cluster A — SOLVED IN MECHANISM (not yet in spelling)

Target's callee-save assignment:

| hard reg | target holds | our build holds |
|---|---|---|
| `$s0` (16) | `D_800A125C` base | **the `a0` param** |
| `$s1` (17) | `&D_800A1494` base | `D_800A125C` base |
| `$s2` (18) | **the `a0` param** | `&D_800A1494` base |
| `$s3` (19) | `D_800A11DC` base | `D_800A11DC` base |

`cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -da`
on the baseline form gives, for this function:

```
;; 5 regs to allocate: 73 72 77 76 75
;; Register dispositions:
72 in 16   73 in 2   74 in 3   75 in 19   76 in 18   77 in 17
```

so pseudo **72 = the param**, **75 = tbl_11dc**, **76 = idx_1494**,
**77 = tbl_125c**, and `global_alloc` fills `$s0..$s3` in the printed
allocation order. Target therefore requires allocation order **77 76 72 75**;
we produce **72 77 76 75**. Only the param needs to fall two ranks.

`tools/gcc-2.7.2/global.c:allocno_compare` computes

```
priority = floor_log2(n_refs) * n_refs / live_length   (* size, which is 1 here)
```

and `flow.c` accumulates `n_refs` **weighted by the enclosing loop_depth**.
The function's loop is a `goto` back-edge, which emits **no
`NOTE_INSN_LOOP_BEG`**, so `loop_depth` is 1 throughout and the 1-use param
(2 refs) is not separated from the 2-use table pointers (3 refs).

**Confirmed by measurement:** rewriting the loop as a real C loop (a
`do { ... } while (a0 == 0);` with a shared `ret` exit) changes the dump to

```
;; 7 regs to allocate: 73 77 76 72 109 84 75
77 in 16   76 in 17   72 in 18   84 in 20   75 in 21
```

— i.e. `tbl_125c → $s0`, `idx_1494 → $s1`, **param → `$s2`**, exactly the
target permutation for the three pseudos in question. Cluster A's mechanism
is the loop notes.

**Cost that blocks this spelling:** the loop notes also enable `loop.c`
invariant hoisting, which lifts the two loop-invariant compare constants
(`0x3C0000` → `lui $s4,0x3c`; `0x1000000` → `lui $s3,0x100`) into two fresh
callee-saves. That adds allocnos 84 and 109, displaces `tbl_11dc` to `$s5`,
and costs +7 instructions (98 build vs 91 target, score 29). Target
materialises both constants **inline inside the loop** (target idx 38 and 77)
and uses only `$s0-$s3`. Full write-up + the next-probe list:
`rejected/real-loop-confirms-ra-but-hoists-constants.c`.

Corroborating structural read of the target tail: all exits converge on
`.L80081CFC` and the function returns `$v0`, with the back-edge spelled
`beqz $s2, .L80081C10` and `addiu $v0,$zero,0x1` in its delay slot. That is a
bottom-tested loop with a single shared return value — consistent with a real
`do { ... } while (param == 0);` and NOT with the current per-path `return`
spelling. A plain `do { ... } while (1);` with inline `return`s measured
30/99 insns, marginally worse than the shared-`ret` shape at 29/98, so the
shared-exit shape is the one to keep when re-probing.

## Cluster B — the `debug_printf` argument block

Target order (target idx 46-61):

```
lbu  $a0, 0x0($s1)        ; idx_1494[0]
lbu  $v0, 0x1($s1)        ; idx_1494[1]
lui/lw $a1 = D_800F19C0
sll  $v0,$v0,2 ; addu $v0,$v0,$s0 ; sll $a0,$a0,2
lw   $v1, 0x0($v0)        ; tbl_125c[idx[1]]
lui/lbu $v0 = D_800A11D5
addu $a0,$a0,$s0 ; sll $v0,$v0,2 ; addu $v0,$v0,$s3
sw   $v1, 0x10($sp)       ; 5th (stacked) arg
lw   $a2, 0x0($v0)        ; tbl_11dc[D_800A11D5]
lw   $a3, 0x0($a0)        ; tbl_125c[idx[0]]   <-- LAST
```

Two measured spellings, both scoring 18:

* staging `arg5 = tbl_125c[idx[1]]` first (the inherited baseline) puts the
  `$a1`/`D_800F19C0` load in the right early slot but loads the two `lbu` in
  the wrong order (`0x1` before `0x0`);
* staging `arg4 = tbl_125c[idx[0]]` first (the current candidate) fixes the
  `lbu` order but pushes the `$a1` load late (build idx 56/57) and pulls the
  `lw $a3` early (build idx 52 vs target 61).

The named intermediates themselves are load-bearing — collapsing them into a
single call expression scores 23
(`rejected/single-expression-printf-call.c`).

Note that cluster B has NOT been re-measured on top of a correct cluster-A
allocation. The `$2 <-> $3` / `$2 <-> $4` renames may partly be downstream of
the `$s0`/`$s1` mix-up (the arg-block address chains index off `$s0` in target
and off `$s1` in our build), so cluster B should be re-diffed once cluster A
lands rather than optimised independently.

## Structural constraints established (do not violate)

1. The three table base pointers **must be explicit source-level locals**
   assigned before the loop. Written as inline global references at their use
   sites, GCC does not hoist the `%hi/%lo` pairs out of the goto-loop at all:
   85 build insns vs 91 target, score 40
   (`rejected/inline-globals-kills-hoisted-base-pointers.c`).
2. The staged `arg4`/`arg5` locals for the `debug_printf` call must stay.
3. `$s0..$s3` and only those four callee-saves; any form that allocates `$s4`
   or `$s5` is already +4 insns from the save/restore pair.

## Artifacts (session 1)

* `tmp/grind/saEft01Init/s1/grind_diff.py` — positional target-vs-build
  disassembly differ (objdump on the sandbox `.o` vs `asm/funcs/<func>.s`).
  Reusable for any function: `python3 <script> <func>`.
* `tmp/grind/saEft01Init/s1/greg.sh` — reproduces the `cc1 -da` RTL dump set
  for `src/system.c` into `tmp/grind/saEft01Init/s1/dump/`. NB: cc1 reports
  errors on other functions in the TU (`marionation_Exec`, `cdrom_Initialize`)
  because the raw `-da` path skips some pipeline preprocessing; it still emits
  every dump file, so this is harmless for reading dispositions.
* `tmp/grind/saEft01Init/s1/dump/system.i.greg` — the `.greg` dump quoted
  above.
* `tmp/grind/saEft01Init/s1/v2_argswap.c` — full `src/system.c` at the
  candidate form.

- [s1] Baseline honest pure-C distance is 18 with build_insns == target_insns == 91, so the entire gap is register allocation plus instruction scheduling — there is no insertion/deletion component.

- [s1] The 15 regfix rules split into exactly two clusters: a 3-way callee-save rotation (regfix.txt:97-103 and 115-119) and the debug_printf argument block (regfix.txt:105-113).

- [s1] Pseudo identities from the cc1 -da .greg dump: 72 = the a0 param, 75 = tbl_11dc (D_800A11DC), 76 = idx_1494 (&D_800A1494), 77 = tbl_125c (D_800A125C). Baseline allocation order is 72 77 76 75; target requires 77 76 72 75.

- [s1] Target's callee-save map is $s0 = D_800A125C, $s1 = &D_800A1494, $s2 = the a0 param, $s3 = D_800A11DC; our build's is $s0 = param, $s1 = D_800A125C, $s2 = &D_800A1494, $s3 = D_800A11DC.

- [s1] global_alloc fills $s0..$s3 strictly in the printed allocation order for this function, so the whole rotation reduces to moving the param from allocation rank 1 to rank 3.

- [s1] allocno_compare in tools/gcc-2.7.2/global.c: priority = floor_log2(n_refs) * n_refs / live_length, with size 1 for all four SI pseudos; ties break on allocno number.

- [s1] Emitting loop notes is sufficient to produce target's exact disposition for the three pseudos that matter (measured, see H4), so cluster A is a solved mechanism blocked only on a spelling that denies loop.c its invariant hoisting of the 0x3C0000 and 0x1000000 compare constants.

- [s1] Target materialises both loop-invariant compare constants inline inside the loop (target idx 38: lui $v0,(0x3C0000>>16); target idx 77: lui $v1,(0x1000000>>16)) and uses only $s0-$s3, so no matching form may allocate $s4/$s5 — that alone costs +4 insns in save/restore.

- [s1] Target's tail structure — all exits converging on .L80081CFC, returning $v0, with the back-edge spelled 'beqz $s2, .L80081C10' and 'addiu $v0,$zero,0x1' in its delay slot — is a bottom-tested loop with a single shared return value, consistent with do { ... } while (param == 0); and not with the current per-path return spelling. do{...}while(1) with inline returns measured 30 at 99 insns, marginally worse than the shared-ret shape at 29/98.

- [s1] The three table base pointers must remain explicit source-level locals assigned before the loop: written inline at their use sites, GCC hoists nothing and the build is 6 instructions short (85 vs 91).

## Session 2 — the allocation arithmetic is now DIRECTLY OBSERVABLE (no instrumented cc1 needed)

Session 1's F3 wanted an instrumented cc1 build to read `allocno_n_refs` /
`allocno_live_length`. **That is unnecessary.** `flow.c:dump_flow_info` already
prints both fields for every pseudo, and cc1 emits it into the `-da` dumps:

* `system.i.flow`  — values as of `life_analysis` (BEFORE `local_alloc`)
* `system.i.lreg`  — values as of AFTER `local_alloc`; **these are the ones
  `global_alloc` actually sorts on**

Line format: `Register 77 used 3 times across 96 insns; ... ; pointer`
→ `allocno_n_refs = 3`, `allocno_live_length = 96`.

Feeding those into `global.c:allocno_compare`

```
pri = floor_log2(n_refs) * n_refs / live_length * 10000 * size      (size == 1)
ties broken by allocno index, i.e. by ascending pseudo number
```

reproduces the `;; N regs to allocate:` order in the `.greg` dump EXACTLY, on
every one of the 10 variants measured this session. **The allocation order is
now a closed-form prediction from a fast `cc1 -da` run — no sandbox build
needed to screen a structural variant.** Harness:
`tmp/grind/saEft01Init/s2/sweep.py` (splices a variant body into
`src/system.c`, dumps, parses, prints per-pseudo priority + predicted order).

Baseline (goto-loop) numbers, from `.lreg`:

| pseudo | what | n_refs | live_len | pri | gets |
|---|---|---|---|---|---|
| 73 | sys_VSync return | 5 | 4 | 25000 | `$v0` |
| 72 | the `a0` param | 2 | 52 | 384.6 | `$s0` |
| 77 | tbl_125c | 3 | 96 | 312.5 | `$s1` |
| 76 | idx_1494 | 3 | 98 | 306.1 | `$s2` |
| 75 | tbl_11dc | 2 | 100 | 200.0 | `$s3` |

### Why the param outranks the table pointers: the REG_EQUIV live-length doubling

`local-alloc.c:update_equiv_regs` line 1064 does `reg_live_length[regno] *= 2`
for every pseudo whose single set carries a `REG_EQUIV` note. All three table
pointers are `(set (reg) (symbol_ref "D_800A1xxx"))` and carry
`REG_EQUIV (symbol_ref ...)`, so their raw lengths 48/49/50 become 96/98/100.
The param's set is `(set (reg/v 72) (reg:SI 4 a0))` — no `REG_EQUIV`, no
doubling, so 52 stays 52. That single ×2 is the whole inversion.

**Consequence (proved arithmetically, not just measured):** with the pointers
doubled and the param not, `P76 > P72` requires `2*raw76 < 1.5*raw72`, i.e.
`raw72 > 65`, but `raw72` is bounded by the function's total live span (~52).
**No goto-loop form can reach target's order.** Statement placement moves the
raw lengths by only ±4 (measured: `v2_ptrs_late` = no change at all,
`v3_ptrs_first` = +4 on all three pointers). The loop notes are therefore not
one option among several — they are structurally required.

### Cluster A closed: real loop + ONE reused scratch for both constants

Session 1's blocker was that the loop notes also let `loop.c` hoist the two
loop-invariant compare constants into `$s4`/`$s5` (+7 insns). The suppression
condition, read out of `tools/gcc-2.7.2/loop.c:scan_loop`, is the movable
gate at line 702-709:

```
&& (n_times_set[REGNO (SET_DEST (set))] == 1
    || (tem1 = consec_sets_invariant_p (SET_DEST (set), n_times_set[...], p)))
```

A pseudo SET TWICE in the loop, with the two sets NOT consecutive, satisfies
neither disjunct, so `scan_loop` never builds a movable for it and
`move_movables` cannot hoist it. Writing both constants through **one** local
`k` (`k = 0x3C0000;` at the timeout compare, `k = 0x1000000;` at the mask
test) does exactly that. Measured `.lreg` for that form:

```
79 (tbl_125c) nrefs=5 len= 98  pri=1020.4  -> $s0
78 (idx_1494) nrefs=5 len=100  pri=1000.0  -> $s1
72 (param)    nrefs=3 len= 53  pri= 566.0  -> $s2
77 (tbl_11dc) nrefs=3 len=102  pri= 294.1  -> $s3
76 (k)        nrefs=8 len=  6  pri=40000   -> $a0   (caller-saved, as wanted)
```

— target's map exactly, four callee-saves only, both `lui` constants
materialised inline in the loop. Sandbox: **18 at 92 insns** (was 18 at 91).

Also confirmed on the way: the three explored suppression routes that do NOT
work, and why, so no future session re-derives them —
(a) loop.c's `reg_single_usage` substitution (line 735-768) would delete the
constant set and fold it into the use, but `validate_replace_rtx` must produce
a recognizable insn; MIPS has no 32-bit-immediate `slt` or `and`, so it always
fails; (b) the `maybe_never` / `reg_in_basic_block_p` safety gate at line
695-701 never fires for a compiler temp, because clause (2)
`! REG_USERVAR_P && ! REG_LOOP_TEST_P` is unconditionally true for one;
(c) the `threshold` savings test in `move_movables` (line 1631) is
`threshold * savings * lifetime >= insn_count` with
`threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` ≈ 61 here, so
it passes for anything — it is never the suppressor.

### The remaining 92-vs-91 residual (positional, from grind_diff.py)

* **+2, tail block layout.** Our mask exit emits
  `bnez v0,<cont> / nop / j <end> / move v0,zero`; target emits the
  un-inverted fall-through `beqz $v0,.L80081CFC / addu $v0,$zero,$zero`.
  GCC places the `j end` BEFORE the `ret = 1; while (a0 == 0)` tail block
  instead of after it. Return-vs-break spelling does not move it (two
  measurements, both 19/93).
* **-1, the 0x3C0000 materialisation.** `k` lands in `$a0`, so the scheduler
  fills the `bnez v1` delay slot with `lui a0,0x3c` (build idx 31) where
  target keeps a `nop` and emits `lui $v0,(0x3C0000>>16)` after the
  `D_800F19BC` store. Target's holder is `$v0`, freed by the preceding `slt`.

## Session 2 caveat to carry forward

The `k` reuse is the [[defeat-licm-hoist-var-reuse]] / SOTN "variable reuse
for codegen control" family. It has NOT been through `cheat-reviewer`. Before
any completion claim on a form containing it, run layer-1 + layer-2 review,
and prefer a more natural two-set spelling if one can be found (`v0` as the
holder was tried and regresses to 22/93 — banked in `rejected/`).

- [s1] Cluster B has not yet been re-measured on top of a correct cluster-A allocation; the arg-block address chains index off $s0 in target and off $s1 in our build, so part of the $2<->$3 / $2<->$4 rename cluster may be downstream of the callee-save rotation rather than an independent scheduling problem.

- [s2] [s2] The allocation order is a closed-form prediction from a cc1 -da run: flow.c:dump_flow_info prints 'Register N used R times across L insns' into system.i.flow (pre local_alloc) and system.i.lreg (post local_alloc); feeding the .lreg values through global.c:allocno_compare's pri = floor_log2(n_refs)*n_refs/live_length*10000, ties broken by ascending pseudo number, reproduced the .greg ';; regs to allocate' order exactly on all 10 variants measured. No instrumented cc1 build is required.

- [s2] [s2] local-alloc.c:update_equiv_regs line 1064 doubles reg_live_length for any pseudo whose set carries a REG_EQUIV note. The three table pointers are (set (reg) (symbol_ref)) with REG_EQUIV and are scored on 96/98/100 (raw 48/49/50); the param's (set (reg/v 72) (reg:SI 4 a0)) has no REG_EQUIV and stays at 52. That single doubling is the entire cause of the callee-save rotation.

- [s2] [s2] Because of that doubling, P(idx_1494) > P(param) requires raw_param > 65, which exceeds the function's total live span (~52). NO goto-loop form can reach target's allocation order — proved arithmetically and confirmed across seven placement variants whose param live_length never left 52-55.

- [s2] [s2] loop.c:scan_loop builds a movable only when n_times_set[regno] == 1 or consec_sets_invariant_p succeeds. Writing BOTH loop-invariant compare constants (0x3C0000 and 0x1000000) through one reused local gives that pseudo two non-consecutive sets, so neither constant becomes a movable and move_movables cannot hoist them — the pure-C lever that makes the real-loop form affordable.

- [s2] [s2] Three loop.c suppression routes measured/read dead, so no future session re-derives them: (a) the reg_single_usage substitution at loop.c:735-768 requires validate_replace_rtx to yield a recognizable insn, and MIPS has no 32-bit-immediate slt or and, so it always fails; (b) the maybe_never / reg_in_basic_block_p safety gate at loop.c:695-701 never fires for a compiler temp because clause (2) '! REG_USERVAR_P && ! REG_LOOP_TEST_P' is unconditionally true for one; (c) the move_movables savings test at loop.c:1631 has threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) ~= 61 here, so it passes for everything and is never the suppressor.

- [s2] [s2] The session-2 candidate (real do-while loop + shared ret + one reused scratch k) produces target's exact callee-save map — $s0 = D_800A125C, $s1 = &D_800A1494, $s2 = the a0 param, $s3 = D_800A11DC — with only $s0-$s3 allocated and both compare constants materialised inline. Score 18 at 92 build insns vs 91 target.

- [s2] [s2] The entire remaining 92-vs-91 residual is two items: +2 from tail block layout (our mask exit is 'bnez v0,<cont> / nop / j <end> / move v0,zero' where target has the un-inverted fall-through 'beqz $v0,.L80081CFC / addu $v0,$zero,$zero'), and -1 from the 0x3C0000 holder landing in $a0 so the scheduler fills the bnez delay slot with 'lui a0,0x3c' at build idx 31 where target keeps a nop and emits 'lui $v0,(0x3C0000>>16)' after the D_800F19BC store.

- [s2] [s2] The k reuse belongs to the [[defeat-licm-hoist-var-reuse]] / SOTN 'variable reuse for codegen control' family and has NOT been through cheat-reviewer. Any completion claim on a form containing it must clear layer-1 and layer-2 review first; a more natural two-set spelling is preferred if one can be found (using v0 as the holder was tried and regresses to 22/93).

## Session 3 (structural) � banked facts

* **Floor unchanged at 18** (92 build insns vs 91 target). 13 further
  structural variants measured this session; none beat the session-2
  candidate, which is re-adopted unchanged.
* **The entire residual is the tail.** `grind_diff.py` on the candidate
  shows the prologue, the loop head, the timeout block, the debug_printf
  argument block and the flag exit all line up; the only differences are
  (a) the mask exit's branch sense / block layout (+2) and (b) `lui a0,0x3c`
  filling a delay slot at build idx 31 where target keeps a `nop` (-1).
* **jump.c:1764 is the gate on the mask exit's branch sense.** The
  "conditional jump jumping over an unconditional jump" inversion requires
  `prev_active_insn (reallabelprev) == insn`; a compound if-body puts the
  `ret = 0;` set in between and blocks it. A BARE `goto` body makes the
  build emit target's exact `beqz $v0,<exit>`.
* **Exit spelling and out-of-loop label order are exhausted.** Eight goto /
  if-else / continue variants: every goto form emits a byte-identical tail
  at 19/93 regardless of label order (GCC normalises the block order); the
  if-else and `continue` spellings collapse back to the candidate's exact
  18/92 output. The remaining +2 is one reorg `fill_slots_from_thread`
  decision, not a C-level control-flow choice.
* **reorg is asymmetric between the two exits of this function.** In the
  goto form it fully consumes the `return -1;` block (constant into the
  `bnez` delay slot, label -> NOTE_INSN_DELETED_LABEL, branch redirected to
  the shared return label 218) but refuses the identical steal for the
  `return 0;` block (`insn 208 (set (reg/i:SI 2 v0) (const_int 0))`,
  followed directly by `code_label 218`). RTL captured in
  `tmp/grind/saEft01Init/s3/dbr_w1_goto_exits/system.i.dbr`.
* **Target materialises the two loop constants in TWO DIFFERENT hard
  registers** � `lui $v0,(0x3C0000>>16)` at asm idx 41 and
  `lui $v1,(0x1000000>>16)` at idx 83. One C variable is one pseudo and one
  hard register (ours puts both in `$a0`), so the candidate's single reused
  `k` is provably NOT the original spelling. It stays the best-scoring form
  but is a synthetic LICM defeat pending cheat-review.
* **The real LICM gate is loop.c:695, not only loop.c:702.** A movable is
  skipped when all of (A) `! maybe_never && ! loop_reg_used_before_p`,
  (B) `! REG_USERVAR_P && ! REG_LOOP_TEST_P`, (C) `reg_in_basic_block_p`
  are false. So a USER local whose live range crosses a branch, set where
  `maybe_never` is already 1, stays inline in its own pseudo with no
  double-set. Measured: this works for the 0x1000000 mask constant
  (27/97 vs the 30/98 both-hoisted baseline) but not for 0x3C0000 �
  `maybe_never` is still 0 that early in the loop body (loop.c:930 sets it
  at the first in-loop CODE_LABEL/JUMP_INSN), and moving the set later
  makes it basic-block-local.
* **Any single-set constant local in this loop is hoisted** (x2/x3/y2 all
  30/98), including one written as a plain literal.
* **The exit flag `v0` cannot hold a constant in any combination**: v0 for
  both (s2 H8) 22/93; v0 for the timeout plus a spanning mask local 22/94;
  v0 for the timeout plus a literal mask 29/95.
* **Tooling:** `BB2_DBR_DEBUG=1` produces NOTHING � the shipped
  `tools/gcc-2.7.2/build/cc1` predates reorg.c's DBRDBG instrumentation,
  exactly as session 2 found for `BB2_ALLOC_DEBUG`. Read the `-da` `.dbr`
  RTL dump instead; `tmp/grind/saEft01Init/s3/dbrscan.py` extracts the
  saEft01Init tail with insn UIDs and correlates it with any DBRDBG lines.
  Harness: `s3/score.py <variant>...` (splice + sandbox, auto-restores
  src), `s3/dd.sh <variant>` (splice + positional diff), `s3/dbr.sh
  <variant>` (splice + cc1 -da + dbrscan).

- [s3] Floor unchanged at 18 (92 build insns vs 91 target); 13 structural variants measured, none beat the session-2 candidate, which is re-adopted unchanged.

- [s3] grind_diff.py on the candidate shows prologue, loop head, timeout block, debug_printf argument block and the flag exit all aligned; the only residual is the mask exit's branch sense/block layout (+2) and `lui a0,0x3c` filling a delay slot at build idx 31 where target keeps a nop (-1).

- [s3] jump.c:1764 requires prev_active_insn(unconditional jump) == the conditional jump for the inversion; a bare `goto` if-body unblocks it and reproduces target's `beqz $v0,<exit>` exactly.

- [s3] All eight goto/if-else/continue exit variants converge on two outputs only: 19/93 (any goto form, byte-identical tails, label order irrelevant) or 18/92 (the candidate's output, which if-else and `continue` spellings collapse back into).

- [s3] reorg is asymmetric between this function's two exits: it fully consumes the `return -1;` block (constant into the bnez delay slot, label becomes NOTE_INSN_DELETED_LABEL, branch redirected to the shared return label 218) but refuses the identical steal for `insn 208 (set (reg/i:SI 2 v0) (const_int 0))`, which is followed directly by code_label 218. RTL captured in tmp/grind/saEft01Init/s3/dbr_w1_goto_exits/system.i.dbr.

- [s3] Target holds the two loop constants in two different hard registers (lui $v0,0x3c at asm idx 41; lui $v1,0x100 at idx 83) while our build puts both in $a0 — so the candidate's single reused `k` local is provably not the original spelling.

- [s3] The real LICM gate is loop.c:695 (three OR-ed branches A/B/C), not only loop.c:702: a user local whose live range crosses a branch and whose set sits where maybe_never is already 1 stays inline in its own pseudo with no double-set. Measured working for the 0x1000000 mask constant (27/97 vs the 30/98 both-hoisted baseline).

- [s3] It does NOT work for 0x3C0000: maybe_never is still 0 that early in the loop body (loop.c:930 sets it at the first in-loop CODE_LABEL/JUMP_INSN), and moving the set later makes it basic-block-local so reg_in_basic_block_p holds.

- [s3] Any single-set constant local in this loop is hoisted — x2, x3 and y2 are all 30/98, including a plain literal spelling.

- [s3] The exit flag `v0` cannot hold a constant in any combination: both constants 22/93 (s2), timeout-only plus spanning mask local 22/94, timeout-only plus literal mask 29/95.

- [s3] BB2_DBR_DEBUG=1 produces no output — the shipped cc1 predates reorg.c's DBRDBG instrumentation, same as BB2_ALLOC_DEBUG in session 2; use the -da .dbr dump.

- [s3] src/system.c was left byte-identical to its session-start state (score.py restores it after every splice); no build-pipeline file was touched.

- [s4] Honest floor moved 18 -> 7 (`sandbox saEft01Init --disable all`), flat across sessions 1-3 before this. Build instruction count is now 91, EXACTLY target's 91, so the entire remaining residual is register choice and scheduling — no missing or extra instructions.

- [s4] Ladder of measured forms, all on the session-3 chassis: 18/92 (session-3 candidate) -> 11/93 (`cnt = k;` mask staging) -> 9/91 (+ `ret = *D_800A14C0 & cnt;`) -> 7/91 (+ `tbl_125c = &tbl_125c[idx_1494[0]]`).

- [s4] The mask-staging lever ONLY works through an already-live local: staging through a fresh `s32 m` is byte-identical to not doing it at all (18/92), through `ret` it is 15/94, through the loop counter `cnt` it is 11/93. That asymmetry places the construct squarely in the [[defeat-licm-hoist-var-reuse]] / [[staged-value-reused-variable]] family — it is NOT clean pure C by default and needs a /* FAKE */ annotation, the sessions 1-4 lever-exhaustion record, and cheat-reviewer sign-off before any completion claim.

- [s4] The argument block (cluster B, open since session 1 and untouched by sessions 2-3) was a real part of the residual, not downstream of the callee-save rotation: the base-vs-target disassembly diff shows six differing instructions in it (`lbu v1,1(s1)/lbu v0,0(s1)` vs target's `lbu a0,0(s1)/lbu v0,1(s1)`, plus a different address-chain order). The pointer re-base closes 2 of the remaining distance there.

- [s4] Permuter weighted score does NOT track the engine's honest distance for this function: campaign-1 finds at weighted 1235 screened anywhere from 13 to 72 on the sandbox, and the weighted-955 find screened to 17 while the weighted-1235-5 find screened to 13. Any future permuter session on saEft01Init must screen every find through the sandbox; ranking by permuter score would have discarded the best leads.

- [s4] Two campaigns ran and BOTH were harvested with --stop before session end (ws2: 850s, 24055 iterations, 19 new finds harvested at stop; ws3: reseeded from the sandbox-11 a1 chassis, best weighted find 660 which screened to sandbox 9). No permuter process survives the session (pgrep clean).

- [s4] Workspace-construction facts worth reusing: decomp-permuter's pycparser front end cannot parse this project's preprocessed TUs (K&R definitions like `inline int ENCODE_BCD(n)`, plus file-scope multi-line __asm__ blocks whose string literals cpp splits across lines). tmp/grind/saEft01Init/s4/trim.py drops every top-level chunk that is not a declaration and not the function under study, and mkws2.sh VERIFIES the trimmed base compiles byte-identically to the full-TU compile before the campaign is allowed to use it.

- [s4] Also worth reusing: the maspsx output for this project sets `.set at` at file scope and `.set noreorder` per function, so a single-function permuter workspace must prepend `.set noreorder` + `.set at` (NOT the decomp-permuter prelude's `.set noat`, which makes every `la` pseudo-op fail to assemble), and must extract from `.ent <func>` through `.end <func>` so `.frame`/`.mask` sit inside an `.ent` scope.

- [s5] **The session-4 floor of 7 does not stand as a matching form.** Its
  third lever re-bases `tbl_125c` onto `idx_1494[0]` and then indexes the
  re-based pointer with `idx_1494[1]`, so `arg5` becomes `tbl[i0+i1]` where
  target passes `tbl[i1]`; the emitted `addu s0,s0,v1` is an instruction
  target does not contain. Target computes BOTH index chains off the
  unmodified base register `s0`. The honest floor for a semantically faithful
  form is 8 / 91, and that is what `candidate.c` now holds.

- [s5] The debug_printf argument block (session-1 cluster B / session-2 F6) is
  now CLOSED as far as source-level ordering goes: the assignment STATEMENT
  order is the only lever (idx[0]'s assignment first = 8, idx[1]'s first = 9)
  and the DECLARATION order of the two intermediates is byte-inert on the
  correct callee-save allocation. Session 1's 18-vs-18 tie was an artefact of
  the wrong allocation and is superseded.

- [s5] Only ONE named intermediate is needed for the 8 / 91 form: `arg4`
  named, `tbl_125c[idx_1494[1]]` written inline as the fifth call argument
  (variant d3). The mirror image (`arg5` named, fourth argument inline) costs
  6 points (14 / 91) in all three spellings measured.

- [s5] The whole remaining residual of the 8 / 91 form is TWO clusters and
  nothing else: (1) the argument block's fourth-argument chain — our build
  runs it through `v1` and issues `lw a3,0(v1)` early, target runs it through
  `a0` and defers `lw a3,0(a0)` to after the `sw v1,16(sp)` / `lw a2` pair,
  and the `D_800F19C0` load moves with it; (2) the known session-3 F7 tail,
  where target fills the `beqz $v0` delay slot with `move v0,zero` and our
  build emits a `nop`.

- [s5] Permuter campaign hygiene for this function: a chassis whose base the
  permuter scores oddly high (the distance-7 form reported base_score 9000
  against a real weighted diff nearer 650) writes EVERY mutant to disk as a
  "better score" find — thousands of dirs, mostly textual duplicates.
  `tmp/grind/saEft01Init/s5/pick.py` dedupes by the whitespace-normalised
  function body and emits a lowest-N + stratified-sample screening list;
  `run_screen.sh` drives it end to end. Use it before any --all screen.

- [s5] The session-4 floor of 7 does not stand as a matching form: its pointer re-base mutates the table base (addu s0,s0,v1 — not in target) and changes arg5 from tbl[i1] to tbl[i0+i1]. Target computes both index chains off the unmodified s0. The honest floor for a semantically faithful form is 8/91, and memory/grind/saEft01Init/candidate.c now holds that form; the 7 is banked at rejected/pointer-rebase-changes-arg5-semantics-7.c with the target disassembly that disproves it.

- [s5] The debug_printf argument block (session-1 cluster B / session-2 F6) is closed at the source-ordering level: assignment STATEMENT order is the only lever (idx[0]'s lookup assigned first = 8, idx[1]'s first = 9) and the DECLARATION order of the intermediates is byte-inert on the correct callee-save allocation.

- [s5] Only ONE named intermediate is needed for 8/91: arg4 named, tbl_125c[idx_1494[1]] written inline as the fifth call argument. The mirror image (arg5 named, fourth argument inline) costs 6 points in all three spellings measured (block-scope, decl-with-initialiser, function-scope) and all three are byte-identical.

- [s5] The whole residual of the 8/91 form is exactly two clusters: (1) the fourth argument's address chain — ours runs through $v1 and issues `lw a3,0(v1)` early, target runs it through $a0 and defers `lw a3,0(a0)` past the `sw v1,16(sp)` / `lw a2,0(v0)` pair, with the D_800F19C0 load moving with it; (2) the known session-3 F7 tail, where target fills the `beqz $v0` delay slot with `move v0,zero` and our build emits a nop.

- [s5] Permuter campaign hygiene for this function: a chassis the permuter scores oddly high at base (the d7 form reported base_score 9000 against a real weighted diff nearer 650) writes EVERY mutant to disk as a 'better score' find — 2647 dirs, mostly textual duplicates. tmp/grind/saEft01Init/s5/pick.py dedupes by whitespace-normalised function body and emits a lowest-N + stratified-sample screening list; run_screen.sh drives it end to end. Use it before any --all screen.

- [s5] Both session-5 campaigns (d7-chassis 14045 iterations, d8-faithful-chassis 29684 iterations) were harvested with --stop inside the session; `permuter_campaign.py status` shows both dead and inactive.

## Session 6 (forensics) — the argument block's shape is decided at EXPAND, its order at SCHED1

Floor unchanged at **8 / 91** (`sandbox saEft01Init --disable all` on the
inherited candidate, re-measured this session). Ten variants measured; the
whole session's value is mechanism, not score.

### Tooling note (closes a standing question)

`tools/gcc-2.7.2/sched.c:2435` **already carries a `BB2_RANK_DEBUG`
instrumentation block** (prints `RANKDBG last=<uid> y=<uid> cls=… x=<uid>
cls2=… val=…` from `rank_for_schedule`), and there is an analogous
`BB2_DBR_DEBUG` in `reorg.c`. **Neither is in the shipped binary** —
`strings tools/gcc-2.7.2/build/cc1 | grep -E "RANKDBG|DBRDBG|ALLOCDBG"` is
empty. The instrumented-cc1 modality therefore cannot be run without
rebuilding `tools/gcc-2.7.2/build/cc1`, which is outside a grind session's
allowed surface. **All cc1 forensics on this function must be done by reading
the `-da` dumps.** Harness this session:
`tmp/grind/saEft01Init/s6/dump.sh <tag>` (full `-da` dump set for the CURRENT
`src/system.c` into `s6/<tag>/`), `s6/x.py <tag> <stage> [--from RE]
[--grep RE]` (extract the saEft01Init section of one dump), `s6/order.py
<tag> <stage>` (compact one-line-per-insn order of the `debug_printf` block).

### The fourth argument: named local vs inline is an EXPAND-time difference

Read from the `.rtl` (raw expand output, before every optimisation pass):

* **arg4 written INLINE** in the call (variant `n0`): expand emits
  `(insn 129 (set (reg:SI 7 a3) (mem/s:SI (reg:SI 99))))` — the load goes
  **directly into the hard argument register**, as the last insn of the
  argument sequence, after the stacked-argument store. There is no separate
  load insn at all. This is `expand_call`'s argument-loading phase passing the
  hard reg as the target of `expand_expr`; **combine is not involved**
  (checked: the `REG_USERVAR_P` guards in `combine.c:1279/1134` are
  `SMALL_REGISTER_CLASSES`-gated and `config/mips/mips.h` does not define it,
  so they are dead on this target).
* **arg4 written as a NAMED local** (the inherited candidate): expand emits
  `(insn 101 (set (reg/v:SI 88) (mem/s:SI (reg:SI 93))))` at the *statement's*
  position plus `(insn 134 (set (reg:SI 7 a3) (reg/v:SI 88)))`. `local_alloc`
  then gives pseudo 88 the hard reg `$a3` by copy preference and the move
  disappears — so the LOAD itself ends up written to `$a3` at the EARLY
  position. That is exactly the observed `lw a3,0(v0)` at build idx 55.

**Target's block ends with `lw a3,0(a0)` as its last memory reference**, i.e.
target has the EXPAND shape of the INLINE fourth argument, not of a named
local. So the inherited 8/91 candidate reaches its score with a structurally
WRONG fourth argument, and the structurally right one (`n0`) scores 14/91.
The two are separate basins, not neighbours.

### The block's ORDER is fixed by sched1's priority + dependence-class rules, not by source order

`n0` (arg4+arg5 both inline), `n2` (`i0 = idx_1494[0];` as a statement, both
lookups inline), `n3` (both indices named as statements) and `n5` (arg5 named
+ i0 named) produce **byte-identical assembly** (md5 of the `.s` body equal).
Their `.rtl`/`.combine` LUID orders are genuinely different — in `n2` the
`idx_1494[0]` `lbu` is insn **94, the first insn of the block**, in `n0` it is
insn **104, the third chain** — and `.sched` (the pre-reload scheduler's
output) is **the same order in both**, with that `lbu` at position 9 of 16.

That inverts the LUID relation between the `idx_1494[0]` `lbu` and the
`D_800A11D5` `lbu` (n0: 104 vs 96; n2: 94 vs 102) without changing the
schedule, which **proves the LUID tie-break at `sched.c:2452-2455` is not what
orders these chains**. The deciding rule is upstream of it in
`rank_for_schedule`: `INSN_PRIORITY` (longest path to the block end) first,
then the dependence-class test at `sched.c:2412-2449` (class 3 = independent
of `last_scheduled_insn` beats class 1 = data-dependent). Both are functions of
the dependency DAG alone, and the DAG is fixed by the call's semantics.

**Consequence:** the session-5 frontier's first next-probe — "sweep spellings
that split the fourth argument's chain across two statements, varying the
intervening statements" — is CLOSED. Statement placement inside this basic
block cannot move the argument block, whatever the spelling.

### Why target's chain lands in $a0 and ours in $v0/$v1

The chain register is downstream of sched1, not an allocator preference:
sched1's output order fixes the live ranges, `local_alloc` then fills them in
`reg_alloc_order`. In target the `idx_1494[0]` address temp is live from the
FIRST insn of the block to the LAST (`lbu a0,0(s1)` … `lw a3,0(a0)`), so
`$v0` and `$v1` are both busy across it and it lands in `$a0` — and that in
turn forces `lui a0,%hi(D_800161C8)` after it (anti-dependence), which is
exactly target's tail. In our builds the same temp is short-lived (candidate:
positions 1-7 of the block; n0: positions 9-15), so it gets `$v0`.
Matching therefore requires sched1 to schedule the `idx_1494[0]` `lbu` FIRST
and its load LAST, which requires that chain to outrank the `D_800A11D5` chain
in `INSN_PRIORITY` — the two chains are currently isomorphic
(`lbu → sll → addu → lw <hardreg>`), so they tie and the class rule breaks the
tie against us.

- [s6] Floor re-measured and unchanged at 8/91 on the inherited candidate; ten variants measured this session (n0-n9), none below 8.

- [s6] tools/gcc-2.7.2/sched.c:2435 already contains a BB2_RANK_DEBUG env-gated fprintf inside rank_for_schedule, but the shipped tools/gcc-2.7.2/build/cc1 does not contain the string (strings | grep -E "RANKDBG|DBRDBG|ALLOCDBG" is empty), so the instrumented-cc1 modality is unavailable without rebuilding cc1 — outside a grind session's allowed surface. Use the -da dumps.

- [s6] EXPAND-time fact from the .rtl dump: an INLINE call argument's final load is emitted directly into the hard argument register at the END of the argument sequence ((set (reg:SI 7 a3) (mem/s:SI (reg 99)))), while a NAMED local's load is emitted at its own statement position and followed by a register copy to a3, which local_alloc then coalesces by giving the pseudo $a3 — so the load itself moves early. combine plays no part: the REG_USERVAR_P guards at combine.c:1279/1134 are SMALL_REGISTER_CLASSES-gated and config/mips/mips.h does not define it.

- [s6] Target's arg block ends with `lw a3,0(a0)` as its last memory reference, which is the EXPAND shape of an INLINE fourth argument. The inherited 8/91 candidate (arg4 named) therefore has a structurally wrong fourth argument, and the structurally right form (arg4 inline) is a different basin scoring 14/91.

- [s6] n0 (both lookups inline), n2 (index 0 named as a statement, both lookups inline), n3 (both indices named) and n5 (arg5 named + index 0 named) are BYTE-IDENTICAL (md5 of the .s function body), and their .sched (pre-reload) orders are identical, despite .rtl LUID orders that invert the relative position of the idx_1494[0] lbu and the D_800A11D5 lbu (n0: 104 vs 96, n2: 94 vs 102).

- [s6] That LUID inversion with no schedule change proves the tie-break at sched.c:2452-2455 (INSN_LUID, original order) is NOT what orders the argument block's chains; the deciding rules are INSN_PRIORITY and then the dependence-class test at sched.c:2412-2449, both functions of the dependency DAG alone. Source-level statement placement inside this basic block is therefore byte-inert, closing the session-5 frontier probe that proposed sweeping split-chain spellings.

- [s6] The chain's hard register is downstream of sched1: target's idx_1494[0] address temp is live from the block's first insn to its last, so $v0 and $v1 are busy across it and local_alloc lands it in $a0 (which then forces `lui a0,%hi(D_800161C8)` after it by anti-dependence — target's exact tail). Our temp is short-lived in both basins (candidate positions 1-7, n0 positions 9-15) and gets $v0.

- [s6] Matching the arg block needs the idx_1494[0] chain to OUTRANK the D_800A11D5 chain in INSN_PRIORITY; the two are currently isomorphic (lbu -> sll -> addu -> lw <hardreg>) and tie, and the tie breaks against us on the dependence-class rule.

- [s6] Measured variants, all at 91 build insns unless noted: n0 both-inline 14, n1 (i0 named + arg4 named) 8, n2 (i0 named, lookups inline) 14, n3 (both indices named, lookups inline) 14, n4 (arg2 named, lookups inline) 13, n5 (arg5 named + i0 named) 14, n6 (arg2 named then arg4 named) 15 at 90 insns, n7 (arg4 named then arg2 named) 14 at 90 insns, n8 (D_800F19C0 staged in a local + arg4 named) 8, n9 (arg4 and arg5 both named) 8. Naming arg2 changes the instruction COUNT (90), so it is not a pure scheduling lever.

- [s6] Floor re-measured and unchanged at 8/91 with the inherited candidate applied to src/system.c; ten variants measured this session (n0-n9), none below 8.

- [s6] EXPAND-time fact (.rtl dump): an INLINE call argument's final load is emitted directly into the hard argument register as the LAST insn of the argument sequence — (set (reg:SI 7 a3) (mem/s:SI (reg:SI 99))) — while a NAMED local's load is emitted at its statement position and followed by a copy to a3 that local_alloc coalesces by giving the pseudo $a3, moving the LOAD early. combine plays no part; its REG_USERVAR_P guards at combine.c:1279/1134 are SMALL_REGISTER_CLASSES-gated and config/mips/mips.h does not define that macro.

- [s6] Target's argument block ends with `lw a3,0(a0)` as its last memory reference, which is the expand shape of an INLINE fourth argument — so the 8/91 candidate's named arg4 is structurally wrong, and the structurally right form is a separate basin at 14/91 (banked at rejected/arg4-inline-is-target-expand-shape-but-14.c).

- [s6] n0 (both lookups inline), n2 (index 0 named as a statement), n3 (both indices named) and n5 (arg5 named + index 0 named) emit BYTE-IDENTICAL assembly (md5 5e95422b of the .s function body) and identical .sched orders, despite .rtl LUID orders that invert the relative position of the idx_1494[0] lbu and the D_800A11D5 lbu (n0: 104 vs 96; n2: 94 vs 102).

- [s6] That LUID inversion with no schedule change proves the INSN_LUID tie-break at sched.c:2452-2455 is NOT what orders this block; INSN_PRIORITY and the dependence-class test at sched.c:2412-2449 are, and both are functions of the dependency DAG alone. Source statement placement inside this basic block is therefore byte-inert.

- [s6] The argument block's hard registers are downstream of sched1: target's idx_1494[0] address temp is live across the whole block so $v0/$v1 are busy and local_alloc lands it in $a0, which forces `lui a0,%hi(D_800161C8)` after it by anti-dependence; ours is short-lived in both basins (candidate positions 1-7, n0 positions 9-15) and gets $v0.

- [s6] Matching the argument block reduces to ONE scheduler fact: the idx_1494[0] chain must outrank the D_800A11D5 chain in INSN_PRIORITY at sched1. The two chains are currently isomorphic (lbu -> sll -> addu -> lw <hardreg>), they tie, and the tie breaks against us on the dependence-class rule.

- [s6] Variant scores, all 91 build insns unless noted: n0 both-inline 14, n1 (i0 named + arg4 named) 8, n2 14, n3 14, n4 (arg2 named, lookups inline) 13, n5 14, n6 (arg2 then arg4 named) 15 at 90 insns, n7 (arg4 then arg2 named) 14 at 90 insns, n8 (D_800F19C0 staged in a local + arg4 named) 8, n9 (arg4 and arg5 both named) 8.

- [s6] tools/gcc-2.7.2/sched.c:2435 already contains a BB2_RANK_DEBUG env-gated fprintf in rank_for_schedule, but the shipped cc1 binary contains none of RANKDBG/DBRDBG/ALLOCDBG, so the instrumented-cc1 route is closed without rebuilding the compiler.

- [s6] src/system.c holds the inherited 8/91 candidate at session end (re-verified: sandbox score 8, build_insns 91); no build-pipeline file was touched and no campaign or background process was started this session.


---

# Session 7 additions

## [inherited] salvaged write-up of the discarded 15:03-15:18 session-7 attempt

## Session 7 (forensics) — the instrumented cc1 EXISTS, and it rewrites the argument-block model

Floor unchanged at **8 / 91** (re-measured on the inherited candidate with the
edits in place in `src/system.c`). Nine new variants measured. The session's
value is a tooling unlock plus a corrected, now-numerically-verified model of
how sched1 orders the `debug_printf` argument block.

#### THE TOOLING UNLOCK — `tools/gcc-2.7.2/cc1` is the instrumented binary

Sessions 2, 3 and 6 each concluded that the instrumented-cc1 modality was
unavailable because `strings tools/gcc-2.7.2/build/cc1` contains none of
ALLOCDBG / DBRDBG / RANKDBG. **They checked the wrong file.**
`tools/gcc-2.7.2/build/` holds only the shipped driver binaries; the compiler
tree itself is configured in place and carries a SECOND, NEWER cc1 at
`tools/gcc-2.7.2/cc1` (built 2026-07-18, after the 2026-07-03/04 source edits
that added the hooks). That binary contains ALL of them:

```
BB2_ALLOC_DEBUG   BB2_PRIO_DEBUG     BB2_RANK_DEBUG    BB2_SCHED_DEBUG
BB2_DBR_DEBUG     BB2_FINDREG_DEBUG  BB2_FLOW_DEBUG    BB2_QTY_DEBUG
BB2_SLL_DEBUG     BB2_XJUMP_DEBUG    BB2_NO_FT_STEAL   BB2_ALLLIVE_LABEL
ALLOCDBG ord=%d pseudo=%d hardreg=%d nrefs=%d livelen=%d pri=%d
PRIODBG insn=%d pred=%d kind=%d pred_pri=%d cost=%d contrib=%d (max=%d)
PRIODBG SET insn=%d final_pri=%d
RANKDBG last=%d y=%d cls=%d x=%d cls2=%d val=%d
DBRDBG thr / simp / mtlr ...
```

**It is codegen-identical to the frozen `build/cc1`**: `s7/idump.sh` compiles
the same `system.i` with BOTH binaries and diffs the `.s` (ignoring the
`# options` comment line) on every run, printing
`CODEGEN-IDENTICAL: instrumented cc1 == shipped build/cc1 on this TU`. It is
therefore a pure diagnostic instrument, not a compiler divergence — the actual
build pipeline still uses `build/cc1` and is untouched. Use
`bash tmp/grind/saEft01Init/s7/idump.sh <tag>` (writes the full `-da` dump set
PLUS `cc1.log` with the traces into `tmp/grind/saEft01Init/s7/<tag>/`); it
re-runs the identity check every time, so a future compiler change cannot
silently invalidate the readings. **This unlocks the instrumented modality for
every function in the project, not just this one.**

#### sched1 is a REVERSE list scheduler and its priorities are now MEASURED

`INSN_PRIORITY` in this compiler is the weighted longest path from the BLOCK
START (not to the block end), and `schedule_block` runs bottom-up, so the
highest-priority insn is placed LATEST in the block. Costs come from
`config/mips/mips.md`'s `define_function_unit "memory"`: load = 2 (the
`r3000` alternative, selected by `-mcpu=3000`), store = 1, everything else 1.

Measured priorities for the whole argument block of the candidate
(`s7/icand/cc1.log`, the sched1 `PRIODBG SET` window; UIDs are the `.rtl`
UIDs, and the second identical-UID window later in the log is sched2):

| insn | what | pri |
|---|---|---|
| 94  | `lbu` idx_1494[0]  (chain X head) | 1 |
| 117 | `lbu` idx_1494[1]  (chain Z head) | 1 |
| 109 | `lbu` D_800A11D5   (chain Y head) | 1 |
| 106 | `lw`  D_800F19C0   ($a1)          | 1 |
| 128 | `la`  &D_800161C8  ($a0)          | 1 |
| 97 / 99 / 101 | X `sll` / `addu` / load   | 2 |
| 112 / 114     | Y `sll` / `addu`          | 2 |
| 120 / 122 / 124 | Z `sll` / `addu` / load | 2 |
| 130 | `$a1 =`                            | 2 |
| 126 | `sw` 16($sp) (the stacked arg)     | 3 |
| 132 | `$a2 =` (the load)                 | 3 |
| 134 | `$a3 =` (copy from the named arg4) | 3 |
| 136 | the call                           | 4 |

**All four chain heads tie at priority 1** — confirmed numerically, not
inferred. This closes session 6's open question about whether the two `lbu`
chains tie.

#### CORRECTION to session 6: the dependence-class rule is INERT here

Session 6 concluded "INSN_PRIORITY first, then the dependence-class test at
`sched.c:2412-2449` ... the class rule breaks the tie toward D_800A11D5. Only a
DAG change can separate them." **The RANKDBG trace falsifies the second half.**
Every single rank comparison inside this block prints `cls=3 ... cls2=3 val=0`:

```
RANKDBG last=136 y=134 cls=3 x=130 cls2=3 val=0
RANKDBG last=126 y=109 cls=3 x=101 cls2=3 val=0
RANKDBG last=128 y=124 cls=3 x=109 cls2=3 val=0
RANKDBG last=124 y=122 cls=3 x=101 cls2=3 val=0
RANKDBG last=101 y=99  cls=3 x=122 cls2=3 val=0
RANKDBG last=99  y=117 cls=3 x=97  cls2=3 val=0     (13 comparisons, all val=0)
```

Class 3 means "independent of `last_scheduled_insn`, or latency 1", and every
competitor in this block qualifies, so `tmp_class - tmp2_class` is always 0 and
control always falls through to the `INSN_LUID` tie-break at
`sched.c:2452-2455`. The ordering is therefore decided by **priority, then
READINESS in the reverse pass, then source (LUID) order** — never by the class
rule.

#### Why LUID nevertheless looked inert in session 6: READINESS COUPLING

`n2` names the index (`i0 = idx_1494[0];`) so chain X's `lbu` has the SMALLEST
LUID in the block (94), yet sched1 still places it 9th of 16. The reason is not
the tie-break: in a bottom-up schedule an insn only becomes READY once all of
its dependents are placed, so a chain is dragged toward the block END as a unit
as soon as its terminal insn is placed there. That coupling is visible in every
form measured to date — **the order of the three chains' HEADS is always the
same as the order of their LOADS**:

| form | head order | load order | score |
|---|---|---|---|
| candidate (arg4 named) | X, Z, Y | X, Z, Y | 8 |
| n4 (arg2 named)        | Y, Z, X | Y, Z, X | 13 |
| a5n / n0 (arg4 inline) | Z, Y, X | Z, Y, X | 14 |
| **target**             | **X, Z, Y** | **Z, Y, X** | — |

(X = `tbl_125c[idx_1494[0]]`, the fourth argument; Y = `tbl_11dc[D_800A11D5]`,
the third; Z = `tbl_125c[idx_1494[1]]`, the stacked fifth.)

Target is the only arrangement that DECOUPLES them: chain X's `lbu` is the
first insn of the block and its `lw $a3` the last. So the real question is not
"which chain outranks which" (they tie, and the class rule cannot separate
them) but "what source spelling stretches chain X across the whole block".

#### The p4 basin — the first form that actually stretches chain X

Predicted from the model above and confirmed by measurement: write the fourth
argument's ADDRESS as a named pointer local and leave the DEREFERENCE inline —

```c
s32 *p4;
p4 = &tbl_125c[idx_1494[0]];
debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], *p4, tbl_125c[idx_1494[1]]);
```

The `lbu` / `sll` / `addu` are then expanded at the STATEMENT (low LUIDs, so
they schedule early) while `expand_call` still emits the load itself as
`(set (reg:SI 7 a3) (mem ...))`, the last insn of the argument sequence —
target's expand shape (session 6's finding). Measured: chain X's head lands at
block position 2 and its load at 13, against 1 and 9 for the candidate and
9-12 / 16 for the whole inline family. **Score 10 / 91.** Eight neighbours in
the same basin measured 10-14. Banked with the full comparison table at
`rejected/named-address-pointer-stretches-arg4-chain-but-10.c`.

It does not beat the 8, but it is the only measured basin whose argument block
has target's TOPOLOGY, and it is a better permuter seed than the n4 chassis the
session-6 frontier nominated (which is 13 and has the head in the wrong place).

- [s7-a] tools/gcc-2.7.2/cc1 (NOT tools/gcc-2.7.2/build/cc1, which sessions 2/3/6 checked) is a fully instrumented compiler built 2026-07-18 carrying BB2_PRIO_DEBUG, BB2_RANK_DEBUG, BB2_ALLOC_DEBUG, BB2_DBR_DEBUG, BB2_FLOW_DEBUG, BB2_QTY_DEBUG, BB2_FINDREG_DEBUG, BB2_SLL_DEBUG, BB2_XJUMP_DEBUG, BB2_SCHED_DEBUG and the DBRDBG/PRIODBG/RANKDBG/ALLOCDBG format strings. The instrumented-cc1 modality is AVAILABLE for every function in the project; the three prior "unavailable" verdicts are wrong.

- [s7-a] The instrumented cc1 is codegen-identical to the frozen build/cc1: tmp/grind/saEft01Init/s7/idump.sh compiles the same system.i with both and diffs the .s (ignoring the '# options' comment) on every invocation, printing CODEGEN-IDENTICAL. It is a diagnostic instrument only — the build pipeline still uses build/cc1 — so reading it is not a compiler divergence under [[no-compiler-divergence]].

- [s7-a] Measured (not inferred) sched1 INSN_PRIORITY for the whole debug_printf argument block of the candidate: all four chain heads (idx_1494[0] lbu, idx_1494[1] lbu, D_800A11D5 lbu, D_800F19C0 lw) and the &D_800161C8 la tie at 1; every sll/addu/intermediate load is 2; the 16(sp) store, the $a2 load and the $a3 copy are 3; the call is 4. Costs are load=2 (the mips.md r3000 memory unit, selected by -mcpu=3000), store=1, everything else 1, and INSN_PRIORITY is the weighted longest path from the BLOCK START because schedule_block is a bottom-up (reverse) list scheduler — the highest-priority insn is placed LATEST.

- [s7-a] CORRECTION to session 6: the dependence-class test at sched.c:2412-2449 never separates anything in this block. All 13 RANKDBG comparisons emitted for it print cls=3 cls2=3 val=0, because every competitor is independent of last_scheduled_insn or has latency 1. Ordering therefore falls through to INSN_LUID at sched.c:2452-2455 in every case, and the operative constraint above LUID is READINESS in the reverse pass, not the class rule.

- [s7-a] Readiness coupling explains why session 6 saw LUID as inert: in a bottom-up schedule a chain is dragged toward the block end as a unit once its terminal insn is placed, so in EVERY form measured so far the order of the three chains' HEADS equals the order of their LOADS (candidate X,Z,Y / X,Z,Y at 8; n4 Y,Z,X / Y,Z,X at 13; n0-a5n Z,Y,X / Z,Y,X at 14). Target is the only arrangement that decouples them — chain X's lbu first, its lw $a3 last — so the matching problem is 'stretch chain X across the block', not 'make chain X outrank chain Y'.

- [s7-a] Naming the fourth argument's ADDRESS while leaving the dereference inline (`p4 = &tbl_125c[idx_1494[0]]; debug_printf(..., *p4, ...)`) is the first measured form that decouples head from load: chain X's head lands at block position 2 and its load at 13 (candidate: 1 and 9; the whole inline family: 9-12 and 16; target: 1 and 16). It scores 10/91. Eight neighbours measured 10-14 (p4d 10 byte-identical to p4, p45 11, p45r 11, q1 10, q2 10, q3 10, q4 11, q5 14). Banked at rejected/named-address-pointer-stretches-arg4-chain-but-10.c.

- [s7-a] src/system.c was left holding the inherited 8/91 candidate (verified byte-identical to tmp/grind/saEft01Init/s7/system.c.candbase, sandbox score 8 at 91 build insns); no build-pipeline file was touched, no rule file was edited, and no background process was started.



## Session 7 (forensics) — the function is Sony's `CD_datasync` and the matched C is IN HAND

Floor unchanged at **8 / 91**. This session's value is provenance plus a
now-numerically-pinned answer to "which GCC pass produces the divergence".

### A note on this session's two halves

An earlier session-7 process ran on 2026-08-01 15:03-15:18 and died before
writing its outcome, so the driver discarded it; its scratch survived in
`tmp/grind/saEft01Init/s7/`. Its write-up is folded in above under
**[inherited]** and its facts are tagged `[s7-a]`. Of its claims this session
independently RE-VERIFIED only the tooling one — `grep -a` on
`tools/gcc-2.7.2/cc1` returns all 13 of ALLOCDBG / PRIODBG / RANKDBG / DBRDBG /
BB2_*_DEBUG, and `idump.sh` printed CODEGEN-IDENTICAL on every run this session
(that also matches the standing project memory `instrumented-cc1-location`).
Its sched1 priority table, its RANKDBG `cls=3` reading and its p4 basin numbers
were NOT re-measured here; treat them as strong leads, not banked facts.

### saEft01Init IS PsyQ LIBCD `CD_datasync`, and the matched C is now on disk

The queue name is an auto-generated misnomer. The matched reference C, the
full BB2→Sony symbol mapping, the sibling-function note and the session-7
measurement table are all in
**`memory/grind/saEft01Init/ref/sotn_libcd_bios_CD_datasync.c`** (source:
Xeeynamo/sotn-decomp `src/main/psxsdk/libcd/bios.c`, fetched with `gh api`;
full file cached at `tmp/grind/saEft01Init/s7/ref/sotn_bios.c`; five further
independent decomps of the same object are listed there if a second opinion is
ever wanted).

The identification does not rest on the census alone. `*D_800A14C0 &
0x1000000` is the DMA3 CHCR channel-busy bit; `0x3C0` is 960 vblanks;
`D_800A1494[0]`/`[1]` are the adjacent `sync`/`ready` bytes of Sony's
`CD_intr` struct; `tslTm2LoadImage_2` is `puts` and `cdrom_ClearIrq` is
`CD_flush`. Every element of the disassembly is accounted for by the reference,
statement for statement.

**Sibling payoff:** `set_alarm`/`get_alarm` are inlined into three functions in
this TU. `src/system.c` already carries two more of them in the same
hand-derived goto shape — `cpu_side_move_dir_4` (= `CD_sync`, name string
D_80016240, ~line 366) and the function at ~line 480 (= `CD_ready`, name string
D_80016248). Whatever spelling matches here matches those.

### What the reference settles: the inherited 8/91 is a wrong-basin optimum

The original body has **no named `arg4` intermediate** (all four table lookups
are written inline in the `printf` call — which session 6 had already deduced
independently from the expand-time RTL) and **no `k`** (both compare constants
are plain literals). Both of the levers the 8 is built on are absent from the
original. The 8 is therefore not a cheat-free 8, and a cheat-reviewer must be
shown the clean number alongside it.

### The honest cheat-free floor of this basin is 32 / 96

Measured this session by stripping every lever off the proven sessions-2/3
chassis and writing the statements exactly as the reference has them, then
adding the levers back one at a time:

| form | levers | score / insns |
|---|---|---|
| `clean` | none (reference statements) | **32 / 96** |
| `cleana` | + named `arg4` only | 27 / 96 |
| `cleank` | + the `k` double-set LICM defeat only | 21 / 90 |
| `candidate` | `k` + `cnt = k` staging + named `arg4` | 8 / 91 |

### The exact pass and decision, from the instrumented cc1

`ALLOCDBG` for `clean` (`tmp/grind/saEft01Init/s7/clean/cc1.log`, 8 allocnos):

```
ord=2 pseudo=78  hardreg=16 nrefs=5 livelen=96  pri=1041   tbl_125c  -> $s0
ord=3 pseudo=77  hardreg=17 nrefs=5 livelen=98  pri=1020   idx_1494  -> $s1
ord=4 pseudo=72  hardreg=18 nrefs=3 livelen=52  pri= 576   the param -> $s2
ord=5 pseudo=108 hardreg=19 nrefs=3 livelen=92  pri= 326   0x1000000 -> $s3
ord=6 pseudo=85  hardreg=20 nrefs=3 livelen=94  pri= 319   0x3C0000  -> $s4
ord=7 pseudo=76  hardreg=21 nrefs=3 livelen=100 pri= 300   tbl_11dc  -> $s5
```

**The first three dispositions are already target's, with no lever of any
kind.** The entire allocation defect is `loop.c`'s LICM (`scan_loop` /
`move_movables`) creating pseudos 85 and 108 for the two loop-invariant compare
constants: they rank 5th and 6th in `global_alloc` (priority 326 and 319
against `tbl_11dc`'s 300) and displace `tbl_11dc` from target's `$s3` to `$s5`,
adding a fifth and sixth callee-save (+5 insns, 96 vs 91). Nothing else in the
clean form's allocation is wrong.

`ALLOCDBG` for `cleank` (the same form plus only the double-set defeat)
confirms the payoff is exact and complete:

```
ord=1 pseudo=76 hardreg=4  nrefs=8 livelen=6   pri=40000  k         -> $a0
ord=3 pseudo=79 hardreg=16 nrefs=5 livelen=96  pri=1041   tbl_125c  -> $s0
ord=4 pseudo=78 hardreg=17 nrefs=5 livelen=98  pri=1020   idx_1494  -> $s1
ord=5 pseudo=72 hardreg=18 nrefs=3 livelen=52  pri= 576   the param -> $s2
ord=6 pseudo=77 hardreg=19 nrefs=3 livelen=100 pri= 300   tbl_11dc  -> $s3
```

Target's exact map, four callee-saves, both constants materialised inline, the
holder caller-saved — at 90 insns, one FEWER than target's 91.

**This reframes the whole function.** The clean-C matching problem is no longer
"the debug_printf argument block"; it is one question: a legitimate spelling
that denies `loop.c` those two specific hoists. Everything else in the clean
form is already right, or is scheduling.

### Reference-derived forms measured DEAD this session

* `r0` — the reference hand-inlined verbatim: **35 / 91** (hits target's exact
  instruction count, but five callee-saves and no hoisted base for `Intr`).
  `r1` (+ the three table-base locals) 37 / 93; `r2` (`set_alarm`/`get_alarm`
  as real `static __inline__` helpers) 31 / 94.
* **Sony's `volatile` on `Alarm` and `Intr`** — the one DAG-changing lever the
  session-6 frontier wanted, and legitimate as a header-type correction rather
  than a coercion. **Dead:** 8 → 16 on the candidate chassis, and literally no
  change on the reference chassis (35 → 35). BB2 links PsyQ 4.0 against sotn's
  3.5-era reference, so the shipped object was evidently not built with these
  volatile-qualified. Harness: `s7/vscore.py` (`--novol` to disable).
* **The `||` short-circuit timeout test** — isolated against an otherwise
  byte-for-byte identical candidate: 8 → **27 / 92**. `c2` (only the exits
  changed to `while (1) { ... break; }`, inner gotos kept) 61 / 129; `c4`
  (both) 74 / 131 — pathological block duplication, not near-misses.
* Conversely the reference CONFIRMED the candidate's tail scaffolding: target's
  `j / li v0,-1 / move v0,zero / bnez v0 / li v0,-1` at idx 68-72 is the
  inlined-`get_alarm()` return-value shape, i.e. the `v0` temp and the `check:`
  label ARE the original's structure, and `if (mode != 0) { ret = 1; break; }`
  is exactly target's `beqz $s2 / addiu $v0,$zero,1` back-edge.
* Two inherited "structural requirements" are now known to be artifacts of the
  goto chassis rather than facts about the function: with a real loop the table
  bases hoist WITHOUT explicit source-level locals (r0 is 91 insns, where
  session 1's H1 goto-chassis test was 85), and `Intr`'s base does NOT get a
  hoisted register even WITH an explicit pointer local.

- [s7] saEft01Init @0x80081BB0 is Sony PsyQ LIBCD `CD_datasync`, and the matched C is now on disk at memory/grind/saEft01Init/ref/sotn_libcd_bios_CD_datasync.c (from Xeeynamo/sotn-decomp src/main/psxsdk/libcd/bios.c, fetched via `gh api`; full file cached at tmp/grind/saEft01Init/s7/ref/sotn_bios.c). The identity is independently corroborated by the disassembly, not just the census: *D_800A14C0 & 0x1000000 is the DMA3 CHCR channel-busy bit, 0x3C0 is 960 vblanks, and D_800A1494[0]/[1] are the adjacent sync/ready bytes of Sony's CD_intr struct.

- [s7] Full BB2->Sony symbol mapping (all confirmed against the target disassembly): a0=mode; D_800F19B8/BC/C0 = Alarm.unk0/unk4/unk8 (deadline / spin counter / caller name pointer); D_800162C0 = "CD_datasync"; D_800161B8 = "CD timeout: "; D_800161C8 = the printf format; D_800A11DC[] = the CdlCom name table; D_800A11D5 = CD_com; D_800A125C[] = the interrupt-state name table; D_800A1494 = the CD_intr struct (sync at +0, ready at +1); *D_800A14C0 = DMA3 CHCR; sys_VSync=VSync; tslTm2LoadImage_2=puts; cdrom_ClearIrq=CD_flush; debug_printf=printf. tslTm2LoadImage_2 and cdrom_ClearIrq are splat misnomers like saEft01Init itself.

- [s7] set_alarm/get_alarm are inlined into three functions in this TU, and src/system.c already carries two more of them in the same hand-derived goto shape: cpu_side_move_dir_4 (= CD_sync, name string D_80016240, ~line 366) and the function at ~line 480 (= CD_ready, name string D_80016248). Any spelling that matches saEft01Init transfers to both.

- [s7] The reference proves the inherited 8/91 candidate is a wrong-basin local optimum: the original body has NO named arg4 intermediate (all four table lookups inline in the printf call, corroborating session 6's independent expand-RTL deduction) and NO k (both compare constants are plain literals). Both levers the 8 rests on are absent from the original.

- [s7] The honest cheat-free floor of the known-good basin is 32/96, measured by stripping every lever off the sessions-2/3 chassis and writing the reference's statements: clean (no levers) 32/96, cleana (+named arg4 only) 27/96, cleank (+k double-set LICM defeat only) 21/90, candidate (k + cnt=k staging + arg4) 8/91. The 8 is therefore not a cheat-free 8.

- [s7] Instrumented-cc1 ALLOCDBG on the clean form names the divergence exactly: pseudo 78 (tbl_125c) -> $s0, 77 (idx_1494) -> $s1, 72 (the param) -> $s2 are ALREADY target's dispositions with no lever of any kind. The whole allocation defect is loop.c's LICM (scan_loop/move_movables) creating pseudos 85 and 108 for the two loop-invariant compare constants; they rank 5th and 6th in global_alloc (pri 326 and 319 against tbl_11dc's 300) and displace tbl_11dc from target's $s3 to $s5, adding a fifth and sixth callee-save (+5 insns, 96 vs 91).

- [s7] ALLOCDBG on cleank proves the payoff of suppressing exactly those two hoists is complete: 79->$s0, 78->$s1, param->$s2, 77 (tbl_11dc)->$s3 with the k holder in $a0 — target's exact callee-save map, four callee-saves, both constants materialised inline, at 90 insns (one fewer than target's 91). The clean-C matching problem for this function is therefore ONE question: a legitimate spelling that denies loop.c those two specific hoists.

- [s7] The Sony reference transcribed verbatim scores 35/91 (r0) — it does hit target's exact instruction count but allocates five callee-saves (both constants hoisted to $s3/$s2 in the prologue) and fails to hoist a base register for Intr, folding the constant address into each lbu as %hi/%lo where target uses lbu a0,0(s1) / lbu v0,1(s1). r1 (+the three table-base locals) 37/93; r2 (set_alarm/get_alarm as real static __inline__ helpers) 31/94.

- [s7] KILLED — Sony's `volatile` on Alarm and Intr, applied as a header-type correction to BB2's extern declarations (the DAG-changing lever the session-6 frontier wanted): 8 -> 16 on the candidate chassis and no change whatever on the reference chassis (35 -> 35). BB2 links PsyQ 4.0 where sotn's reference is 3.5-era; the shipped object was evidently not built with these volatile-qualified. Harness tmp/grind/saEft01Init/s7/vscore.py applies the rewrite to every extern block in the TU (--novol disables).

- [s7] KILLED — the reference's `||` short-circuit timeout test, isolated against an otherwise byte-for-byte identical candidate: 8 -> 27/92. Also c2 (only the exits changed to the reference's while(1){...break;}, inner gotos kept) 61/129 and c4 (c1+c2) 74/131, both pathological block duplication rather than near-misses. The candidate's do{...}while(a0==0) + check: scaffolding stands.

- [s7] CONFIRMED by the reference — target's idx 68-72 (`j / li v0,-1 / move v0,zero / bnez v0 / li v0,-1`) is the inlined-get_alarm() return-value shape, so the candidate's v0 temp and `check:` label ARE the original's structure, and `if (mode != 0) { ret = 1; break; }` is exactly target's `beqz $s2 / addiu $v0,$zero,1` back-edge.

- [s7] Two inherited "structural requirements" are artifacts of the goto chassis, not facts about the function: with a real loop the three table bases hoist WITHOUT explicit source-level locals (r0 is 91 insns, where session 1's H1 goto-chassis test was 85), and Intr's base does NOT get a hoisted register even WITH an explicit pointer local.

- [s7] src/system.c was restored to the inherited 8/91 candidate at session end (md5 727793a9be0db10bee83fa2d5fae1e89 == tmp/grind/saEft01Init/s7/system.c.candbase; `sandbox saEft01Init --disable all` re-run afterwards printed score 8, build_insns 91). No build-pipeline file, rule file, engine file or queue file was touched, and no background process was started.

- [s7] saEft01Init @0x80081BB0 is Sony PsyQ LIBCD CD_datasync; the matched C is banked at memory/grind/saEft01Init/ref/sotn_libcd_bios_CD_datasync.c (Xeeynamo/sotn-decomp src/main/psxsdk/libcd/bios.c, fetched via gh api; full file cached at tmp/grind/saEft01Init/s7/ref/sotn_bios.c).

- [s7] Symbol mapping, all confirmed against the target disassembly: a0=mode; D_800F19B8/BC/C0 = Alarm.unk0/unk4/unk8 (VSync deadline / spin counter / caller name pointer); D_800162C0="CD_datasync"; D_800161B8="CD timeout: "; D_800161C8 = the printf format; D_800A11DC[] = the CdlCom name table; D_800A11D5 = CD_com; D_800A125C[] = the interrupt-state name table; D_800A1494 = the CD_intr struct (sync at +0, ready at +1); *D_800A14C0 = DMA3 CHCR; sys_VSync=VSync; tslTm2LoadImage_2=puts; cdrom_ClearIrq=CD_flush; debug_printf=printf. tslTm2LoadImage_2 and cdrom_ClearIrq are splat misnomers exactly like saEft01Init itself.

- [s7] set_alarm/get_alarm are inlined into three functions in this TU: src/system.c already carries cpu_side_move_dir_4 (= CD_sync, name string D_80016240, ~line 366) and the function at ~line 480 (= CD_ready, name string D_80016248) in the same hand-derived goto shape. Any spelling that matches saEft01Init transfers to both.

- [s7] The original body has NO named arg4 intermediate and NO `k` — both levers the inherited 8/91 rests on are absent from the reference, corroborating session 6's independent expand-RTL deduction that the named arg4 is structurally wrong.

- [s7] The honest cheat-free floor of the known-good basin is 32/96: clean (no levers, reference statements) 32/96, cleana (+named arg4 only) 27/96, cleank (+k double-set LICM defeat only) 21/90, candidate (k + cnt=k staging + arg4) 8/91.

- [s7] Instrumented-cc1 ALLOCDBG on the clean form: tbl_125c->$s0, idx_1494->$s1, the param->$s2 are already target's dispositions with no lever at all; the whole allocation defect is loop.c LICM creating pseudos 85 and 108 for the two loop-invariant compare constants, which rank 5th/6th in global_alloc (pri 326 and 319 vs tbl_11dc's 300) and displace tbl_11dc from target's $s3 to $s5, costing two extra callee-saves (+5 insns).

- [s7] ALLOCDBG on cleank: suppressing exactly those two hoists gives target's exact callee-save map (79->$s0, 78->$s1, param->$s2, tbl_11dc->$s3), four callee-saves, both constants materialised inline, the holder in $a0, at 90 insns — one FEWER than target's 91.

- [s7] KILLED: Sony's volatile on Alarm and Intr as a declaration correction — 8->16 on the candidate chassis, 35->35 (no change) on the reference chassis. BB2 links PsyQ 4.0 against sotn's 3.5-era reference.

- [s7] KILLED: the reference's || short-circuit timeout test in isolation — 8->27/92; c2 (exits only) 61/129 and c4 (both) 74/131 are pathological block duplication.

- [s7] KILLED: the reference transcribed verbatim — r0 35/91, r1 37/93, r2 (real static __inline__ helpers) 31/94.

- [s7] CONFIRMED by the reference: target's idx 68-72 (j / li v0,-1 / move v0,zero / bnez v0 / li v0,-1) is the inlined-get_alarm() return-value shape, so the candidate's v0 temp and check: label are the original's structure, and `if (mode != 0) { ret = 1; break; }` is target's `beqz $s2 / addiu $v0,$zero,1` back-edge.

- [s7] Two inherited 'structural requirements' are artifacts of the goto chassis, not facts about the function: with a real loop the three table bases hoist WITHOUT explicit source-level locals (r0 is 91 insns where session 1's goto-chassis H1 test was 85), and Intr's base does NOT get a hoisted register even WITH an explicit pointer local.

- [s7] Tooling, re-verified this session (and matching the standing project memory instrumented-cc1-location): tools/gcc-2.7.2/cc1 — NOT tools/gcc-2.7.2/build/cc1, which sessions 2/3/6 checked — carries all 13 of ALLOCDBG / PRIODBG / RANKDBG / DBRDBG / BB2_*_DEBUG, and tmp/grind/saEft01Init/s7/idump.sh printed CODEGEN-IDENTICAL against the frozen build/cc1 on every run.

- [s7] An earlier session-7 process ran 2026-08-01 15:03-15:18 and died before writing its outcome; the driver discarded it but its scratch survived. Its write-up is salvaged verbatim into evidence.md/hypotheses.md under an [inherited] heading with its facts tagged [s7-a]. Only its tooling claim was re-verified here; its sched1 priority table, RANKDBG cls=3 reading and p4-basin numbers (p4 = 10/91, the first form to decouple chain X's head from its load) are strong leads, not banked facts.

- [s7] src/system.c holds the inherited 8/91 candidate at session end (md5 727793a9be0db10bee83fa2d5fae1e89 == tmp/grind/saEft01Init/s7/system.c.candbase; sandbox re-run afterwards printed score 8, build_insns 91). No build-pipeline file, rule file, engine file or queue file was touched, and no background process was started.

## Session 8 (rederive) — measured

- [s8] READ OF THE TARGET BYTES (asm/funcs/saEft01Init.s, done first this
  session and it reframes F13): the two loop-invariant compare constants are
  materialised INSIDE the loop in the shipped object — `lui $v0,(0x3C0000>>16)`
  at 0x80081C48 (immediately before its `slt`) and `lui $v1,(0x1000000>>16)` at
  0x80081CE4 (immediately before its `and`) — while ALL THREE loop-invariant
  SYMBOL ADDRESSES are hoisted into the pre-loop block as callee-saves
  (`lui/addiu $s3,D_800A11DC` at 0x80081BD4, `$s1,D_800A1494` at 0x80081BDC,
  `$s0,D_800A125C` at 0x80081BE4).  So the original compile ran LICM and it
  moved the address movables but NOT the two const_int movables.  This kills
  the alternative reading of F13 ("maybe target hoists them too and our problem
  is elsewhere") and pins the question to exactly one asymmetry inside
  loop.c/scan_loop: address movables move, single-use const_int movables do not.

- [s8] TARGET'S CALLEE-SAVE MAP, read off the bytes: the parameter -> $s2
  (`addu $s2,$a0,$zero` in the prologue, `beqz $s2` as the loop-back test),
  D_800A125C -> $s0, D_800A1494 -> $s1, D_800A11DC -> $s3.  Four callee-saves,
  91 instructions.

- [s8] THE HONEST ZERO-LEVER FLOOR OF THIS FUNCTION IS 18 / 91, NOT 32 / 96 —
  a correction to how session 7's H30 has been read.  Session 7's `clean`
  (32/96) is the zero-lever score of the REAL-LOOP chassis.  The chassis that
  is actually committed at HEAD is the GOTO-LOOP (`loop:` label + `goto loop`),
  it carries no `k`, no `cnt = k` staging and no FAKE-family construct of any
  kind, and it measures 18 / 91 — target's exact instruction count, with both
  compare constants correctly materialised in-loop.  Re-measured this session
  as variant `g0` (HEAD's body verbatim, `git show HEAD:src/system.c`).  The
  reason is mechanical: a goto-loop emits no NOTE_INSN_LOOP_BEG, loop.c never
  sees a loop, so LICM never runs and the two const hoists that cost the
  real-loop chassis +2 callee-saves / +5 insns simply do not happen.

- [s8] The price of full reference-faithfulness on the goto-loop chassis is 5
  points and zero instructions: `g1` = g0 with the printf's four table lookups
  written inline (no named arg4/arg5 intermediates, exactly as the Sony source
  has them) measures 23 / 91.  Banked at
  rejected/goto-loop-reference-inline-args-23.c.  Compare the same delta on the
  real-loop chassis: clean 32 vs cleana 27.

- [s8] FIVE further independent decomps of this same Sony object were fetched
  and read (session 7 had only sotn-decomp).  Two carry real C bodies:
  ladysilverberg/xenogears-decomp src/slus_006.64/psyq/libcd/bios.c and
  hansbonini/psx_tomba src/scus_942.36/psyq/libcd/bios.c (tomba's CD_datasync
  has its INCLUDE_ASM commented out, i.e. that project considers the C body
  the accepted form).  celophi/lom-decomp, ser-pounce/rood-reverse and
  Xeeynamo/psyz are still INCLUDE_ASM stubs for CD_datasync and carry no
  information.  All copies are cached under tmp/grind/saEft01Init/s8/ref/.
  The three real sources agree on the statements and DISAGREE on the exit
  spelling, which is what made them worth transcribing:
    * sotn   — `while (true)` + a `ret` local + three `break`s.
    * xeno   — `while (true)` + direct `return -1/1/0`, with the mask test
               NESTED (`if (chcr & mask) { if (mode) return 1; } else return 0;`)
               and a dead `return 0;` after the loop.
    * tomba  — `while (true)`, early `return -1`, and a `sync` local set by the
               two other exits which `break` out to a trailing `return sync;`.

- [s8] Every `while (true)` transcription pays the LICM tax regardless of exit
  spelling: xeno's shape measures 27 / 96 with candbase's named argument
  intermediates and 31 / 96 with reference-inline arguments; tomba's shape
  measures exactly the same pair, 27 / 96 and 31 / 96.  The +5 instructions are
  the same two hoisted constants and the same two extra callee-saves session 7
  priced on `clean`.  Banked at rejected/xeno-while1-direct-returns-licm-hoists-27.c
  and rejected/tomba-while1-sync-var-breaks-27.c.

- [s8] The xeno NESTING itself (mask test nested with an `else return 0`,
  instead of candbase's `if (a0 == 0) goto loop; return 1;` fall-through) is
  BYTE-INERT on the goto-loop chassis: variant `x3` measures 18 / 91, identical
  to g0.  jump.c normalises the two spellings to the same CFG.  So the exit
  branch-sense axis is closed on this chassis.
  Banked at rejected/xeno-nesting-on-goto-loop-byte-inert-18.c.

- [s8] THE ENTIRE 18-POINT RESIDUAL OF THE GOTO-LOOP CHASSIS IS ONE THREE-CYCLE
  ROTATION OF $s0/$s1/$s2, and the instrumented cc1 prices it exactly.
  `tmp/grind/saEft01Init/s8/idump.sh g0` -> system.i.greg "Register
  dispositions" for saEft01Init: 72 in 16, 77 in 17, 76 in 18, 75 in 19, i.e.
  param -> $s0, D_800A125C -> $s1, D_800A1494 -> $s2, D_800A11DC -> $s3, against
  target's D_800A125C -> $s0, D_800A1494 -> $s1, param -> $s2, D_800A11DC -> $s3.
  Only the PARAMETER is misplaced; the other three keep their relative order.
  The ALLOCDBG block (cc1.log lines 4486-4490 of tmp/grind/saEft01Init/s8/g0):

      ord=1 pseudo=72 (param)     nrefs=2 livelen=52  pri=384  -> $s0
      ord=2 pseudo=77 (D_800A125C) nrefs=3 livelen=96  pri=312  -> $s1
      ord=3 pseudo=76 (D_800A1494) nrefs=3 livelen=98  pri=306  -> $s2
      ord=4 pseudo=75 (D_800A11DC) nrefs=2 livelen=100 pri=200  -> $s3

  global.c's allocno priority is exactly `nrefs * 10000 / livelen` on these
  four (2*10000/52 = 384, 3*10000/96 = 312, 3*10000/98 = 306, 2*10000/100 = 200
  — all four reproduce to the integer), and `find_reg` then hands out $s0, $s1,
  $s2, $s3 in that sorted order.  Target's map is the SAME sort with the param
  moved from first to third, so the whole 18 is bought by putting the param's
  priority anywhere strictly inside the open interval (306, 200) — i.e. below
  D_800A1494's 306 and above D_800A11DC's 200.  Two arithmetic ways to get
  there: raise both pointers' nrefs from 3 to 4 (-> 416 and 408, both above the
  param's 384, relative order preserved), or lengthen the param's live range
  from 52 to anywhere in 66..99 (-> pri 303 down to 202).

- [s8] The dead-store route to that ref-lift is DEAD, measured: `g6` (g0 plus
  `idx_1494 = idx_1494; tbl_125c = tbl_125c;`, run as an INSTRUMENT and never
  as a candidate — dead self-assigns are a forbidden family) scores 18 / 91 and
  its greg dispositions are byte-identical to g0's (72 in 16, 77 in 17, 76 in
  18, 75 in 19).  flow.c deletes the self-assigns before reg_n_refs is counted,
  so they never reach the allocno priority — which is exactly the caveat the
  standing [[duplicated-statement-into-arms]] rule records ("dead stores
  measured INERT for this — flow deletes before counting").  Any ref-lift here
  has to come from a LIVE fourth reference.
  Banked at rejected/dead-self-assign-ref-lift-deleted-by-flow-18.c.

- [s8] Statement and declaration ORDER inside the pre-loop block is byte-inert
  on this chassis, which closes the cheap end of the live-range axis: moving all
  three table-pointer assignments to the END of the pre-loop block (`g2`),
  reversing the DECLARATION order to target's callee-save order (`g4`), and
  reversing the ASSIGNMENT order to target's callee-save order (`g5`) all
  measure 18 / 91 unchanged.  Only moving them BEFORE the `sys_VSync(-1)` call
  moves the needle, and it regresses: `g3` = 23 / 91.
  Banked at rejected/pointer-inits-before-vsync-regresses-23.c.

- [s8] Housekeeping: src/system.c is left EXACTLY at HEAD (`git checkout --
  src/system.c`; `git status` clean for src).  Note for the next session that
  s7's tmp/grind/saEft01Init/s7/system.c.candbase — which score.py restores to
  — is the 8/91 CANDIDATE body, NOT HEAD's body; s8/score.py inherits that
  base, so "restored" leaves the candidate spliced in, not HEAD.  No
  build-pipeline file, rule file, engine file or queue file was touched, and no
  background process was started.

- [s8] Target-bytes read (done first this session, and it reframes the inherited F13): the shipped object materialises BOTH loop-invariant compare constants INSIDE the loop — `lui $v0,(0x3C0000>>16)` at 0x80081C48 immediately before its slt, `lui $v1,(0x1000000>>16)` at 0x80081CE4 immediately before its and — while ALL THREE loop-invariant symbol addresses ARE hoisted into the pre-loop block as callee-saves (lui/addiu $s3,D_800A11DC at 0x80081BD4; $s1,D_800A1494 at 0x80081BDC; $s0,D_800A125C at 0x80081BE4). So the original compile ran LICM and moved the address movables but not the const_int movables.

- [s8] Target's callee-save map read off the bytes: parameter -> $s2 (addu $s2,$a0,$zero in the prologue, beqz $s2 as the loop-back test), D_800A125C -> $s0, D_800A1494 -> $s1, D_800A11DC -> $s3; four callee-saves, 91 instructions.

- [s8] CORRECTION to how session 7's H30 has been read: the honest zero-lever floor of this function is 18/91, not 32/96. The 32/96 is the zero-lever score of the REAL-LOOP chassis; the body committed at HEAD is the GOTO-loop, carries no k / no cnt=k staging / no FAKE-family construct at all, and measures 18/91 at target's exact instruction count with both constants correctly in-loop.

- [s8] Full reference-faithfulness costs 5 points and zero instructions on the goto-loop chassis: g1 (HEAD's body with all four table lookups written inline in the printf, exactly as the Sony source has them) = 23/91, versus g0's 18/91. Same delta shape as clean 32 vs cleana 27 on the real-loop chassis.

- [s8] Five further independent decomps of this same Sony object were fetched and cached under tmp/grind/saEft01Init/s8/ref/ (session 7 had only sotn-decomp). Only two carry real C bodies — xenogears-decomp and psx_tomba, the latter with its INCLUDE_ASM commented out. lom-decomp, rood-reverse and psyz are still INCLUDE_ASM stubs for CD_datasync and carry no information. The three real sources agree on every statement and disagree only on the exit spelling.

- [s8] ALLOCDBG priority table for the zero-lever chassis (tmp/grind/saEft01Init/s8/g0/cc1.log lines 4486-4490): ord=1 pseudo=72 param nrefs=2 livelen=52 pri=384 -> $s0; ord=2 pseudo=77 D_800A125C nrefs=3 livelen=96 pri=312 -> $s1; ord=3 pseudo=76 D_800A1494 nrefs=3 livelen=98 pri=306 -> $s2; ord=4 pseudo=75 D_800A11DC nrefs=2 livelen=100 pri=200 -> $s3. global.c's priority is exactly nrefs*10000/livelen on all four. Two arithmetic routes into target's map: raise both pointers' nrefs 3->4 (416 and 408, relative order preserved), or lengthen the param's livelen from 52 into 66..99.

- [s8] The instrumented cc1 printed CODEGEN-IDENTICAL against the frozen build/cc1 on every run this session (both dump sets), so all of the above is diagnostic instrumentation, not a compiler divergence.

- [s8] Housekeeping: src/system.c is left EXACTLY at HEAD (git checkout -- src/system.c). Note for the next session that s7/system.c.candbase — which both s7's and s8's score.py restore to — is the 8/91 CANDIDATE body, not HEAD's body, so a 'restored' tree from score.py leaves the candidate spliced in. No build-pipeline file, rule file, engine file or queue file was touched; no background process was started.


- [s9] **CORRECTION to session 8's priority formula.** `global.c:allocno_compare` is `pri = floor_log2(nrefs) * nrefs / livelen * 10000` (times allocno_size, which is 1 here), not `nrefs * 10000 / livelen`. The two agree for nrefs 2 and 3 (floor_log2 = 1), which is why s8's fit looked exact, and they diverge from nrefs 4 up. Re-verified to the integer on 14 allocnos across three chassis this session. The correct value of F17's route (b) is therefore 833 / 816, not 416 / 408 — the conclusion (both pointers rise above the param's 384) is unchanged.

- [s9] **The parameter can NEVER be demoted by live-range work on a goto-loop chassis — F16 route (a) is arithmetically dead, not merely unspelled.** `flow.c` bounds `reg_live_length` by the function's insn count (59 real insns; the whole `;; Function saEft01Init` region of `system.i.lreg`), and the param's 52 is already within 7 of that ceiling. The three table pointers' 96-100 are NOT raw lengths: `local-alloc.c:update_equiv_regs` (the `reg_live_length[regno] *= 2;` at line ~1058) DOUBLES the live length of any pseudo carrying a REG_EQUIV note, and the same file (lines 1019-1052) attaches REG_EQUIV only to a single-set pseudo whose source is CONSTANT_P (via a promoted REG_EQUAL note) or an unchanging MEM. The param's set is `(set (reg/v 72) (reg:SI 4 a0))` — a hard-register source — so it can never qualify. Hence param pri >= 2*10000/59 = 338 for every possible spelling, always above D_800A1494's 306. Route (a) is closed permanently.

- [s9] **loop.c's threshold test, read and priced.** `move_movables` (loop.c:1631) moves a movable when `already_moved || (threshold * savings * m->lifetime) >= insn_count`. `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`; MIPS has FIRST_PSEUDO_REGISTER 68 with 8 entries set in FIXED_REGISTERS, so n_non_fixed_regs = 60 and threshold = 61 (this loop has calls). The `-da` `.loop` dump for the real-loop `clean` chassis prints `Loop from 38 to 187: 50 real insns.` and, for both compare constants, `regno 85 (life 1), move-insn savings 1  moved to 201` / `regno 108 (life 1), move-insn savings 1  moved to 203`. savings and lifetime are both already at their minimum of 1, so the test reduces to `61 >= 50` and is unconditionally true: **on a real loop of this size the two compare constants are ALWAYS hoisted.** The loop would need >= 62 real insns at loop.c time for the test to fail.

- [s9] **The loop.c:691-701 movable-creation gate, measured on both disjunct halves.** The gate skips building a movable only when all three of `(1) ! maybe_never && ! loop_reg_used_before_p(...)`, `(2) ! REG_USERVAR_P(dest) && ! REG_LOOP_TEST_P(dest)`, `(3) reg_in_basic_block_p(p, dest)` are false. REG_LOOP_TEST_P is set only in `jump.c:2253` and never for these, so (2) is false exactly when the constant lives in a named C local. Measured: constants in named locals at the TOP of the loop body are still hoisted (q2, 32/96, both listed as moved in the `.loop` dump) because maybe_never is 0 there; the same locals set AFTER the loop's first conditional branch make maybe_never 1 and the MASK then escapes for the first time (q5: only one movable listed as moved, 36/95). `maybe_never` is set at the first CODE_LABEL or JUMP_INSN encountered inside the loop (loop.c:921-930), and the loop's own top label does not count.

- [s9] **Escaping the hoist is not the same as reproducing target.** In q5 the escaped mask local measures 4 refs / 98 insns and crosses 5 calls, so it takes a callee-save; target has it as a block-local `lui $v1,0x100` immediately before its `and`. Block-local and not-hoisted are mutually exclusive inside a real loop: block-local makes disjunct (3) true, which builds the movable, which the threshold test then always moves. **The real-loop chassis therefore cannot reproduce target's constant placement at all**, which is why the reference corpus's `while (true)` spellings all sit at 27-35.

- [s9] **THE SESSION'S MAIN RESULT — `do { } while (0)` supplies flow.c's loop_depth weighting without giving loop.c a loop.** `flow.c` accumulates `REG_N_REFS += loop_depth`, and loop_depth is driven by NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END notes, which the front end emits for `do{}while(0)` exactly as for a real loop. Wrapping ONLY the timeout/printf block of the goto-loop chassis therefore lifts D_800A125C and D_800A1494 from 3 refs to 5 and D_800A11DC from 2 to 3, while the parameter (set in the prologue, used at the bottom, both outside the wrapper) stays at 2 refs / livelen 52. Measured `.lreg` for that body: D_800A125C 5/96 -> pri 1041, D_800A1494 5/98 -> 1020, param 2/52 -> 384, D_800A11DC 3/100 -> 300 — exactly target's $s0/$s1/$s2/$s3 order, and exactly the predicted numbers. Score 18 -> 8 / 91 with nothing else changed (r1). The function's real back edge is still a bare `goto`, so loop.c never runs on it and both compare constants stay in-loop as plain literals: **no `k`, no staging, no FAKE-family LICM defeat anywhere.**

- [s9] **Positional diff of the wrapped chassis against target (tmp/grind/saEft01Init/s1/grind_diff.py, 91 vs 91):** every instruction from the prologue through build idx 45 and from idx 62 through the epilogue is byte-exact, INCLUDING the whole tail that sessions 3 and 7 spent turns on — `bnez $v0` at 71 with `addiu $v0,-1` in its delay slot, `beqz $v0` at 79 with target's `addu $v0,$zero,$zero` in its delay slot (session 3's F7, closed for free), and `beqz $s2` at 81. The entire residual is build idx 46-61, the debug_printf argument block.

- [s9] **Argument-block sweep on the corrected chassis (twelve spellings, all at 91 insns unless noted):** both lookups inline 13; arg5 named only 13; the two index bytes named + lookups inline 13; named pointer with inline deref 9; named pointer + named value 9; both slots as named pointers 10; third argument also named 14 at **90** insns (one fewer than target — the third argument must stay inline); arg4 named and assigned first with arg5 inline **7**; both named with arg4 assigned first **7**; both named with arg5 assigned first 8; index staged into a local then arg4 named **7**. 7 is the floor of the entire family and is the session's banked floor.

- [s9] **What the remaining 7 diffs are.** With arg4 named the two `lbu` now land in target's order (`lbu 0(s1)` before `lbu 1(s1)`), but the idx[0] address chain runs through `$v0` and issues `lw a3,0(v0)` at build idx 54, whereas target keeps that chain alive in `$a0` across the third argument's whole computation and issues `lw a3,0(a0)` as the very LAST memory reference of the block, after `sw v1,16(sp)` and `lw a2,0(v0)`. Session 6's expand-shape reading (inline argument => load emitted last into the hard reg) does NOT reproduce this on the corrected chassis — the all-inline form is 13, six worse than the named form.

- [s9] Housekeeping: `src/system.c` is left EXACTLY at HEAD (`git checkout -- src/system.c`; verified clean). Session 9 built its own harness under tmp/grind/saEft01Init/s9/ (score.py, idump.sh, gen.py, gen2.py, gen3.py, gen4.py, bank.py) whose `system.c.candbase` is still the session-5 8/91 candidate body, so `[src restored]` from score.py does NOT mean "restored to HEAD" — check `git status` explicitly. No build-pipeline file, rule file, engine file or queue file was touched; no background process was started; the instrumented cc1 printed CODEGEN-IDENTICAL against the frozen build/cc1 on all four dump runs.

- [s9] The floor is 7 / 91 (sandbox saEft01Init --disable all), down from the inherited 8, and the new form carries NO k, NO mask staging and NO FAKE-family LICM defeat - its only match device is one do{}while(0) wrapper, which the owner's 2026-07-06 ruling sanctions for ANY codegen effect with a mandatory inline FAKE annotation (single level, so no nesting justification is needed). memory/grind/saEft01Init/candidate.c is this form and was re-verified at 7/91 after being written.

- [s9] global.c:allocno_compare is pri = floor_log2(nrefs) * nrefs / livelen * 10000, not session 8's nrefs*10000/livelen; the two agree for nrefs 2 and 3 and diverge from 4 up. Re-verified to the integer on 14 allocnos across three chassis.

- [s9] The parameter can never be demoted by live-range work: reg_live_length is bounded by the function's 59 real insns, the param is already at 52, and its (set (reg/v 72) (reg:SI 4 a0)) can never receive the REG_EQUIV note that doubles the three pointers' lengths (local-alloc.c:1019-1052 attaches REG_EQUIV only to a single-set pseudo with a CONSTANT_P or unchanging-MEM source; the doubling is at line ~1058).

- [s9] loop.c's move_movables test (line 1631) is threshold * savings * lifetime >= insn_count with threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) = 61 for MIPS (FIRST_PSEUDO_REGISTER 68, 8 entries set in FIXED_REGISTERS). The .loop dump for the real-loop chassis prints 'Loop from 38 to 187: 50 real insns' and both constants at 'life 1, savings 1, moved', so the test reduces to 61 >= 50 and is unconditionally true: on a real loop of this size the compare constants are ALWAYS hoisted, and the loop would need >= 62 real insns for that to change.

- [s9] A named C local for a compare constant escapes the hoist only when its set sits after the loop's first CODE_LABEL/JUMP_INSN (maybe_never == 1) AND its live range leaves that basic block - measured, q5, mask only. But escaping is useless: the escaped local then spans the loop (4 refs / 98 insns, crosses 5 calls) and takes a callee-save, where target has a block-local lui immediately before its use. Block-local and not-hoisted are mutually exclusive inside a real loop, which closes the whole real-loop branch and explains why every while(true) spelling in the reference corpus sits at 27-35.

- [s9] do{}while(0) emits NOTE_INSN_LOOP_BEG/END, so flow.c's REG_N_REFS += loop_depth weighting applies inside it while loop.c never sees a loop. Wrapping only the timeout/printf block of the goto chassis lifts D_800A125C and D_800A1494 from 3 refs to 5 and D_800A11DC from 2 to 3, leaving the parameter at 2/52, which is exactly target's $s0/$s1/$s2/$s3 sort order. Measured 18 -> 8 with nothing else changed; the .lreg numbers matched the prediction to the integer.

- [s9] On the wrapped chassis the positional diff against asm/funcs/saEft01Init.s is byte-exact everywhere except build idx 46-61, INCLUDING the entire tail that sessions 3 and 7 spent turns on - session 3's F7 (beqz $v0 with addu $v0,$zero,$zero in the delay slot) closed for free.

- [s9] The remaining 7 diffs: target keeps the idx[0] address chain alive in $a0 across the third argument's whole computation and issues lw a3,0(a0) as the LAST memory reference of the block (after sw v1,16(sp) and lw a2,0(v0)); we compute the same chain in $v0 and issue lw a3,0(v0) six insns early. Twelve argument spellings were measured and 7 is the floor of that family, so this is a scheduling/allocation question rather than a spelling one.

- [s9] Two extra facts for the next session: naming the THIRD argument as well produces a 90-instruction build (one fewer than target), so it must stay inline; and two nested do{}while(0) wrappers regress all the way back to 18/91, so one level is the whole lever.

- [s9] Housekeeping: src/system.c was restored to HEAD with git checkout and verified clean; no build-pipeline, rule, engine, tools or queue file was touched; no background process was started. The instrumented cc1 printed CODEGEN-IDENTICAL against the frozen build/cc1 on all four dump runs. NOTE for the next session: tmp/grind/saEft01Init/s9/system.c.candbase (inherited from s7/s8) is the session-5 8/91 candidate body, so score.py's '[src restored]' does NOT mean restored-to-HEAD.

- [s10] The session-9 candidate re-applied to src/system.c re-measures at exactly 7 / 91 (`sandbox saEft01Init --disable all`: `"score": 7, "target_insns": 91, "build_insns": 91`, rules_dropped 15, cheat_asm_stripped 7). The floor is reproducible from `memory/grind/saEft01Init/candidate.c` alone; HEAD's `src/system.c` still carries the older 18/91 body.

- [s10] **GCC 2.7.2's instruction scheduler runs BACKWARD.** `sched.c` computes `priority()` over an insn's LOG_LINKS (its PREDECESSORS) and `schedule_block` walks the block from the end, emitting each chosen insn at the current tail. So the number the instrumented cc1 prints as `PRIODBG ... final_pri` is the dependence DEPTH from the top of the basic block, and a HIGHER priority means the insn is emitted LATER in the block. `insn_cost` charges 1 for a load->use edge and 0 otherwise, so the depth is a count of load-use delays. Earlier sessions' reasoning about "the lowest-priority insn in the block" was inverted.

- [s10] **The argument block's real sched2 priority table** (instrumented cc1, `cc1.log` line 4601; the sched1 table at line 4382 differs only in that the pre-reload copies 129/133 are still present): level 1 = insn 93 `lbu` idx[0], 116 `lbu` idx[1], 105 `lw` D_800F19C0, 127 `lui` fmt; level 2 = 96/98/100 (the idx[0] `sll`/`addu`/`lw a3` chain), 119/121/123 (the idx[1] chain), 108 `lbu` D_800A11D5; level 3 = 125 `sw 16(sp)`, 111 `sll`, 113 `addu`, 131 `lw a2`; level 4 = 135 `jal debug_printf`, 138 `jal cdrom_ClearIrq`. SEVEN insns are tied at level 2, so `rank_for_schedule`'s tie-breaks decide the emitted order: first the load/store dependence class against `last_scheduled_insn`, then `INSN_LUID`.

- [s10] `INSN_LUID` at sched2 is the order SCHED1 left the insn chain in, not source order. That is the mechanical reason session 6 measured source statement placement inside the argument block as byte-inert, and it means any statement-order lever has to survive sched1 first.

- [s10] Tooling: cc1 insn UIDs restart per function and `cc1.log` carries no function separators, so PRIODBG/RANKDBG traces are ambiguous by UID alone. `tmp/grind/saEft01Init/s10/findprio.py` resolves it by sliding a window over the SET lines and picking the region whose UID multiset best matches the UID list read out of the `-da` `.sched` dump (21/21 overlap here). `tmp/grind/saEft01Init/s10/schedscan.py` prints that dump as one compact line per insn. Both are the tools of record for this class of question. RANKDBG lines are NOT emitted in this function's regions, so the tie-break itself still has to be inferred rather than read.

- [s10] **The residual 7 decomposes into two independent sub-goals, and two different measured spellings each solve one of them.** (A) `lw a3` as the LAST memory reference (target build idx 61): the both-lookups-inline form lands it at exactly idx 61, with its whole idx[0] chain at the block tail. (B) both `lbu` at the block head, idx 46/47 in target's order: the candidate (arg4 named) has this, the named-pointer form has them reversed, the inline form has only one. No measured spelling has both. Target's idx[0] chain is SPREAD across the block (`lbu` 46, `sll` 52, `addu` 56, `lw a3` 61); every spelling reachable from C emits that chain as one contiguous run, either early or late.

- [s10] Session 9's H39 read the all-inline form's 13 as refuting session 6's expand-shape prediction (inline argument => its load emitted last into the hard register). That reading was wrong: the inline form does exactly what session 6 predicted for sub-goal (A) and pays all of its 13 on sub-goal (B). Session 6's expand forensics are rehabilitated.

- [s10] Naming the index BYTE is completely inert once the value lookup is inline: idx[0] named, both named, and both named in reverse source order all measure 13 at 91 insns, byte-identical to the pure-inline form. CSE/combine folds the named index pseudo straight back into the address chain.

- [s10] Nothing in the argument block may be lifted out of the `do{}while(0)` wrapper's call-free stretch: assigning the arg4 address pointer BEFORE the `tslTm2LoadImage_2` call makes it live across a call, so it takes a callee-save and destroys the `$s0-$s3` map the wrapper buys — 31 at 92 insns, the worst result on this chassis since the wrapper was found.

- [s10] The register difference F19 blamed (`$a0` vs `$v0` for the idx[0] address chain) is DOWNSTREAM of the schedule. Our chain is live idx 51-54 and does not overlap arg5's loaded value in `$v1` (live 57-60); target's is live 46-61 and does overlap. Different conflict graph, different colouring. And an anti-dependence cannot pin an insn late — it raises the LATER insn's depth, never the earlier one's.

- [s10] Argument-block spelling family: NINETEEN spellings now measured (session 9's twelve plus i1-i9 here) and 7/91 is still the floor. New this session: both inline 13; idx[0] byte named + inline 13; both bytes named + inline 13; both bytes named reversed + inline 13; arg4 address named + inline deref 9; arg5 address named + arg4 inline 14; both addresses named 10; arg4 address named above the call 31 at 92.

- [s10] Housekeeping: `src/system.c` was restored to HEAD (`git checkout -- src/system.c`) and verified clean; no build-pipeline, rule, engine, tools or queue file was touched; no background process was started. The instrumented cc1 printed CODEGEN-IDENTICAL against the frozen `build/cc1`. NOTE: `tmp/grind/saEft01Init/s10/system.c.candbase` IS the session-9 7/91 candidate body (unlike s9's candbase, which was the session-5 8/91 body), so score.py's `[src restored]` restores to the CANDIDATE, not to HEAD.

- [s10] The session-9 candidate re-applied to src/system.c re-measures at exactly 7 / 91 (sandbox saEft01Init --disable all: score 7, target_insns 91, build_insns 91, rules_dropped 15, cheat_asm_stripped 7). HEAD's src/system.c still carries the older 18/91 body, so the floor is reproducible only from memory/grind/saEft01Init/candidate.c.

- [s10] GCC 2.7.2's instruction scheduler runs BACKWARD: sched.c computes priority() over an insn's LOG_LINKS (predecessors) and schedule_block emits each chosen insn at the block's current tail. The instrumented cc1's PRIODBG final_pri is therefore dependence DEPTH from the top of the basic block, and HIGHER priority means emitted LATER. Every earlier session's reasoning about 'the lowest-priority insn in the block' was inverted.

- [s10] The argument block's real sched2 priority table (cc1.log line 4601): level 1 = 93 lbu idx[0], 116 lbu idx[1], 105 lw D_800F19C0, 127 lui fmt; level 2 = 96/98/100 (idx[0] sll/addu/lw-a3), 119/121/123 (idx[1] chain), 108 lbu D_800A11D5; level 3 = 125 sw 16(sp), 111 sll, 113 addu, 131 lw a2; level 4 = 135 jal debug_printf, 138 jal cdrom_ClearIrq. Seven insns tie at level 2.

- [s10] INSN_LUID at sched2 is the order SCHED1 left the insn chain in, not source order - the mechanical reason session 6 measured source statement placement inside this block as byte-inert, and the reason any statement-order lever must survive sched1 first.

- [s10] The residual 7 decomposes into two independent sub-goals: (A) lw a3 as the last memory reference at build idx 61, achieved exactly by the both-inline spelling; (B) both lbu at the block head at idx 46/47 in target's order, achieved by the candidate. No measured spelling has both, because target's idx[0] chain is SPREAD across the block (lbu 46, sll 52, addu 56, lw a3 61) while every C-reachable spelling emits it as one contiguous run.

- [s10] Naming the index BYTE is completely inert once the value lookup is inline (13/13/13, byte-identical to pure inline): CSE/combine folds the named index pseudo straight back into the address chain.

- [s10] Nothing in the argument block may be lifted out of the do{}while(0) wrapper's call-free stretch - assigning the arg4 address pointer before the tslTm2LoadImage_2 call makes it live across a call, takes a callee-save, and destroys the $s0-$s3 map (31 at 92 insns, the worst result on this chassis since the wrapper was found).

- [s10] Nineteen argument-block spellings are now measured (session 9's twelve plus i1-i9) and 7/91 remains the floor of the entire family. New this session: both inline 13; idx[0] byte named 13; both bytes named 13; both bytes named reversed 13; arg4 address named + inline deref 9; arg5 address named + arg4 inline 14; both addresses named 10; arg4 address named above the call 31 at 92.

- [s10] Tooling of record for priority questions: tmp/grind/saEft01Init/s10/findprio.py (resolves cc1.log's per-function UID restarts by matching SET-insn UID multisets against the -da .sched dump, 21/21 overlap here) and s10/schedscan.py (compact one-line-per-insn view of the scheduled RTL). RANKDBG lines are NOT emitted in this function's regions, so the tie-break itself still has to be inferred rather than read.

- [s10] Housekeeping: src/system.c restored to HEAD with git checkout and verified clean; no build-pipeline, rule, engine, tools or queue file touched; no background process started; the instrumented cc1 printed CODEGEN-IDENTICAL against the frozen build/cc1. NOTE: tmp/grind/saEft01Init/s10/system.c.candbase IS the 7/91 candidate body (unlike s9's, which was the 8/91 one), so score.py's '[src restored]' restores to the CANDIDATE, not to HEAD.


## [s11] structural — the positional model of the residual 7, read off target itself

Floor unchanged at **7 / 91** (candidate re-applied and re-measured this session
via `tmp/grind/saEft01Init/s11/setup.py`; `sandbox saEft01Init --disable all` =
7, build_insns 91).  Thirty-one further C forms measured across three axes that
had never been touched; all three axes are now dead, and the model of what the
7 IS became much sharper.

### The scheduler's emitted order in target is EXACTLY ascending priority

Read straight off `asm/funcs/saEft01Init.s` lines 50-65 (block idx 46-61) using
session 10's confirmed rule (GCC 2.7.2 schedules backward; printed priority is
dependence DEPTH from the block top; higher priority = emitted LATER):

```
46 lbu a0,0(s1)        level 1     (idx[0] byte, base already live in $s1)
47 lbu v0,1(s1)        level 1     (idx[1] byte)
48 lui a1 / 49 lw a1   level 1     (D_800F19C0, symbol-addressed)
50 sll v0,2            level 2     idx[1] chain
51 addu v0,v0,s0       level 2
52 sll a0,a0,2         level 2     idx[0] chain
53 lw v1,0(v0)         level 2     arg5 VALUE
54 lui v0 / 55 lbu v0  level 2     D_800A11D5 (one level deeper: lui-based)
56 addu a0,a0,s0       level 2     idx[0] chain
57 sll v0,2            level 3     arg3 chain
58 addu v0,v0,s3       level 3
59 sw v1,0x10(sp)      level 3     arg5 stack home
60 lw a2,0(v0)         level 4     arg3 load
61 lw a3,0(a0)         level 4     arg4 load
```

The sequence 1,1,1,2,2,2,2,2,2,3,3,3,4,4 is monotone — which is exactly what a
backward list scheduler emits when it always pops the highest-priority ready
insn.  So target's `lw a3` is NOT a level-2 insn that got lucky on a tie-break
(session 10's reading): it is a **level-4** insn, one whole level ABOVE
`sw 16(sp)`.  The only edge that can put a load a level above the outgoing-arg
store is a **store->load memory dependence on the outgoing-argument stack
slot** — i.e. in target's RTL stream the `sw 16(sp)` is emitted BEFORE the
`lw a2` and `lw a3`, while both `lbu`s and the whole arg5 value chain are
emitted before the `sw`.

This reframes F22 completely.  The question is NOT "how do I give the idx[0]
address chain one more unit of dependence depth" (F22's route 1 — see the kill
below); it is "how do I get arg4's LOAD emitted after the arg5 stack store while
its ADDRESS chain is still emitted before it".

### calls.c is the mechanism, and it is not loop-gated

`tools/gcc-2.7.2/calls.c:1615-1665` — "Precompute all register parameters" runs
FORWARD (i = 0 .. num_actuals-1) over the register args, calls `expand_expr` on
each (which emits the ADDRESS computation and, for an array element, returns a
MEM), and then at 1653-1664 optionally forces the value into a pseudo:

```c
if (! (REG || SUBREG(REG))  &&  mode != BLKmode  &&  rtx_cost (value, SET) > 2
    && preserve_subexpressions_p ())            /* SMALL_REGISTER_CLASSES undef on mips */
  args[i].value = copy_to_mode_reg (args[i].mode, args[i].value);
```

and `stmt.c:2435 preserve_subexpressions_p()` returns 1 immediately when
`flag_expensive_optimizations` is set — which `toplev.c:3387` does for every
`optimize >= 2` build, i.e. always for this project's `-O2`.  So the loop-nesting
half of that predicate (`loop_stack`, the short-loop UID window) is dead code in
our builds: **the do{}while(0) wrapper cannot be influencing argument expansion
at all.**  What decides early-vs-late is purely whether `args[i].value` is
already a REG when the precompute loop sees it, i.e. whether the C wrote the
argument as a named local (REG -> nothing emitted here, the load already happened
at the statement) or inline (MEM -> address emitted here, load deferred to
`load_register_parameters` after the stack stores).

### The argument block has exactly THREE reachable states, and none is target

Twenty-seven spellings were already banked; this session added another twenty-
four (wrapper-extent 7, arg3-axis 8, decl/init-order 10 -- listed below -- plus
6 i2-repairs).  Every single one lands on one of three attractors:

| attractor | C shape | score | positional signature |
|---|---|---|---|
| 7 / 91 | arg4 written as a named VALUE local | 7 | both `lbu` at 46/47 in target's order; `lw a3` six insns EARLY at 54 |
| 9 / 91 | arg4 written as a named ADDRESS pointer with inline deref | 9 | `lw a3` late (58); the two `lbu` REVERSED at 46/47; D_800A11D5 chain pushed past the `sw` |
| 13 / 91 | arg4 fully inline | 13 | `lw a3` at exactly 61 (target's slot) but the whole idx[0] chain dragged to 57-61 |

q1/q3/q4 (i2 plus arg5's index named / arg5's value named / pointer arithmetic
instead of `&tbl[i]`) are all 9, byte-identical to i2; q5/q6 (candidate plus an
arg5 index/value local) are all 7, byte-identical to the candidate.  The
attractors are rigid: nothing bolted onto them moves the block.

### Axis 1 — do{}while(0) wrapper EXTENT (7 forms).  DEAD, with a boundary.

w1 (wrapper around the printf statement only, `tslTm2LoadImage_2` outside) = 8;
w2 (wrapper opens after `tslTm2LoadImage_2`, keeps `cdrom_ClearIrq` inside) = 8;
w3 (candidate extent plus `v0 = -1;` inside) = 7; w4 (candidate extent minus
`cdrom_ClearIrq`) = 7; w5 (arg4 declared at function scope) = 7; w6 (the whole
do_timeout section incl. `goto check` inside) = 7; w7 (arg4's store hoisted OUT
of the wrapper, above it) = **16 at 92 insns**.

So the wrapper's extent is byte-INERT in both directions as long as it contains
(a) the `tslTm2LoadImage_2` call and (b) the arg4 store.  Dropping the call out
of it costs exactly one; lifting the arg4 store out of it costs nine and an
instruction, reproducing session 10's i8 result (the value goes live across a
call, takes a callee-save, and the $s0-$s3 map collapses).

### Axis 2 — the arg3 (`tbl_11dc[D_800A11D5]`) spelling (8 forms).  DEAD.

Never touched in 27 prior spellings.  n1 (arg3 address named, then arg4 address
named) 13; n2 (arg3 address named alone) 13; n3 (arg3 address named + the
candidate's arg4 value local) 15; n4 (arg4 address then arg3 address) 10;
n5 (all three addresses named in target's chain order) 14; n6 (arg3 VALUE named)
12; n7 (arg3 then arg4 values named) 14 at 90 insns; n8 (arg3 + arg5 addresses
named, arg4 inline) 14.  Every arg3 spelling regresses; the candidate's fully
inline arg3 is the family optimum.

### Axis 3 — declaration order vs initialisation order of the three pointers (10 forms).

DECLARATION order is byte-inert: o6/o7/o8 permute `s32 *tbl_11dc; u8 *idx_1494;
s32 *tbl_125c;` and all score 7.  INITIALISATION order is live and the
candidate's (`tbl_11dc`, `idx_1494`, `tbl_125c`) is the unique optimum: o5
(11dc, 125c, 1494) 8; o2/o4/o10 11; o1/o3/o9 12.  So the assignment order is
load-bearing (it feeds pseudo numbering -> INSN_LUID -> the level-2 tie-breaks)
and it is already at its best value.

### Artifacts

`tmp/grind/saEft01Init/s11/{gen,gen2,gen3,gen4,bank,ledger}.py`,
`score.py` (s10's harness, re-pointed at WSL so the sandbox actually runs),
`diff_cand.txt`, `diff_i1.txt`, `diff_i2.txt` (side-by-side vs target for the
three attractors), `variants/*.c`.

- [s11] Floor re-measured this session at 7 / 91 with the session-9/10 candidate body applied to src/system.c (sandbox saEft01Init --disable all: score 7, target_insns 91, build_insns 91). The instruction COUNT is exact; the whole residual is inside the debug_printf argument block, build idx 46-61.

- [s11] Target's emitted argument block (asm/funcs/saEft01Init.s lines 50-65) is monotone-ascending in scheduler priority: lbu idx0 / lbu idx1 / lw a1 at level 1; sll+addu of both index chains, lw v1 (arg5 value) and the D_800A11D5 lbu at level 2; the arg3 sll+addu and sw v1,16(sp) at level 3; lw a2 and lw a3 at level 4. A backward list scheduler that always pops the highest-priority ready insn emits exactly this shape, so the ordering is fully explained without any tie-break.

- [s11] Because target's two register-arg loads sit a whole level ABOVE the outgoing-arg store, target's RTL stream must emit `sw 16(sp)` BEFORE `lw a2` and `lw a3` (store->load memory dependence on the 16(sp) slot) while both lbu's and the arg5 value chain are emitted before it. Session 10's F22 (a dependence-depth deficit in the idx[0] address chain) is refuted: target's chain starts from `lbu 0($s1)`, a base already live in a callee-save, exactly like ours.

- [s11] calls.c:1615-1665 precomputes register arguments in FORWARD order and only forces a value into a pseudo when it is not already a REG; stmt.c:2435 + toplev.c:3387 show the gating predicate preserve_subexpressions_p() is unconditionally 1 at -O2 (flag_expensive_optimizations). So an INLINE array-element argument has its address chain emitted in the precompute loop and its load deferred to load_register_parameters (after the stack stores) — target's split — while a NAMED-local argument has both emitted at the statement. This is the mechanism behind the three attractors.

- [s11] The argument block has exactly THREE reachable states and they are rigid: arg4 as a named VALUE = 7/91 (both lbu at the block head in target's order, lw a3 six insns early at 54); arg4 as a named ADDRESS with inline deref = 9/91 (lw a3 late, the two lbu reversed, the D_800A11D5 chain pushed past the sw); arg4 fully inline = 13/91 (lw a3 at exactly target's idx 61 but the whole idx[0] chain dragged to 57-61). Six bolt-on variants (q1-q6) were byte-identical to whichever attractor they started from.

- [s11] do{}while(0) wrapper EXTENT is a dead axis with a sharp boundary (7 forms): byte-inert in both directions as long as the wrapper contains BOTH the tslTm2LoadImage_2 call and the arg4 store. Excluding the call costs exactly one (8); hoisting the arg4 store above the wrapper costs nine and an instruction (16 at 92 insns) because the value goes live across a call and takes a callee-save, destroying the $s0-$s3 map.

- [s11] The arg3 (tbl_11dc[D_800A11D5]) spelling axis is dead (8 forms, scores 10-15): named address alone 13, named address + arg4 address 13, named address + candidate arg4 15, arg4-address-then-arg3-address 10, all three addresses named in target's chain order 14, named value 12, arg3+arg4 values named 14 at 90 insns, arg3+arg5 addresses named with arg4 inline 14. Fully-inline arg3 is the family optimum.

- [s11] The three table pointers' DECLARATION order is byte-inert (three permutations, all 7). Their INITIALISATION order is LIVE and the candidate's order (tbl_11dc, idx_1494, tbl_125c) is the unique optimum — the other five permutations score 8, 11, 11, 12, 12. Future sessions must not reorder those three assignments.

- [s11] Fifty-one distinct C forms for this argument block are now banked across sessions 4-11 and 7/91 remains the floor of the entire family.

## [s12] structural — four new axes closed; `const` is a FOURTH attractor

Floor unchanged at **7 / 91** (candidate re-applied via
`tmp/grind/saEft01Init/s12/score.py z0`: sandbox 7, build_insns 91).
Fifty-two further C forms measured across five axes that no prior session had
touched.  Four are dead; the fifth found the first genuinely new positional
signature since session 4, and it costs 2.

### F25 is ANSWERED at the calls.c level (the emission order is settled)

`tools/gcc-2.7.2/calls.c` read directly:

* **1618-1665** — "Precompute all register parameters": a FORWARD loop over the
  register args.  For an arg whose value is not already a REG it calls
  `expand_expr` (emitting the address computation) and then, at 1653-1664,
  `copy_to_mode_reg`s the value only when `rtx_cost (value, SET) > 2` and
  `preserve_subexpressions_p ()`.
* **1736-1739** — `store_one_arg` for every arg with `args[i].reg == 0`
  (i.e. our single stack argument, arg5) runs AFTER that loop.
* **~1876** — `load_register_parameters` runs after both.

So the RTL emission order for this call really is
`[register-arg address chains] [arg5's whole chain + sw 16(sp)] [the register
loads]`, exactly as session 11's reading predicted, and target's split
(arg4's address chain before the `sw`, its load after) is reachable at expand.
**The i1 attractor's failure is therefore NOT an expand-order problem** — with
arg4 fully inline the RTL order is already target's and the block still comes
out with the whole idx[0] chain contiguous at 57-61.  What differs is the
SCHEDULE, i.e. sched.c's priorities and its memory-dependence view, not the
order the insns were handed to it.  F25's stated puzzle ("something is emitting
store_one_arg before the register-arg precompute loop") is refuted: nothing is.

### The one live lever found: RTX_UNCHANGING_P on arg4's load (a FOURTH attractor)

`const`-qualifying the hoisted `tbl_125c` pointer (c1) scores **9 / 91** and its
diff (`tmp/grind/saEft01Init/s12/diff_c1.txt`) shows a block shape no prior form
produced:

```
target   ... sll a0,2 | lw v1,0(v0) | lui/lbu D_800A11D5 | addu a0,s0 |
             sll v0,2 | addu v0,s3  | sw v1,16(sp)       | lw a2,0(v0) | lw a3,0(a0)
c1       ... sll v1,2 | lw v0,0(v0) | addu v1,s0         | sw v0,16(sp)|
             lw a3,0(v1) | lui/lbu D_800A11D5 | sll v0,2 | addu v0,s3  | lw a2,0(v0)
candidate ...(lw a3 issues at 54, SIX insns before the sw at 60)
```

i.e. c1 is the first measured form whose `sw 16(sp)` precedes `lw a3` — target's
relation, and the thing sessions 10 and 11 identified as the residual's core.
It costs 2 for two new reasons: the two `lbu` come out swapped (idx1 before
idx0) and the whole D_800A11D5/arg3 chain is pushed AFTER `lw a3` instead of
being interleaved before the `sw`.

Two further measurements pin the mechanism precisely:

* a per-ACCESS cast — `((const s32 *)tbl_125c)[idx_1494[0]]` with the pointer
  itself left non-const (e1) — reproduces the 9 exactly, and the same cast on
  arg5 (e2) or arg3 (e3, 90 insns) does not: the lever is specifically
  **RTX_UNCHANGING_P on the ARG4 load**, nothing about the pointer's type.
* the same cast on a FULLY INLINE arg4 (e7/e8) is byte-inert at 13.  So
  `const` only perturbs the schedule for a load emitted at a STATEMENT; a load
  deferred to `load_register_parameters` already sits after the stack store and
  the unchanging flag changes nothing.

The const chassis is as RIGID as the other three: thirteen bolt-ons measured on
it (arg4 inline 13, arg4 address named 9, arg5 named 9, arg5 index named 9,
arg3 value named 14/90i, arg3 address named 13, arg4 index named 9, idx also
const 9, and all five pointer-init permutations 10/13/14/13/14) and nothing goes
below 9.  There are now FOUR attractors — 7 (arg4 named value), 9 (arg4 named
address), 9 (arg4 named value + unchanging), 13 (arg4 inline) — and 63 measured
forms distribute over exactly those four.

### Dead axes measured this session

1. **arg1 / the format-string address** (3 forms, a1-a3).  Every prior form
   touched only args 2-5.  Naming `&D_800161C8` (inside or before the wrapper)
   or `&D_800161B8` for the `tslTm2LoadImage_2` call is byte-inert at 7.
2. **MEM_IN_STRUCT_P / the alias class of the table MEMs** (6 forms, m1-m6).
   With `s32 *p`, `p[i]` is an INDIRECT_REF and MEM_IN_STRUCT_P is 0 — which is
   why the pointer-arithmetic spellings (p1-p3) were byte-inert.  Re-typing the
   bases as pointers-to-array (`s32 (*)[]`, `u8 (*)[2]`) makes the accesses real
   ARRAY_REFs and sets MEM_IN_STRUCT_P while keeping the hoisted-pseudo base;
   all six forms are byte-identical at 7.  Alias CLASS is not the lever;
   RTX_UNCHANGING is.
3. **Pre-loop global-store placement** (4 forms, g1-g4).  Moving
   `D_800F19BC = 0;` / `D_800F19C0 = &D_800162C0;` among the three pointer
   initialisations is byte-inert (7); only lifting the pointer inits above the
   `sys_VSync` call regresses (g3 = 17), which is the already-banked family.
   Session 11 had only permuted the three pointers among themselves.
4. **The `debug_printf` PROTOTYPE** (9 runs).  HEAD declares a fixed 5-arg
   prototype.  Re-declaring it `(void *, ...)` — a real printf-style varargs
   signature, almost certainly the original Sony one — and `()` (K&R, no
   prototype) are both byte-identical to the fixed form against three different
   bodies (candidate 7, arg4-inline 13, arg5-named 13).  expand_call's
   stdarg/promotion path contributes nothing here.  (Note for the technique
   index: [[fake-varargs-explicit-homing]] does not apply — its symptom is bulk
   pre-`subu` argument homes, and this call has none.)
5. Also inert: pointer-arithmetic deref spellings of arg4/arg5 on the candidate
   chassis (p1-p3), and an arg4 index byte in its own local, `s32` or `u8`
   (k1/k2) — all 7.

### Artifacts

`tmp/grind/saEft01Init/s12/{gen,gen2,gen3,gen4,ledger}.py`, `score.py`
(s11's harness re-pointed at s12), `score2.py` (the same harness with a
swappable `debug_printf` prototype), `sbs.py` (a reusable target-vs-built
side-by-side lister built on `engine.score.normalized_insns`),
`diff_c1.txt`, `variants/*.c` (52 forms).

- [s12] Floor re-measured at 7 / 91 with the session-9 candidate body applied to src/system.c. Fifty-two further forms measured; the floor of the whole family is still 7 and the candidate body is unchanged since session 9.

- [s12] calls.c settles F25's expand question: the register-arg precompute loop (1618-1665) runs BEFORE store_one_arg for the stack argument (1736-1739), which runs before load_register_parameters (~1876). So an inline arg4 really does get its address chain emitted before the `sw 16(sp)` and its load after it — target's split is reachable at expand, and the i1 attractor's contiguous 57-61 idx[0] chain is a SCHEDULING outcome, not an expand-order one. F25's premise ("something emits store_one_arg before the precompute loop") is refuted.

- [s12] `const`-qualifying the arg4 load (either `const s32 *tbl_125c` or a per-access `((const s32 *)tbl_125c)[...]` cast) is a FOURTH attractor at 9/91 and the first measured form whose outgoing-arg `sw 16(sp)` precedes `lw a3`, which is target's relation. It costs 2 because the two `lbu` come out swapped and the D_800A11D5/arg3 chain is pushed after `lw a3` instead of interleaved before the `sw`. Thirteen bolt-ons on that chassis all score >= 9.

- [s12] The const lever is RTX_UNCHANGING_P on a STATEMENT-emitted load, nothing else: the same cast on arg5 (e2) or on a fully-inline arg4 (e7/e8) is byte-inert, and the cast on arg3 costs an instruction (e3, 9 at 90 insns). A load deferred to load_register_parameters already sits after the stack store, so the unchanging flag has nothing to move.

- [s12] MEM_IN_STRUCT_P is NOT the lever: re-typing the hoisted bases as pointers-to-array (`s32 (*)[]`, `u8 (*)[2]`) turns the accesses into real ARRAY_REFs (expand sets MEM_IN_STRUCT_P, unlike the INDIRECT_REF a `T *p` subscript produces) and all six such forms are byte-identical at 7.

- [s12] The arg1 (format-string address) spelling axis is dead — 3 forms, all byte-inert at 7. Every one of the 51 prior banked forms touched only args 2-5.

- [s12] Pre-loop global-store placement is dead — moving `D_800F19BC = 0;` and `D_800F19C0 = &D_800162C0;` among the three pointer initialisations is byte-inert (g1/g2/g4 = 7); only hoisting the pointer inits above the sys_VSync call regresses (g3 = 17).

- [s12] The `debug_printf` prototype axis is dead — `(void *, ...)` varargs and `()` K&R declarations are byte-identical to HEAD's fixed 5-arg prototype against three different bodies (7 / 13 / 13). expand_call's stdarg and default-promotion paths contribute nothing to this argument block.

- [s12] Floor re-measured this session at 7/91 with the session-9 candidate body in src/system.c (sandbox saEft01Init --disable all: score 7, target_insns 91, build_insns 91); the body is unchanged since session 9 and remains the family floor.

- [s12] calls.c settles the expand-order question: the register-arg precompute loop (1618-1665) runs BEFORE store_one_arg for the stack argument (1736-1739), which runs before load_register_parameters (~1876). An inline arg4 therefore gets its address chain emitted before the `sw 16(sp)` and its load after it — target's split is reachable at expand, and the fully-inline attractor's contiguous idx[0] chain at build idx 57-61 is a scheduling outcome, not an expand-order one.

- [s12] `const`-qualifying the arg4 load — either `const s32 *tbl_125c` or a per-access `((const s32 *)tbl_125c)[...]` cast with the pointer left non-const — is a FOURTH attractor at 9/91 and the first measured form whose outgoing-arg `sw 16(sp)` precedes `lw a3`, which is target's relation and the thing sessions 10 and 11 identified as the residual's core.

- [s12] The const lever is specifically RTX_UNCHANGING_P on a STATEMENT-emitted load: the same cast on arg5 is byte-inert, on arg3 it costs an instruction (9 at 90 insns), and on a fully inline arg4 it is byte-inert at 13 (a load deferred to load_register_parameters already sits after the stack store, so the flag has nothing to move).

- [s12] The const chassis is as rigid as the other three attractors — thirteen bolt-ons measured on it (arg4 inline 13, arg4 address named 9, arg5 named 9, arg5 index named 9, arg3 value named 14 at 90 insns, arg3 address named 13, arg4 index named 9, idx also const 9, five pointer-init permutations 10/13/14/13/14) and nothing goes below 9.

- [s12] MEM_IN_STRUCT_P is not the lever: re-typing the hoisted bases as pointers-to-array (`s32 (*)[]`, `u8 (*)[2]`) makes the accesses real ARRAY_REFs (which expand flags, unlike the INDIRECT_REF a `T *p` subscript produces) and all six such forms are byte-identical at 7.

- [s12] The arg1 (format-string address) spelling axis is dead — 3 forms, all byte-inert at 7 — and it was the last untouched argument position; all 51 prior banked forms only ever touched args 2-5.

- [s12] Pre-loop global-store placement is dead: moving `D_800F19BC = 0;` and `D_800F19C0 = &D_800162C0;` among the three pointer initialisations is byte-inert (7), and only hoisting the pointer inits above the sys_VSync call regresses (17).

- [s12] The debug_printf prototype axis is dead: `(void *, ...)` varargs and `()` K&R declarations are byte-identical to HEAD's fixed 5-arg prototype against three different bodies (7 / 13 / 13).

- [s12] Sixty-three distinct C forms for this argument block are now banked across sessions 4-12, and they distribute over exactly four rigid attractors: 7 (arg4 as a named value), 9 (arg4 as a named address), 9 (arg4 as a named value with an unchanging load), 13 (arg4 fully inline).

## [s13] permuter — decomp-permuter's objective is ANTI-CORRELATED with the honest metric on this basin

Two fresh-seed campaigns were run on structurally different chassis, both built
with the same full-TU-context workspace recipe sessions 4/5 used (full `cpp` of
src/system.c, trimmed to declarations + saEft01Init for pycparser, verified
byte-identical to the full-TU compile, target.o assembled from
asm/funcs/saEft01Init.s at offset 0, no regfix/asmfix in the pipeline):

  * `ws2` = the session-9 zero-lever do{}while(0) candidate (sandbox 7 / 91),
    permuter base score 435.  16,487 iterations over 883 s on 5 jobs, 6 finds.
  * `ws3` = the session-12 `const s32 *tbl_125c` fourth attractor (sandbox
    9 / 91), permuter base score 455.  13,249 iterations over 758 s, 3 finds.

EVERY find was re-scored by splicing its saEft01Init body back into
src/system.c and running `sandbox saEft01Init --disable all`
(tmp/grind/saEft01Init/s13/rescore.py + rescore.ps1).  The full table:

    permuter 435 (ws2 base)  -> sandbox  7 / 91     <- the candidate
    permuter 435 (x4 finds)  -> sandbox  7 / 91     (byte-identical respellings)
    permuter 428             -> sandbox 18 / 92
    permuter 423             -> sandbox 17 / 92
    permuter 455 (ws3 base)  -> sandbox  9 / 91
    permuter 455 (find)      -> sandbox 10 / 91
    permuter 450             -> sandbox 10 / 91

Both campaigns' ONLY score improvements are large sandbox REGRESSIONS, and on
ws3 a single permuter score (455) covers two different sandbox distances.  The
permuter's gradient here points away from the candidate basin, and its score is
not even a function of the honest distance.

### The mechanism (exact, from tools/decomp-permuter/src/scorer.py:14-18)

decomp-permuter's weighted score charges PENALTY_REGALLOC = 5 per differing
register, PENALTY_REORDERING = 60 per reordered instruction, PENALTY_INSERTION
= PENALTY_DELETION = 100, and it re-ALIGNS the two instruction streams before
counting.  The engine sandbox's distance is POSITION-LOCKED and weights every
differing instruction at 1.  So:

  * the candidate's residual is 7 reorderings + 3 register differences =
    7*60 + 3*5 = 435 exactly — i.e. the whole 435 is the SAME seven
    instructions the sandbox charges 7 for;
  * a form that rotates the entire callee-save map pays 16 renames = 80 to the
    permuter but 16 to the sandbox.

Ratio: the permuter is ~12x more sensitive to ordering than to renaming, the
sandbox is 1x.  Our entire remaining residual is ordering.  Therefore, from a
7 / 91 base whose diffs are all reorderings, ANY mutation that trades ordering
for renaming looks like progress to the permuter and is a regression to us —
which is precisely what both campaigns found and nothing else.  This also
explains why permuter WORKED in sessions 4/5 (base distance 18 with a mixed
regalloc + ordering residual, where large genuine wins existed in both
currencies) and cannot work now.

The scorer's penalties are class attributes with no CLI or settings.toml knob,
and tools/ is outside a grind session's writable surface, so retuning them is
an operator decision, not a session one.

### The one substantive form the campaigns produced

`D_800A125C[idx_1494[1]]` for arg5 — the first of 65 measured argument
spellings to read a table through its own global symbol rather than the hoisted
base.  permuter 423, sandbox 17 / 92: the second symbol reference costs a fresh
`lui at` / `addu at,v1` / `lw v1,0(at)` chain (+1 insn) and, by dropping
tbl_125c to one use in the block, collapses its allocno priority so the whole
callee-save map rotates.  Banked as
rejected/permuter-global-arg5-rotates-callee-save-map-17-92.c.

### Operational note for any future permuter session on this function

Campaigns are still runnable, but their output must be treated as a candidate
GENERATOR, never as a gradient: re-score every find with the sandbox before
believing it.  tmp/grind/saEft01Init/s13/{rescore.py,rescore.ps1,odf.sh} do
exactly that (splice -> sandbox; and a positional build-vs-target disassembly
diff for any find worth reading).

- [s13] The candidate chassis re-applied to src/system.c measures exactly 7 / 91 this session (91 target insns, 91 build insns), unchanged since session 9.

- [s13] decomp-permuter's weighted score is NOT a monotone function of the honest sandbox distance for this function: on ws3 the same permuter score (455) covers sandbox 9/91 and sandbox 10/91, and on both chassis the only score improvements are sandbox regressions.

- [s13] The candidate's permuter base score 435 decomposes exactly as 7 reorderings * 60 + 3 register differences * 5 — arithmetic proof that the permuter is charging ~12x more per unit of the residual we still have (ordering) than per unit of the currency it can cheaply buy (renaming).

- [s13] tools/decomp-permuter/src/scorer.py:14-18 fixes those penalties as class attributes with no CLI or settings.toml override, so re-weighting the objective is outside a grind session's writable surface (tools/ is off-limits) and is an operator decision.

- [s13] Campaign telemetry: ws2 (candidate chassis) 16,487 iterations / 882.9 s / 6 finds, best 423; ws3 (const chassis) 13,249 iterations / 758.1 s / 3 finds, best 450. Both harvested with --stop; 12 permuter processes killed; no campaign outlives this session.

- [s13] Both permuter workspaces were verified sound before use: the pycparser-trimmed base.c compiles BYTE-IDENTICAL to the full-TU compile, and base/target are both 91 instructions at offset 0 — so the negative result is about the objective, not a broken harness.

- [s13] The 65-form argument-spelling record now includes the global-symbol axis, which is dead: the hoisted base is strictly cheaper, which is exactly why sessions 2-9 hoisted.

- [s13] A reusable re-scoring harness now exists for any future permuter work on this function: tmp/grind/saEft01Init/s13/rescore.py (splice a find into src/system.c) + rescore.ps1 (drive the engine sandbox over a list of finds) + odf.sh (positional build-vs-target disassembly diff for one find).

## Session 14 (permuter) — facts

* **The candidate chassis's permuter base score of 435 is NOT seven
  reorderings.** Read out of the scorer's own Penalty List, forced on inside
  the real permuter process: Register Differences 7 (x5) = 35, Reorderings 0,
  Insertions 2 (x100) = 200, Deletions 2 (x100) = 200. Session 13's
  "435 = 7*60 + 3*5" is a numerical coincidence with the true
  "2*100 + 2*100 + 7*5". scorer.py only calls an insertion/deletion pair a
  60-point reordering when the two rows are IDENTICAL strings; ours differ in
  mnemonic, so they cost 200 each way. The stock objective's misalignment is
  100-vs-5, and it is nearly blind to the seven field differences the sandbox
  charges 7 for.
* **decomp-permuter's objective is patchable from inside a grind session.** The
  penalties are class attributes read as `self.PENALTY_*` at score time;
  `tools/permuter_campaign.py` launches with `Popen(..., cwd=ROOT)` and no
  `env=`, so a `sitecustomize.py` on PYTHONPATH monkeypatches them in the
  permuter process with no write to `tools/` and no loss of campaign telemetry.
  Reusable shim: `tmp/grind/saEft01Init/s14/sbshim/`. `BB2_SHIM_DEBUG=1` forces
  `debug_mode` so the Penalty List and the permuter's own asm diff land in
  campaign.log — that is the way to read what a base score is MADE of, for any
  function.
* **An aligned objective (REGALLOC 1 / REORDERING 2 / INSERTION-DELETION 4) is
  monotone with the sandbox across chassis but not within one.** Bases:
  candidate 23 <-> sandbox 7, const 29 <-> sandbox 9. All four candidate-chassis
  finds tie at 23 and screen sandbox 7 exactly. But the const chassis's only
  real descent (26 from base 29) screens sandbox 10 against the base's 9, and
  its three 29-ties screen 9 / 10 / 12. Cause: difflib re-aligns the candidate
  stream before penalties apply, and saEft01Init's whole residual is positional
  (the same seven instructions in the wrong order), which re-alignment destroys.
* **Campaign volume this session: 60,127 iterations across two fresh-seeded
  chassis, 9 finds, best sandbox result 7 / 91 — i.e. the floor, never below.**
  Combined with session 4/5/13's campaigns, the permuter has now had five
  campaigns and two objectives on this function and has produced exactly one
  lasting lever (s4's, later shown to be a wrong-basin optimum).
* The 7 / 91 floor is re-confirmed with the session-9 chassis re-applied to
  src/system.c this session.

- [s14] The candidate chassis's permuter base score of 435 is 2 insertions (200) + 2 deletions (200) + 7 register differences (35) and ZERO reorderings — read from the scorer's own Penalty List, not inferred. Session 13's '435 = seven reorderings + three register diffs' is falsified; the two decompositions collide at 435 by coincidence.

- [s14] scorer.py converts an insertion/deletion pair into a single 60-point 'reordering' only when the two objdump rows are IDENTICAL strings (Counter over full row text, common = min(ins, dels)). Rows that differ in mnemonic are charged 100 + 100 instead, which is why saEft01Init's misalignment is 100-vs-5 rather than 60-vs-5.

- [s14] decomp-permuter's objective is patchable from inside a grind session without touching tools/: the penalties are class attributes read as self.PENALTY_* at score time, and tools/permuter_campaign.py launches with Popen(..., cwd=ROOT) and no env=, so a PYTHONPATH sitecustomize.py monkeypatch reaches the permuter process with campaign telemetry intact. Reusable shim: tmp/grind/saEft01Init/s14/sbshim/. BB2_SHIM_DEBUG=1 additionally forces Scorer(debug_mode=True) so the Penalty List and the permuter's own asm diff land in campaign.log — that is how to read what ANY function's base score is made of.

- [s14] Under the aligned objective (REGALLOC 1 / REORDERING 2 / INSERTION-DELETION 4) the candidate chassis bases at 23 and the const chassis at 29, against sandbox 7 and 9 — monotone across chassis. All four candidate-chassis finds tie at permuter 23 and screen sandbox exactly 7.

- [s14] Within a basin the aligned objective is still misleading: the const chassis's only genuine descent, permuter 26 from base 29, screens sandbox 10 against the base's 9, and its three permuter-29 ties screen sandbox 9, 10 and 12.

- [s14] Campaign volume this session: 60,127 iterations across two fresh-seeded chassis, 9 finds, best sandbox result 7/91 — the floor, never below. Across sessions 4, 5, 13 and 14 the permuter has now had five campaigns and two different objectives on this function and produced exactly one lasting lever (session 4's, later shown by session 7 to be a wrong-basin optimum).

- [s14] The session-9 chassis was re-applied to src/system.c and re-measured at exactly 7/91 this session; src/system.c is left carrying it.

- [s14] Both campaigns were harvested with --stop before the session ended (procs_killed 9 each) and `pgrep -af permuter.py` is clean — no campaign outlives this session.

## Session 15 (forensics) — banked facts

- Floor re-verified at **7 / 91** with the session-9 candidate body applied to
  `src/system.c` (`sandbox saEft01Init --disable all`, build_insns 91,
  target_insns 91). The tree as inherited at HEAD carried the OLD session-2-era
  body (staged `arg5`/`arg4`, no `do{}while(0)` wrapper) scoring 18; applying
  `memory/grind/saEft01Init/candidate.c` is a REQUIRED first step every session.
- The instrumented cc1 at `tools/gcc-2.7.2/cc1` (NOT `tools/gcc-2.7.2/build/cc1`)
  carries working `BB2_PRIO_DEBUG` / `BB2_RANK_DEBUG` / `BB2_ALLOC_DEBUG` hooks.
  `tmp/grind/saEft01Init/s10/idump.sh <tag>` runs both compilers on the current
  `src/system.c` and prints `CODEGEN-IDENTICAL` when the instrumented binary
  agrees with the frozen one — it did on all three dumps taken this session.
- `RANKDBG` only prints on an INSN_PRIORITY tie (`sched.c:2409` returns before
  the classification), so every RANKDBG line is by construction a genuine
  tie-break decision.
- **The load/store dependence-class tie-break is inert in this function's
  argument block.** Candidate chassis: 54 in-block RANKDBG decisions, 54 with
  `val=0`, every operand `cls=3`. Const chassis: 49 in-block decisions, 49 with
  `val=0`. Among equal-priority insns the order is therefore decided purely by
  INSN_LUID, i.e. by the incoming chain order (at sched2, sched1's output).
- **The candidate and the const chassis have byte-identical final
  INSN_PRIORITY vectors** across all 19 block insns
  (`89=2 93=2 96=1 98=1 100=2 105=1 108=1 111=1 113=1 116=1 119=2 121=1 123=1
  125=1 127=1 129=2 131=1 133=2 135=1`). RTX_UNCHANGING_P moves no priority.
- **The whole 7-vs-9 difference is one edge.** `.sched` LOG_LINKS for insn 125
  (`sw 16(sp)`, the outgoing 5th argument):
  candidate `(insn_list 89 (insn_list:REG_DEP_ANTI 100 (insn_list 123 (nil))))`
  vs const `(insn_list 89 (insn_list 123 (nil)))`. Insn 100 is
  `arg4 = tbl_125c[idx_1494[0]]` (the `lw a3` value); the anti-dependence is a
  write-after-read on the outgoing-arg stack slot emitted by
  `sched.c:sched_analyze_1` because the load's MEM is aliasable. It forces
  `lw a3` ahead of `sw 16(sp)`; target has the opposite relation.
- Removing that edge flips the sched1 order from
  `89,93,96,116,98,119,121,100,108,123,127,125,111,105,113,129,131,133,135` to
  `89,116,119,121,93,123,127,125,96,108,98,111,105,113,129,100,131,133,135`,
  which becomes sched2's LUID order: the `sw` now precedes `lw a3` (target's
  relation) but the two `lbu` come out in the wrong order (116 before 93).
  That trade is exactly the +2.
- A per-access `((const s32 *)tbl_125c)[idx_1494[0]]` on arg4 ALONE is not just
  the same score as the whole-pointer `const s32 *tbl_125c` — it is the same
  sched1 order, the same sched2 order and the same priority vector. Marking
  arg5's load unchanging is completely inert. The const axis is closed.
- The named-ADDRESS-local + inline-deref form
  (`s32 *arg4 = &tbl_125c[idx_1494[0]]; debug_printf(..., *arg4, ...)`)
  re-measured on the session-9 chassis is 9 / 91 (one fewer RTL insn — the
  separate `move a3` copy disappears — but the same 9-attractor, not a fifth
  basin).
- Target's idx[0] chain is SPREAD across the `sw 16(sp)` boundary
  (`lbu 46 / sll 52 / addu 56 / sw 58 / lw a3 61`). All four known attractors
  put the whole chain on one side of it or pay 2 to cross it. That boundary is
  `calls.c`'s `store_one_arg` (1736-1739), which runs after the register-arg
  precompute loop (1618-1665) and before `load_register_parameters` (~1876).
- Tooling of record for this class of question:
  `tmp/grind/saEft01Init/s15/blocksum.py <tagdir>` (auto-derives the argument
  block from `system.i.sched`, prints sched1/sched2 orders, the priority vector
  and the RANKDBG class-vs-LUID census) and
  `tmp/grind/saEft01Init/s15/rankscan.py <tagdir> <uids>` (full PRIODBG edge and
  RANKDBG dump for a UID set — VERY verbose, prefer blocksum.py).

- [s15] Floor re-verified at 7 / 91 this session (sandbox saEft01Init --disable all, build_insns 91, target_insns 91) with the session-9 candidate body applied to src/system.c and left in place.

- [s15] IMPORTANT for the next session: the tree as inherited at HEAD carried the OLD session-2-era body (staged arg5/arg4, no do{}while(0) wrapper) scoring 18. Applying memory/grind/saEft01Init/candidate.c to src/system.c is a REQUIRED first step every session; the SessionStart hook's 'pure-C distance 18' is that stale state, not a regression.

- [s15] The instrumented cc1 lives at tools/gcc-2.7.2/cc1 (NOT tools/gcc-2.7.2/build/cc1) and its BB2_PRIO_DEBUG / BB2_RANK_DEBUG / BB2_ALLOC_DEBUG hooks all work. tmp/grind/saEft01Init/s10/idump.sh <tag> compiles the current src/system.c with BOTH compilers and printed CODEGEN-IDENTICAL on all four dumps taken this session, so the instrumented traces describe the frozen toolchain's real behaviour.

- [s15] RANKDBG only prints on an INSN_PRIORITY tie (sched.c:2409 returns before the classification), so every RANKDBG line is by construction a genuine tie-break decision - the census is a complete enumeration of the tie-breaks, not a sample.

- [s15] The load/store dependence-class tie-break is INERT in this function's argument block: 54/54 val=0 on the candidate chassis and 49/49 val=0 on the const chassis, every operand cls=3, zero class-resolved decisions on either. Order among equal-priority insns is decided purely by INSN_LUID.

- [s15] The candidate and the const chassis have byte-identical final INSN_PRIORITY vectors over all 19 block insns; RTX_UNCHANGING_P changes the sched1 DAG, never a priority.

- [s15] The entire 7-vs-9 split is ONE edge: insn 125 (sw 16(sp), the outgoing 5th argument) carries REG_DEP_ANTI 100 on the candidate and does not on the const chassis, where the loads' MEMs are mem/s/u:SI instead of mem/s:SI. Insn 100 is arg4's statement-expanded load, i.e. target's lw a3 value.

- [s15] Insn 131 (lw a2, the arg3 chain) carries a true dependence on 125 on BOTH chassis, so arg3's load is already after the sw in every form - target's relation there is not at risk and is not the residual.

- [s15] A per-access const cast on arg4 alone reproduces the whole-pointer const chassis exactly (same sched1 order, same sched2 order, same priority vector, same 9/91). Marking arg5's load unchanging is inert. Do not re-probe any const / RTX_UNCHANGING_P spelling on this function.

- [s15] The named-ADDRESS-local + inline-deref form is 9 / 91 with one fewer RTL insn (no separate move a3) - the same 9-attractor, not a fifth basin.

- [s15] Target's idx[0] chain is SPREAD across the sw 16(sp) boundary (lbu 46 / sll 52 / addu 56 / sw 58 / lw a3 61). All four known attractors put the whole chain on one side of that boundary or pay 2 to cross it. The boundary is calls.c's store_one_arg (1736-1739), between the register-arg precompute loop (1618-1665) and load_register_parameters (~1876).

- [s15] New tooling of record: tmp/grind/saEft01Init/s15/blocksum.py <tagdir> auto-derives the argument block from system.i.sched and prints the sched1/sched2 orders, the priority vector and the RANKDBG class-vs-LUID census in ~6 lines. tmp/grind/saEft01Init/s15/rankscan.py <tagdir> <uids> gives the full PRIODBG edge + RANKDBG dump for a UID set but is VERY verbose - prefer blocksum.py.

- [s15] The do{}while(0) wrapper is still the candidate's single match device and has still never been through a fresh adversarial cheat-reviewer (F23, unchanged since session 12).

## Session 16 (forensics) — sched1 runs FLAT-PRIORITY, and the block problem is SHARED with two sibling queue functions

Floor re-measured at **7 / 91** with the session-9 candidate body in place
(unchanged; re-applied from `memory/grind/saEft01Init/candidate.c`, sandbox
`--disable all` = 7, 91 vs 91 insns).  No new C spelling was proposed this
session — the whole session went into instrumented-cc1 forensics on the three
already-banked attractors, and it produced two findings that change the model
of the residual.

### E16.1 — the SAME argument block appears in THREE queue functions, with the SAME target order, and ALL THREE of our builds fail it the same way

`src/system.c` contains the identical `debug_printf` timeout block in three
functions: `cpu_side_move_dir_4` (src/system.c:~400), `marionation_Exec`
(src/system.c:~519) and `saEft01Init` (src/system.c:~781).  Target bytes
(`asm/funcs/*.s`, the 14 insns before `jal debug_printf`):

    cpu_side_move_dir_4  ... addu $a0,$a0,$s3 / sll $v0,2 / sw $v1,0x10($sp) /
                             lui $at,%hi(D_800A11DC) / addu $at,$at,$v0 /
                             lw $a2,%lo(D_800A11DC)($at) / lw $a3,0($a0)
    marionation_Exec     ... identical modulo $s5 for the table base
    saEft01Init          ... addu $a0,$a0,$s0 / sll $v0,2 / addu $v0,$v0,$s3 /
                             sw $v1,0x10($sp) / lw $a2,0($v0) / lw $a3,0($a0)

i.e. in all three ORIGINAL objects the `sw $v1,0x10($sp)` precedes BOTH
register-argument loads and `lw $a3,0($a0)` is the last memory reference of
the block.  The only structural difference is that saEft01Init's arg3 goes
through a hoisted `$s3 = D_800A11DC` base while the two siblings re-materialise
the symbol with `lui $at / addu / lw %lo`.

Our builds fail identically: `sandbox cpu_side_move_dir_4 --disable all` = 7 /
160, and its per-index diff (artifact
`sibling_cpu_side_move_dir_4_diff.txt`) shows `lw $a3,0($v1)` emitted at index
55 where target has it at 65 — the SAME early-`lw a3` residual, on a body that
names BOTH arg4 and arg5 (`arg4 = tbl_125c[idx_1494[0]]; arg5 =
tbl_125c[idx_1494[1]];`).  So the residual is not an artefact of saEft01Init's
chassis or of its do{}while(0) wrapper: it is one shared block-scheduling
problem, and whatever C closes it here closes two more queue items.

### E16.2 — calls.c's split IS observable in the RTL, and all three attractors' expand shapes are now on disk

`blockdump.py` (new, this session) prints the block from any `-da` dump.  For
the three attractors, the pre-sched (`system.i.combine`) order is:

  * candidate / named-VALUE arg4 (7): 93 lbu idx0, 96 sll, 98 addu, **100 lw**
    (the statement), 105 lw a1, 108/111/113 arg3 ADDRESS chain (no load),
    116/119/121/123 arg5 chain, 125 sw 16(sp), 127 a0, 129 a1, **131 lw a2**,
    133 move a3, call.
  * fully-inline arg4 (13): 93 a1, 96/99/101 arg3 address, 104/107/109 arg4
    ADDRESS chain, 112..119 arg5 chain, 121 sw, 123 a0, 125 a1, **127 lw a2,
    129 lw a3**, call.
  * named-ADDRESS pointer + inline deref (9): 93 lbu idx0, 96 sll, **98 addu →
    the pointer pseudo** (statement), 103 a1, 106/109/111 arg3 address,
    114..121 arg5 chain, 123 sw, 125 a0, 127 a1, **129 lw a2, 131 lw a3**.

This CONFIRMS the s12/s15 reading of calls.c 1618-1665 directly from RTL: an
inline array-element argument leaves its load to `load_register_parameters`
(after `store_one_arg`'s `sw`), a named-VALUE argument performs the load at the
statement, and arg3 — inline in every form we ship — has ALWAYS had the split
shape.  The pointer form is therefore EXACTLY target's expand shape (address
chain at the earliest LUIDs, load last) and it still scores 9: the residual is
100% a scheduling outcome, not an expand-order one.  F28's premise ("split the
chain across the store_one_arg boundary") is thus ANSWERED — we already have a
form that does it — and F28 as written is closed.

### E16.3 — the decisive pass is SCHED1, and at sched1 the whole block carries LAUNCH_PRIORITY (flat priorities)

`BB2_SCHED_DEBUG=1` (the SCHEDDBG PICK hook at sched.c:3956, which prints the
whole ready list with priorities and LUIDs at every pick) was run for the first
time on this function.  Artifacts `ptr_sched1_picks.txt` /
`ptr_sched2_picks.txt`.

  * `schedule_block` is confirmed BACKWARD by source: sched.c:3970-3975 does
    `NEXT_INSN (insn) = last; last = insn;` — each picked insn is PREPENDED, so
    the final order is the exact REVERSE of the pick order, and readiness
    propagates to an insn's PREDECESSORS.  The trace reverses to the emitted
    `.sched` order exactly, on both passes.
  * At SCHED1 nearly every in-block insn is picked with
    `pri=2130706433 = 0x7F000001 = LAUNCH_PRIORITY`.  sched.c:3985 sets
    `INSN_PRIORITY (insn) = LAUNCH_PRIORITY` around `schedule_insn`, and
    `birthing_insn_p` (sched.c:2495) re-raises any insn that makes a register
    live, so at sched1 (`reload_completed == 0`) the register-pressure
    heuristic FLATTENS the dependence-depth priorities that sessions 10/11/15
    modelled.  **Dependence-depth priority does not order this block at
    sched1.**  What orders it is (a) when each insn leaves the latency queue
    (clock ticks skip: 19 → 21), and (b) INSN_LUID = the pre-sched RTL order.
  * s15's class-inertness finding replicates on a THIRD chassis: every RANKDBG
    line in the ptr chassis' sched1 block window returns `val=0`, including the
    two decisive ones (`last=125 y=121 cls=3 x=93 cls2=3 val=0` and
    `last=121 y=119 cls=3 x=93 cls2=3 val=0`).  The load/store dependence-class
    tie-break is inert on candidate, const AND pointer chassis.
  * The pointer chassis' whole 9-point residual comes down to two adjacent
    picks at sched1 clock 16/17: with the ready set `[121(l=16), 93(l=3)]` it
    takes 121, then with `[93(l=3), 119(l=15)]` it takes 93 — so 93 (the idx[0]
    `lbu`, the LOWEST LUID in the block) is placed AFTER the whole idx[1] chain
    and target's head order (`lbu a0,0(s1)` first) is lost by one pick.  At
    sched2 the same block is re-picked with flat `pri=1` and pure-LUID
    tie-breaks, so sched2 merely re-states sched1's order.

### E16.4 — what this means for the search

Sessions 10-15 searched C spellings against a model in which dependence-depth
priorities decide the block.  At sched1 they do not.  The sched1 output is a
deterministic function of (pre-sched RTL order, dependence edges, latency
queue) with flat priorities — which makes it SIMULABLE.  Instead of sweeping
more C spellings blind (67 banked forms over four attractors), the next
session can invert the scheduler: replay sched1's backward pass over the block
DAG for every permutation of the pre-sched RTL order that C can actually
produce, and keep only those whose reverse-pick order equals target's 14-insn
sequence.  That yields the REQUIRED LUID order, and only then do we look for C
that emits it.  The C surface that moves LUID order is small and already
mapped: statement vs inline per argument (4 attractors), statement ORDER among
the named ones, and the arg3/arg5 spellings.

- [s16] Floor re-measured this session at 7/91 (91 vs 91 insns) with the session-9 candidate body re-applied to src/system.c; body unchanged, no new C spelling proposed.

- [s16] src/system.c contains the same debug_printf timeout block three times: cpu_side_move_dir_4 (~line 400), marionation_Exec (~line 519), saEft01Init (~line 781). Their targets agree on the block's order; only arg3's base differs (siblings re-materialise D_800A11DC with lui/addu/lw %lo, saEft01Init uses the hoisted $s3).

- [s16] cpu_side_move_dir_4 honest distance is 7/160 and its residual includes the same early `lw a3` — measured with tmp/grind/saEft01Init/s16/odiff.py (artifact sibling_cpu_side_move_dir_4_diff.txt). Closing the block here would also move two other queue functions.

- [s16] Pre-sched (system.i.combine) block orders are now on disk for three attractors: named-VALUE arg4 (7) loads at the statement (insn 100, before the sw at 125); fully-inline arg4 (13) emits its address chain at 104/107/109 (AFTER arg3's) and its load at 129 (after the sw at 121); named-ADDRESS pointer (9) emits the address chain at 93/96/98 (earliest LUIDs) and the load at 131 (after the sw at 123) — target's exact expand shape.

- [s16] sched.c:3970-3975 proves schedule_block is BACKWARD: each picked insn is prepended (NEXT_INSN(insn) = last; last = insn), so the emitted order is the exact reverse of the pick order; the traces reverse to the .sched/.sched2 orders exactly on both passes.

- [s16] SCHED1 flat-priority fact: sixteen of nineteen in-block picks print pri=2130706433 (LAUNCH_PRIORITY, sched.c:3985 + birthing_insn_p sched.c:2495). Dependence-depth priority does NOT order this block at sched1.

- [s16] The ptr chassis' 9 points come down to two adjacent sched1 picks: with ready [121(l=16), 93(l=3)] it takes 121, then with ready [93(l=3), 119(l=15)] it takes 93 — so insn 93, the idx[0] lbu and the LOWEST-LUID insn in the block, lands after the whole idx[1] chain and target's head order (lbu a0,0(s1) first) is lost by one pick.

- [s16] s15's class-tie-break inertness replicates on a THIRD chassis: every RANKDBG line in the ptr chassis' sched1 block window returns val=0, including both decisive comparisons (last=125 y=121 cls=3 x=93 cls2=3; last=121 y=119 cls=3 x=93 cls2=3).

- [s16] New reusable tooling written this session: s16/blockdump.py (print the block from any -da dump), s16/probe.py (patch the do_timeout region of saEft01Init only, dump, print — anchored on the function so it cannot hit cpu_side_move_dir_4's identically-named label), s16/odiff.py (per-index sandbox-vs-target disassembly diff for any function), s16/collect.sh.

## Session 17 (escalation) — measured, then DISPOSED

Re-applied `memory/grind/saEft01Init/candidate.c` to `src/system.c` and
re-measured: `sandbox saEft01Init --disable all` → **score 7, target_insns 91,
build_insns 91, rules_dropped 15**. (HEAD's committed body — the pre-session-9
two-named-args form without the wrapper — measures 18 / 91, so the candidate
must be applied before any probe; do not measure against HEAD.)

### H45 — KILLED (a genuinely un-tried axis, now closed on both halves)

**Statement:** the second index read can be spelled through the SEPARATE byte
global `D_800A1495` (which `src/system.c` already declares at line 778 and this
function has never referenced) instead of through `idx_1494[1]`, and since two
adjacent byte globals is a plausible ORIGINAL object model, this may reorder the
argument block's address chains without paying the callee-save rotation that
s13's table-global probe paid.

**Mechanism:** every prior global-spelling probe in this ledger — s1's H1
(`inline-globals-kills-hoisted-base-pointers`, 40 / 85) and s13's permuter find
(`permuter-global-arg5-rotates-callee-save-map-17-92`) — respelled the TABLE
reference `D_800A125C`. The INDEX reads had never been touched. Respelling only
the index leaves both table bases hoisted, so the two allocnos that carry the
callee-save map are untouched; only `idx_1494`'s own ref count moves.

**Probe:** two variants, each a one-line edit on the candidate chassis, each
scored with `sandbox saEft01Init --disable all`:
 - p1 — arg5's index only: `debug_printf(..., arg4, tbl_125c[D_800A1495]);`
 - p2 — both indices: `arg4 = tbl_125c[D_800A1494];` as well, so the hoisted
   `u8 *idx_1494` disappears from the function entirely.

**Result:** p1 = **14 / 92**. p2 = **24 / 89**. The candidate was restored and
re-verified at 7 / 91 afterwards.

**Verdict: KILLED.** p1 fails for the reason s13's table-global probe failed,
one register earlier in the chain: the second reference to the array base is
what gives `idx_1494` its two in-loop refs (five after the `do{}while(0)`
`loop_depth` weighting), and dropping it to a single ref collapses that
allocno's priority while the separate `%hi/%lo` chain for `D_800A1495` adds an
instruction. Banked:
`rejected/arg5-index-via-separate-D_800A1495-global-14-92.c` and
`rejected/both-indices-via-globals-drops-idx-base-24-89.c`.

### H46 — CONFIRMED (the useful half of H45's negative result)

**Statement:** target's own instruction count tells us whether the original
source held the two indices in ONE array/pointer or in TWO independent scalars.

**Mechanism:** a hoisted base costs a prologue `lui`/`addiu` pair. If the
original used two independent byte globals there would be no index base to
hoist, and a faithful C spelling of that model would come out two instructions
SHORTER than target.

**Probe:** H45's p2 (both indices through globals, no `idx_1494` base).

**Result:** **89 build insns against target's 91** — exactly two short, and the
two missing instructions are the index base's `lui`/`addiu` pair. This is the
same shape s1's H1 measured for the TABLE bases (85 vs 91) reproduced for the
INDEX base.

**Verdict: CONFIRMED.** All three hoisted bases (`tbl_11dc`, `idx_1494`,
`tbl_125c`) are structurally REQUIRED by target's instruction count, not merely
score-preferred. Any future session that is tempted to "simplify" the pointer
block by inlining a global reference is contradicting a measured fact — the
count moves against you before the score does.

### Endgame-lock gate evaluation (the mandated work of an escalation session)

**Gate 1 — hand-coded-asm signals: FAIL.** `python3 tools/scan_hand_coded.py
--single saEft01Init` → `tier=LOW score=1/8`. The only signal set is S4 (four
loads in an 8-insn window @ insn 46) and that window IS the `debug_printf`
argument block, i.e. GCC 2.7.2's ordinary `load_register_parameters` shape, not
a hand-written-asm artefact. None of the STRONG S1/S2/S6 signals fire. The
engine's canonical gate routes the function **C**. The same gate was already
ruled on directly for this function in the **2026-07-09 01:25** decision
(`docs/grind/decisions.md:8`), which DENIED canonical-asm for the twin family
`{cpu_side_move_dir_4, marionation_Exec, saEft01Init}` on three independently
dispositive grounds; nothing measured since changes any input to it.

**Gate 2 — in-hand SOTN-master precedent for the closing construct: FAIL.**
There is no closing construct. The honest floor is 7 / 91, not 0, so no
construct closes the function and no file+line citation exists to offer. (The
candidate's one non-obvious construct, the `/* FAKE */`-annotated single
`do { } while (0)` wrapper, is already covered by the owner's 2026-07-06 ruling
and is a floor-LOWERING device, never claimed as a closing one. F23 — that it
still owes a fresh adversarial `cheat-reviewer` — remains open but is moot for
this disposition.)

**What holds the byte-match on main:** 15 `regfix.txt` rules, lines 97–119; zero
`asmfix.txt` rules; zero cheat-asm in this function's body. Two callee-save
swaps + six prologue/epilogue stack-offset substs are downstream paperwork for
the register rotation; the load-bearing residual is the 14-instruction argument
-block `reorder` at line 113 plus the four `$2<->$3` / `$2<->$4` renames feeding
it — exactly the 7 the sandbox charges.

**Disposition:** both gates FAIL, which is the owner's pre-decided
REFUSED / OWNER-ACCEPTED INCOMPLETE case under the standing 2026-07-27 ruling.
Filed `docs/grind/decisions.md:2789` —
`## 2026-08-01 — saEft01Init (src/system.c) — **OWNER-ESCALATION — RESOLVED BY
STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE**` — and
returned `owner-gated`. The driver parks the function terminally; nothing is
pending on the owner. This is expressly not a claim of unmatchability: per
`no-compiler-divergence` and `difficult-is-not-impossible` the matching pure C
exists; what is exhausted is this project's search of it across 17 sessions and
six distinct modalities. If the function is ever un-parked, F29 (build and
VALIDATE the sched1 replay simulator against the four banked chassis dumps, then
invert it) is where the next session starts — not another argument-spelling
sweep and not the permuter.

- [s17] HEAD's committed body (the pre-session-9 two-named-args form, no wrapper) measures 18 / 91. memory/grind/saEft01Init/candidate.c must be applied to src/system.c before any probe; applied and re-measured this session at exactly score 7, target_insns 91, build_insns 91, rules_dropped 15.

- [s17] tools/scan_hand_coded.py --single saEft01Init: tier=LOW, score=1/8. Only S4 set (4 loads in an 8-insn window @ insn 46). S1 multu pacing, S2 empty branch, S3 no-spills, S5 cluster, S6 BIOS jumptable, S7 unsaved $sN, S8 redundant mask all unset. 91 insns, 6 spills, 10 distinct regs.

- [s17] What holds the byte-match on main: 15 regfix.txt rules, lines 97-119; zero asmfix.txt rules; zero cheat-asm in this function's body. Decomposition: 2 callee-save swaps ($16<->$18, $16<->$17) + 3 prologue stack-offset substs + 4 $2<->$3 / $2<->$4 renames over idx 35-42 + 1 subst '0($3)'->'0($2)' @44 + the 14-instruction reorder 36,35,37,38,39,40,44,43,41,45,46,47,48,42 @35-48 + 3 epilogue stack-offset substs + reorder 72,70,71 @70-72. The prologue/epilogue substs and callee-save swaps are downstream paperwork for the register rotation; the load-bearing residual is the argument-block reorder plus its four renames — exactly the 7 the sandbox charges.

- [s17] NEW THIS SESSION (the index-global axis, never probed in 16 prior sessions): arg5's index via the separate global D_800A1495 = 14 / 92; both indices via globals (dropping the idx_1494 base) = 24 / 89. The 89 is the substantive finding — two instructions SHORT of target, so target demonstrably DOES hoist an index base and the original held the two indices in one array, not two scalars.

- [s17] Exhaustion, from the ledger not asserted: 17 sessions; floor flat at 7 / 91 for nine consecutive sessions (s9-s17); six distinct modalities all measured dead on this residual (structural s2/s3/s11/s12, permuter s4/s5/s13/s14, forensics s6/s7/s15/s16, rederive s8/s9, synthesis s10, escalation s17); ~114,000 permuter iterations across four fresh-seed campaigns under two different objectives, including s14's re-aligned objective (REGALLOC 1 / REORDERING 2 / INSERTION-DELETION 4 via a PYTHONPATH shim, no write to tools/) which is monotone with the sandbox ACROSS chassis (23<->7, 29<->9) but still does not descend WITHIN the basin (31.7k iterations, only ties at 23, all four screening to sandbox exactly 7); 67 argument-block spellings over four rigid attractors (named-VALUE 7, named-ADDRESS 9, fully-inline 13, const/RTX_UNCHANGING_P 9); 60+ disproven forms banked in memory/grind/saEft01Init/rejected/.

- [s17] Axes previously measured dead and banked, each with its own probe set: do{}while(0) wrapper EXTENT (7 forms, byte-inert both directions); arg3 tbl_11dc[D_800A11D5] spelling (8 forms, 10-15); arg1 / format-string address (3 forms, all 7); arg2 (costs an instruction, 90); pointer DECLARATION order (byte-inert — but INITIALISATION order is live and the banked order is the unique optimum, the five permutations scoring 8/11/11/12/12); MEM_IN_STRUCT_P via pointer-to-array retyping (6 forms, all 7); pre-loop global-store placement (4 forms); debug_printf's PROTOTYPE (varargs and K&R byte-identical to the fixed 5-arg form against three bodies); const / RTX_UNCHANGING_P measured to the dependence edge.

- [s17] Forensic model of the residual (instrumented cc1, verified codegen-identical to the frozen build/cc1): rank_for_schedule's load/store dependence-CLASS tie-break is totally inert here (every in-block RANKDBG returns val=0 — 54/54, 49/49, and again on a third chassis); at sched1 the block's INSN_PRIORITYs are FLAT (16 of 19 picks carry LAUNCH_PRIORITY 0x7F000001 because sched.c:3985 + birthing_insn_p re-raise every register-birthing insn while reload_completed == 0), so order at sched1 = latency-queue release + INSN_LUID and nothing else; and the expand-time split target needs is ALREADY achieved by the named-ADDRESS-pointer form (address chain at the block's three earliest LUIDs, load deferred to load_register_parameters) which still scores 9. The residual is 100% a scheduling outcome with no remaining C-visible input.

- [s17] The identical 14-instruction block with the identical target order and the same early-lw-a3 failure occurs in two other queue functions (cpu_side_move_dir_4, marionation_Exec), one of which names both arguments and has no do{}while(0) wrapper. Per the 2026-07-09 ruling cluster precedent is disqualified as authorization evidence; recorded only so that a lever found on any one of the three is re-tried on the other two.

- [s17] DISPOSITION FILED: docs/grind/decisions.md line 2789 — '## 2026-08-01 — saEft01Init (src/system.c) — **OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE**'. Both gates' evidence and the full exhaustion record are in the entry. Nothing is pending on the owner.

- [s17] src/system.c was reverted to HEAD before finishing, so the working tree stays oracle-clean for the driver's park commit; the 7 / 91 body is preserved in memory/grind/saEft01Init/candidate.c with an s17 header section.

---

## Session 18 (2026-08-25) — modality: escalation (owner directive executed)

- [s18] CHASSIS RE-MEASURED ON THE MIGRATED TU. HEAD is `INCLUDE_ASM("asm/funcs", CD_datasync);` and scores 91/91 (`no_c_body: true`). src/system.c has been renamed since the ledger's last chassis note: `saEft01Init` -> `CD_datasync`, `sys_VSync` -> `VSync`, `tslTm2LoadImage_2` -> `puts`, `debug_printf` -> `printf`, `cdrom_ClearIrq` -> `CD_flush` (and `CD_flush` is now DEFINED in the same TU at src/system.c:403, an intra-TU `jal` rather than an extern), `&D_800161B8` -> `&g_str_cd_timeout`. The session-9 body transcribed into those names re-measures at **score 7, target_insns 91, build_insns 91, rules_dropped 0** — so the 7/91 floor is invariant across the asm-until-matched migration AND across CD_flush becoming an intra-TU call. memory/grind/CD_datasync/candidate.c has been rewritten in the migrated names; the old names no longer compile.

- [s18] OWNER DIRECTIVE, HALF 1 (SOTN LIBCD reference) — already executed and measured in the ledger, deliberately not re-run: session 7 transcribed `memory/grind/CD_datasync/ref/sotn_libcd_bios_CD_datasync.c` into BB2 and measured 27-35 honest distance; session 8 exhausted the whole 5-project reference corpus at the same cost; the 2026-08-19 read-only research note (`research-psyz-axis-2026-08-19.md`) re-confirmed psyz leaves CD_datasync as INCLUDE_ASM, so there is no PsyQ-4.0 matched body to transplant. The reference is a provenance/naming oracle, not a score lever.

- [s18] OWNER DIRECTIVE, HALF 2 (F29 — sched1 LUID-order replay) — EXECUTED, and the model is now empirical rather than hypothetical. The sched1 pick rule was decoded directly out of the banked `-da` trace (tmp/grind/saEft01Init/s16/s16cand/system.i.sched, `;; Function saEft01Init`, basic blocks 0 and 3) and VALIDATED on block 0 before use: the pick is always the FIRST element of the printed `now` list, and the `now` list is the ready list sorted by DESCENDING priority with ties broken by DESCENDING insn UID. Because the pass is backward and prepends, a descending-UID tie-break means ties PRESERVE the original LUID order. Worked examples in the trace: `133 (7f000001) 129 (7f000001) 127 (1) -> now 133 129 127`; `100 (7f) 108 (7f) 123 (7f) -> now 123 108 100`; `96 (7f) 116 (7f) -> now 116 96`. So sched1's output order = pre-sched LUID order, deviated ONLY by the latency queue and the "greater potential hazard" / "blocking insn N for k cycles" reorderings.

- [s18] F29 — THE FULL PICK TRACE OF THE RESIDUAL BLOCK IS DECODED. Basic block 3 (insns 77..154) is the printf block, 23 schedulable insns. Backward picks, in order: 154, 152, 138, 135, 133, 131, 129, 113, 105, 111, 125, 127, 123, 108, 100, 121, 119, 98, 116, 96, 93, 89, 87. Reversing gives the emitted forward order 87, 89, 93, 96, 116, 98, 119, 121, 100, 108, 123, 127, 125, 111, 105, 113, 129, 131, 133, 135, 138, 152, 154 — exactly the block the sandbox builds.

- [s18] F29 — THE PRE-SCHED RTL AND ITS LOG_LINKS ARE ON DISK AND DECODED (tmp/grind/CD_datasync/s18/rtl.py against the banked .combine). The block's dependence DAG is four independent chains plus the call: 93 (lbu idx_1494[0]) -> 96 (sll 2) -> 98 (+tbl_125c) -> 100 (lw, arg4 value) -> 133 (a3); 116 (lbu idx_1494[1]) -> 119 (sll 2) -> 121 (+tbl_125c) -> 123 (lw, arg5 value) -> 125 (sw 16(sp)); 108 (lbu D_800A11D5) -> 111 (sll 2) -> 113 (+tbl_11dc) -> 131 (lw a2); 105 (lw D_800F19C0) -> 129 (a1); 127 (a0 = fmt symbol) -> 135 (call).

- [s18] F29 — TARGET'S ORDER MAPPED INTO THE SAME RTL NUMBERING, AND PROVED SCHEDULABLE. Target's 14-insn window (asm/funcs/CD_datasync.s, 0x80081C68-0x80081CA4) is `lbu a0,0(s1) / lbu v0,1(s1) / lui+lw a1 / sll v0,2 / addu v0,s0 / sll a0,2 / lw v1,0(v0) / lui+lbu D_800A11D5 / addu a0,s0 / sll v0,2 / addu v0,s3 / sw v1,16(sp) / lw a2,0(v0) / lw a3,0(a0)` = 93, 116, 105, 119, 121, 96, 123, 108, 98, 111, 113, 125, 131, 100. Every chain edge above is respected, so target's order IS a valid topological order of the candidate's own dependence DAG: the DAG is not the obstruction — the pre-sched LUID order is. The two structural facts that separate target from every BB2 form are (a) target defers arg4's load (100) to the very LAST slot of the block, immediately before the a0/fmt materialisation, and (b) target splits arg4's address chain maximally, keeping it live in `$a0` across the whole block (lbu a0 ... sll a0 ... addu a0 ... lw a3,0(a0)), where every BB2 form runs that chain contiguously through `$v0`.

- [s18] F29's C-SIDE PREDICTIONS MEASURED AND KILLED — 14 new spellings, all >= 7, all banked in memory/grind/CD_datasync/rejected/ with `s18-` prefixes. (i) Index-naming family (NEW — never probed in 17 sessions; s17 only probed indices spelled through separate GLOBALS): naming `idx_1494[0]` and/or `idx_1494[1]` as `s32` locals is codegen-IDENTICAL to the fully-inline attractor — v1 both = 13, v4 idx1 only = 13, v5 idx0 only = 13. A named index local is therefore INERT: GCC folds the local straight back into the address chain and the statement boundary buys no LUID separation. (ii) Which argument is the pre-call named local: naming arg5 instead of arg4 = 13 (v2); naming BOTH with arg5 assigned first = 8 (v3) — v3's emitted block simply swaps the two `lbu` and still issues `lw a3` early. (iii) Pointer-ADDRESS family crossed with statement order (5 forms): w1 arg5-value-then-p4-address = 9, w2 p4-address-then-arg5-value = 9, w4 p4-address-only = 9, x3 `p4 = tbl_125c + idx_1494[0]` = 9, x4 p4-address-then-named-stack-arg5 = 9; making arg5 an address too costs one more (w3 = 10, w5 = 10). The address attractor is RIGID at 9 regardless of statement order — the order of the two initialisations does not move it at all, which is a stronger statement than the ledger's earlier "initialisation order is live". (iv) arg3 combinations: x1 p3+p4 addresses = 13, x2 arg3-value + p4-address = 12.

- [s18] w4 (p4-address-only, 9) IS STRUCTURALLY THE CLOSEST FORM EVER MEASURED and shows exactly what the address split buys and what it does not. Its block is `lbu v0,1(s1) / lbu v1,0(s1) / lui+lw a1 / sll v0,2 / addu v0,s0 / sll v1,2 / lw v0,0(v0) / addu v1,s0,v1 / sw v0,16(sp) / lui+lbu / lw a3,0(v1) / sll v0,2 / addu v0,s3 / lw a2,0(v0)`. Against target it gets arg5's value issued BEFORE arg4's load (target's shape) and defers arg4's load past the arg5 chain — but it still (a) issues the two `lbu` in the wrong order, (b) commits `sw 16(sp)` immediately after the arg5 load instead of deferring it past the arg3 address chain, (c) emits `addu v1,s0,v1` with the operands swapped relative to target's `addu a0,a0,s0`, and (d) lands `lw a3` at slot 11 rather than slot 14.

- [s18] NEW AXIS, NEVER RUN IN 17 SESSIONS — cc1psx CALIBRATION. `tools/cc1psx_wrapper.sh` (the ORIGINAL PsyQ `cc1psx.exe`, GCC 2.7.2.SN.1, via dosemu2 — sanctioned for calibration / self-disproof by the cc1psx-calibration-only policy) had never been pointed at this function. Run this session on the byte-identical preprocessed `system.i` (project CPP flags; `-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -w`), against the frozen open-port cc1 output for the same input:
  - open-port cc1 block: `lbu $3,1($17) / lw $5 / sll $2 / addu $2,$16 / sll $3 / addu $3,$16 / lw $7,0($2) / lbu $2 / lw $3,0($3) / sll $2 / addu $2,$19 / sw $3,16($sp) / lw $6,0($2)`
  - cc1psx block: `lbu $3,0($17) / lbu $2,1($17) / lw $5 / sll $3 / sll $2 / addu $2,$16 / lw $4,0($2) / lbu $2 / addu $3,$16 / lw $7,0($3) / sll $2 / addu $2,$19 / sw $4,16($sp) / lw $6,0($2)`
  - target block: `lbu $4,0($17) / lbu $2,1($17) / lw $5 / sll $2 / addu $2,$16 / sll $4 / lw $3,0($2) / lbu $2 / addu $4,$16 / sll $2 / addu $2,$19 / sw $3,16($sp) / lw $6,0($2) / lw $7,0($4)`

  The two compilers produce DIFFERENT schedules for this block from byte-identical C, and NEITHER equals target. This is the first hard evidence in 18 sessions that the residual block is fork-SENSITIVE: every one of the ~81 C spellings searched to date was scored against a compiler that provably diverges from the one that built the target on exactly this construct. It also disposes of the lazy reading of that fact — cc1psx is not a free match, so routing the TU through the Makefile's `CC1_PSX_FILES` gate would not close the function on the current C; what it changes is the SEARCH ORACLE. Note also that cc1psx is strictly closer to target than the open port on two counts the open port gets wrong: it emits the two `lbu` in target's order (idx0 first) and it issues arg5's value load before arg4's.
  Artifacts: tmp/grind/CD_datasync/s18/{psx.sh, system.i, open.s, psx.s, cmp.py, rtl.py, blk.py, built.dis}.

- [s18] DISPOSITION REASONING (why no accept-the-debt entry was re-filed). The s17 entry `docs/grind/decisions.md:2796` — `OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE` — is already in the record, and the owner REVERSED that disposition for this exact function on 2026-08-24 (`escalation-not-parked`: the parked state is retired; CD_datasync was kicked back to active with a directive to exploit the SOTN reference and F29). Under the same 2026-08-24 ruling an accept-the-debt packet is in the AUTO-REJECT class and must NOT be filed; that residual stays ACTIVE. Both endgame-lock gates still FAIL exactly as s17 measured them (`scan_hand_coded --single` tier LOW 1/8, S4 only; no closing construct, so no SOTN precedent is citable even in principle), but this session found a live, un-tried axis — cc1psx as a CALIBRATION search oracle, which needs no ruling because calibration/self-disproof is already sanctioned — so the honest outcome is `progress` with the kills banked, not a disposition.

- [s18] CHASSIS RE-MEASURED ON THE MIGRATED TU. HEAD is INCLUDE_ASM("asm/funcs", CD_datasync) and scores 91/91 (no_c_body). src/system.c has been renamed since the ledger's last chassis note: saEft01Init -> CD_datasync, sys_VSync -> VSync, tslTm2LoadImage_2 -> puts, debug_printf -> printf, cdrom_ClearIrq -> CD_flush (now DEFINED in the same TU at src/system.c:403, an intra-TU jal rather than an extern), &D_800161B8 -> &g_str_cd_timeout. The session-9 body transcribed into those names re-measures at score 7, target_insns 91, build_insns 91, rules_dropped 0 - the floor is invariant across the asm-until-matched migration AND across CD_flush becoming an intra-TU call. candidate.c has been rewritten in the migrated names; the old names no longer compile.

- [s18] OWNER DIRECTIVE HALF 1 (SOTN LIBCD reference) is already executed and measured in the ledger and was deliberately not re-run: s7 transcribed memory/grind/CD_datasync/ref/sotn_libcd_bios_CD_datasync.c into BB2 at 27-35 honest distance, s8 exhausted the whole 5-project reference corpus at the same cost, and research-psyz-axis-2026-08-19.md re-confirmed psyz leaves CD_datasync as INCLUDE_ASM. The reference is a provenance/naming oracle, not a score lever.

- [s18] OWNER DIRECTIVE HALF 2 (F29) is EXECUTED and the model is now empirical. sched1's pick is always ready[0]; the ready list is sorted descending by priority with ties by DESCENDING insn UID; backward pass + prepend therefore PRESERVES pre-sched LUID order, and the only sources of deviation are the latency queue and the potential-hazard swap. Validated on basic block 0 before use.

- [s18] The residual block's pre-sched dependence DAG is four independent chains plus the call: 93(lbu idx_1494[0])->96(sll)->98(+tbl_125c)->100(lw arg4)->133(a3); 116(lbu idx_1494[1])->119(sll)->121(+tbl_125c)->123(lw arg5)->125(sw 16(sp)); 108(lbu D_800A11D5)->111(sll)->113(+tbl_11dc)->131(lw a2); 105(lw D_800F19C0)->129(a1); 127(a0=fmt)->135(call).

- [s18] Target's 14-insn window (asm/funcs/CD_datasync.s, 0x80081C68-0x80081CA4) is RTL order 93,116,105,119,121,96,123,108,98,111,113,125,131,100 and respects every edge of that DAG, so target's order is a valid topological order of our own dependence graph - the DAG is not the obstruction, the emitted LUID order is.

- [s18] 14 new spellings measured and banked to memory/grind/CD_datasync/rejected/ with s18- prefixes: named-both-indices 13, named-idx1 13, named-idx0 13, named-arg5-value 13, both-named-arg5-first 8, arg5-value-then-p4-address 9, p4-address-then-arg5-value 9, p5-then-p4-addresses 10, p4-address-only 9, p4-then-p5-addresses 10, p3+p4-addresses 13, arg3-value+p4-address 12, p4-as-tbl-plus-index 9, p4-address-then-named-stack-arg5 9.

- [s18] NAMED INDEX LOCALS ARE CODEGEN-INERT (new, and distinct from s17's separate-global index probe): naming idx_1494[0] and/or idx_1494[1] as s32 locals reproduces the fully-inline attractor exactly (13/91 in all three spellings). The statement boundary buys no LUID separation; GCC folds the local straight back into the address chain.

- [s18] THE POINTER-ADDRESS ATTRACTOR IS RIGID AT 9 UNDER EVERY STATEMENT ORDER (five distinct spellings all 9/91 exactly). This supersedes the ledger's earlier 'initialisation order is live, the banked order is the unique optimum' - that was measured on the VALUE form; on the ADDRESS form order is completely inert. Making arg5 an address as well costs exactly one position (10/91).

- [s18] w4 (p4-address-only, 9/91) is the structurally closest form ever measured: `lbu v0,1(s1) / lbu v1,0(s1) / lui+lw a1 / sll v0,2 / addu v0,s0 / sll v1,2 / lw v0,0(v0) / addu v1,s0,v1 / sw v0,16(sp) / lui+lbu / lw a3,0(v1) / sll v0,2 / addu v0,s3 / lw a2,0(v0)`. It achieves target's arg5-before-arg4 issue order and defers arg4's load past the arg5 chain, but misses on the lbu order, an undeferred sw 16(sp), swapped addu operands (addu v1,s0,v1 vs target's addu a0,a0,s0), and lw a3 at slot 11 rather than 14.

- [s18] NEW AXIS NEVER RUN IN 17 SESSIONS - cc1psx CALIBRATION. tools/cc1psx_wrapper.sh (PsyQ cc1psx.exe, GCC 2.7.2.SN.1, dosemu2; sanctioned for calibration/self-disproof) had never been pointed at this function. On byte-identical preprocessed input the open-port cc1 and cc1psx emit DIFFERENT schedules for the residual block, and neither equals target. cc1psx is strictly closer (correct lbu order, arg5 value before arg4) but still issues lw $7 at slot 10 instead of 14. The block is fork-sensitive; the ~81-spelling search to date was scored against a diverging compiler.

- [s18] DISPOSITION REASONING: both endgame-lock gates still FAIL exactly as s17 measured them (scan_hand_coded --single tier LOW score 1/8, S4 only, no S1/S2/S6; no closing construct exists at floor 7 so no SOTN-master precedent is citable even in principle). No accept-the-debt entry was re-filed: the s17 entry docs/grind/decisions.md:2796 already carries that disposition, the owner REVERSED it for this exact function on 2026-08-24 (escalation-not-parked: parked state retired, CD_datasync kicked back to active with the SOTN-reference + F29 directive), and under that same ruling an accept-the-debt packet is in the AUTO-REJECT class and must NOT be filed. This session found a live un-tried axis (cc1psx as a calibration SEARCH ORACLE, which needs no ruling because calibration is already sanctioned), so the item is still grindable and progress is the honest outcome.

- [s18] The working tree was restored to HEAD (git checkout -- src/system.c) before finishing; only memory/grind/CD_datasync/ files were written.

- [campaign-sweep 2026-08-30] First solver-chain run on this function
  (sched-tie endgame campaign spec, docs/superpowers/specs/). Chassis: candidate.c
  (session-9 zero-constant-lever form) spliced under current callee names —
  sandbox 7, 91/91, floor confirmed live. Artifacts: tmp/grind/CD_datasync/s19_campaign/.
  **FAMILY CONFIRMATION (third sibling):** the entire residual sits in the SAME
  debug_printf window as CD_sync and CD_ready (both enumerated same day, see
  their evidence banks): normalized diff sites 46-62 = lbu/lbu + sll/addu/lw
  cluster, with the idx-chain pseudo in $v1 where target wants $a0 — the exact
  exchange the twins enumerate — PLUS a second rotation ($v1->$v0 on the other
  lbu) and a 2-slot order component (addu/lw swap vs sites 53/57). Any lever
  proven on CD_sync/CD_ready transfers here first.
  **Tool-limit finding (recorded for the campaign):** perturb.py --goal-from-target
  SKIPPED the divergent block ("goal is not a topological order, 2 violations")
  — the hop-3 difflib alignment mis-pairs duplicate instruction text in this
  window (two slls, repeated lw skeletons). CD_datasync needs either a
  hand-verified --goal-order for that block or an alignment upgrade
  (anchor-based, not similarity-based) before its sched half can be typed.
  **RA goal (goal_from_tgt on this chassis):** 4 renamed pairs + 2
  skeleton-differ (= the order component): $v0->$a0 x2, $v1->$a0 x2, $v1->$v0
  x1 — a 3-register rotation, attribution ambiguous (15/8 candidate pseudos).
  Next session: resolve attribution via QTYDBG blk dump (the window is local-
  alloc territory per the twins), then run inverse.py local with the resolved
  goal; check the twins' p104-demotion/live-split axes for direct transfer
  before any new spelling.

## s19 (escalation / disposition) — 2026-08-30

- [s19] OWNER DIRECTIVE EXECUTED. The 2026-08-30 escalation-batch **ruling 10**
  (docs/grind/decisions.md:14870) returned CD_datasync to ACTIVE-with-modality-change because the
  latest ledger entry (the 2026-08-25 driver auto-filed exhaustion backstop) contained no question
  pending the owner. This session executed the directive by running the ledger's top un-executed
  frontier item **F31** (the cc1psx calibration oracle) to a measured kill, then filed the
  standing-ruling disposition (docs/grind/decisions.md, 2026-08-30 CD_datasync entry).

- [s19] LIVE FLOOR RE-MEASURED: candidate.c spliced into src/system.c,
  `sandbox CD_datasync --disable all` -> **score 7, build_insns 91, target_insns 91,
  rules_dropped 0, cheat_asm_stripped 5** (the 5 stripped blocks are other functions in system.c,
  not CD_datasync). The ledger floor of 7 is chassis-current, not stale. src/system.c restored to
  HEAD afterwards (tree clean apart from engine-written metrics/events.jsonl).

- [s19] NEW TOOL — dual-fork calibration harness (reusable for ANY function in this repo):
  `tmp/grind/CD_datasync/s19/psx.sh` runs ONE `mipsel-linux-gnu-cpp` pass and feeds the result to
  BOTH `tools/gcc-2.7.2/build/cc1` (open port, `-mel`) and `tools/cc1psx_wrapper.sh` (PsyQ cc1psx
  2.7.2.SN.1). `score.py` canonicalises cc1 assembly (numeric->named registers, `move`/`li`/`subu`
  ->canonical forms, hex->decimal immediates, `%hi`/`%lo` symbol masking, label masking) and
  `expand.py` additionally expands the assembler MACROS cc1 emits (`la R,S` -> `lui/addiu`;
  symbol-form `lw`/`sw`/`lbu` -> `lui` + `%lo` form; `li` of a 0x----0000 constant -> `lui`;
  `j $ra` -> `jr $ra`), which is what makes pre-maspsx output comparable instruction-for-
  instruction with `asm/funcs/*.s`. `sweep.sh` scores a list of body files on both forks at
  ~1.5 s/form — roughly 15x cheaper than a sandbox run.
  CALIBRATION: on the banked candidate the harness reports 82 insns for BOTH forks vs target's 82,
  with LCS distance 6 against the sandbox's masked 7. The metric tracks the sandbox; it is not an
  invented scale.

- [s19] **F31 KILLED — negative #1 (same distance).** On the banked candidate, open-port cc1 and
  cc1psx are EQUALLY far from target: dist 6 each. s18's claim that "cc1psx is strictly closer than
  the open port on two counts" does NOT survive macro-expansion normalisation — it was an artifact
  of diffing macro-form cc1 output against already-expanded target bytes.

- [s19] **F31 KILLED — negative #2 (same residual in KIND).** Expanded opcode diffs, both forks:
    OPEN: [41:43] `lbu $v0,0($s1)` / `lbu $v1,1($s1)`  <> TGT `lbu $a0,0($s1)` / `lbu $v0,1($s1)`
          [47:50] `sll $v1,$v1,2` / `addu $v1,$v1,$s0` / `lw $a3,0($v0)` <> TGT `sll $a0,$a0,2` /
                  `lw $v1,0($v0)`
          [52:53] `lw $v1,0($v1)` <> TGT `addu $a0,$a0,$s0`;  missing TGT `lw $a3,0($a0)` at 57.
    PSX:  [41:42] `lbu $v1,0($s1)` <> TGT `lbu $a0,0($s1)` (note: PSX DOES match target's second
          `lbu $v0,1($s1)`, the open port does not) ; then the same early `lw $a3,0($v1)`,
          `sw $a0,16($sp)` instead of `sw $v1,16($sp)`, and the same missing final `lw $a3,0($a0)`.
  Both forks allocate target's `$a0` idx-0 address chain to `$v0`/`$v1` and issue its `lw $a3`
  EARLY; NEITHER ever emits `lw $a3,0($a0)` as the block's last memory reference. The residual is
  not a fork artifact — it is the same sched/RA attractor s9-s18 mapped on the open port.

- [s19] **F31 KILLED — negative #3 (perfect ranking correlation, identical floor).** 23 body
  spellings scored on both forks: the 15 banked s9/s18 forms (base, v1-v5, w1-w5, x1-x4) plus 8
  written this session (n1 arg3-named 11/10; n2 arg5-named+arg4-inline 10/10; n3 arg5-named then
  p4-address 6/6; n4 second base pointer for arg4 10/10; n5 arg5-named + idx0 byte named 10/10;
  n6 arg5-then-arg3 named 11/10; n7 arg4 staged before the puts 28/23 and +3 insns; n8 idx1 byte
  named only 10/10 — open/psx). 19 of 23 scored IDENTICALLY on the two forks, 3 had cc1psx exactly
  1 lower, 1 (n7) had it 5 lower. **No form on either fork went below the shared minimum of 6.**
  The form ranking is the same on both compilers, so a cc1psx-scored gradient is not a different
  search space and the 18-session-flat open-port floor is NOT explained by the fork. 8 new forms
  banked as `memory/grind/CD_datasync/rejected/s19-dualfork-*.c` (bank now 84).

- [s19] GATE (a) canonical-asm RE-MEASURED FAILING: `python3 tools/scan_hand_coded.py --single
  CD_datasync` -> tier=LOW score=1/8 (91 insns). Only S4 (4 loads in an 8-insn window @ insn 46)
  fires, and that window IS the printf argument block under study. S1/S2/S6 (the only STRONG-tier
  carriers) absent; S3/S5/S7/S8 absent. Independently, provenance is compiled Sony C (PsyQ LIBCD
  CD_datasync), and the twin-family canonical-asm request was already DENIED 2026-07-09
  (docs/grind/decisions.md:15).

- [s19] GATE (b) SOTN precedent FAILS BY CONSTRUCTION: the residual is a sched.c emission-order +
  global.c/local-alloc.c seat outcome with no C-level closing construct, so there is no construct
  for which a precedent could be cited. The constructs that HOLD the floor at 7 (one FAKE-annotated
  `do{}while(0)` wrap + three plain pointer locals) are already in sanctioned families and are not
  the blocker.

- [s19] DISPOSITION FILED: REFUSED / OWNER-ACCEPTED INCOMPLETE under the 2026-07-27 standing ruling
  (docs/grind/decisions.md, 2026-08-30 CD_datasync entry). No decision packet — the only questions
  the residual could pose are standard-lowering and PRE-DECIDED NO under the 2026-08-24 auto-reject
  ruling. Re-activation trigger: a toolchain model that can search the JOINT sched.c-order x
  global.c-seat space (identical to the trigger recorded for the twin CD_ready on 2026-08-30).

- [s19] LIVE FLOOR re-measured this session: candidate.c spliced into src/system.c, `sandbox CD_datasync --disable all` -> score 7, build_insns 91, target_insns 91, rules_dropped 0 (cheat_asm_stripped 5 are other functions in system.c). The ledger floor of 7 is chassis-current. src/system.c restored to HEAD afterwards; tree clean apart from engine-written metrics/events.jsonl.

- [s19] GATE (a) canonical-asm FAILS, re-measured: `python3 tools/scan_hand_coded.py --single CD_datasync` -> tier=LOW score=1/8 (91 insns). Only S4 (4 loads in an 8-insn window @ insn 46) fires, and that window IS the printf argument block under study. S1 (multu pacing), S2 (empty branch), S6 (BIOS jumptable) - the only STRONG-tier carriers - all absent, as are S3/S5/S7/S8. Provenance independently defeats the claim: this body is compiled Sony C (PsyQ LIBCD CD_datasync, 91 words = exactly 0x14F4-0x1388 in the library object, memory/closer/libcd-groundtruth.md:59-67), and the twin-family canonical-asm request covering this exact function was already DENIED on three independently dispositive grounds (docs/grind/decisions.md:15, 2026-07-09).

- [s19] GATE (b) in-hand SOTN-master precedent FAILS BY CONSTRUCTION: the residual is a sched.c emission-order plus global.c/local-alloc.c seat outcome inside a call's argument block, with no C-level closing construct - so there is no construct for which a precedent could be cited. 23 argument spellings across the named/inline/pointer/byte-named/base-pointer/statement-order axes land on the same attractor on BOTH compiler forks. The constructs that HOLD the floor at 7 (one FAKE-annotated do{}while(0) wrap plus three plain pointer locals) are already inside sanctioned families and are not the blocker.

- [s19] NEW REUSABLE TOOL: the dual-fork calibration harness in tmp/grind/CD_datasync/s19/ (psx.sh + score.py + expand.py + sweep.sh) scores a body file against target on BOTH the open-port cc1 and PsyQ cc1psx at ~1.5s/form - roughly 15x cheaper than a sandbox run - and its macro-expanding canonicalizer is function-agnostic (only the function name is hardcoded). Any future session facing a fork-sensitivity question can reuse it directly.

- [s19] The s18 frontier claim that 'cc1psx is strictly closer to target than the open port on two counts' is FALSIFIED: it compared macro-form cc1 output (la / symbol-form lw) against already-expanded target instructions. After macro expansion both forks emit exactly 82 instructions and sit at the identical distance.

- [s19] DISPOSITION FILED this session at docs/grind/decisions.md:15943 - REFUSED / OWNER-ACCEPTED INCOMPLETE under the owner's 2026-07-27 standing ruling. No decision packet was filed: the only questions this residual could pose ('sanction a new family for a scheduler tie with no C spelling', 'grant canonical-asm despite a LOW scan tier') are standard-lowering and PRE-DECIDED NO under the 2026-08-24 auto-reject ruling. Provenance is settled (PsyQ LIBCD CD_datasync), routing is settled (C, not canonical-asm), nothing waits on the owner.

- [s19] NOT an integration handoff: nothing is bytes-proven and nothing is blocked by an untouchable surface. src/system.c carries INCLUDE_ASM("asm/funcs", CD_datasync); with 0 regfix/asmfix rules. The best honest form is simply 7 masked points away and stays in memory/grind/CD_datasync/candidate.c.

- [s19] Exhaustion at disposition: 19 sessions, >=7 distinct modalities (structural, permuter, forensics, rederive, synthesis, recon, escalation), 84 banked rejected forms, the s9 twelve-spelling sweep, the s13/s14 permuter basins, the s15/s16 RANKDBG sched1 forensics (0 of 103 in-block tie-break decisions class-resolved), the s17 index-globals axis, the s18 empirical descending-UID sched1 model, and the s19 dual-fork sweep - all closed negative.

## s20 (escalation / Ruling-A named probe = Ruling D, F14 verbatim) — 2026-09-01

- [s20] CHASSIS RE-MEASURED LIVE. memory/grind/CD_datasync/candidate.c spliced into
  src/system.c (HEAD carries INCLUDE_ASM("asm/funcs", CD_datasync);) and scored:
  `sandbox CD_datasync --disable all` -> score **7**, target_insns 91, build_insns 91,
  rules_dropped 0 (cheat_asm_stripped 5 belong to other functions in the TU). The ledger floor
  of 7 is chassis-current; every conclusion below is measured against it.

- [s20] **OWNER DIRECTIVE EXECUTED IN FULL.** The 2026-09-01 FORECLOSED-BUCKET REVIEW
  (docs/grind/decisions.md:17743) Ruling A row `CD_datasync (d7)` named the probe
  "Ruling D session (F14 verbatim)". F14 (hypotheses.md:976-991) asks for Sony's aggregate
  declarations of `Intr` (typedef struct { u8 sync, ready, c; } CD_intr; at 0x800A1494/95/96)
  and `Alarm` (0x800F19B8/BC/C0) instead of the per-word splat scalars. Both halves were built
  and measured this session; neither had ever been measured on this function.

- [s20] **PRONG (c) RE-DERIVED FIRST-HAND FOR CD_datasync — the Intr merge FAILS structurally.**
  `grep -rcE 'D_800A149[456]' asm/` returns nine files: the data definition
  asm/data/7D920.data.s (10 sites, carrying dlabel D_800A1494/95/96 and D_800A1498 whose
  first word is `.word D_800A1494`), the three INCLUDE_ASM twin bodies CD_datasync.s /
  CD_ready.s / CD_sync.s (2 sites each), and **five asm-only consumers**: getintr.s (22),
  CD_cw.s (8), func_800817A0.s = CD_flush (8), func_800819C4.s (8), func_80081E1C.s (2).
  The per-word symbols must therefore survive in the splat symbol config, so any CD_intr
  declaration is necessarily a SECOND handle on the same storage — verbatim the prong the
  g_stage_id merge died on (decisions.md:10722), the prong CD_sync's Ruling D died on
  (decisions.md:18305-18317) and the prong CD_ready's Ruling D died on (CD_ready evidence [s68]).
  This is now re-derived first-hand on the third and last member of the set; the finding is
  symbol-level, not function-level, and the Ruling D scope grant is unspendable by any of the
  three twins while those five asm-only consumers exist.

- [s20] **PRONG (c) FOR THE `Alarm` HALF ALSO FAILS.** `grep -rcE 'D_800F19(B8|BC|C0)' asm/`
  returns exactly four files — CD_cw.s (14 sites), CD_datasync.s (14), CD_ready.s (14),
  CD_sync.s (14). CD_cw is an asm-only queue body with no C form, so the three per-word
  symbols must likewise survive the splat config and a CD_alarm declaration is again a second
  handle. The Alarm merge is prong-(c)-blocked for the same structural reason as Intr.

- [s20] **F14 MEASURED AND KILLED ON THE BYTES — the aggregate is either BYTE-INERT or a
  REGRESSION; it cannot reach the residual.** Seven spellings built on the candidate chassis
  (all banked in rejected/s20-f14-*.c, harness tmp/grind/CD_datasync/s20/{apply,gen}.py):

  | form | spelling | score | build_insns |
  |---|---|---|---|
  | base | candidate.c | **7** | 91 |
  | f14b | extern CD_intr D_800A1494; + CD_intr *ip = &D_800A1494;, ip->sync / ip->ready | 7 | 91 |
  | f14c | f14b with volatile CD_intr | 7 | 91 |
  | f14a | extern CD_intr D_800A1494;, direct D_800A1494.sync / .ready (no pointer local) | 24 | 89 |
  | f14d | f14a with volatile | 24 | 89 |
  | f14e | typedef struct { s32 timeout; s32 count; void *arg; } CD_alarm; extern CD_alarm D_800F19B8; + member accesses | 12 | 91 |
  | f14f | f14e with volatile | 12 | 91 |
  | f14g | both merges together | 12 | 91 |

- [s20] **THE DECISIVE MEASUREMENT: f14b and f14c are BYTE-IDENTICAL to the base**, not merely
  equal-scoring. Full-object disassemblies captured for all three
  (tmp/grind/CD_datasync/s20/dis_{base,f14b,f14c}.txt, `mipsel-linux-gnu-objdump -d` of the
  cheat-stripped sandbox object) diff clean. MECHANISM: the `u8 *idx_1494` pointer local the
  candidate already carries produces exactly the RTL the struct produces — one hoisted base
  pseudo plus lbu 0(base) / lbu 1(base). F14's stated premise ("Sony's source has ONE object
  with two member offsets — a shape this repo cannot currently express") is FALSIFIED: the repo
  already expresses it, the base-register addressing target uses is already achieved, and the
  aggregate declaration adds nothing the pointer local did not. `volatile` on the aggregate is
  likewise byte-inert here (both member reads are single, unduplicated, already ordered).
  F14 is therefore not a deferred-for-scope opportunity — it is a NO-OP on the residual, and
  the two prong-(c) failures above make it unspendable as a family claim regardless.

- [s20] The 24 / 89 result for the direct-member forms REPRODUCES the s17 measurement
  (rejected/both-indices-via-globals-drops-idx-base-24-89.c, 24 / 89): dropping the pointer
  local loses the hoisted base and the build comes out two instructions SHORT of target's 91,
  re-confirming that target demonstrably hoists an index base and that the three hoisted table
  pointers are structurally required by target's instruction count.

- [s20] **NEW, MODEL-CORRECTING FINDING — the solver chain types the FIRST divergence as RA,
  not scheduling.** `tools/ra_solver/inverse_compose.py classify system CD_datasync
  --target-object build/src/system.o --ours-object tmp/sandbox/CD_datasync/system.o` (the
  INCLUDE_ASM-routed object path; build/src/system.o verified to carry target's bytes — its
  CD_datasync head matches asm/funcs/CD_datasync.s instruction-for-instruction) returns:
      FIRST DIVERGENCE: RA   next tool: tools/ra_solver/inverse.py (global / local)
      ours   addu v1,v1,s0 / lbu v0,0(s1) / lbu v1,1(s1) / lw a3,0(v0) / lw v1,0(v1) / sll v1,v1,2
      target addu a0,a0,s0 / lbu a0,0(s1) / lbu v0,1(s1) / lw a3,0(a0) / lw v1,0(v0) / sll a0,a0,2
  This is the first time the solver suite has been pointed at CD_datasync's residual at all
  (the 2026-08-30 campaign got as far as goal_from_tgt and then STOPPED: `perturb.py
  --goal-from-target` skipped the block on a difflib alignment failure, and the recorded
  next step — "resolve attribution via QTYDBG blk dump, then run inverse.py local" — was never
  executed by s19, which spent its session on the cc1psx axis instead).

- [s20] **POSITIONAL BLOCK MAP (build idx 44-64, ours | target), captured this session to
  tmp/grind/CD_datasync/s20/blk_ours.txt / blk_tgt.txt:** the residual is one interleaving
  choice. Target computes the idx[1] (arg5) chain FIRST and COMPLETELY in $v0
  (lbu 48 / sll 51 / addu 52 / lw v1 54) while SPREADING the idx[0] (arg4) chain across the
  block in $a0 (lbu 47 / sll 53 / addu 57 / **lw a3 at 62, the block's last memory
  reference**). Ours does the mirror image: idx[0] first and complete in $v0
  (lbu 47 / sll 51 / addu 52 / lw a3 55) and idx[1] spread in $v1 (lbu 48 / sll 53 / addu 54 /
  lw v1 58). Same instruction multiset, same count, different chain-to-register assignment and
  different completion order.

- [s20] **LOCAL-ALLOC GROUND TRUTH FOR THE RESIDUAL BLOCK, extracted first-hand** (the
  2026-08-30 campaign's un-executed step): `python3 tools/ra_solver/local_extract.py system`
  -> tmp/ra_solver_work/system.local.json (40 functions, 287 qty rows). CD_datasync block 3
  is the printf argument block; its main-pass quantities are
      ord0 qty0 pseudo92  birth 8  death 20 refs 12 -> got $v0(2)
      ord1 qty1 pseudo104 birth 12 death 24 refs 12 -> got $v1(3)
      ord2 qty3 pseudo99  birth 22 death 38 refs 12 -> got $v0(2)
      ord3 qty4 pseudo105 birth 24 death 28 refs 4  -> got $v1(3)
  plus two suggested-pass rows (qty5 pseudo94 -> $a1, qty2 pseudo86 -> $a3).
  qty_compare priorities are 30000 / 30000 (tie broken by lower qty) / 22500 / 20000, which
  reproduces the printed `ord` column exactly. The consequence for the goal: target holds one
  chain in $a0 (hard reg 4), and find_free_reg scans ASCENDING, so reaching $a0 requires
  BOTH $v0 and $v1 to be occupied across that chain's whole span at the moment it is allocated
  — i.e. the third-allocated quantity must overlap the first two, whereas here qty0 (death 20)
  has already died before qty3 (birth 22) is allocated. That is a two-part requirement of
  exactly the shape the ra_solver README's title_mv_exec2 worked example describes, and it is
  the first time this function's residual has had a stated allocation-level spec rather than a
  scheduling narrative.

- [s20] GATE (a) canonical-asm FAILS, re-measured this session:
  `python3 tools/scan_hand_coded.py --single CD_datasync` -> **tier=LOW score=1/8** (91 insns),
  "no strong hand-coded indicators". Only S4 (4 loads in an 8-insn window @ insn 46) fires —
  and that window IS the ordinary load_register_parameters argument block under study. S1
  (multu pacing), S2 (empty branch) and S6 (BIOS jumptable), the only STRONG-tier carriers, are
  all negative, as are S3/S5/S7/S8. Fifth independent reproduction (s17, s18, s19, the
  2026-08-30 campaign, and now s20). Independently barred by provenance: this body is compiled
  Sony PsyQ 3.5 LIBCD CD_datasync (91 words = exactly 0x14F4-0x1388 in the library object,
  memory/closer/libcd-groundtruth.md:59-67), and the twin-family canonical-asm request covering
  this exact function was DENIED on three independently dispositive grounds
  (docs/grind/decisions.md:15, 2026-07-09).

- [s20] GATE (b) in-hand SOTN-master precedent FAILS BY CONSTRUCTION, restated against the
  2026-09-01 review's systemic finding #2 (cap-truncated censuses are not evidence): the
  objection does not apply here, because there is no construct to census. The floor is 7, not
  0 — no C construct CLOSES this function, so there is nothing for which a precedent could be
  cited. The constructs that HOLD the floor at 7 (one FAKE-annotated do{}while(0) wrap plus
  three plain pointer locals) are already inside sanctioned families and are not the blocker.
  The two aggregate spellings measured this session are the sanctioned aggregate-merge family
  and they FAIL prong (c) on their own evidence, above.

- [s20] EXHAUSTION AT DISPOSITION: 20 sessions; floor flat at 7 since s9 (eleven consecutive
  sessions); modalities spent — recon, structural, permuter, forensics, rederive, synthesis,
  escalation, and now the solver chain; 91 disproven forms banked in
  memory/grind/CD_datasync/rejected/ (7 added this session); ~60k permuter iterations across
  four campaigns (s13/s14) on a re-aligned objective proven monotone with the sandbox and still
  unable to descend; 23 argument spellings scored on BOTH compiler forks (s19) landing on one
  shared attractor and one shared minimum.

- [s20] The working tree was restored to HEAD (`git checkout -- src/system.c`) before finishing;
  only memory/grind/CD_datasync/, docs/grind/decisions.md and tmp/ were written.

- [s20] Chassis re-measured live this session: candidate.c spliced into src/system.c (HEAD carries INCLUDE_ASM) -> sandbox CD_datasync --disable all = score 7, target_insns 91, build_insns 91, rules_dropped 0. The ledger floor of 7 is chassis-current.

- [s20] F14's Intr aggregate reached through a pointer local (with or without volatile) is BYTE-IDENTICAL to the existing u8 *idx_1494 base — objdump diff of the cheat-stripped sandbox objects is clean. The aggregate declaration adds nothing the pointer local did not; target's base-register addressing was already achieved.

- [s20] Prong (c) of the sanctioned aggregate-merge family FAILS symbol-level, re-derived first-hand on the third and last twin: five asm-only consumers of D_800A1494/95/96 (getintr 22 sites, CD_cw 8, func_800817A0 8, func_800819C4 8, func_80081E1C 2) and one asm-only consumer of D_800F19B8/BC/C0 (CD_cw, 14 sites) force the per-word symbols to survive the splat config. The Ruling D scope grant is unspendable by CD_sync / CD_ready / CD_datasync alike until those consumers have C bodies.

- [s20] Direct-member access without a pointer local scores 24 / 89 — two instructions SHORT of target's 91 — reproducing s17 and re-confirming that target hoists an index base and that the three hoisted table pointers are required by target's instruction count.

- [s20] The Alarm (0x800F19B8/BC/C0) aggregate merge, never measured before, scores 12 / 91 volatile or not, and 12 / 91 combined with the Intr merge.

- [s20] inverse_compose.py classify types the FIRST divergence as RA, not scheduling — the first solver-chain verdict ever obtained for this function's residual, and a correction to the model sessions 10-19 worked under.

- [s20] First-hand local-alloc ground truth for the residual block (tmp/ra_solver_work/system.local.json, CD_datasync block 3) reproduces GCC's printed allocation order exactly from qty_compare priorities 30000/30000/22500/20000, and shows why the third-allocated quantity falls back to $v0 instead of target's $a0: qty0 dies at 20 before qty3 is born at 22.

- [s20] scan_hand_coded --single CD_datasync = tier LOW, score 1/8, S4 only — fifth independent reproduction; provenance (PsyQ 3.5 LIBCD CD_datasync, 91 words = 0x14F4-0x1388) independently bars a canonical-asm claim, and the twin-family request was DENIED 2026-07-09 (decisions.md:15).

- [s20] Exhaustion at disposition: 20 sessions, floor flat at 7/91 since s9 (eleven consecutive sessions), eight distinct modalities (recon, structural, permuter, forensics, rederive, synthesis, escalation, solver), 91 banked rejected forms (7 added this session), ~60k permuter iterations over four campaigns including a re-aligned objective proven monotone with the sandbox that still could not descend, and 23 argument spellings scored on BOTH compiler forks with identical ranking and no form below the shared minimum.

- [s20] Foreclosure record filed this session at docs/grind/decisions.md:19229; working tree restored to HEAD (git checkout -- src/system.c) — only memory/grind/CD_datasync/, docs/grind/decisions.md and tmp/ were written.

- [operator 2026-09-02] owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): re-activated with the exhaustion window RESET — the 2026-09-01 Ruling-A unpark was re-foreclosed after one session because the window did not reset. The 09-01 named probe is spent (see ledger); work the ladder from its next rung. All standing banned_constructs remain in force. exhaustion_base=20
