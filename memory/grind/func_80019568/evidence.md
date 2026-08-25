# Evidence bank — func_80019568

## [s1] 2026-08-25 — recon (fresh derivation; rule-era chassis discarded)

Function map (from asm/funcs/func_80019568.s, 141 insns, frame 0x40, one callee-save s0=arg0):
- Frame struct at sp+0x10..0x37: `s16 output[4]` @0x10, `s32 voice_mask` @0x18, 3 unknown
  words @0x1C-0x27, `s32 packets[4]` @0x28 (loaded from D_800FF580/584/5A4/5A8).
- Loop 1 (2 iters): per 8-byte packet record, byte tests + a switch through
  **jtbl_80010068** (8 entries); computes `bits`; `voice_mask = ((u32)voice_mask>>16)|(bits<<16)`.
  Register file: a2=output walker (offsets 0/4), a3=packet walker (offsets 0..3),
  t0=i (slti t0,2), t1=voice_mask, t2=const 4, t3=jtbl base.
- `func_8001B138(&sp.voice_mask)` with the mask store in the jal delay slot.
- Conditional block (D_800A3834==1 && arg0==0): switch on lh D_800A38DC through
  **jtbl_80010088** (7 entries); two `|= 0x08000800` sites gated on D_8010278E / D_8010278C.
- `func_8003A728(&sp.output)`; copy loop (2 iters) sp.output[0..1]->D_80102788/8A,
  sp.output[2..3]->D_8010278C/8E via two dest walkers (a0,a1) + one src walker + i in t0.
- Tail: old=D_80102790; D_80102790=mask; D_80102794=mask&~old; D_8010279C=~mask;
  D_80102798=~mask&old. Target materializes &D_80102790 fully (lui+addiu, then lw 0(v0)/
  sw 0(v0)) and stores D_80102790 EARLY (before the nor/and chain); store order 9C, 94, 98.

Measurements (sandbox func_80019568 --disable all; target_insns 141):
- Committed rule-era body (pins + hardcoded asm + 5 regfix): honest floor **42**
  (47 cheat insns stripped; stripped object is structurally broken — extra callee-save s1,
  split output[2] pointer, packet lbu through sp+43 base). Rule-era chassis carries no
  evidence about the original structure; discarded per the 2026-08-19 chassis warning.
- v1 clean C (explicit walking pointers out++/pk+=8 + counter i, no cheats): **49**.
  loop.c strength-reduced every (biv+const) deref into separate walking givs
  (out[2]->own walker sp+20; pk[1..3]->combined walker anchored sp+43 offsets -2..0),
  hoisted the `li 1`, and cse FOLDED the output[0] store->reload (no lhu).
