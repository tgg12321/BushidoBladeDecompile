# func_8001BE20 — Ruling 11 submission for the reused local `temp` (manual s4, 2026-09-28)

Ruling: .claude/rules/ordinary-c-judge-decidable.md § Ruling 11 (owner, 2026-09-26; verbatim
record docs/grind/owner-rulings-2026-09-26.md Q1). This is a FRESH submission: the 2026-09-25
layer-2 FAIL of `shift` (Rulings 5/6/9) and its entry on Ruling 9's "what stays banned" list
are not contested — Ruling 11 is a different test and "nothing here pre-decides one".

Files in this directory:
- `final.c` — the submitted body (byte-identical to what is spliced into src/code6cac.c).
- `reuse-form.c` — same body with the variable named `shift` (pre-rename, identical tokens).
- `one-var-per-value-form.c` — the one-variable-per-value spelling (value 1 in its own `half`).
- `dumps.txt` — (D)(1) excerpts for both spellings; `mk.py`, `dumps.sh`, `spans.py` produced them.

## The variable and its values

```c
s32 temp;                              /* function scope */
temp = arg0 * 16;                      /* W1 */
buf[0..3] = (D_80102788.<word> >> temp) & 0xFFFF;   /* R1..R4 */
if (D_800A38DC == 6) temp = 0;         /* W2 */
else                 temp = arg0 * 4;  /* W3 */
... loop: D_80106A50.color[k] >> temp  /* R5..R7, each executed per iteration */
```

- **Value 1** = {W1}, read by R1..R4 (this player's 16-bit half of each pad word).
- **Value 2** = {W2, W3}, read by R5..R7 (this player's 4-bit colour-config nibble; 0 selects the
  low nibble in mode 6). W2 and W3 both reach R5..R7, so they are one value. W1 reaches no read in
  R5..R7 (W2/W3 dominate the loop) and W2/W3 reach none of R1..R4, so there are exactly two values.

## (A) Fresh local, not a borrow
`temp` is a function-scope `s32` local of func_8001BE20, declared once. Its writes are in the
function body and in both arms of the if/else, so function scope is the innermost scope
enclosing all of them. No other declaration moved or re-scoped relative to the
one-var-per-value form (diff: only `half` removed). No `&temp`. Not a parameter/global/static/
register variable.

## (B) Every write is live
(1) W1 is read by R1 on every path (straight line). W2 is read by R5..R7 on the feasible path
mode == 6 && arg0 == D_800A38A0 (the loop's else arm; `D_800A38A0` is a runtime global, so the
pair is jointly satisfiable). W3 is read by R5..R7 on every path with mode != 6 (loop's else arm
is then always taken). No write is dead.
(2) Re-store check (Ruling 5 2(c), 2026-09-26 clarification):
- W2 stores 0. The variable holds 0 on the incoming path arg0 == 0 (W1 = 0). It does NOT hold 0 on
  the feasible path arg0 == 1, mode == 6 (holds 16). So W2 does not hold the written value on
  EVERY feasible incoming path. Ledger record of the differing path: arg0 = 1, D_800A38DC = 6 →
  temp = 16 before W2.
- W3 stores arg0 * 4. On arg0 == 0 the variable holds 0 == 0 * 4; on the feasible path arg0 == 1,
  mode != 6 it holds 16 ≠ 4. Record: arg0 = 1, D_800A38DC = 0 → temp = 16 before W3.
- W1 is the first write (nothing held).

## (C) Same statements; real computations
(1) One-var-per-value spelling recorded: `one-var-per-value-form.c` (= ledger candidate.c,
respelled for the 2026-09-26 `D_80106A50.color` merge) — value 1 in `s32 half` (its writes and
reads are all at function-body level, so function scope is its innermost enclosing scope), value 2
in `shift`.
(2) Same statement list: `diff reuse-form.c one-var-per-value-form.c` = one added declaration and
five identifier changes (W1, R1..R4), no statement added or removed.
(3) Value 1's write W1 is an arithmetic computation (arg0 * 16 → `sll ...,4`; in the target
it is the shared first step of the arg0 * 0x44C multiply, the a2 that `move a3,a2` copies).
Value 2's W3 is an arithmetic computation (`sll a3,s1,2` in the target). Value 2 also has the
constant write W2, but (3) needs only one real computation per value. No value is a bare copy.

