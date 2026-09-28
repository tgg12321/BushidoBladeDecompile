# func_800290B8 — Ruling 11 submission for `temp` and `temp2` (manual, 2026-09-28)

Ruling: .claude/rules/ordinary-c-judge-decidable.md § Ruling 11 (owner, 2026-09-26; verbatim
record docs/grind/owner-rulings-2026-09-26.md Q1). First submission for this function (it had
no ledger; rotated 2026-09-21 unworked). Each variable is judged on its own; both are argued
below, prong by prong.

Files in this directory:
- `final.c` — the submitted body (byte-identical to what is spliced into src/code6cac_b.c).
- `one-var-per-value-form.c` — the one-variable-per-value spelling (see (C)(1)).
- `dumps.txt` — (D)(1) excerpts for both spellings; `mk.py`, `dumps.sh`, `dumps2.sh`,
  `pmap.py` (pseudo ↔ variable mapping from the dump), `collect.sh` produced them (WSL, repo root: `bash <dir>/dumps.sh && bash <dir>/dumps2.sh &&
  bash <dir>/collect.sh`; the scripts expect the bodies at tmp/f290b8/{final,split}.c).
- `mkperm.sh`, `pc.sh` — the permuter workspace builder and campaign wrapper used for (D)(4).

## The variables and their values

```c
s32 temp;                                  /* function scope */
s32 temp2;                                 /* function scope */
for (i = 1; i < 4; i++) {
    temp = i / 2;                          /* T1 */
    temp2 = i & 1;                         /* K1 */
    n = idx * 4 + temp * 2 + temp2;        /* RT1, RK1 */
    ... }
for (temp = 0 /* T2 */; rec->type != 0; temp++ /* T3 */, rec++) {
    ...
    for (temp2 = 0 /* K2 */; temp2 < 2 /* RK2 */; temp2++ /* K4 */) {
        if (temp2 == 0) /* RK3 */ ...
        ... temp2 = -1; /* K3 */ continue;
        func_80044B30(temp /* RT2 */, func_8002FC80(a, b, c));
```

- `temp` value 1 = {T1}, read by RT1 (the grid row). Value 2 = {T2, T3}, read by RT2 and by T3
  itself (the list index). T2 dominates every value-2 read and follows the first loop, so T1
  reaches no value-2 read; T2/T3 reach no RT1. Two values.
- `temp2` value 1 = {K1}, read by RK1 (the grid column). Value 2 = {K2, K3, K4}, read by RK2,
  RK3 and K4 (the triangle number). K2 dominates every value-2 read and follows the first loop.
  Two values.

## (A) Fresh local, not a borrow
Both are function-scope `s32` locals, declared once. Each has writes in the first loop and in
the record loop, so the function body is the innermost scope enclosing all of its writes. No
other declaration moved or re-scoped. (The one-var form differs only in its own
declarations, per (C)(1): the two comments dropped, `row`/`col` added in the head-loop body,
and value 2 of `temp2` declared in the record-loop body.) No `&temp` / `&temp2`. Not a parameter, global,
`static` or `register` variable.

## (B) Every write is live
(1) T1 is read by RT1 in the same iteration. T2 is read by RT2 on the path first-entry-is-an-
unused-type-2-in-bounds-hit (and by T3 on every path with a second entry). T3 is read by the
next RT2/T3. K1 is read by RK1. K2 is read by RK2 and RK3. K3 is read by K4 (the `continue`
goes to the increment). K4 is read by RK2. No write is dead.
(2) Re-store check (Ruling 5 2(c), 2026-09-26 clarification):
- T1 stores i / 2. i = 1: first write. i = 2: holds 0, stores 1. i = 3: holds 1, stores 1 — a
  re-store on that path only; on the i = 2 path it holds 0 ≠ 1. Record: i = 2 → temp = 0
  (= 1 / 2) before T1 stores 1.
- T2 stores 0. The first loop always runs i = 1..3, so temp holds 3 / 2 = 1 on every incoming
  path; 1 ≠ 0.
- T3 (`temp++`) always changes the value.
- K1 stores i & 1: i = 2 holds 1 stores 0; i = 3 holds 0 stores 1; i = 1 is the first write.
- K2 stores 0. On the first entry temp2 holds 3 & 1 = 1; afterwards the inner loop only leaves
  by falling out at temp2 == 2 (every other exit returns), so it holds 2; 1, 2 ≠ 0.
