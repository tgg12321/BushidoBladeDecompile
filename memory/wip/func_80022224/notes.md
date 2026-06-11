# func_80022224 (code6cac.c) — WIP

## TL;DR

Spawn-point picker: loop 1 computes 4 squared XZ distances (records of six
s16s: fields 0-2 one triple, 3-5 = x,y,z), loops 2-3 mark the two nearest
with -1, loop 4 compacts surviving indices, tail returns the position of a
random survivor. HEAD carries 5 regfix rules (3 LICM-unhoist substs + 2
reorders, regfix.txt:691-698). HEAD floor 7.

**Candidate floor 3** (candidate.c): do/while + three clean levers —
explicit `s32 *d` store walker, loop-3 operand swap
(`dists[i] > dists[best]`, per compare-operand-order-register), loop-4
preheader source-ordering (`i = 0;` then block-scoped `s32 stop = -1;`
sentinel then `w = r;`, per the hirahira_w_frie precedent in
exit-path-return-set-cse-join). These retire the 2 reorder rules' diffs +
1 more. All three levers cheat-reviewer-PASSed (session 2026-06-10).

The remaining 3 diffs are loop 1's giv reduction: cc1's loop.c combines the
two DEST_ADDR givs (p+6, p+10) and reduces them into an IV anchored at
base+10 (`addiu a0,s1,10`, loads at -4/0); target keeps `move a0,s1` +
inline 6/10 offsets. The 3 substs paper over exactly this.

## The mechanism wall (fully characterized, instrumented)

- **Noted loop + single `p += 6`**: combine_givs sums benefits 2+2; decision
  `benefit -= add_cost*biv_count` = 4-2 = 2 > 0 → reduced, anchored +10
  (cc1 -dL dump: "giv at 45 combined with giv at 54"). Wrong bytes. No
  in-loop spelling breaks combine_givs_p for two const-offset loads off one
  biv (express_from only needs g1->add_val CONST_INT — always true here).
- **Goto-form loop** (house style of matched sibling func_80022408): no
  loop notes → no strength-reduce → offsets inline, structure exact
  (121/121), but score 17: flow.c loop-depth ref weighting is also lost, and
  BB2_ALLOC_DEBUG shows the consequence: arg2 (3 refs/livelen 24, pri 1250)
  loses s0 to base (7 refs/livelen 85, pri 1647); the d walker (pri
  3478-3809) allocates before the dx² mflo temp (pri 2500) and takes a2
  (target: dx²→a2, d→a3). Priority arithmetic is closed — no semantic 4th
  arg2 ref exists (arg2[1] never read), base can't lose 2 refs (in-place
  tail `base += ...` byte-required), d can't reach livelen ≥ 33 or refs 3.
  Measured: d-init-first 18, dx/dz self-square merge 19 (merges pseudos the
  target keeps separate: subu in v0, mflo in a2).
- **Noted loop + `p += 3; p += 3;`** (biv_count=2 → combined benefit
  4-4=0 → givs ignored → inline offsets; combine merges the adds back to
  one addiu): **sandbox 0, full-register diff 0/121**. Cheat-reviewer
  FAIL (2026-06-10): split has no semantic purpose, justification is
  loop.c internals. Re-review with the proven-spelling-class framing
  (noted-loop register evidence + single-increment byte evidence ⇒ original
  inhabited the biv_count≥2 class): see meta.json reviewer entry.
  Form preserved in rejected/split_increment_biv_count.c.

## Resume instructions

1. Apply candidate.c to src/code6cac.c (replaces func_80022224 body).
2. `& tools/eng.ps1 sandbox func_80022224 --disable all` → expect 3.
3. The open question is loop 1 only. Either (a) the user sanctions the
   split-increment proven-class reconstruction (then: apply rejected form,
   sandbox 0, retire, done), or (b) derive a new lever: a noted-loop
   construct outside {biv_count≥2} that defeats combine_givs/reduction, or
   a goto-form register lever outside the priority-arithmetic space mapped
   above (e.g. something that changes livelens, not refs).
4. Diff tool: tmp/cmp22224.sh (objdump extract + tmp/norm_diff.py).
   ALLOCDBG: tmp/allocdump.sh (needs tmp/gccdbg/cc1). Loop dump:
   tmp/loopdump.sh (-dL; note cc1 parse errors on GameObj decls are
   pre-existing/recovered, the function compiles fine).

## Ruled out (do not re-derive)

- Conditional `d++` inside the goto-loop's if → beqz+j tail, +1 insn (21).
- p+=3 mid-loop (between loads) → intermediate p has a use → two addius.
- `p = base+3` with loads p[0]/p[2] → offsets 0/4, wrong bytes.
- Two walkers px/pz → two increments emitted.
- base[i*6+5] indexing → mult giv reduced → second walking register.
- Stride-variable increment (`p += stride`) → cse const-props → biv again.
- Caching arg2[0]/arg2[2] in pre-loop locals → target reloads per iteration.
