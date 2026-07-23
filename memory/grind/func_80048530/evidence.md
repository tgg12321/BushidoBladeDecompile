# Evidence bank — func_80048530

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
