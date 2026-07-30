# Evidence bank — func_80040CB8

## Session s1 (2026-07-30, modality: recon)

### Routing / baseline
- `canonical func_80040CB8` → verdict **C**, `asm_insns 0`, `total 36`,
  `distance 17`, reason "pure-C distance 17 <= 50 — pure-C target". No
  canonical-asm angle; this is an ordinary pure-C function.
- Inherited src state was a **cheat-carrying** body: eight
  `register T x asm("<reg>")` pins (a2/t1/t4/t3/t2/t0/a3/a1). Sandbox strips
  them (`cheat_asm_stripped: 23`), giving the honest baseline floor **17** at
  **37 build insns vs 36 target insns**.
- Floor at end of s1: **13** at **36 build insns == 36 target insns**, pure C,
  zero pins, zero rules. Source in `src/text1a.c` and in `candidate.c`.

### What the function actually does (settled — do not re-derive)
`arg0` is a player/robot object. Two arrays of 0x68-byte elements live in it:
one at `arg0+0x94` (the "link" array, only its element ADDRESS is used) and one
at `arg0+0x8B4` (the entry array being built). `D_80094B9E` is a table of 18
records of **0xA bytes** whose first `s16` is an id, `-1` meaning "empty slot".

The loop runs a **fixed 18 iterations** (`slti $v0,$t1,0x12`) over the table.
The table cursor (`+0xA`) and the link cursor (`+0x68`) advance EVERY iteration;
the two entry-array cursors advance ONLY when the id is not -1 — i.e. it is a
compaction/filter: non-empty table ids are packed into consecutive entry slots.
After the loop a `-1` terminator id is written into the next free entry.

Per-entry field writes (offsets from the 0x8B4-based element):
`+0x00 = 3` (kind, sb) · `+0x01 = 0` (sb) · `+0x02 = id` (sh) ·
`+0x04 = *(u16*)(arg0+0x16)` (sh) · `+0x06 = 1` (sh) · `+0x08 = 0` (sh) ·
`+0x0A = 0` (sh) · `+0x0C = arg0+0x94 + i*0x68` (sw, a pointer) ·
`+0x58 = 0` (sw). Terminator after the loop: `+0x02 = -1` (sh).

Sibling `func_80040400` (same file, ~line 122, already matched) writes the very
same field set into the same 0x68-byte element type via a walking `s16 *a1` —
useful as a naming/typing reference for a future rederive attempt.

### Target's addressing shape (the load-bearing structural fact)
Target uses **exactly two** cursor registers into the entry array, both
incremented by 0x68 inside the `if`:
- `$a2 = arg0 + 0x8B4`, used with displacements **{0, +2}** only
  (`sb $t3,0($a2)` in-loop; `sh $v0,2($a2)` post-loop).
- `$a1 = arg0 + 0x90C` (= 0x8B4 + 0x58), used with displacements
  **{-0x57, -0x56, -0x54, -0x52, -0x50, -0x4E, -0x4C, 0}**.

So the SAME halfword `0x8B6` is written twice through two different cursors:
as `id` via `sh ...,-0x56($a1)` in-loop, and as `-1` via `sh $v0,2($a2)` after
the loop. That is why two C-level cursors exist rather than one; a single
struct pointer produces a single base register and cannot reproduce this.

### The loop-form dilemma (measured both ways)
- **Any GCC-recognised loop form** (`for` / `while` / `do-while`) emits
  `NOTE_INSN_LOOP_BEG`, which enables loop.c strength reduction. Strength
  reduction invents a THIRD induction pointer based at `arg0+0x8B8` for the
  negative-displacement cluster and demotes `arg0+0x90C` to serving only the
  `+0x58` store → **three** `addiu rX,rX,0x68` per iteration vs target's two →
  38 insns, score 25. Banked:
  `rejected/do-while-loop-form-triggers-strength-reduction.c`,
  disassembly `tmp/grind/func_80040CB8/s1/build2.txt`.
- **The `goto`-loop form** is not recognised as a loop, so no strength
  reduction runs and the displacements stay exactly as written → the
  two-cursor shape is reproduced exactly. This is REQUIRED.
