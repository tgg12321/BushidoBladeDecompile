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
