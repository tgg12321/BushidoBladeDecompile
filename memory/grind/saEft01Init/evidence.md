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
