# func_800187F4 — Ruling 11 (D) proof and landing record, v10 (manual, 2026-09-29)

History. v1 (this text's base) FAILed a fresh layer-2 on `work`/`delta`
(rejected/r11-work-delta-reuse-layer2-fail-0.md); v2 split them with two FAKE dead stores and FAILed a
fresh layer-2 on `nforce`, which six FAKE do-while(0) wraps close as a per-value spelling
(rejected/r11-nforce-dowhile-layer2-fail-0.md). The owner then ruled (Q30, seventeenth batch, record commit 45ecac2fc; rule text
Ruling 11 (D) § "FAKE-construct spellings are set aside") that one-variable-per-value spellings needing a
FAKE-annotated construct do not count against (D)(3). v3 lands the v1 code (seven reused locals, no FAKE
construct for any of them) with the v1 reviewer's other corrections (mechanism wording, lz[6] census,
naming, header text). Every FAKE-construct spelling measured is banked in §5.

v4 (same body): the third fresh layer-2 (rejected/r11-proof-args-v3-layer2-fail-0.md) found no FAKE-free
per-value spelling reaching the target in its 2,071 FAKE-free per-value probes (2,170 bodies) but FAILed (D)(3)'s universal arguments. The owner
then ruled Q31 (Mechanism + search, the Q25 standard, for Ruling 11 (D)(3)) (record commit ee84164e3) and Q32 (a FAKE construct resized
within its own admitted range is the same construct). §3 (rewritten in v5) states the
named MECHANISM plus the measured record; §3A is the (D)(3) record under Q31 (every banked counting spelling,
none reaching the target), and the universal sentences the v3 reviewer refuted are corrected in place.

v5 (same body): the fourth fresh layer-2 FAILed the ledger only (the landing spelling's dumps for work and
delta were no longer banked at HEAD; §3 still stated universals; tmp-only reviewer probes; three figures).
v5 banks the landing-chassis dumps (r11/dumps_table_landing.txt, r11/lreg_excerpts_landing.txt), rewrites
§3 as mechanism + record only, banks every reviewer probe (r11/reviewer_probes/), and corrects the figures.
r11/dumps_table.txt remains the v2 (c6) chassis table; the landing citations below use the landing files.

v6 (same bytes): the fifth fresh layer-2 FAILed the text only: the lz[6] frame derivation said "16 bytes
left" of the 0x40 (8 + 32 + 16 = 56); the correct figure is 24 (sp+0x10-0x27: lz[0]/lz[1] written, a
16-byte unwritten tail), now fixed in the source comment, §7 and the Match message. It also required the
(D)(1) and nbits citations to point at the landing-chassis dumps, "banked" in the §3A summary sentence, and
its 14 new FAKE-free probes banked (r11/reviewer_probes/rv5_*, all missing, best 2).

