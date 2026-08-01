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