- K3 stores -1 while temp2 is 0 or 1 (inside the inner loop body); ≠ -1.
- K4 (`temp2++`) always changes the value.

## (C) Same statements; real computations
(1) One-variable-per-value spelling recorded: `one-var-per-value-form.c`. Each value has its
own local declared at the innermost scope enclosing that value's writes:
- `temp` value 1 → `s32 row`, declared in the head loop's body (T1 is there);
- `temp2` value 1 → `s32 col`, declared in the head loop's body (K1 is there);
- `temp` value 2 → `s32 temp` at function scope: T2 is the record loop's `for` initialiser,
  which is a statement of the function body (C89 has no for-scope declarations), and T3 is
  its step, so the function body is the innermost block enclosing both;
- `temp2` value 2 → `s32 temp2` declared at the top of the record loop's body block, which
  encloses K2 (the inner `for` initialiser), K3 and K4.
(Correction 2026-09-28 after layer-2 round 1: the first recorded form left `temp2` value 2 at
function scope. Moving it into the record-loop body gives byte-identical asm — `func.s` cmp
equal — and the same 21/231; the dumps and the third permuter campaign below use the
corrected form.)
(2) Same statement list: the diff is declarations only (the two declaration comments dropped,
`row`/`col` declared in the head-loop body, `temp2` moved into the record-loop body) plus
identifier changes at T1, K1, RT1, RK1. No statement added or removed (the declarations carry
no initialisers; an initialised-declaration variant was also measured, identical asm).
(3) Every value has a real computation in the target's bytes: T1 `sra t0,v0,0x1` (i / 2),
K1 `andi s0,a1,0x1` (i & 1), T3 `addiu t0,t0,0x1` (at 0x80029404, and the reorg copy in the
delay slot at 0x800293DC), K4 `addiu s0,s0,0x1` (0x800293B8). No value is a bare copy or a
constant-only value.

## (D) Allocator-dump proof of necessity

### (D)(1) Dumps — `dumps.txt`
Both spellings compiled with the build compiler `tools/gcc-2.7.2/build/cc1` and the build flags
(`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel
-msoft-float -da`), and with the instrumented cc1 `tools/gcc-2.7.2/cc1`
(`BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=82`), which emits byte-identical asm for each spelling.

**Which pseudo is which is derived, not assumed:** `pmap.py` classifies every write to a
`reg/v` pseudo in each spelling's own f.lreg (constant / self-increment / `i & 1` /
`ashiftrt` = i / 2) and prints the f.flow line and dispositions; `collect.sh` runs it
per spelling. (Correction 2026-09-28 after layer-2 round 1: the first version of this file
named pseudo 98 as the split `col` from a dump of an earlier split variant; pseudo numbers
move with declarations, which is why the mapping is now computed.)

| role | final (reuse) | split | hard reg final / split |
|---|---|---|---|
| list index: T2 const 0, T3 incr | 82 (insns 250, 556) | 82 (insns 252, 560) | t0 ($8, greg `82 in 8`) / none (no greg entry → stack 24(sp)) |
| grid row: T1 ashiftrt | 82 (insn 52) | 93 (insn 53, `row`) | t0 / v0 (lreg `;; Register 93 in 2.`) |
| triangle number: K2 0, K3 -1, K4 incr | 83 (insns 340, 465, 511) | 176 (insns 343, 468, 514) | s0 (greg `83 in 16`) / s0 (greg `176 in 16`) |
| grid column: K1 `i & 1` | 83 (insn 54) | 94 (insn 55, `col`) | s0 / v1 (lreg `;; Register 94 in 3.`) |

- f.flow: final `Register 82 used 12 times across 90 insns; dies in 2 places; crosses 2 calls.`
  split `Register 82 used 8 times across 87 insns; crosses 2 calls.`
- f.flow: split `Register 94 used 4 times across 4 insns in block 1.` and
  `Register 93 used 4 times across 3 insns in block 1.`; final `Register 83 used 21 times
  across 37 insns; crosses 1 call.`
- f.lreg: final `(insn 54 ... (set (reg/v:SI 83) (and:SI (reg/v:SI 80) (const_int 1)))`;
  split `(insn 55 ... (set (reg/v:SI 94) (and:SI (reg/v:SI 80) (const_int 1)))`.