v7 (same bytes): the sixth fresh layer-2 FAILed the records only: the lz[6] comment's "Measured:" line
and §7 gave frame 0x68 for lz[3]/lz[4] (measured 0x70: rv6_lz3, rv6_lz4, lzc5_4), and template.c still
carried the v5 wording (so §0's "template.c (= r11/variants_v3/c7.c)" was false). Both fixed; template.c
is now byte-identical to r11/variants_v3/c7.c. Its seven new FAKE-free probes (best 4) and six frame
probes are banked (r11/reviewer_probes/rv6_*).

v8 (same bytes): the seventh fresh layer-2 FAILed the records only: §5 said the frame-probe bodies
"lack lz[6] ... none reaches 0", but lz[5] (lzc5_5, rv6_lz5, the Q32 twin) and lz[6] (rv6_lz6) reach 0;
probes.md's closing line still counted "four zeros" (there are eight); and the v7 ledger the landing cites
was uncommitted. §5 and probes.md are corrected, its probes banked (rv7_*), and the ledger is committed
before the auth: / Match: commits.

v9 (same bytes): before re-submitting, an author-side audit (not a layer-2) checked every figure and
cross-reference against the banked files. Corrected: §3A now classifies every banked body under Q30 (the v2
chassis c6 carries two FAKE dead stores the landing body lacks, so every c6-based body, the v2 reviewer's
1,064 included, is set aside; 49 of those reach 0 and are listed in §5); the v3 reviewer's lz[8] early-init
body (3 lines) is a counting spelling and is now in §3A; the lz-declaration claim in §5 rests on a scan of all
banked bodies (r11/fake_scan.md); §0 dump pointer, §2 table pointer, §5 counts (72 + 5, not 77), §6
iteration count, §7 address and idiom citations; tmp-only evidence banked in r11/banked/.

v10 (same bytes): the eighth fresh layer-2 FAILed the records only: the frame-census bodies
(r11/variants_v2/fr_*.c) were called "the reuse body, or its lz[2] form" and left out of the counting
record, although they are v2-chassis spellings (work and delta split) whose lz[2] / scalars do not carry the
landing's lz[6] unchanged, so Q30 does not set them aside; the w2 delta-ablation bodies with one value per
variable were missing, and their chassis (w1/w2) was not banked, although the v9 note said the tmp-only
evidence was banked. §3A is now r11/manifest.md, generated from one uniform sweep of every banked body plus
every result-file set, which lists each body reaching 0 with its reason and fails on any zero without one;
w1/w2 and the v8 reviewer's probes are banked.

Rulings spent: .claude/rules/ordinary-c-judge-decidable.md § Ruling 11 (owner 2026-09-26) incl. its
(C)(3) clause "GTE-macro input copies" (owner Q28, record commit 206e77db8); inline-asm-policy.md
§ Owner ruling 2026-09-26 (inline_o.h class) and § Per-function grant: func_800187F4 (owner Q29,
206e77db8); dead-vars-local-array.md OVERSIZED-LOCALS carve-out (owner 2026-07-13). Recognizer
update for the islands: engine commit 21b9bbebd. Declaration fix: 65f4f1730.

## 0. Bodies, tools, commands

- **Landing template** `template.c` (= r11/variants_v3/c7.c; the `@gte_*` markers are expanded by
  `gen.py`, verbatim inline_o.h statements) -> the landing body `candidate.c`. c7 is v1's code (c5, c5a)
  with comment and type-name changes only (Scr1F800000); its objdump is identical to c5's (0 lines
  each), so every v1 measurement below applies to it unchanged (variants in r11/variants/,
  measurements_v1.md, sandbox_sweep.txt).
- **Measurement 1 (all tables)**: `r11/tools/fast2.sh <tag>` = the Makefile's per-file recipe for
  code6cac (`cpp | build cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w
  -mel -msoft-float | prologue_fix | maspsx (code6cac gates) | sed .align 3 -> 2 (RODATA_ALIGN2) |
  multu_pad | as`) on src/code6cac.c with the body spliced (and, for `s16 *arg0` bodies before
  65f4f1730, the declaration patched), objdump of func_800187F4 aligned with difflib against the
  function in build/src/code6cac.o (`r11/tools/cmp.py`): **"lines" = differing lines** (target 662
  objdump lines). Measurement 2: the engine sandbox, `sandbox --disable all --candidate`
  (r11/sandbox_sweep.txt, engine 21b9bbebd). Figures are fast2 lines unless labelled "sandbox";
  measurements_v1.md gives both for every v1 body.
- **Whole object**: `r11/tools/objcmp.sh c5` — .text (60932 B), .rodata (964 B), .data, .bss and every
  relocation of the spliced code6cac.o are IDENTICAL to build/src/code6cac.o.
