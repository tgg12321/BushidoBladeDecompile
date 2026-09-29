# func_800187F4 — Ruling 11 (D) proof and landing record (manual, 2026-09-28)

Rulings spent: .claude/rules/ordinary-c-judge-decidable.md § Ruling 11 (owner 2026-09-26) incl. its
(C)(3) clause "GTE-macro input copies" (owner Q28, record commit 206e77db8); inline-asm-policy.md
§ Owner ruling 2026-09-26 (inline_o.h class) and § Per-function grant: func_800187F4 (owner Q29,
206e77db8); dead-vars-local-array.md OVERSIZED-LOCALS carve-out (owner 2026-07-13). Recognizer
update for the islands: engine commit 21b9bbebd. Declaration fix: 65f4f1730.

## 0. Bodies, tools, commands

- **Landing template** `r11/c5.c` (the `@gte_*` markers are expanded by `gen.py`, verbatim
  inline_o.h statements) -> the landing body `candidate.c` (= `tmp/func_800187F4/out_c5.c`).
  `r11/c5a.c` is the same code without comments (codegen identical).
- **Measurement 1 (all tables)**: `r11/tools/fast2.sh <tag>` = the Makefile's per-file recipe for
  code6cac (`cpp | build cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w
  -mel -msoft-float | prologue_fix | maspsx (code6cac gates) | sed .align 3 -> 2 (RODATA_ALIGN2) |
  multu_pad | as`) on src/code6cac.c with the body spliced (and, for `s16 *arg0` bodies before
  65f4f1730, the declaration patched), objdump of func_800187F4 aligned with difflib against the
  function in build/src/code6cac.o (`r11/tools/cmp.py`): **"lines" = differing lines** (target 662
  objdump lines). Measurement 2: the engine sandbox, `sandbox --disable all --candidate`
  (r11/sandbox_sweep.txt, engine 21b9bbebd). Where they differ, both are given.
- **Whole object**: `r11/tools/objcmp.sh c5` — .text (60932 B), .rodata (964 B), .data, .bss and every
  relocation of the spliced code6cac.o are IDENTICAL to build/src/code6cac.o.
- **Dumps (D)(1)**: `r11/tools/dump2.sh <tag> -dl -dg` (the build cc1, same flags, on the preprocessed TU:
  `.lreg`, `.greg`) and `r11/tools/alloc2.sh <tag>` (the instrumented `tools/gcc-2.7.2/cc1` with
  `BB2_ALLOC_DEBUG=1`; it checks that its output equals the build cc1's output, "instrumented==build"
  for every tag). `r11/tools/r11table.py` names each variable's pseudo from its setting insns and prints
  allocator, hard register and the global.c ALLOCDBG line (ord = allocation order, nrefs, livelen,
  pri). Output banked: `r11/dumps_table.txt`. Registers: v0 $2, v1 $3, a0 $4, a1 $5, t1 $9, t2 $10.
- Scripts: r11/tools/ (copies; run from the repo root, they read and write tmp/func_800187F4/ and
  tmp/f187/). Per-value spellings are generated, never hand-edited: `r11/tools/mkpv3.py` (R11BASE=c5a), `r11/tools/mkalt.py`
  (structural respellings + sanctioned-family probes). Files: `r11/variants/`.

## 1. The reused locals and their values

"Value" as Ruling 11 defines it (writes reaching a common read). Seven locals hold more than one:

| local | scope (innermost enclosing all writes) | values | kind (E) |
|---|---|---|---|
| `idx` | node-loop body | V1 add-force loop counter (`idx = 0` / `idx++`), V2 subtract-force loop counter, V3 ellipsoid loop counter | loop index (E)(ii) |
| `nforce` | node-loop body | V1 `node[7]` (forces added), V2 `node[8]` (forces subtracted) | force count (E)(ii) |
| `temp` | ellipsoid-loop body | V1 `temp = work` (copy of the focus-0 squared distance, read only by gte_Lzc), V2 focus-0 table byte, V3 focus-1 table byte | generic (E)(i) |
| `work` | ellipsoid-loop body | V1 focus-0 squared distance (sum of the GTE squares), V2 its root: the small-arm and LZC-arm roots and `work = pen / work` (one value: all reach `gte_lddp(work)`) | generic (E)(i) |
| `delta` | collision block | V1 depth below the ground `pos.y - ground`, V2 Y delta to focus 0 `cpos.y - foc0.y` | Y delta (E)(ii) |
| `nbits` | LZC-1 arm | V1 `lz[0]` (GTE leading-zero count), V2 `0x16 - (nbits & ~1)` (table shift) | bit count (E)(ii) |
| `nbits2` | LZC-2 arm | the same for `lz[1]` | bit count (E)(ii) |

