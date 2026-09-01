# Evidence bank — func_8002EA24

Function: `func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq)` in
`src/code6cac_b.c` (~line 1610). Kengo hint in the source banner:
`sa_tan2/saTan2LinePrimInit`, 110i. Target: `asm/funcs/func_8002EA24.s`,
104 instructions.

## Session 1 (recon, 2026-07-30)

### Baseline
- `canonical func_8002EA24` → **ASM-PARTIAL**, 8/104 insns canonical-asm
  (GTE/cop2: `c2`, `lwc2`, `mtc2`, `swc2`), regions [21,23] [26,28] [65,65]
  [67,67].
- `sandbox --disable all` at HEAD → **score 18**, build **93** insns vs
  target 104, 10 regfix rules dropped.

### What the function does (fully understood — no dataflow mystery left)
1. Writes three `s16` deltas at `obj+0xF8/0xFA/0xFC` =
   `pos[i] - (*(s32**)(obj+0x60))[i]`. Target re-loads `lw $v1,0x60($t0)`
   three times (once per component); our build reproduces that for free —
   the intervening `sh` stores alias-kill the CSE, so no anti-CSE lever is
   needed here.
2. GTE: `lwc2 $0/$1` from `obj+0xF8`, two `nop`s, `mvmva 1,0,0,3,0`
   (`.word 0x4A486012`), then `swc2 $25/$26/$27` to `obj+0x100` — i.e.
   rotate the delta vector into `obj+0x100..0x108`.
3. Range rejects: `x = *(s32*)(obj+0x100)`, `z = *(s32*)(obj+0x104)`, each
   tested against `[-threshold, threshold]`; `-threshold` is computed ONCE
   (`negu $t1,$a2`) and reused by both tests.
4. `a0 = x*x + z*z`; reject if `r_sq < a0`; else `a0 = r_sq - a0` and take a
   square root through the 0x400-entry byte table `D_8008D118`, with the
   `>= 0x400` path using the GTE leading-zero-count unit (`mtc2 …,$30` =
   LZCS, `swc2 $31,…` = LZCR) to pick the shift.
5. y-range diamond over `obj+0xB0` (`min_y`/`max_y`) vs `obj+0x108`, then
   `return 1`.

### THE decisive finding — the honest distance is almost entirely GTE spelling
An instruction-level diff of the sandbox object against target showed that
**all 11 net-missing instructions at HEAD live in the GTE regions**, not in
the C dataflow. HEAD spells the GTE ops as cheat-forms that the sandbox
strips: `register … asm("t4")` pins, `addu %0,%1,$zero` moves, standalone
`__asm__("nop")`, and `.word 0xE99A0004`-style hardcoded encodings carrying a
pinned operand. Stripping those deletes 12 instructions and leaves 1 extra
(`xori`), hence 104-93 = 11.

Re-spelling the GTE work in PsyQ-macro shape — one `__asm__` per logical GTE
group, `%0` placeholders bound to an ordinary `"r"(pointer)` operand, and the
pipeline `nop`s **inside** the canonical template — is accepted by gas/maspsx
verbatim (`swc2 $26`/`$27` and `mtc2 %0,$30` all assemble; only `mvmva` still
needs `.word 0x4A486012`). That form builds **99** insns, leaving only **5**
structural gaps. It scores 20 rather than 18 purely because HEAD's hardcoded
`0($12)` templates match target's `$t4` operand register for free; see
"Residual" below.

### TOOL ARTIFACT (load-bearing; cost ~3 turns to find)
`tools/classify_inline_asm.split_template()` splits an asm template on the
literal `\n` but does NOT strip a following literal `\t`. So in
`"nop\n\tnop\n\tswc2 $31, 0(%0)"` the third instruction is seen as
`\tswc2 …`, whose first token is `\tswc2` — not in `CANONICAL_ASM_OPS`, does
not start with `.`, so it classifies **cheat**; since then no instruction in
the block classifies canonical, `engine.inlineasm._block_category` returns
"cheat" and the sandbox **silently deletes the whole GTE block**.
Multi-instruction templates whose FIRST instruction is not itself canonical
(e.g. leading `nop`s) MUST use bare `"\n"` separators, never `"\n\t"`.
(`.word …` blocks are immune because `CANONICAL_DOTWORD_RE` is a whole-text
search, not a first-token test.) This is an engine-side bug — recorded here,
not fixed, because `engine/` and `tools/` are outside a grind session's
allowed surface.

### Residual after the honest respelling (5 insns + register choice), score 20
1. Target copies **every** GTE operand into `$t4` through an extra
   `addu $t4, <reg>, $zero` at three sites:
   `addiu $v0,$t0,0xF8` + `addu $t4,$v0,$zero`;
   `addiu $v0,$t0,0x100` + `addu $t4,$v0,$zero`;
   `addu $t4,$a0,$zero` before `mtc2 $t4,$30`.
   Our build feeds the operand register directly (`lwc2 $0,0($v0)`,
   `mtc2 $a0,$30`), so it is 3 insns short AND mismatches the register on ~6
   canonical GTE instructions. **This single lever is worth roughly 9 of the
   20 points.** The third site proves the copy is a real C-level copy rather
   than a reload artifact: `$a0` (a0_var) is live afterwards, so the original
   genuinely held a *second* variable.
2. The tail `if (y + a0 < min_y) return 0; return 1;` folds to
   `slt $v0,$v0,$a2; xori $v0,$v0,1` (GCC `jump.c` store-flag /
   if-conversion), where target keeps the unfolded diamond
   `bnez $v0,END` / `addu $v0,$zero,$zero` (delay) / `addiu $v0,$zero,1`.
   Costs 2 missing insns + 1 wrong insn. The four EARLIER `return 0` sites do
   NOT fold — only the last, because it is the one followed by `return 1`.
3. Register assignment in the compare chain: our build puts `x` in `$a0` and
   `neg_threshold` in `$a1`; target wants `x` in `$a1` and `neg_threshold` in
   `$t1`. (This is exactly what regfix rules 3454-3456 paper over at HEAD.)

### Confirmed-dead C-level worries (do NOT re-derive)
- The `srl`/`sll`/`li 0x13` ordering cluster that regfix 3457-3460 rewrites is
  **already emitted in target order** by the plain-C form. Those four rules
  are stale relics of the pinned spelling.
- Likewise `slt $v0,$a1,$v0` (regfix 3461/3462) already matches.
- The triple `lw $v1,0x60($t0)` re-load needs no CSE-defeat lever.
- A single reused GTE-operand scratch variable does NOT create the `$t4`
  copies — see `rejected/shared-gte-operand-scratch.c` (score 24, KILLED).

### Artifacts
`tmp/grind/func_8002EA24/s1/` — `dis.sh` (disassemble the sandbox object),
`diff.sh` (side-by-side target/build), `cls.py` (classifier repro that found
the `\n\t` artifact), `form_A_score20.c` (whole file, honest form),
`candidate_body.c`, `rejected_body.c`, `build.txt`, `target.txt`.

### End-of-session state
`src/code6cac_b.c` was restored to HEAD so main's recorded floor stays 18.
The honest 99-insn form is banked at `memory/grind/func_8002EA24/candidate.c`
and is the intended starting point for session 2.

- [s1] canonical func_8002EA24 = ASM-PARTIAL, 8/104 insns canonical-asm (GTE c2/lwc2/mtc2/swc2), regions [21,23] [26,28] [65,65] [67,67].

- [s1] sandbox --disable all at HEAD = 18 (build 93 insns vs target 104, 10 regfix rules dropped, 334 cheat-asm constructs stripped file-wide).

- [s1] Honest PsyQ-macro GTE respelling = score 20, build 99 insns. Saved as memory/grind/func_8002EA24/candidate.c and tmp/grind/func_8002EA24/s1/form_A_score20.c.

- [s1] Function semantics fully mapped: writes s16 deltas pos[i]-(*(s32**)(obj+0x60))[i] to obj+0xF8/0xFA/0xFC; mvmva-rotates them into obj+0x100..0x108; range-rejects x and z against [-threshold,threshold] with -threshold computed once; a0=x*x+z*z, reject if r_sq<a0; sqrt of r_sq-a0 via the 0x400-entry byte table D_8008D118 using the GTE leading-zero unit (mtc2 $30 LZCS / swc2 $31 LZCR) for the >=0x400 path; then a y-range diamond over obj+0xB0 vs obj+0x108; return 1.

- [s1] Residual after the honest respelling is exactly 5 instructions plus register choice: (a) 3 missing `addu $t4,<reg>,$zero` copies before the lwc2/swc2/mtc2, which also cost ~6 register mismatches on the canonical GTE insns (worth ~9 of the 20 points); (b) the tail `if (...) return 0; return 1;` folds to `slt; xori $v0,$v0,1` where target keeps the unfolded diamond `bnez / addu $v0,$zero,$zero / addiu $v0,$zero,1` (2 missing + 1 wrong); (c) x lands in $a0 and neg_threshold in $a1, target wants $a1 and $t1.

- [s1] The mtc2-site copy proves target held a genuine SECOND C variable, not a reload artifact: $a0 (a0_var) is live after the mtc2 and is consumed by the later srlv.

- [s1] Only the LAST `return 0` is if-converted to xori; the four earlier ones keep the diamond — so the store-flag trigger is the 0/1 pair with the trailing `return 1`, not the comparison shape.

