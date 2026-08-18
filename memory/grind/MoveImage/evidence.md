# EVIDENCE — MoveImage (src/display.c:169)

## Session 1 (recon, 2026-08-11)

### Baseline
- `canonical MoveImage` → verdict **C**, asm_insns 0, total 49, distance 21
  ("pure-C distance 21 <= 50 — pure-C target"). Not an asm-routing candidate.
- `sandbox MoveImage --disable all` at session start → **score 21**,
  target_insns 49, **build_insns 47**, 19 regfix rules dropped.
- The 19 regfix rules (regfix.txt:2461-2485) are: 2 `insert_after` + 1
  `insert_label` reconstructing a `j .L8007B770 / addiu $v0,$zero,-1` pair and
  a `.LbodyB6C8` label, 9 register `subst`s, 2 `delete`s, 1 `insert`, and one
  16-index `reorder 26,29,21,22,23,24,30,25,28,32,33,27,31,34,35,36 @ 21-36`.
  Read as a fingerprint: the cheats were papering over (a) a missing
  two-instruction inverted-guard tail and (b) a whole-block scheduling +
  register-rename divergence. Both are now understood; (a) is CLOSED.

### End-of-session state
- **Floor 21 → 7.** `build_insns` 47 → **49 == target_insns 49**.
- Best form is in `src/display.c` as edited AND saved to `candidate.c`.

### What the function is
A PsyQ libgpu `MoveImage(RECT *rect, int x, int y)` reimplementation. It calls
the debug/trace helper `func_8007B3A8(&D_80015F74, rect)`, rejects zero width
(`((s16*)arg0)[2]`) or zero height (`((s16*)arg0)[3]`) with `-1`, fills a
three-word GPU move primitive at D_8009BF24 / D_8009BF28 / D_8009BF2C
(srcXY / dstXY / WH), then tail-calls `((s32(*)())g_gpu_dev_table[2])` with
`g_gpu_dev_table[6]`, the primitive base `&D_8009BF24 - 8`, `0x14`, `0`.
The `-8` is why `&D_8009BF24` is materialized into a register at all: target
keeps that pointer in `$a1` all the way to the `jalr` delay slot, where it is
decremented in place (`addiu $a1,$a1,-0x8`).

### THE LOAD-BEARING FINDING (session 1)
Moving `D_8009BF28 = packed;` to be the FIRST of the three primitive stores
drops the floor 21 → 7 **and closes the 2-instruction shortfall on its own.**

Mechanism, end to end:
1. With the BF28 store first, the packed-value chain
   (`sll $v0,$s1,16` → `andi $v1,$s2,0xFFFF` → `or`) becomes the top-priority
   work at the head of the post-guard basic block, so `sll $v0,$s1,16` is the
   FIRST instruction of that block.
2. cc1's delay-branch pass (`reorg.c`, `fill_eager_delay_slots` →
   `fill_slots_from_thread`) can then steal that first instruction from the
   branch's TAKEN thread. Stealing from the taken thread only makes sense when
   the branch points at the body, so the guard is emitted in its
   un-collapsed, inverted form:
      `bnez $v0,.L720` (delay: `sll $v0,$s1,16`)
      `j <epilogue>`   (delay: `addiu $v0,$zero,-1`)
   which is byte-for-byte target's shape and supplies the 2 missing insns.
3. Knock-on: `packed` now stays live in `$v0` four instructions longer, so the
   `rect[0]` read is allocated `$a0` — target's register — instead of reusing
   `$v0`. Several of the 9 register `subst` regfix rules were papering over
   exactly this.

The corollary matters for the next session: **the guard's branch shape is NOT
controlled by how the guard is spelled — it is a downstream consequence of what
instruction sits at the top of the body block.** Three different guard
spellings all produced byte-identical output (see hypotheses.md H1), and one
body-order change flipped it.

### Compiler-source facts established (cited, not guessed)
- `tools/gcc-2.7.2/jump.c:1764-1784` — "Detect a conditional jump jumping over
  an unconditional jump": collapses `bnez L2 / j Lend / L2:` into
  `beqz Lend` only when `prev_active_insn (reallabelprev) == insn` and
  `no_labels_between_p (insn, reallabelprev)` and `simplejump_p (reallabelprev)`.
  So an active insn sitting between the conditional branch and the `j`
  (e.g. the `li $v0,-1`) blocks the collapse at jump.c time.
- `tools/gcc-2.7.2/reorg.c:3877-3910` — the same peephole again in
  `relax_delay_slots`, guarded by `GET_CODE (next) == JUMP_INSN &&
  simplejump_p (next)`: once the `j` has acquired a delay slot it is a
  SEQUENCE, so this collapse no longer fires.
- `tools/gcc-2.7.2/reorg.c:4080-4118` — the SEQUENCE-form variant, additionally
  gated on `! INSN_ANNULLED_BRANCH_P (delay_insn)` and
  `redirect_with_delay_slots_safe_p`.
- `tools/gcc-2.7.2/reorg.c:1377-1385` (`mostly_true_jump`) — a branch whose
  label carries `LABEL_OUTSIDE_LOOP_P` is scored `-1` (highly unlikely), which
  suppresses eager fill from the taken thread. This is the mechanism the
  `do { } while (0)` carve-out exploits; **it was NOT needed here** and must not
  be reached for while ordinary statement-order levers remain (see H4).

### Residual at floor 7 (positional diff, post-guard block)
target                              | ours (floor 7)
------------------------------------|-------------------------------
andi $v1,$s2,0xFFFF                 | same
or   $v0,$v0,$v1                    | same
lui  $a1,%hi(D_8009BF24)            | same
addiu $a1,$a1,%lo(D_8009BF24)       | same
lw   $a0,0x0($s0)                   | lui  $at,%hi(D_8009BF28)
lui  $v1,%hi(D_8009BE6C)            | sw   $v0,%lo(D_8009BF28)($at)
lw   $v1,%lo(D_8009BE6C)($v1)       | lw   $v0,0x0($s0)
addiu $a2,$zero,0x14                | lui  $v1,%hi(D_8009BE6C)
lui  $at,%hi(D_8009BF28)            | lw   $v1,%lo(D_8009BE6C)($v1)
sw   $v0,%lo(D_8009BF28)($at)       | addiu $a2,$zero,0x14
sw   $a0,0x0($a1)                   | sw   $v0,0x0($a1)
lw   $v0,0x4($s0)                   | lw   $a0,0x18($v1)
addu $a3,$zero,$zero                | lw   $v0,0x4($s0)
lui  $at,%hi(D_8009BF2C)            | addu $a3,$zero,$zero
sw   $v0,%lo(D_8009BF2C)($at)       | lui  $at,%hi(D_8009BF2C)
lw   $a0,0x18($v1)                  | sw   $v0,%lo(D_8009BF2C)($at)
lw   $v0,0x8($v1)                   | same
nop                                 | same
jalr $v0                            | same
addiu $a1,$a1,-0x8  (delay)         | same

