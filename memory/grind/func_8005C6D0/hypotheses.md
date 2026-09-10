# Hypothesis ledger — func_8005C6D0

## s1 (recon, 2026-09-10)

### H1 — OBJECT MODEL: the flagged split-aggregate symbols are the ORIGINAL spelling
**CONFIRMED (measured).** D_800EFB78 / D_800EFB7C / D_800EFB7D as three independent
scalar symbols addressed by `(u8 *)&SYM + off`, and D_800EFC44 / D_800EFC50 as
independent scalars rather than `D_800EFC38[3]` / `D_800EFC38[6]`, reproduce the target's
per-use `lui %hi / addu / {lw,lbu,sw} %lo` relocations exactly and reach score 5. The
aggregate merge the brief flagged as the sanctioned spelling is not needed here and would
be contradicted by the bytes for D_800EFC44/D_800EFC50 (they are loaded through their own
%hi/%lo pairs, not through the already-live `$s6` = &D_800EFC38 base). Full per-symbol
verdicts in evidence.md under `OBJECT MODEL:`.

### H2 — the LICM hoist of %hi(D_800EFB78) is controlled by how many distinct index
### pseudos reach the two pool-entry accesses
**CONFIRMED (measured, three points).** With both pool accesses reading a single named
`off` local: no hoist. With both written inline as `i * 8` (two distinct index pseudos
after the join recompute): loop.c moves the whole symbol address into a callee-saved
register, costing a preheader `lui/addiu` pair, a save/restore pair and 8 frame bytes.
With the LOAD through its own `o` local and the CLEAR inline: no hoist AND the join
recompute is preserved. Array-indexed spelling (`D_800EFB78[i * 2]`) does not change the
decision, which rules out array-vs-pointer-arithmetic as the mechanism.

### H3 — the target's duplicated voice-loop guard requires a statement inside the guard
### block, otherwise jump.c cross-jumps it into the do-while bottom test
**CONFIRMED (measured).** `voice = next; if ((s16)voice < 0x18) { do { ... } while (...); }`
with an EMPTY guard prologue collapses to `j <bottom test>`. Placing `off = i * 8;` as the
first statement of the guard block keeps the two tests separate and, because CSE folds it
against the outer-loop computation, materialises it as the target's `addu $s2,$v1,$zero`
copy at 0x8005C768. Guarding on `next` instead of on `voice` also keeps the tests separate
but swaps the s3/s4 register roles and loses the copy.

### H4 — the score-5 residual is the `beq $v0,$s5` delay-slot fill (reorg.c thread-fill vs
### sched2 hoisting `li $v0,6` above the branch)
**OPEN — this is the whole remaining frontier.** Everything else (register allocation,
frame, stack-arg slots, relocations, block order) is already identical. The target fills
that delay slot with `addiu $v0,$s0,0x1` pulled out of the .L8005C838 thread and lets
`li 6` fill the following `lhu` load-delay slot; our build schedules `li $v0,6` above the
branch, forcing a `nop` after the `lhu` and leaving the increment split at the loop bottom
(+1 insn). No C-level lever has been tried against this yet.

### Frontier for s2 (in priority order)
1. **H4 via the loop-continue spelling.** The continue path is currently
   `voice = (s16)(voice + 1);` as the last statement of the do-while body, tested by
   `while ((s16)voice < 0x18)`. Try the increment in the loop-control position
   (`do { ... } while ((s16)(voice = (s16)(voice + 1)) < 0x18);`), and try reading
   `vab = *p;` BEFORE the `SpuGetKeyStatus` test so the `== 6` constant is not ready above
   the branch. Probe: `sandbox --disable all`, then
   `pwsh tools/grinder/dump.ps1 func_8005C6D0` and read
   `tmp/grind/func_8005C6D0/dumps/text1b.sched2` and `.dbr` for the actual fill decision
   instead of inferring it.
2. **H4 via sched_solver.** The residual is an emission-order tie, exactly the class
   `tools/sched_solver` + `tools/ra_solver/inverse_compose.py classify` are built for.
   Classify the `li $v0,6` / `addiu $v0,$s0,0x1` ordering before spending more spellings.
3. **The `SpuGetKeyStatus(1 << voice) != 1` test shape.** Target compares against the
   hoisted `$s5` constant (`beq $v0,$s5`), which we already reproduce; the ORDER in which
   `li 6` becomes ready depends on where `vab = *p` sits relative to that test, so
   permuting those two statements is the cheapest byte-neutral C lever available.

