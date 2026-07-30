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