i.e. **one displacement**: our two-instruction BF28 store group
(`lui $at` + `sw`) is emitted FOUR slots too early — target puts the
`rect[0]` load, the `g_gpu_dev_table` load and `addiu $a2,$zero,0x14` ahead of
it. The `$a0`-vs-`$v0` rename on the rect[0] read is a CONSEQUENCE of that
displacement (whoever holds `packed` longer wins `$v0`), not an independent
problem — fix the displacement and the rename should fall out.

### Measured probe table (all this session, sandbox --disable all)
| # | form | score | build_insns |
|---|---|---|---|
| baseline | two separate `if (... == 0) return -1;`, stores BF24/BF28/BF2C | 21 | 47 |
| A | second guard inverted: `if (h != 0) { body; return ...; } return -1;` | 21 | 47 (byte-identical to baseline) |
| B | mixed exit forms: `if (w==0) goto err;` + trailing `err: return -1;` | 21 | 47 (byte-identical to baseline) |
| C | `if (w == 0 \|\| h == 0) return -1;` | 21 | 47 (byte-identical to baseline) |
| **D/F** | **C + `D_8009BF28 = packed;` hoisted to first store** | **7** | **49** |
| E | D + `p = g_gpu_dev_table;` hoisted above the stores, `fn = p[2]` left below | 9 | 49 |
| G | D + named local `src = arg0[0];` placed before the BF28 store | 21 | 47 (branch shape LOST) |
| H | D + both `p =` and `fn = p[2]` hoisted above the stores | 10 | 48 |
| I | D + `p = g_gpu_dev_table;` placed between the BF28 and BF24 stores | 9 | 49 |

G is the most informative negative: putting ANY instruction that outranks the
`sll` at the head of the body block destroys the delay-slot steal and takes the
2-instruction shortfall back. Any future reordering must be checked against
`build_insns == 49`, not just the score.

- [s1] canonical MoveImage: verdict C, asm_insns 0, total 49, distance 21 — a pure-C target, not an asm-routing candidate.

- [s1] Session start floor: sandbox --disable all = 21, target_insns 49, build_insns 47, 19 regfix rules dropped. Session end floor: 7, build_insns 49 == target_insns 49, edits live in src/display.c.

- [s1] MoveImage is a PsyQ libgpu MoveImage(RECT*, int x, int y): debug-trace call to func_8007B3A8(&D_80015F74, rect), `-1` on zero width ((s16*)arg0)[2] or zero height ((s16*)arg0)[3], fills a three-word GPU move primitive at D_8009BF24 (srcXY) / D_8009BF28 (dstXY) / D_8009BF2C (WH), then tail-calls ((s32(*)())g_gpu_dev_table[2])(g_gpu_dev_table[6], &D_8009BF24 - 8, 0x14, 0).

- [s1] The `-8` is why &D_8009BF24 is materialized into a register at all: target holds that pointer in $a1 from the `lui/addiu` all the way to the `jalr` delay slot, where it is decremented in place (`addiu $a1,$a1,-0x8`).

- [s1] THE LEVER: statement order alone. Making `D_8009BF28 = packed;` the first of the three stores drops the floor 21 -> 7 AND closes the entire 2-instruction shortfall. No FAKE construct, no dead store, no pin, no volatile, no unused declaration is involved — the 7-floor form is ordinary program logic with every local read.

- [s1] The 19 regfix rules read as a fingerprint of exactly the two problems found: regfix.txt:2463-2465 (2 insert_after + 1 insert_label) were reconstructing the missing `j .L8007B770 / addiu $v0,$zero,-1` pair and its `.LbodyB6C8` label — that half is now produced by the compiler; the 9 register substs + `reorder 26,29,21,22,23,24,30,25,28,32,33,27,31,34,35,36 @ 21-36` were papering over the store-cluster schedule that is the remaining 7.

- [s1] GCC source facts established by reading, not guessing: jump.c:1764-1784 is the invert-around-unconditional-jump peephole and its three guards; reorg.c:3877-3910 is the same peephole in relax_delay_slots, gated on `GET_CODE (next) == JUMP_INSN && simplejump_p (next)` so it stops firing once the `j` owns a delay slot; reorg.c:4080-4118 is the SEQUENCE variant gated on `! INSN_ANNULLED_BRANCH_P` and `redirect_with_delay_slots_safe_p`; reorg.c:1377-1385 (mostly_true_jump) is where LABEL_OUTSIDE_LOOP_P scores a branch -1 and suppresses eager fill.

- [s1] That last point matters as a NEGATIVE: the do-while(0) / LABEL_OUTSIDE_LOOP_P carve-out is the documented way to steer this exact peephole, and it was NOT needed — ordinary statement order reached the shape. Any future session reaching for that carve-out here would fail its lever-exhaustion prerequisite on the record.

- [s1] Residual at floor 7 is ONE displacement in the post-guard block: our two-instruction D_8009BF28 store group (`lui $at,%hi` + `sw $v0,%lo`) is emitted four slots too early — target puts `lw $a0,0x0($s0)` (rect[0]), `lui $v1,%hi(D_8009BE6C)` / `lw $v1,%lo(D_8009BE6C)($v1)` and `addiu $a2,$zero,0x14` ahead of it. The $a0-vs-$v0 rename on the rect[0] read is a consequence of that displacement (whoever holds `packed` longer wins $v0), not an independent defect.

- [s1] STANDING CONSTRAINT for every future session: score alone is not a valid report for this function. A form scoring under 21 with build_insns == 47 has lost the delay-slot steal and is a dead end. Always report and check build_insns == 49.