## (D) Allocator-dump proof of necessity

### (D)(1) Dumps — `dumps.txt`
Both spellings compiled with the build compiler `tools/gcc-2.7.2/build/cc1` and the build flags
(`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel
-msoft-float -da`), and with the instrumented cc1 (`BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=1`),
which emits byte-identical asm for both. Pseudo numbers (reuse / split):

| role | reuse | split | hard reg |
|---|---|---|---|
| P = arg0<<4 (multiply's first step) | 95 | 96 | a2 (both) |
| value-1 holder | 75 (= temp) | — (CSE folded `half` into P) | a3 / — |
| value-2 (nibble shift) | 75 (= temp) | 76 | a3 / a3 |
| `out` (loop) | 76 | 77 | a1 |
| loop pointer over buf | 479 | 480 | a2 |

- f.cse, reuse: `(insn 106 (set (reg/v:SI 75) (reg:SI 95)))` survives; R1/R2 read `reg/v 75`.
- f.cse, split: no copy insn; R1/R2 read `(reg:SI 96)` = P directly.
- f.greg conflicts: reuse `75 conflicts: ... 76 ... 479 2 3 4 29`; split `96 conflicts: 72 73 96 2 3 4 5 29`
  (no loop pseudo).
- ALLOCDBG: reuse ord 12 pseudo 75 → hardreg 7 (a3); split ord 10 pseudo 96 → hardreg 6 (a2).
- Final asm: `move $7,$6` count 1 (reuse) vs 0 (split). Sandbox: reuse 0/393, split 5/393 =
  the missing `move a3,a2` + four srlv reading a2 instead of a3.

### (D)(2) The mechanism — two decisions, both named
1. **cse.c `make_regs_eqv` (tools/gcc-2.7.2/cse.c:826-882; the test at :846-858).** When the copy
   `X = P` is seen, X joins P's quantity; X becomes the canonical register (so the copy survives
   and later reads use X) only if `uid_cuid[regno_last_uid[X]] > cse_basic_block_end` or
   `uid_cuid[regno_first_uid[X]] < cse_basic_block_start`, AND X's last use is later than the
   current canonical's. Otherwise P stays canonical, every read of X is replaced by P, and the
   copy becomes dead and is deleted. Reuse: pseudo 75's last reference is in the colour loop
   (insn-order position 278, outside the followed-jump CSE path that holds the struct copy,
   extraction and the nibble if/else) → canonical → copy kept. Split: `half`'s references are the
   write and R1..R4, all inside that path → folded into P (dump: reads use reg 96).
