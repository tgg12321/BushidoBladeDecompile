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