## Session 2 (structural, 2026-08-11)

### Start-of-session correction
`src/display.c` was at the BASELINE form, NOT session 1's 7-floor form — s1's
edits did not survive into s2's tree. The first act of any session on this
function must be to APPLY `memory/grind/MoveImage/candidate.c` to
`src/display.c` and re-measure; the ledger's floor is only real once the
candidate is in place. (Re-measured floor 7 / build_insns 49 after applying.)

### FLOOR 7 -> 2 (build_insns stays 49)
The lever is a WALKING POINTER over the rect argument:
```c
rect = arg0;
src = *rect++;      /* rect[0] read AHEAD of the D_8009BF28 store */
D_8009BF28 = packed;
*bf24 = src;
D_8009BF2C = *rect; /* rect[1] read AFTER it */
```
This puts `lw $a0,0x0($s0)` at target's slot 5 (ahead of the whole BF28 store
group) and hands the rect[0] value `$a0` — target's register — closing 5 of the
7. Session 1's probe G had shown that hoisting the rect[0] read ahead of the
store through a PLAIN named local (`src = arg0[0];`) destroys the H2
delay-slot steal (21 / 47). Session 2 re-measured that (W1/W2/W3: all 21 / 47,
under three placements) and then found that the post-increment pointer spelling
does the same hoist WITHOUT paying that price (W10: 2 / 49). So the plain-local
and walking-pointer hoists are NOT equivalent — the walking pointer is the one
that works, and that difference is the single most valuable fact from s2.

### Residual at floor 2 — ONE instruction's position
```
target                       | ours (floor 2)
-----------------------------|---------------------------
sw   $a0,0x0($a1)            | sw   $a0,0x0($a1)
lw   $v0,0x4($s0)            | lw   $a0,0x18($v1)   <-- p[6], 4 slots early
addu $a3,$zero,$zero         | lw   $v0,0x4($s0)
lui  $at,%hi(D_8009BF2C)     | addu $a3,$zero,$zero
sw   $v0,%lo(D_8009BF2C)($at)| lui  $at,%hi(D_8009BF2C)
lw   $a0,0x18($v1)           | sw   $v0,%lo(D_8009BF2C)($at)
lw   $v0,0x8($v1)            | lw   $v0,0x8($v1)
```
Every other instruction in the function is byte-identical to target.

### THE MECHANISM, read out of cc1's own scheduler dump (not inferred)
`tmp/grind/MoveImage/s2/dumps/display.i.sched2`, basic block 3 (the post-guard
body). cc1 2.7.2's sched.c is a BACKWARD list scheduler: it fills the block
from the last slot (T-1) upward, picking the highest INSN_PRIORITY ready insn,
and `priority()` is the longest TRUE-dependence path from the block START to
that insn (anti/output links contribute cost 0, so they cannot raise a
priority). Dumped priorities for our build:

| insn | what | priority |
|---|---|---|
| 54 | `sll $v0,$s1,16` | 1 |
| 68 | `lw $a0,0x0($s0)`  (rect[0]) | 1 |
| 72 | `sw %lo(D_8009BF28)` | 1 |
| 75 | `sw 0x0($a1)` (BF24) | 2 |
| 78 | `lw $v0,0x4($s0)`  (rect[1]) | 2 |
| 80 | `sw %lo(D_8009BF2C)` | **3** |
| 83 | `lw $v1,%lo(D_8009BE6C)` (dev table) | 1 |
| 86 | `lw $v0,0x8($v1)` (fn) | 3 |
| 91 | `lw $a0,0x18($v1)` (p[6]) | **2** |
| 99 | the call | 4 |

The whole residual is ONE pick: at T-5 the ready list is
`80 (3) 91 (2) 97 (1) 95 (1)` and the scheduler takes 80. Target takes 91
there. So target's build must have `priority(91) >= priority(80)`.

Both numbers are structurally pinned in our form:
- `priority(80) = 3` because the BF2C store's value comes from insn 78, the
  rect[1] load, which is depth 2 — it carries a true memory dependence on the
  D_8009BF28 store (an unknown-base `0x4($s0)` load cannot be disambiguated
  from a SYMBOL_REF store). Lowering it to 2 requires reading rect[1] BEFORE
  the BF28 store — which is precisely the hoist that costs the delay-slot
  steal (10 measured forms, all build_insns 48; banked in
  `rejected/rect1-read-hoisted-above-bf28-store.c`).
- `priority(91) = 2` because the p[6] load's only true predecessor is insn 83,
  the dev-table load, which is depth 1 (nothing feeds it: cc1 DOES disambiguate
  `%lo(D_8009BE6C)` from the BF28/BF2C symbol stores, and it also disambiguates
  the `0x0($a1)` store because `$a1` carries a REG_EQUIV to `&D_8009BF24`).
  91's anti-dependence on insn 75 (both touch `$a0`) contributes cost 0 and
  therefore cannot lift it.

So the two knobs are mutually exclusive on the statement-order axis, which is
what makes floor 2 a genuine plateau for THIS modality rather than a plateau
for the function.

### What was measured this session (six scripted sweeps, ~50 forms)
All gated on `build_insns == 49`; full tables in
`tmp/grind/MoveImage/s2/sweep{,2,3,4,5,6}_results.md`.
- sweep 1 (15 forms — store order, pointer materialization, split init, decl
  order, sibling convention): every form either 7/49 (inert) or worse. The
  7-floor form is a broad flat basin: 10 of 15 were byte-identical.
- sweep 2 (12 forms — the rect[0]-read-ahead axis): W10 walking pointer 2/49;
  plain-local hoists 21/47; variable-reuse forms 17-19/48-50.
- sweep 3 (15 forms — dispatch tail): 14 of 15 EXACTLY 2/49. The tail is inert.
- sweep 4 (14 forms — type / signature views incl. the s16* RECT view and the
  LoadImage/StoreImage sibling spelling): 10 of 14 exactly 2/49; none better.
- sweep 5 (11 forms — the sched-priority levers): lever (a) uniformly 48 insns.
- sweep 6 (9 forms — dev-table read placement + call spelling): 7 of 9 at 2/49.