## [s1] The brief-flagged split-aggregate symbols (D_800EFB78 / D_800EFB7C / D_800EFB7D, and D_800EFC44 / D_800EFC50 as pieces of D_800EFC38) are reproduced by the SPLIT scalar-symbol spelling, not by an aggregate merge: the target emits a separate lui %hi / addu / {lw,lbu,sw} %lo relocation per symbol per use, and D_800EFC44/D_800EFC50 are loaded through their own %hi/%lo pairs rather than through the already-live $s6 = &D_800EFC38 base.
- mechanism: Address selection: `(u8 *)&SYM + off` and `extern s32 *ARRAY[]` produce different MIPS addressing (symbol+addend lo_sum vs force_reg'd base). The target uses lo_sum for the pool symbols and a hoisted register base only for D_800EFC38, so the merge would have to emit s6-relative loads for D_800EFC44/D_800EFC50.
- probe: Wrote the body with the split spelling (mirroring the already-byte-matching sibling func_8005C650 at src/text1b.c:2686-2689 and func_8005BF78 at src/text1b.c:2599); measured `sandbox func_8005C6D0 --disable all` and disassembled tmp/sandbox/func_8005C6D0/text1b.o against asm/funcs/func_8005C6D0.s.
- result: Score 5 with all six %hi/%lo relocations matching the target one-for-one; no aggregate declaration was needed anywhere. OBJECT MODEL verdicts: D_800EFB78 MATCHES, D_800EFB7C MATCHES, D_800EFB7D MATCHES, D_800EFC38 MATCHES (array declaration), D_800EFC44 MISMATCH-unmeasured for the merge (target's independent %hi/%lo pair contradicts an s6-relative load), D_800EFC50 MISMATCH-unmeasured for the merge (same reason).
- verdict: CONFIRMED

## [s1] Writing both pool-entry accesses (`(u8 *)&D_800EFB78 + i * 8` at the load and at the clear) as inline expressions makes loop.c hoist the whole symbol address into a callee-saved register, adding a preheader lui/addiu pair, a save/restore pair and 8 frame bytes; routing the LOAD through its own named local while leaving the CLEAR inline suppresses the hoist and still preserves the target's recomputed sll/sra at the join.
- mechanism: loop.c LICM (move_movables) treats the SYMBOL_REF as a movable invariant when two distinct index pseudos reach it; with a single named local carrying the offset the address is CSE'd into one lo_sum form and is not moved.
- probe: Three isolated one-edit measurements on the same chassis: off-local-everywhere (score 26 / 114 insns), inline-everywhere (score 20 / 121 insns, 0x60 frame, s7 hoist visible in the disassembly), and load-through-`o` + clear-inline (score 5 / 119 insns, 0x58 frame). Also measured an array-indexed spelling `D_800EFB78[i * 2]` (score 20) to rule out array-vs-pointer-arithmetic as the mechanism.
- result: Hoist reproduced and suppressed on demand; the winning combination is banked as memory/grind/func_8005C6D0/candidate.c and the two losers as rejected/inline-i8-everywhere-licm-hoist.c and rejected/off-local-everywhere-guard-crossjumped.c.
- verdict: CONFIRMED

## [s1] With an empty guard prologue, `voice = next; if ((s16)voice < 0x18) { do { ... } while ((s16)voice < 0x18); }` has its guard test cross-jumped into the do-while bottom test (a plain `j` to the shared test), losing the target's duplicated sll/sra/slti/beqz guard; placing `off = i * 8;` as the first statement of the guard block keeps the two tests separate and materialises the target's `addu $s2,$v1,$zero` copy at 0x8005C768.
- mechanism: jump.c cross-jumping merges the identical guard and loop-latch test tails when nothing separates the guard block from the loop top; a real statement in the guard block blocks the merge, and CSE folds that statement against the outer-loop `i * 8` computation into a register copy.
- probe: Measured the empty-guard form (score 26 / 114 insns, `j` to the shared test in the disassembly), the guard-on-`next` variant (score 20 at exactly 118 insns, duplicated guard but s3/s4 roles swapped and the s2 copy missing — banked as rejected/guard-on-next-no-s2-copy.c), and the `off`-inside-the-guard form (score 5, guard duplicated and `move s2,v1` present at the exact target index).
- result: The target's guard shape and the s2 copy are both reproduced by the `off`-inside-the-guard spelling; the register roles (s4=next, s3=i, s2=off, s1=p, s0=voice, s5=1, s6=&D_800EFC38) then come out identical to the target for every value.
- verdict: CONFIRMED

## [s1] Caching the packed note word in a scalar local (`n = notes[p[1]]`) emits one load where the target emits two, and re-writing the full `notes[p[1]]` expression at all four argument positions additionally reloads the `lhu $v0,0x2($s1)` index; only a pointer local `ev = &((u32 *)D_800EFC38[vab][0])[p[1]]` dereferenced four times reproduces the target's one-address / two-loads shape.
- mechanism: GCC 2.7.2 CSE invalidates cached MEM values at the `sw $v0,0x10($sp)` outgoing-argument store but keeps the register-only address computation live, so the fourth argument re-loads through the surviving address in $v1 while the earlier three keep using the value already in $a2.
- probe: Isolated single edit from the scalar-local form to the pointer-local form on an otherwise identical body; `sandbox func_8005C6D0 --disable all`.
- result: Score moved 57 -> 37 on that one edit; the disassembly then shows `lw $a2,0($v1)` followed after the 0x10($sp) store by `lw $v0,0($v1)` at the same indices as the target (0x8005C7DC / 0x8005C7F0). Also confirmed in the same sweep: `vab` must be u16 (the target's `andi $v1,$a1,0xFFFF` is HImode zero-extend-before-compare; an s16 carrier emits a second `lh`), and the note word must be u32 (target uses `srl`, an s32 carrier emits `sra`).
- verdict: CONFIRMED

## s2 (structural, 2026-09-10) — H4 CLOSED, function MATCHED at distance 0

### H4 — the score-5 residual is the `beq $v0,$s5` delay-slot fill
**CONFIRMED and CLOSED.** Root cause read out of the instrumented reorg.c trace
(BB2_DBR_DEBUG=1), not inferred: `mostly_true_jump` (tools/gcc-2.7.2/reorg.c:1379) returns 0
for this EQ branch because no loop note sits next to its target label, so
`fill_eager_delay_slots` (tools/gcc-2.7.2/reorg.c:3812) tries the FALL-THROUGH thread first,
rejects the `lhu $a1` on `may_trap_p`, and steals the `li $v0,6` compare constant into the
slot. Fixed by spelling the inner voice scan as a top-tested `for` inside the existing
explicit guard, which lets jump.c's `duplicate_loop_exit_test` (tools/gcc-2.7.2/jump.c:2338)
rotate the loop and emit the `NOTE_INSN_LOOP_VTOP` that flips the prediction; reorg then fills
the slot from the loop-continue thread with `addiu $v0,$s0,0x1` as the target does.
Measured 5 -> 2 (119 -> 118 insns).

### H5 — the residual argument-setup order is fixed by the POSITION of `next = voice + 1`
### relative to the SsUtKeyOnV call, not by the expression's shape
**CONFIRMED (measured, five points).** With the statement before the call, the increment is
emitted ahead of the argument setup and sched2 keeps it there (`addiu $s4 / sll $a0 /
sra $a0`). Four rewrites of the expression in that position (before `ev`, first statement of
the if-body, `next = voice + 1` without the inner cast, and split into `next = voice;
next = (s16)(next + 1);`) all reproduce that order. Moving the statement AFTER the call —
semantically identical, since SsUtKeyOnV cannot touch the local — emits it after the call,
where sched2 hoists it back across the call (both `$s4` and `$s0` are callee-saved, so there
is no dependency) into the target's slot between the `$a0` sign-extension and the `$a1`
shift. Measured 2 -> 0.

### H6 — the brief's SPLIT-AGGREGATE / declaration-pun signal is contradicted by the bytes
**CONFIRMED (measured, not argued).** The aggregate/array-declaration spelling of the pool and
the two volume symbols makes loop.c hoist `%hi(D_800EFB78)` into a callee-saved register
(112 insns, frame 0x60) and is structurally different from the target, which emits three
independent `lui %hi / addu $at / {lw,lbu,sw} %lo` triples against three distinct symbols and
never shares a base register. The split `(u8 *)&SYM + off` spelling — identical to the one the
byte-matching sibling func_8005C650 already ships on main at src/text1b.c:2685-2689 against
the same three symbols — is what reaches distance 0. Banked as
rejected/array-decl-licm-hoist-frame-0x60.c.

### H7 — s1's two-offset-locals arrangement survives the chassis change
**CONFIRMED (re-measured on the `for` chassis).** Single shared offset local: 105 insns (guard
cross-jumped). Single shared local also used at the clear site: 99 insns. The s1 arrangement
(pool load through its own local, volumes through a local declared inside the voice guard,
clear site inline `i * 8`) is the only one of the three that reaches 118 insns, so the
conclusion s1 banked on the do-while chassis holds unchanged on the matching chassis.
