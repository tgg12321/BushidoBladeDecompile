# Evidence bank — func_80048530

## s3 (structural, 2026-07-23) — NEW FLOOR 10 -> 1; the "combine lh-fold wall" was NOT a wall

- **NEW CLEAN FLOOR = 1** (sandbox --disable all, confirmed on the canonical
  wteng path, deterministic, build 47 == target 47). candidate.c updated.
  Two coupled structural changes vs the s1/s2 floor-10 candidate:
    1. Walker as a FRESH assignment `arg0 = base + off` (NOT `arg0 += off`).
    2. c,d read with the SAME walking-pointer idiom as a,b — `c=*p; p+=2; d=*p;`
       (advance the walker, never a fixed `*(arg0+2)`), (s16) at the call.
- **The s2 "combine lh-fold wall" is DISPROVEN.** With the fresh-assign walker +
  walking-pointer c,d, c/d emit lhu+sll+sra (NOT lh) and REUSE the dead walker:
  `lhu v0,0(v1); lhu v1,2(v1)` — exactly target. The old frontier F1 ("make the
  walker dead-reusable at the d-load") is genuinely satisfied. The lh-fold was an
  artifact of the accumulate form + fixed-offset d read, not a hard combine wall.
- **THE SOLE RESIDUAL (score 1) is one instruction — the walker-relocation add:**
    build : addu $v1,$v1,$v0   (base-first)
    target: addu $v1,$v0,$v1   (off-first)
  Everything else is byte-identical (move t0,v1 early; move a1,a3; entry+=base
  as addu a0,a0,t0; all four halfwords lhu+sll+sra).
- **The off-first order is reachable in C ONLY via `arg0 = off + base`** — a
  commutative operand swap (or-tree-shape-shift, FORBIDDEN; byte-0, banked
  rejected/offbase-operand-shuffle-cheat.c). Every NON-swap off-first spelling
  MISROUTES the walker and regresses (measured, rejected/offfirst-structural-
  misroutes.c): off+=base ->22 ($a1 walker), mem-inline ->20/22 ($t0 walker),
  fresh-walker base+off ->12/22. MIPS addu is 3-operand so GCC orders commutative
  operands by canonicalization; for two plain pseudos source order controls, and
  raising off's precedence (MEM/accumulator) drags the walker OFF $v1. off-first
  and v1-walker are mutually exclusive except via the swap.
- **cc1psx CONFIRMS (difficult-is-not-impossible obligation discharged).** The
  ORIGINAL compiler cc1psx emits base-first (`addu $8,$8,$2`) from the natural
  `base + off` too (tmp/grind/func_80048530/s3/n1.psx.s) — it does NOT reproduce
  target's off-first from the natural order either. So the original SOURCE used
  the off-first order; no non-swap C reaches it on either compiler. This is NOT
  a fork divergence (our fork reaches byte-0 from the swap, sandbox=0).
- **scan_hand_coded = LOW 1/8** (only S4 front-loads; no S1/S2/S6 STRONG signals)
  -> canonical-asm refused per endgame-lock-disposition.
- **Endgame-lock species (.claude/rules/endgame-lock-disposition.md):** byte-
  matches on main only via 5 regfix rules; 1 insn short in honest pure C;
  residual = commutative-operand-order RA tie; only closer forbidden; no SOTN
  precedent for ADD operand-order-for-codegen; no hand-coded evidence. Both owner
  AND-gates fail => textbook INCOMPLETE-owner-accepted candidate. NOT escalated
  this session (structural modality; floor just dropped): NEXT is a permuter pass
  from the floor-1 base, then owner-escalation if that is also dead.
- CAUTION for the next session: measuring this function is SENSITIVE to the exact
  c/d spelling. `c=*p; p+=2; d=*p;` (walking) = score 1; `c=*p; d=*(p+2);`
  (fixed offset) = score 20 (walker misroutes to $t0, c folds to lh). Always use
  the walking-pointer read. Artifacts: tmp/grind/func_80048530/s3/ (sweep*.py,
  n1.psx.s, n1.fork.s, cc1psx_cmp.sh).

## s2 (structural, 2026-07-23) — FLOOR held at 10; combine-fold wall CONFIRMED via RTL

- **ROOT-CAUSE NAILED (RTL greg dump).** The ONLY residual at floor 10 is c,d
  emitting `lh` (1 insn) vs target's `lhu`+`sll`+`sra` (3 insns) — a 4-insn gap.
  The floor-10 (lh) build ALREADY has PERFECT register allocation: walker in
  v1, c->v0, d->v1 reusing the dead walker (`lh v0,0(v1); lh v1,2(v1)`), exactly
  mirroring target's `lhu v0,0(v1); lhu v1,2(v1)`. So frontier F1 ("make walker
  dead-reusable") is already satisfied — the walker reuse is NOT the problem.
- **The wall is GCC's combine pass folding `sign_extend(mem:HI)` -> lh.** For any
  SINGLE-USE c/d, combine collapses every zero-extend+shift spelling back into a
  single sign-extending load. Measured dead this session:
    * in-place reassignment `c=*(u16*)p; c=(c<<16)>>16;` -> refolds to lh, score 10
      (rejected/inplace-shift-refolds-lh.c).
    * symmetric `(s16)` at CALL (both spellings) -> score 22 (already banked s1).
    * d-before-c source order, symmetric -> score 22.
- **Why symmetric cascades (RTL, tmp/grind/func_80048530/s2/text1b.i.greg).** With
  `(s16)` at the call, combine folds the offset-0 c into `lh` (insn 91:
  `sign_extend(mem(t0))`) but leaves the offset-2 d as `zero_extend(mem(t0+2))`
  + sll/sra (asymmetric: offset-0 folds, offset-2 doesn't). Both c(v1) and d(v0)
  then occupy the temps, so the walker is EVICTED v1->t0 and base t0->t1 (the
  full register-rename cascade). Target instead keeps c,d BOTH as lhu+sll+sra
  with walker in v1 reused for d.
- **a,b escape the fold; c,d cannot.** a,b compile to lhu+sll+sra (matching
  target) because their load is far from their `(s16)`-cast use (register args,
  ABI arg-setup separates them). c,d are stack args; their load->sext folds.
  No pure-C spelling found that gives c,d the a/b treatment without the cascade.
- **KILLED: permuter campaign from floor-10 (F3).** 10,593 iterations, `-j6`,
  `--stop-on-zero`, fresh-seed windows. NO score-0 find. Best candidates were
  either byte-identical to the floor-10 form or SEMANTIC-BREAKERS (dropping the
  `(s16)` cast on c). The permuter's diff metric is mis-aligned/noisy for this
  function (identical source scored 1020 at launch vs 750 as an output), so it
  gives no reliable gradient. Random C mutation does not crack the combine-fold
  RA tie. Artifacts: tmp/grind/func_80048530/s2/ (base.c, compile.sh, target.o,
  settings.toml, greg dumps, campaign.log, output-* dirs).

## s1 (recon, 2026-07-23) — FLOOR 12/11 -> 10

- **NEW FLOOR 10** (clean pure C, no cheats). Achieved by adopting the
  COMPLETED-C sibling func_800483DC's base-routing idiom: mutate the `arg0`
  parameter as the walking pointer + hold `base = arg0` in a SEPARATE local for
  the final `entry += base`. Produces target's `move t0,v1` 2nd-copy + v1-walker
  routing. Sibling proves this exact idiom is pure-C-reachable. candidate.c saved.

- Sibling func_800483DC (src/text1b.c:299, similarity 0.694, COMPLETED-C) is the
  Rosetta stone: `arg1 = idx_expr + arg0; base = arg0; off = *(s32*)arg1;
  arg0 += off; ...; entry = base + *(s32*)arg0;`. Same base/walker split.

- Residual at floor 10 (43 build vs 47 target insns):
  (a) 4-insn count gap = c,d compile to `lh` (the `(s32)(s16)*(u16*)` fold);
      target uses `lhu`+`sll`+`sra` (6 insns) for c,d like a,b.
  (b) scheduling: `move t0,v1` (base copy) placed AFTER `lw v0` vs BEFORE the
      `sw ra/sw s0` prologue stores in target.
  (c) `move a1,a3` (arg3 passthrough) now lands naturally in the beqz delay slot
      (was diff #2; effectively resolved by base-routing).

- **KILLED: c/d sign-extend fix in isolation -> score 22** (both spellings:
  symmetric-with-a/b `(s16)c` at call, AND `((s32)(c<<16))>>16`). Regresses
  because c/d sign-extend needs v0+v1 scratch, colliding with v1 as the walker;
  GCC shifts walker v1->t0, base t0->t1 (full register-rename cascade). Target
  instead REUSES the dead walker v1 as the d-scratch (`lhu v0,0(v1);
  lhu v1,2(v1)`) — v1 dies after its own load. Our build won't reuse it.
  Artifacts: tmp/grind/func_80048530/s1/{floor10_baseroute,cd_signext_cascade22}.txt
  rejected/cd-signext-cascade22.c

## Prior (WIP import — superseded by s1 base-routing)
- WIP rejected: explicit u8* base var reused (score 12); index-load-into-temp
  (12); arg3-early-local (12); c/d-u16-hold (22, "coupled to base routing").
  NB: the c/d-u16-hold cascade was hypothesized to vanish "once base registers
  are right" — s1 DISPROVED that: base routing IS right (floor 10) and c/d fix
  STILL cascades to 22. The coupling is register-scratch collision, not base
  routing.

- [s1] New clean pure-C floor 10 (sandbox --disable all), improved from prior clean 12 / HEAD honest 11. candidate.c saved.

- [s1] COMPLETED-C sibling func_800483DC (text1b.c:299, sim 0.694) proves the base/walker split idiom (base=arg0; arg0+=off; entry=base+*arg0) is pure-C-reachable and produces the move-t0,v1 2nd-copy routing.

- [s1] Residual at floor 10: (a) 4-insn gap = c,d fold to lh vs target lhu+sll+sra; (b) move t0,v1 scheduled after lw v0 vs before the sw ra/sw s0 prologue stores.

- [s1] move a1,a3 (arg3 passthrough) now lands in the beqz delay slot naturally under base routing (former WIP diff #2 effectively resolved).

- [s1] DISPROVED the WIP conjecture that the c/d-u16-hold cascade would vanish once base registers were correct: base routing is correct at floor 10 and the c/d fix STILL cascades to 22. The coupling is a v0/v1 sign-extend-scratch collision with the v1-walker, not base routing.

- [s1] The target's c/d load reuses the dead walking pointer v1 as the d sign-extend scratch (lhu v0,0(v1); lhu v1,2(v1)) — needs no extra register; that is the reuse our build fails to make.

- [s2] Floor held at 10 (43 vs 47 insns). candidate.c (floor-10 base-routing form) is unchanged and remains the best form; src/text1b.c restored to HEAD.

- [s2] RTL-confirmed: the sole residual is c,d emitting `lh` (1 insn each) vs target's `lhu`+`sll`+`sra` (3 insns each) = the 4-insn gap. Register allocation at floor 10 is already PERFECT: walker in v1, c->v0, d->v1 reusing the dead walker, exactly mirroring target.

- [s2] The wall is GCC's combine folding `sign_extend(mem:HI)` -> lh for any single-use, offset-0 c/d. a,b escape the fold (compile to lhu+sll+sra like target) only because they are register args and ABI arg-setup separates their load from their (s16) use; c,d are stack args and always fold.

- [s2] The symmetric ((s16)-at-call) form is the only spelling that prevents the fold, but combine's fold is asymmetric (offset-0 c folds to lh, offset-2 d does not), so c,d occupy both temps and the walker is evicted v1->t0 / base t0->t1 (cascade, score 22).

- [s2] Permuter: 10,593 iters, no zero; metric unreliable (identical source 1020 vs 750). Campaign stopped in-turn (0 live campaigns).

- [s3] NEW clean floor = 1 (sandbox --disable all, canonical wteng path, deterministic, build 47 == target 47). Down from floor 10 (s1/s2). candidate.c = fresh-assign walker + walking-pointer c/d.

- [s3] The s2 'combine lh-fold wall' is DISPROVEN: with the fresh-assign walker + walking-pointer c,d, c/d emit lhu+sll+sra and REUSE the dead walker (lhu v0,0(v1); lhu v1,2(v1)) exactly as target; the old frontier F1 is genuinely satisfied.

- [s3] The sole residual (score 1) is ONE instruction: the walker-relocation add, build addu $v1,$v1,$v0 (base-first) vs target addu $v1,$v0,$v1 (off-first). Everything else is byte-identical.

- [s3] off-first with v1-walker routing is reachable only via arg0 = off + base (or-tree-shape-shift, FORBIDDEN; byte-0, banked rejected/offbase-operand-shuffle-cheat.c). Every non-swap off-first spelling misroutes the walker (22/20/12) — rejected/offfirst-structural-misroutes.c.

- [s3] cc1psx emits base-first from the natural base+off too (n1.psx.s); the original source used the off-first order. Not a fork divergence.

- [s3] scan_hand_coded = LOW 1/8 (only S4 front-loads) -> canonical-asm refused per endgame-lock-disposition.

- [s3] MEASUREMENT CAUTION for the next session: score is sensitive to the c/d spelling. Walking-pointer `c=*p; p+=2; d=*p;` = score 1; fixed-offset `c=*p; d=*(p+2);` = score 20 (walker misroutes to $t0, c folds to lh). Always use the walking-pointer read.

- [s3] Endgame-lock species (.claude/rules/endgame-lock-disposition.md): byte-matches on main only via 5 regfix rules; 1 insn short in honest pure C; residual is a commutative-operand-order RA tie; only closer forbidden; no SOTN precedent for ADD operand-order-for-codegen; no hand-coded evidence. Both owner AND-gates fail -> textbook INCOMPLETE-owner-accepted candidate.