- [s2] Session-2 floor: 7 -> 2, build_insns 49 == target 49. Lever: walk the rect argument with a post-increment pointer (`rect = arg0; src = *rect++;`) so the rect[0] read sits AHEAD of the D_8009BF28 store and the rect[1] read after it.

- [s2] The plain-local hoist and the walking-pointer hoist are NOT interchangeable. `src = arg0[0];` placed before the BF28 store measured 21/47 in three separate placements (destroys the H2 delay-slot steal, exactly as session 1's probe G reported); `rect = arg0; src = *rect++;` in the same position measured 2/49. Session 1's H4 note that "the naive way destroys the steal" was right about the naive way and wrong to generalize it to the whole read-ahead axis.

- [s2] The floor-2 residual is ONE instruction's schedule slot: `lw $a0,0x18($v1)` (the p[6] call argument) is emitted immediately after `sw $a0,0x0($a1)` where target emits it after the D_8009BF2C store, four slots later. Every other instruction in MoveImage is byte-identical to target.

- [s2] Read out of cc1's .sched2 dump (tmp/grind/MoveImage/s2/dumps/display.i.sched2, block 3): cc1 2.7.2's sched.c schedules a block BACKWARD from the last slot, picking the highest INSN_PRIORITY ready insn, where priority() is the longest TRUE-dependence path from the block start (anti/output dep links cost 0). At T-5 the ready list is `80 (3) 91 (2) 97 (1) 95 (1)` and it picks insn 80, the D_8009BF2C store; target picks insn 91, the p[6] load. That single pick IS the whole residual.

- [s2] priority(80) = 3 is forced: the BF2C store's value is insn 78, the rect[1] load, which is depth 2 because an unknown-base `0x4($s0)` load carries a true memory dependence on the preceding `%lo(D_8009BF28)` symbol store. The only way to make it depth 1 is to read rect[1] before that store — measured 10 ways, all build_insns 48 (delay-slot steal lost).

- [s2] priority(91) = 2 is forced: the p[6] load's only true predecessor is the dev-table load (insn 83), which is depth 1 — cc1 successfully disambiguates `%lo(D_8009BE6C)` from both symbol stores AND from the `0x0($a1)` store, because $a1 carries a REG_EQUIV to `&D_8009BF24`. 91's anti-dependence on insn 75 (shared $a0) contributes cost 0 and cannot lift the priority.

- [s2] STANDING CORRECTION for every future session: `src/display.c` did NOT carry session 1's edits at session-2 start — it was at the raw baseline. ALWAYS apply memory/grind/MoveImage/candidate.c to src/display.c and re-measure before probing, and never trust the inherited floor until sandbox has printed it this session.

- [s1] Full measured probe table (sandbox --disable all): baseline two-ifs 21/47; A inverted guard 21/47 byte-identical; B goto-err mixed exits 21/47 byte-identical; C `||` 21/47 byte-identical; D/F BF28-store-first 7/49; E dev-table ptr hoisted 9/49; G named src local before BF28 store 21/47; H dev-table fully hoisted 10/48; I `p =` between stores 9/49.

- [s2] src/display.c was at the RAW BASELINE at session-2 start - session 1's edits did not survive into this session's tree. The first act of any session on this function must be to apply memory/grind/MoveImage/candidate.c to src/display.c and re-measure; the inherited floor is not real until sandbox prints it this session. (Re-measured 7 / 49 after applying, then 2 / 49 after the session-2 lever.)

- [s2] FLOOR 7 -> 2, build_insns 49 == target_insns 49. The lever is a walking pointer over the rect argument: `rect = arg0; src = *rect++;` before the D_8009BF28 store, `D_8009BF2C = *rect;` after it. Ordinary program logic - no dead stores, no pins, no volatile, no unused declarations; packed, bf24, rect, src, p and fn are all written and read.

- [s2] The plain-local hoist and the walking-pointer hoist are NOT interchangeable: `src = arg0[0];` placed ahead of the BF28 store measured 21/47 in three separate placements (delay-slot steal destroyed, reproducing session 1's probe G), while `rect = arg0; src = *rect++;` in the same position measured 2/49. This is the single most transferable fact from the session and likely generalizes to other functions where a read must be hoisted past a store without disturbing block-head priority.

- [s2] The floor-2 residual is ONE instruction's schedule slot: `lw $a0,0x18($v1)` (the p[6] call argument) is emitted immediately after `sw $a0,0x0($a1)`, whereas target emits it after the D_8009BF2C store, four slots later. Every other instruction in MoveImage is byte-identical to target.

- [s2] Read directly out of cc1's own scheduler dump (tmp/grind/MoveImage/s2/dumps/display.i.sched2, basic block 3): cc1 2.7.2's sched.c fills a block BACKWARD from the last slot, picking the highest INSN_PRIORITY ready insn, and priority() is the longest TRUE-dependence path from the block start - anti/output dependence links have insn_cost 0 and cannot raise a priority. At T-5 the ready list is `80 (3) 91 (2) 97 (1) 95 (1)` and cc1 picks insn 80 (the %lo(D_8009BF2C) store); target picks insn 91 (the p[6] load). That single pick IS the entire residual, so target's build satisfies priority(91) >= priority(80).

- [s2] Dumped priorities for our floor-2 build, block 3: insn 54 sll = 1, insn 68 rect[0] load = 1, insn 72 BF28 store = 1, insn 75 BF24 store = 2, insn 78 rect[1] load = 2, insn 80 BF2C store = 3, insn 83 dev-table load = 1, insn 86 fn load = 3, insn 91 p[6] load = 2, insn 99 call = 4.

- [s2] priority(80) = 3 is structurally forced: the BF2C store's value is the rect[1] load, which is depth 2 because an unknown-base `0x4($s0)` load carries a true memory dependence on the preceding %lo(D_8009BF28) symbol store. The only way to make it depth 1 is to read rect[1] before that store, and all 10 measured spellings of that hoist cost the delay-slot steal (build_insns 48).

- [s2] priority(91) = 2 is structurally forced on the C axis: the p[6] load's only true predecessor is the dev-table load, which is depth 1 - cc1 successfully disambiguates %lo(D_8009BE6C) from both symbol stores and from the `0x0($a1)` store, because $a1 carries a REG_EQUIV to &D_8009BF24. 91's anti-dependence on insn 75 (both touch $a0) contributes cost 0.

- [s2] The two knobs are mutually exclusive on the statement-order axis, which is exactly why floor 2 is a plateau for the STRUCTURAL modality and not for the function: only one of the two rect reads can be hoisted past the BF28 store, and the p[6] load's depth cannot be lifted from the tail.

- [s2] Approximately 50 distinct forms were measured across six scripted sweeps, every one gated on build_insns == 49 (a form scoring below 21 with build_insns 47 or 48 has lost the H2 steal and is a dead end regardless of score). Full tables: tmp/grind/MoveImage/s2/sweep{,2,3,4,5,6}_results.md; per-variant objdumps in tmp/grind/MoveImage/s2/asm{2,3,4,5,6}/.

- [s2] Floor-2 form is live in src/display.c and saved to memory/grind/MoveImage/candidate.c. Two disproven families are banked: rejected/rect1-read-hoisted-above-bf28-store.c (10 forms, all build_insns 48) and rejected/tail-respellings-inert-at-floor-2.c (29 forms, all inert or worse).

## Session 3 (structural, 2026-08-11) — measured facts

* The instrumented cc1 (`tools/gcc-2.7.2/cc1`, `BB2_PRIO_DEBUG=1`) reproduces
  MoveImage's dependence graph exactly from a 34-line standalone file
  (`tmp/grind/MoveImage/s3/mv.c`), with the SAME insn UIDs as the whole-file
  dump.  This makes the priority graph a one-command instrument for this
  function: tmp/grind/MoveImage/s3/mv.prio (floor-2 form) and mv2.prio
  (plain-argument form).
* `sched.c:1497` — `contrib = priority(pred) + insn_cost(...) - 1`.  insn_cost
  is 2 only for a load feeding a true dependence; anti (kind 14) and output
  (kind 15) links cost 1, so they do not add depth but DO propagate the
  predecessor's priority.  The s2 ledger's "anti/output cost 0" is corrected.
* Floor-2 priorities: rect[0] load 1, D_8009BF28 store 1, packet store 2,
  rect[1] load 2, D_8009BF2C store 3, dev-table load 1, `fn` load 3, p[6] load 2.
* The rect[1] load's depth comes from the PACKET store (a pointer store), not
  from the D_8009BF28 symbol store: symbol stores are alias-exempt from the
  `mem/s` refs and merely anti-ordered against the plain ones.
* `expr.c:4567-4577` marks an INDIRECT_REF with a PLUS_EXPR operand as
  MEM_IN_STRUCT_P.  `p[6]` -> `mem/s`; `*q` after `q = p + 6` -> plain `mem`.
  `sched.c:834-839` exempts a (MEM_IN_STRUCT && varying) ref from a
  (non-struct && fixed) ref — this is why `p[6]` never depends on the
  `%lo(D_8009BF2C)` store and floats 4 slots early.
* Making the dispatch argument read plain puts `lw $a0,0x18($v1)` at target's
  exact slot and lifts its priority to 3 via a true dependence on the
  D_8009BF2C store (`insn=94 pred=80 kind=0`).  Score 4 / 49; the residual
  becomes the dev-table pair being 4 slots late.
* `canon_rtx` (sched.c:370-377) + `init_alias_analysis` (sched.c:421-433):
  a pointer pseudo is resolved to its symbol only via a REG_EQUAL note with
  `reg_n_sets == 1` or a REG_EQUIV note; hard registers are never given known
  values (the loop requires `REGNO >= FIRST_PSEUDO_REGISTER`).
* Target's own emission order (asm/funcs/MoveImage.s 8007B720-8007B76C) proves
  its build has NO memory dependence into the dev-table load: the load pair sits
  5 instructions ahead of the packet store.
* 41 forms measured this session (sweeps 7-11), all gated on build_insns:
  8 alias-exemption forms on the rect (all 47/48), 9 opaque/reorder forms
  (9-20), 6 inter-store-window forms (all 48), 9 plain-dispatch-read forms
  (4 or 7 at 49), 11 dev-table placements (10 inert at 4 / 49).

- [s3] The s2 ledger's claim that anti/output dependence links have insn_cost 0 and cannot raise a priority is WRONG and is corrected in the ledger: sched.c:1497 computes contrib = priority(pred) + insn_cost(...) - 1, insn_cost is 2 only for a load feeding a true dependence, and anti (kind 14) / output (kind 15) links cost 1 -- they do not add depth but they DO propagate the predecessor's priority (the dump carries `insn=86 pred=80 kind=14 pred_pri=3 cost=1 contrib=3`).

- [s3] Measured floor-2 priorities (BB2_PRIO_DEBUG=1): rect[0] load 1, D_8009BF28 store 1, packet store 2, rect[1] load 2, D_8009BF2C store 3, dev-table load 1, fn load 3, p[6] load 2.

- [s3] The rect[1] load's depth comes from the PACKET store (a pointer store), NOT from the D_8009BF28 symbol store -- the s2 ledger had this backwards, which is why its 'hoist above the BF28 store' framing tested the wrong boundary.

- [s3] expr.c:4567-4577 marks an INDIRECT_REF whose operand is a PLUS_EXPR as MEM_IN_STRUCT_P; sched.c:834-839 exempts such a ref (varying address, non-QImode) from ever conflicting with a non-struct fixed-address ref. Together these are why `p[6]` never depends on the `%lo(D_8009BF2C)` store while `*rect` does depend on the packet store.

- [s3] A 34-line standalone repro (tmp/grind/MoveImage/s3/mv.c) reproduces MoveImage's dependence graph with the SAME insn UIDs as the whole-file dump, so the priority graph is now a one-command instrument for this function.

- [s3] On the plain-argument base the dev-table load acquires a true memory dependence on the packet store (mv2.prio: `insn=83 pred=75 kind=0 pred_pri=2`, final_pri=2) that is absent in the floor-2 build (mv.prio: only two anti/output preds, final_pri=1) -- even though the RTL of insns 61, 75 and 83 is textually identical between the two builds. That unexplained edge is the last blocker.

- [s3] canon_rtx/init_alias_analysis (sched.c:370-377, 421-433) install known values only for pseudos with a REG_EQUAL note at reg_n_sets == 1 or a REG_EQUIV note; hard registers are never given known values (the loop requires REGNO >= FIRST_PSEUDO_REGISTER).

- [s3] 41 forms measured this session across sweeps 7-11, every one gated on build_insns as well as score; the standing gate is confirmed again -- any form at 47 or 48 instructions has lost the H2 delay-slot steal.

- [s3] src/display.c was returned to its HEAD state; the session's forms live in memory/grind/MoveImage/candidate.c (floor 2), candidate_alt_plain_arg.c (the 4/49 alternate base) and rejected/alias-exemption-and-opaque-pointer-forms.c.

## Session 4 (permuter, 2026-08-11) � measured facts

* `src/display.c` was again at the RAW BASELINE at session start (third session
  running: s1's, s2's and s3's edits do not persist). Applied
  `memory/grind/MoveImage/candidate.c` and re-measured **2 / 49** before probing.
  Returned `src/display.c` to its HEAD state at session end.
* A reusable permuter-workspace builder now exists:
  `tmp/grind/MoveImage/s4/mkws.sh <wsdir> <base.c>`. It emits settings.toml, a
  compile.sh running the exact Makefile pipeline (CC_FLAGS **including `-mel`**,
  prologue_fix, maspsx with every gate list, multu_pad, as), a clean
  single-function `target.o` from `asm/funcs/MoveImage.s` +
  `tools/decomp-permuter/prelude.inc` with `.set gp=64` stripped, and a
  validation diff. NOTE for reuse: `tools/mar_perm_workspace.sh` is STALE � it
  omits `-mel`, which has been load-bearing since 2026-08-04.
* Standalone fidelity is MEASURED: the standalone base reproduces each whole-TU
  residual exactly � ws_a (floor-2 candidate) 49 insns with the single
  `lw a0,24(v1)` displacement, ws_b (plain-argument candidate) 49 insns with the
  single dev-table `lui/lw` displacement.
* Permuter base scores: floor-2 candidate **225**, plain-argument candidate
  **415**. These are the permuter's weighted metric and are NOT comparable to the
  sandbox's 2 and 4 � note in particular that the permuter ranks the
  structurally-closer plain-argument base as much WORSE, because it weights the
  two displaced dev-table instructions above the one displaced load.
* Four campaigns, **~94,000 iterations total**, all launched through
  `tools/permuter_campaign.py` (telemetry) and all harvested with `--stop`
  in-session:
  | ws | chassis | base | iterations | best find |
  |---|---|---|---|---|
  | ws_a | random, floor-2 base | 225 | 52,580 | 210 (artifact) |
  | ws_b | random, plain-arg base | 415 | ~16,000 | 225 |
  | ws_c | PERM_LINESWAP x10 stmts, plain-arg base | 415 | 11,527 | 400 |
  | ws_d | PERM_LINESWAP x10 stmts + PERM_GENERAL over BOTH dispatch reads, floor-2 base | 225 | 30,462 | none |
* ws_b's random search from the plain-argument base never produced anything below
  225, i.e. it never found its way back even to the floor-2 base's quality; its
  best forms simply respell the dispatch argument as `*(p + 6)`, which is
  INDIRECT_REF(PLUS) again and therefore the floor-2 form by another name.
* ws_d is the important negative: it is exactly session 3's frontier items 2 and
  3 (the q/fn/dev-table placement cross-product, and the MEM_IN_STRUCT_P lever on
  the `fn = p[2]` read) enumerated mechanically against every statement ordering,
  and 30,462 samples produced ZERO forms at or below the base.
* The only sub-base class the permuter ever produced (210) is a narrow-load
  artifact: sweep12 measures P1/P2/P4 at 2 / 49 with a single-line objdump diff
  that is a WIDTH change at the SAME index (`lw` -> `lhu`/`lh`), and the
  wide-staging control P3 is byte-identical to the base.

- [s4] Permuter modality is now SPENT on this function: four campaigns, ~94,000 iterations, two bases, both a random and a directed (PERM_LINESWAP x PERM_GENERAL) chassis; not one form beat either base. All four campaigns were harvested with --stop inside the session.
- [s4] A reusable, fidelity-validated permuter workspace builder for MoveImage lives at tmp/grind/MoveImage/s4/mkws.sh; it uses the current Makefile flags including -mel, unlike the stale tools/mar_perm_workspace.sh, and its standalone base reproduces the whole-TU residual exactly on both bases.
- [s4] The permuter's weighted score DISAGREES with the sandbox's honest distance on which base is closer: the floor-2 candidate scores 225 and the structurally-closer plain-argument candidate scores 415, because two displaced dev-table instructions outweigh one displaced load. Do not use permuter score to choose between the two bases.
- [s4] The dispatch-argument load's WIDTH is steerable from C (u16/s16 staging or a narrow callee prototype all emit lhu/lh at the same index) but its SLOT is not -- confirming once more that the residual is a scheduler-placement problem and not a spelling problem.
- [s4] src/display.c was at the raw baseline at session-4 start for the third consecutive session; apply memory/grind/MoveImage/candidate.c and re-measure (2 / 49) before any probe.

- [s4] src/display.c was at the RAW BASELINE at session-4 start for the third consecutive session; memory/grind/MoveImage/candidate.c was applied and re-measured at 2 / 49 (build_insns 49 == target 49) before any probe, and src/display.c was returned to its HEAD state at session end.

- [s4] A reusable, fidelity-validated permuter-workspace builder for this function now exists: tmp/grind/MoveImage/s4/mkws.sh <wsdir> <base.c>. It emits settings.toml, a compile.sh running the exact Makefile pipeline (CC_FLAGS including -mel, prologue_fix, maspsx with every gate list, multu_pad, as), a clean single-function target.o from asm/funcs/MoveImage.s + tools/decomp-permuter/prelude.inc with `.set gp=64` stripped, and a validation diff.

- [s4] WARNING for reuse elsewhere: tools/mar_perm_workspace.sh is STALE -- its compile.sh omits -mel, which has been load-bearing since the 2026-08-04 adoption. mkws.sh is the current-flags template.

- [s4] Standalone fidelity is MEASURED, not assumed: the standalone base reproduces each whole-TU residual exactly -- ws_a (floor-2 candidate) 49 insns with the single `lw a0,24(v1)` displacement, ws_b (plain-argument candidate) 49 insns with the single dev-table `lui/lw` displacement.

- [s4] Permuter base scores: floor-2 candidate 225, plain-argument candidate 415. The permuter's weighted metric DISAGREES with the sandbox on which base is closer -- it ranks the structurally-closer plain-argument base far worse because two displaced dev-table instructions outweigh one displaced load. Do not use permuter score to choose between the two bases.

- [s4] Four campaigns, ~94,000 iterations total, all launched through tools/permuter_campaign.py (telemetry) and all harvested with --stop in-session: ws_a random/floor-2 base 225, 52,580 iters, best 210 (artifact); ws_b random/plain-arg base 415, ~16,000 iters, best 225; ws_c PERM_LINESWAP over 10 statements/plain-arg base, 11,527 iters, best 400; ws_d PERM_LINESWAP over 10 statements crossed with PERM_GENERAL over BOTH dispatch reads/floor-2 base, 30,462 iters, ZERO finds at or below base.

- [s4] ws_b's random search from the plain-argument base never produced anything below 225 -- it never found its way back even to the floor-2 base's quality, and its better forms simply respell the dispatch argument as `*(p + 6)`, which is INDIRECT_REF(PLUS) again and therefore the floor-2 form by another name.

- [s4] ws_d is the load-bearing negative: it is exactly session 3's frontier items 2 and 3 enumerated mechanically against every statement ordering, and 30,462 samples produced ZERO forms at or below the base. Those two frontier items are now measured dead rather than unmeasured.

- [s4] The dispatch-argument load's WIDTH is steerable from C (u16/s16 staging, or a narrow callee prototype, all emit lhu/lh at the same index) but its SLOT is not -- further confirmation that the residual is a scheduler-placement problem, not a spelling problem.

- [s4] No campaign outlived the session: `permuter_campaign.py status` reports every registered campaign with alive=false at session end, and all four carry an explicit stop_reason.

- [s4] Nothing in the session's diff touches src/ (restored to HEAD), regfix.txt, asmfix.txt, .claude/rules/, engine/, tools/, the Makefile or any *.ld. The diff is the MoveImage ledger plus metrics/events.jsonl.

## Session 5 (permuter, 2026-08-11) — measured facts

* Chassis E (`tmp/grind/MoveImage/s5/ws_e`, label `packetstore-directed`): the
  plain-argument base with PERM_LINESWAP over all ten body statements crossed
  with PERM_GENERAL at three packet-side sites (packet-pointer materialization
  x3, packet store x3, call packet address x4). 57,795 iterations / ~34 min /
  base score 415 / two finds (400, 410), both statement reorderings that put the
  packet store ahead of the D_8009BF28 store — the known build_insns-48 shape.
  ZERO finds at or below the floor-2 base's 225. Harvested with `--stop`
  in-session. The permuter axis for MoveImage is now spent across FIVE campaigns
  and ~152,000 iterations (s4: ws_a/ws_b/ws_c/ws_d; s5: ws_e).
* `g_gpu_dev_table` = 0x8009BE6C is READ-ONLY across the whole executable:
  `grep 8009BE6C asm/funcs/*.s` matches 17 files and every reference is a `lw`
  (`lui %hi` + `lw %lo`); there is no `sw` to that address in `asm/`, and no
  `g_gpu_dev_table = ...` assignment in any `src/*.c`.
* A const-qualified declaration of that global CLOSES MoveImage:
  - standalone repro `tmp/grind/MoveImage/s5/base_f.c` -> `ws_f`: validation diff
    EMPTY at 49 insns against `asm/funcs/MoveImage.s`;
  - real whole-TU build, `sandbox MoveImage --disable all`:
      plain-argument body, non-const decl .......... score 4, build_insns 49
      + block-scope `extern s32 *const g_gpu_dev_table;` .... score 0, 49
      + file-scope decl const'd (display.c:126) ............. score 0, 49
  - collateral at the file-scope spelling: ClearImage 0/37, ClearImage2 0/39,
    LoadImage 0/25, StoreImage 0/25 — no sibling regressed.
* The construct was NOT submitted as a candidate. It fails cheat tests T1 (no
  observable effect on emitted behaviour) and T3 (the mechanism I can state is
  `sched.c:828` RTX_UNCHANGING_P inside `true_dependence`), and no frozen
  sanctioned family covers a const/unchanging type claim — the nearest catalog
  family is the FORBIDDEN "volatile-coercion by plain extern", of which const is
  the mirror. Session 5 returned `ruling-request`; `src/display.c` is left in the
  bytes-proven state for the owner to re-verify.

## [s6] 2026-08-17 — psxsdk-adoption structural session (owner-authorized ONE session) — **MATCHED, COMPLETED-C**

Outcome: **MoveImage byte-matches in pure C with ZERO rules.** All 19 regfix rules
retired (`retire MoveImage` -> SHA1 62efab4f...), `sandbox MoveImage --disable all`
== 0 (49/49, rules_dropped 0), `canonical` verdict C distance 0, `queue done`
accepted (COMPLETED-C), `verify-oracle --rebuild` green on the final body.
The const/RTX_UNCHANGING lever was NOT used and is not needed.

### What closed it: the packet buffer is an ARRAY OBJECT, not three scalars
The refused const lever and every session-1..5 body treated 0x8009BF24/28/2C as
three unrelated `extern s32` scalars, with the packet address carried by a
pointer local (`bf24 = &D_8009BF24; ... (s32)bf24 - 8`). That pointer store
(`*bf24 = src`) is a VARYING, non-struct MEM: `canon_rtx` (sched.c:370-377) can
only resolve it to a symbol through `reg_known_value`, and when it fails the
store aliases every fixed-address symbol read -- including the `%lo(g_gpu_dev_table)`
load. That false true-dependence is what pinned the dev-table load 4 slots late
(the entire plain-argument residual, score 4) and what `const` was deleting at
sched.c:828.

Declaring the storage as what it is -- PsyQ's `u_long param[5]` blit packet at
0x8009BF1C, whose first two words are the initialized command words already
sitting in .data -- removes the pointer entirely: all three per-call writes
become ARRAY_REFs at fixed addresses, no varying store exists, the dependence
cannot form, and the dev-table load schedules into target's slot with no
qualifier anywhere. The residual did not have to be suppressed; it was an
artifact of the wrong object model.

### Evidence that this IS the object model (not a convenient spelling)
1. `tmp/sotn-decomp/src/main/psxsdk/libgpu/sys.c:269-279` -- the matched decomp of
   this same PsyQ source file: `u_long param[5]; param[0]=0x04FFFFFF;
   param[1]=0x80000000; param[2]=LOW(rect->x); param[3]=(u16)y<<16|(u16)x;
   param[4]=LOW(rect->w); return ...->addque2(...->cwc, param, sizeof(param), 0);`
2. BB2's .data at 0x8009BF1C/0x8009BF20 holds exactly `0x04FFFFFF` / `0x80000000`
   -- param[0]/param[1] verbatim, and splat had lumped them onto the tail of the
   unrelated 5-entry height table D_8009BF08 (asm/data/7D920.data.s:24180).
3. `0x14 == sizeof(u_long[5])` -- the size argument the call already passed.
4. Target's `addiu $a1,$a1,-0x8` is the compiler deriving `param` from the
   address of `param[2]`: cse.c materializes the FIRST store's address into a
   register and reaches the array base by a negative offset. Measured both ways
   -- with `param[3]` written first (v8) the register holds param+12 and the
   delay slot is `addiu a1,a1,-12`; with `param[2]` first (v10/v11) it holds
   param+8 and `-8`, target exactly. The "-8" nobody could motivate from three
   scalars falls straight out of the array declaration.

### The dev-table struct (lever 1) -- adopted, and byte-neutral by itself
`include/gpu.h` now declares the 0x40-byte PsyQ `gpu` struct (`GpuDevTable`,
member names/offsets from sys.c:6-22) and `extern GpuDevTable *g_gpu_dev_table;`;
the contradictory TU-local `extern s32 g_gpu_dev_table;` (display.c:126) is
deleted. Every index used anywhere in src/ maps onto a member exactly
(p[2]=addque2, p[3]=clr, p[5]=cwb, p[6]=cwc, p[7]=drs, p[8]=dws, p[0xB]=otc,
0x28/4=getctl, 0x10/4=ctl, p[0xD]=reset, p[0xE]=status, p[0xF]=sync) -- itself
independent confirmation of the layout.
MEASURED: the struct declaration ALONE does not close the function. `p->cwc`
behaves exactly like `p[6]`: COMPONENT_REF sets MEM_IN_STRUCT_P (expr.c:4888)
just as INDIRECT_REF(PLUS) does (expr.c:4567), so the floor-2 float returns
(v2 = 2). It is the packet-side array declaration that closes it; the struct is
what makes the closing body read as the original code.
The other 13 call sites keep a `(u32 *)` word view deliberately -- respelling
them as member accesses is a codegen change on already-matched bodies, noted in
the gpu.h comment.

### Variant grid (sandbox --disable all, all at build_insns 49 == target)
| v | body | score |
|---|---|---|
| HEAD | 3 scalars, pointer store, `p[6]` arg, 19 rules | 21 |
| v1 | plain-arg base (candidate_alt_plain_arg.c) reproduced | 4 |
| v1+hdr | same, with the GpuDevTable header adopted (cast form) | 4 |
| v2 | struct member dispatch `fn(p->cwc,...)`, scalars | 2 |
| v4 | 3-word array at D_8009BF24 + struct dispatch | 11 |
| v6/v7 | param[5] array, pointer store `*pkt`, struct dispatch | 7 (=4 true) |
| v8 | param[5], all-symbol stores, `param[3]` written FIRST | 12 |
| v9 | param[5], all-symbol stores, `pkt-2` call arg | 3 (=0 true) |
| v10 | v9 with store order [2],[3],[4] and `param` as the arg | **0** |
| v11 | v10 minus the walking rect pointer (plain `arg0[0]`/`arg0[1]`) | **0** |
| v12 | v11 with the packed word inlined into the store | 7 |
| v13 | v11 minus the `fn` local (`p->addque2(p->cwc,...)`) | **0** |
| v14 | v13 minus the `p` local (global dereferenced directly) — **ADOPTED** | **0** |
(v6-v9 scores carry up to 3 FALSE points: named-symbol HI16/LO16 addends are not
masked by engine/score.py, so `%lo(g_gpu_move_param)+8` scores as a diff against
target's `%lo(D_8009BF24)` even though the linked word is identical. The
instruction-by-instruction alignment for v9 was verified by hand
(tmp/grind/MoveImage/s6/mvdiff.py); once the symbol existed and the reference
object was rebuilt, the score reads a true 0.)

### Every prior lever is now GONE from the body
The final body carries NO session-1/2/3 construct: no packed-store-first
ordering, no walking rect pointer, no `q = p + 6` plain-deref dispatch, no
`bf24`/`pkt` pointer local, no `- 8`. What remains is PsyQ's own statement
sequence. The only named local is `packed` (inlining it costs 7 -- v12).

### Tree changes
* `asm/data/7D920.data.s` -- `dlabel g_gpu_move_param` at 0x8009BF1C (moves the
  `enddlabel D_8009BF08` up so that symbol is the 5-entry height table it always
  was); no emitted byte changes.
* `symbol_addrs.txt` -- `g_gpu_move_param = 0x8009BF1C;`
* `include/gpu.h` -- GpuDevTable struct + `extern GpuDevTable *g_gpu_dev_table;`
* `src/display.c` -- MoveImage body; the TU-local wrong-type extern deleted;
  `(u32 *)` casts at the untouched word-view sites.
* `src/gpu.c` -- `(u32 *)` casts at its five sites (byte-neutral: full-build SHA1).
* `regfix.txt` -- 19 MoveImage rules deleted by `retire`.

### Side probe (owner asked for it, hardens the const refusal for the record)
`extern int T[]; int *const p = T;` through the real cc1
(-O2 -G0 -mel, tmp/grind/MoveImage/s6/rdata_probe.*) emits
`.globl p / .section .rodata / p: .word T`, while an unqualified initialized
object emits `.data`. So a const-qualified `g_gpu_dev_table` would have been
placed in .rodata, contradicting its actual .data address at 0x8009BE6C --
the ruling's secondary argument is confirmed, not merely plausible.
