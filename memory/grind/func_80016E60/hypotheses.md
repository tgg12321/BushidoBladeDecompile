# Hypothesis ledger — func_80016E60

## [s1] H1 CONFIRMED — the `1` of `1 << (select-3)` is hoisted only because loop.c combine_movables merges the two arms' constants (threshold 61 vs insn_count 127); a multi-set mask variable (`mask = 1; mask <<= shift;` in both arms, function scope) keeps `li v1,1` inside the arms with the target's `addiu; li` order. Measured 46 -> 34 (v1 -> v3). Evidence E-s1-3. KILLED sub-forms (instance, chassis asm-until-matched, no FAKE present): single-set `mask = 1 << shift` (re-hoists, 21 on carrier); block-scoped mask (order flips to `li; addiu`, 17 on carrier).

## [s1] H2 CONFIRMED — prev/next arms are conditional expressions: `select = (select == 0) ? limit - 1 : select - 1;` / `select = (select == limit - 1) ? 0 : select + 1;` reproduce the a1/v1 temp seats exactly. Measured 34 -> 30 (v3 -> v4). Evidence E-s1-4.

## [s1] H3 KILLED (instance; chassis asm-until-matched; no FAKE) — a `goto loop;` spelling of the main loop. Mechanism guess: no loop notes -> no LICM/loop-depth weighting. Measured 54 (v1 46): the exit-arm layout changes, and the `1` hoist is not what governs the arm placement. rejected/goto-loop-breaks-exit-arm-layout-54.c.

## [s1] H4 CONFIRMED (mechanism) / OPEN (honest spelling) — env/select seat swap is global.c allocno priority: env 4615 < select 6878; env needs >= 8 weighted refs (one more real in-loop use) or livelen <= 17. do-while(0) carrier on {PutDispEnv, PutDrawEnv} measures 11 (E-s1-6). Honest levers NOT yet measured: (i) a genuine 4th in-loop use of env — none identified from the bytes (env feeds only +0x5C and PutDrawEnv; DrawOTag uses arg0+0x408C via $s5); (ii) lowering select's refs below 18 weighted (needs ~5 fewer real in-loop uses — every use is byte-visible, judged implausible but unmeasured); (iii) shortening env's livelen via fewer RTL insns between its birth and PutDrawEnv on BOTH if/else paths (e.g. a different spelling of the special-arm argument `select | (D_800A3788 << 16)` or of the two func_8005C8A8 calls; needs 9 fewer counted insns — unmeasured). KILLED (instance, no FAKE): copy alias `disp = env` (cse deletes it before flow; 30 unchanged).

## [s1] H5 OPEN — bit-arm seats (chain $a0 / shift $v0 / mask $v1). Mechanism E-s1-8: local-alloc seats the chain in $v0 before global.c seats shift/mask. Six spellings measured on the carrier (14-17, none < 11). Untried: (a) giving the chain a hard-reg suggestion for $a0 (qty_phys_copy_sugg) — would need the or-result to be an argument copy, no such call exists in the bytes; (b) making shift+mask local (single block) AND higher priority than the chain — s4 (all block-scoped) = 15, but the order flip (`li` first) suggests block-scoped temps change expand order; try block-scoped with `mask` declared BEFORE `shift`, and the `mask <<= select - 3` no-shift form inside a block; (c) check whether the v5 prologue pair reorder (E-s1-5c) is coupled to these seats via .sched2.

## [s1] The `li s4,1` hoist comes from loop.c combine_movables merging the two arms' single-use `1` constants (threshold 61 vs insn_count 127); a function-scope multi-set `mask = 1; mask <<= shift;` in both arms keeps the constant inside the arms with the target's addiu-then-li order.
- mechanism: loop.c:532 threshold=(1)*(1+60)=61 (hard-float build: 32 FP regs non-fixed); loop.c:1631 threshold*savings*lifetime >= insn_count; combine_movables loop.c:1244-1284 merges equal CONST_INT movables (life 1+1, savings 1+1 -> 244 >= 127); n_times_set != 1 makes mask non-movable
- probe: v1 (plain 1 << (select-3)) vs v3 (mask=1; mask<<=...) sandbox --disable all; .loop dump lines 'Insn 385 ... moved to 516' / 'Insn 419 ... matches 385'
- result: 46 -> 34; single-set `mask = 1 << shift` re-hoists (21 on the carrier vs 11)
- verdict: CONFIRMED

## [s1] prev/next arms are `select = (select == 0) ? limit - 1 : select - 1;` and `select = (select == limit - 1) ? 0 : select + 1;` - reproduces the target insns and the a1/v1 temp seats exactly.
- mechanism: COND_EXPR expands into a fresh target pseudo per arm; jump.c 'if (...) x = a; else x = b' -> 'x = b; if (...) x = a' produces the delay-slot unconditional form
- probe: v3 (explicit next temp, v1 seat in arm 1) vs v4 (?:) objdiff
- result: 34 -> 30, arm insns byte-identical
- verdict: CONFIRMED

## [s1] A `goto loop;` spelling of the main loop, measured on the asm-until-matched chassis with no FAKE constructs present.
- mechanism: no NOTE_INSN_LOOP_BEG -> no LICM, loop_depth weight 1
- probe: v2 sandbox
- result: 54 (worse than v1's 46): exit-arm layout changes, hoist not the governing factor
- verdict: KILLED
- kill_scope: instance
- measured_on: asm-until-matched chassis (INCLUDE_ASM main, 0 rules), no FAKE constructs, body_v2_goto.c

## [s1] A pure copy alias `disp = env; PutDispEnv(disp + 0x5C);` measured as a ref lift for env's global.c priority, on the asm-until-matched chassis with no FAKE present.
- mechanism: cse.c replaces the copy's uses and flow deletes the dead copy before reg_n_refs is counted
- probe: v8 sandbox + objdiff
- result: 30 unchanged; inert
- verdict: KILLED
- kill_scope: instance
- measured_on: asm-until-matched chassis, no FAKE constructs, body_v8_copyalias.c

## [s1] env/select seat swap is global.c allocno priority (env 4615 < select 6878; env needs >= 8 weighted refs or livelen <= 17); a do-while(0) wrap of {PutDispEnv, PutDrawEnv} raises env to 8 refs / 9230 and seats env=$s0, select=$s1.
- mechanism: flow.c reg_n_refs += loop_depth; global.c allocno_compare priority floor_log2(refs)*refs/livelen; ra_solver inverse ranks refs_up env 6->8 first
- probe: tools/ra_solver extract + inverse --swap 74,75; v5 carrier sandbox
- result: v5 = 11 (only bit-arm seats + a prologue pair reorder remain). Mechanism measurement only - NOT a submission: lever exhaustion incomplete (H4 (ii)/(iii) unmeasured)
- verdict: CONFIRMED
