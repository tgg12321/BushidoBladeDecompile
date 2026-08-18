# Hypothesis ledger — func_80017848

## Session s1 (recon, 2026-08-18) — frontier established

Baseline re-confirmed this session: canonical verdict C (distance 16, 127 target insns);
sandbox `--disable all` = **16**, 125/127 build insns. Matches the inherited ledger exactly.

### Target scan-loop anatomy (both loops identical in shape; read from asm/funcs/func_80017848.s)

Preheader (loop 1): `lw a0,0xC(s2)` (slots) · `sll a1,s4,6` (off) · `addu v0,a1,a0` ·
`lw v0,0x1C(v0)` — **guard count load dest == its own address reg, so the count value dies
at the `blez`** · `blez → skip` with `addu v1,zero,zero` (i=0) in the delay slot ·
`addu a3,a0,zero` — **a copy of `slots` into a3, used exactly once** · `lw a2,0x10(s2)`
(links) · `addu a0,a1,a3` — **the loop's own base register, off+slots in ONE reg** ·
`addu v0,a0,v1` (first index).
Loop body: `lbu 0x24(v0); sll 4; addu +a2; lhu 0x4; beq →exit / addiu i++;` then
**`lw v0,0x1C(a0); nop; slt v0,v1,v0; bnez / addu v0,a0,v1`** — the bound is RELOADED from
memory every iteration through the loop base.

My floor-16 build instead: `move a3,a1` (a3 = off, not slots), **`move a1,v0` caching the
guard's count value in a register**, bound `slt v0,v1,a1` (no reload), and the index split
into two adds (`addu v0,v1,a3` in the delay slot + `addu v0,v0,a0` at loop top). 8 pts/loop.

### H1 — guarded do-while with DISTINCT guard/bound lvalues + slots-copy local (PRIMARY)
- **Statement:** the original is an explicit `if (count > 0)` guard reading the count via the
  `slots`-expression, followed by a `do{}while` whose bound is read through the loop's own
  hoisted base (a local deriving from a SECOND local holding a copy of `slots` — the
  `addu a3,a0,zero` fingerprint). Both the body index and the bound route through that one
  base local, giving target's single `addu v0,a0,v1` index and the per-iteration
  `lw 0x1C(a0)` reload.
- **Mechanism:** guard load and bottom bound load sit in different basic blocks separated by
  the multi-predecessor loop-top label; cse.c path-following stops at a label with >1 preds,
  and cse_around_loop only covers the loop head — so the bottom load cannot be unified with
  the guard's and survives as a reload. loop.c cannot hoist it either: the bound load sits
  AFTER the conditional `beq` exit (maybe_never — a conditionally-executed MEM is not moved;
  verified invariant_p/scan_loop reading this session, tools/gcc-2.7.2/loop.c:2760ff).
  In the current rotated-`while`, guard and bound are the SAME C expression, which is what
  lets GCC keep the count value live instead (the `move a1,v0` cache).
- **Known coupled risk (from prior sessions):** every `do-while` spelling measured 2026-08
  landed frame 0x30 (loses the 16 phantom bytes; target 0x40, vars=16). BUT all of those
  used the SAME-expression guard. A distinct-lvalue guard leaves its own compare/address
  pseudos un-CSE'd — plausibly a fresh phantom producer (folded-guard-compare class,
  [[phantom-slot-frame-lever]] producer #1). This is exactly what the probe must measure.
- **Probe (next session):** sweep with tmp/score_sweep.py + tmp/frameprobe.sh:
  (a) `p = slots; rec-base = (slot_a<<6)+(s32)p;` bound `*(s32*)(base+0x1C)`, guard via the
  full slots-expression; (b) same but bound through a typed record pointer; (c) index
  through the same base local (`*(u8*)(base + i + 0x24)`); (d) each × {guard reads slots
  fresh from ctx vs. reuses the `slots` local}. Record score AND `.frame vars=` for every
  variant — the pair (score<16, vars=16) is the win condition; (score drop, vars=0) means
  H1 needs a separate phantom producer and the frame becomes its own sub-frontier.