- [s1] ENGINE BUG (recorded, not fixed — engine/ and tools/ are outside a grind session's surface): tools/classify_inline_asm.split_template() splits templates on the literal \n but leaves a following literal \t attached, so in "nop\n\tnop\n\tswc2 $31, 0(%0)" the third instruction's first token is \tswc2, which is not in CANONICAL_ASM_OPS and does not start with '.', so it classifies as cheat; with no canonical instruction in the block, engine.inlineasm._block_category returns "cheat" and the sandbox silently deletes the entire GTE block. Multi-instruction templates whose first instruction is not itself canonical MUST use bare \n separators. .word blocks are immune because CANONICAL_DOTWORD_RE is a whole-text search.

- [s1] src/code6cac_b.c was restored to HEAD at end of session (re-verified sandbox = 18), so main's recorded floor is unchanged and the tree is clean apart from the memory/grind ledger.

## Session 2 (structural, 2026-07-30)

### Floor: 18 -> **9**  (build 102 insns vs target 104)

### The decisive finding - the `$t4` copies are HAND-WRITTEN ASM, not lost codegen
Session 1's H1 assumed target's `addu $t4, <src>, $zero` before each cop2 op
came from a second C variable GCC failed to coalesce. **That whole family is
dead.** Nine distinct pure-C spellings of the GTE operand were measured and
seven of them emit **byte-identical** code to the plain form (md5 of the
disassembly identical, score 20, 99 insns):

| spelling | score | codegen |
|---|---|---|
| plain block-local `s32 *vp = (s32*)(obj+0xF8)` (s1 candidate) | 20 | baseline |
| per-site `base`+`vp` alias PAIR (both pseudos referenced) | 20 | byte-identical |
| store address derived from load address (`rp = vp + 2`) | 20 | byte-identical |
| derived + pair | 20 | byte-identical |
| one enclosing scope, operand written `vp + 2` inline | 20 | byte-identical |
| one `__asm__` statement PER cop2 insn (N RTL uses of the pseudo) | 20 | byte-identical |
| address cast written inline in the operand, no named local | 20 | byte-identical |
| second local holding the LZCS operand (`s32 lz = a0_var;`) | 20 | byte-identical |
| pointers declared at function top (live across the delta stores) | 23 | worse |

GCC 2.7.2 coalesces every reg-reg copy feeding an asm operand regardless of
aliasing, reference count, derivation or scope. There is no pure-C spelling of
a GTE operand that produces the copy.

### Forensic census that resolved it
`addu $t4, X, $zero` appears in **46** target functions (`asm/funcs/*.s`) -
including plainly non-GTE ones (Pad_Prs, cpu_check_run_attack), so it is a
general original-toolchain artefact, not a GTE-macro tic. Cross-referencing
against `engine/queue.json` + `regfix.txt`/`asmfix.txt` leaves exactly **three**
matched, rule-free functions that contain it: `func_8001A67C`,
`func_800274BC`, `func_8004DDB4`. The first two are in
`inline_asm_canonical.txt` - **user-authorized 2026-06-10** as hand-written GTE
LZCS/LZCR blocks, and their authorized C spells the `$t4` routing *inside* a
single canonical `__asm__` block (`"addu $t4, %1, $zero"` / `"mtc2 $t4, $30"` /
two `nop`s / ... / `"swc2 $31, 0($t4)"`, with an `"=m"(sp_tmp)` output and a
`"$12"` clobber). `func_800274BC`'s authorized block is
instruction-for-instruction identical to func_8002EA24's LZC block, down to
`addu $t4, $sp, $zero`.

func_8002EA24 carries all three hand-asm signals the two authorizations cite:
splat `/* handwritten instruction */` tags (on `mtc2 $t4,$30` at 8002EB38 and
on `swc2 $26/$27` at 8002EA98/8002EA9C), `$t4` reused back-to-back for two
unrelated values, and unfilled GTE delay `nop`s.

### The measurement
Spelling BOTH GTE regions in the authorized-sibling shape:
- vector region only: score **13** (104 insns)
- LZC region only: score **16** (103 insns)
- both: score **9** (102 insns) - the entire GTE region then matches target
  exactly, including the two address computations, the three `$t4` copies, all
  8 canonical cop2 insns and the four unfilled delay nops.

This is a canonical-asm DISPOSITION question, not a self-approvable pure-C
result - see the ruling request in the session outcome. By the letter of
[[inline-asm-injection]] the hardcoded-`$N` template is the forbidden injection
pattern; the two in-tree sibling authorizations for the identical construct are
why it is a genuine classification question rather than a cheat.

### H2 (tail `xori` fold) - the pure-C axis is EXHAUSTED
Target keeps `bnez .L8002EBD0` / `addu $v0,$zero,$zero` (delay) /
`addiu $v0,$zero,1`; our build folds to `slt $v0,$v0,$a2` / `xori $v0,$v0,1`.
Measured on top of the score-20 form and re-confirmed on top of the score-9
form:

| tail shape | score | codegen |
|---|---|---|
| shared end label (`ret = 0; goto end; ... end: return ret;`) | 20 / 9 | byte-identical |
| reversed final comparison (`if (min_y <= y + a0) return 1; return 0;`) | 20 | byte-identical |
| if/else, both arms set `ret` | 20 | byte-identical |
| ternary `return (...) ? 0 : 1;` | 10 | worse (extra insn) |
| swap the order of the two y-range tests | 17 | worse |
| `goto reject;` for both rejects + inline `return 1` | 30 | much worse |

jump.c's store-flag if-conversion fires because every one of these source
shapes collapses to the same RTL. The documented closure for this exact shape
is [[dead-store-fake-exception]] (dead `ret = 1;` INSIDE the else arm) -
last-resort, FAKE-annotated, layer-2 reviewed; NOT attempted this session
because the modality was structural and the GTE ruling gates the function.

### H3 (x in `$a0` vs target `$a1`, neg_threshold in `$a1` vs target `$t1`)
Did NOT move when the GTE region closed (session 1 predicted it might). It is
now the larger half of the residual: 6 of the 9 points (`lw`, `negu`, three
`slt`, the `mult`). Declaration-order levers measured on top of the score-9
form: `xdefer` (x declared uninitialised, loaded as the first statement),
`a0first` (a0_var declared ahead of x), `negfirst` (-threshold materialised
before the x load) - all three **byte-identical**; `zdecl` (z given an up-front
initialiser) = 12, worse. `twovars` (separate `d2` pseudo) is invalid C89
declaration placement and mis-builds - do not re-try in that spelling.

### Artifacts
`tmp/grind/func_8002EA24/s2/` - `mk.py` (slot-based body generator: GTE / MTC2
/ TAIL / CMP variants), `swap.py` (splice a body into src/code6cac_b.c),
`run.sh` (score a spec list), `cmp.sh` (score + md5 of the emitted
disassembly, which is what proved the byte-identical kills), `body_*.c`,
`dis_*.txt`, `ledger.py`.

- [s2] The GTE-operand "second C variable" family is DEAD: 8 pure-C spellings (alias pair, derived address, derived+pair, one-scope, per-instruction asm statements, inline cast operand, second LZCS local, plus the s1 baseline) all emit BYTE-IDENTICAL code (score 20, 99 insns, identical disassembly md5). Only declaring the pointers at function top changed anything, and it was worse (23). GCC 2.7.2 coalesces every reg-reg copy feeding an asm operand regardless of aliasing, reference count, derivation or scope.

- [s2] FORENSIC CENSUS: `addu $t4, X, $zero` appears in 46 target functions, including non-GTE ones, so it is a general original-toolchain artefact. Of those 46, exactly 3 are matched with zero regfix/asmfix rules and not queued: func_8001A67C, func_800274BC, func_8004DDB4. func_8001A67C and func_800274BC are both in inline_asm_canonical.txt, user-authorized 2026-06-10, as hand-written GTE LZCS/LZCR blocks whose authorized C puts the $t4 routing inside a single canonical __asm__ block.

- [s2] func_8002EA24's LZC block is instruction-for-instruction identical to func_800274BC's authorized block (addu t4,<val>; mtc2 t4,$30; nop; nop; addu t4,$sp; swc2 $31,0(t4)) and carries the same three hand-asm signals: splat "handwritten instruction" tags (mtc2 $t4,$30 at 8002EB38; swc2 $26/$27 at 8002EA98/8002EA9C), $t4 reused back-to-back for two unrelated values, unfilled GTE delay nops.

- [s2] Spelling both GTE regions in the authorized-sibling shape drops the honest floor 18 -> 9 (build 102 insns vs target 104): vector region alone = 13, LZC region alone = 16, both = 9. The whole GTE region then matches target byte-for-byte including both address computations, the three $t4 copies and the four unfilled delay nops.

- [s2] H2's pure-C axis is exhausted: shared-end-label, reversed final comparison and if/else-both-arms are all BYTE-IDENTICAL to the plain form; ternary = 10, test-order swap = 17, double-goto-reject = 30. The remaining documented closure for this exact shape is [[dead-store-fake-exception]], not a pure-C restructure.

- [s2] H3 did NOT move when the GTE region closed (session 1 predicted it would). It is now 6 of the 9 residual points. xdefer / a0first / negfirst declaration-order levers are byte-identical; zdecl (z initialised up-front) = 12, worse.

- [s2] The sandbox's inline-asm classifier keeps a multi-instruction block if ANY instruction in it is canonical, so a block whose first instruction is `addiu $v0, %0, 0xF8` survives as long as the block also contains lwc2/swc2/mtc2 (and uses bare newline separators per the s1 tool artifact). That is what makes the authorized-sibling spelling measurable in the cheat-invisible sandbox at all.

## Session 3 (structural, 2026-07-30)

### Floor: 9 (unchanged) — but the Judge's shape constraint is satisfied at ZERO cost

The Judge's binding constraint for this session was: keep the LZC block ONLY in
the exact `func_800274BC`-authorized shape, and re-spell the vector/mvmva block
MINIMALLY — operand address computed in C and bound via `%N`, template limited
to the `$t4` copy + `lwc2`/`swc2` + the mvmva `.word`, **no** hardcoded
`addiu $v0, %0, 0xF8` inside the template and **no** `$2` clobber. That form was
built and measured:

| vector-block spelling | score | codegen |
|---|---|---|
| session-2 form (both `addiu`s hardcoded in the template, `$2` clobbered) | 9 | baseline, md5 `1fc26fe12849` |
| **TWO `__asm__` statements, `vin`/`vout` computed in C, bound `"r"()`, no `$2` clobber** | **9** | **BYTE-IDENTICAL (md5 `1fc26fe12849`)** |
| ONE `__asm__` statement taking both pointers as `%0`/`%1` | 13 | worse (md5 `d74f8524ff76`) |

So the constrained spelling costs nothing: both address computations now come
from ordinary C and land as `addiu $v0,$t0,0xF8` / `addiu $v0,$t0,0x100` in
target's exact positions and target's exact register. **The split into two
statements is load-bearing** — a single block forces GCC to materialise both
pointers ahead of the `lwc2`, emitting `addiu $v1,$t0,0xF8` +
`addiu $v0,$t0,0x100` back-to-back (banked as
`rejected/gte-single-block-two-operands-score13.c`).

### SANDBOX ARTIFACT — the four GTE `nop`s are stripped, not missing
The cheat-invisible sandbox removes bare `nop` lines from an otherwise-kept
canonical `__asm__` block (the surrounding `addu $t4, %0, $zero` survives
because it carries a `%N` placeholder). So the sandbox disassembly of the
score-9 form has NO GTE pipeline nops — 2 before the mvmva and 2 after the
mtc2 — even though they are in the templates and appear in a real build. Do
NOT chase them as a codegen gap: modulo those four, the ENTIRE GTE region
(both address computations, all three `addu $t4,<reg>,$zero` copies, all 8
cop2 instructions) matches target instruction-for-instruction.

### THE session's decisive finding — H5 is ONE missing live-range conflict
The cc1 `.greg` dump (`-dg`, dumped to `tmp/grind/func_8002EA24/s2/base.i.greg`,
function slice at `greg_8002EA24.txt`) gives the allocator's actual state for
the score-9 form:

```
;; 14 regs to allocate: 102 97 98 110 96 109 72 103 118 104 101 74 100 75
;; 96  conflicts: 72 74 75 96 97 104   2 29 64 66            <- x
;; 98  conflicts: 72 75 98 100 101 102 103 118  2 3 12 29    <- a0_var ; preferences: 4
;; 104 conflicts: 72 74 75 96 97 104   2 29                  <- neg_threshold
;; dispositions: 72 in 8(t0)  74 in 6(a2)  75 in 7(a3)  96 in 4(a0)
;;               97 in 3(v1)  98 in 4(a0)  104 in 5(a1)
```

Identification: 72=`obj`, 74=`threshold`, 75=`r_sq`, 96=`x`, 97=`z`,
98=`a0_var`, 104=`neg_threshold`.

`global.c` allocates allocnos in the priority order printed above
(`allocno_compare`: `floor_log2(n_refs)*n_refs / live_length`) and `find_reg`
takes the FIRST hard register in `REG_ALLOC_ORDER` that is not conflicting;
`prune_preferences` only lets an allocno skip a register preferred by a
**lower-priority allocno it conflicts with**. Walking that by hand reproduces
our build exactly:
- 97 (`z`) is 2nd, conflicts hard `$v0` → takes `$v1`.
- 98 (`a0_var`) is 3rd, conflicts `$v0`/`$v1`/`$t4` → takes `$a0`.
- 96 (`x`) is 5th; `$v0` conflicts and `$v1` is held by `z` (a conflict), so it
  takes `$a0` — **legal because `x` does NOT conflict with `a0_var`** (x dies at
  the `mult`, a0_var is born at the following `addu`).
- 104 (`neg_threshold`) is 10th; blocked from `$v0`, `$v1`(z), `$a0`(x) → `$a1`.

Target wants `x` in `$a1` and `neg_threshold` in `$t1`. Feed the same
first-fit walk the single extra conflict **`a0_var` <-> {`x`, `neg_threshold`}**
and BOTH fall out automatically:
- `x` additionally blocked from `$a0` → `$a1`.
- `neg_threshold` blocked from `$v0`, `$v1`(z), `$a0`(a0_var), `$a1`(x),
  `$a2`(threshold), `$a3`(r_sq), `$t0`(obj) → next in order is `$t1`.

**All six H5 register mismatches are one missing conflict.** That conflict
requires `a0_var`'s live range to start BEFORE the range-test chain.

### Measured confirmation of the mechanism (and why it is not yet closable)
| form | score | effect on the compare chain |
|---|---|---|
| base (score-9 candidate) | 9 | `lw a0,256` / `negu a1,a2` |
| `accearly` — `a0_var = x*x + z*z;` computed before ALL four range tests | 19 | **`negu t1,a2` — neg_threshold IS in `$t1`**, x -> `$v1`, z -> `$a1` |
| `accpre` — `a0_var = x*x;` before x's test, `a0_var += z*z;` after z's test | 15 | x -> `$v1`, neg -> `$a1` |
| `accsplit` — `a0_var = x*x;` after x's test, `+= z*z` after z's test | 14 | x -> `$a0`, neg -> `$a1` (a0_var shares `$a0` with x, so only one reg is blocked) |
| `accmid` — `a0_var = x*x + z*z;` between the x test and the z test | 18 | x -> `$v1`, neg -> `$a1` |
| `xzptr` — x/z/y read as `v[0]`/`v[1]`/`v[2]` off a live `s32 *v` | 11 | x -> `$a0`, neg -> `$a1`; +2 insns |

`accearly` is the direct proof of the mechanism: making `a0_var` live across
the chain moves `neg_threshold` into `$t1`, exactly target's allocation.

**Why it does not close the function.** Every shape that makes `a0_var` live
early also SETS it early, which hoists the two `mult`/`mflo` pairs ahead of the
`slt` chain — target emits them after. And once `a0_var` is born before `x`'s
last use, `x` dies immediately and stops conflicting with `z`, so `x` falls into
`$v1` (earlier in `REG_ALLOC_ORDER` than `$a1`) instead of target's `$a1`.
Target's allocation needs `a0_var` live from before the x test WHILE `x` stays
live past `z`'s load — i.e. `a0_var`'s live range must begin at a set that is
NOT the multiply. No pure-C shape measured this session produces that.

### The open question this hands to the next session
In target, nothing visibly writes `$a0` between the prologue
(`addu $t0,$a0,$zero`, which kills the `obj` parameter) and
`addu $a0,$v0,$v1` (the sum of squares). Yet the allocator behaved as though
`$a0` were occupied across the whole compare chain. Either (a) the original C
had a value live in `$a0` there that is never referenced in that window and
dies at/just before the `mult` pair, or (b) `a0_var`'s allocno acquired the
conflict from a source outside this model (local-alloc quantity merging, or a
copy preference propagated by `expand_preferences`). Resolving which is the
whole of H5.

- [s3] The Judge-constrained minimal vector-block spelling (two __asm__ statements; `vin`/`vout` computed in C and bound via "r"(); templates limited to `addu $t4,%0,$zero` + lwc2/swc2 + the mvmva .word; no hardcoded `addiu $v0` and no `$2` clobber) is BYTE-IDENTICAL to the session-2 form: score 9, 102 insns, disassembly md5 1fc26fe12849. The constraint costs nothing.

- [s3] The two-statement split IS load-bearing: one __asm__ block taking both pointers as %0/%1 makes GCC materialise both addresses before the block (`addiu $v1,$t0,0xF8` + `addiu $v0,$t0,0x100` back-to-back ahead of the lwc2) = score 13. Banked as rejected/gte-single-block-two-operands-score13.c.

- [s3] SANDBOX ARTIFACT: the cheat-invisible sandbox strips bare `nop` lines out of an otherwise-kept canonical __asm__ block (the neighbouring `addu $t4, %0, $zero` survives because it carries a %N placeholder). All four GTE pipeline nops are therefore absent from the sandbox disassembly even though they are in the templates and appear in a real build. Modulo those four, the entire GTE region matches target instruction-for-instruction.

- [s3] H5's six register mismatches reduce to exactly ONE missing live-range conflict: a0_var <-> {x, neg_threshold}. Derived from the cc1 .greg conflict lists plus global.c's first-fit allocation (allocno_compare priority = floor_log2(n_refs)*n_refs/live_length; find_reg takes the first non-conflicting hard reg in REG_ALLOC_ORDER; prune_preferences only lets an allocno skip a register preferred by a LOWER-priority allocno it conflicts with). Adding that one conflict to the hand-walk puts x in $a1 and neg_threshold in $t1 automatically.

- [s3] MEASURED CONFIRMATION: accearly (a0_var = x*x + z*z computed before all four range tests) emits `negu t1,a2` — neg_threshold lands in $t1, target's register — purely because a0_var is then live across the chain. Score 19 (the two mult/mflo pairs hoist ahead of the slt chain, which target does not do).

- [s3] The a0_var-live-early family is KILLED as a closing form but CONFIRMED as the mechanism: accearly 19, accmid 18, accpre 15, accsplit 14, xzptr 11 — all worse than the score-9 base. Every shape that makes a0_var live early also SETS it early, hoisting the mults; and once a0_var is born before x's last use, x stops conflicting with z and falls into $v1 (earlier in REG_ALLOC_ORDER) instead of target's $a1.

- [s3] OPEN: in target nothing visibly writes $a0 between the prologue (`addu $t0,$a0,$zero`, which kills the obj parameter) and `addu $a0,$v0,$v1`, yet the allocator behaved as if $a0 were occupied across the compare chain. Either the original C had a value live in $a0 there that is never referenced in that window and dies at/just before the mult pair, or a0_var's allocno acquired the conflict from local-alloc quantity merging / expand_preferences copy-preference propagation. Resolving which IS H5.

- [s2] The Judge-constrained minimal vector-block spelling (two __asm__ statements; vin/vout computed in C and bound via "r"(); templates limited to `addu $t4,%0,$zero` + lwc2/swc2 + the mvmva .word; no hardcoded `addiu $v0`; no `$2` clobber) is BYTE-IDENTICAL to the session-2 form -- score 9, 102 insns, disassembly md5 1fc26fe12849. The constraint costs nothing and this is the banked candidate.

- [s2] The two-statement split IS load-bearing: one __asm__ block taking both GTE pointers as %0/%1 makes GCC materialise both addresses before the block (`addiu $v1,$t0,0xF8` + `addiu $v0,$t0,0x100` back-to-back ahead of the lwc2) = score 13. Banked as rejected/gte-single-block-two-operands-score13.c.

- [s2] SANDBOX ARTIFACT (do not chase as a codegen gap): the cheat-invisible sandbox strips bare `nop` lines out of an otherwise-kept canonical __asm__ block, while the neighbouring `addu $t4, %0, $zero` survives because it carries a %N placeholder. All four GTE pipeline nops (2 before the mvmva, 2 after the mtc2) are therefore absent from the sandbox disassembly even though they are in the templates and appear in a real build. Modulo those four, the ENTIRE GTE region -- both address computations, all three $t4 copies, all 8 cop2 instructions -- matches target instruction-for-instruction.

- [s2] cc1 .greg for the score-9 form: `;; 14 regs to allocate: 102 97 98 110 96 109 72 103 118 104 101 74 100 75`; `96 conflicts: 72 74 75 96 97 104 2 29 64 66`; `98 conflicts: 72 75 98 100 101 102 103 118 2 3 12 29` with `98 preferences: 4`; `104 conflicts: 72 74 75 96 97 104 2 29`; dispositions 72 in 8, 74 in 6, 75 in 7, 96 in 4, 97 in 3, 98 in 4, 104 in 5. Identification: 72=obj, 74=threshold, 75=r_sq, 96=x, 97=z, 98=a0_var, 104=neg_threshold.

- [s2] H5's six register mismatches are exactly ONE missing conflict: a0_var <-> {x, neg_threshold}. Hand-walking global.c's first-fit allocation over the printed conflict graph reproduces our build precisely, and adding that one conflict yields target's x=$a1 / neg_threshold=$t1 automatically.

- [s2] MEASURED CONFIRMATION of the mechanism: `accearly` (a0_var = x*x + z*z computed before all four range tests) emits `negu t1,a2` -- neg_threshold in target's $t1 -- purely from the new live-range overlap. Score 19 because the two mult/mflo pairs hoist ahead of the slt chain.

- [s2] The a0_var-live-early family is KILLED as a closing form: accearly 19, accmid 18, accpre 15, accsplit 14, xzptr 11, all worse than the score-9 base. Every shape that makes a0_var live early also SETS it early (hoisting the mults), and shortens x's live range so x falls into $v1 instead of $a1.

- [s2] OPEN QUESTION handed forward: in target, nothing visibly writes $a0 between the prologue `addu $t0,$a0,$zero` (which kills the obj parameter) and `addu $a0,$v0,$v1`, yet the allocator behaved as if $a0 were occupied across the whole compare chain. Either the original C had a value live in $a0 there that is never referenced in that window and dies at/just before the mult pair, or a0_var's allocno acquired the conflict from local-alloc quantity merging or expand_preferences copy-preference propagation.

- [s2] src/code6cac_b.c was restored to HEAD at end of session (the score-9 form is banked in memory/grind/func_8002EA24/candidate.c only), so main's recorded floor and the tree are unchanged apart from the memory/grind ledger and tmp/ scratch.

- [s2] H6 (the tail 0/1 diamond, 3 of the 9 residual points) was NOT attempted this session: the modality was structural and session 2 already measured the pure-C tail axis exhausted (six shapes, three byte-identical). Its documented closure is [[dead-store-fake-exception]], which requires a /* FAKE */ annotation and layer-2 review.

## Session 4 (structural, 2026-07-30)

### Floor: 9 (unchanged). What this session bought: the ALLOCATOR MODEL WAS WRONG, and the corrected model narrows H5 to a precisely stated requirement.

Session 3 modelled `find_reg` as "first non-conflicting hard register in
REG_ALLOC_ORDER". That model is incomplete, and the missing part is exactly
where the target's assignment must come from. Read out of
`tools/gcc-2.7.2/global.c` (READ-ONLY -- the compiler is frozen,
[[no-compiler-divergence]]):

1. **MIPS defines no `REG_ALLOC_ORDER`** (no match in `config/mips/mips.h`), so
   the scan really is ascending hard-reg number. That part of the model holds.
2. `find_reg` runs **TWO passes** (global.c:1012-1044). Pass 0's exclusion set is
   `used1 | ~regs_used_so_far | regs_someone_prefers[allocno]`; only if pass 0
   finds nothing does pass 1 fall back to plain conflicts.
3. `regs_used_so_far` is **vacuous on MIPS**: global.c:353-355 pre-marks every
   `call_used_regs` entry, which here is $v0-$v1, $a0-$a3, $t0-$t9. So the only
   extra exclusion in pass 0 is `regs_someone_prefers`.
4. `prune_preferences` (global.c:851-899) sets
   `regs_someone_prefers[A] = union of hard_reg_full_preferences[B]` over
   allocnos B that **conflict with A and have LOWER priority** (are allocated
   later), minus (when B is no larger than A) the registers A itself prefers.
5. After the pass loop, find_reg **overrides** `best_reg` with a register from
   `hard_reg_copy_preferences[allocno]` (then `hard_reg_full_preferences`) of the
   same class if one is still free (global.c:1057-1080). So an allocno's OWN
   preference can flip its assignment away from the first-fit answer.
6. Preferences are seeded by `set_preference` on each store (global.c:1404) and
   then **spread by `expand_preferences`** (global.c:798-841) across ANY
   `single_set` insn carrying a `REG_DEAD` note for a non-conflicting allocno --
   not just reg-reg copies. Ordering matters: `expand_preferences` runs at
   global.c:523 and `prune_preferences` at :548, so a preference can propagate to
   a pseudo that does NOT conflict with the hard register and survive the pruning
   that kills it on the original owner.

### Measured allocator state at x's allocation (the decisive datum)
`tmp/gccdbg/cc1` carries a read-only `BB2_FINDREG_DEBUG=<pseudo>` diagnostic hook
(the shipped `tools/gcc-2.7.2/build/cc1` does NOT -- `strings` finds no
`FINDREGDBG`). Running it over the score-9 form
(`tmp/grind/func_8002EA24/s3/findreg.sh 96`, output `findreg_96.txt`; six blocks
across the file, ours is the one whose conflict set matches the `.greg` state at
x's turn):

```
FINDREGDBG pseudo=96 alt=0 acc=0 retry=0
  conflicts:       2 3 29                 <- $v0, z's $v1, $sp
  someone_prefers: 6 7                    <- $a2, $a3
  pass0_used:      0 1 2 3 6 7 16..23 26..31
  own_copy_prefs:  (empty)
  own_full_prefs:  (empty)
```

`$a0` (4) is simply the first register not excluded, and x has no preference of
its own to override it. The `6 7` come from `threshold` (74) and `r_sq` (75):
lower-priority allocnos that conflict with x and prefer their incoming argument
registers.

### Why the incoming-parameter route to an `$a0` preference is CLOSED
`obj` (allocno 72) is the first parameter, so it starts with a copy preference
for `$a0` from the prologue `addu $t0,$a0,$zero`, and it IS lower priority than
x (7th vs 5th) -- so it ought to put `$a0` into `someone_prefers[x]`. It does
not, because `prune_preferences` line 877 first removes from an allocno's
preferences every register it CONFLICTS with, and obj's `.greg` hard conflicts
are `2 3 4 5 6 7 12 29` -- including `$a0` itself (obj is born at the prologue
copy, where the incoming argument hard regs are still live). A pseudo live from
function entry can never keep a preference for an argument register.

### The corrected statement of H5
Target needs `$a0` excluded from BOTH `x` and `neg_threshold`. Under the
corrected model there are exactly two ways:
  (a) an allocno that **prefers `$a0`**, **conflicts with** x and neg_threshold,
      and is **LOWER priority than both**. The only allocno in this function that
      prefers `$a0` is `a0_var` (98; `.greg` prints `98 preferences: 4`, inherited
      through `expand_preferences`, and it does not conflict with hard `$a0`, so
      the preference survives pruning). Session 3 said "one missing conflict"; the
      correct requirement is **conflict AND priority-below** -- strictly harder.
      Note `accearly` satisfies both (lengthening a0_var's live range also lowers
      its priority), which is why it alone reached `negu $t1,$a2`.
  (b) `x` acquires its OWN copy/full preference for `$a1` (the find_reg override
      at global.c:1057). `pos` (parameter 2) is the only `$a1`-preferring pseudo
      and it does NOT conflict with x, so `expand_preferences` may legally
      propagate to x -- but only across an insn that both sets x's pseudo and
      carries the `REG_DEAD` note for `pos`.

### Measured this session (all `sandbox --disable all`; base = 9, md5 1fc26fe12849)
| variant | score | compare-chain registers |
|---|---|---|
| `xtop` -- x declared at FUNCTION scope, loaded after the block decls | **9, byte-identical** | unchanged |
| `tshare` -- x IS the delta temp (all three deltas) | 28 | x -> $v1 |
| `tshare1` -- x IS the last delta temp only | 19 | x -> $v1 |
| `zshare` -- z IS the last delta temp | 16 | unchanged (x $a0, z $v1, neg $a1) |
| `negshare` -- neg_threshold IS the last delta temp | 16 | **x -> $a1 (target!)**, neg -> $v1, z -> $a0 |
| `accshare` -- a0_var IS the last delta temp (dead early set) | 16 | unchanged |
| `negearly` -- `-threshold` computed at function scope | 11 | unchanged |
| `vinlive` -- y read as `vin[4]`, so vin lives across the chain | 14 | unchanged |
| `minmaxearly` -- min_y/max_y zeroed at function scope | 18 | unchanged |

Four things are settled by that table:
- **A dead early set does NOT lengthen a live range** (`accshare`): liveness kills
  the value at the re-set, so no conflict with x/neg is created. The obvious
  reading of session 3's frontier -- "give a0_var a cheap early set that is not
  the multiply" -- is DEAD; the early value must be genuinely live-out into the
  chain.
- **Extra pseudos live across the chain do not take `$a0`** (`vinlive`,
  `minmaxearly`): adding conflicts is not enough; the occupier must be the
  `$a0`-preferring allocno.
- **The delta-temp-sharing family is dead on cost**, even though it is the only
  lever that has ever put x in `$a1`: sharing a compare-chain variable with the
  delta temp costs ~7 points in the delta/GTE region even when it changes no
  register at all (zshare 16, accshare 16), which no register win repays.
- `xtop` re-confirms for the third time that declaration position alone is inert.

### Artifacts
`tmp/grind/func_8002EA24/s3/` -- `run.sh` (splice+score+md5), `gen.py` (variant
generator, one literal-substitution recipe per variant), `sweep.sh` (gen+score+
register read-off), `findreg.sh` (BB2_FINDREG_DEBUG dump), `bank.py`,
`findreg_96.txt`, `greg_8002EA24.txt`, `base.i`, per-variant `*.c` and `dis_*.txt`.

- [s4] find_reg is NOT plain first-fit. GCC 2.7.2 global.c:1012-1044 runs two passes; pass 0 additionally excludes regs_someone_prefers[allocno], and regs_used_so_far is vacuous on MIPS because global.c:353-355 pre-marks every call_used_reg ($v0-$v1,$a0-$a3,$t0-$t9). MIPS defines no REG_ALLOC_ORDER, so the scan is ascending hard-reg number. After the pass loop find_reg OVERRIDES best_reg with a free register from the allocno's own hard_reg_copy_preferences / hard_reg_full_preferences of the same class (global.c:1057-1080).

- [s4] prune_preferences (global.c:851-899) builds regs_someone_prefers[A] from the full preferences of allocnos that CONFLICT with A and are LOWER priority, and it first strips from every allocno's preference set the registers that allocno itself conflicts with (line 877). expand_preferences (global.c:798-841) runs BEFORE pruning (523 vs 548) and spreads preferences across any single_set insn carrying a REG_DEAD note for a non-conflicting allocno -- not only reg-reg copies -- so an inherited preference can survive on a pseudo where the original owner's conflict would have killed it.

- [s4] MEASURED with the read-only BB2_FINDREG_DEBUG hook in tmp/gccdbg/cc1 (the shipped tools/gcc-2.7.2/build/cc1 does NOT carry it): at x's allocation in the score-9 form, conflicts={2,3,29}, someone_prefers={6,7} (from the lower-priority threshold/r_sq param allocnos preferring $a2/$a3), own_copy_prefs and own_full_prefs both EMPTY, pass0_used={0,1,2,3,6,7,16-23,26-31}. $a0 is simply the first non-excluded register and x has no preference to override it.

- [s4] The incoming-parameter route to an $a0 preference is CLOSED: obj (allocno 72) is lower priority than x and does start with an $a0 copy preference from the prologue `addu $t0,$a0,$zero`, but prune_preferences strips it because obj's hard conflicts include $a0 itself (.greg: `72 conflicts: ... 2 3 4 5 6 7 12 29`). Any pseudo live from function entry conflicts with the argument registers and therefore cannot keep a preference for one.

- [s4] H5 restated correctly: $a0 must be excluded from BOTH x and neg_threshold, which needs either (a) an allocno that prefers $a0, conflicts with x and neg, AND is LOWER priority than both -- a0_var is the only $a0-preferring allocno (`98 preferences: 4`), so this is session 3's conflict requirement PLUS a priority-below requirement (accearly satisfies both, which is why it alone produced `negu $t1,$a2`); or (b) x acquiring its OWN preference for $a1 via find_reg's post-pass override, which expand_preferences could only supply across an insn that sets x's pseudo and carries the REG_DEAD note for `pos`.

- [s4] KILLED: a dead early set does not lengthen a live range. `accshare` (a0_var holds the third delta, whose value is stored and never read again) scores 16 with the compare-chain registers COMPLETELY unchanged. The early value must be genuinely live-out into the chain for a conflict to exist.

- [s4] KILLED: merely adding pseudos live across the compare chain does not put anything in $a0. `vinlive` (y read as vin[4], so the GTE input pointer lives across the chain) = 14 and `minmaxearly` (min_y/max_y zeroed at function scope, real values used after the chain) = 18, both with unchanged compare-chain registers.

- [s4] The delta-temp-sharing family is the ONLY lever measured so far that puts x in target's $a1 (`negshare`: neg_threshold IS the third delta temp -> `lw $a1,0x100($t0)`), but it is dead on cost: sharing costs ~7 points in the delta/GTE region even when it changes no register at all (zshare 16, accshare 16), and negshare's own total is 16 vs the 9 base, with neg in $v1 and z in $a0. tshare 28, tshare1 19 (x falls to $v1, not $a1).

- [s4] `xtop` (x declared at function scope and loaded after the block's declaration list) is BYTE-IDENTICAL to the base (score 9, md5 1fc26fe12849) -- a third independent confirmation that declaration position alone is inert here. NOTE the C89 trap that cost two builds: a statement may not precede declarations in a block, so a function-scope x must be loaded AFTER the block's declaration list (an assignment spliced in among the declarations mis-builds to a 31-insn stub scoring 85-94).

- [s4] src/code6cac_b.c was restored to HEAD at end of session (`git checkout --`), so main's recorded floor stays 18 and the tree is clean apart from the memory/grind ledger, the six new rejected/ forms and tmp/ scratch.

- [s3] find_reg is NOT plain first-fit: two passes (global.c:1012-1044), pass 0 excluding regs_someone_prefers[allocno]; regs_used_so_far is vacuous on MIPS (global.c:353-355 pre-marks every call_used_reg); MIPS defines no REG_ALLOC_ORDER so the scan is ascending hard-reg number; and after the pass loop find_reg overrides best_reg with a free register from the allocno's own hard_reg_copy_preferences / hard_reg_full_preferences (global.c:1057-1080).

- [s3] prune_preferences (global.c:851-899) builds regs_someone_prefers[A] from the full preferences of allocnos that conflict with A and are LOWER priority, after first stripping from every allocno's preference set the registers that allocno itself conflicts with (line 877). expand_preferences (global.c:798-841) runs BEFORE pruning (523 vs 548) and spreads preferences across any single_set insn carrying a REG_DEAD note for a non-conflicting allocno -- not only reg-reg copies.

- [s3] MEASURED with the read-only BB2_FINDREG_DEBUG hook in tmp/gccdbg/cc1 (the shipped tools/gcc-2.7.2/build/cc1 has no FINDREGDBG strings): at x's allocation in the score-9 form, conflicts={2,3,29}, someone_prefers={6,7}, own_copy_prefs and own_full_prefs both EMPTY, pass0_used={0,1,2,3,6,7,16-23,26-31}. $a0 is the first non-excluded register and x has no preference to override it.

- [s3] The incoming-parameter route to an $a0 preference is CLOSED: obj (72) is lower priority than x and starts with an $a0 copy preference from the prologue addu $t0,$a0,$zero, but prune_preferences strips it because obj's hard conflicts include $a0 (.greg: 72 conflicts ... 2 3 4 5 6 7 12 29).

- [s3] H5 restated correctly: $a0 must be excluded from BOTH x and neg_threshold, which needs either (a) an allocno that prefers $a0, conflicts with x and neg, AND is lower priority than both -- a0_var (98, '.greg preferences: 4') is the only $a0-preferring allocno, so this is session 3's conflict requirement PLUS a priority-below requirement, and accearly satisfies both, which is why it alone produced negu $t1,$a2 -- or (b) x acquiring its OWN preference for $a1 via find_reg's post-pass override, which expand_preferences can only supply across an insn that sets x's pseudo and carries the REG_DEAD note for the pos parameter.

- [s3] KILLED: a dead early set does not lengthen a live range (accshare = 16, compare-chain registers unchanged). The early value must be genuinely live-out into the chain, which is exactly what forces the mult hoist that killed the accearly family in session 3.

- [s3] KILLED: merely adding pseudos live across the chain does not put anything in $a0 (vinlive = 14, minmaxearly = 18, both with unchanged compare-chain registers).

- [s3] The delta-temp-sharing family is the only lever ever measured to put x in target's $a1 (negshare -> lw $a1,0x100($t0)), but it is dead on cost: sharing costs ~7 points in the delta/GTE region even when it changes no register at all (zshare 16, accshare 16); negshare totals 16 with neg in $v1 and z in $a0; tshare 28; tshare1 19.

- [s3] xtop (x declared at function scope, loaded after the block's declaration list) is BYTE-IDENTICAL to the base (score 9, md5 1fc26fe12849) -- a third confirmation that declaration position alone is inert here. C89 trap that cost two builds: a statement may not precede declarations in a block, so the load must follow the declaration list (an assignment spliced among the declarations mis-builds to a 31-insn stub scoring 85-94).

- [s3] src/code6cac_b.c was restored to HEAD at end of session (git checkout --), so main's recorded floor stays 18; the banked score-9 candidate in memory/grind/func_8002EA24/candidate.c is unchanged and re-verified this session (score 9, 102 insns, md5 1fc26fe12849).

## Session 5 (permuter) -- floor 9 -> 2

### Workspace / tooling (reusable)
`tmp/grind/func_8002EA24/s4/mkws.sh <ws-dir> <body.c>` builds a decomp-permuter
workspace for this function, modelled on `tools/mar_perm_workspace.sh`: a FULL-TU
compile (so the codegen context is the real one) through the exact per-file
pipeline for `code6cac_b` (cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 |
prologue_fix | maspsx --expand-lb ... | fix_lwl | `.align 3`->`.align 2` |
multu_pad), after which the single function's region is extracted from the
assembly and assembled alone -- so func_8002EA24 sits at offset 0 in BOTH base.o
and target.o and the permuter score is the honest per-function diff with no
branch-address noise.  regfix / asmfix are deliberately NOT in the pipeline, so
the permuter measures the same honest form the cheat-invisible sandbox does.

Three gotchas cost time and are recorded so the next session does not repeat them:
1. `asm/funcs/func_8002EA24.s` uses the `mvmva` pseudo-op, so `target.s` needs
   `.include "gte_macros.inc"` after the permuter prelude.
2. maspsx emits `.ent` / `.end` / `.size` at COLUMN 0 (no leading tab), unlike the
   mar chassis's assumption; the extraction awk must anchor on
   `^\.globl[ \t]+func_8002EA24$` .. `^\.size.*func_8002EA24`.
3. `mktemp /tmp/...` files are not reliably visible to the next command in this
   WSL install (the systemd user session fails to start); derive the pipeline's
   temp path from the compile's own `$OUT` instead.

**`tmp/grind/func_8002EA24/s2/swap.py` HAS A BUG** -- it ends the replaced region
at the `/* kengo:LOW  |  su_menu_single` banner, which sits AFTER the FOLLOWING
function, so every session-2/3 splice silently deleted `DispSchoolBG` from the TU
as well.  `s4/swap.py` is the fixed version (region ends at `void DispSchoolBG(`).
The session-3 floor of 9 was re-measured this session with the fixed splice and is
unchanged, so no prior measurement is invalidated -- but do not reuse the s2 script.

### Campaign telemetry (fresh-seed discipline)
| workspace | base body | permuter base | first find | outcome |
|---|---|---|---|---|
| tmp/perm_ea24  | session-3 candidate (sandbox 9) | 440 | 45 s -> 40 | `z = 0; return z;` (H6) |
| tmp/perm_ea24b | + L1 (sandbox 6) | 40 | 30 s -> 15 | `(max_y = x)` single local (H5/x) |
| tmp/perm_ea24c | + L2 (sandbox 3) | 15 | 150 s -> 10 | `a0_var = max_y < neg_threshold` (H5/neg) |
| tmp/perm_ea24d | + L3 (sandbox 2) | 10 | none in 29,050 iters | STOPPED, basin exhausted |

Every campaign was harvested with `--stop` in-session; the harvest JSONs are
`tmp/grind/func_8002EA24/s4/harvest_r{1,2,3,4}.json`.  The three productive
campaigns each returned their find in under three minutes, which is the strongest
available evidence that the 29k-iteration silence on the last one is a real basin
boundary rather than an under-run.

Permuter finds are PROPOSALS: of the ~15 outputs produced across the four
campaigns, only three survived vetting.  The rest were the usual coercion shapes
(`volatile unsigned short new_var2`, `(long long)` width casts, `inline_fn`
extraction, `if (1)` wrappers, unused `new_var` declarations) and were discarded
without measurement.

### The residual, exactly
```
ours     slt a0,a1,t1 ; bnez a0,<reject>
target   slt v0,a1,t1 ; bnez v0,<reject>
```
Everything else matches: 104 instructions in both, same opcodes throughout, the
whole GTE region byte-identical, `x` in $a1, `neg_threshold` in $t1, and `a0_var`
in $a0 in BOTH builds.  Target therefore ALSO has a value occupying $a0 across the
range-test chain -- but that value is not the boolean, and its boolean is a plain
$v0 temp.  Identifying what target keeps in $a0 there is the entire remaining
problem; it is stated as H5' in hypotheses.md.

### Load-bearing check on the two annotated exceptions
Both were measured by removing them from the final body:
- score-2 body with L1 reverted to `return 0;` -> **5**
- score-3 body with L1 reverted to `return 0;` -> **6**
- score-2 body with L3 (the staged boolean) removed -> **3**
Each is worth exactly what the candidate header claims; neither is decoration.

### Which staging variable, measured
Same statement, six different locals, on the score-3 base:
`a0_var` **2**, `y_low` 3, `z` 4, `y` 4, `sp_var` 6, `min_y` 10.  This is the
cleanest direct confirmation to date of session 4's corrected `find_reg` model:
liveness across the chain is not what matters -- being the $a0-PREFERRING allocno
is.  (Session 4 had already shown the negative half of this with `vinlive` and
`minmaxearly`, which add live pseudos and change no compare-chain register.)

- [s4] Honest floor went 9 -> 6 -> 3 -> 2 this session, all four values measured with `sandbox func_8002EA24 --disable all` and the edits in place in src/code6cac_b.c.

- [s4] The build now emits 104 instructions against target's 104, with every opcode matching. The entire residual is one register pair: ours `slt a0,a1,t1 ; bnez a0,<reject>` vs target `slt v0,a1,t1 ; bnez v0,<reject>`.

- [s4] Both `x` (now $a1) and `neg_threshold` (now $t1) sit in target's registers, and `a0_var` is in $a0 in BOTH builds -- so target also has something occupying $a0 across the range-test chain; it just is not the comparison boolean. Identifying that value is the whole remaining problem (H5' in hypotheses.md).

- [s4] Each of the two annotated exceptions was measured load-bearing by removal: the score-2 body without the tail staging scores 5; the score-3 body without it scores 6; the score-2 body without the staged boolean scores 3.

- [s4] Staging-variable sweep on the score-3 base (identical statement, six locals): a0_var 2, y_low 3, z 4, y 4, sp_var 6, min_y 10 -- the cleanest confirmation yet of session 4's corrected find_reg model (only the $a0-PREFERRING allocno can deny $a0; mere liveness across the chain does nothing).

- [s4] Campaign telemetry, all four harvested with --stop in-session: perm_ea24 (base 440, find at 45 s), perm_ea24b (base 40, find at 30 s), perm_ea24c (base 15, find at 150 s), perm_ea24d (base 10, 29,050 iterations, no find). No campaign is left running; `permuter_campaign.py status` shows every pid dead.

- [s4] Permuter output is proposals, not answers: of ~15 outputs across the four campaigns only three survived vetting; the rest were coercion shapes (`volatile unsigned short` holders, `(long long)` width casts, `inline_fn` extraction, `if (1)` wrappers, unused `new_var` declarations) and were discarded without measurement.

- [s4] TOOLING BUG FOUND AND FIXED: tmp/grind/func_8002EA24/s2/swap.py ended the replaced region at the `/* kengo:LOW | su_menu_single` banner, which sits AFTER the FOLLOWING function -- so every session-2/3 splice silently deleted DispSchoolBG from the TU. s4/swap.py ends at `void DispSchoolBG(`. The session-3 floor of 9 was re-measured with the fixed splice and is unchanged, so no prior measurement is invalidated.

- [s4] Reusable chassis: tmp/grind/func_8002EA24/s4/mkws.sh builds a permuter workspace for this function (full-TU compile through code6cac_b's exact pipeline, then single-function extraction so the function is at offset 0 in base.o and target.o, regfix/asmfix deliberately excluded so the permuter score tracks the honest sandbox score). Three gotchas recorded in evidence.md: target.s needs `.include "gte_macros.inc"` for the `mvmva` pseudo-op; maspsx emits .ent/.end/.size at column 0; /tmp mktemp files are not reliably visible between commands in this WSL install.

- [s4] ACCEPTANCE IS NOT SELF-APPROVED: the score-2 body carries TWO /* FAKE */-annotated constructs, both cited to [[staged-value-reused-variable]] (SANCTIONED 2026-07-03) -- a real value, read by the very next expression, staged through an existing local that is provably dead at the staging point, with zero dead stores. Lever-exhaustion receipts are the six pure-C tail shapes from session 2 and the ~20 structural shapes from sessions 2-4. If either is refused, memory/grind/func_8002EA24/candidate_alt_score3_no_fake.c is the score-3 fallback and the floor without both is 6.

- [s4] No AgentTool / layer-1 cheat-reviewer was invoked this session: the session brief carries a standing instruction not to call the Agent tool unless the user requests it. The two constructs are therefore submitted to the driver's default-FAIL Judge and a fresh layer-2 cheat-reviewer with the full vetting argument written into the candidate header.

## Session 6 (permuter; driver session 5, scratch tmp/grind/func_8002EA24/s5)

### Floor: 2 (unchanged).  What this session bought: the residual register is shown to be INSTRUCTION-BUDGET-LOCKED, and three independent searches now converge on the banked score-2 form.

The session-5 candidate was re-verified first: `sandbox func_8002EA24 --disable all`
= **2** with the banked `memory/grind/func_8002EA24/candidate.c` spliced into
`src/code6cac_b.c` (107 dis lines; the permuter workspace's own objdump comparison
prints `base insns: 107  target: 107`, so the counts still match).

### The decisive argument (new, and it re-frames H5')
Our build and target are instruction-for-instruction identical except for ONE
register field:

```
ours     lw a1,0x100(t0) ; negu t1,a2 ; slt a0,a1,t1 ; bnez a0,<reject>
target   lw a1,0x100(t0) ; negu t1,a2 ; slt v0,a1,t1 ; bnez v0,<reject>
```

Target's range-test chain contains NO instruction that writes `$a0`, and there is
no spare instruction anywhere in the function (104 = 104).  Under the corrected
session-4 allocator model the ONLY way `$a0` is denied to `neg_threshold` is an
`$a0`-preferring allocno (`a0_var` is the function's only one) that CONFLICTS with
it, i.e. a value genuinely live across the chain.  Materialising any such value
costs at least one instruction -- UNLESS the value is a computation the chain
performs anyway, and the only such computation is the first test's boolean, which
is exactly what the banked form stages.  Staging it costs the boolean its register.
So within GCC 2.7.2's allocator model there is no zero-instruction pure-C occupant
of `$a0` across this chain, and the last two points are a budget lock rather than a
search problem.  Every occupant / hoist variant measured this session emits 108
instructions where the base emits 107, which is the direct confirmation.

### Measured this session (all `sandbox func_8002EA24 --disable all`; base = 2)
| variant | score | what it does |
|---|---|---|
| `plain1` (control) | 3 | no boolean staging at all (re-confirms session 5's L3 is worth 1) |
| `stage2` | 5 | stage the SECOND test's boolean (`threshold < max_y`) through a0_var |
| `stage3` | 7 | stage the THIRD test's boolean (`z < neg_threshold`) through a0_var |
| `stage4` | 5 | stage the FOURTH test's boolean through a0_var |
| `rsqcarry` | 22 | a0_var carries `r_sq` across the chain, tail rewritten to `a0_var -= sum` |
| `negtop` | 5 | `neg_threshold` at function scope, assigned as the first statement |
| `negmid` | 5 | ... assigned just before the GTE input block |
| `neggte` | 4 | ... assigned just before the GTE output block |
| `negtop_stage` | 5 | function-scope neg_threshold PLUS the score-2 boolean staging |
| `sharedend` | 5 | all five rejects become `ret = 0; goto end;` (target's own branch shape) |

Four things are settled by that table:
- **Staging the boolean is a FIRST-TEST-only lever.**  Moving the staged statement
  to any later test is strictly worse (5 / 7 / 5 vs 2): the register win is lost
  and the staging cost is paid anyway.
- **Hoisting `-threshold` is dead on instruction count, not on registers.**  All
  three placements emit 108 instructions vs the base's 107: with the `negu` hoisted
  out of the `lw $a1,0x100($t0)` load-delay slot, maspsx fills that slot with a
  `nop` and the `negu` costs its own slot elsewhere.  Target fills the delay slot
  WITH the negu, so the computation must stay exactly where the base puts it.
  (A first `negtop` attempt scored 1 but was INVALID: the recipe left the
  inner-block `s32 neg_threshold;` declaration in place, shadowing the hoisted one,
  so the chain read an uninitialised pseudo with no set at all.  The accident is
  informative -- a pseudo whose live range reaches function entry conflicts with
  the incoming `$a0` argument register and lands in `$t1` with the boolean in
  `$v0`, i.e. target's exact assignment -- but every LEGAL way to give
  `neg_threshold` that live range costs the negu's delay-slot position.)
- **`a0_var` carrying a real non-boolean live value is very expensive.**
  `rsqcarry` is the H5' frontier's own next-probe (r_sq staged through a0_var: a
  real, live-out, non-boolean value that does not hoist the multiplies) and it
  scores 22 -- it needs a `move a0,a3` before the chain AND it inverts the operands
  of both `slt $v0,$a3,$a0` and `subu $a0,$a3,$a0` in the tail, which currently
  match target exactly.
- **The shared-end-label reject shape is not free on this body.**  It scores 5
  because rewriting the LAST reject as `ret = 0; goto end;` destroys session 5's L1
  construct (the two-statement arm that defeats jump.c's store-flag if-conversion),
  so the tail diamond folds again.

### Campaign telemetry (fresh-seed discipline; all harvested with --stop in-session)
| workspace | chassis | permuter base | iterations | finds |
|---|---|---|---|---|
| tmp/perm_ea24e | score-2 body + PERM_GENERAL 5-way over the first range test + PERM_RANDOMIZE over the rest of the chain | 10 | 15,016 | none |
| tmp/perm_ea24f | score-2 body + PERM_LINESWAP hoisting the `y` load into the chain + PERM_GENERAL over the sum-of-squares association | 335 | 4,688 | one at 235; chassis base degraded by the hoist, stopped and reseeded |
| tmp/perm_ea24g | `plain1` (score 3, NO staging) + PERM_GENERAL over the `-threshold` spelling + PERM_RANDOMIZE over the chain | ~15 | 14,556 | output-10-1 at 135 s, then 15 / 15 |

`tmp/perm_ea24g/output-10-1` is the decisive one: starting from a chassis that does
NOT contain the staging construct, and whose directed axis was the SPELLING of
`-threshold`, the search reconstructed **exactly** the banked score-2 body --
`a0_var = max_y < neg_threshold; if (a0_var || threshold < max_y) return 0;` with
`neg_threshold = 0 - threshold` (an equivalent spelling of the same negation).
Permuter score 10 = the score-2 chassis's own base score, and nothing below it
appeared in 14.5k further iterations.  Together with session 5's 29k-iteration
silence from the score-2 chassis and this session's 15k directed iterations from
it, three independent searches now converge on the banked form and none beats it.

### Tooling (reusable)
- `tmp/grind/func_8002EA24/s5/run.sh` -- splice + `sandbox --disable all` + insn
  count + disassembly md5, using the FIXED `s4/swap.py`.
- `tmp/grind/func_8002EA24/s5/gen.py` -- literal-substitution variant generator on
  top of the score-2 body (one recipe per variant).
- `tmp/grind/func_8002EA24/s5/annot.py` / `annot2.py` -- write the PERM_*-annotated
  chassis bodies.
- `tmp/grind/func_8002EA24/s5/mkws_annot.sh` -- **the missing piece for directed
  permutation on this function**: `s4/mkws.sh` cannot build a workspace from an
  annotated body because its pipeline runs the body through cc1, which cannot parse
  `PERM_*`.  This script builds the workspace from the CLEAN body, then re-runs only
  the cpp step on the ANNOTATED body and overwrites `base.c` (PERM macros survive
  cpp because they are undefined function-like macros).
- ENVIRONMENT: `tools/wsl.sh` fails from the Bash tool in this install
  (`wsl: command not found`); `wsl -e bash -lc "..."` / `wsl -e bash <script>` from
  PowerShell works.  The `Failed to start the systemd user session` line printed by
  every WSL call is cosmetic.

- [s5] Re-verified the banked session-5 candidate at the start of this session: sandbox --disable all = 2 with it spliced into src/code6cac_b.c.

- [s5] THE INSTRUCTION-BUDGET ARGUMENT for H5': our build equals target instruction-for-instruction except the register field of the `slt`/`bnez` pair on the first range test, and target's chain writes $a0 nowhere while the function has no spare instruction (104 = 104). Under the corrected session-4 allocator model $a0 can only be denied to neg_threshold by an $a0-preferring allocno (a0_var is the only one) that CONFLICTS with it, i.e. by a value genuinely live across the chain; materialising any such value costs at least one instruction unless it is a computation the chain performs anyway -- and the only one is the first test's boolean, which is exactly the banked construct. There is therefore no zero-instruction pure-C occupant of $a0 across this chain.

- [s5] KILLED: staging the range-test boolean through a0_var at any test OTHER than the first. stage2 (second test) 5, stage3 (third) 7, stage4 (fourth) 5, against the base 2. The lever is first-test-specific.

- [s5] KILLED: hoisting `-threshold` to function scope on the score-2 base. negtop (first statement) 5, negmid (before the GTE input block) 5, neggte (before the GTE output block) 4, negtop_stage (hoist + keep the staged boolean) 5 -- and every one emits 108 instructions where the base emits 107, because the negu leaves the `lw $a1,0x100($t0)` load-delay slot (maspsx then fills it with a nop) and costs its own slot elsewhere. Target fills that delay slot with the negu, so the computation must stay where the base puts it.

- [s5] A first `negtop` attempt scored 1 but was INVALID -- the recipe left the inner-block `s32 neg_threshold;` declaration shadowing the hoisted one, so the chain read an uninitialised pseudo with no set anywhere. Informative accident: a pseudo whose live range reaches function entry conflicts with the incoming `$a0` argument register and lands in $t1 with the boolean in $v0 = target's exact assignment. Every LEGAL way to give neg_threshold that live range costs the negu's delay-slot position.

- [s5] KILLED: `rsqcarry` -- a0_var carries r_sq across the chain (the H5' frontier's own next-probe: a real, live-out, non-boolean value that does not hoist the multiplies). Score 22: it needs a `move a0,a3` before the chain and it inverts the operands of both `slt $v0,$a3,$a0` and `subu $a0,$a3,$a0` in the tail, which currently match target exactly.

- [s5] KILLED: rewriting all five rejects as `ret = 0; goto end;` (target's own shared-branch shape) on the score-2 base = 5, because it destroys session 5's L1 two-statement arm and jump.c's store-flag if-conversion folds the tail diamond again.

- [s5] Three directed permuter campaigns, all harvested with --stop in-session: tmp/perm_ea24e (score-2 body, PERM_GENERAL 5-way over the first range test + PERM_RANDOMIZE over the rest of the chain, permuter base 10) 15,016 iterations and ZERO finds; tmp/perm_ea24f (PERM_LINESWAP hoisting the y load, base 335 -- chassis degraded by the hoist) 4,688 iterations, one useless find at 235, stopped and reseeded; tmp/perm_ea24g (the plain1 score-3 chassis, PERM_GENERAL over the `-threshold` spelling + PERM_RANDOMIZE, base ~15) 14,556 iterations, best find 10.

- [s5] CONVERGENCE EVIDENCE: tmp/perm_ea24g/output-10-1 -- from a chassis that does NOT contain the staging construct, and whose directed axis was the spelling of `-threshold`, the permuter independently reconstructed the banked score-2 body exactly (`a0_var = max_y < neg_threshold; if (a0_var || threshold < max_y) return 0;`, with `neg_threshold = 0 - threshold`). Permuter score 10 = the score-2 chassis's own base score; nothing below it in 14.5k further iterations. Three independent searches (session 5's 29k random from the score-2 chassis, this session's 15k directed from it, and 14.5k from the score-3 chassis) now converge on the same form.

- [s5] TOOLING: s4/mkws.sh cannot build a permuter workspace from a PERM_*-annotated body (its pipeline runs the body through cc1, which cannot parse the macros). tmp/grind/func_8002EA24/s5/mkws_annot.sh is the fix: build the workspace from the CLEAN body, then re-run only the cpp step on the ANNOTATED body and overwrite base.c -- PERM macros survive cpp because they are undefined function-like macros.

- [s5] ENVIRONMENT: tools/wsl.sh fails from the Bash tool in this install (`wsl: command not found`); use `wsl -e bash -lc "..."` or `wsl -e bash <script>` from PowerShell. The `Failed to start the systemd user session` line printed by every WSL call is cosmetic.

- [s5] src/code6cac_b.c was restored to HEAD at the end of the session, so main's recorded floor stays 18 and the tree is clean apart from the memory/grind ledger, the four new rejected/ forms and tmp/ scratch.

- [s5] Re-verified the banked session-5 candidate at the start of this session: sandbox func_8002EA24 --disable all = 2 with it spliced into src/code6cac_b.c (107 dis lines; the permuter workspace's own objdump comparison prints 'base insns: 107  target: 107').

- [s5] THE INSTRUCTION-BUDGET ARGUMENT (new framing of H5'): our build equals target instruction-for-instruction except the register field of the slt/bnez pair on the first range test, and target's chain writes $a0 NOWHERE while the function has no spare instruction. Under the corrected session-4 allocator model $a0 can only be denied to neg_threshold by an $a0-preferring allocno (a0_var is the only one) that CONFLICTS with it, i.e. by a value genuinely live across the chain; materialising any such value costs at least one instruction unless it is a computation the chain performs anyway -- and the only such computation is the first test's boolean, which is exactly the banked construct. There is therefore no zero-instruction pure-C occupant of $a0 across this chain.

- [s5] Measured this session on the score-2 base: plain1 (control, no staging) 3; stage2 5; stage3 7; stage4 5; rsqcarry 22; negtop 5; negmid 5; neggte 4; negtop_stage 5; sharedend 5. Every hoist/occupant variant emits 108 instructions where the base emits 107.

- [s5] An invalid-but-informative build (an inner-block `s32 neg_threshold;` left shadowing the hoisted declaration, so the chain read an uninitialised pseudo) scored 1 and emitted target's exact assignment -- slt $v0,$a1,$t1 with neg_threshold in $t1 and no negu at all. A pseudo whose live range reaches function entry conflicts with the incoming $a0 argument register; that is the best available explanation of what the original's allocator saw, and every legal spelling of it costs the negu's delay-slot position.

- [s5] CONVERGENCE: tmp/perm_ea24g/output-10-1 -- from a chassis that does NOT contain the staging construct (plain1, sandbox 3) and whose directed axis was the SPELLING of -threshold, the permuter independently reconstructed the banked score-2 body exactly, at permuter score 10 = the score-2 chassis's own base score, and found nothing below it in 14.5k further iterations. With session 5's 29k random iterations from the score-2 chassis and this session's 15k directed ones, three independent searches now converge on the same form.

- [s5] Campaign telemetry, all harvested with --stop in-session and all pids confirmed dead: perm_ea24e 15,016 iterations / 0 finds; perm_ea24f 4,688 iterations / 1 useless find at 235 (chassis base degraded to 335 by the y-load lineswap, reseeded); perm_ea24g 14,556 iterations / best find 10.

- [s5] TOOLING: s4/mkws.sh cannot build a permuter workspace from a PERM_*-annotated body because its pipeline runs the body through cc1, which cannot parse the macros. tmp/grind/func_8002EA24/s5/mkws_annot.sh is the fix -- build the workspace from the CLEAN body, then re-run only the cpp step on the ANNOTATED body and overwrite base.c (PERM macros survive cpp because they are undefined function-like macros).

- [s5] ENVIRONMENT: tools/wsl.sh fails from the Bash tool in this install ('wsl: command not found'); use `wsl -e bash -lc "..."` or `wsl -e bash <script>` from PowerShell. The 'Failed to start the systemd user session' line printed by every WSL call is cosmetic.

- [s5] src/code6cac_b.c was restored to HEAD at the end of the session (git checkout --), so main's recorded floor stays 18 and the tree is clean apart from the memory/grind ledger updates and the four new rejected/ forms.

## Session 7 (forensics; driver session 6) -- the divergence localised to ONE bit

- [s6] THE DIVERGENCE, NAMED EXACTLY. Pass: `global_alloc` in `tools/gcc-2.7.2/global.c`. Decision: the pass-0 hard-register exclusion set computed inside `find_reg` (global.c:1012-1044) for the `neg_threshold` allocno (pseudo 103 in this TU). Measured with the read-only `BB2_FINDREG_DEBUG` hook in `tmp/gccdbg/cc1` via `tmp/grind/func_8002EA24/s3/findreg.sh 103`, on two bodies:
    * banked score-2 body:  conflicts {2,3,4,5,8,29}  someone_prefers {6,7}  own_copy_prefs {}  own_full_prefs {}  ->  pass0_used = {0,1,2,3,4,5,6,7,8,16..23,26..31}  ->  first free = 9 = $t1 = TARGET.
    * `plain1` control (score 3, no staged boolean): conflicts {2,3,5,8,29}  someone_prefers {6,7}  own prefs {}  ->  pass0_used = {0,1,2,3,5,6,7,8,16..23,26..31}  ->  first free = 4 = $a0 = WRONG.
  The two exclusion sets differ in exactly ONE bit: hard register 4 ($a0). Everything else about the two allocations is identical. The whole remaining sandbox distance of 2 is that bit plus its knock-on: buying the bit costs us $a0 as the staged boolean's home, so we emit `slt $a0,$a1,$t1 / bnez $a0` where target has `slt $v0,$a1,$t1 / bnez $v0`.

- [s6] THE ALLOCNO TABLE (from the `-dg` dumps, `tmp/grind/func_8002EA24/s6/{base,plain1}/fn.greg`). Priority order printed by global_alloc: `101 96 97 100 109 108 72 102 117 103 74 99 75`. Identifications: 72=obj($t0), 73=pos($a1), 74=threshold($a2), 75=r_sq($a3), 96=z($v1), 97=a0_var($a0), 99=min_y($a2), 100=max_y/x($a1), 101=y_low($v0), 102=y($v1), 103=neg_threshold, 108/109=the two mflo results. Printed hard-reg preferences exist only for 74->6, 75->7, 97->4, 102->4 (the dump prints `hard_reg_preferences`, global.c:1741). In the score-2 body 97's conflict list gains 103 (and 74, 96); in plain1, 97 and 103 are disjoint and BOTH are assigned $a0.

- [s6] IN `plain1` THE BOOLEAN IS A LOCAL-ALLOC QUANTITY, NOT A GLOBAL ALLOCNO. The first range test's boolean is pseudo 104; it appears in `;; Register dispositions` as `104 in 2` but NOT in the `;; 13 regs to allocate` list, i.e. `local_alloc` (local-alloc.c) gave it $v0 before global_alloc ran. That is exactly target's shape: target's booleans are $v0 local-alloc quantities and `neg_threshold` is a global allocno. So the original's C differed from `plain1` ONLY in whatever set bit 4 of 103's exclusion set -- it did NOT differ in how the boolean is handled. This answers the session-6 frontier's probe 2 (the `.lreg` quantity-merging question) without a separate experiment: local-alloc is not where the two builds diverge.

- [s6] COMPLETE ENUMERATION OF THE FOUR WAYS GCC 2.7.2 CAN SET THAT BIT, each with its measured cost. `find_reg` excludes a hard register for allocno A iff (1) it is in `hard_reg_conflicts[A]`; (2) it is already assigned to an allocno conflicting with A; (3) it is in `regs_someone_prefers[A]`, built by `prune_preferences` (global.c:851-899) from the FULL preferences of LOWER-priority allocnos that CONFLICT with A; or (4) A's own copy/full preference override (global.c:1057-1080) moves it elsewhere. Applied to bit 4 / allocno 103:
    (1) hard-reg conflict with $a0 -- $a0 is live only from function entry to the prologue copy `addu $t0,$a0,$zero`, so 103 would have to be live at function entry. MEASURED DEAD (s5: negtop 5, negmid 5, neggte 4, negtop_stage 5, all emitting 108 insns against the base's 107, because the negu then leaves the `lw $a1,0x100($t0)` load-delay slot that target fills with it).
    (2) conflict with an allocno assigned $a0 -- only 97 (a0_var) is ever assigned $a0, so 103 must overlap a0_var's live range. MEASURED: free ONLY when the value a0_var carries is a computation the chain already performs -- i.e. the first test's boolean, which is the banked score-2 form and costs the boolean its own register. Every other carrier costs >=1 instruction (accearly 19 / accmid 18 / accpre 15 / accsplit 14 / xzptr 11 / accshare 16 / rsqcarry 22 / yearly 11).
    (3) `regs_someone_prefers[103]` = {6,7} in BOTH builds, contributed by 74 (threshold, prefers $a2) and 75 (r_sq, prefers $a3) -- the only two allocnos that are both LOWER priority than 103 and conflicting with it. The two $a0-preferring allocnos, 97 and 102, are both HIGHER priority than 103, so their preferences can never enter 103's exclusion set; and 72 (obj)'s $a0 copy-preference is stripped by prune_preferences because obj conflicts with $a0 itself (s4). Route (3) is CLOSED BY THE PRIORITY ORDER, not by the conflict graph -- which is a stronger and more durable kill than a score measurement.
    (4) 103's `own_copy_prefs` and `own_full_prefs` are EMPTY in both builds (measured), and seeding a $t1 preference would need a reg-reg copy between neg_threshold and a $t1-resident pseudo -- $t1 has no fixed role on MIPS, so no copy can seed it.

- [s6] KILLED: the session-6 frontier's probe #1 -- putting `a0_var`'s FIRST set inside a reject arm, on the theory that the conflict could come from conflict-graph construction rather than a real dynamic overlap. `rejstage` (`if (max_y < neg_threshold || threshold < max_y) { a0_var = 0; return a0_var; }` on the plain1 base) = score 3, and the emitted chain still reads `negu $a0,$a2`: a0_var still gets $a0 and neg_threshold does not move. The reason is structural, not incidental -- every reject arm RETURNS, so a value set there is never live at any join, and along the arm's own path neg_threshold is already dead. GCC 2.7.2's `global_conflicts` walks flow's path-accurate `basic_block_live_at_start`, so there is no over-approximation to exploit in a diamond whose arms both exit. Banked at rejected/a0var-first-set-in-reject-arm-inert-score3.c.

- [s6] KILLED: `yearly` -- hoisting the `y` load (`y = *(s32 *)(obj + 0x108);`) ahead of the whole range-test chain on the plain1 base. This was the one structurally NEW candidate for route (2): allocno 102 (`y`) is the function's OTHER $a0-preferring allocno, so making it live across the chain might have supplied the exclusion without touching a0_var. Score 11, 106 insns, and the chain still reads `negu $a0,$a2` -- 102 does not take $a0 (it conflicts with 97, which is allocated first and wins $a0), and the hoist costs the tail's `lw $v1,0x108($t0)` position. Banked at rejected/y-load-hoisted-across-chain-score11.c.

- [s6] CONSEQUENCE FOR H5''. Session 6's frontier hypothesis -- that target denies $a0 to neg_threshold via a preference/conflict-graph effect costing no instruction -- is KILLED in all three of its stated sub-mechanisms: (i) conflict-graph over-approximation from a reject arm (measured inert, and structurally impossible because the arms exit); (ii) `prune_preferences` propagation (closed by the priority order: both $a0-preferring allocnos outrank 103); (iii) an $a0 preference on some other pseudo that already conflicts with 103 (no such pseudo exists -- the only allocnos both lower-priority-than and conflicting-with 103 are the two parameters 74 and 75, whose preferences are their own argument registers). What survives is exactly one generator, route (2) with a chain-computed carrier, and that is the banked score-2 body.

- [s6] TOOLING: `tmp/grind/func_8002EA24/s6/dump.sh <body> <tag>` splices a body, preprocesses it and runs the shipped cc1 with `-dl -dg -df -dc`, then slices the func_8002EA24 section of each dump into `tmp/grind/func_8002EA24/s6/<tag>/fn.{lreg,greg,flow,combine}`. It must NOT use `set -e`: cc1 exits non-zero on this TU (pre-existing conflicting-declaration warnings in code6cac_b.c) even though it writes correct output, which silently skipped the slicing step on the first run.

- [s6] ENVIRONMENT (correcting the s5 note): `bash tools/wsl.sh '<cmd>'` DOES work from the Bash tool in this install. The `wsl: Failed to start the systemd user session for 'trenton'` line printed on every call is cosmetic.

- [s6] src/code6cac_b.c carries the banked score-2 candidate body at the end of this session (re-verified this session: `sandbox func_8002EA24 --disable all` = 2, 107 dis lines, disassembly md5 b74fd7ba27c3). No pipeline file, rule file or engine surface was touched.

- [s6] PASS AND DECISION NAMED: global_alloc in tools/gcc-2.7.2/global.c; the pass-0 hard-register exclusion set computed inside find_reg (:1012-1044) for the neg_threshold allocno (pseudo 103). Everything else in the function -- 104 instructions, every opcode, every other register -- already matches target.

- [s6] MEASURED EXCLUSION SETS (BB2_FINDREG_DEBUG via tmp/grind/func_8002EA24/s3/findreg.sh 103). Banked score-2 body: conflicts {2,3,4,5,8,29}, someone_prefers {6,7}, own_copy_prefs {}, own_full_prefs {} -> pass0_used {0,1,2,3,4,5,6,7,8,16..23,26..31} -> first free 9 = $t1 (target). plain1 control (score 3, no staging): conflicts {2,3,5,8,29}, someone_prefers {6,7}, own prefs {} -> first free 4 = $a0 (wrong). Exactly one bit apart: hard register 4.

- [s6] ALLOCNO TABLE (tmp/grind/func_8002EA24/s6/{base,plain1}/fn.greg). Priority order: 101 96 97 100 109 108 72 102 117 103 74 99 75. Identities: 72=obj($t0), 73=pos($a1), 74=threshold($a2), 75=r_sq($a3), 96=z($v1), 97=a0_var($a0), 99=min_y($a2), 100=max_y/x($a1), 101=y_low($v0), 102=y($v1), 103=neg_threshold, 108/109=the two mflo results. Printed hard-reg preferences: 74->6, 75->7, 97->4, 102->4 only.

- [s6] ROUTE (3) IS CLOSED BY THE PRIORITY ORDER, NOT BY THE CONFLICT GRAPH. regs_someone_prefers[103] is built only from allocnos LOWER priority than 103 that conflict with it; in the printed order those are only 74 (threshold) and 75 (r_sq), whose preferences are their own argument registers $a2/$a3. Both $a0-preferring allocnos (97, 102) outrank 103, so no source shape that leaves the priority order intact can ever route an $a0 preference into 103's exclusion set. This is a structural kill, stronger than a score measurement.

- [s6] IN plain1 THE BOOLEAN IS ALREADY A LOCAL-ALLOC $v0 QUANTITY (pseudo 104: `104 in 2` in Register dispositions, absent from the `13 regs to allocate` list) -- exactly target's shape. So the original's C differed from plain1 ONLY in whatever set bit 4 of allocno 103's exclusion set; it did not differ in how the boolean is handled. Session 6's frontier probe 2 (local-alloc quantity merging) is answered and dead.

- [s6] rejstage (a0_var's first set inside the first reject arm -- session 6's own next-probe #1) = score 3, 107 insns, `negu $a0,$a2` unchanged. Structurally impossible rather than merely unlucky: reject arms RETURN, so a value set there is live at no join and neg_threshold is already dead on that path; global_conflicts uses flow's path-accurate basic_block_live_at_start.

- [s6] yearly (the y load hoisted ahead of the whole chain, targeting allocno 102 -- the function's other $a0-preferring allocno) = score 11, 106 insns, `negu $a0,$a2` unchanged. 102 never wins $a0 because 97 is allocated earlier.

- [s6] The banked score-2 body is re-verified with edits in place in src/code6cac_b.c: sandbox func_8002EA24 --disable all = 2, 107 dis lines, disassembly md5 b74fd7ba27c3 (identical to the session-5 measurement).

- [s6] TOOLING: tmp/grind/func_8002EA24/s6/dump.sh <body> <tag> splices a body, preprocesses it and runs the shipped cc1 with -dl -dg -df -dc, slicing the func_8002EA24 section of each dump into s6/<tag>/fn.{lreg,greg,flow,combine}. It must NOT use `set -e` -- cc1 exits non-zero on this TU (pre-existing conflicting-declaration warnings in code6cac_b.c) even though it writes correct output.

- [s6] ENVIRONMENT (correcting the session-5 note): `bash tools/wsl.sh '<cmd>'` DOES work from the Bash tool in this install; the `wsl: Failed to start the systemd user session` line printed on every call is cosmetic.

- [s6] No pipeline file, rule file, engine surface, queue action or commit was touched. Working tree carries only src/code6cac_b.c (the candidate body), the three memory/grind ledger files, two new rejected/ forms and tmp/ scratch.


## Session 7 (forensics) -- the $a0 exclusion is a CONFLICT requirement, not a priority requirement

- [s7] CORRECTED (session 6's frontier premise): the exclusion of hard register 4 ($a0) from the
  `neg_threshold` allocno (pseudo 103) in the banked score-2 build does NOT come from
  `regs_someone_prefers` / `prune_preferences`, so no allocno-PRIORITY lever can supply it.
  - mechanism: the `.greg` allocation order is `101 96 97 100 109 108 72 102 117 103 74 99 75` in EVERY
    build measured this session (11 distinct bodies). Allocno 97 (a0_var, the function's $a0-preferring
    allocno) is allocated BEFORE 103, so when 103 is allocated 97 already HOLDS hard reg 4 and conflicts
    with it: 4 is excluded as an ordinary assigned-conflict. `someone_prefers[103]` stays {6,7} (from 74 =
    threshold and 75 = r_sq, the only lower-priority conflicting allocnos) in both the score-2 body and the
    score-3 control -- exactly what session 6 measured -- but that set is IRRELEVANT to the outcome here.
    Session 6's frontier ("demote 102 or 97 below 103 and route (3) opens") is therefore a no-op even where
    it succeeds: demoting 97 below 103 merely substitutes a preference-based exclusion for an
    assigned-conflict exclusion, and leaves 97 both holding $a0 and holding the staged boolean.
  - probe: `-dg` dumps (tmp/grind/func_8002EA24/s7/probe.sh writes per-variant fn.greg) for cand (score 2),
    plain (3), ret1 (3), ret12 (3), b2 (5), b2or (5), yrefs (3), a0refs (3), negtail1 (22), negtail2 (5),
    negtail3 (24); the allocation-order line, the 97/102/103 conflict lists and the register dispositions
    were read out of each.
  - result: 103 lands in hard reg 9 ($t1 = target) in exactly the builds whose `103 conflicts:` line
    contains 97, and in hard reg 4 ($a0) in exactly the builds where it does not. Nothing else correlates:
    103's own copy/full preferences are EMPTY in every build and someone_prefers[103] is {6,7} in every
    build.
  - verdict: CONFIRMED

- [s7] KILLED: the session-6 frontier's own probe -- changing a REFERENCE COUNT to move `allocno_compare`
  priority. `yrefs` (the `*(s32 *)(obj + 0x108)` load written twice so CSE collapses it, intended to raise
  allocno 102's reg_n_refs) and `a0refs` (an extra collapsed reference to a0_var) both leave the printed
  allocation order BIT-IDENTICAL to the control, leave every conflict list unchanged, and score 3 = the
  control. The lever never reaches the allocator: cc1's CSE removes the duplicate reference long before
  `global_alloc` counts refs, so the source-level reference count is not the quantity `allocno_compare`
  sees. Banked at rejected/refcount-priority-lever-inert-score3.c.

- [s7] KILLED: staging the return-value 0 of a reject arm through a0_var (`ret1` = one arm, `ret12` = two
  arms) -- an independent re-derivation of session 6's `rejstage`, now measured for two arms as well: score
  3 in both, `103 conflicts:` unchanged (no 97), `negu $a0,$a2` unchanged. Reject arms RETURN, so a value
  set there is live at no join and neg_threshold is already dead along that path; `global_conflicts` walks
  flow's path-accurate live sets, so there is no over-approximation to exploit.

- [s7] The instruction-free conflict generator is NOT unique to the first range test, but every
  alternative is strictly worse. `b2` (the SECOND range test's boolean staged through a0_var, written as
  three statements) and `b2or` (the same staged inside the `||` so the branch structure is preserved) BOTH
  create the 97<->103 conflict and BOTH put neg_threshold in target's $t1 ($t1 = hard reg 9 in the
  dispositions) -- and both score 5 against the banked 2, because the staged boolean then occupies $a0 on
  the second `slt` while the first test's boolean also moves. The residual is invariant under placement:
  any a0_var-staged boolean costs its own slt/bnez pair the $v0 that target uses. Banked at
  rejected/staged-boolean-at-second-test-into-a0var-score5.c.

- [s7] KILLED: making `neg_threshold` itself the carrier of a tail value, so that its live range reaches
  a0_var's and the conflict appears with NO boolean staged anywhere. `negtail1`
  (`neg_threshold = y - a0_var; if (max_y < neg_threshold) ...`) DOES create both the 97<->103 and the
  102<->103 conflicts -- but the extra reference lifts 103's `allocno_compare` priority from order position
  10 to position 4 (priority is floor_log2(n_refs)*n_refs / live_length; going from 3 to 4 references takes
  the numerator 3 -> 8, swamping the longer live range), so 103 is allocated while $a1 is still free, takes
  hard reg 5 and displaces `x` to $t0: score 22. `negtail3` (both tail expressions staged) = 24.
  `negtail2` (only the `y + a0_var` expression staged) never reaches a0_var's range -- no 97 conflict, 103
  back in $a0 -- and scores 5. Banked at rejected/neg-threshold-carries-tail-subtraction-score22.c and
  rejected/neg-threshold-carries-tail-addition-score5.c.

- [s7] TARGET'S OWN ALLOCATION IS NOT REPRODUCIBLE BY AN ASSIGNED-CONFLICT -- the sharpest fact this
  session produced, and where session 8 should start. Reading asm/funcs/func_8002EA24.s across the
  range-test window (8002EAA0 `lw $a1,0x100($t0)` .. 8002EAD4 `slt $v0,$a2,$v1`), the values live there in
  TARGET are exactly six: obj ($t0=8), x ($a1=5), threshold ($a2=6), r_sq ($a3=7), z ($v1=3, born
  mid-window) and the boolean temps ($v0=2). Hard reg 4 ($a0) is written NOWHERE between the prologue copy
  `addu $t0,$a0,$zero` (the function's first instruction) and `addu $a0,$v0,$v1` after the multiplies. Our
  build's `103 conflicts:` set covers exactly the same six registers {2,3,5,6,7,8}, leaving 4 free for
  first-fit -- yet target's negu writes $t1. Target's compile therefore excluded $a0 through
  `regs_someone_prefers`: it had an $a0-preferring allocno that BOTH conflicted with neg_threshold AND
  ranked BELOW it in allocno_compare order. Our two $a0-preferring allocnos are 97 (a0_var) and 102 (y);
  102 is the shape-compatible one (target keeps y in $v1, exactly where our 102 lands), but 102 conflicts
  with 103 only if y is live in the window, and hoisting the y load costs two instructions (session 6's
  `yearly` = 11 / 106 insns). An $a0-preferring allocno conflicting with 103 from BELOW it in the order at
  zero instruction cost is the one unexplored shape -- and it is now a statement-level search problem, not
  an allocator-model problem: the model is fully determined and hand-checkable from the dumps.

- [s7] The allocation order printed by cc1 -dg is `;; 13 regs to allocate: 101 96 97 100 109 108 72 102 117 103 74 99 75` in 9 of the 11 bodies dumped this session (all but negtail1/negtail3, where 103 jumps to position 4); allocno 97 = a0_var (preference 4), 102 = y (preference 4), 103 = neg_threshold (no preferences), 74 = threshold (pref 6), 75 = r_sq (pref 7), 72 = obj (in $t0).

- [s7] Across all 11 dumps, allocno 103 is assigned hard reg 9 ($t1, target's register) iff its `103 conflicts:` list contains 97, and hard reg 4 ($a0) iff it does not. someone_prefers[103] = {6,7} and 103's own copy/full preferences are empty in every build, so no preference route is in play in ANY of them.

- [s7] Because 97 is allocated before 103, the exclusion of $a0 is an assigned-conflict, not a prune_preferences effect: session 6's 'route (3) is closed by priority' is a true statement about a mechanism that is not the operative one, and no priority lever can substitute for the conflict.

- [s7] Reference-count levers are inert at the allocator: yrefs and a0refs leave the allocation order and every conflict list bit-identical to the control (score 3 = control) because CSE collapses the duplicate reference before global_alloc counts refs.

- [s7] Staging a reject arm's return-0 through a0_var creates no conflict for one arm or for two (ret1, ret12 = 3): every reject arm returns, so the value is live at no join and neg_threshold is already dead on that path; global_conflicts uses flow's path-accurate live sets.

- [s7] The second range test's boolean is a SECOND instruction-free conflict generator -- b2 and b2or both put neg_threshold in $t1 -- but both cost 3 points (score 5 vs the banked 2), so L3 keeps its place as the cheapest generator rather than the only one.

- [s7] Making neg_threshold carry a tail value creates the conflict but adds a reference, which lifts 103's priority from order position 10 to position 4 and lands it in $a1: negtail1 = 22, negtail3 = 24, negtail2 (no conflict) = 5.

- [s7] TARGET's range-test window has hard reg 4 unwritten and carries the same six conflicting values our build does, yet allocates neg_threshold to $t1 -- so the original compile's exclusion came from regs_someone_prefers with an $a0-preferring allocno conflicting with 103 from BELOW it in the order. Our 102 (y) already prefers $a0 and already lands in $v1 exactly where target keeps y; it is two positions above 103 and does not conflict with it.

- [s7] The banked score-2 candidate was re-applied to src/code6cac_b.c and re-measured this session: sandbox --disable all = 2 (disassembly md5 b74fd7ba27c3). No body change was made; the session's product is the corrected allocator account plus four killed carrier families.

## Session 8 (rederive) — measured facts

- [s8] The banked score-2 candidate was re-applied to `src/code6cac_b.c` and
  re-measured: `sandbox func_8002EA24 --disable all` = **2**, build 104 insns vs
  target 104. Floor unchanged; the body was restored to exactly this form at the
  end of the session.

- [s8] A fresh m2c decompile is available and reproduces the function cleanly
  apart from the six cop2 instructions (`lwc2`/`mvmva`/`swc2`/`mtc2`), which it
  emits as `M2C_ERROR(/* unknown instruction ... */)`. Command:
  `python3 tools/m2c/m2c.py --target mipsel-gcc-c -f func_8002EA24
  asm/funcs/func_8002EA24.s`. Two structural facts it exposes that our banked
  body does not have: (a) it renders the five early returns as ONE result
  variable `var_v0` with four nested ifs and a single `return var_v0;`; (b) it
  splits our single `a0_var` local into THREE — `temp_a0` (sum of squares),
  `temp_a0_2` (u32 remainder `arg3 - temp_a0`) and `var_a0` (u32 sqrt result).
  Both were built and measured (see hypotheses.md [s8]); neither reaches below 2.

- [s8] **The score-2 plateau does not move with source shape.** Six structurally
  distinct bodies all measure exactly 2 at 104 instructions:
  `v6_neg_inline_no_local` (no `neg_threshold` local at all — `-threshold`
  written inline in both range tests), `v7_neg_init_after_x_load` (local declared
  uninitialised, assigned after the x load), `v10_nested_range_tests_only`
  (m2c's nesting over the four range tests, banked tail kept),
  `v12_single_four_way_if` (all four range tests fused into ONE short-circuit
  `if`, with `z` assigned inside the condition so its load stays in the third
  test's block — the source shape that literally matches target's four branches
  to a single label), `v11_sum_own_local` (sum-of-squares split into its own
  local, remainder+sqrt still in `a0_var`), `v14_split_stage_into_sum` (that
  split with the staged boolean moved into the sum local). All six are banked;
  four are in `rejected/` as inert forms so they are not re-proposed.

- [s8] **The "seventh live value" carrier axis is dead by allocno PRIORITY, not
  by cost.** Probe: read `y` as `vout[2]` off the GTE output pointer the swc2
  block already computes, so the carrier is instruction-free. Scores: 6 without
  the staged boolean, 5 with it (both 104 insns). The `-dg` dump
  (`tmp/grind/func_8002EA24/s8/v4_vout_carries_y_no_L3/fn.greg`) shows:
    * allocation order `;; 14 regs to allocate: 101 96 97 100 109 108 72 102 117
      103 74 99 75 77` — the carrier (allocno 77) is LAST, i.e. allocated AFTER
      103, so it can never be an assigned conflict for 103;
    * there is no `;; 77 preferences:` line — the carrier has no hard-register
      preference, so it contributes nothing to `regs_someone_prefers[103]`
      through `prune_preferences` either;
    * dispositions: `77 in 9` ($t1 — target's register for neg_threshold) and
      `103 in 4` ($a0). The probe inverts the exact bit it targets.
    * it additionally destroys allocno 102's `preferences: 4` line (present in
      the banked body's dump), removing the function's other $a0-preferring
      allocno.
  Since allocno priority is `floor_log2(n_refs)*n_refs / live_length`, any
  carrier long enough to span the range-test window and cheap enough to be
  instruction-free necessarily sorts to the bottom of the order. The axis is
  therefore closed as a family, not merely for `vout`.

- [s8] For the record, the banked body's own dump (same probe, `candidate`) is
  `;; 13 regs to allocate: 101 96 97 100 109 108 72 102 117 103 74 99 75`,
  `97 preferences: 4`, `102 preferences: 4`,
  `103 conflicts: 72 74 75 96 97 100 103 2 29` (97 present — the assigned
  conflict L3 buys), dispositions `96 in 3, 97 in 4, 100 in 5, 102 in 3,
  103 in 9`. This is the session-7 account re-measured and unchanged.

- [s8] **Kengo transplant is not available for this function.** The in-tree
  Kengo corpus consists of symbol-name lists only
  (`.claude/worktrees/agent-a4d27f1fe98d52a20/Kengo/kengo_functions*.txt`,
  `kengo_globals*.txt`) plus the name-application tools
  (`tools/apply_kengo_names.py`, `kengo_match.py`, `kengo_ref.py`). There is no
  Kengo C source in the tree, so the `/* kengo:HIGH | sa_tan2/
  saTan2LinePrimInit | 110i */` annotation above func_8002EA24 is a NAME
  mapping, not a source lead. Do not re-open this avenue.

- [s8] The banked score-2 candidate was re-applied to src/code6cac_b.c and re-measured this session: sandbox --disable all = 2, build 104 insns vs target 104. src/ was restored to exactly that form at the end of the session and re-verified at 2.

- [s8] m2c runs cleanly on this function: `python3 tools/m2c/m2c.py --target mipsel-gcc-c -f func_8002EA24 asm/funcs/func_8002EA24.s` reconstructs everything except the six cop2 instructions. Its two structural departures from our banked body are (a) one result variable with four nested ifs and a single return, and (b) a THREE-way split of our single a0_var local into temp_a0 (sum of squares), temp_a0_2 (u32 remainder), var_a0 (u32 sqrt result). Both measured; neither helps.

- [s8] Six structurally distinct bodies all score exactly 2 at 104 insns: no-neg_threshold-local, init-after-x-load, nested range tests, single four-way short-circuit if with z assigned inside the condition, sum-in-its-own-local, and sum-split-with-the-boolean-staged-into-sum. The plateau is source-shape-invariant.

- [s8] The vout-carries-y probe's .greg dump is the session's core artifact: allocation order `;; 14 regs to allocate: 101 96 97 100 109 108 72 102 117 103 74 99 75 77` (carrier 77 LAST, after 103), no `77 preferences:` line, dispositions `77 in 9` ($t1) and `103 in 4` ($a0). Adding a free live value across the window is not neutral -- it actively steals target's $t1.

- [s8] The banked body's own dump is unchanged from session 7: `;; 13 regs to allocate: 101 96 97 100 109 108 72 102 117 103 74 99 75`, `97 preferences: 4`, `102 preferences: 4`, `103 conflicts: 72 74 75 96 97 100 103 2 29` (97 present -- the assigned conflict L3 buys), dispositions `96 in 3, 97 in 4, 100 in 5, 102 in 3, 103 in 9`.

- [s8] Consequence for the frontier: allocno 97 (a0_var) is now the ONLY allocno that can deny $a0 to 103, because (i) session 7 showed it is the only $a0-preferring allocno above 103 in the order and (ii) session 8 showed newly-introduced carriers always sort BELOW 103. The axis therefore reduces to a single question -- what REAL value, computed before the range chain and read after it, can 97 hold? -- and the enumeration of the function's pre-chain values (three deltas stored to memory and dead; vin/vout dead after the asm blocks; -threshold, which IS allocno 103; the sum of squares, whose hoist is the measured-dead accearly family) currently returns nothing.

- [s8] No new /* FAKE */ construct was introduced this session and no cheat-class construct was attempted. The body in src/ and in memory/grind/func_8002EA24/candidate.c is byte-for-byte the session-4/5 form whose two annotated staged-value exceptions (L1, L3) are still awaiting the Judge / a fresh layer-2 cheat-reviewer.

- [s8] Kengo transplant is unavailable: the in-tree corpus is symbol names only, no C source.

## Session 9 (rederive) — the H5''''' enumeration is COMPLETE, and two decomp.me corpus censuses come back negative

Floor re-measured at the start and the end of the session: **2**, build 104
insns (`sandbox func_8002EA24 --disable all`, banked `candidate.c` spliced into
`src/code6cac_b.c`).  Nothing in this session moved it.  Two distinct lines of
work were run: (A) the mandated completion of the H5''''' pre-chain-value
enumeration, and (B) the decomp.me corpus route that the `rederive` modality
names and that no previous session had used (session 8 killed the Kengo
transplant leg of the same modality; the decomp.me leg was still open).

### A full `.greg` dump of the banked score-2 body (the reference state)

`tmp/grind/func_8002EA24/s9/banked/fn.greg`, produced by
`tmp/grind/func_8002EA24/s9/dump.sh` (cc1 `-dg` on the real preprocessed TU).
This is the first time the WHOLE allocator picture for the banked body has been
written down rather than the three or four lines previous sessions grepped, so
it is recorded here in full because every future probe is read against it:

```
;; 13 regs to allocate: 101 96 97 100 109 108 72 102 117 103 74 99 75
;; 72  conflicts: 72 74 75 96 97 99 100 101 103 108 109 117 2 3 4 5 6 7 12 29 64 66
;; 74  conflicts: 72 74 75 96 97 100 103 2 3 5 7 12 29        preferences: 6
;; 75  conflicts: 72 74 75 96 97 100 103 108 109 2 3 5 12 29 64 66   preferences: 7
;; 96  conflicts: 72 74 75 96 100 103 108 2 29 64 66
;; 97  conflicts: 72 74 75 97 99 100 101 102 103 117 2 3 12 29  preferences: 4
;; 99  conflicts: 72 97 99 100 101 102 2 29
;; 100 conflicts: 72 74 75 96 97 99 100 101 102 103 2 29 64 66
;; 101 conflicts: 72 97 99 100 101 29
;; 102 conflicts: 97 99 100 102 2 29                            preferences: 4
;; 103 conflicts: 72 74 75 96 97 100 103 2 29
;; 108 conflicts: 72 75 96 108 109 29 64 66
;; 109 conflicts: 72 75 108 109 29 64 66
;; 117 conflicts: 72 97 117 2 29
dispositions: 72 in 8   96 in 3   97 in 4   99 in 6   100 in 5   102 in 3   103 in 9
```

Read-off: `97` (a0_var) is the only allocno assigned hard reg 4 ($a0) and it is
allocated third, well before `103` (neg_threshold, position 10), so `103`'s
`conflicts:` list containing `97` is what removes 4 from its free set and lets
first-fit reach 9 ($t1) = target.  `102` also carries `preferences: 4` but does
NOT conflict with `103`, and it is assigned 3 ($v1), so it contributes nothing.
`100` is `x` in 5 ($a1) = target.  Everything in the function is target's
register except the destination of the FIRST range test's `slt`, which is 97
($a0) where target uses an ordinary $v0 temp — the whole residual, two
instructions, `slt $a0,$a1,$t1 / bnez $a0` vs `slt $v0,$a1,$t1 / bnez $v0`.

### (A) The H5''''' enumeration, completed

The frontier question inherited from session 8 was: what REAL value, computed
before the range-test chain and read after it, can allocno 97 hold at zero
instruction cost?  Sessions 3/4/8 had each measured part of the list.  This
session measured the remaining entries.  Harness: `s9/gen.py` generates whole
bodies from ONE template so that the only textual difference between variants
is the range-test region, `s8/apply.py` splices them into `src/code6cac_b.c`,
and the control regenerated through the same template scores exactly 2 at 104
insns — so a template artefact cannot be mistaken for a variant effect.

| variant | value staged into `a0_var` before the chain | score | insns |
|---|---|---|---|
| `v0_control_banked` | the first test's boolean (L3, the banked form) | **2** | 104 |
| `v1_rsq_into_a0var` | `r_sq` (real: it is the radius-test operand and the minuend of the remainder, both read after the chain; sum-of-squares split into its own local, which session 8 measured inert on its own) | 19 | 104 |
| `v2_x_into_a0var` | the rotated X itself (real by construction: loaded before the chain, IS the first two test operands, read after by `x*x`) | 6 | 104 |
| `v3_rsq_into_a0var_L3_on_y` | `r_sq`, with the first test's boolean moved onto the `y` local | 19 | 104 |
| `v4_threshold_into_a0var` | `threshold` (real: the right operand of both upper tests) | 24 | 104 |

Every one of them holds the instruction COUNT at target's 104 — these are pure
register/schedule differences, not missing or extra work — and every one is
worse than the banked 2.

`v2` is the informative failure and its `.greg` is banked at
`tmp/grind/func_8002EA24/s9/v2_x_into_a0var/fn.greg`.  Making `a0_var` carry X
does everything the model asks: 97 rises to FIRST in the allocation order
(`;; 13 regs to allocate: 97 101 96 109 108 72 102 117 103 100 74 99 75`), it
keeps `preferences: 4`, it is assigned 4 ($a0), and `103 conflicts:` contains
97.  But 103 then lands in **5 ($a1)**, not 9 ($t1) — because X is no longer a
separate allocno, nothing occupies $a1 across the chain, so first-fit reaches
$a1 before $t1.  The lever that supplies the conflict is the same lever that
vacates target's register for `x`.  That is the structural reason the whole
"put a real value in 97" family cannot close: the only pre-chain values large
enough in reference count to outrank 103 are the compare operands themselves,
and consuming one of them as the carrier frees the register the OTHER one needs.

With `r_sq` (19), `threshold` (24), `x` (6), the sum of squares (accearly 19,
sessions 3/4), the three delta temps (16–28, session 4), `vin`/`vout` (6/5,
session 8), `y` hoisted (11, session 5) and `-threshold` itself (it IS allocno
103) all measured, **the enumeration of pre-chain values is complete**.  No real
value can occupy allocno 97 across the chain at a cost the function can repay.
The staged boolean (L3) remains the unique instruction-free generator of the
one allocator bit, at 2 points, with the next-cheapest alternative at 5.

### (A2) The dual generator class — carrier dying INSIDE the chain — also closed

Every carrier measured before this session was live PAST the chain.  The dual
had never been probed: a value assigned $a0 that overlaps `103` but dies
mid-chain would supply the conflict WITHOUT competing for the $a1 that `v2`
stole from `x`.  `v4` (`threshold`) is a parameter member of that class and
works mechanically but displaces `threshold` out of $a2 (24).  The only
non-parameter candidate is the GTE output pointer, which the `swc2` block
already computes for free:

| variant | | score | insns |
|---|---|---|---|
| `v5_vout_dies_in_chain` | X and Z read as `vout[0]`/`vout[1]`, `y` still off `obj`, L3 kept | 2 | 104 |
| `v6_vout_dies_in_chain_no_L3` | same body, L3 removed | 3 | 104 |

Those are exactly the banked body's score and exactly the no-L3 control's score,
and `tmp/grind/func_8002EA24/s9/v6_vout_dies_in_chain_no_L3/fn.greg` says why:
the allocno set and the allocation order are IDENTICAL to the banked body's
(`;; 13 regs to allocate: 101 96 97 100 109 108 72 102 117 103 74 99 75`), with
no additional allocno anywhere and `103 conflicts:` again missing `97`.  GCC
re-materialises `obj+0x100` and `obj+0x104` as $t0-relative addresses instead of
keeping the pointer live, so `vout` is not a pseudo by the time `global_alloc`
runs and can conflict with nothing.  The dual direction is therefore closed too,
and the generator matrix — {carrier live past the chain, carrier dying inside
it} × {compare-operand carrier, parameter carrier, introduced carrier} — is now
exhausted in both directions.

### (B) decomp.me corpus census #1 — no precedent for the register bit

`tools/decomp_me_scrape.py`'s local corpus (`tmp/decomp_me_corpus`, 3754
GCC-2.7.2 scratches, of which **1751 are MATCHING**) was searched for the
codegen fact we are trying to reproduce: a `negu` whose destination is a
caller-save `$tN` and which is then consumed by an `slt` range test.

* `negu` into `$t0-$t9` or `$s0-$s7` in a matched scratch: **8 of 1751**.
* Of those 8, **7 are `$s0`/`$s1`** — callee-saved, i.e. the value is live
  across a `jal`.  func_8002EA24 is a leaf; that mechanism is unavailable.
* The single `$tN` case is `8Otmf` / `func_80051BB4`, and it is not a
  precedent: its `$t2` is a whole-function copy of an argument (`var_t2 =
  arg2;`) in a function that then performs five division expansions and a
  seven-argument call, so $a0-$a3 are consumed as computation temps; the
  scratch additionally uses an explicit `register s32 var_t6 asm("t6")` pin.
* Refining the query to "negu into $t/$s AND that register used as an `slt`
  right operand" returns **0 of 1751**.

So the corpus contains no matched pure-C example of GCC 2.7.2 placing a negated
compare operand in a caller-save `$tN` in a leaf function.  This is a NEGATIVE
census — it does not prove impossibility, but it removes the "find a corpus
example and transplant its shape" leg of the rederive modality, and it is the
kind of result that a future endgame-lock evaluation must treat as a FAILED
gate rather than an open question.

### (C) decomp.me corpus census #2 — no pure-C precedent for L1's symptom either

Same corpus, asking the L1 question from an independent direction: does any
matched GCC-2.7.2 scratch keep the UNFOLDED 0/1 diamond (both `addiu $v0,$zero,1`
and a `$v0 = 0` arm, an `slt` into $v0, and **no** `xori $v0,$v0,1`)?

* matched scratches whose target shows the `xori $v0,$v0,1` store-flag fold: 15
* matched scratches whose target keeps the unfolded diamond: **39**
* of those, with no `for`/`while`/`do` anywhere in the source: **12**
* every one inspected (`hX3z3`/`Ntlgo` func_8009C090, plus the loop cases
  `67gdn`, `4bMqu`, `kUZCB`, `7KNEh`) keeps the diamond because the arm has
  REAL other work in it — a loop body, a store, or a call.  `func_8009C090`'s
  fall-through is `*arg2 |= arg3; func_800B0574(a0, arg3); return 1;`.

None of the 39 is the shape func_8002EA24 has: a bare `if (cond) return 0;`
followed by a bare `return 1;` with nothing else on either path.  The corpus
therefore independently reproduces session 2's measured conclusion (six pure-C
tail shapes, three byte-identical to the folded form) from a completely
different direction: with nothing real to put in the arm, GCC 2.7.2 always
folds, and the only spellings that do not fold put something in the arm.  L1
puts a real, immediately-read value there (`{ z = 0; return z; }`) rather than a
dead store, which is the mildest member of that family, but it is a member of it.

Incidental observation, recorded for honesty rather than as authorization: the
corpus does contain matched scratches with self-declared coercions (`VZWgF`
carries a literal `// hack` above a no-op `check238++; check238--;` pair).
decomp.me scratches are not a SOTN-master precedent and this project's endgame
gate does not accept them as one; noted only so a later session does not
"discover" it and mistake it for a citable authority.

### Artifacts

* `tmp/grind/func_8002EA24/s9/dump.sh` — full `.greg` dumper for the current src
* `tmp/grind/func_8002EA24/s9/gen.py` — one-template variant generator
* `tmp/grind/func_8002EA24/s9/banked/fn.greg` — reference allocator state (score 2)
* `tmp/grind/func_8002EA24/s9/v2_x_into_a0var/fn.greg` — the informative failure
* `tmp/grind/func_8002EA24/s9/v6_vout_dies_in_chain_no_L3/fn.greg` — proof the
  dual-direction carrier never becomes an allocno
* `tmp/grind/func_8002EA24/s9/v{0,1,2,3,4,5,6}_*.c` — the measured bodies
* `tmp/grind/func_8002EA24/s9/dmsearch.py`, `dmsearch2.py`, `dmsearch3.py` — the
  three corpus censuses (re-runnable; the corpus is in `tmp/decomp_me_corpus`)

- [s9] Floor re-measured at 2 (build 104 insns = target) at both the start and the end of the session, with the banked candidate.c spliced into src/code6cac_b.c; src is left in that state.

- [s9] The FULL .greg allocator state of the banked score-2 body is written down for the first time (evidence.md, tmp/grind/func_8002EA24/s9/banked/fn.greg): order `101 96 97 100 109 108 72 102 117 103 74 99 75`; allocno 97 (a0_var) is the ONLY allocno assigned hard reg 4 ($a0), carries `preferences: 4`, is allocated third, and appears in `103 conflicts:` -- which is exactly why neg_threshold reaches hard reg 9 ($t1) = target. Allocno 102 also carries `preferences: 4` but does NOT conflict with 103 and is assigned 3 ($v1), so it is inert. Allocno 100 (x) is in 5 ($a1) = target.

- [s9] Every register in the function now matches target except the destination of the FIRST range test's slt: ours `slt $a0,$a1,$t1 / bnez $a0`, target `slt $v0,$a1,$t1 / bnez $v0`. That is the entire two-point residual and it is a consequence of L3, not an independent defect.

- [s9] A control body regenerated through this session's variant template scores exactly 2 at 104 insns, so no measurement below can be a template artefact.

- [s9] Forward carrier enumeration (value live PAST the chain), now complete: boolean/L3 2 (banked), x 6, vin/vout 6/5 (s8), y hoisted 11 (s5), delta temps 16-28 (s4), r_sq 19, sum of squares 19 (s3 accearly), threshold 24, -threshold = allocno 103 itself. Nothing beats 2.

- [s9] Dual carrier enumeration (value dying INSIDE the chain), now complete: threshold 24 (works mechanically, displaces threshold out of $a2) and vout 2/3 (never becomes an allocno -- GCC re-materialises obj+0x100/0x104 off $t0). The generator matrix is exhausted in both directions.

- [s9] decomp.me corpus census #1 (NEGATIVE): of 1751 MATCHED GCC-2.7.2 scratches, 0 have a `negu` into a $t/$s register that is then an `slt` right operand; the 8 loose hits are 7 callee-saved $s cases (live across a jal; this function is a leaf) plus one argument-copy case that also uses a register-asm pin.

- [s9] decomp.me corpus census #2 (NEGATIVE): 39 of 1751 matched scratches keep an unfolded 0/1 diamond, 12 without any loop, and every one inspected keeps it because the arm contains real work. No matched pure-C precedent exists for func_8002EA24's bare-arm shape.

- [s9] Recorded for honesty, NOT as authorization: the corpus does contain matched scratches carrying self-declared coercions (VZWgF has a literal `// hack` over a no-op `check238++; check238--;` pair). decomp.me scratches are not a SOTN-master precedent and this project's endgame gate does not accept them as one.

## Session 10 (synthesis) — the `regs_someone_prefers` channel is REACHABLE, and it is manufacturable

Floor: **2** (re-measured at the start and the end of the session with the banked
body applied to `src/code6cac_b.c`; build 104 insns = target's).  Body unchanged.

### What the merge of sessions 2-9 actually said, and the hole in it

Session 7 established two things that had never been put together:

1. Target's own compile CANNOT have excluded `$a0` from allocno 103
   (`neg_threshold`) by an assigned conflict — target writes `$a0` nowhere
   between the prologue copy `addu $t0,$a0,$zero` and `addu $a0,$v0,$v1` after
   the multiplies — so the original's exclusion came from
   `regs_someone_prefers`, i.e. from an allocno that PREFERS `$a0`, CONFLICTS
   with 103 and ranks BELOW it in `allocno_compare` order.
2. In every one of the 11 bodies dumped up to that point, the only two
   `$a0`-preferring allocnos were 97 (`a0_var`) and 102 (`y`), and BOTH outrank
   103 — so the channel was declared closed and sessions 8-9 spent themselves on
   the assigned-conflict route (the H5-prime-5 carrier enumeration) instead.

The hole: **the set of `$a0`-preferring allocnos is not an input, it is an
output.**  `expand_preferences` (`tools/gcc-2.7.2/global.c:797-841`) walks every
insn, and for any `single_set` whose REG_NOTES carry a `REG_DEAD` for an allocno
that does NOT conflict with the set allocno, it IORs the two allocnos'
`hard_reg_preferences` / `hard_reg_full_preferences` **in both directions**.
Hard reg 4 exists as a preference in this function at all only because `obj`
arrives in `$a0`; 102 (`y`) has `preferences: 4` because `obj` (72) dies at the
`lw $v1,0x108($t0)` that loads `y`.  So the preference is propagable — the
question is only which allocno it can be propagated ONTO.

### The requirement, restated exactly (from global.c:851-899)

For `$a0` to be excluded from 103 through the preference channel, some allocno R
must satisfy ALL of:
  (a) `hard_reg_full_preferences[R]` contains 4 after pruning (so R must not
      itself conflict with hard reg 4);
  (b) `CONFLICTP(103, R)`;
  (c) R ranks BELOW 103 in `allocno_order` (the loop at :888 only merges
      `j > i`);
  (d) 103 must NOT itself prefer hard reg 4 — line 893 removes from the merged
      set every register the higher-priority allocno also prefers (when the
      sizes are equal, which they are here).

Only three allocnos rank below 103 in this function: 74 (`threshold`), 99 and
75 (`r_sq`).  74 and 75 conflict with 103; 99 does not.

### (1) The mechanism FIRES, and it reaches target's register pair without L3

`v1_thr_tail_noL3` — the no-L3 base with the last range test written as
`threshold = y + a0_var; if (threshold < min_y) { z = 0; return z; }`, so that
the insn which KILLS 102 (`y`) SETS 74 (`threshold`), and 102/74 do not conflict.

Measured `.greg` (`tmp/grind/func_8002EA24/s9/s10_v1/fn.greg`):
```
;; 13 regs to allocate: 101 96 97 100 109 108 72 102 117 103 74 99 75
;; 74 preferences: 4 6          <- was `6`; hard reg 4 propagated from 102
;; 102 preferences: 4 6         <- the reverse leg of the same IOR
;; 103 conflicts: 72 74 75 96 100 103 2 29      (no 97 — L3 is absent)
103 in 9   (= $t1 = TARGET)     104 in 2  (the first test's boolean, = $v0 = TARGET)
```
This is the first time in ten sessions that `neg_threshold` reaches `$t1` and
the first range test's boolean stays in `$v0` **simultaneously and without the
L3 staged boolean** — i.e. the exact configuration session 7 proved the original
compile had, produced from pure C.

### (2) Why it does not repay yet: the IOR is symmetric

`102` (`y`) inherits 74's own `$a2` preference in the same statement, and
`find_reg`'s own-preference override then assigns `y` to `$a2` instead of
target's `$v1`.  Tail diff against target:
```
target   lw v1,0x108(t0) ... addu v0,v1,a0 ; slt v0,v0,a2
ours     lw a2,0x108(t0) ... addu a2,a2,a0 ; slt v0,a2,v1
```
Score 6 against the no-L3 control's 3.  Same story for the `r_sq` spelling
(`r_sq = y + a0_var`, propagates 4 onto 75, `y` then takes `$a3`): 6.  With L3
kept, both are 8 (L3's assigned conflict and the preference channel are
redundant, and the `y` displacement is paid twice).  A variant that writes the
parameter at a test where `y` does NOT die (`threshold = y - a0_var` at the
first tail test) is 8 with no propagation at all — the `REG_DEAD` note is the
load-bearing part, not the parameter write.

### (3) A fresh local as recipient is inert — and gives condition (d) teeth

`v6_freshlocal_bool_and_tail` — a new local `t` carries the first range test's
boolean (to be live across the chain and conflict with 103) and is re-used at the
tail for `y + a0_var` (to be set by the insn that kills `y`).  Score 3 = the
control.  The dump says why, and it is a general constraint:
```
;; 14 regs to allocate: 103 101 96 97 100 110 109 72 102 118 104 74 99 75
;; 103 preferences: 2 4      <- 103 ITSELF now prefers hard reg 4
103 in 2   104 in 4
```
The same symmetric IOR that feeds `t` also feeds hard reg 4 (and the boolean's
`$v0`) back into 103, and `prune_preferences` line 893 then removes 4 from
`regs_someone_prefers[103]` — the route cancels itself.  103 is additionally
promoted to FIRST in the allocation order by the extra references.  So the
recipient must be preference-connected to `y` (or another pref-4 allocno) and
NOT to 103.

### Measured this session (all `sandbox func_8002EA24 --disable all`)

| variant | base | score | what it changes |
|---|---|---|---|
| banked candidate | — | **2** | control, start and end of session |
| v0_control_noL3 | no-L3 | 3 | regenerated control |
| v1_thr_tail_noL3 | no-L3 | 6 | `threshold = y + a0_var` tail carrier (mechanism fires) |
| v2_rsq_tail_noL3 | no-L3 | 6 | same with `r_sq` |
| v3_thr_tail_L3 | L3 | 8 | v1 with L3 kept |
| v4_rsq_tail_L3 | L3 | 8 | v2 with L3 kept |
| v6_freshlocal_bool_and_tail | no-L3 | 3 | fresh local recipient (poisons 103's own prefs) |
| v7_freshlocal_tail_only | no-L3 | 3 | fresh local, no early segment (no conflict with 103) |
| v8_L3_plus_freshlocal_tail | L3 | 2 | inert on the banked body |
| v9_thr_first_tail_test | no-L3 | 8 | parameter written where `y` does NOT die |

### Artifacts

- `tmp/grind/func_8002EA24/s10/gen.py`, `gen2.py` — variant generators (round 1 / round 2)
- `tmp/grind/func_8002EA24/s10/v*.c` — the nine measured bodies
- `tmp/grind/func_8002EA24/s10/bank.py`, `fix_header.py`, `ledger.py` — banking
- `tmp/grind/func_8002EA24/s9/s10base/fn.greg`, `s10_v1/fn.greg`, `s10_v6/fn.greg` — allocator dumps
- `memory/grind/func_8002EA24/rejected/pref-propagation-into-threshold-param-score6.c`
- `memory/grind/func_8002EA24/rejected/fresh-local-pref-recipient-poisons-103-score3.c`

- [s10] Floor re-measured at 2 (104 build insns = target) with the banked candidate applied to src/code6cac_b.c, at the start and the end of the session; the banked body is unchanged.

- [s10] GCC 2.7.2's expand_preferences (tools/gcc-2.7.2/global.c:797-841) IORs hard_reg_preferences / hard_reg_full_preferences BOTH ways across any single_set insn whose REG_NOTES carry a REG_DEAD for an allocno that does not conflict with the set allocno — so which allocnos prefer $a0 is an output of the C shape, not a fixed input. Hard reg 4 exists as a preference in this function only because `obj` arrives in $a0; 102 (`y`) has `preferences: 4` because `obj` (72) dies at the y load.

- [s10] The full requirement for the preference route, read out of global.c:851-899: the recipient R must have 4 in its PRUNED full preferences (so R must not conflict with hard reg 4), must CONFLICTP with 103, must rank BELOW 103 in allocno_order (the :888 loop only merges j > i), and 103 must NOT itself prefer 4 (line 893 cancels the merge for equal allocno_size).

- [s10] Only three allocnos rank below 103 in this function (74 = threshold, 99, 75 = r_sq); 74 and 75 conflict with 103, 99 does not. 102 (`y`) conflicts with neither 74 nor 75, which is exactly what makes it a legal preference donor to both.

- [s10] v1_thr_tail_noL3 (`threshold = y + a0_var;` as the last range test, no L3) puts `74 preferences: 4 6` in the dump and assigns 103 to hard reg 9 = $t1 (target) with the first test's boolean in $v0 — the first form in ten sessions to reach target's register pair without the L3 staged boolean. Score 6 against the no-L3 control's 3, entirely because the reverse IOR leg moves `y` from $v1 to $a2.

- [s10] The r_sq spelling behaves identically with $a3 (6); both spellings with L3 kept are 8; writing the parameter at a test where `y` does not die (v9) is 8 with no propagation at all.

- [s10] A fresh-local recipient is inert (3) and poisons the route by giving 103 itself `preferences: 2 4`, which prune_preferences line 893 then cancels; the fresh local also promotes 103 to first in the allocation order.

- [s10] Nine bodies measured this session, all at 104 instructions: control 3 (no-L3) / 2 (banked); v1 6, v2 6, v3 8, v4 8, v6 3, v7 3, v8 2, v9 8. Nothing below the banked 2, so candidate.c is unchanged apart from the session-10 header block.

## Session 11 (structural) — H8' closed by recipient enumeration

### Artifacts

- `tmp/grind/func_8002EA24/s11/apply.py`, `score.ps1`, `dump.sh` — harness (splice a variant into src, score it, dump the .greg)
- `tmp/grind/func_8002EA24/s11/gen.py`, `gen2.py`, `gen3.py`, `gen4.py` — variant generators (rounds 1-4)
- `tmp/grind/func_8002EA24/s11/{v1,v2,v3,v4,v5,w1,w2,w3,w4,x1,x2,y1,y2}*.c` — the thirteen measured bodies
- `tmp/grind/func_8002EA24/s11/{ctl,v1,v3,v5,w2,x1_z_carrier_L1_via_ylow,x2_z_carrier_no_L1}/fn.greg` — allocator dumps
- `memory/grind/func_8002EA24/rejected/z-pref-recipient-harmless-but-outranks-103-score16.c`
- `memory/grind/func_8002EA24/rejected/maxy-pref-recipient-conflicts-with-donor-score5.c`
- `memory/grind/func_8002EA24/rejected/fresh-local-secondbool-recipient-poisons-103-score3.c`
- `memory/grind/func_8002EA24/rejected/miny-hoist-to-block-y-demotes-the-blocker-score13.c`
- `memory/grind/func_8002EA24/candidate_alt_L1_via_ylow.c` — score-2 alternative spelling of L1

- [s11] Floor re-measured at 2 (104 build insns = target's) with the banked candidate applied to src/code6cac_b.c at the start of the session; the banked body is unchanged at the end.

- [s11] The control .greg for the no-L3 base (score 3) reads: allocation order `101 96 97 100 109 108 72 102 117 103 74 99 75`; `103 conflicts: 72 74 75 96 100 103 2 29`; dispositions 96→$v1, 97→$a0, 99→$a2, 100→$a1, 101→$v0, 102→$v1, 103→$a0. Target wants 103 in $t1. 103's exclusion set at find_reg time is {2,3,5,6,7,8,29} — every register up to $t0 EXCEPT hard reg 4, which is the single missing bit sessions 6-10 named.

- [s11] The recipient set for the preference route is CLOSED at six candidates, because a recipient must CONFLICT with 103 and only {72, 74, 75, 96, 100} do (plus a manufactured fresh local). All six are now measured; see the four rejected forms above and session 10's two.

- [s11] `max_y` (allocno 100) cannot be a recipient: it CONFLICTS with the donor 102 (`y`) and expand_preferences is gated on `! CONFLICTP` in both directions. The .greg for the max_y-recipient body is BIT-IDENTICAL to the control's (same allocnos, order, conflicts, preferences and dispositions); its 2-point loss (5 vs 3) is pure scheduling. With L3 kept it is 4.

- [s11] `z` (allocno 96) is the ONLY recipient that satisfies both of session 10's side-conditions simultaneously: with `z = y + a0_var;` as the last range test the dump shows `96 preferences: 4` (forward leg fired) AND `102 preferences: 4` unchanged (harmless reverse leg — z owns no argument-register preference) AND `103 preferences` still empty (no line-893 cancellation). First time in eleven sessions that all three hold at once.

- [s11] `z` nonetheless fails, on rank alone: 96 is allocated SECOND, so it takes $a0 itself rather than denying it to 103. The cascade moves 100 from target's $a1 to $v1, 102 from target's $v1 to $a1, and 103 to $a1. Score 16 at 104 instructions (pure register loss); with L3 kept, 17.

- [s11] Demoting 96 below 103 is unreachable. allocno_compare ranks by floor_log2(n_refs)*n_refs/live_length and 103 has only 3 references, so 96 needs a 3-8x live-length penalty. Cutting z's reference count does nothing: staging L1's return through `y_low` instead of `z` (x1, 16) and removing L1 altogether (x2, 19 at 102 insns) both leave the allocation order BIT-IDENTICAL with 96 second. Lengthening z's live range is blocked by the function's dataflow — `*(s32 *)(obj + 0x104)` is WRITTEN by the GTE store block (`swc2 $26, 4($t4)`) immediately above the load, so z cannot be born earlier.

- [s11] A manufactured fresh local below 103 poisons `103 preferences` regardless of what value it carries. Session 10's spelling carried the first range test's boolean (which reads neg_threshold); this session's carried the other half of the same short-circuit (`threshold < max_y`, which never mentions neg_threshold) and the dump is the same: `103 preferences: 2 4` and 103 promoted to FIRST in the order, landing in $v0 while the fresh local takes $a0. Both inert at 3. The poisoning is a re-ranking effect, not an operand-level one.

- [s11] The reverse leg of the IOR cannot be blocked. find_reg only refuses `y` a register that is in `used`, and a non-conflicting holder of $a2 does not put it there, so keeping `y` off the parameter recipient's argument register needs an allocno X with CONFLICTP(102,X), rank above 102, and assignment $a2. Placing that conflict on the recipient itself is self-defeating (the IOR gate is symmetric, so it kills the forward leg too).

- [s11] The only X candidate is 99 (min_y), and promoting it is impossible in the required direction: the sole lever is an earlier birth, and allocno_compare has live_length in the DENOMINATOR. Measured (`min_y = 0;` hoisted above the z load, with the threshold recipient): 99 moves from 12th to LAST (`101 97 96 100 109 108 72 102 117 74 103 75 99`), acquires a conflict with 103 and lands in hard reg 9 — it STEALS target's $t1 — while 103 keeps $a0 and 102 keeps $a2. Score 13 at 105 instructions; the hoist also re-ranks 74 above 103, breaking condition (c) for the recipient as well. The control for the hoist alone is 10 at 105 insns.

- [s11] L1's staging variable is FREE. Staging the returned 0 through `y_low` (dead from the min_y/max_y if-else onwards) instead of through `z` measures 2 on the banked body and 3 on the no-L3 body — identical scores at target's 104 instructions. The store-flag defeat is therefore a property of the two-statement arm (jump.c's single-set precondition), not of the particular local reused. Banked as `candidate_alt_L1_via_ylow.c`.

- [s11] Thirteen bodies measured, all against the same two controls (no-L3 = 3, banked = 2): v1 16, v2 17, v3 5, v4 4, v5 3, w1 10 (105 insns), w2 13 (105), w3 12 (105), w4 8, x1 16, x2 19 (102), y1 3, y2 2. Nothing below the banked 2, so candidate.c's body is unchanged.

- [s11] Floor re-measured at 2 (build 104 insns = target's 104) with the banked candidate applied to src/code6cac_b.c at the start of the session; src restored to HEAD at the end, banked body unchanged.

- [s11] Control .greg (no-L3 base, score 3): order `101 96 97 100 109 108 72 102 117 103 74 99 75`; `103 conflicts: 72 74 75 96 100 103 2 29`; dispositions 96 -> $v1, 97 -> $a0, 99 -> $a2, 100 -> $a1, 101 -> $v0, 102 -> $v1, 103 -> $a0. 103's find_reg exclusion set is {2,3,5,6,7,8,29} -- everything up to $t0 EXCEPT hard reg 4, the single missing bit sessions 6-10 named.

- [s11] The recipient set for the preference route is CLOSED at six candidates: a recipient must conflict with 103, and only {72 obj, 74 threshold, 75 r_sq, 96 z, 100 max_y} do, plus any manufactured fresh local. All six are now measured -- 72 pruned by its own hard-reg self-conflict (s3); 100 gated out by CONFLICTP with the donor (s11, .greg bit-identical to control, score 5); 74/75 reach target's $t1 but move `y` (s10, 6/6); fresh local poisons `103 preferences` regardless of carried value (s10 + s11, both 3); 96 satisfies everything but rank (s11, 16).

- [s11] The fresh-local poisoning is a RE-RANKING effect, not an operand-level one: session 11's spelling carried `threshold < max_y`, which never mentions neg_threshold, and the dump still shows `103 preferences: 2 4` with 103 promoted to first in the allocation order.

- [s11] Cutting z's reference count does not move the allocation order at all (x1 16, x2 19 at 102 insns, order bit-identical), and z cannot be born earlier because the GTE store block writes *(s32 *)(obj + 0x104) immediately above the load.

- [s11] Promoting min_y to block `y` inverts into a demotion (live_length is allocno_compare's denominator): 99 falls to LAST, steals target's $t1, and the body costs an extra instruction (105 vs 104). Scores w1 10, w2 13, w3 12.

- [s11] A conflict placed on 102 to stop the reverse leg also stops the forward leg -- expand_preferences' `! CONFLICTP` gate is symmetric -- so the parameter-recipient family has no repair.

- [s11] L1's staging variable is FREE: `y_low` reproduces the `z` spelling exactly (2 on the banked body, 3 on the no-L3 body, 104 insns both). Relevant to the L1 disposition review: the construct cannot be defended or attacked on the grounds that one specific local was required.

- [s11] Thirteen bodies measured against the two standing controls (no-L3 = 3, banked = 2): v1 16, v2 17, v3 5, v4 4, v5 3, w1 10 (105), w2 13 (105), w3 12 (105), w4 8, x1 16, x2 19 (102), y1 3, y2 2. Nothing below 2, so candidate.c's body is unchanged (header block appended).

## Session 12 (escalation) — DISPOSITION REACHED.  Floor re-measured at 2; H7 and H9 both KILLED.

The driver assigned `escalation` after the honest floor stayed flat at 2 across
sessions 5–11 and six distinct modalities.  This session (a) re-measured the floor
with the banked candidate applied to src/ — `sandbox --disable all` → **score 2,
104 insns vs target 104, 10 rules dropped, 298 chars of cheat-asm stripped**;
(b) ran the last un-tried MECHANICAL axis (H7) and killed it with measurements;
(c) closed H9 by case analysis over the banked `.greg` dumps; (d) evaluated the two
endgame-lock AND-gates, found BOTH failing, and applied the owner's standing
auto-ruling of 2026-07-27 — entry filed in `docs/grind/decisions.md` under
**OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED /
OWNER-ACCEPTED INCOMPLETE**.

### [s12] H7 — the score-2 plateau is a property of ONE permuter mutation neighbourhood, so a campaign launched from a structurally different score-2 chassis can escape it.
- mechanism: decomp-permuter mutates the SOURCE, not the RTL, so two bodies that
  compile to identical bytes still have different mutation neighbourhoods.  Session 5's
  29,050-iteration basin exhaustion (plus 34,300 directed iterations) was measured from
  the banked chassis ONLY, and nine structurally distinct score-2 bodies are now known.
- probe: Three fresh-seed campaigns launched CONCURRENTLY with
  `tools/permuter_campaign.py launch -j 4` from workspaces built by
  `tmp/grind/func_8002EA24/s12/mkws.py` (session-5 workspace `tmp/perm_ea24g` cloned;
  only the `func_8002EA24` body inside the preprocessed `base.c` swapped) —
  A = `s8/v10_nested_range_tests_only.c` (m2c's nested shape),
  B = `s8/v12_single_four_way_if.c` (one fused short-circuit `if`, z assigned inside
  the condition), C = `s9/v5_vout_dies_in_chain.c` (X and Z read off the GTE output
  pointer).  All three reported `base_score: 10` = the two register diffs × the
  permuter's reg weight, i.e. the sandbox floor of 2 in the permuter metric.  Waited
  in-turn with three blocking `wait` windows (~28 minutes of wall clock, all three
  running throughout), then `harvest --stop` on each.
- result: **Zero novel finds on every chassis.**  Iteration counters at the wait
  windows: A 9,800 @ 9 min, B 18,148 @ 18 min, C 27,384 @ 27 min (~1,000 iters/min
  per campaign → ~28,000 each, ~84,000 total).  Every harvest returned
  `finds_new: 0`, `best_new_score: null`, `finds: []`.  Nothing anywhere in the three
  neighbourhoods scored below 10.  The permuter axis now stands at ~148,000 measured
  iterations across FOUR structurally distinct score-2 chassis with nothing below the
  plateau.  Artifact: `tmp/grind/func_8002EA24/s12/permuter_h7_summary.md`.
- verdict: KILLED

### [s12] H9 — the missing `$a0` exclusion for allocno 103 (neg_threshold) can come from a THIRD case sessions 3–11 never enumerated: an allocno assigned hard reg 4 for a reason other than preferring it, which also conflicts with 103.
- mechanism: 103's pass-0 exclusion set is already {2,3,5,6,7,8,29}; the single missing
  bit is 4.  Sessions 3–9 enumerated the assigned-conflict route through allocno 97
  (a0_var, the function's only `$a0`-preferring allocno) and sessions 10–11 enumerated
  the `regs_someone_prefers` preference route.  The untouched case is an allocno whose
  OWN first-fit lands on `$a0` because `$v0`/`$v1`/`$a1` are excluded for it.
- probe: Case analysis over the banked `.greg` dumps rather than a new build — the case
  set is finite and every case already carries a measurement.  From this body's `.greg`,
  the allocnos that CONFLICT with 103 are exactly {72 obj, 74 threshold, 75 r_sq, 96 z,
  100 max_y}, and the allocation order is `101 96 97 100 109 108 72 102 117 103 74 99 75`,
  so the ones allocated BEFORE 103 — the only ones that can supply an assigned conflict —
  are {72, 96, 100}.
- result: The case set is empty by construction.  In TARGET those three allocnos hold
  `$t0` (`addu $t0,$a0,$zero`), `$v1` (`slt v0,v1,t1`) and `$a1` (`lw a1,256(t0)` /
  `mult a1,a1`) respectively, so any edit that pushes one of them onto `$a0` necessarily
  vacates a register target itself requires — and each case is separately measured:
  `z` forced onto `$a0` = score 16 (s11, banked as
  `rejected/z-pref-recipient-harmless-but-outranks-103-score16.c`), the `max_y` recipient
  = 5 with a BIT-IDENTICAL `.greg` (s11), `obj`'s `$a0` preference self-pruned by
  prune_preferences line 877 because obj hard-conflicts with `$a0` (s3).  A *newly
  introduced* allocno cannot substitute either: `allocno_compare` divides by live_length,
  so any zero-instruction-cost carrier ranks LAST — measured in session 8, where the
  added carrier (allocno 77) was allocated AFTER 103 and stole target's `$t1` outright.
  With H9 closed, all three routes to the one missing allocator bit (assigned conflict
  from 97, preference propagation, assigned conflict from anything else) are enumerated
  and exhausted.
- verdict: KILLED

### [s12] Gate 1 — func_8002EA24 shows hand-written-asm signals strong enough to authorize a canonical-asm disposition.
- mechanism: `.claude/rules/endgame-lock-disposition.md` criterion 1 — canonical-asm is
  allowed ONLY with STRONG-tier `scan_hand_coded` signals (S1 multu pacing / S2 empty
  branch / S6 BIOS jumptable).
- probe: `python3 tools/scan_hand_coded.py --single func_8002EA24`.
- result: `tier=TIGHT_C score=3/8` — "tight pure-C function or cluster (no
  GCC-impossible signals)".  S1, S2 and S6 all absent; the three that fire (S3 no
  spills, S4 front loads, S5 cluster with func_8002D320 at jaccard 0.60) are weak-tier
  and are not credited by the policy.  A non-STRONG score is dispositive: refuse asm.
- verdict: KILLED (gate FAILS)

### [s12] Gate 2 — an in-hand SOTN-master precedent exists for the construct that would close the residual.
- mechanism: `.claude/rules/endgame-lock-disposition.md` criterion 2 — a coercion /
  spelling family is sanctioned ONLY with an EXHIBITED community precedent (file+line or
  commit), never "same spirit" or "only lever left".
- probe: Identify the closing construct and look for its precedent.
- result: There is no closing construct to cite a precedent FOR.  The two coercion
  constructs already in the body (L1, L3) belong to the already-sanctioned
  [[staged-value-reused-variable]] family and are what took the floor 9 → 2; they do not
  close the last 2 points, and after twelve sessions no construct of any kind is known
  that does.  The two corpus censuses that were run came back NEGATIVE (session 9,
  decomp.me: "negu into `$t`/`$s` consumed by an slt" = 0 of 1,751 matching GCC-2.7.2
  scratches; 0 of the 39 matched unfolded-0/1-diamond scratches has this function's bare
  `return 0;` / `return 1;` shape), which per the policy is a failed gate, not an open
  question.
- verdict: KILLED (gate FAILS)

### Disposition
BOTH gates fail → the owner's 2026-07-27 standing auto-ruling applies with no owner
wait.  Entry filed at `docs/grind/decisions.md` (2026-07-30, func_8002EA24):
REFUSED / OWNER-ACCEPTED INCOMPLETE — keep the 10 rules + cheat-asm on main so the
oracle stays green, classify INCOMPLETE-owner-accepted (neither COMPLETED state), park
out of active grind but eligible for re-attempt if a genuine pure-C lever or new tooling
emerges.  src/code6cac_b.c was restored to HEAD at the end of the session so main keeps
its byte match; the honest score-2 body remains banked at
`memory/grind/func_8002EA24/candidate.c`.

### Frontier at park time (for any future re-attempt)
Nothing mechanical is left on the current toolchain.  The two things that would reopen
this function are (1) a new *tool* — e.g. an exhaustive RTL-level search over the
allocno conflict graph that can answer "which C dataflow produces conflict set X" in the
forward direction, rather than the guess-and-measure loop twelve sessions have run; or
(2) a genuinely new sanctioned pure-C construct family with its own community precedent,
which would have to arrive from outside this function's grind.  The residual is one bit
in `find_reg`'s pass-0 exclusion set for pseudo 103, and every C-visible generator of
that bit is enumerated and measured.

- [s12] Floor re-measured THIS session with the banked candidate applied to src/code6cac_b.c: sandbox func_8002EA24 --disable all -> score 2, build_insns 104 vs target_insns 104, rules_dropped 10, cheat_asm_stripped 298. Every instruction's opcode and operand already matches; the whole residual is one register pair (ours slt $a0,$a1,$t1 / bnez $a0 vs target slt $v0,$a1,$t1 / bnez $v0).

- [s12] What holds the byte-match on main: 10 regfix/asmfix rules plus a cheat-asm body (register asm() pins on t4/v0/a1/t1/a0/a2, hardcoded-$N __asm__ templates such as 'lwc2 $0, 0($12)' and '.word 0xE99A0004' carrying a pinned operand, and a bare __asm__ volatile("" ::: "memory") scheduling barrier). This is the endgame RA-lock species exactly as .claude/rules/endgame-lock-disposition.md defines it.

- [s12] The residual is localised in the compiler, not the source: one bit (hard register 4 = $a0) in the pass-0 hard-register exclusion set computed by find_reg (tools/gcc-2.7.2/global.c:1012-1044) for the neg_threshold allocno (pseudo 103). With the bit, first-fit hands 103 target's $t1; without it, $a0.

- [s12] GATE 1 FAILS: scan_hand_coded --single func_8002EA24 = tier TIGHT_C, score 3/8, no S1/S2/S6.

- [s12] GATE 2 FAILS: no closing construct exists to exhibit a precedent for; the two corpus censuses that were run returned negative.

- [s12] H7 KILLED: ~84,000 fresh-seed permuter iterations across three structurally distinct alternative score-2 chassis, all harvested with finds_new 0 / best_new_score null. Combined with session 5's 29,050 random + 34,300 directed iterations, the permuter axis stands at ~148,000 iterations over FOUR chassis with nothing below the plateau. This was the last un-tried MECHANICAL axis on the frontier.

- [s12] H9 KILLED: the allocnos that both conflict with 103 and are allocated before it are exactly {72 obj, 96 z, 100 max_y}; target itself needs those three in $t0/$v1/$a1, and each case already carries its own measurement (16 / 5 / self-pruned). A newly introduced zero-cost carrier ranks LAST by allocno_compare (live_length in the denominator) and cannot be allocated before 103 - measured in session 8.

- [s12] Exhaustion record: floor 18 -> 9 (s2-s4) -> 2 (s5) and flat at 2 for sessions 5,6,7,8,9,10,11,12; six distinct modalities (recon, structural x4, permuter, forensics x2, rederive x2, synthesis, escalation); ~60 distinct C bodies measured, 44 banked in memory/grind/func_8002EA24/rejected/; the plateau is source-shape-invariant across nine structurally distinct bodies all scoring exactly 2 at 104 instructions.

- [s12] Standing-ruling entry FILED this session at docs/grind/decisions.md, titled '2026-07-30 - func_8002EA24 (src/code6cac_b.c) - OWNER-ESCALATION - RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE'. Nothing is pending on the owner.

- [s12] src/code6cac_b.c was restored to HEAD at the end of the session (git checkout), so main keeps its byte match and the full-build oracle stays green; the honest score-2 body remains banked at memory/grind/func_8002EA24/candidate.c with candidate_alt_score3_no_fake.c and candidate_alt_L1_via_ylow.c as fallbacks.

- [s12] All three permuter campaigns were harvested with --stop before this outcome was written; procs_killed 5 on each, no campaign left running.


## [s13] 2026-08-20 -- escalation -- chassis re-measurement + disposition re-filed

FACT (measured this session, not inherited).  The banked candidate body
(candidate.c:229-373) applied to src/code6cac_b.c over the post-migration tree
scores exactly as it did before the 2026-08-19 asm-until-matched migration:

    sandbox func_8002EA24 --disable all
    {"score": 2, "target_insns": 104, "build_insns": 104, "scorable": true,
     "rules_dropped": 0, "cheat_asm_stripped": 237}

This matters because the migration changed the chassis under the whole ledger:
main's representation for this function is now `INCLUDE_ASM("asm/funcs",
func_8002EA24);` at src/code6cac_b.c:1160, and the 10 legacy regfix rules that
the 2026-07-30 escalation entry cited as "what holds the byte-match" are
retired (archived at retired-chassis-2026-08/rules.txt).  There is now NO cheat
on main for this function at all -- zero regfix, zero asmfix, zero cheat-asm,
not in inline_asm_canonical.txt.  Since the floor is unchanged at 2, every
chassis-relative conclusion in this ledger is confirmed to carry over and does
not need re-derivation.

FACT.  Gate (a) re-run this session, unchanged from 2026-07-30:
`scan_hand_coded --single func_8002EA24` -> tier=TIGHT_C score=3/8, with S3
(no spills), S4 (front loads), S5 (cluster: func_8002D320 jaccard 0.60) set and
all three STRONG signals (S1 multu pacing, S2 empty branch, S6 BIOS jumptable)
clear.  No canonical-asm grant path.

FACT.  Gate (b) census run this session, NEGATIVE.  Grepping
docs/reference/sotn-construct-index.md (1,365 indexed SOTN-master constructs)
for `conflict`, `allocno`, `find_reg`, `global_alloc`, `zero-cost`, `costless`
returns zero hits.  SOTN master ships no indexed construct whose purpose is to
force a register-allocation conflict at zero instruction cost.  This confirms
in-tree what session 9's two decomp.me corpus censuses found externally.

FACT.  The owner's 2026-08-07 LZC-island pre-approval (decisions.md:3906) is
NOT spendable here.  It is scoped "effective ONLY when the function otherwise
matches"; at floor 2 it does not.  The same ruling explicitly excludes the
vector/MVMVA block.  And neither island is where the residual lives -- the two
mismatching instructions are ordinary range-test register allocation roughly 40
instructions away from either GTE island.  This closes the one axis that looked
NEW since the last escalation.

DISPOSITION.  Both gates fail; filed under the owner's 2026-07-27 standing
ruling as REFUSED / OWNER-ACCEPTED INCOMPLETE at docs/grind/decisions.md:7843
(that entry supersedes the 2026-07-30 one at :2457, whose gate-(b) facts the
migration invalidated).  Terminal, nothing pending on the owner, explicitly
re-attempt-eligible on either (1) new tooling that inverts the C-dataflow ->
allocno-conflict-graph mapping, or (2) a newly sanctioned pure-C family with
its own community precedent that produces an allocator conflict at zero
instruction cost.

- [s14] Honest floor on the current post-migration chassis is 2, re-measured this session with the banked candidate applied: {"score": 2, "target_insns": 104, "build_insns": 104, "scorable": true, "rules_dropped": 0, "cheat_asm_stripped": 237}. Instruction count and every instruction opcode match target.

- [s14] The entire residual is two register choices on one compare: ours `slt $a0,$a1,$t1 ; bnez $a0,<reject>` vs target `slt $v0,$a1,$t1 ; bnez $v0,<reject>`.

- [s14] The floor is unchanged across the migration, so the 44 rejected forms, ~118k permuter iterations and the find_reg forensics banked in memory/grind/func_8002EA24/ are all valid on today's chassis and need no re-derivation.

- [s14] On main today src/code6cac_b.c:1160 is `INCLUDE_ASM("asm/funcs", func_8002EA24);` -- zero regfix rules, zero asmfix rules, zero cheat-asm, not in inline_asm_canonical.txt. The 10 legacy regfix rules the 2026-07-30 escalation entry cited as 'what holds the byte-match' are retired and archived at memory/grind/func_8002EA24/retired-chassis-2026-08/rules.txt. This is therefore NOT an accept-a-cheat request and NOT an integration handoff: no operator step exists that would complete it.

- [s14] Gate (a) FAILS: scan_hand_coded --single func_8002EA24 -> tier=TIGHT_C score=3/8, no S1/S2/S6.

- [s14] Gate (b) FAILS: there is no closing construct to cite a precedent for, and the SOTN construct-index census (1,365 entries, terms conflict/allocno/find_reg/global_alloc/zero-cost/costless) returns zero hits.

- [s14] The 2026-08-07 LZC-island pre-approval cannot be spent: it is conditioned on the function otherwise matching, it excludes the vector/MVMVA block, and neither island contains the residual.

- [s14] Exhaustion: 13 sessions, floor FLAT at 2 for the last ten, six distinct modalities spent (recon, structural s2/s3/s11, permuter s4/s5, forensics s6/s7, rederive s8/s9, synthesis s10, escalation s12/s13); plateau proven source-shape-invariant by s8's fresh-m2c rebuild plus 11 structural rewrites over six distinct bodies.

- [s14] Residual localized inside the compiler, not the source: pass global_alloc (tools/gcc-2.7.2/global.c), decision point find_reg (:1012-1044), the pass-0 hard-register exclusion set for the neg_threshold allocno (pseudo 103) is missing hard register 4 ($a0); with the bit first-fit yields $t1 (target), without it $a0 (ours). All four GCC generators of that bit are enumerated and each measured dead (s5 live-at-entry costs instructions; s7 assigned-conflict-from-97 killed the priority axis; s11 closed expand_preferences by exhaustive recipient enumeration; s8/s9 completed the seventh-live-value carrier enumeration in both directions).

- [s14] Disposition filed this session at docs/grind/decisions.md:7843, superseding the 2026-07-30 entry at :2457 whose gate-(b) facts the migration invalidated. Terminal per the owner's 2026-07-27 standing ruling; nothing pending on the owner; explicitly re-attempt-eligible.

## SESSION 15 (2026-08-25, escalation modality; owner directive: run the SOLVER first)

Chassis re-measured at the start and the end of the session with the banked
candidate body applied to `src/code6cac_b.c`:
`{"score": 2, "target_insns": 104, "build_insns": 104, "rules_dropped": 0}` —
floor unchanged at 2, 0 regfix/asmfix rules on main.

This is the first session to execute the owner directive of 2026-08-24 to run the
solver suite (`tools/ra_solver`) before any further re-grind. The frontier that
sessions 12-14 left ("re-attempt is gated on NEW TOOLING: a forward inversion of
the mapping from C dataflow to the allocno conflict graph; plausible substrate
tools/ra_solver, which does not invert") is now RESOLVED: `inverse.py`,
`inverse_compose.py` and `sweep.py` exist and were run on this function for the
first time. Four results.

### 1. The residual is confirmed to be exactly ONE register substitution
`goal_from_tgt.py goal code6cac_b func_8002EA24 --show` aligns our cheat-stripped
object against `build/src/code6cac_b.o` (which still carries the INCLUDE_ASM
target bytes) and reports:

    align ours->tgt: |A|=104 |B|=104 {equal: 102, replace: 2}
    register substitutions:  $a0 -> $v0   x2
      ours[31] slt a0,a1,t1   tgt[31] slt v0,a1,t1
      ours[32] bnez a0,@      tgt[32] bnez v0,@

Attribution is AMBIGUOUS at register level (three pseudos hold $a0: 97, 122, 126),
which is why the work below was done on the PLAIN control (no L3) instead: on
that body the allocation matches target on EVERY allocno except pseudo 103
(`neg_threshold`), which takes $a0 where target takes $t1. The plain control is
therefore the honest statement of the residual: **one allocno, one register.**

Note for whoever runs the solver next on an asm-until-matched function:
`inverse_compose.py classify` and `goal_from_asm.py` are UNSOUND on this chassis
— `mkasm_honest.sh` builds its "target" stream from `src/<stem>.c` plus the
regfix/asmfix stages, which for an INCLUDE_ASM function with zero rules means it
compares our own build against our own build minus the cheat-stripped GTE
islands. Run this session it reported a fictitious `FIRST DIVERGENCE: PRE-RA`
with a 93-vs-105 insn gap. `goal_from_tgt.py`, which compares OBJECTS
(`tmp/sandbox/<func>/<stem>.o` vs `build/src/<stem>.o`), is the correct entry
point and gave the exact 104/104 alignment above.

### 2. `inverse.py` returns a FALSE NEGATIVE here — a tooling defect, recorded
`inverse.py global <plain model> --goal {"103": 9} --depth 2` reports
`NEGATIVE RESULT: no perturbation of any modelled input, up to depth 2, reaches
the target assignment` and advises "the next move is INSTRUMENTATION, not another
spelling search". That verdict is WRONG, and the counterexample is this
function own banked candidate. The cause is structural, at
`tools/ra_solver/inverse.py:182-190`: every atom is generated over

    focus = goal pseudos + {q : q conflicts with a goal pseudo}

so `CONFLICT_ADD` edges can only ever be proposed between the goal pseudo and
someone it ALREADY conflicts with. On the plain control, 103 conflicts with
{72, 74, 75, 96, 100}; the edge that actually works — 103 to 97 — is exactly the
one the focus set cannot name, and 97 is the allocno L3 makes conflict with 103.
**Consequence for other functions: an `inverse.py` NEGATIVE is only a negative
over already-conflicting pairs; it must not be read as "no conflict lever
exists".** (Recorded here rather than fixed: `tools/` is outside a grind
session writable surface.)

### 3. Unfocused depth-1 sweep — the modelled input space is now EXHAUSTED
`tmp/grind/func_8002EA24/s15/depth1_sweep.py` re-runs the search without the
focus restriction: for EVERY pseudo in the function (43 of them, not 6), every
`nrefs` from 1 to +24, every live length from -16 to +32, `calls_crossed`, every
conflict edge in the whole function, and every hard-register preference 2..25 —
one atom at a time, against the validated `simulate.py` model of `global.c`.

    atoms reaching 103->$t1: 3
      CLEAN (no collateral allocno change): 3
         conflict 97<->103
         conflict 103<->97          (the same edge, both orientations)
         pseudo 103: prefs []->[9]
      with collateral: 0

So over the whole modelled input space there are exactly **two** distinct
perturbations that reach target, and both are clean:

* **Route A — an own hard-register preference for $t1 on 103.** FORECLOSED with
  a named mechanism, and the `inverse.py` appearability gate states it:
  "$t1 never appears as a hard reg in this function pre-RA RTL, so
  `global.c set_preference` can never record a preference for it." Checked
  against target this session: the only $t1 occurrences in
  `asm/funcs/func_8002EA24.s` are `negu $t1,$a2` and the two `slt`s that read it
  — i.e. the ALLOCATED result, not a pre-RA hard reg — and target two GTE
  islands use $t4 (`addu $t4,$v0,$zero`), exactly the register our authorized
  island wording already pins. There is no fidelity question hiding here: no C
  source and no island re-spelling puts a hard $t1 in the pre-RA RTL.
* **Route B — the conflict edge 97 to 103** (a0_var live across the range chain).

### 4. Route B is closed in C, now by a MECHANISM rather than by enumeration
The model says the edge is clean even when it comes with everything a real
carrier brings: `carrier_model.py` adds the edge together with conflicts against
the entire window {72,74,75,96,100,102}, live-length growth to +32 and ref growth
to +4 — 103 still lands in $t1 with zero collateral. So the model does NOT
forbid a carrier. What forbids it is the RTL:

* **Backward extension (a0_var born earlier).** a0_var can only be live before
  the chain if it is DEFINED before the chain. Every value computed before the
  chain in this function is ALREADY an allocno, so re-using one as the a0_var
  pre-chain value creates the edge and simultaneously DELETES an allocno,
  vacating the register target needs elsewhere. Measured this session on the one
  carrier sessions 8-9 never tried — `z`, the rotated Z, the only value loaded
  BETWEEN the two range tests: the edge IS created (`97 conflicts:` gains 103)
  but the 97 refs go 13->18 and its live length 32->39, which re-ranks it from
  THIRD to FIRST in the allocation order; max_y (100) falls $a1->$v1, y (102)
  rises $v1->$a1, and 103 takes the vacated $a1. **score 15 at 104 insns**
  (`rejected/zcarrier-merges-allocno-96-reranks-97-score15.c`). Same shape as the
  session-9 `v2` carrier — with `z` measured, the family is closed generally: a
  value that is not already an allocno is a new computation, i.e. extra
  instructions.
* **Forward extension (neg_threshold living longer).** The edge is symmetric, so
  the dual is to move the a0_var BIRTH before the last read of neg_threshold.
  That is semantics-preserving here (both remaining early-return tests are pure
  and return the same value), and it was measured: computing the sum of squares
  between the Z load and the Z range test keeps 104 instructions but emits the
  two mult/mflo pairs and the add BEFORE the Z test, where target emits them
  after both tests — **score 21**
  (`rejected/ztest-after-sum-hoists-mults-score21.c`). This is the `accearly`
  wall reached from the opposite direction. There is no honest later USE of
  -threshold to extend 103 with instead: the tail tests mention only min_y,
  max_y, y and a0_var.

### Endgame-lock gates, re-evaluated on the current chassis
* Gate (a) canonical-asm: `scan_hand_coded.py --single func_8002EA24` gives
  `tier=TIGHT_C score=3/8`, signals S3/S4/S5 only, **no S1/S2/S6**. FAILS.
* Gate (b) SOTN precedent for a closing construct: there is no closing construct
  to cite a precedent FOR — both routes above are closed by named GCC mechanisms,
  not by a missing spelling. The session-13 construct-index census
  (conflict / allocno / find_reg / global_alloc) returned zero hits and has not
  changed. FAILS.

No decision packet was filed. Under the owner second ruling of 2026-08-24
(`.claude/rules/escalation-not-parked.md`) the previously-filed
"REFUSED / OWNER-ACCEPTED INCOMPLETE" shape is in the AUTO-REJECT class (a YES
would lower a standard), and no fidelity / routing / provenance question with a
decidable answer survives this session measurements — the one candidate (island
register fidelity, i.e. "could the original island have put a hard $t1 in the
pre-RA RTL and opened route A?") was CHECKED against the target bytes and
answered NO. The residual therefore stays ACTIVE under standing policy, with the
search space narrowed to the two model-external mechanisms in hypotheses.md H9.

- [s15] Chassis re-measured at session start and end with the banked candidate applied: score 2, target_insns 104, build_insns 104, rules_dropped 0 -- floor unchanged, zero regfix/asmfix rules on main. src/code6cac_b.c reverted to INCLUDE_ASM at the end of the session; the candidate lives only in memory/grind/func_8002EA24/candidate.c.

- [s15] First execution of the owner directive attached to this queue item (2026-08-24: run the solver modality before deeper re-grind). It also discharges the session-12..14 frontier item, which said re-attempt was gated on 'new tooling: a forward inversion of C dataflow to the allocno conflict graph' -- tools/ra_solver now has inverse.py / inverse_compose.py / sweep.py and they were run here.

- [s15] The residual is one register substitution, not two: goal_from_tgt.py reports align 104/104 {equal 102, replace 2}, $a0 -> $v0 on `slt a0,a1,t1` / `bnez a0`. Register-level attribution is ambiguous on the banked body (pseudos 97, 122, 126 all hold $a0), which is why the analysis was done on the plain control, where the allocation equals target on every allocno except pseudo 103.

- [s15] Exhaustive in-model result: over 43 pseudos and the full modelled input space at depth 1, exactly 3 atoms reach 103->$t1 and all 3 are collateral-free; they are two distinct routes (an own $t1 preference on 103; the conflict edge 97<->103).

- [s15] Route A is FORECLOSED by set_preference appearability -- $t1 never appears as a hard reg in a C compile of this leaf. Target's own $t1 is the allocated `negu $t1,$a2`, and target's two GTE islands use $t4, matching our authorized island wording, so no island re-spelling opens the route.

- [s15] Route B closed with a general mechanism rather than by enumeration: every value computed before the range chain is already an allocno, so any carrier creates the edge AND deletes an allocno. Measured on the one carrier sessions 8-9 never tried (z, the rotated Z, the only value loaded between the two range tests): score 15 at 104 insns, 97 re-ranks THIRD->FIRST, max_y and y swap registers, 103 takes the vacated $a1.

- [s15] The symmetric forward extension (move a0_var's birth before neg_threshold's last read, semantics-preserving because both early-return tests are pure and return 0) measures 21 at 104 insns -- the mult/mflo pairs hoist above the Z range test. There is no honest later USE of -threshold: the tail tests mention only min_y, max_y, y and a0_var.

- [s15] reload_sim.py: exactly one RETRY in the whole function, pseudo 117 (MD/$lo quantity) -> $v1, which both builds agree on. 103 never enters retry_global_alloc, so the pre-reload allocation is final for it and the H9a/H9b closure stands unqualified.

- [s15] TOOLING DEFECT worth propagating: an inverse.py NEGATIVE is only a negative over already-conflicting pairs (focus set, inverse.py:182-190) -- it returned a confident negative on a goal for which this very ledger banks a working lever. And inverse_compose.py classify / goal_from_asm.py are unsound for asm-until-matched functions because mkasm_honest.sh builds its 'target' from src + regfix/asmfix; use goal_from_tgt.py (object-level) instead. tools/ is outside a grind session's writable surface, so this is recorded, not fixed.

- [s15] Endgame-lock gates re-evaluated on the current chassis and both still FAIL: scan_hand_coded.py --single func_8002EA24 -> tier=TIGHT_C score=3/8 with S3/S4/S5 only and no S1/S2/S6; and there is no closing construct to cite a SOTN precedent FOR, since both routes are closed by named GCC mechanisms rather than by a missing spelling.

- [s15] No decision packet was filed, deliberately. Under the owner's second 2026-08-24 ruling (.claude/rules/escalation-not-parked.md) the previously-filed 'REFUSED / OWNER-ACCEPTED INCOMPLETE' shape is in the AUTO-REJECT class -- its YES would lower a standard -- and the one fidelity/provenance question this session could have posed (could the original GTE island have put a hard $t1 in the pre-RA RTL and opened route A?) was answered NO against target's own bytes. Per that ruling the residual stays ACTIVE under standing policy.

- [s15] 45 rejected forms now banked in memory/grind/func_8002EA24/rejected/ (2 added this session).

## SESSION 16 (escalation modality, owner directive: solver-first, 2026-08-25)

**Chassis re-measured first.** Banked candidate body (candidate.c lines 229-373)
pasted over `src/code6cac_b.c:905` (`INCLUDE_ASM("asm/funcs", func_8002EA24);`),
`sandbox func_8002EA24 --disable all` -> **score 2, target_insns 104,
build_insns 104, rules_dropped 0, cheat_asm_stripped 47**. Unchanged from s13/s14/s15.
The whole ledger is valid on the current chassis.

**Owner directive executed.** The queue item's 2026-08-25 directive ("run SOLVER
modality first once the func_800645B0 toolkit repair lands") is now executable:
the repair is commit `661dc8dc`. Both halves were run.

### 1. The repaired `classify` is still unsound in the state a grind session uses it

`python3 tools/ra_solver/inverse_compose.py classify code6cac_b func_8002EA24`
with the candidate pasted still prints `PATH: text-stream classifier`,
`honest 93 insns, target 105 insns`, `FIRST DIVERGENCE: PRE-RA`. The repair's
refusal predicate (`_is_include_asm_routed`, inverse_compose.py:137-190) keys on
whether `src/<stem>.c` currently carries the `INCLUDE_ASM` line -- but a grind
session pastes its candidate body into src before running any classifier, which
is the only state where the honest stream means anything. The general form of
the defect: for ANY zero-rule function the src-derived `<stem>.tgt.s` IS our own
build modulo cheat-asm stripping, so the text path can only ever report the
stripping artifact (here: the two GTE islands, 93 vs 105). `goal_from_tgt.py`
(object-level) remains the correct entry point and aligns 104/104.
Artifact: `tmp/grind/func_8002EA24/s16/classify.txt`.

### 2. Depth-2 solver sweep -- a NEW route (Route C), then killed

`tmp/grind/func_8002EA24/s16/depth2_sweep.py` on the plain control model
(`s15/plain.model.json`, score-3 body = candidate minus L3): 947 atoms, 435,448
admissible pairs, pairs containing a depth-1-reaching atom excluded.

    depth-1 reaching: 3   (conf97<->103 x2 orientations, prefs[103]=[$t1])
    depth-2 reaching pairs (neither atom reaching alone): 142
      CLEAN (no collateral allocno change): 4
        p102:refs3->2      + conf102<->103
        p102:livelen5->9   + conf102<->103
      with collateral: 138 -- every one of them also contains conf102<->103

So **every reaching vector at depth <= 2 requires a new conflict edge from 103
onto 97 or onto 102.** Route C (the 102 edge) is new: 15 sessions of hand and
depth-1 search only ever reached 97.

**Route C measured dead, four bodies (`tmp/grind/func_8002EA24/s16/variant.py`):**

    nol3 (plain control)                        score  3   104 insns
    nol3 + y load before the z test             score 11   102
    L3   + y load before the z test             score 10   103
    L3   + y load before the z LOAD             score 10   103
    nol3 + y load before the z LOAD             score 11   102
    L3 / nol3 + y load above the x range test   score 11   103

**Why, from the extracted model** (`s16/yhoist.model.json` vs
`s15/plain.model.json`, compared by `s16/cmp.py`):

    plain   p102: refs=3 livelen=5  conf=[97,99,100,102]                    prefs=[4]
    yhoist  p102: refs=3 livelen=41 conf=[72,74,75,96,97,99,100,101,102,108,109,117] prefs=[]

    plain   order [101,96,97,100,109,108,72,102,117,103,74,99,75]  103 -> $a0 (4)
    yhoist  order [101,97,96,100,109,108,72,117,103,99,74,75,102]  103 -> $a0 (4), 102 -> $t1 (9)

The C move overshoots the live-length atom by 8x (5->41, the route needs 5->9),
`y` loses its `$a0` preference, `allocno_compare` (live_length in the
denominator) demotes it from 8th to LAST, and it takes target's `$t1` for
itself. And the edge the route needs is **not created at all** -- 103 is absent
from 102's conflict set in the hoisted model, because GCC sinks the load back
past the test it was hoisted over.

**The geometric reason (H10b).** 103 (`neg_threshold`) is born at the top of the
block and dies at the z range test; 102 (`y`) is born ~25 instructions later,
past the whole squared-distance + LZC/GTE reciprocal block. An overlap therefore
costs >= ~25 live-length units on one side; the route needs +4. The attainable
values of `y`'s live length are 5 (no overlap) and ~41 (overlap) -- 9 is not on
the menu. Shrinking the gap means moving the mult/mflo + LZC block, which is the
already-banked `ztest-after-sum` family (score 21). Route C is dead for exactly
the reason Route B is: **the edge is obtainable only by paying an allocno.**

Banked: `memory/grind/func_8002EA24/rejected/yhoist-route-c-edge-never-created-score10.c`.

### 3. Depth-3 closure over the non-foreclosed atom families

`tmp/grind/func_8002EA24/s16/depth3_sweep.py`: a coarsened 461-atom grid that
EXCLUDES both foreclosed families (any new conflict edge touching 103; any own
hard-reg preference on 103), swept at depths 1, 2 and 3.
Log: `tmp/grind/func_8002EA24/s16/depth3_no103.txt`.

    depth 1 cumulative evaluated=461         hits=0
    depth 2 cumulative evaluated=104,722     hits=0
    depth 3 cumulative evaluated=15,525,735  hits=0
    TOTAL vectors evaluated: 15,525,735
    reaching vectors with NO 103-edge / NO 103-pref: 0

i.e. **no vector of one, two or three ordinary live-range / reference-count /
preference perturbations anywhere else in the function moves 103 out of `$a0`.**
Every route to the goal inside the model runs through one of the two
RTL-foreclosed atom families, and that is now a closed-form result over
15.5 million vectors rather than an exhaustion argument.

### Standing summary after s16

The modelled input space of `global.c` is now exhausted to depth 2 in full and
to depth 3 outside the two foreclosed atom families. All three routes that reach
the goal are RTL-foreclosed with measurements:

  * **Route A** own `$t1` preference on 103 -- `set_preference` can only record a
    hard register that appears in the pre-RA RTL; `$t1` never does (s15).
  * **Route B** edge 97<->103 -- creating it deletes an allocno (backward) or
    hoists the mult/mflo pairs (forward); measured 15 / 21 (s15).
  * **Route C** edge 102<->103 -- 25-instruction gap, cheapest overlap is +36
    live length against the +4 the route needs; measured 10 / 11 and the edge is
    not even produced (s16, this session).

The one mechanism in the allocation stack that has still never been OBSERVED is
local-alloc's suggested-register pass (`qty_phys_copy_sugg` / `qty_phys_sugg`),
because the `BB2_QTY_DEBUG` hook in `tools/gcc-2.7.2/local-alloc.c` does not dump
the suggestion sets (stated as a known gap in `tools/ra_solver/README.md`).
That is an engine/tools change, outside a grind session's writable surface.

## [s16] H10d -- the score-2 CANDIDATE body's residual is not an allocation question at all, so ra_solver cannot express it; only the plain control's 103 -> $t1 goal is RA-expressible.
- mechanism: ra_solver's simulate.py permutes and renames a FIXED set of allocnos. If our body and target differ in the NUMBER of pseudos, no perturbation of the model's inputs can reach target, however the goal is phrased.
- probe: simulate tmp/grind/func_8002EA24/s15/cand.model.json (the banked score-2 body) and read off 103's assignment (tmp/grind/func_8002EA24/s16/candmodel.py).
- result: the candidate already allocates 103 to hard reg 9 = $t1, i.e. TARGET's choice -- that is exactly what L3 buys. The remaining 2 points are that our body computes the first range test's boolean INTO a0_var (97, $a0) while target computes it into a separate short-lived $v0 temp and still has a0_var in $a0: target has one MORE pseudo than we do, at the same 104 instructions. That is a pseudo-identity (split) difference, not an assignment difference, so it is FORECLOSED to the RA model by construction, and the only RA-expressible framing of this function's residual is the plain control's 103 -> $t1 goal -- which s15 (depth 1) and s16 (depth 2, plus depth 3 outside the foreclosed families) have now closed.
- verdict: CONFIRMED

- [s16] Chassis re-measured this session: banked candidate body applied to src/code6cac_b.c -> sandbox func_8002EA24 --disable all = score 2, target_insns 104, build_insns 104, rules_dropped 0, cheat_asm_stripped 47. Unchanged since session 4; src restored to INCLUDE_ASM afterwards (tree clean, no src modification).

- [s16] Depth-2 unfocused sweep of global.c's modelled inputs (947 atoms, 435,448 admissible pairs): 142 reaching pairs, 4 CLEAN, and EVERY reaching pair contains the new conflict edge 102 (y) <-> 103 (neg_threshold). Combined with s15's depth-1 result, every reaching vector at depth <= 2 requires a new conflict edge from 103 onto 97 or onto 102.

- [s16] Route C (edge 102<->103) measured dead across six bodies: 10 or 11 at 102-103 insns versus the banked 2 at 104. The extracted model shows the C hoist takes y's live length 5 -> 41 (route needs 5 -> 9), strips y's $a0 preference, demotes it from 8th to LAST in the allocation order where it takes target's $t1 for itself, and does not create the 102<->103 edge at all because GCC sinks the load back past the test.

- [s16] Geometric closure: 103 dies at the z range test and y is born ~25 instructions later past the squared-distance + LZC/GTE block, so the cheapest attainable overlap costs ~+36 live-length units against the +4 the modelled route needs. Same structural reason Route B is foreclosed: the edge is obtainable only by paying an allocno.

- [s16] Depth-3 sweep excluding both foreclosed atom families (461 atoms): zero reaching vectors at depths 1 and 2 and at depth 3 (15,525,735 vectors evaluated in total); log at tmp/grind/func_8002EA24/s16/depth3_no103.txt.

- [s16] TOOLING: the 661dc8dc classify repair is defeated by the candidate-pasted state every grind session uses. The predicate should key on 'zero regfix/asmfix rules' (or on the queue/canonical routing), not on src's transient INCLUDE_ASM line -- for any zero-rule function the src-derived target stream is our own build.

- [s16] Endgame-lock gate (a): scan_hand_coded --single func_8002EA24 = tier=TIGHT_C score=3/8, all three STRONG signals clear -- FAIL (unchanged from 2026-07-30, 2026-08-20, s13, s14).

- [s16] Endgame-lock gate (b): no closing construct exists to cite a precedent for; three censuses (decomp.me x2 in s9, sotn-construct-index.md in s13/s14) returned NEGATIVE -- FAIL.

- [s16] Decision packet filed at docs/grind/decisions.md:11688 asking one decidable tooling question (extend BB2_QTY_DEBUG to dump qty_phys_copy_sugg / qty_phys_sugg + qty_size, and repair inverse_compose.py's routing predicate). No rule, family, grant or standard is requested; nothing in it lowers a standard.

- [s16] Banked this session: memory/grind/func_8002EA24/rejected/yhoist-route-c-edge-never-created-score10.c (47 rejected bodies total); candidate.c body UNCHANGED with a session-16 header note.

- [s16] Gate (a) re-run first-hand THIS session, not quoted from the ledger: `python3 tools/scan_hand_coded.py --single func_8002EA24` -> `HAND_CODED: tier=TIGHT_C score=3/8 (func_8002EA24, 110 insns)`; S3/S4/S5 set, S1/S2/S6/S7/S8 clear.

- [s16] The candidate body already allocates pseudo 103 to $t1 (target's choice); its score-2 residual is a pseudo COUNT difference (target splits the range-test boolean into its own $v0 temp), which the RA model cannot express at all -- so the plain control's 103 -> $t1 goal is the only RA-expressible framing, and it is now closed to depth 2 in full and depth 3 outside the foreclosed families.

## [s17] 2026-08-30 — escalation modality; owner ruling 1 (2026-08-30) executed

- CHASSIS: banked `candidate.c` body pasted over `src/code6cac_b.c:1062`
  (`INCLUDE_ASM("asm/funcs", func_8002EA24);`) → `sandbox func_8002EA24 --disable all` =
  **score 2, target_insns 104, build_insns 104, rules_dropped 0, cheat_asm_stripped 46**.
  Identical to the ledger floor; src restored to INCLUDE_ASM before the session ended.
- OWNER RULING 1(a) IS ALREADY BUILT. The 2026-08-30 escalation-batch ruling granted the
  local-alloc suggested-register cc1 instrumentation "awaiting execution". It does not
  await anything: `tools/gcc-2.7.2/local-alloc.c` carries the `BB2_SUGG_DEBUG` block
  (dumping `qty_size`, `qty_min_class`, `qty_alternate_class`, `ncopysugg`, `nsugg`,
  `copysugg[]`, `sugg[]` per qty, printed before the suggested pass so find_free_reg's
  retry cannot clear `qty_phys_num_copy_sugg` first), the built cc1 honours it, and
  `local_extract.py --suggest` / `inverse.py --sugg` already consume it (landed
  `70d6c905`, "ra_solver — the suggested-register pass, modelled EXACTLY (Phase 7)").
  The ledger's standing frontier line ("local_alloc.py reports but cannot SCORE `sugg`
  rows because the hook does not dump the suggestion sets") is STALE and is retired here.
- MEASUREMENT: `local_extract.py code6cac_b --suggest` → 53 functions / 977 qty rows;
  suggestion table 49 functions / 970 qtys / 33 carrying a suggestion. For func_8002EA24:
  26 quantities over blocks 0,1,3,4,6,8,9,12,16,18; **exactly one suggestion in the entire
  function** — blk 0 qty 0 (pseudo 73, birth 4 death 32, refs 4) `ncopysugg=1
  copysugg=[5] nsugg=0`, i.e. a copy suggestion for $a1 on a vector/GTE-prologue value.
  Main pass assignments for the whole function are only $v0, $v1 and $a1.
- CONSEQUENCE: pseudos 102 (`y`) and 103 (`neg_threshold`) are cross-block and therefore
  never become local-alloc quantities; no suggestion set for either can exist, and
  local-alloc never occupies $a0 or $t1 anywhere in the function. The one remaining
  local→global coupling, global.c's `local_reg_n_refs` kick-out (global.c:1198-1250), is
  gated on `best_reg < 0 && !retrying` and so cannot fire for an allocno that is placed —
  103 is placed in every measured build. The suggested-register mechanism is INERT here.
- REPAIRED CLASSIFY (ruling 1(b), commit `1ce408a4`): `inverse_compose.py classify` now
  refuses the text path for this zero-rule function and redirects to `goal_from_tgt.py`.
  Object-level verdict: `ours 104 / target 104`, `FIRST DIVERGENCE: RA`, residual
  `$a0 -> $v0 x2`. With the freshly extracted model the attribution is **AMBIGUOUS** (three
  pseudos hold $a0: 97, 122, 126) and the emitted goal is **empty** — there is no
  well-formed target assignment to invert, because the difference is a pseudo SPLIT
  (target has one more pseudo than we do), not a seat swap. This reproduces s16's hand
  reading from the repaired tooling.
- GATES (both re-evaluated on today's facts, both FAIL):
  (a) `python3 tools/scan_hand_coded.py --single func_8002EA24` → `tier=TIGHT_C score=3/8`;
      only the weak signals S3/S4/S5 fire; all three STRONG signals (S1 multu pacing,
      S2 empty branch, S6 BIOS jumptable) clear. Same reading as 2026-07-30, 2026-08-20,
      s13, s14 and s16. **FAIL.**
  (b) SOTN-master precedent: there is no closing construct to cite a precedent FOR (the
      banked body is ordinary C plus the two authorized GTE islands). The construct-index
      census run this session for the newly-touched mechanism —
      `sugg`, `local_alloc`, `qty_phys`, `local-alloc`, `suggested` against
      `docs/reference/sotn-construct-index.md` — returns **0 hits each**, joining the
      negative censuses of s9 (decomp.me ×2) and s13/s14 (conflict / allocno / find_reg /
      global_alloc). A negative census is a failed gate, not an open question. **FAIL.**
- ARTIFACTS: `tmp/grind/func_8002EA24/s17/scan.txt`, `classify.txt`, `classify_obj.txt`,
  `cur.model.json`, `apply.py`; `tmp/ra_solver_work/code6cac_b.sugg.json`,
  `tmp/ra_solver_work/code6cac_b.local.json`.

- [s17] CHASSIS (this session): banked memory/grind/func_8002EA24/candidate.c body applied over src/code6cac_b.c:1062 -> `sandbox func_8002EA24 --disable all` = score 2, target_insns 104, build_insns 104, rules_dropped 0, cheat_asm_stripped 46. src restored to INCLUDE_ASM before the session ended; working tree left clean apart from the ledger/docs appends.

- [s17] Owner ruling 1(a) (2026-08-30 escalation-batch) is ALREADY BUILT, not awaiting execution: tools/gcc-2.7.2/local-alloc.c carries the BB2_SUGG_DEBUG block (per-qty qty_size / minclass / altclass / ncopysugg / nsugg / copysugg[] / sugg[], printed BEFORE the suggested pass so find_free_reg's retry cannot clear qty_phys_num_copy_sugg first); the built cc1 honours it; local_extract.py --suggest and inverse.py --sugg already consume it (landed 70d6c905). The ledger's standing frontier line claiming the hook 'does not dump the suggestion sets' is STALE and is retired here.

- [s17] func_8002EA24 has 26 local-alloc quantities and exactly ONE suggestion in the whole function (blk 0 qty 0, pseudo 73, copysugg=[$a1]). Neither residual pseudo (102 y, 103 neg_threshold) is a local-alloc quantity -- both are cross-block. Local-alloc's main pass assigns only $v0, $v1 and $a1 here, so it never occupies either contested register.

- [s17] global.c's local_reg_n_refs kick-out (tools/gcc-2.7.2/global.c:1198-1250) is gated on `best_reg < 0 && !retrying` and therefore cannot fire for an allocno that is successfully placed; 103 is placed in every measured build. The local->global coupling is closed by reading the pass, not by search.

- [s17] The allocation stack is now observed END TO END for this function: local-alloc's suggested pass (s17, inert), local-alloc's main pass (s17, three registers, neither of them the contested pair), global.c's allocno ordering + first-fit + prune_preferences (s3-s11, s15 depth 1, s16 depths 2 and 3 -- 15,525,735 vectors, zero reaching), and reload (0 spills, S3 in scan_hand_coded). No un-instrumented mechanism remains.

- [s17] Repaired classify (ruling 1(b), commit 1ce408a4): FIRST DIVERGENCE: RA at 104/104 with residual `$a0 -> $v0 x2`; model attribution AMBIGUOUS (pseudos 97/122/126 hold $a0) and goal EMPTY. The banked body's residual is a pseudo split (target has one more pseudo), so the only RA-expressible framing this function has is the plain control's `103: $a0 -> $t1` goal -- closed by s15/s16 at depths 1, 2 and 3.

- [s17] GATE (a) canonical-asm: `python3 tools/scan_hand_coded.py --single func_8002EA24` -> tier=TIGHT_C score=3/8; only the weak signals fire (S3 no spills 110 insns/0 spills/16 distinct regs, S4 front loads 5-in-8 @ insn 1, S5 cluster -- one approx-sibling func_8002D320 jaccard 0.60); all three STRONG signals clear (S1 multu pacing, S2 empty branch, S6 BIOS jumptable). Identical reading to 2026-07-30, 2026-08-20, s13, s14 and s16. FAIL.

- [s17] GATE (b) SOTN-master precedent: there is no closing construct to cite a precedent FOR (the banked body is ordinary C plus the two already-authorized GTE islands). The census run this session over docs/reference/sotn-construct-index.md for the newly-touched mechanism (sugg / local_alloc / qty_phys / local-alloc / suggested) returns 0 hits each, joining s9's two decomp.me censuses and s13/s14's conflict / allocno / find_reg / global_alloc census. A negative census is a failed gate, not an open question. FAIL.

- [s17] EXHAUSTION of record: 17 sessions; honest floor flat at 2 since session 4; 8 distinct modalities (escalation, forensics, permuter, recon, rederive, structural, synthesis, solver); ~118k + ~84k fresh-seed permuter iterations; 47 disproven bodies banked in memory/grind/func_8002EA24/rejected/. All three model-reaching routes are RTL-foreclosed with measurements: A (own $t1 preference on 103 -- unrepresentable in the pre-RA RTL, s15); B (edge 97<->103 -- 15/21, s15); C (edge 102<->103 -- 25-instruction gap, 10/11, s16). The forward enumeration of a0_var carriers is complete and empty (s9/s11).

- [s17] There is NO cheat on main for this function: 0 regfix/asmfix rules, and src/code6cac_b.c carries INCLUDE_ASM("asm/funcs", func_8002EA24); per asm-until-matched. This is an honest floor-2 residual, not a cheat-held byte match; the retired chassis is preserved at memory/grind/func_8002EA24/retired-chassis-2026-08/rules.txt.

## [s18] forensics -- the owner's 2026-09-01 Ruling A probe, and the expand_preferences donation channel

### The Ruling A probe (directive, executed first): NEGATIVE on all eight bodies
Owner ruling 2026-09-01 (docs/grind/decisions.md:17743, Ruling A) reopened this
function with a named probe: "re-run `s16/depth2_sweep.py` over the other seven
banked bodies (filter AMBIGUOUS via goal_from_tgt.py first)".  Executed for all
EIGHT score-2 bodies (`tmp/grind/func_8002EA24/s18/harvest.sh`: splice ->
`sandbox --disable all` -> `ra_solver/extract.py` -> `goal_from_tgt.py goal
--model`):

| body | sandbox | residual | attribution |
|---|---|---|---|
| memory/grind/func_8002EA24/candidate.c | 2 | ours[31] `slt a0,a1,t1` / ours[32] `bnez a0` vs tgt `$v0` | AMBIGUOUS, 3 pseudos hold $a0 -- goal {} |
| s8/v6_neg_inline_no_local | 2 | IDENTICAL (same two indices, same $a0->$v0) | AMBIGUOUS -- goal {} |
| s8/v7_neg_init_after_x_load | 2 | IDENTICAL | AMBIGUOUS -- goal {} |
| s8/v10_nested_range_tests_only | 2 | IDENTICAL | AMBIGUOUS -- goal {} |
| s8/v11_sum_own_local | 2 | IDENTICAL | AMBIGUOUS, 4 pseudos hold $a0 -- goal {} |
| s8/v12_single_four_way_if | 2 | IDENTICAL | AMBIGUOUS -- goal {} |
| s8/v14_split_stage_into_sum | 2 | IDENTICAL | AMBIGUOUS, 4 pseudos -- goal {} |
| s9/v5_vout_dies_in_chain | 2 | IDENTICAL | AMBIGUOUS -- goal {} |
| candidate_alt_L1_via_ylow | 2 | IDENTICAL | AMBIGUOUS -- goal {} |

The frontier's own stated filter ("DISCARD any body whose attribution returns
AMBIGUOUS / goal {}") therefore discards every single one: the depth-2 sweep is
UNRUNNABLE on all of them, and the s17 caveat generalises from the banked body
to the whole bank.  The directive is executed and its hypothesis is KILLED.
The stronger fact the sweep produced: **all nine bodies have the byte-identical
residual at the same two instruction indices** -- the score-2 plateau is one
single divergence, not eight body-specific ones.  Artifacts:
`tmp/grind/func_8002EA24/s18/*.goal.txt`, `*.model.json`.

### A defect in the s15/s16 solver sweeps (forensics)
`simulate.py:161` builds prune_preferences' someone_prefers accumulation from
`self.m["full_prefs"]`:

    pruned_full = full_prefs_in.get(a, prefs[a]) - temp_excl

`full_prefs` carries an entry for EVERY allocno in this model (most of them
empty lists), so the `.get` never falls back and a `prefs` OVERRIDE IS
DISCARDED for someone_prefers.  Every `pN:prefs[...]->[R]` atom in the s15
depth-1 and s16 depth-2 sweeps therefore exercised only the allocno's own
upgrade path; the DONATION half of a preference -- global.c:928,
`regs_someone_prefers[allocno] |= hard_reg_preferences[b]` for lower-priority
conflicting b -- was never simulated.  In real GCC both halves read the same
`hard_reg_preferences` array and cannot be separated.  A second, independent
gap: the atom generator only ever set prefs to a SINGLETON `[r]`, so "ADD $a0
to an allocno that already prefers its own argument register" was not in the
atom alphabet at all.  Repaired sweep + probes:
`tmp/grind/func_8002EA24/s18/sweep_fixedprefs.py`, `pref_probe.py`,
`donor_probe.py`, `symmetric_probe.py`, `combined_probe.py`.

### What the repaired model says (measured, plain control)
Donation only works from an allocno that is LOWER priority than 103 and
CONFLICTS with 103; in the order `101 96 97 100 109 108 72 102 117 103 74 99 75`
that is exactly {74 threshold, 99 min_y (needs an added conflict edge), 75
r_sq}.  Donors are the allocnos already carrying $a0: {97 a0_var, 102 y}.
Modelling expand_preferences' SYMMETRIC IOR (both directions, global.c:863-866):

  * 74 <- 97  : 103 -> $t1, **zero collateral**  (and `! CONFLICTP` holds both ways)
  * 75 <- 97  : 103 -> $t1, zero collateral, but 75 and 97 CONFLICT -> the pass skips it
  * 74 <- 102 : 103 -> $t1 but y is handed $a2 and min_y $v1  (this is the s11 failure)
  * 75 <- 102 : 103 -> $t1 but y is handed $a3 and min_y $v1
  * 99 <- any : needs an added 99<->103 conflict edge; s11 measured that edit demotes 99 to last (score 13)

So **74 <- 97 is the unique clean donation vector**, and s11's "74/75 -- they
work but the symmetric IOR hands `y` their argument register" was measured with
donor 102 only; donor 97 was never paired.

### Three C spellings built, and the first chain-clean body in 18 sessions
All three delete the L3 staged boolean (they do not need it) and all three keep
104 instructions.

| variant | spelling | sandbox | residual |
|---|---|---|---|
| `s18/v0_plain_control.c` | the banked body minus L3 | 3 | insns 30/31/39: neg_threshold in $a0, target $t1 |
| `s18/v1_threshold_reuse_tailsum.c` | `threshold = y + a0_var;` for the tail sum | 6 | chain CLEAN; tail poisoned exactly as modelled (y -> $a2, min_y -> $v1) -- the 74<->102 leg fires alongside 74<->97 |
| `s18/v3_threshold_holds_sumsq.c` | `threshold = x*x + z*z; a0_var = r_sq - threshold;` | 3 | chain CLEAN; the whole sum/remainder chain sits in $a2 |
| `s18/v4_threshold_holds_zsq.c` | `threshold = z*z; a0_var = x*x + threshold;` | 4 | chain CLEAN; both mflo operands displaced |
| `s18/v5_threshold_holds_xsq.c` | `threshold = x*x; a0_var = threshold + z*z;` | **2** | chain CLEAN; ONLY `mflo a2`/`addu a0,a2,v1` vs target's `mflo v0`/`addu a0,v0,v1` |

v5 is banked as `memory/grind/func_8002EA24/candidate_alt_s18_pref_donation.c`.
It ties the floor (2) but it is a genuinely new chassis: for the first time the
range-test chain -- target insns 30/31/39, the divergence every one of the nine
banked bodies carries -- MATCHES, and it matches with the L3 FAKE deleted.  The
residual has moved to the sum of squares: the x*x product lands in 74's own
argument register $a2, which target leaves dead from the 4th range test onward.
Every poisonous expand_preferences leg in v5 is blocked by an EXISTING conflict
(74<->100 and 74<->96 are both in 74's conflict list; 75<->97 is in 97's), so
the 74<->97 leg is the only one that fires -- the shielding is by construction,
not by luck.

### Chassis
HEAD floor re-measured this session with the banked candidate applied:
**2, 104/104 insns**, disassembly attribution unchanged from s16/s17.  src/ was
restored to HEAD (`git checkout -- src/code6cac_b.c`) at the end of every probe
batch; the tree is clean.

- [s18] CHASSIS: HEAD floor re-measured this session with the banked candidate applied to src/code6cac_b.c -- `sandbox func_8002EA24 --disable all` = 2, 104/104 insns, residual `ours[31] slt a0,a1,t1` / `ours[32] bnez a0` vs target's $v0. Unchanged from s16/s17.

- [s18] All NINE banked score-2 bodies (candidate.c, s8/v6, s8/v7, s8/v10, s8/v11, s8/v12, s8/v14, s9/v5, candidate_alt_L1_via_ylow) measure 2 at 104/104 and share the BYTE-IDENTICAL residual at the same two instruction indices, every one attributed AMBIGUOUS with goal {}. The score-2 plateau is ONE divergence, not nine body-specific ones.

- [s18] tools/ra_solver/simulate.py:161 discards a `prefs` override when computing someone_prefers (full_prefs has an entry for every allocno, so the .get fallback never fires). This is a live tool defect that silently under-reports every sweep run through simulate.py on any function whose model carries full_prefs -- not just this one.

- [s18] The s15/s16 atom generator emits SINGLETON preference sets only, so additive vectors ('keep $a2, add $a0') are outside its alphabet. Both defects together are why 18 sessions of solver work never surfaced the donation channel.

- [s18] expand_preferences (global.c:843-870) is the mechanism, verified in the compiler source in-tree: the IOR is symmetric and gated on `! CONFLICTP` both ways, and it fires on any single_set whose REG_NOTES carry a REG_DEAD for a non-conflicting allocno -- no hard-register copy required.

- [s18] Model verdicts on the symmetric IOR (s18/symmetric_probe.txt): 74<-97 CLEAN (103 -> $t1, zero collateral); 75<-97 clean but CONFLICTP-blocked; 74<-102 and 75<-102 reach $t1 but hand y the parameter's argument register and min_y $v1 -- which is exactly the s11 measured failure, i.e. s11 tested the 74/75 recipients with donor 102 only and never paired donor 97.

- [s18] NEW CHASSIS BANKED: memory/grind/func_8002EA24/candidate_alt_s18_pref_donation.c (= s18/v5) -- sandbox 2, 104 insns, NO L3 staged boolean, chain fully matched. Its residual is `ours[46] mflo a2` / `ours[49] addu a0,a2,v1` against target's `mflo v0` / `addu a0,v0,v1`: the x*x product occupies allocno 74's own argument register $a2, which target leaves dead from the 4th range test onward.

- [s18] The v5 construct (`threshold = max_y * max_y;`) is a borrow of an existing PARAMETER for a second unrelated value = the variable-reuse family; it is banked as EVIDENCE and is NOT submitted, carries no FAKE annotation, and no family claim is made this session because the body does not reach 0.

- [s18] Three disproven spellings banked: rejected/prefdonation-tailsum-y-leg-poisons-tail-score6.c, rejected/prefdonation-threshold-holds-whole-sumsq-score3.c, rejected/prefdonation-threshold-holds-zsq-score4.c.

- [s18] src/code6cac_b.c was restored to HEAD (`git checkout --`) after every probe batch; the only tracked dirt at session end is the ledger files and metrics/events.jsonl.

## [s19] forensics -- the donation axis is structurally closed; v5 stands as the best body

Chassis re-measured at session start with `s18/v5_threshold_holds_xsq.c` applied to
src/code6cac_b.c: `sandbox func_8002EA24 --disable all` = **score 2, build_insns 104,
target_insns 104, rules_dropped 0, cheat_asm_stripped 36**.  The ledger floor of 2 carries
over unchanged; the driver's dispatch note ("measurement unavailable") is superseded by this
measurement.

**Owner directive status.** The 2026-09-01 FORECLOSED-BUCKET REVIEW Ruling A probe for this
function (re-run the depth-2 sweep over the other seven banked score-2 bodies after filtering
AMBIGUOUS) was executed and KILLED by s18 (H18a): all nine banked bodies produce the
byte-identical residual with an empty goal.  s19 therefore worked the frontier s18 left, which
is the directive's own success condition ("any reaching atom voids it" -- none was found).

**1. v5's relocated residual is not an allocation problem (H19a).**  The repaired sweep
(`s18/sweep_fixedprefs.py`, which propagates a preference override into `full_prefs` so the
prune_preferences DONATION half is actually simulated) was pointed at v5's own model with the
hand-resolved goal `74 -> $v0`: 872 atoms at depth 1 -> **0 reaching**; 370,615 pairs at
depth 2 -> **0 reaching** (CLEAN 0, with-collateral 0).  Artifacts
`tmp/grind/func_8002EA24/s19/depth1_v5_goal74v0.txt` and `depth2_v5_goal74v0.txt`.  The
baseline seat vector for v5 is `{101:$v0, 96:$v1, 97:$a0, 100:$a1, 108:unassigned, 72:$t0,
102:$v1, 116:$v1, 103:$t1, 74:$a2, 99:$a2, 75:$a3}` -- **103 (neg_threshold) sits on target's
$t1**, which is what eighteen sessions were chasing, and it is bought at the price of allocno
74 (the `threshold` parameter) holding the `max_y*max_y` product in its own argument register
$a2 where the target holds it in $v0.

**2. WHY the price is structural, read off the RTL (H19b).**  `pwsh tools/grinder/dump.ps1
func_8002EA24` with the plain control body applied, then `s19/scan_flow.py
tmp/grind/func_8002EA24/dumps/code6cac_b.flow 97 74` (log `s19/flow_scan_plain.txt`):

    insn 101  DEAD74  (set (reg:SI 107) (lt:SI (reg/v:SI 74) (reg/v:SI 96)))   ; threshold < z
    insn 118  SET97   (set (reg/v:SI 97) (plus:SI (reg:SI 108) (reg:SI 109)))  ; x*x + z*z
    insn 131  SET97   (set (reg/v:SI 97) (minus:SI (reg/v:SI 75) (reg/v:SI 97)))
    insn 146  SET97   (set (reg/v:SI 97) (zero_extend (subreg:QI (reg:SI 116) 0)))
    insn 189  SET97   (set (reg/v:SI 97) (lshiftrt:SI (reg:SI 126) (reg:SI 128)))
    BOTH (expand_preferences would fire): []

`threshold` (pseudo 74) dies in exactly ONE insn in the whole function, and that insn's
SET_DEST is a fresh boolean pseudo (107), not a0_var (97).  a0_var is set in exactly four
insns, none of which reads threshold.  `global.c:838-870` requires ONE insn that is both a
single_set to 97 and a bearer of REG_DEAD(74); the two sets are disjoint, and the only way to
intersect them is to make threshold an OPERAND of an a0_var-setting statement -- i.e. to make
threshold carry a value.  **The REG_DEAD note and the carried value travel together**, and the
carried value then occupies threshold's argument register.  s18's frontier item 1 asked
whether the note could be obtained without displacing a register; the answer, from the RTL
rather than from another spelling, is NO.

**3. Both escapes from that reading are measured dead.**
  - donor = the product temp instead of the parameter (H19c): allocno 108 ALREADY dies on
    insn 118, whose dest IS 97, so its side-condition is free; it fails only on priority
    (108 outranks 103, and prune_preferences folds only LOWER-priority conflicting allocnos).
    Demoting it by lengthening its live range -- `xsq = max_y*max_y;` hoisted above the z
    range test, ordinary C, no reuse, no FAKE -- measures **9**.
  - reverse direction, single_set to 74 carrying REG_DEAD(97) (H19d): v1's shape plus the
    y-load hoist that would give 74 and 102 the conflict needed to block v1's parasitic leg
    measures **11 at 103 insns** (one instruction short of target).  Independently, the
    reverse direction is structurally impossible to spell cleanly: both tail tests read y AND
    a0_var, so whichever executes second carries both REG_DEAD notes, and y has no third use.
  - carrier choice inside the surviving family (H19e): `threshold` carrying z*z measures 4;
    the source-level operand order of the sum is inert (2, identical to v5).

**4. What is banked.**  v5 is preserved as
`memory/grind/func_8002EA24/candidate_alt_s19_v5_threshold_holds_xsq.c`.  It is NOT promoted
over `candidate.c` (both measure 2) because it trades one annotated construct for another: it
DELETES the L3 staged boolean but ADDS the `threshold` parameter reuse.  Its standing value is
diagnostic -- it is the first body in nineteen sessions whose range-test chain (target insns
30/31/39) matches, so it isolates the remaining divergence to two instructions with a named
cause.  Four new rejected forms banked (54 total).

- [s19] Chassis re-measured this session with the s18 v5 body applied: sandbox func_8002EA24 --disable all = score 2, build_insns 104, target_insns 104, rules_dropped 0, cheat_asm_stripped 36. The floor is unchanged at 2; the dispatch brief's 'measurement unavailable' is superseded.

- [s19] Owner directive (2026-09-01 FORECLOSED-BUCKET REVIEW, Ruling A) was already executed and KILLED by s18 H18a -- all nine banked score-2 bodies produce a byte-identical residual with an empty goal, so the re-run sweep is unrunnable on any of them. s19 worked the frontier that probe left, which is the directive's own success condition; no reaching atom was found, so the directive's outcome stands as 'foreclosure hardened'.

- [s19] global.c:838-870 read in full this session: the IOR is keyed on a REG_DEAD note on a single_set whose SET_DEST has a global allocno, symmetric, gated on !CONFLICTP in both directions, with an extra copy-preference IOR when the dead reg IS the SET_SRC.

- [s19] The .flow dump proves REG_DEAD(74) occurs on exactly ONE insn (101, dest = fresh boolean pseudo 107) and 97 is set on exactly FOUR insns (118, 131, 146, 189), with empty intersection -- so the preference donation is only obtainable by making threshold an operand of an a0_var-setting statement, i.e. by making it carry a value into its own argument register $a2.

- [s19] Model baselines compared this session: plain control {103:$a0, 108:$v0, 109:unassigned}, candidate.c {103:$t1, 108:$v0}, v5 {103:$t1, 108:unassigned, 74:$a2}. The plain body seats the x*x product temp (108) in target's $v0 correctly; v5 buys $t1 for 103 by destroying that temp. The two halves of the match are, on today's evidence, mutually exclusive within the donation family.

- [s19] Four new rejected forms banked (54 total): s19-threshold-holds-zsq-mirror-residual-score4.c, s19-sum-operand-flip-inert-score2.c, s19-xsq-hoist-demotes-product-temp-score9.c, s19-tailsum-plus-yhoist-blocks-both-legs-score11.c.

- [s19] v5 preserved as memory/grind/func_8002EA24/candidate_alt_s19_v5_threshold_holds_xsq.c. NOT promoted over candidate.c (both measure 2): it deletes the L3 staged boolean but adds a `threshold` parameter reuse, so it trades one annotated construct for another. Its value is diagnostic -- first body in 19 sessions whose range-test chain (target insns 30/31/39) matches.

- [s19] src/code6cac_b.c was restored to HEAD at end of session; the working tree carries only memory/ ledger additions and the untracked tmp/ scratch.

## Session 20 (rederive) -- 2026-09-01

Chassis re-measured first: the s18/s19 v5 body (`threshold = max_y*max_y;
a0_var = threshold + z*z;`) applied to `src/code6cac_b.c` measures
`sandbox func_8002EA24 --disable all` = **2 at 104/104 insns**, so the whole
post-s18 ledger carries over unchanged.  The owner's 2026-09-01 Ruling-A named
probe for this function (re-run s16's depth-2 sweep over the other seven banked
score-2 bodies, AMBIGUOUS-filtered) was already executed and KILLED in s18; this
session did not repeat it.

**Rederive sweep on the DONATION chassis (new -- every earlier rederive, s8/s9,
ran on the plain chassis).**  Eight structurally distinct bodies were generated
from the v5 control by `tmp/grind/func_8002EA24/s20/gen.py` and measured with
`s20/sweep.sh` (log `s20/sweep1.txt`):

| body | what changed | score | insns |
|---|---|---|---|
| b0 v5 control | -- | 2 | 104/104 |
| b6 zsq own local | `z*z` hoisted into `s32 zsq` | **2** | 104/104 |
| b8 separate x local | the L2 variable-reuse DELETED (`s32 x`) | **2** | 104/104 |
| b3 threshold carries x | `threshold = max_y; a0_var = threshold*threshold + z*z;` | 3 | 104/104 |
| b1 no L1 | last reject arm back to plain `return 0;` | 5 | 102/104 |
| b9 threshold reused as min_y | tail's min_y carried by the parameter | 5 | 104/104 |
| b10 second product carrier | `threshold = z*z` written after `a0_var = x*x` | 6 | 104/104 |
| b4 min_y born early | `min_y = 0;` hoisted above the range tests | 9 | 105/104 |
| b5 split accumulation | `a0_var = z*z; a0_var += threshold;` | 18 | 104/104 |

Two results matter.

**(1) The donation family is CLOSED, with the mechanism read out of the RA model
rather than inferred.**  `s20/b0_v5_control.model.json` (from
`tools/ra_solver/extract.py`, instrumented-cc1 backed -- `allocdbg` carries the
real per-allocno hardreg/nrefs/livelen/pri) gives, for the carrier allocno 74
(`threshold`):

    hard_conflicts["74"] = [2, 3, 5, 7, 12, 29, 64, 66]      <- 2 is $v0
    copy_prefs["74"]     = [6]                               <- $a2, its own arg reg
    prefs["74"]          = [4, 6]                            <- 4 = the donated $a0
    allocdbg 74: hardreg 6, nrefs 6, livelen 32, pri 3750

Hard register 2 is $v0 and 74 conflicts with it because `threshold` is live
across the four early-return points where $v0 carries the return value; and the
incoming-parameter copy gives 74 a COPY preference for $a2, which GCC 2.7.2's
`find_reg` consults before the general preference set (this is why 74 takes $a2
even though the donated $a0 is free -- 97 holds $a0 but does not conflict with
74).  Since s19 H19b proved the donation only fires when 74 CARRIES a value into
the sum statement, and the target seats that value in $v0 (`mflo v0` /
`addu a0,v0,v1`), the 2-instruction residual is irreducible inside the family
for every possible carried value.  The measured carrier ladder confirms it:
x*x = 2, whole sum = 3, z*z = 4, tail sum = 6, second product = 6.

The complementary half was measured too.  b3 (`threshold = max_y;
a0_var = threshold*threshold + z*z;`) reproduces target's `mult a1,a1`,
`mflo v0` and `addu a0,v0,v1` EXACTLY -- the product seat problem disappears --
but the REG_DEAD(74) note then lands on the multiply insn, whose SET_DEST is the
product pseudo and not 97, so the donation never fires and allocno 103 falls
back to $a0: score 3, residual back at insns 30/31/39 (`negu a0,a2` /
`slt v0,a1,a0` / `slt v0,v1,a0`).  The two halves are mutually exclusive by
construction, not by search.

**(2) The L2 variable-reuse construct is NOT load-bearing on this chassis.**
b8 declares a separate `s32 x` for the rotated-X test value -- deleting the
"one local, two jobs" reuse that every candidate since session 4 carried -- and
still measures 2 at 104/104 with the byte-identical residual.  Banked as
`memory/grind/func_8002EA24/candidate_alt_s20_v5_no_L2_reuse.c`; it is the
score-2 body with the fewest non-ordinary constructs found in 20 sessions
(L3 deleted by s18, L2 deleted here, leaving only L1 -- which b1 re-confirms is
still load-bearing at 5/102 insns without it).

**Complete enumeration of the remaining generators of the one bit that
separates every build from the target** (hard reg 4 entering `used` for allocno
103) is recorded as hypotheses.md [s20] H20b: (i) a hard conflict -- needs
neg_threshold live at function entry, measured s5 at 108 insns; (ii) a conflict
with the $a0 holder 97 -- the L3 family, score 2 with the boolean stranded in
$a0; (iii) `regs_someone_prefers` -- and the model's own conflict/priority
tables show the candidate set is exactly {74, 75} (the only conflicting allocnos
ranked below 103), with 75 permanently unable to receive $a0 because it
conflicts with 97, and 74 closed by (1) above.  Allocno 72 (`obj`) prefers $a0
naturally but has it pruned by its own hard conflict (its pseudo is live at
entry alongside all four argument registers) and outranks 103; allocno 102
(`y`) is the only other allocno carrying 4 in its preferences, does not conflict
with 103, outranks it, and the single edit that fixes both costs instructions
(measured 10/11 in s16/s19).

- [s20] Chassis re-measured at session start: the s18/s19 v5 body applied to src/code6cac_b.c measures `sandbox func_8002EA24 --disable all` = 2 at 104/104 insns, 0 rules - the ledger floor of 2 is current, not stale.

- [s20] The owner's 2026-09-01 Ruling-A named probe for this function (re-run s16's depth-2 sweep over the other seven banked score-2 bodies, AMBIGUOUS-filtered first) was executed and KILLED in session 18; it was not repeated. Session 20 answers the surviving requirement behind it - s18's frontier 'spell any reaching pair that does not touch allocno 74' - NEGATIVELY at the source rather than by another sweep: allocno 74 is the only allocno that can supply the missing bit at zero instruction cost, and it is barred from the target's seat.

- [s20] Model facts (tmp/grind/func_8002EA24/s20/b0_v5_control.model.json, instrumented-cc1 backed): order = [101,96,97,100,108,72,102,116,103,74,99,75]; hard_conflicts[74] = [2,3,5,7,12,29,64,66]; copy_prefs[74] = [6]; prefs[74] = [4,6]; conflicts[75] contains 97; hard_conflicts[72] = [2,3,4,5,6,7,12,29,64,66]; full_prefs[102] = [4]; allocdbg priorities 72:6093, 102:6000, 103:4285, 74:3750, 99:2727, 75:833.

- [s20] s20 sweep scores on the v5 chassis: b0 control 2, b6 zsq-own-local 2, b8 separate-x 2, b3 threshold-carries-x 3, b1 no-L1 5 (102 insns), b9 threshold-as-min_y 5, b10 second-product-carrier 6, b4 min_y-born-early 9 (105 insns), b5 split-accumulation 18.

- [s20] b3 is the first body in 20 sessions to reproduce target's product block exactly (`mult a1,a1` / `mflo v0` / `addu a0,v0,v1`) - it proves the seat is reachable in ordinary C, and that reaching it costs the donation.

- [s20] Six new disproven bodies banked in memory/grind/func_8002EA24/rejected/ (s20-*.c, now 60 files); the fewest-construct score-2 body banked as candidate_alt_s20_v5_no_L2_reuse.c; candidate.c carries a SESSION-20 NOTE pointing at it.


## Session 21 (rederive) -- 2026-09-01 -- **SOLVED: sandbox 0, full build == oracle SHA1**

Chassis re-measured first: s20's b8 body
(`candidate_alt_s20_v5_no_L2_reuse.c`) applied to `src/code6cac_b.c` measures
`sandbox func_8002EA24 --disable all` = **2 at 104/104 insns, 0 rules**, so the
ledger floor of 2 was current, not stale.  The owner's 2026-09-01 FORECLOSED-
BUCKET-REVIEW Ruling-A named probe for this function (re-run s16's depth-2 sweep
over the other seven banked score-2 bodies) was executed and KILLED in s18 and
was not repeated; s21 is the rederive the ladder called for.

**The 2-insn residual is closed, and the last coercion construct in the body is
gone with it.**  Final state: `sandbox --disable all` = **0** at 104/104 insns,
0 rules; `verify-oracle` -> `build_sha1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa`
== `original_sha1_locked`, `build_matches: true`; `canonical func_8002EA24` =
ASM-PARTIAL 8/104 (four "GTE/cop2 op" reasons only).  The adopted body is
`memory/grind/func_8002EA24/candidate.c` and is in `src/code6cac_b.c` now.

### How it was found: read global.c first, then measure

Sessions 15-20 characterised the residual correctly but attacked it through the
model's allocno tables.  s21 went to the compiler source instead and read the
three functions that decide the one bit everyone was chasing (`$a0` entering
`used` for the `neg_threshold` allocno):

* `expand_preferences` (tools/gcc-2.7.2/global.c:828-871) -- for any insn whose
  SET_DEST is an allocno, every REG_DEAD note on that insn names an allocno; if
  the two do NOT conflict, their `hard_reg_preferences` /
  `hard_reg_copy_preferences` / `hard_reg_full_preferences` sets are merged
  **both ways**.  This is the "donation" the s18/s19 ledger named, now pinned to
  source.
* `prune_preferences` (global.c:876-935) -- `regs_someone_prefers[A]` is the
  union of the (already pruned) `hard_reg_full_preferences` of every **lower-
  priority** allocno that CONFLICTS with A, minus A's own full preferences when
  the sizes are equal.
* `find_reg` (global.c:1001) -- `IOR_HARD_REG_SET (used, regs_someone_prefers[allocno])`
  on the non-retrying pass.

That gives the exact recipe for denying `$a0` to `neg_threshold` at zero
instruction cost: find an allocno that (a) conflicts with `neg_threshold`,
(b) ranks below it, and (c) can be handed `$a0` by an `expand_preferences` merge
without occupying a seat the target needs.  s20's answer was the parameter
`threshold` (allocno 74) -- it satisfies (a)-(c) but only while it CARRIES
`x * x`, and carrying it means the first product lands in `threshold`'s own
register `$a2` instead of the target's `$v0`.  That is the whole 2-insn residual
(ours[46] `mflo a2`, ours[49] `addu a0,a2,v1`).

**The zero-cost donor nobody had tried is `r_sq`.**  It is live from function
entry to the subtraction, so it conflicts with `neg_threshold`; it has very few
references over a very long live range, so its priority is far below
`neg_threshold`'s; and it DIES on the insn that computes the remaining distance.
The only thing standing in the way was the spelling: with

    a0_var = r_sq - a0_var;          /* every candidate, sessions 4-20 */

`a0_var` is both source and dest, hence live across its own set insn, hence it
CONFLICTS with `r_sq`, and `expand_preferences` refuses the merge.  Give the sum
its own name and the merge fires:

    sq     = x * x + z * z;          /* plain sum: product -> $v0 */
    if (r_sq < sq) return 0;
    a0_var = r_sq - sq;              /* a0_var is BORN here: no conflict with
                                        r_sq or sq -> preferences merge */

`a0_var` prefers hard reg 4, the merge hands that preference to `r_sq`, and
`prune_preferences` therefore puts 4 into `regs_someone_prefers[neg_threshold]`,
which `find_reg` ORs into `used`.  `neg_threshold` skips `$a0` and takes `$t1`
-- the target's register -- while the sum, having no named carrier, takes `$v0`.
Both halves of the s20 "mutually exclusive by construction" dilemma are
satisfied at once, and the fix costs no instruction because `sq` and `a0_var`
have disjoint live ranges and both land in the registers the target uses.

Predicted from the source read BEFORE any build; confirmed on the first body of
the sweep (r1).

### The L1 borrowed-local return construct is also gone

Every candidate since session 4 ended with `if (y + a0_var < min_y) { z = 0;
return z; }` -- a dead local borrowed to carry the return constant so that
jump.c would not fold the final 0/1 diamond into `slt`/`xori`.  It had no
program-logic account and would have needed a variable-reuse FAKE claim.  Ten
replacements were measured on the new chassis; nine ordinary ones fail (plain
`return 0` 3/102, `goto` label 3/102, result variable 3/102, inverted test
3/102, final sum into the dead `sq` 12/102, all-arms-goto 13/102, mixed exit
forms 4/103), one succeeds and is a borrowed local again (`min_y = 0; return
min_y;`, score 0 -- deliberately NOT used, same family), and the tenth is
ordinary C:

    if (max_y < y - a0_var || y + a0_var < min_y) return 0;
    return 1;

Merging the two vertical bound checks into one short-circuit condition -- the
same `||` shape this function already uses for its two horizontal bound checks
-- keeps the diamond unfolded with no borrowed variable.  Score 0.

### Sweep results (tmp/grind/func_8002EA24/s21/sweep1.txt, sweep2.txt)

| body | change against the s20 b8 control | score | insns |
|---|---|---|---|
| r0 | s20 b8 control (v5 donation, separate x) | 2 | 104/104 |
| **r1** | **plain sum into `sq`; `a0_var = r_sq - sq`** | **0** | **104/104** |
| r2 | three-way split (sq / d / root) | 21 | 105/104 |
| r3 | v5 donation carrier kept PLUS the split | 2 | 104/104 |
| r4 | plain sum, single `a0_var`, no donation | 6 | 104/104 |
| r5 | route (i): `-threshold` above the first GTE island, b8 chassis | 7 | 105/104 |
| r6 | r1 plus route (i) | 3 | 105/104 |
| r7 | r1 with the addends swapped (`z*z + x*x`) | 2 | 104/104 |
| **L1n** | **r1 + the two vertical checks merged into one `||`** | **0** | **104/104** |
| L1h | r1 + `min_y` borrowed instead of `z` | 0 | 104/104 |
| L1a/L1b/L1c/L1f | plain return / goto label / result var / inverted | 3 | 102/104 |
| L1g | final sum into the dead `sq` | 12 | 102/104 |
| L1i, L1j | all-arms-goto, mixed exit forms | 13, 4 | 102, 103 |

- [s21] Chassis re-measured at session start: s20's b8 body measures `sandbox func_8002EA24 --disable all` = 2 at 104/104 insns, 0 rules; the ledger floor of 2 was current.
- [s21] **SOLVED.** `sandbox func_8002EA24 --disable all` = **0** at 104/104 insns, 0 rules, with the adopted body in src/code6cac_b.c; full-build `verify-oracle` returns ok:true with build_sha1 == original_sha1_locked == 62efab4f73f992798c43e8c730aa43baa10bb4fa.
- [s21] `canonical func_8002EA24` on the matched body = ASM-PARTIAL, asm_insns 8 / total 104, regions [[21,23],[26,28],[65,65],[67,67]], reasons exclusively "GTE/cop2 op" (c2, lwc2, mtc2, swc2). The completion bucket (COMPLETED-C vs COMPLETED-INLINE-ASM-CANONICAL) is the operator's call; this session is forbidden to write inline_asm_canonical.txt and did not.
- [s21] Mechanism pinned to compiler source, not inferred: expand_preferences at tools/gcc-2.7.2/global.c:828-871 merges preference sets between an insn's SET_DEST allocno and every non-conflicting REG_DEAD allocno on that insn; prune_preferences at global.c:876-935 builds regs_someone_prefers from LOWER-priority CONFLICTING allocnos' pruned full preferences; find_reg ORs that into `used` at global.c:1001.
- [s21] The 20-session residual was a SPELLING artifact of `a0_var = r_sq - a0_var;`: with the same pseudo as source and dest, a0_var is live across its own set insn and therefore conflicts with r_sq, which blocks the expand_preferences merge. Naming the sum separately (`sq = x*x + z*z; ... a0_var = r_sq - sq;`) makes a0_var BORN at that insn, so it conflicts with neither r_sq nor sq, the merge fires, r_sq inherits the $a0 preference and (being lower-priority and conflicting) denies $a0 to neg_threshold, which takes $t1 as the target does.
- [s21] r_sq is the zero-cost donor the model-side analysis of s15-s20 never enumerated: those sessions searched over allocnos that could be MADE to conflict with 103 and concluded the candidate set was exactly {74, 75}; r_sq already conflicted and already ranked below 103, and only lacked the preference bit, which is a spelling question, not an allocation question.
- [s21] The L1 borrowed-local return construct (`{ z = 0; return z; }`, carried by every candidate since session 4) is NOT required: merging the two vertical bound checks into one `||` condition holds the 0/1 diamond unfolded and measures 0 at 104/104. Nine other ordinary replacements were measured and all cost 1-2 insns (score 3-13 at 102/103 insns).
- [s21] The final body carries ZERO coercion-class constructs: no variable reuse, no dead store, no constant holder, no pointer alias, no volatile, no pad, no FAKE annotation anywhere. The only non-C content is the three canonical GTE cop2 islands, each byte-for-byte the spelling already accepted in-tree (vector/mvmva: the matched twin func_8002D320 at src/code6cac_b.c:869 and :878; LZCS: func_800274BC at src/code6cac_b.c:293).
- [s21] Twelve new disproven bodies banked in memory/grind/func_8002EA24/rejected/ (s21-*.c, now 72 files); self_vet.md written; candidate.c replaced with the score-0 body and a full mechanism header.


## [s21b] H21c (structural) -- the layer-1-banned LZC guard is NOT byte-neutral: it materialises two instructions the target has, so deleting it costs the match. **CONFIRMED**

**Statement.** The construct banned on 2026-09-01 13:04 (`s32 lzcr = 0;
if (a0_var >= 0) { <LZCS island>; lzcr = sp_var; }`) was banned as an
"undisclosed always-true guard ... paired with a dead `lzcr = 0` init".
Both halves of that reading are false on this function: the guard is present in
the ORIGINAL binary, and the negative arm is reachable.

**Mechanism / evidence.**
1. Target bytes. `asm/funcs/func_8002EA24.s:69` -- `/* 1F32C 8002EB2C 08008004 */
   bltz $a0, .L8002EB50` with `/* 1F330 8002EB30 21180000 */ addu $v1, $zero,
   $zero` in its delay slot, i.e. the original code branches around the LZCS
   island (`mtc2 $t4,$30` at 0x8002EB38 / `swc2 $31,0($t4)` at 0x8002EB48 /
   `lw $v1,0($sp)` at 0x8002EB4C) when the operand is negative, and the shift
   computation at `.L8002EB50` (`and $v0,$v1,-2` / `subu $v1,0x16,$v0`) then
   consumes the zero from the delay slot. The guard and the zero are the
   original source's own control flow, not a spelling of ours.
2. Reachability. The enclosing test is UNSIGNED -- `if ((u32)a0_var < 0x400)`
   (`sltiu $v0, $a0, 0x400` at 0x8002EB0C) -- so every NEGATIVE `a0_var` fails
   it and lands in the else arm where the guard lives. `a0_var = r_sq - sq` is
   not provably non-negative either: `sq = x*x + z*z` wraps negative for a large
   `threshold`, in which case `r_sq < sq` is false and the difference is not a
   distance. The guard is an ordinary domain check protecting the GTE LZCS
   operand (LZCS counts leading ONES on a negative operand, so the table index
   would be garbage).

**Probe.** Six bodies generated by `tmp/grind/func_8002EA24/s21/gen2.py` and
`gen3.py` from `memory/grind/func_8002EA24/candidate.c`, each spliced into
`src/code6cac_b.c` and scored with `engine sandbox func_8002EA24 --disable all`
(log `tmp/grind/func_8002EA24/s21/sweep2b.txt`).

**Result.**

| body | score | insns | shape |
|---|---|---|---|
| g0 control | 0 | 104/104 | the banned zero-init + `if (a0_var >= 0)` |
| g1 uncond, shift reads `sp_var` | 7 | **102**/104 | guard deleted |
| g2 uncond, declared `lzcr` | 7 | **102**/104 | guard deleted |
| g3 uncond, inner-scope `lzcr` | 7 | **102**/104 | guard deleted |
| g4 uncond, nested lookup | 7 | **102**/104 | guard deleted |
| g5 two-arm, negative arm first | **0** | 104/104 | `if (a0_var < 0) lzcr = 0; else { island; lzcr = sp_var; }` |
| g8 two-arm, arms swapped | 2 | 106/104 | tested arm carries the island |

All four unconditional spellings lose exactly the two target instructions
(`bltz` + delay-slot zero). The layer-1 remediation option (a) -- "drop the
guard and the init entirely and unconditionally execute the LZCS island" -- is
therefore MEASURED DEAD at score 7.

**Verdict: CONFIRMED.**

## [s21b] H21d -- the "dead `lzcr = 0` init" half of the ban is removable at zero cost by spelling the guard as an ordinary two-arm conditional. **CONFIRMED**

**Statement.** The stylistic objection (an initialiser that looks dead) can be
answered without touching the control flow.

**Probe.** `g5_ifelse_negfirst.c` -- `s32 lzcr; if (a0_var < 0) { lzcr = 0; }
else { <island>; lzcr = sp_var; }` -- and the mirrored `g8_ifelse_posfirst.c`.

**Result.** g5 = **score 0 at 104/104, 0 rules**, and with g5 applied the FULL
build verifies byte-identical: `verify-oracle` -> `build_sha1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa == expected`, `build_matches: true`
(log `tmp/grind/func_8002EA24/s21/verify_oracle_g5.txt`). `canonical
func_8002EA24` -> ASM-PARTIAL, 8/104 insns, GTE/cop2 only. In g5 the zero is a
live assignment on a live path and nothing is initialised twice. g8 (arms
swapped) = 2 at 106 insns: GCC emits the tested arm as the fall-through and the
target falls through into the island, so the arm ORDER is load-bearing.

**Verdict: CONFIRMED.** `memory/grind/func_8002EA24/candidate.c` is now the g5
body (also copied to `candidate_alt_s21b_two_arm_lzc_guard.c`); the previous
banned-shape body is superseded. Rejected forms banked:
`rejected/s21b-lzcs-island-unconditional-drops-target-bltz-score7.c`,
`rejected/s21b-lzc-guard-posarm-first-adds-two-insns-score2.c`.

**Session disposition.** `ruling-request`, not `candidate-ready`: the driver's
`check_banned_constructs` tripwire (tools/grinder/grindlib.py:306) scans the
self-vet's CONSTRUCTS: block for the banned phrase's content words, and an
HONEST declaration of this construct -- which is still an LZC guard around the
LZCS island writing `lzcr` from `sp_var` -- necessarily trips it, so a
candidate-ready would be discarded before any reviewer read the measurements
above. The question put to the Judge is narrow and is recorded in
`tmp/grind/outcome_func_8002EA24.json`.


## [s21c] Session 21, third run (structural) -- 2026-09-01 -- **CANDIDATE SUBMITTED: sandbox 0, full build == oracle SHA1**

**What this run was for.** Session 21's first run reached sandbox 0 and was
layer-1 FAILed on the LZC guard (docs/grind/decisions.md:18266). Its second run
(s21b) measured both halves of that objection, re-spelled the guard as an
ordinary two-arm conditional at zero cost, and returned `ruling-request` rather
than `candidate-ready` because the driver's `check_banned_constructs` tripwire
would have discarded an honest declaration of the construct. **The Judge answered
PASS on 2026-09-01 (docs/grind/decisions.md:18270)**: the ban does not survive,
both `banned_constructs` entries concerned the same guard and both are
superseded, and the two-arm form is ordinary C, not a coercion family. This third
run is the mechanical follow-through: re-apply, re-measure, self-vet, submit.

**Chassis re-measured from a clean HEAD, not inherited.** `src/code6cac_b.c` was
at `INCLUDE_ASM("asm/funcs", func_8002EA24);` at session start (git status clean
apart from `metrics/events.jsonl`). Applying
`memory/grind/func_8002EA24/candidate.c` (== `candidate_alt_s21b_two_arm_lzc_guard.c`,
the g5 body) via `tmp/grind/func_8002EA24/s21/apply.py`:

- `engine sandbox func_8002EA24 --disable all` -> **score 0**, `build_insns` 104,
  `target_insns` 104, `rules_dropped` 0, `cheat_asm_stripped` 36 (the three
  canonical GTE islands), `scorable` true.
- `engine verify-oracle --rebuild --allow-dirty` -> `ok: true`,
  `build_sha1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa`,
  `original_sha1_locked = 62efab4f73f992798c43e8c730aa43baa10bb4fa`,
  `build_matches: true`. Log:
  `tmp/grind/func_8002EA24/s21/verify_oracle_s21c.txt`.
  (The first `verify-oracle --rebuild` was REFUSED with `dirty-build-inputs`;
  that refusal is by design during the edit loop, and `--allow-dirty` is the
  documented flag for the case where the dirty state IS the intended new
  reference. Recorded here so the next session does not mistake it for a
  failure.)

**Both s21b measurements reproduce on a fresh chassis**, so the s21b conclusions
are not chassis-relative artifacts: the score-0 body is the two-arm guard body,
and the ledger's floor for this function is now **0**.

**Self-vet.** `memory/grind/func_8002EA24/self_vet.md` rewritten for this diff:
CONSTRUCTS enumerated (three canonical GTE/cop2 islands with their `vin`/`vout`
operand-address locals; the two-arm LZCS domain guard; the `sq`/`a0_var` split;
`neg_threshold`; the merged `||` vertical bound test; `min_y`/`max_y`), all six
cheat tests answered per construct, two SANCTIONED-FAMILY-CLAIMS blocks (canonical
GTE/cop2 inline asm, scope quoted from `.claude/rules/inline-asm-policy.md`,
precedent `src/code6cac_b.c:869`; the two-arm guard, scope quoted from the Judge's
own ruling text, precedent `docs/grind/decisions.md:18270`), and
ANNOTATION-CONFORMANCE `n/a` on the ground that neither claimed family mandates a
`/* FAKE */` annotation and no coercion-class construct is present.

**T3 disclosure carried forward explicitly** (the thing layer-1 will look for):
the sum-of-squares split was *discovered* by reading `tools/gcc-2.7.2/global.c`
(`expand_preferences` 828-871, `prune_preferences` 876-935, `find_reg` 1001), but
it is not *justified* by that mechanism -- `sq` and `a0_var` are two different
quantities and would be two locals in any straightforward writing of the
algorithm. The allocator account explains why the OLD spelling
(`a0_var = r_sq - a0_var;`, one local for both values) was worse, not why this
one is present.

- [s21c] Floor is **0**. `sandbox func_8002EA24 --disable all` = 0 at 104/104 insns, 0 rules, measured this session from a clean HEAD with the candidate applied to `src/code6cac_b.c`.
- [s21c] Full build verifies byte-identical with the body in place: `verify-oracle --rebuild --allow-dirty` -> ok:true, build_sha1 == original_sha1_locked == 62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches:true.
- [s21c] The 2026-09-01 Judge ruling (docs/grind/decisions.md:18270) LIFTED both `banned_constructs` entries for this function; `state.json` `banned_constructs` is `[]`, so an honest CONSTRUCTS declaration of the two-arm guard no longer trips the driver tripwire that forced s21b to return `ruling-request`.
- [s21c] `verify-oracle --rebuild` refuses with `dirty-build-inputs` whenever a build-input edit is uncommitted; `--allow-dirty` is the documented escape when the dirty tree IS the intended reference. A grind session that needs a full-build proof must pass it.
- [s21c] Both s21b measurements reproduce on a freshly-applied chassis, so the score-0 result is not chassis-relative.