- ALLOCDBG: final `ord=16 pseudo=82 hardreg=8 nrefs=12`, `ord=3 pseudo=83 hardreg=16`; split
  `ord=16 pseudo=82 hardreg=-1 nrefs=8`, `ord=3 pseudo=176 hardreg=16`.
- FINDREGDBG pseudo 82, both spellings: conflicts `2 3 4 5 6 7 16 17 18 19 20 21 22 23 29 30`,
  someone_prefers / own_copy_prefs / own_full_prefs EMPTY; first pass (acc=0) finds nothing.
  Final has a second find_reg call with acc=1 (the caller-save retry); split has none.
- Final asm: `sra $8,$2,1` / `andi $16,$5,0x0001` / `move $8,$0` + `sw/lw $8,24($sp)` around
  the calls (the target's bytes). Split asm: `sra $2,$2,1` / `andi $3,$5,0x0001` /
  `sw $0,24($sp)` + `lw $4,24($sp)` (the counter lives in memory).

### (D)(2) The mechanism — two decisions, both named
1. **global.c `find_reg` caller-save retry (tools/gcc-2.7.2/global.c:1174-1188).** When the
   first pass finds no register (pseudo 82 conflicts with every call-saved register it could
   take: s0-s7 and fp are all held by higher-priority allocnos, and a call-clobbered register is
   refused because it crosses calls), find_reg retries with call-clobbered registers only if
   `CALLER_SAVE_PROFITABLE (allocno_n_refs, allocno_calls_crossed)`, which regs.h:164-165
   defines as `4 * calls < refs`. Pseudo 82 crosses 2 calls (func_8002E6B0 and func_8002FC80),
   so it needs refs > 8. Final: 12 → retry → first free call-clobbered register not in its
   conflicts is 8 (t0) → caller-save `sw/lw $8,24($sp)`. Split: 8 → no retry → no register →
   reload gives it the stack slot. `reg_n_refs` is loop-depth weighted in flow.c (2081, 2329,
   2515, 2725: `reg_n_refs[regno] += loop_depth`); T1 and RT1 sit in the first loop (depth 2),
   so they add 2 + 2 = 4 — exactly the 12 − 8 difference. The retry is the ONLY route to
   t0: the later local-alloc kick-out path (global.c:1197-1250) skips any register in `used2`,
   and on the first pass (`accept_call_clobbered` = 0, calls crossed ≠ 0) `used2` starts from
   `call_used_reg_set` (global.c:970-983), so it cannot hand out a call-clobbered register;
   at most it could evict a local-alloc occupant of a call-saved register, which is not t0.
2. **local-alloc eligibility (tools/gcc-2.7.2/local-alloc.c:469-477) and `find_free_reg`
   (local-alloc.c:2135 ff.).** A pseudo whose references all lie in one basic block and that
   dies once gets `reg_qty = -2` and is allocated by local-alloc's block_alloc; find_free_reg
   takes the first hard register (numeric order; config/mips defines no REG_ALLOC_ORDER) not
   live over its lifetime, and a quantity that crosses no call may take any call-clobbered
   register. Split `col` (94) lives in block 1 only → local-alloc → v1 ($3). The target's
   `andi s0` needs the i & 1 result in the pseudo global-alloc seats in s0: in the final
   spelling that is 83 (`temp2`), multi-block and crossing func_8002E6B0, allocated by global
   (ALLOCDBG ord=3 → 16).

### (D)(3) Necessity, not effect
**`temp` — property of the reuse spelling:** the list-index pseudo carries references beyond
the list-index statements (T1 and RT1, loop-depth weighted), lifting its ref count above
4 × calls crossed = 8.
**Every one-variable-per-value spelling lacks it, because value 1 has its own variable:** the
list index's variable is then referenced only by T2, T3 and RT2 ((C)(2): no extra reads or
writes may exist), and those statements are fixed by the program: T2 before the record loop,
T3 in its step, RT2 as func_80044B30's argument after func_8002FC80 returns. flow counts them
at 8 (dumps). Its live range runs from T2 through the last RT2/T3 and so always spans the
func_8002E6B0 call inside the inner loop and the func_8002FC80 call evaluated before RT2's
consumer — 2 calls in any statement order of the fixed statements (moving RT2 ahead of the
func_8002FC80 call changes the call's argument evaluation, which the (C)(2) statement list
does not allow either). 4 × 2 < 8 is false, so global.c never retries with call-clobbered
registers, and every call-saved register is taken by higher-priority allocnos (FINDREGDBG
conflicts 16-23 and 30), so the list index gets no register: the target's `move t0,zero`,
`addiu t0,t0,1` and caller-save `sw/lw $8,24($sp)` cannot be produced. Declaration order and
scope do not change refs or calls crossed. Type: `u32` gives the same RTL (21, measured); a
narrower type adds extension instructions the target does not have (`s16` measured, 21). A
per-value spelling of the row value that still reached t0 would need the row's variable to be
the list index's pseudo — which is the reuse.

