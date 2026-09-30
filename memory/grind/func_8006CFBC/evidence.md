# func_8006CFBC -- evidence

## 2026-09-29 -- REOPENED (retro-audit FAIL, Q37 class C, 803d0fea1)

The 5c79ba8f1 landing FAILed the 2026-09-29 retro-audit: union word view `Counts_8006CFBC { s32 word; s16 half[2]; }` landed 2026-09-20, before Q33 (2026-09-29); also the 3-write carrier `value` with no FAKE annotation or admitting ruling (score 8 without it). Per owner Q37 class C the body went back to `INCLUDE_ASM("asm/funcs", func_8006CFBC);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-29.c`. Removed with the body: the private Env_8006CFBC / Counts_8006CFBC typedefs (no other user). Kept: the extern block between them and the body (D_800A3524, D_800A34FC, g_gpu_ot_ptr, func_8007352C, func_8006E480, SetDrawMode).

## 2026-09-30 -- laneC session: both objections answered (candidate.c, sandbox 0)

Start: `rejected/retro-audit-2026-09-29.c` re-measured in its new TU (src/text1b_tu1c.c,
-G0, after the 6fd3fb9d3 split): sandbox 0 (218/218). `candidate.c` is that body with
two changes: the union's members are `s16 count[2]; s32 word;` (was `s32 word;
s16 half[2];`), and the carrier is `s8 *temp` (was `s32 value` with two casts), each
annotated. candidate.c: sandbox 0, 218/218.

### Q33 union (objection 1)
Owner ruling Q33 (2026-09-29) + the no-new-park-categories amendment (1)-(7):
- (1) one word access: the only site naming `word` is `counts.word = 0;` at the top of
  the outer loop; target 0x8006D018 `sw $zero,0x48($sp)` (asm/funcs/func_8006CFBC.s:26)
  covers counts' 4 bytes (sp+0x48..0x4B; s is sp+0x18..0x43). The build emits it
  (sandbox 0).
- (2) exactly the real object plus one word: `s16 count[2]` (two per-row sprite
  counters) + `s32 word`, both at offset 0; the store is the first byte; 4 bytes. No
  instruction reveals the word's signedness (only a zero store).
- (3) every other access is through count[]: 0x8006D0A0 `lh $a0,0x30($s0)`
  (`s.x = row * 280 + counts.count[row] * 23`), 0x8006D0D4 `lhu` + 0x8006D0E0 `sh`
  (`counts.count[row]++`), 0x8006D12C `lh $v0,0x30($v0)` (`counts.count[row] == 0`);
  $s0/$v0 = sp+0x18 + row*2, so 0x30 off them is sp+0x48 + row*2. lh => s16.
- (4) no casts, no constructor, not a parameter/return.
- (5) local: the union is the local's one declaration.
- Measured: without the word view (plain `s16 counts[2]`) the clear costs a store --
  q33/u_pair.c (`counts[0] = 0; counts[1] = 0;`) 2, 219/218; q33/u_chain.c
  (`counts[0] = counts[1] = 0;`) 2; q33/u_rev.c (reversed) 2; q33/u_loop.c (a loop) 46.
  The Q36 cast store is not needed (the union reaches the target).

### Ruling 11 `temp` (objection 2)
Full package in `r11/proof.md` (Rulings 5-10 ruled out, values, (B) path records, dumps,
mechanism, banked search, ablations, permuter). Short form: without a multi-write
destination, sched1's birth boost (sched.c:2584) puts each add below the header store
and local-alloc's combine_regs (local-alloc.c:1784) ties it to the dying header load
(score 8, every one-variable-per-value spelling banked in r11/spellings/ misses).

## 2026-09-30 -- LANDED COMPLETED-C (9813176b2, queue eef645a1b)
Layer-2 PASS, reviewer l2-8006CFBC-r1, round 1, body_hash 30c7eb47811df8a5, scope match
(memory/grind/func_8006CFBC/layer2.jsonl). Staged body = candidate.c minus its extern
block (the block already precedes the body in src/text1b_tu1c.c). Reviewer checked
Q33/Q46 against the asm (only word site 0x8006D018; two-store spellings 2), the whole
Ruling 11 package (twin differs only in declarations/identifiers, dumps of the exact body,
ADJPRI/SELBEST/.greg traces), and the Env layout against sibling Env77D94. Non-blocking
cosmetic note: `return (s16)result;` is a no-op cast (left, to keep the reviewed body).
Rebuild SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa; check_completion_integrity OK.