- **Dumps (D)(1)**: `r11/tools/dump2.sh <tag> -dl -dg` (the build cc1, same flags, on the preprocessed TU:
  `.lreg`, `.greg`) and `r11/tools/alloc2.sh <tag>` (the instrumented `tools/gcc-2.7.2/cc1` with
  `BB2_ALLOC_DEBUG=1`; it checks that its output equals the build cc1's output, "instrumented==build"
  for every tag). `r11/tools/r11table.py` names each variable's pseudo from its setting insns and prints
  allocator, hard register and the global.c ALLOCDBG line (ord = allocation order, nrefs, livelen,
  pri). Output banked: `r11/dumps_table_landing.txt` + `r11/lreg_excerpts_landing.txt` (landing chassis
  c7), `r11/dumps_table.txt` (v2 chassis c6); the full landing ALLOCDBG list: r11/banked/alloc_c7_landing.txt. Registers: v0 $2, v1 $3, a0 $4, a1 $5, t1 $9, t2 $10.
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

No other local holds two values: `f` (force row) is one local per loop in the body (`f_add`/`f_sub`;
r11pv_f, which moves both to node-loop scope, is byte-identical, 0); `dist2` holds root then `pen / dist2` as ONE value (both reach `gte_lddp(dist2)`); `vx/vy/vz`,
`bits`, `bits2`, `pen`, `vy_new`, `i`, `node` are single values (every write reaches a common read).
The five focus-1 / focus-0 X,Z axis deltas are their own locals (dx0, dz0, dy1, dx1, dz1): the
ablation below shows sharing any of them with `delta` is byte-identical, so they are not shared.

## 2. Measured one-variable-per-value spellings, per variable (fast2 lines; sandbox in r11/sandbox_sweep.txt)

Full table with the engine sandbox score next to each (they agree on every zero and every
nonzero; magnitudes differ by up to 44 lines, e.g. idx_add alone 114 vs 70): r11/measurements_v1.md. Sandbox: landing 0 (644/644); idx 67
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
| delta ablation (127 sharing subsets of the 7 deltas, on w2, a pre-c5 spelling of this code under generic names: `s32 arg0`, function-scope locals;
r11/banked/w1.c, w2.c, generator r11/tools/mkabl.py): 0 whenever the ground depth shares with at least one sphere delta; 4 whenever it is alone | abl_d.log |