**`temp2` — property of the reuse spelling:** the i & 1 result is written into a pseudo that is
live in more than one basic block and crosses a call, so global-alloc (not local-alloc) seats
it, in s0.
**Every one-variable-per-value spelling lacks it, because value 1 has its own variable:** the
column's variable is written by K1 and read by RK1 only ((C)(2)), both in the first loop's
body block with no branch between them, so it is a single-block pseudo that dies once
(dumps: `Register 94 ... in block 1`, `;; Register 94 in 3.`) and local-alloc allocates it. A single-block quantity crossing no call
takes the first free register in numeric order, and v1 is free there (it is what it gets), so
it can never be seated in s0: the target's `andi s0,a1,0x1` / `addu a0,v0,s0` cannot be
produced. Scope (column split alone: loop-body `col` 2, function-scope `col` 2; with the row
also split: loop-body 21, function-scope 21), type (`u32`, `s16` measured) and declaration
order do not change which blocks reference it.

This covers the unmeasured spellings by the ref-count and basic-block arguments, not by
enumeration.

### (D)(4) Measured alternatives (`sandbox --disable all`, current build compiler)
- Full one-variable-per-value spelling (`one-var-per-value-form.c`): **21/231** (229 insns).
- Per-variable ablation (each variable has two values, so this is beyond what (D)(4) asks):
  row split, `temp2` reused → **22/231** (loop-body or function-scope `row`, both 22); column
  split, `temp` reused → **2/231** (loop-body or function-scope `col`, both 2: the `andi` and
  its use in v1 instead of s0 — the local-alloc decision alone).
- Structural respellings: function-scope `row`/`col` 21; block-scoped initialised
  `row`/`col` 21; `(idx << 2) + (row << 1) + col` 21; `(idx * 2 + row) * 2 + col` 52; fully
  inline index `idx * 4 + (i / 2) * 2 + (i & 1)` 22; inline nested form 52; `u32` row/col 21;
  `s16` row/col 22; `u32` list index 21; `s16` list index 21. (`i % 2` in place of `i & 1`
  with the reuse: 34, 225 insns — the signed-remainder expansion, not the target.)
