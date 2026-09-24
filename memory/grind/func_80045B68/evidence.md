# func_80045B68 — evidence (manual lane, 2026-09-24)

`src/text1a_c.c`, 302 target insns, queue verdict **C** (`canonical`: hand_coded
tier LOW — ordinary pure-C target). Distance walked **302 → 9** this session;
`candidate.c` is at **302/302 instructions**, so the residual is register
allocation and two instruction orderings, not missing/extra code.

## What the function does

Builds the display list for scene id 6.

```
p    = func_800455AC(6)                    /* slot alloc */
hdr  = arg3 ? (s32 *)arg3 : p              /* header block */
func_80044F50(arg0, arg1, (s32)hdr)
cnt  = hdr[0]
last = (s32)hdr + ALIGN4(hdr[1])           /* spilled, sp+0x160 */
dl   = (s32)hdr + ALIGN4(hdr[cnt])         /* $s3 */
prev = cnt >= 3 ? (s32)hdr + ALIGN4(hdr[cnt-1]) : 0   /* spilled, sp+0x168 */
```
`ALIGN4(x)` is the file-local `(((u32)x >> 2) << 2)` idiom (same spelling as
`func_80045878`, `src/text1a_c.c:1889`).

Then five blocks:
1. fill `sp108[32]` with -1, counting **down** from 31;
2. walk `arg2` until `(s16)v == -2`, setting `sp108[D_800993FC[i]] = 1` for each
   non-negative entry;
3. for `i` in 0..31, if `sp108[i] >= 0`, `switch (n)` over 0..3 →
   `func_800480C0(dl, i, {0,0x80}, {0,0x40}, {-0x180,-0x160}, {0xF0,0xF1})`,
   then `sp108[i] = n; n++`;
4. `DrawSync(0)`; walk `arg0 ? arg2+0x33 : arg2` writing **pairs** of ±1 into
   `sp18[120]`, terminated by -2;
5. build via `func_80044378`/`func_8004428C`, then walk `sp18` in steps of 2
   doing the same 4-way `switch` on `sp108[D_800993FC[i]]` into
   `func_800433E4(6, y, …, {0xE8,0xE9})`, `y += 2` per kept entry.

Frame: `s16 sp18[120]` at sp+0x18, `s16 sp108[32]` at sp+0x108, then five
8-byte-strided spill slots (0x148 arg0, 0x150 arg3, 0x158 p, 0x160 last,
0x168 prev). The `0xF0` offsets in the asm are `sp108`'s frame offset relative
to `sp18`, not a source-level `+0x78` index.

## Levers that moved the distance (each measured in isolation)

| # | Lever | Δ |
|---|---|---|
| 1 | `func_800433E4`'s **arg1 is `int`, not `s16`** — target passes `y` with a bare `move`; an `s16` prototype makes GCC keep `y` pre-shifted (`y<<16`) and emit `sra`/`lui 0x2;addu` per use | 128 → 109 |
| 2 | Block 2 loads through a **`u16` holder** (`t = *q++; (s16)t …`) — one `lhu` + one shared `sll`, reused by both the `!= -2` compare and the sign test. A plain `s16 *` deref emits `lh` *and* `lhu` | 109 → 62 |
| 3 | Block 2 indexes `D_800993FC[i]` (an **index**, not a walking pointer) — loop.c strength-reduces it and puts the table base in the loop **preheader**, where the target has it | 62 → 57 |
| 4 | `cnt` is the **reused `arg1` parameter** — one DECL ⇒ one pseudo ⇒ one hard register across the call, which is why the target's `$s0` holds arg1 *and* `hdr[0]` | 56 → 52 |
| 5 | **Block 2's walker is the same variable as blocks 4/5's `sp18` walker.** That single pseudo's live range crosses `func_800433E4`, so it must be callee-saved — that is the target's `move $s0,$fp` at 0x80045C44. Splitting it put the block-2 walker in `$a1`, which cascaded: `$t0` got taken by a loop constant, so every spill reload became `$t1` instead of `$t0` | 52 → 26 |
| 6 | `q = sp18` hoisted **above** the block-4 `arg0` if/else — it then fills the `bnez` delay slot and the if/else keeps both arms (the `j`), as in the target | 26 → 22 |
| 7 | **`cnt` is the same variable as block 3's switch counter `n`**, not a standalone local. Their live ranges are disjoint, so it is one pseudo with the *combined* ref count — which outranks `hdr` in `global_alloc` and takes `$s0` first, leaving `hdr` at `$s1`. That is the target's `$s0` holding arg1, `hdr[0]` **and** the block-3 counter | 22 → 9 |

Lever 5 is the big structural one: the `$t0`/`$t1` reload divergence at seven
sites was a *downstream* symptom of one variable being split in two.

## Residual at 9 (302/302 insns, **0 operand-only hunks**)

Register allocation is now exact — every remaining scored hunk is one
instruction sitting in a different *slot of the same basic block*. Three of
them, and in all three ours emits the value **earlier** than the target, and
in all three the value is a def of a long-lived callee-saved pseudo:

| # | Instruction | ours | target |
|---|---|---|---|
| 1 | `lw s0,0(s1)` (`n = hdr[0]`) + its `sll`/`addu` pair | 26, 28-29 | 27, 30-31 |
| 2 | `li s1,31` (fill-loop counter) | 48 | 49 |
| 3 | `move s2,zero` (`y = 0`) | 196 | 204 (loop preheader) |

Item 1 is GCC's insn scheduler picking the longer dependency chain first: the
`hdr[0]` chain (`sll`→`addu`→`lw`→`srl`→`sll`→`addu`, plus `slti`→`bnez`)
is strictly longer than the `hdr[1]` chain (`srl`→`sll`→`addu`), so it issues
first. The target issues the *shorter* one first. Statement order does not
control this — both orders were measured and both emit `hdr[0]` first.

Item 2: the `-1` is a loop invariant that `loop.c move_movables` hoists with
`emit_insn_before(…, loop_start)`, landing it *after* `i = 31`. The target has
it *before*, i.e. there it was never a movable.

Item 3: the target's `y = 0` sits in the loop preheader, after the four
hoisted `li` constants; ours sits before the guard test. An explicit guard
(`if (*q != -2) { y = 0; do {…} while (…); }`) did not move it.

## Reference points

- `func_80045878` (same file, COMPLETED-C) — sibling allocator/`ALIGN4` idiom.
- `func_800480C0` (`src/text1b.c:131`) is declared `(s32, s32, s16, s16, s16,
  s16)` — its `s32 arg1` is the direct precedent for lever 1's claim about
  `func_800433E4`.
- `src/text1a_pre.c:271` / `text1a_post.c:276` — same `func_800480C0` call
  shape with the `{-0x140, 0xE8/0xF0}` constants.