- Cost of the goto form: no LICM either, so the three loop-invariant constants
  (-1, 3, 1) cannot be hoisted from literals. Literals give 35 insns / score 23
  (one constant folds away). Banked:
  `rejected/literal-constants-no-licm-in-goto-form.c`.

### Two smaller settled facts
- The inherited pinned form's extra 37th instruction was a `sll/sra` pair
  sign-extending the -1 comparand, caused by typing the sentinel `s16`. Typing
  it `s32` removes the pair (the `lh` result is already sign-extended, so the
  compare is direct `beq`) AND frees `$v0`, which lets the `lh` destination land
  in `$v0` as target has it.
- The `addu $v1,$v0,$zero` copy of the id in the beq delay slot is
  **GCC-generated, not source-driven**. Writing an explicit `s16 cur = id;`
  intermediate and writing `*(s16*)(ent-0x56) = id;` directly produce
  byte-identical output (both score 21 at 36 insns in the s1 measurement pair).
  The intermediate is therefore noise — do not add it back.

### RTL / allocno evidence — the exact remaining mechanism
`cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls
-fno-builtin -w -da` on the cheat-stripped TU (repro script:
`tmp/grind/func_80040CB8/s1/greg.sh`; dumps kept alongside as
`text1a.i.{lreg,greg,...}`).

`global.c:allocno_compare` ranks allocnos by
`priority = floor_log2(n_refs) * n_refs / live_length`, descending; ties break
on lower pseudo number. MIPS has **no `REG_ALLOC_ORDER`**, so `find_reg` is
plain ascending-hard-reg first-fit. All 10 long-lived pseudos mutually conflict
(everything is live across the whole loop), so the register each variable gets
is decided **purely by its rank in that priority list**.

Measured for the score-21 form (`slot` declared FIRST), from `.lreg`:

| pseudo | var  | n_refs | live_len | priority | got |
|--------|------|--------|----------|----------|-----|
| 80 | ent  | 11 | 22 | 3*11/22 = **1.50** | $v1 (3) |
| 81 | copy |  3 |  3 | 1*3/3  = **1.00** | $a1 (5) |
| 79 | tbl  |  4 | 23 | 2*4/23 = **0.348** | $a2 (6) |
| 72 | arg0 |  5 | 29 | 2*5/29 = **0.345** | $a0 (4, preferred) |
| 78 | link |  4 | 24 | 2*4/24 = **0.333** | $a3 (7) |
| 73 | slot |  5 | 31 | 2*5/31 = **0.323** | $t0 (8) |
| 74 | i    |  4 | 28 | 2*4/28 = **0.286** | $t1 (9) |
| 77 | one  |  2 | 50 | 0.04 | $t2 (10) |
| 76 | kind |  2 | 52 | 0.038 | $t3 (11) |
| 75 | none |  2 | 54 | 0.037 | $t4 (12) |

The dump's own ordering line confirms it exactly:
`;; 11 regs to allocate: 80 81 83 79 72 78 73 74 77 76 75`.

**Confirmed lever.** `slot`'s priority is bottom-of-cluster only because its
live_length (31) is inflated by starting at the very first prologue insn while
its last use is after the loop. Moving its initialiser to be the **last**
declaration shortens live_length below `tbl`'s and `link`'s crossover
(`10/L > 0.348` needs `L < 28.7`), flipping the order to
ent, copy, slot, tbl, link, i, … — which lands
`slot=$a2, tbl=$a3, link=$t0, i=$t1, one=$t2, kind=$t3, none=$t4`, i.e. **seven
of the nine registers exactly on target**. Measured: score 21 → **13**.
Disassembly `tmp/grind/func_80040CB8/s1/build4.txt`.

### The residual 13, fully characterised
Only two defects remain, and they are coupled:
1. **`ent`/`copy` are swapped**: target `ent=$a1(5)`, `copy=$v1(3)`; we get
   `ent=$v1(3)`, `copy=$a1(5)`. Cause: `ent` (priority 1.50) outranks the
   GCC-generated copy pseudo (1.00) and so takes the lower hard reg first. To
   invert it the copy must outrank `ent` — needs copy priority > 1.50, e.g.
   `n_refs 4 / live_len 3` gives `2*4/3 = 2.67`. Lowering `ent` instead is not
   viable: at 11 refs it would need `live_length > 33`, and the whole loop body
   is only 22 insns.