- v2 clean C (indexed accesses `sp.output[i]`, `sp.output[i+2]`, `packets[i*8+k]`,
  do-while i<2): **34**, build 141 == target 141. Combined givs now EXACTLY match target
  addressing (a2 offsets 0/4; a3 offsets 0..3) AND the lhu store->reload survives
  (indexed cse lookup does not fold it — v1's pointer form did fold it). Residual: loop.c
  ELIMINATED biv i ("Final biv value for 75, calculated. biv 75 can be eliminated",
  dumps/code6cac.loop:885-886) — exit test rewritten to `slt a2,limit` with hoisted
  limit sp+20 (an out-class giv, mult 2 add reg191); knock-on renames everywhere
  (mask t0 vs t1, loop-2 counter a2 vs t0).
- s16 i probe: **74** (build 159) — KILLED; sign-extensions everywhere. MIPS does not
  neutralize this via PROMOTE_MODE in a helpful way.
- v2 + DIAGNOSTIC exit test `while (i + 1 < 3)`: **28**, build 143. maybe_eliminate_biv_1's
  REG case returns 0 on the biv inside the PLUS -> elimination blocked -> i survives in t0
  and the ENTIRE register file snaps to target (t0/t1/t2/t3, loop-2 counter t0). combine
  does NOT fold (lt (plus i 1) 3) -> (lt i 2) (simplify_comparison only shifts constants
  across PLUS for equality compares, not signed <), so the diagnostic leaves
  `addiu v0,t0,1; slti v0,v0,3` (+2 insns). THIS SPELLING IS A PROBE, NOT A CANDIDATE:
  it exists only to defeat the eliminator and does not byte-match anyway.

Residual inventory at 28 (from tmp/grind/func_80019568/s1 normalized diff; LO-vs-0 and
split-immediate lines are normalizer artifacts, not real):
1. Exit-test spelling (+2 insns, diagnostic construct) — need an honest form that leaves
   `slti v0,t0,2` while blocking biv elimination (frontier F1).
2. `li 1` for `sp.output[i+2] = 1` hoisted out of the loop (loop.c movables:
   "Insn 87: regno 109 (life 1), move-insn savings 1 moved" — dumps/code6cac.loop:861)
   vs target keeps `addiu v0,zero,1` inside, filling the lhu load-delay slot; the store
   `sh ,4(a2)` sits early (between lhu and addiu -1) in target, late in build (F2).
3. bits/scratch v0-v1 rename cluster in loop 1 (lhu into v1 vs v0; nor v1 vs v0;
   srl/sll operand roles swapped). Likely coupled to F2's slot pressure.
4. Tail block: build accesses D_80102790 via %lo forms and stores it late; target
   materializes the full address (lui+addiu; lw/sw through 0(v0)) and stores early.
   Rule-era body used a pointer local (pointer-alias FAKE family) for this; an honest
   producer is unproven (F3). Note D_80102788..D_8010279C is a plausible single aggregate
   (cf. .claude/rules/split-scalars-hide-aggregate.md) but no independent evidence yet;
   func_800194F4/func_80019534 write the same cluster as scalars.
5. jtbl_80010088 addressing: build emits `lw v0,32(at)` off the FIRST jtbl's %hi/%lo
   (tables adjacent in TU rodata) where target references %lo(jtbl_80010088) directly.
   May be masked by the scorer or may be a real point — not yet isolated.

GCC-internals facts established (read from tools/gcc-2.7.2/loop.c):
- Biv elimination gate: strength_reduce:4035 `all_reduced == 1 && bl->eliminable &&
  maybe_eliminate_biv(...)`. A biv with NO derived givs survives ("Cannot eliminate biv
  75: biv used in insn 400" — this is exactly why the copy-loop counter survives, its
  arrays walk as independent bivs).
- Compare rewrite eligibility (loop.c:6127+): giv needs CONSTANT positive mult_val,
  `!ignore && !maybe_dead && always_computable && mode == biv mode`.
- Worthwhile test loop.c:3824: `v->lifetime * threshold * benefit < insn_count` -> giv
  not reduced -> all_reduced=0 -> biv survives. (Alternate honest blocking axis: make a
  giv fail this; untested.)
- final_biv_value (unroll.c:3020) needs loop_n_iterations != 0, no early exits,
  invariant init; falls back to reg_dead_after_loop.

Artifacts: tmp/grind/func_80019568/s1/{objdump.txt,norm_diff.py,dump.sh},
tmp/grind/func_80019568/dumps/code6cac.loop (v2-state dump; NB dump.ps1 reports
pre-existing parse errors at src/code6cac.c:661/977 for later functions but our function's
dumps are complete).

- [s1] Honest floors this session: committed chassis 42; v1 clean pointers 49; v2 clean indexed 34 (build 141==target); v2 + diagnostic elimination blocker 28 (build 143)

- [s1] Full residual inventory at 28 and GCC loop.c internals (elimination gate, compare-rewrite eligibility, worthwhile test, final_biv_value conditions) banked in memory/grind/func_80019568/evidence.md

- [s1] s16-counter form banked to memory/grind/func_80019568/rejected/s16-loop-counter.c

- [s1] src/code6cac.c reverted to HEAD (5 regfix rules are index-calibrated to the committed body; probe bodies live in candidate.c)

- [s1] Owner directive (RULES-TO-ZERO, 2026-08-24) acknowledged and executed: this session began the pure-C route; jtbl coupling confirmed (jtbl_80010068 8 entries, jtbl_80010088 7 entries, both emitted by the C switches)
