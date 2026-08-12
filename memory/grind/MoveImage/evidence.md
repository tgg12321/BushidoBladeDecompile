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

- [s1] Full measured probe table (sandbox --disable all): baseline two-ifs 21/47; A inverted guard 21/47 byte-identical; B goto-err mixed exits 21/47 byte-identical; C `||` 21/47 byte-identical; D/F BF28-store-first 7/49; E dev-table ptr hoisted 9/49; G named src local before BF28 store 21/47; H dev-table fully hoisted 10/48; I `p =` between stores 9/49.