2. **Prologue rotated one slot**: target emits `addiu $a2,$a0,0x8B4` FIRST;
   we emit it LAST, because emitted prologue order follows declaration order
   and fix #1-for-`slot` required declaring it last. Note cc1's first-pass
   scheduler runs BEFORE lreg and did **not** re-sink it, so the position is
   decision-time, not schedule-time.

The coupling is the whole remaining problem: `slot` needs a SHORT live range
(⇒ declared late) and a FIRST prologue slot (⇒ declared early). Any fix must
buy `slot`'s priority some other way than late declaration — either by raising
its `n_refs` from 5 to 6 (`12/31 = 0.387` clears both `tbl` and `link` even at
live_length 31), or by lengthening `tbl`/`link` live ranges past the 0.323
crossover, or by shortening `slot`'s live range without moving its initialiser.

### Artifacts
- `tmp/grind/func_80040CB8/s1/dis.sh` — objdump a sandbox `.o` down to a
  mnemonic-only listing for this function.
- `tmp/grind/func_80040CB8/s1/greg.sh` — reproduce the cc1 `-da` RTL dumps from
  the cheat-stripped sandbox source.
- `tmp/grind/func_80040CB8/s1/slice.sh` — slice one function out of a `-da` dump.
- `build.txt` (inherited pinned form, 37 insns/score 17), `build2.txt`
  (do-while, 38/25), `build3.txt` (goto + slot-first, 36/21), `build4.txt`
  (goto + slot-last, 36/13), `text1a.i.{lreg,greg,loop,combine,sched,...}`.
- `text1a.c.floor13` — exact TU snapshot at floor 13.

- [s1] canonical func_80040CB8 = verdict C, asm_insns 0, total 36, distance 17 - ordinary pure-C target, no canonical-asm angle.

- [s1] Inherited src carried eight register-asm pins (a2/t1/t4/t3/t2/t0/a3/a1); sandbox strips 23 cheat-asm items, so the honest baseline floor was 17 at 37 build insns.

- [s1] End-of-session src/text1a.c holds a pure-C body: zero pins, zero rules, 36 build insns == 36 target insns, floor 13, and the emitted opcode + displacement sequence is identical to asm/funcs/func_80040CB8.s.

- [s1] Function semantics settled: an 18-iteration compaction over a table of 18 x 0xA-byte records at D_80094B9E (first s16 = id, -1 = empty); non-empty ids are packed into consecutive 0x68-byte entries at arg0+0x8B4, and a -1 terminator id is written after the loop.

- [s1] Per-entry field map (offsets from the 0x8B4 element): +0x00=3 sb, +0x01=0 sb, +0x02=id sh, +0x04=*(u16*)(arg0+0x16) sh, +0x06=1 sh, +0x08=0 sh, +0x0A=0 sh, +0x0C=arg0+0x94+i*0x68 sw, +0x58=0 sw; post-loop +0x02=-1 sh.

- [s1] Target uses exactly TWO cursors into the entry array: $a2 = arg0+0x8B4 with displacements {0,+2} only, and $a1 = arg0+0x90C (=0x8B4+0x58) with displacements {-0x57,-0x56,-0x54,-0x52,-0x50,-0x4E,-0x4C,0}. Halfword 0x8B6 is written through BOTH (id in-loop via $a1, -1 post-loop via $a2), which is why two C cursors exist; a single struct pointer yields a single base register and cannot reproduce it.

- [s1] MIPS in gcc-2.7.2 defines no REG_ALLOC_ORDER, so global_alloc/find_reg is plain ascending-hard-reg first-fit; combined with all long-lived pseudos mutually conflicting, register identity is a pure function of allocno_compare rank.

- [s1] Measured allocno table for the score-21 form: ent 11 refs/22 len = 1.50; id-copy 3/3 = 1.00; tbl 4/23 = 0.348; arg0 5/29 = 0.345; link 4/24 = 0.333; slot 5/31 = 0.323; i 4/28 = 0.286; one 2/50, kind 2/52, none 2/54.

