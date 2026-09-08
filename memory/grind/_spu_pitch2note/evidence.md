# Evidence bank — _spu_pitch2note

## s1 (2026-09-08, recon) — baseline, target decode, mechanism reads

OBJECT MODEL: the target touches NO globals (asm/funcs/_spu_pitch2note.s has no
%hi/%lo/%gp_rel references; it is pure register arithmetic on three u16 params).
No DATA MODEL signals; no declaration fix applies. MATCHES trivially.

CHASSIS: HEAD src/main.c has `INCLUDE_ASM("asm/funcs", _spu_pitch2note);` at
main.c:3113, directly after the COMPLETED-C `_spu_note2pitch` candidate and
before `SpuGetVoiceVolume`. No prototype anywhere in include/ (grep
"pitch2note" include/ is empty); no caller in asm/funcs (grep "jal.*_spu_pitch2note"
is empty) - the function is an exported library entry with no in-EXE caller, so
the prototype is free (s1 used `s32 _spu_pitch2note(u16 cen_note, u16 cen_fine,
u16 pitch)`; the three `andi $r,0xFFFF` widenings at 0x8008BC74/BCB0/BD5C/BD74
confirm all three params are u16).

canonical: verdict C, hand_coded_tier LOW (no S1/S2/S6), pure-C distance 59 at
the first honest body. sandbox --disable all: v1 = 59 (target 74 insns).

TARGET ALGORITHM (decoded from asm/funcs/_spu_pitch2note.s):
  1. search = ~pitch & 0xFFFF; scan shift 15..0 for the first CLEAR bit of
     search (= highest set bit of pitch); bit = shift; break. (`srav` = signed
     shift -> search is int-promoted u16; `andi 0xFFFF` after the `nor`.)
  2. oct = bit - 12 ($t7); scale = 1 << bit ($t6); curve = 0x1000 ($t0);
     target = pitch ($a2, sltu => unsigned).
  3. outer 0..47 ($t2): lower = scale*curve (mult before loop label + reorg
     peel in the back-edge slot, mflo $t4); curve = curve*0x103B>>12 (shift
     chain, srl => u32); upper = scale*curve (mflo $v1); step = (upper-lower)>>5
     ($t1, srl); base = outer<<5 ($t5, LICM-hoisted out of the inner loop);
     acc = 0 ($t3); next = step ($a3).
  4. inner 0..31 ($a0): v0 = lower+acc; v1 = lower+next (BOTH adds in the
     first BB, before the first branch); v0>>=12; if (target < v0) goto next;
     v1>>=12 (in the bnez delay slot); if (target < v1) -> hit (bnez straight
     to .L8008BD44 with `addu v0,t5,a0` = base+inner in the delay slot -
     reorg fill_eager stole the single-insn hit block and deleted it);
     next: next += step; inner++; slti; bnez top with `acc += step` in the
     back-edge delay slot (fill_simple from before the branch).
  5. exhaust: outer++; bnez outer-loop with `mult t6,t0` peel; li v0,0x600.
  6. found (.L8008BD44): quot = result/128 (signed: bgez/addiu 127/sra 7),
     rem = result - (quot<<7), note = cen_note + quot + oct*12,
     fine = cen_fine + rem, return (note<<8)|fine.