No other local holds two values: `f` (force row) was split per loop (`f_add`/`f_sub`, byte-identical,
r11pv_f 0); `dist2` holds root then `pen / dist2` as ONE value (both reach `gte_lddp(dist2)`); `vx/vy/vz`,
`bits`, `bits2`, `pen`, `vy_new`, `i`, `node` are single values (every write reaches a common read).
The five focus-1 / focus-0 X,Z axis deltas are their own locals (dx0, dz0, dy1, dx1, dz1): the
ablation below shows sharing any of them with `delta` is byte-identical, so they are not shared.

## 2. Measured necessity, per variable (fast2 lines; sandbox in r11/sandbox_sweep.txt)

Full table with the engine sandbox score next to each (they agree on every zero and every
nonzero; magnitudes differ slightly): r11/measurements.md. Sandbox: landing 0 (644/644); idx 67
(ablations 70/70/67), nforce 21, temp 40 (2/9/29), work 26, delta 4, nbits 2/2, all 102.

| body | fast2 lines |
|---|---|
| landing c5 / c5a | **0** (662/662) |
| r11pv_idx (3 counters) | 67 |
| abl: idx_add alone / idx_sub alone / idx_sph alone | 114 / 70 / 67 |
| r11pv_nforce | 21 |
| r11pv_temp (3 locals) | 42 |
| abl: copy alone (lzc_in) / table byte 1 alone / table byte 2 alone | 2 / 9 / 31 |
| r11pv_work (sq1, dist1) | 26 |
| r11pv_delta (dg, dy0) | 4 |
| r11pv_nbits / r11pv_nbits2 | 2 / 2 |
| r11pv_all (all seven split) | 106 |
| delta ablation (sharing of the 7 deltas, 127 partitions, on w2): 0 whenever the ground depth shares with at least one sphere delta; 4 whenever it is alone | abl_d.log |