Structural respellings (each on that variable's per-value spelling): idx `while` loops 67,
function-scope counters 67; counts read in the loop tests (`idx < node[7]` / `idx < node[8]`, no count local) 64 (664 insns),
function-scope counts 21; temp: no copy and no byte local (bytes read inline) 42, function scope 42;
work: `temp = sq1 = ...` chained 26, function scope 26; delta: ground depth written inline three
times 4, function scope 4; nbits: one statement `shift = 0x16 - (lz[0] & ~1)` 2 (both 4), function
scope 2. Copy placements for temp's V1 (the Q28 copy): fresh local at function scope 2, in the LZC arm
2, as the arm's `s32 lzc_in = work;` initializer 2, no copy at all 2.

## 3. Mechanism per Ruling 11 local (Q31 (a)), with the measured record (Q31 (b)-(c) in §3A)

Dumps of the LANDING spelling (c7 = v1 code) and of each one-variable-per-value spelling on that chassis:
r11/dumps_table_landing.txt (r11table rows: pseudo, allocator, register, ALLOCDBG ord/nrefs/livelen/pri) and
r11/lreg_excerpts_landing.txt (.lreg insns of the two cse decisions); commands in the files' headers.
Mechanism sources: global.c:635-656 `allocno_compare` (priority = floor_log2(n_refs) * n_refs /
live_length * 10000, seated in that order, find_reg taking the lowest free register); flow.c:2081
(reg_n_refs += loop depth); local-alloc.c:472 (a pseudo in one block with one death is a local quantity,
seated before global); local-alloc.c combine_regs (:1784, called :1295-1346); cse.c make_regs_eqv (:840-857,
class head = the register whose last reference, reads and sets per regclass.c:1763-1764, is later).

### idx — global.c priority, with the guard fold
- Landing (c7): pseudo 89 (all three counters) nrefs 45 livelen 245 pri 9183, ord 36 -> $t2, seated after
  (r11/banked/alloc_c7_landing.txt) bits ($a0), vx/vy/vz, the node+8 giv and nforce (pseudo 90, pri 10000, ord 34, $t1); each init in its
  for-init, so the guard `0 < count` folds into `blez` with the init in the delay slot. Target: `addiu
  $t2,$t2,1; slt $v0,$t2,$t1` / `... $v0,$t2,$v0`.
- Per-value (v1pv_idx): the force counters nrefs 16, livelen 25 / 22, pri 25600 / 29090, ord 21 / 20 ->
  $a0; 67 lines. With the init hoisted (v3 reviewer, rv3I_1000000_ff_loop) the priority falls below the
  count's (9552 < 9767, $t2) but the guard no longer folds and a phantom slot is lost (27; 3 with lz[8], r11/banked/rv3I_1000000_ff_loop_lz8.c, a counting spelling, §3A).
- Record: every banked counting spelling for idx misses (§3A).

### nforce — global.c priority, the floor_log2 step
- Landing: pseudo 90 (both counts) nrefs 14 over 42 -> 3*14/42 = pri 10000, seated before idx (9183) ->
  $t1; idx -> $t2.
- Per-value (v1pv_nforce): each count nrefs 7 over 21 -> 2*7/21 = 6666, seated after idx; the seats swap
  (21 lines). Loop-form respellings (goto/while/do-while, v3 reviewer) change the depth weighting and are
  measured in §3A.
- Record: every banked counting spelling for nforce misses (§3A).

### temp — local-alloc of single-block values vs global priority
- Landing: pseudo 300 (copy + both table bytes) nrefs 18 livelen 8 -> pri 90000, ord 0 -> $a0; the shift
  pseudos (359 / 423, pri 64285) take $v1. Target: `addu $a0,$a1,$zero` (0x80018E18), `lbu $a0`
  (0x80018E6C / 0x80018FE4), shift in $v1.
- Per-value (v1pv_temp): each table byte is one write and one read in its LZC arm, a local-alloc quantity
  (local-alloc.c:472) seated before global into $v1; the shifts then take $a0 and the copy $v1 (42 lines).
  Ablations: copy alone 2, byte 1 alone 9, byte 2 alone 31.
- Q28 clause for V1 (the copy `temp = work;`): (a) one write, whole RHS the named local `work`, no cast;
  `work` is read again (`work < 0x400`, `[work]`, `work >> nbits`). (b) its only read is
  `gte_Lzc(temp, &lz[0])`, the bare variable as the whole `"r"(r1)` operand of gte_ldlzc's `move $12,%0`,
  an admitted class unit. (c) later on that path the variable holds V2, the table byte (a load; `lbu` in the
  target). (d) the copy is the target's `addu $a0,$a1,$zero` at 0x80018E18, emitted at the same position
  (0 lines). (e) Q31 governs its (D)(3): fresh copy locals, every placement (function scope, in the arm, as
  an initializer, u32, register, no copy at all) are banked: 2 each, none reaching the target.
- Record: every banked counting spelling for temp misses (§3A).

### work — cse.c make_regs_eqv class head (lreg_excerpts_landing.txt)
- Landing (c7): insn 883 sets pseudo 301 (work) to the sum, insn 886 copies it to 300 (temp), insn 889
  compares 301 with 1024: work's last reference (the scale, the lddp) is later than temp's, so work stays
  the class head and the copy stays its own move (target `addu $a1,$v0,$a0; slti $v0,$a1,0x400; beqz;
  addu $a0,$a1,$zero`).
- Per-value (v1pv_work): insn 883 sets pseudo 300 (temp's) to the sum directly, there is no copy insn, and
  insn 889 compares 300: sq1's last reference (the LZC-1 index) is before temp's, temp became the head and
  took sq1's reads (`addu $a0,$v0,$a0 ... nop`); 26 lines. With the Q28 copy also its own local, sq1 stays
  the head but the spelling misses through other seats (v3 reviewer rv3x_0011000 27, rv3x_0041001 13).
- Record: every banked counting spelling for work misses (§3A); the FAKE dead-store closers are set aside
  (§5).

### delta — cse.c make_regs_eqv class head, with the `/ 8` copy (lreg_excerpts_landing.txt)
- Landing (c7): insn 645 copies 269 (delta) into 276, jump 647 tests `(ge (reg/v:SI 269) 0)` and insn 649
  adds 7 to `(reg/v:SI 269)`: delta's last reference (the focus-0 Y delta) is later than 276's, so delta
  stays the head: `bgez $v1` with the copy in the delay slot = target.
- Per-value (v1pv_delta): insn 645 carries REG_DEAD for 269 (the ground depth's last reference), 276
  becomes the head, and 647 / 649 read `(reg:SI 276)`: `bgez $v0`; 4 lines.
- Record: every banked counting spelling for delta misses (§3A); the FAKE dead-store closers are set aside
  (§5). Which sphere delta shares is free (each of the six alone reaches 0 while the ground depth shares,
  abl_d.log); the body shares the first one computed, the Y delta to focus 0.

### nbits / nbits2 — local-alloc combine_regs tie
- Landing: pseudo 359 (lz[0], then the shift) is set twice, so not a local quantity (global, pri 64285,
  $v1); the `& ~1` result is a separate local temp that takes $v0: `and $v0,$v1,$s6; subu $v1,$t9,$v0`.
- Per-value (v1pv_nbits): `lzcount = lz[0]` is written once and dies at the `& ~1` in one block: a local
  quantity (landing-chassis dumps dump_v1pv_nbits / dump_v1pv_nbits2 in r11/dumps_table_landing.txt:
  local-alloc $v1), and combine_regs ties the `&` result to it: `and
  $v1,$v1,$s6; subu $v1,$t9,$v1`; 2 lines (4 for both).
- Record: every banked counting spelling for nbits / nbits2 misses (§3A).

## 3A. (D)(3) under Q31 — the banked search (r11/manifest.md)

Q31 (Ruling 11 (D) § "Mechanism + search"): (a) mechanisms named from dumps, §3; (b) every spelling
proposed by the author or a reviewer banked and measured; (c) no counting spelling reaches the target.

(b): r11/manifest.md, generated by r11/tools/manifest.py, lists every measured body: the 210 banked `.c`
bodies (author variants, reviewer probes, rejected and superseded bodies, the ablation chassis), re-measured
in one sweep on 2026-09-29 (r11/tools/fast4.sh = the real per-file recipe; log r11/banked/sweep.log), and the
15 result-file sets whose bodies are banked as generator + results (the v2 reviewer's 1,064, the v3
reviewer's 2,170, the 128 delta ablations, the 209 c6 dead-store / chain-extender sweeps).

(c): 34 banked bodies and 122 result-file bodies reach 0. The manifest lists each with its reason, and the
generator fails on a zero without one. Every one is either
- REUSE: the landing's shared locals are kept, so it is not a one-variable-per-value spelling (the landing
  body and templates, the reviewers' controls, lz[5]/lz[6] frame probes, earlier spellings of this code,
  the v3 reviewer's reuse bodies with loops or order respelled, delta-ablation masks in which the ground
  depth still shares `d`); or
- SET ASIDE under Q30: it needs a FAKE-family construct the landing body does not carry (dead stores,
  do-while(0) wraps, or the v2 chassis c6's two FAKE dead stores), and carries the landing's lz[6] as lz[6]
  or lz[5] (Q32).
Every other measured body misses. So no banked counting spelling reaches the target; how a body that misses
is classified does not bear on (c). Q30 classification is by construct family (the rule sets aside a
spelling that needs a construct whose rule requires a FAKE annotation, whether or not the body writes it).

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
- **(D)(1)** dumps: r11/dumps_table_landing.txt + r11/lreg_excerpts_landing.txt (the landing chassis;
r11/dumps_table.txt is the v2 (c6) chassis only; commands in the files' headers and §0); **(D)(2)** mechanisms named per variable (§3);
  **(D)(3)** under Q31: mechanism per variable (§3) and the banked search (§3A); **(D)(4)** alternatives (§2), sanctioned families (§5),
  permuter (§6).
- **(E)** names: `idx`, `nforce`, `delta`, `nbits`/`nbits2` under (E)(ii) (every value is a loop index /
  a force count / a Y delta / a bit count); `temp`, `work` under (E)(i).
- **(F)** each declaration carries a comment: several values, each named, citing Ruling 11 and this file.
- **(G)** fresh layer-2 cheat-reviewer on the landing (pending at the time of writing).
- **(H)** every other construct: §7.

## 5. FAKE-construct spellings, set aside under Q30 (Ruling 11 (D) § "FAKE-construct spellings are set aside")

Every such spelling measured is banked here with its score, family and the sentence of its rule that
requires the annotation. None of them is used in the landing body.

Families and their annotation requirements, quoted:
- dead store / self-assignment / combine-foldable chain-extender — dead-store-fake-exception.md,
  prerequisite 3: "**Mandatory annotation:** `/* FAKE: <one-line reason> */` or `// FAKE: <reason>` ON the
  statement."
- do-while(0) wrap — do-while-zero-exception.md, prerequisite 1: "**Inline `/* FAKE: ... */` or `// FAKE`
  annotation at the construct site** (not file-header prose), naming the observed effect".
- every v2-chassis body (the author's and the v2 reviewer's 1,064, whatever else it adds) carries c6's two
  dead stores, each `/* FAKE: dead store ... */`-annotated in the body (dead-store family, quoted above), which
  the landing body does not carry; that alone sets it aside. The v2 reviewer's res_c also used !FAKE empty-if
  and `++--` constructs; they are set aside on the same c6 ground without relying on their own families.

Closing spellings (all one-variable-per-value for the variable named, each reaching 0):
| variable | spelling | lines | where |
|---|---|---|---|
| work | per-value + `sq1 = 0;` dead store after temp's last use (end of the ellipsoid body / before `tot`) | 0 / 0 | r11/reviewer_probes/rv_wd_end.c, rv_wd_after2.c (v1 layer-2); v2 chassis c6 (variants_v2/c6.c) |
| delta | per-value + `dg = 0;` / `depth = 0;` dead store after the ellipsoid loop | 0 | variants/fam_delta_deadstore_end.c; c6 |
| work + delta | both | 0 | r11/reviewer_probes/rv_both.c; c6 |
| nforce | per-value + six single-level do-while(0) wraps (loads of vx, vy, vz, nforce_add, bits, bits2) | 0 | rejected/nforce-split-six-dowhile-wraps-closes-0.template.c (v2 layer-2) |
| nforce | per-value on the v2 chassis (two FAKE dead stores) + multi-level do-while(0) wrap combinations (49, the six-wrap template among them) | 0 | rejected/v2_review/res_g.txt (nf4_* rows at diff=0), mknf4.py |
| work / work + delta / nforce | the three closers above with `s32 lz[5]` (Q32 twins) | 0 | r11/banked/rv3_rv_wd_end_lz5.c, rv3_rv_both_lz5.c, rv3_nfdw_lz5.c |

Non-closing FAKE sweeps (for the record): v2 chassis, on idx / nforce / temp / nbits / nbits2 per-value
spellings: 72 dead stores at six anchors plus 5 per-value baselines (r11/ds.log, best 2) and 132 self-assigns / chain-extenders
(r11/ds2.log, best 2); the v2 layer-2's 500 single do-while(0) wraps over the five bodies (bests idx 62,
nforce 21, temp 42, nbits 2; rejected/v2_review/); v1 single probes (measurements_v1.md fam_* rows, fast2 lines:
delta self-assign 4, chain-extenders 117 / 6; temp 42; nforce 21 / 94; work chain 146, dead store placed
before temp's last use 26; idx 67-127; nbits 2 / 9).

Applying the Q30 test ("set aside only if it needs at least one FAKE- or !FAKE-annotated construct that
the reuse body does not carry, and it carries, unchanged, every FAKE- or !FAKE-annotated construct the
reuse body carries"): the landing body carries exactly one such construct, `s32 lz[6]` (OVERSIZED-LOCALS,
FAKE-annotated). Checked mechanically over all banked bodies (r11/fake_scan.md, r11/tools/fakescan.py, 210
bodies): every per-value, ablation, structural, family-sweep, reviewer and closing spelling declares `s32 lz[6]`
at the same site, except the frame probes (lzc5_*, rv6_lz*, rv7_lz*), the frame-census bodies (fr_* and the v8
reviewer's re-measurements rv8_fr_*), the lz[5] Q32 twins of the three closers and
rv3I_1000000_ff_loop_lz8 (lz[8]). The comment on the declaration differs across versions and many bodies
carry none (the v1 variants, the reviewers' probes); the declaration, which is what compiles, is the same.
Each closing spelling above adds dead stores or
do-while(0) wraps, so it is set aside. Spellings whose only FAKE construct is lz[6] count under (D)(3):
the per-value spellings, ablations and structural respellings of §2 and §3A, all nonzero. The v3
reviewer's closers with `s32 lz[5]` in place of lz[6] (rejected/v3_review/bodies/rv3_rv_wd_end_lz5,
rv3_rv_both_lz5, rv3_nfdw_lz5: 0) carry the same construct resized within the OVERSIZED-LOCALS range
annotation, byte-identically, so it is "unchanged" (owner Q32) and they are set aside like their lz[6]
twins. The frame
probes on the landing code (lzc5_* on c5; rv6_lz*, rv7_lz* on c7) are the reuse body with the array resized;
they bear on §7 only: lz[5] (lzc5_5, rv6_lz5, the Q32 twin) and lz[6] (rv6_lz6) reach 0, every other size
(lz[2], lz[3], lz[4], lz[7], lz[8]) misses. The frame-census bodies (r11/variants_v2/fr_*.c) are v2-chassis
spellings (work and delta split, c6's two FAKE dead stores) with lz[2] or two scalars plus each census
respelling; lz[2] is outside the 17-24 range, so they do not carry the landing's lz[6] unchanged and Q30 does
not set them aside: they count, and all miss (24 to 185 lines; r11/frame_census.md, r11/manifest.md).

## 6. Permuter campaigns (tools/permuter_campaign.py, `--stack-diffs`, standalone workspace)
Campaign 2 is the FAKE-free all-split body of this landing's code; campaign 3 (v2) ran from a body
carrying the two FAKE dead stores (14,250 iterations at harvest, r11/banked/camp3_final.json, the log's last line 14,255; best 490 from 625; its best finds re-create
reuses of `dist1`), recorded in proof v2 (commit 05f9b8638, §6); log tail r11/banked/perm_c6pv_campaign_tail.log.
Workspace: `r11/tools/mkperm.sh` (head.h + expanded body, preprocessed; GNU asm statements wrapped as
`#pragma _permuter b64literal` by `r11/tools/b64asm.py`, the func_8002DE20 method; target.o from
asm/funcs with include/gte_macros.inc). Standalone scores equal the in-TU ones (c2 0, pv_all 117,
r11pv_all 106; the campaigns' base scores are permuter scores, not lines).
- Campaign 1 (tmp/f187/perm_pvall, from pv_all: idx/nforce/temp/work/delta split, nbits shared, -j4):
  7,459 iterations, 399 s, 18 finds, best 665 from base 830. The best find names two fresh
  intermediates (`new_var = cpos[2]` before a sphere delta, `new_var2 = lz[1]`); none approaches 0.
- Campaign 2 (tmp/f187/perm_r11pv, from r11pv_all: all seven split, -j4, fresh seed): 17,657
  iterations (r11/banked/perm_r11pv_campaign_tail.log), 956 s, 37 finds, best 565 from base 755 (found at 94 s; nothing better in the remaining
  860 s; stopped on the fresh-seed window). What the best finds do (their diff.txt): 565 writes
  `dg = r < dz0;` inside the ellipsoid loop, i.e. makes the ground-depth local carry a second value
  in a later block (the `delta` reuse property, §3 delta, in a FAKE-shaped spelling); 600 stages
  `lz[0]` through the unrelated `bits2` (`bits2 = lz[0]; lzcount = bits2;`), a variable set twice
  (the `nbits` property, §3). Neither keeps one value per variable, and neither approaches 0.
- Across both campaigns (25,116 iterations) no find reaches the target. The finds that keep one value
  per variable only add named intermediates (campaign 1's best, 665 from 830, still far off); the best
  gains of campaign 2 re-create a reuse.

## 7. Everything else (H)

- **Islands**: 85 asm statements, all recognized as qualifying units by engine/gtemacro.py
  (gte_ldlvl 20, gte_stlvnl 16, gte_Lzc 12, gte_ldv0/rtv0tr/sqr0/lddp/gpl12 6 each, gte_stlvl 4, gte_gpf0
  3); keep-mode strip 0; the seven command units carry the Q29 post-DMPSX words; `($12)` spelled as
  the header writes it. Each island has a comment naming the macro and its pinned line range.
- **`lz[6]`**: OVERSIZED-LOCALS carve-out, frame proof in the declaration comment (0x40 locals = 8 spill
  + 32 orphan-USE phantom slots + 24 of the object: 8 written, a 16-byte unwritten tail); measured on c5: lz[2] frame 0x68, lz[4] 0x70 (24 lines each),
  lz[5] 0x78 (0), lz[7]/lz[8] 0x80 (24); on the landing chassis (r11/reviewer_probes/rv6_lz*): lz[2] 0x68,
  lz[3]/lz[4] 0x70, lz[5]/lz[6] 0x78 (0 lines), lz[7] 0x80 (24 lines each otherwise); rv7_lz2 / rv7_lz3 /
  rv7_lz8 repeat lz[2] 0x68, lz[3] 0x70 and measure lz[8] 0x80 on the landing chassis (24 lines each). lz[0]/lz[1] are the live LZC outputs. Phantom-slot producer
  census: r11/frame_census.md (14 spellings of the lz[2] form on the v2 chassis c6, which splits work and
  delta and carries two FAKE dead stores; the frame is unaffected by them (c6 and c7 are both 0x78); none
  gives 0x78 at zero cost).
- **Scr1F800000 at 0x1F800000** (`#define SCR`): the same constant-pointer scratchpad view as the
  landed func_80018094 (offset-derived type name) (`#define SCRV ((ScrV *)0x1F800000)`); field comments state what the code does
  with each; `force[0][3]` is the GNU zero-length trailing table (indexed by the packed bytes; the
  earlier `force[1][3]` was indexed past its bound; byte-identical).
- **`arg0` as `s16 *`**: the caller's type (65f4f1730); `*(s32 *)((u8 *)arg0 + 0xC)` and
  `*(s16 *)((u8 *)arg1 + 4)` are the file's existing offset-cast style (func_80018300).
- **`(&D_8008D118)[i]`**: the same table idiom as landed func_80018300 in this file (func_80018094 writes
  the equivalent `*(&D_8008D118 + x)`).
- **`vy_new`**: one value (both arms reach the store); the single store reproduces the target's
  shared `sw` at 0x80018CBC (the join label .L80018CB8 is its `lui $at`; evidence [s2 cont.] 9).
- **`dist2`**: one value (root, then `pen / dist2`, both reaching `gte_lddp(dist2)`).
- **Header comment**: the state -0xFF..-1 path pulls toward the anchor and then integrates like state
  < -0xFF (no `continue`); the unevidenced "rope/cloth" naming is dropped.