- Permuter from the one-var body (tools/permuter_campaign.py, workspace built by
  tmp/f290b8/mkperm.sh from `one-var-per-value-form.c`, build cc1, `--stack-diffs`; base
  permuter score 1070 = sandbox 21):
  - Campaign 1 (tmp/perm_290b8_split, -j4, ~11,850 iterations (harvest 11,851; log ends
    11,853), 454 s — stopped early by mistake, so fresh-seed campaigns were run for the full
    window): 28 finds, best 765. 26 sandbox-scored (the other two, output-985-2 and
    output-1015-2, were written after collection; read by the round-2 reviewer: 985-2 leaves
    the record loop's `temp` undeclared, 1015-2 leaves `rec` uninitialised on a path —
    invalid). Of the 26: the lowest is 21 (output-1070-1, = the base with the
    re-merge `n = row; n = idx * 4 + n * 2 + col;`), all others 22-68 or fail to build.
  - Campaign 2 (tmp/perm_290b8_split2, fresh seed, -j8, ~48,920 iterations (log ends 48,932), 1,500 s): 67
    finds, best 765 at ~3 min (a re-find), no better score in the remaining ~22 min. All 67
    sandbox-scored afterwards (with src/ back at INCLUDE_ASM): lowest 21 (output-1070-1/-4/-6,
    base-equivalent), then 22, 23, 24 (incl. 775, the one valid reordering), 25+.
  - Campaigns 1-2 started from the first recorded one-var form (`temp2` value 2 at function
    scope). Campaign 3 starts from the corrected (C)(1) form (its src.c equals the current
    `one-var-per-value-form.c` modulo comments, checked token-wise):
  - Campaign 3 (tmp/perm_290b8_split3, fresh seed, -j8, ~53,890 iterations (log ends 53,898), 1,373 s): 69
    finds, best 739 at ~3 min (`volatile unsigned char temp` declared inside the head loop,
    leaving the record loop's `temp` undeclared; also a volatile/type device), no better score
    in the remaining ~20 min. All 69 sandbox-scored: lowest 21 (output-1070-1,
    base-equivalent), then 22, 23, 24, 25+; 20 fail to build — 15 move a `temp`/`temp2`
    declaration into a block that does not enclose its uses, 2 hoist `col = i & 1` out of its
    scope, 1 subscripts with a `double`, 2 chain the struct copies plus further moves.
  - Across the three campaigns (~114,700 iterations) no find reaches 0, and no valid find
    keeping one value per variable scores below the one-var base's 21.
  - What the finds do (both campaigns, the lowest ones read in full): move
    `rec = func_8004678C()` under the head loop's `if` (765, 800, 810, 835, 880, 1015 —
    `rec` uninitialised when the branch is not taken: invalid); move it before the head loop
    (775 — valid, still non-zero); chain the two struct copies into one expression (785, 820,
    895, 1041); move `b =` / `c =` / the +0x108 store between blocks, including into dead
    code after `return` (846, 856, 970, 996, 1016, 1045); move `temp`'s declaration into an
    unrelated inner block, leaving the record loop's `temp` undeclared (885, 895, 960, 985 —
    fail to build); re-merge or constant-stage `n` (885-2, 1016-2/3, 1061-1, 1070-1 — the
    Ruling 11 (C)(3) / F1 staging shapes); replace `idx == 0` with `1 == 0` (1070-2 —
    changes the program). No find reaches 0, and none that keeps one well-defined value per
    variable scores below the base.

## (E) Honest generic name
`temp` and `temp2` — form (i) of Ruling 11 (E): generic scratch words (`temp` + digit), claiming
nothing about either value. Codegen is name-independent (final.c scores 0).

## (F) Annotation
Each declaration carries a comment naming both values and citing Ruling 11 and this file.

## (G) Layer-2
Required: fresh cheat-reviewer, default-FAIL, walking (A)-(H) with this file.

## (H) Everything else
The rest of the body stands on its own:
- `scr` is the `u8 *` scratchpad-record pointer with displaced `*(s32 *)(scr + off)` accesses,
  the same spelling as the siblings in this file (func_8002C22C documents why; its first-block
  absolute / later register+offset split is the same cse mechanism here: the two struct copies
  come out as absolute 0x1F80033C / 0x1F800330, everything after the loop label as `off($s3)`).
- The two 12-byte struct copies (`*(LeafPos *)(scr + 0x84) = tbl[idx * 4]` and min = max) are
  plain struct assignment of the existing `LeafPos` type.
- `n` is a single-meaning local (the point's index), written once per iteration.
- `PosRec` is a new local typedef for the list entries; field meanings are stated in the
  comment with their evidence (terminator test, `used` cleared by func_8002906C and set here,
  x/y/z compared against the bounds).
- The pointer casts: func_8002E6B0 and func_8002FC80 keep their existing prototypes; the
  `LeafPos *` points are passed as `s32 *` / `VECTOR *`. func_8002FC80 reads only vx/vy/vz
  (src/code6cac_b.c, its body), so no pad word is read through the 12-byte points.
- Control flow: `goto hit` from the inner loop to the shared hit block at the outer loop's tail;
  the retry is `idx = 1; temp2 = -1; continue;` (parameter reassignment, loop restart). Both
  are needed for the target's block layout (a trailing out-of-loop hit block measured 50-52).
- Prototypes for func_8002E6B0 / func_8002FC80 / func_80033550 (defined later in this file,
  identical signatures) and an `extern` for func_80044B30 (src/text1a_c.c, same signature)
  are declared ahead of the function.