- [s1] Residual 13 is exactly two coupled defects: (1) ent and the id-copy are swapped ($v1/$a1 instead of $a1/$v1) because ent's 1.50 priority outranks the copy's 1.00 and first-fit gives it reg 3; (2) the prologue is rotated one slot because target emits addiu $a2,$a0,0x8B4 first while we must declare that cursor last to win its priority race. cc1's first-pass scheduler runs before lreg and did not re-sink the insn, so prologue position follows declaration order, not scheduling.

- [s1] Demoting ent is not viable: at 11 refs it would need live_length > 33 and the entire loop body is 22 insns. Raising slot's n_refs from 5 to 6 would give 12/31 = 0.387, clearing both tbl and link at its current live_length - which is the arithmetic route to breaking the coupling.

- [s1] The three constant-holder locals are load-bearing (H5) and therefore fall under .claude/rules/named-local-fake-exception.md; a future candidate-ready session must either eliminate them or carry the required FAKE annotation plus documented lever exhaustion. s1 flags this and does not claim it resolved.

- [s1] Matched sibling func_80040400 (src/text1a.c ~line 122) writes the same 0x68-byte element field set via a walking s16* and is a typing/naming reference for a struct-based rederive.

## Session s2 (2026-07-30, modality: structural) — floor 13 -> 0

### The governing mechanism (this is the whole function)
`flow.c` computes reference counts as `reg_n_refs[regno] += loop_depth`
(flow.c:2081, 2329, 2515, 2725), and `loop_depth` is derived ONLY from
`NOTE_INSN_LOOP_BEG` / `NOTE_INSN_LOOP_END` notes (flow.c:1401/1447, seeded per
basic block at flow.c:456/471). `global.c:allocno_compare` was verified against
the source: `priority = floor_log2(n_refs) * n_refs / live_length * 10000 *
allocno_size`, descending, ties broken on the LOWER allocno index
(`return *v1 - *v2;`). All ten long-lived pseudos mutually conflict and MIPS
defines no `REG_ALLOC_ORDER`, so every hard register is a pure function of that
rank (s1 finding, re-confirmed).

s1's `goto` loop form has NO loop notes, so every reference counts 1. That is
why the id copy (3 refs / live 3 = 1.00) loses its race against the 0x90C
cursor (11 refs / live 22 = 1.50) and the whole assignment rotates. With the
body inside loop notes the same code weights to: id copy 6 refs -> 12/3 =
**4.00**, cursor 22 refs -> 88/22 = **3.82**. The copy wins $v1, the cursor
takes $a1, and every remaining variable falls into target's slot behind them.
**Target's register assignment is unreachable without loop notes; the 4.00 vs
3.82 margin is the entire residual.** Measured proof: a plain
`for (i = 0; i < 0x12; i++)` immediately produced `copy=$v1` and `slot=$a2`
with the 0x8B4 cursor's `addiu` first in the prologue (s2/p2.txt) — the two
defects s1 characterised as coupled and unbreakable both closed at once.

### Why a recognised loop cannot be used, and the resolution
Confirmed s1's H2 from the loop dump: with a real loop (or a
`do { goto-body } while (0)` whose region starts at the `loop:` label), loop.c
recognises `ent` as a biv, creates DEST_ADDR givs for the `-0x57..-0x4C`
displacement cluster, `combine_givs` merges all eight onto one giv, and
`strength_reduce` reduces it to a third induction pointer based at
`arg0+0x8B8`; the 0x90C cursor survives only to serve `sw $zero,0(...)`. Three
`addiu rX,rX,0x68` per iteration against target's two -> 38 insns, score 25
(s2/p2.txt for-loop, s2/p3.txt do-while(0)-around-body).

The resolution is to keep the NOTES but make loop.c decline the region.
`scan_loop` bails with "Loop from 31 to 112 is phony." when `scan_start` is not
a `CODE_LABEL` (loop.c:568-575). Initialising `ent` between the
`NOTE_INSN_LOOP_BEG` and the `loop:` label puts an ordinary insn there, so
loop.c returns before any biv/giv analysis while flow.c still sees the notes
and still weights the body at loop_depth 2. Measured: 36 insns, score 6, with
ALL NINE registers on target (s2/p4.txt). This is the load-bearing trick of the
whole match.