MECHANISM READS (dumps in tmp/grind/_spu_pitch2note/dumps/ for v1 and v11;
loop-pass commentary saved at tmp/grind/_spu_pitch2note/s1/v1.loop.txt):
  - loop.c giv accounting for `lower + step*j` / `lower + step*(j+1)` (v1):
    insn 128 `lower + (step*j)` IS a giv (mult step, add lower) -> reduced to a
    fresh biv INITIALISED TO lower (that is the `move a3,t3` seat in v1; the
    target has no such init). insn 138 `lower + (step*(j+1))` is NOT a giv:
    simplify_giv_expr's PLUS case (loop.c:5096) rejects an add_val that is the
    sum of two invariant REGs (lower + step), so it stays as an add of the
    reduced inner giv. The target keeps `lower` as a separate add on BOTH
    sides with accumulators initialised to 0 and step -> explicit loop-carried
    accumulators (`acc`, `next`): mult-1 givs `lower + acc` are dropped by the
    "not worth while" gate at loop.c:3823 (lifetime*threshold*benefit <
    insn_count) and remain as in-loop adds. Measured: v2 (explicit acc/next)
    reproduced the two zero/step-initialised bivs and the `addu v0,t4,t3` shape.
  - unroll.c:3051 final_biv_value refuses any loop with
    loop_number_exit_count != 0. A `cents++` running counter read after a
    `goto found` exit therefore cannot be eliminated (v9/v10 keep an
    `addiu cents,1` in the inner loop and lose the `sll t5` base); the target's
    `li v0,0x600` + `base + inner` come from an explicit `result = 0x600`
    after the loops and `result = (outer<<5)+inner` in the hit block (v7).
  - reorg fill_simple_delay_slots (reorg.c:2969ff) stops the backward search at
    a CODE_LABEL. With the if/else form (v7/v13) the then-arm {next+=; acc+=}
    ends in `j Lend` (over the else/hit block) and Lend sits between the
    increments and `inner++`, so the back-edge bnez cannot take `acc += step`
    from before and fill_eager peels the loop-top `addu v0,t3,t1` instead
    (the ONLY structural residual left in v13's inner loop). The target has no
    label/jump between the increments and `inner++`: its hit block is
    out-of-line and its increments fall straight into the loop increment.
  - The hit block IS stolen into the `bnez -> found` slot once `result` is
    allocated to $v0 (v13: `addu v0,t5,a0` in the slot, block deleted). In v4
    `result` sat in $a2 (= live `target`) so the steal was refused; the fix was
    the epilogue order (named `quot`/`rem` first, v7), which shortened
    `result`'s live range so global.c gave it $v0.
  - v11 (hit label placed after the loops + `goto found`): a jump.c pass
    between the .jump and .cse2 dumps (toplev.c:2923/2929 jump_optimize calls,
    i.e. after loop_optimize) RELOCATED the scan loop's break block
    {bit = shift; j after_scan} out of line to a fresh label (262) placed after
    the `j found` barrier, immediately before `hit:`, and inverted the scan
    branch (bnez->beqz). v13 (no `hit:` label) keeps the block inline exactly as
    the target does. The mechanism that minted label 262 was NOT identified
    this session (jump.c candidates that call get_label_after: 1531, 1718,
    2324; range-swap "if (foo) bar; else break;" at jump.c:1840ff is disabled
    post-reload but live at the 2923/2929 calls). Whatever relocates a
    `{X; jump L}` block after a barrier is plausibly the same mechanism that
    put the target's hit block out of line - the frontier item.

SCORE LADDER (sandbox --disable all, target 74 insns; forms in
tmp/grind/_spu_pitch2note/s1/vN.c):
  v1 59  natural `lower + step*j` / `step*(j+1)`, `&&`-goto inline hit, packed return
  v2 60  explicit acc/next accumulators, shift at the compare
  v3 55  v2 + `lo = (lower+acc)>>12; hi = (lower+next)>>12;` before the if
  v4 55  v3 as if/else (`if (t<lo || t>=hi) {incs} else {result; goto found}`)
  v5 60  v3 + `goto hit` with `hit: result = (outer<<5)+inner;` after the loops (base not hoisted)
  v6 51  v4 + `fine` computed before `note`
  v7 36  v4 + named `quot = result/128; rem = result%128;` then note/fine  <- epilogue fully matched
  v8 54  v4 + quot/rem named but packed return expression
  v9 45  v7 with `cents++` counter (`&&`-goto)         - counter NOT eliminated
  v10 45 v7 with `cents++` counter (if/else)           - same
  v11 41 v7 + `hit:` after loops with `base = outer<<5` named at inner-loop entry - scan block relocated, `j found` kept
  v12 41 v11 with `base = outer * 32` at outer-loop top
  v13 36 v7 with `next += step; acc += step;` (increment order swapped)  <- BEST, candidate.c
  v14 57 v13 no-else + `result = 0x600` initialised before the loops
  v15 57 v14 as `if {hit} else {incs}`
  v16 58 v13 (if/else) + `result = 0x600` before the loops
  v17 41 v11 with swapped increments

v13 RESIDUAL (36; side-by-side at tmp/grind/_spu_pitch2note/s1/v13_sbs.txt):
  structural - only the inner-loop back-edge delay slot (target: `addu t3,t3,t1`
  = acc+=step from before the branch; ours: loop-top `addu v0,t3,t1` peel) and
  the order `addu a3 / addu t1 / addiu a0` vs target `addu a3 / addiu a0 / [slot acc]`.
  seats - pitch copy $a3 vs $a0; bit $a0 vs $v1; shift $v1 vs $t2; curve $t2 vs
  $t0; lower $t3 vs $t4; outer $t4 vs $t2; acc $t1 vs $t3; step $t0 vs $t1
  (a cascade; likely resolves once the label/jump between the increments and
  `inner++` is gone and the pseudo set matches). Prologue `move t8,a0` (cen_note
  -> $t8) and the whole epilogue already match.

- [s1] OBJECT MODEL: no globals touched by the target (no %hi/%lo/%gp_rel in asm/funcs/_spu_pitch2note.s); no DATA MODEL signals; MATCHES trivially

- [s1] canonical: verdict C, hand_coded_tier LOW; no header prototype and no in-EXE caller (exported LIBSPU/S_N2P entry) so the prototype s32(u16,u16,u16) is free; three andi 0xFFFF widenings confirm u16 params

- [s1] Target algorithm fully decoded (evidence.md): reverse scan of ~pitch&0xFFFF for the highest set bit, 48x32 0x103B curve walk with two loop-carried accumulators, explicit result=0x600 on exhaustion, /128 %128 divmod, (note<<8)|fine pack

- [s1] loop.c:5096 rejects `lower + step*(j+1)` as a giv (add_val = sum of two invariant regs) while `lower + step*j` is reduced whole with lower folded into its init; explicit accumulators give the target's shape

- [s1] reorg.c:2969 backward delay-slot search stops at a CODE_LABEL: the if/else then-arm's `j Lend` puts a label between the increments and inner++, so the back-edge slot gets the loop-top peel instead of `acc += step` - the only structural residual in v13

- [s1] v13 residual seats: pitch copy $a3 vs $a0; bit $a0 vs $v1; shift $v1 vs $t2; curve $t2 vs $t0; lower $t3 vs $t4; outer $t4 vs $t2; acc $t1 vs $t3; step $t0 vs $t1

- [s1] A pre-reload jump_optimize pass (between .jump and .cse2) can relocate a `{X; jump L}` block reached by fallthrough to a fresh label after a barrier (seen on v11's scan block, label 262); mechanism not yet identified - candidates jump.c:1531/1718/2324 get_label_after sites and the range swap at jump.c:1840ff