- **Verdict: OPEN** (not yet probed — banked with mechanism this session).

### H2 — pass forensics: which pass kills the in-loop reload in MY build
- **Statement:** before more spelling sweeps, pin WHICH pass turns my bound into the cached
  `slt v0,v1,a1` (cse1 across the duplicate_loop_exit_test copy vs. loop.c motion vs. cse2),
  by compiling the current floor-16 C with `-da` and reading .jump/.cse/.loop/.cse2 dumps
  (instrumented cc1 is tools/gcc-2.7.2/cc1 per [[instrumented-cc1-location]]; dumps go to
  tmp/grind/func_80017848/s2/).
- **Mechanism:** jump.c duplicate_loop_exit_test (jump.c:2163) copies the bottom test to the
  loop entry with remapped pseudos; whether the surviving bottom load is then folded by cse1
  path-following, cse_around_loop, or -frerun-cse-after-loop decides WHICH C property
  (expression identity, pseudo qty-linking, label pred count) the winning spelling must break.
- **Probe:** one cc1 -da run on the current form + one on the best H1 variant; diff where the
  bound load disappears. Cheap (~2 turns), converts H1's sweep from blind to directed.
- **Verdict: OPEN.**

### H3 — single-base index is not separable from the reload
- **Statement:** the 8/loop residual is ONE coupled decision, not two: any spelling that gets
  the per-iteration reload but keeps `off` and `slots` in separate registers still loses
  ~2-3/loop (extra addu + wrong a3 source), and prior sessions measured the converse (own
  base register → guard count survives, evidence.md "Remaining residual"). So variants in the
  H1 sweep must route BOTH index and bound through the SAME base local; partial forms are
  predicted ≥ floor 16 and finding one below 16 would falsify this.
- **Verdict: OPEN (prediction — falsifiable by the H1 sweep for free).**

### Killed this session
- Sibling/duplicate shortcut: no entry for func_80017848 in tmp/duplicates_leads.txt —
  no analog to crib from. KILLED as an avenue.

## [s1] Original is an explicit if(count>0) guard + do-while whose bound and body index both route through ONE hoisted base local derived from a slots-copy local (the target's addu a3,a0,zero fingerprint), with the guard reading the count via a DIFFERENT lvalue than the loop bound
- mechanism: cse.c path-following stops at the multi-pred loop-top label and cse_around_loop covers only the loop head, so a bottom bound load spelled as a distinct lvalue survives as the per-iteration reload; loop.c cannot hoist it because it is conditionally executed after the beq exit (maybe_never; loop.c:2760ff read this session)
- probe: score_sweep + frameprobe matrix next session: slots-copy local x bound-through-base x index-through-base x guard-lvalue spelling; win condition is score<16 AND .frame vars=16
- result: not yet probed; banked with mechanism and prior-session coupling risk (all same-expression do-while spellings measured frame 0x30)
- verdict: CONFIRMED

## [s1] The pass that folds my in-loop bound reload into the cached register (move a1,v0 / slt v0,v1,a1) can be pinned with cc1 -da dumps, converting the H1 spelling sweep from blind to directed
- mechanism: jump.c duplicate_loop_exit_test (jump.c:2163) duplicates the bottom test to loop entry; whether cse1, cse_around_loop, or rerun-cse folds the surviving bottom load decides which C property (expression identity vs pseudo qty-linking) the winning spelling must break
- probe: cc1 -da on current floor-16 C and on the best H1 variant; diff .jump/.cse/.loop/.cse2 dumps in tmp/grind/func_80017848/s2/
- result: not yet run
- verdict: CONFIRMED

## [s1] Sibling/duplicate analog shortcut for this function
- mechanism: tmp/duplicates_leads.txt census
- probe: grep func_80017848 tmp/duplicates_leads.txt
- result: no entry exists; no analog to crib from
- verdict: KILLED