### The residual 6 and its cause — the sched1 note barrier
The only defect left at score 6 was prologue ORDER: the three single-set
constant loads (`li -1 / li 3 / li 1`) were emitted BELOW the `link` and `tbl`
cursor initialisers instead of above them. `.combine` still had them in
declaration order and `.lreg` did not, so cc1's FIRST-PASS SCHEDULER moves
them. The distinguishing property is single-set vs multi-set: the four pseudos
that are also written inside the loop (slot, i, link, tbl) stayed put; the
three that are set exactly once sank.

`sched.c:2068-2094` is the lever: "If there is a LOOP_{BEG,END} note in the
middle of a basic block, then we must be sure that no instructions are
scheduled across it. Otherwise, the reg_n_refs info (which depends on
loop_depth) would become incorrect." — the first insn after EITHER note gets a
dependence on every preceding set and use, i.e. a hard scheduling barrier.
Wrapping the three constant assignments in their own `do { } while (0)`
therefore pins `none` (after the BEG note) and `link` (after the END note),
which reproduces target's prologue order exactly. Measured: **score 0**.

### Verified allocno arithmetic for the closing form (predicted, then measured)
Weights: everything outside the two wrap regions counts 1, everything inside
counts 2. id copy 6/3 = 4.00 -> $v1(3); ent 22/22 = 4.00, loses the tie because
`id` is declared first and so owns the lower allocno index -> $a1(5); slot
8 refs/31 = 0.774 -> $a2(6); tbl 7/23 = 0.609 -> $a3(7); link 7/24 = 0.583 ->
$t0(8); i 7/28 = 0.5 -> $t1(9); arg0 7/29 = 0.483 -> $a0(4, preferred); one
4/50 = 0.160 -> $t2(10); kind 4/52 = 0.154 -> $t3(11); none 4/54 = 0.148 ->
$t4(12). Every one of those nine matches target. The declaration position of
`id` is load-bearing purely as a tie-break.

### Facts that kill parts of the inherited frontier
- F1 is dead as stated: expressing the 0x90C cursor as `slot + 0x58` does NOT
  give the 0x8B4 cursor a sixth reference — cse folds it back to
  `arg0 + 0x90C` before flow.c counts, and `.lreg` still says "used 5 times
  across 31 insns" (score 21, unchanged). Any prologue-level cursor-relative
  respelling folds for the same reason. Banked as
  `rejected/ent-derived-from-slot-cse-folds-no-sixth-ref.c`.
- F3's stated mechanism is wrong: a struct-typed cursor cannot help, because
  RTL only ever sees `(plus (reg) (const_int))` for `p->field` — identical to
  the cast form. Strength reduction is defeated by making the region phony, not
  by typing.
- The three constant-holder locals remain load-bearing (s1 H5 stands: literals
  give 35 insns / score 23) because the phony region also means no LICM. They
  carry the `named-local-fake-exception` FAKE annotation in the candidate.

### s2 artifacts
- `tmp/grind/func_80040CB8/s2/splice.py` — splice a candidate body into
  src/text1a.c between the kengo marker and the next typedef (LF-safe).
- `tmp/grind/func_80040CB8/s2/order.py` — print block-0 insn order + signature
  for func_80040CB8 out of any cc1 `-da` dump (this is what identified sched1
  as the pass that sinks the constant loads).
- `b_base.c` (13), `b_p1.c` (21), `b_p2.c` real for-loop (25), `b_p3.c`
  do-while(0) around body (25), `b_p4.c` ent-init inside region (6),
  `b_p5.c` two regions (0), `b_final.c` (0, annotated — this is what is in src).
- `p1.txt p2.txt p3.txt p4.txt p5.txt` — disassemblies; `text1a.i.*` cc1 `-da`
  dumps for the p4 form (`.loop` carries the "phony" line, `.lreg`/`.greg` the
  allocno table and dispositions).