2. **global.c `find_reg` (tools/gcc-2.7.2/global.c:952-1301; allocnos taken in priority order
   by global_alloc, global.c:289).** find_reg takes the first hard register, in numeric order
   (config/mips defines no REG_ALLOC_ORDER), that is not in the allocno's used set (its conflicts
   plus regs_someone_prefers), then may move to a same-class copy/full preference. FINDREGDBG
   (dumps.txt, last section): reuse pseudo 75 has conflicts `2 3 4 5 6 29`, someone_prefers,
   own_copy_prefs and own_full_prefs all EMPTY → first free = 7 (a3). Split P (96): conflicts
   `2 3 4 5 29`, no preferences → 6 (a2). So a holder of arg0 * 16 lands in a3 only if v0, v1,
   a0, a1 AND a2 are all in its conflict set. In 75's row, v0/v1/a0 are hard-reg or v1/a0
   pseudo conflicts from the window; a1 and a2 come ONLY from the loop pseudos 76 (`out`, a1)
   and 479 (loop pointer, a2) (f.greg `75 conflicts: 72 73 75 76 142 ... 441 479 2 3 4 29`, with
   the pseudos' seats in ALLOCDBG). A copy of P that dies where P dies does not conflict with P,
   so without the loop pseudos it would take a2 and the copy would be a deleted no-op.

### (D)(3) Necessity, not effect
**Property of the reuse spelling the decisions depend on:** the pseudo that holds arg0 * 16 during
R1..R4 has references inside the colour loop, i.e. it is live where the loop pseudos `out` (a1)
and the loop pointer (a2) are live.

**Every one-variable-per-value spelling lacks it, because value 1 has its own variable:**
value 1's variable is referenced only by W1 and R1..R4 ((C)(2): no extra reads may exist). R1..R4
write buf[0..3], which the colour loop reads, so in ANY statement order R1..R4 execute before the
loop is entered; `out` is born inside the loop and the loop pointer is set in the loop preheader
(spans in dumps.txt: split P last ref pos 65, nibble shift born 72, loop pointer 81, `out` 100).
So value 1's variable is dead before either loop pseudo is born, whatever its declaration order,
scope, type (`u32 half` measured, 5), or the order of the extraction statements relative to each
other, to the struct copy, or to the nibble if/else. It therefore conflicts with neither a1's nor
a2's occupant, and:
- if all its references lie within the CSE path, decision 1 folds it into P → no copy (5);
- if it is written earlier (outside the path) so that it survives decision 1, decision 2 still
  seats it in a2 or a lower free register (a1/a2 are not in its conflict set), never a3 → no
  `move a3,a2`
  (measured: dead-init L3 → a1; e1 `half = arg0; half *= 16` → `sll a2,a2,4`; s3 `half` before
  the struct copy → 5; hypotheses.md s2 items 1-5).
A value-1 variable that reached a3 would need a read in the loop, which is a statement the
one-var spelling does not have. Hence no one-variable-per-value spelling can emit the target's
`move a3,a2` + four `srlv ...,a3`; the reuse variable does, because its value-2 reads put it in
the loop. This covers the unmeasured spellings by the liveness argument, not by enumeration.

### (D)(4) Measured alternatives (`sandbox --disable all`)
- Full one-var-per-value spelling: **5/393** (re-measured 2026-09-28 on the current compiler).
- Ablation: not applicable (two values).
- Structural respellings (all ≥ 5): see hypotheses.md — pointer loop, flag local, shared `col`,
  arm-scope colour bytes, s32 pad words, ternary nibble shift (8/20), `half` before/after struct
  copy, block-scoped / `u32` / `<< 4` `half`, dead init, `(u16)` casts; s3 enumerate rung
  326/326 spellings of the extraction block all at 5.
- Permuter from the one-var body: s2 campaign (tmp/perm_1be20_a, ~11.8k iterations, then-current
  compiler) — zero single-role finds; s4 campaign on the current build compiler
  (tmp/perm_1be20_s4, base = one-var-per-value body, base score 120, -j4, 9,412 iterations,
  stopped at harvest after no novel find past ~5.7k): 3 finds (115, 110, 105), and all three
  delete `half = arg0 * 16;` from before the extraction and re-insert it AFTER one of R1..R3,
  so buf[0..2] read an uninitialised `half` — invalid C, not a spelling. No find keeps a single
  well-defined value per variable. Best valid permuter score = the base (5/393 in sandbox).

## (E) Honest generic name
`temp` — form (i) of Ruling 11 (E): a generic scratch word, claiming nothing about either value.
(The previous name `shift` is not relied on; codegen is name-independent: final.c scores 0.)

## (F) Annotation
Declaration comment names both values, cites Ruling 11 and this file.

## (G) Layer-2
Required: fresh cheat-reviewer, default-FAIL, walking (A)-(H) with this file.

## (H) Everything else
Unchanged from the body the 2026-09-25 layer-2 ruled SOUND apart from `shift` (aggregate merge of
D_80102788 as PadState; the one-expression `unk_34E` flag store; block-scoped r/g/b; duplicated
`& ~0xF0`), plus one respelling since: the colour bytes are read as `D_80106A50.color[k]`
because 0x80106A70..72 are now `FileRecord.color[3]` (merged 2026-09-26 in 65897593b); the
landing retires the surviving `D_80106A70` alias row in undefined_syms_auto.txt, whose comment
says to retire it with this function.