Structural respellings (each on that variable's per-value spelling): idx `while` loops 67,
function-scope counters 67; nforce read in the loop test (`idx < node[7]`) 64 (664 insns),
function-scope counts 21; temp: no copy and no byte local (bytes read inline) 42, function scope 42;
work: `temp = sq1 = ...` chained 26, function scope 26; delta: ground depth written inline three
times 4, function scope 4; nbits: one statement `shift = 0x16 - (lz[0] & ~1)` 2 (both 4), function
scope 2. Copy placements for temp's V1 (the Q28 copy): fresh local at function scope 2, in the LZC arm
2, as the arm's `s32 lzc_in = work;` initializer 2, no copy at all 2.

## 3. Mechanism and necessity, per variable

### idx — global.c allocno priority (global.c:635-656, `allocno_compare`)
Priority = floor_log2(n_refs) * n_refs / live_length * 10000; allocnos are seated in that order,
each by find_reg into the lowest-numbered free register it does not conflict with.
- Reuse (dumps_table c5): `idx` pseudo 89, nrefs 45, livelen 245, pri **9183**, ord 36 -> **$t2**.
  It is seated after bits (21000, $a0), vx/vy/vz (12698-13333, $a1/$a3/$a2), the node+8 giv and
  `nforce` (10000, $t1), all live across the force loops -> $t2, the target's register for all three
  loop counters (target: `addiu $t2,$t2,1; slt $v0,$t2,$t1` closing each force loop, and
  `addiu $t2,$t2,1; slt $v0,$t2,$v0` closing the ellipsoid loop).
- Per-value (r11pv_idx): idx_add pseudo 89 nrefs 16 livelen 25 pri 25600 ord 21 -> **$a0**;
  idx_sub pseudo 90 nrefs 16 livelen 22 pri 29090 ord 20 -> **$a0**; idx_sph livelen 198 pri 1969 ->
  $t2. The force-loop counters are seated before `bits` and take $a0; bits, vx.. shift (67 lines).
- Necessity (D)(3). Property: the counter allocno's priority is below `nforce`'s (10000) so it is
  seated after every allocno live across the force loops. A force-loop counter's value lives only in
  its own loop (its only writes are the loop's init and increment, its only reads the loop test,
  `idx == 3` and the increment; the next write is the next loop's init). So in ANY spelling that
  gives each loop its own variable, that variable's n_refs (16) and live length (22-25, the loop) are
  fixed by the statement list, whatever its declaration order, scope, name or type (the RTL is SImode
  for every 32-bit integer type; narrower types add extensions): priority >= 25600 > 10000. Its
  priority could drop below 10000 only if its live range exceeded ~64 insns (4*16/64), i.e. spanned
  beyond its loop: only a variable that also carries another loop's value does. Two loops sharing
  (the force loops only) give 32 refs over ~47 insns, 34042 (ledger [s2] item 4), still above 10000;
  only all three (the ellipsoid loop's ~198 insns) give 9183. Declaration order only breaks exact
  priority ties, and none of these is tied.

### nforce — the same priority order, the floor_log2 step
- Reuse: pseudo 90 nrefs 14 livelen 42 -> 3*14/42 = pri **10000**, ord 34 -> **$t1**; `idx` (9183)
  after it -> $t2. Target: $t1 is the count, $t2 the counter (`slt $v0,$t2,$t1`).
- Per-value (r11pv_nforce): nforce_add / nforce_sub nrefs 7 livelen 21 -> 2*7/21 = pri **6666**
  each (ord 36/37) -> $t2; `idx` (9183) is seated first -> $t1. Counter and count swap: 21 lines.
- Necessity: a force count's refs are its load and the loop test (weighted by loop depth): 7, over the
  loop (21 insns), in every one-variable-per-value spelling (the statements are fixed); floor_log2(7)
  = 2, so its priority is 6666 < 9183 whatever its declaration. One allocno holding both counts has 14
  refs over 42 insns and floor_log2(14) = 3: 10000 > 9183. The priority gain is the floor_log2 step,
  which only a variable holding both counts has.

### temp — local-alloc before global (local-alloc.c:472), then global priority
- Reuse: pseudo 300 (copy + both table bytes) nrefs 18 livelen 8 -> pri **90000**, ord 0 -> **$a0**;
  the shift pseudos (nbits 359 / nbits2 423, pri 64285) come next and take **$v1**. Target: copy,
  both bytes in $a0 (`addu $a0,$a1,$zero` at 0x80018E18, `lbu $a0` at 0x80018E6C / 0x80018FE4), shift
  in $v1 (`subu $v1,$t9,$v0`).
- Per-value (r11pv_temp): each table byte is written by `lutK = (&D_8008D118)[...]` and read only by
  the next statement, both in its LZC arm: it lives in one basic block and dies once, so
  local-alloc.c:472 (`reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1`) makes it a local quantity,
  seated before global allocation into the first free register of its block: pseudos 360/425
  **local-alloc $v1**. The shift pseudos then conflict with $v1 and take **$a0** (ord 4/5), and the copy
  (lzc_in, nrefs 6 livelen 4 pri 30000, ord 19) takes $v1. 42 lines. Ablations: copy alone 2 (the
  fresh copy's only conflicts are the compare's $v0 and the island's $t4-$t7, so find_reg seats it in
  $v1: evidence [s2 cont.] 8); byte 1 alone 9; byte 2 alone 31.
- Necessity: the property is that each table byte's variable is NOT a single-block quantity, so it
  is seated by global.c (before the shift, pri 90000) instead of by local-alloc. In every
  one-variable-per-value spelling, a table byte's variable has exactly one write and one read, both in
  its LZC arm (fixed by the statement list), hence one block and one death: local-alloc takes it,
  and at local-alloc time the shift pseudo (global: two sets) is not seated yet, so the byte cannot be
  kept out of $v1 by it; it lands in $v0/$v1, never $a0. Declaration scope does not change the block
  a pseudo lives in (function scope: 42, st_temp_fscope). The copy value alone cannot reach $a0 either
  (the four placements above, all 2).
- Q28 clause for V1 (the copy), (C)(3) "GTE-macro input copies": (a) one write `temp = work;`, whole
  RHS the named local `work`, no cast; `work` is read again after it (`work < 0x400`, `[work]`,
  `work >> nbits`). (b) its only read is `gte_Lzc(temp, &lz[0])`, i.e. the bare variable as the whole
  `"r"(r1)` operand of gte_ldlzc's `move $12,%0` (a qualifying unit, admitted under the 2026-09-26
  class). (c) later on the same path the variable is written with V2, the table byte (a load whose
  `lbu` is in the target). (d) the copy is `addu $a0,$a1,$zero` at 0x80018E18 (beqz delay slot) in the
  target, and the build emits it at the same position (0 lines). (e) every other prong below.

### work — cse.c make_regs_eqv class head (cse.c:840-857)
`temp = work;` makes the two registers equivalent in cse; the class head (the register cse
substitutes into every equivalent use) is the one whose last use is later and beyond the block.
- Reuse: `work`'s last use is `gte_lddp(work)` / the scale at the end of the loop body, after
  `temp`'s last use (the focus-1 table byte): `work` stays the head. The compare, the table index and
  the LZC-arm index read `work` (**$a1**) and the copy survives as its own move (the Q28 copy). Target:
  `addu $a1,$v0,$a0; slti $v0,$a1,0x400; beqz; addu $a0,$a1,$zero`.
- Per-value (r11pv_work): `sq1`'s last use is the LZC-1 index `sq1 >> nbits`, before `temp`'s last
  use: `temp` becomes the head, cse rewrites sq1's reads to `temp`, the sum is computed straight into
  temp's register and the copy disappears (`addu $a0,$v0,$a0 ... nop` where the target has
  `addu $a1,...; move $a0,$a1`); the root (dist1, nrefs 27 livelen 93) is seated in $a2. 26 lines.
- Necessity: in every one-variable-per-value spelling the squared distance's reads are the compare,
  the small-arm index, the copy and the LZC-1 index (fixed statements), so its last use precedes
  temp's; the head is `temp` whatever the declaration (function scope 26). Only a variable whose last
  use is later than temp's (here: the same variable carrying the root/scale to the end of the body)
  keeps the head, and with it the target's `$a1` compare and the copy.

### delta — cse.c make_regs_eqv class head, with the division's copy (cse.c:840-857)
`delta / 8` (signed) expands to `t = delta; if (t < 0) t += 7; t >>= 3` (dump insns 645-651).
- Reuse (dump c5): insn 647 tests `(reg/v 269)` = delta and insn 649 adds 7 to it: delta's last use
  (the Y delta to focus 0, in the ellipsoid loop) is later than t's, so delta stays the head. asm:
  `bgez $v1` with `move $v0,$v1` in the delay slot = target.
- Per-value (r11pv_delta): the ground depth `dg` is read only by `> 0`, `> 0x3200` and `/ 8`; its
  last use is the copy (insn 645, REG_DEAD), t outlives it, t becomes the head and insns 647/649 read
  `(reg 276)` = t: `move $v0,$v1; bgez $v0`. 4 lines.
- Necessity: in every one-variable-per-value spelling the ground depth's reads are those three in the
  ground block (fixed statements), so its last use is the division and the division's temp always
  outlives it; declaration scope does not change last-use order (function scope 4, inline 4). Only a
  variable that is read again later (here: carrying a sphere delta) stays the head. Which sphere delta
  shares is free (each of the six alone reaches 0, abl_d.log); the body shares the first one computed,
  the Y delta to focus 0, so both values are Y deltas.
- Disclosure: a FAKE dead store `dg = 0;` after the ellipsoid loop also reaches 0 (fam_delta_deadstore_end:
  reg_scan counts the dead store as a later use before cse, and flow deletes it afterwards). It is not
  landed: it adds a no-semantic-purpose construct (dead-store-fake-exception) where the reuse adds none
  (Ruling 1(4)); the same choice the func_8008B488 Ruling 11 layer-2 accepted (its SR value closed with
  a FAKE chain-extender; the FAKE-free reuse landed, 0313b22b6).

### nbits / nbits2 — local-alloc combine_regs tie (local-alloc.c:1295-1346, combine_regs :1784)
- Reuse: nbits pseudo 359 (sets lz[0], then the shift) is set twice, so it is not a local quantity
  (global, pri 64285, $v1); the `& ~1` result is a separate local temp that cannot be tied to it and
  takes $v0: `and $v0,$v1,$s6; subu $v1,$t9,$v0` = target.
- Per-value (r11pv_nbits): the count `lzcount = lz[0]` is written once and dies at the `& ~1`, one
  block: a local quantity (local-alloc $v1), and combine_regs ties the `&` result to the dying input:
  `and $v1,$v1,$s6; subu $v1,$t9,$v1`. 2 lines (4 for both).
- Necessity: in every one-variable-per-value spelling the count's only read is `(count & ~1)` in the
  same block (fixed statements): one set, one death, one block, so the tie happens; one statement
  `shift = 0x16 - (lz[0] & ~1)` has no count variable at all and ties the load the same way (2). Only
  a variable set twice (count, then shift) escapes local-alloc. (Ruling 4, compound splits, was not
  claimed: the second statement is not a compound assignment; compound spellings measure 4-49.)

## 4. Prong walk (A)-(H), all seven

- **(A)** Each is a local of func_800187F4, declared once at the innermost scope enclosing all its writes
  (table in §1), no other declaration moved or re-scoped for it; no `&idx`, `&nforce`, `&temp`, `&work`,
  `&delta`, `&nbits`, `&nbits2` anywhere.
- **(B)(1)** every write is read before the next write: idx (each loop's test), nforce (the loop test),
  temp (V1 by gte_Lzc, V2/V3 by the next statement), work (V1 by the compare, V2 by `work >= r`), delta
  (V1 by `delta > 0`, V2 by the range test), nbits (V1 by `& ~1`, V2 by the index).
  **(B)(2)** feasible paths where a write stores a different value than the variable holds:
  idx loop-2 init `idx = 0` (holds node[7] after loop 1 when node[7] >= 1); ellipsoid init `idx = 0`
  (holds node[8] when node[8] >= 1); `nforce = node[8]` (holds node[7]; differs when node[7] != node[8]);
  `temp = work` (holds the previous focus's table byte <= 0xFF while work >= 0x400 on the LZC path, or
  garbage on the first pass); table byte 1 (holds the copy >= 0x400 on that path; bytes are <= 0xFF);
  table byte 2 (holds byte 1 or the copy; differ whenever the looked-up bytes differ); `work = sum`
  (holds the previous iteration's scale); roots (hold the squared distance; differ except sq == root);
  `work = pen / work` (differs unless pen == root*root); `delta = cpos.y - foc0.y` (holds the ground
  depth; differs in general); `nbits = 0x16 - (nbits & ~1)` (holds the count c; c == 0x16 - (c & ~1)
  has no integer solution (even c gives c = 11, odd c gives c = 11.5), so it differs on every path).
- **(C)(1)** the one-variable-per-value spellings are r11/variants/r11pv_<var>.c (each value its own
  local at the innermost scope of its writes); **(C)(2)** they differ from the landing body only in
  declarations and identifiers (generated by mkpv3.py's renames and declaration moves; no statement
  added or removed); **(C)(3)** every value has a write whose instructions are in the target: counters
  (`addiu $t2,$t2,1`), counts (`lw $t1,20($t0)`/`24`), table bytes (`lbu $a0`), sums (`addu $a1,...`),
  roots (`srl`/`srav`), deltas (`subu`), shift (`subu $v1,$t9,$v0`), lz loads (`lw $v1,0x10($sp)`); the
  copy V1 of temp under the Q28 clause (§3 temp).
- **(D)(1)** dumps: r11/dumps_table.txt (+ commands §0); **(D)(2)** mechanisms named per variable (§3);
  **(D)(3)** necessity per variable (§3); **(D)(4)** alternatives (§2), sanctioned families (§5),
  permuter (§6).
- **(E)** names: `idx`, `nforce`, `delta`, `nbits`/`nbits2` under (E)(ii) (every value is a loop index /
  a force count / a Y delta / a bit count); `temp`, `work` under (E)(i).
- **(F)** each declaration carries a comment: several values, each named, citing Ruling 11 and this file.
- **(G)** fresh layer-2 cheat-reviewer on the landing (pending at the time of writing).
- **(H)** every other construct: §7.

## 5. Sanctioned-family (FAKE) probes on the per-value spellings (disclosure)
Measured so that no FAKE-free alternative is missed and every FAKE alternative is on record:
delta: self-assign `dg = dg;` after the loop 4; **dead store `dg = 0;` after the loop 0** (§3 delta);
chain-extender `+ dg - dg` in the first sphere delta 117, in the ground store 6. temp: self-assign
42, chain-extender on byte 1 42, dead store 42. nforce: self-assign 21, chain-extender in loop 2 94,
dead store 21. work: chain-extender `dist1 + sq1 - sq1` 146, dead store 26. idx: chain-extenders
into the ellipsoid loop 127 / loop 2 110, dead stores 67. nbits: dead store 2, chain-extender 9.

## 6. Permuter campaigns (tools/permuter_campaign.py, `--stack-diffs`, standalone workspace)
Workspace: `r11/tools/mkperm.sh` (head.h + expanded body, preprocessed; GNU asm statements wrapped as
`#pragma _permuter b64literal` by `r11/tools/b64asm.py`, the func_8002DE20 method; target.o from
asm/funcs with include/gte_macros.inc). Standalone scores equal the in-TU ones (c2 0, pv_all 117,
r11pv_all 106).
- Campaign 1 (tmp/f187/perm_pvall, from pv_all: idx/nforce/temp/work/delta split, nbits shared, -j4):
  7,459 iterations, 399 s, 18 finds, best 665 from base 830. The best find names two fresh
  intermediates (`new_var = cpos[2]` before a sphere delta, `new_var2 = lz[1]`); no find re-shares a
  split value and none approaches 0.
- Campaign 2 (tmp/f187/perm_r11pv, from r11pv_all: all seven split, -j4, fresh seed): 17,656
  iterations, 956 s, 37 finds, best 565 from base 755 (found at 94 s; nothing better in the remaining
  860 s; stopped on the fresh-seed window). What the best finds do (their diff.txt): 565 writes
  `dg = r < dz0;` inside the ellipsoid loop, i.e. makes the ground-depth local carry a second value
  in a later block (the `delta` reuse property, �3 delta, in a FAKE-shaped spelling); 600 stages
  `lz[0]` through the unrelated `bits2` (`bits2 = lz[0]; lzcount = bits2;`), a variable set twice
  (the `nbits` property, �3). Neither keeps one value per variable, and neither approaches 0.
- Across both campaigns (25,115 iterations) no find reaches the target. The finds that keep one value
  per variable only add named intermediates (campaign 1's best, 665 from 830, still far off); the best
  gains of campaign 2 re-create a reuse.

## 7. Everything else (H)

- **Islands**: 85 asm statements, all recognized as qualifying units by engine/gtemacro.py
  (gte_ldlvl 20, gte_stlvnl 16, gte_Lzc 12, gte_ldv0/rtv0tr/sqr0/lddp/gpl12 6 each, gte_stlvl 4, gte_gpf0
  3); keep-mode strip 0; the seven command units carry the Q29 post-DMPSX words; `($12)` spelled as
  the header writes it. Each island has a comment naming the macro and its pinned line range.
- **`lz[6]`**: OVERSIZED-LOCALS carve-out, frame proof in the declaration comment; measured on c5:
  lz[2]/lz[4] frame 0x68 (24 lines), lz[5] 0x78 (0), lz[7]/lz[8] 0x80 (24). lz[0]/lz[1] are the live
  LZC outputs (stored by gte_stlzc, read back).
- **RopeScratch at 0x1F800000** (`#define SCR`): the same constant-pointer scratchpad view as the
  landed func_80018094 (`#define SCRV ((ScrV *)0x1F800000)`); field comments state what the code does
  with each; `force[0][3]` is the GNU zero-length trailing table (indexed by the packed bytes; the
  earlier `force[1][3]` was indexed past its bound; byte-identical).
- **`arg0` as `s16 *`**: the caller's type (65f4f1730); `*(s32 *)((u8 *)arg0 + 0xC)` and
  `*(s16 *)((u8 *)arg1 + 4)` are the file's existing offset-cast style (func_80018300).
- **`(&D_8008D118)[i]`**: the same table idiom as landed func_80018300 / func_80018094 in this file.
- **`vy_new`**: one value (both arms reach the store); the single store reproduces the target's
  shared `sw` at 0x80018CB8 (evidence [s2 cont.] 9).
- **`dist2`**: one value (root, then `pen / dist2`, both reaching `gte_lddp(dist2)`).
