# Hypothesis ledger — func_800481E8

## s1 (recon, 2026-07-28)

- H-s1-1 "inline-move cheat is dissolvable pure-C via the InitHiraRmd_80047FBC
  prologue technique (base-copy staging + function-scope precompute + FAKE
  `arg0 = 0;`)" — CONFIRMED. cc1 emits target's exact `move $16,$4; move $18,$16`
  staging; sandbox 14 → 10; build 56/56 insns; loop + prologue + epilogue all
  byte-match except frame offsets. Applied to src/text1b.c.
- H-s1-2 "the FAKE dead store is removable" — KILLED. Without `arg0 = 0;` the
  second pointer binds `addu $16,$4,$2` (live $a0) instead of `addu $s0,$s2,$v0`.
  See rejected/no-fake-dead-store.c.
- H-s1-3 "naked s16 call args / s16 v0v trigger phantom frame slots" — KILLED.
  vars=0 both. See rejected/himode-arg-spellings-no-frame.c.

## Frontier (for s2+, modality: frame axis — vars 0 → 32)

1. **Phantom-slot spelling hunt (primary).** Target frame 72 = ALIGN8(vars 32)
   + args 24 + regs 16; our form is identical except vars=0. Per
   [[phantom-frame-slots-gcc272]] LIVE spellings can reserve untouched vars
   bytes (witness: tslLineG5Init vars=8 from an HImode bitwise expr; trigger is
   assign_stack_temp during expansion). Next probe: read GCC 2.7.2
   expr.c/optabs.c for every assign_stack_temp call site reachable from
   HImode/SImode scalar expansion; enumerate which expression shapes allocate
   temps; then find honest spellings of THIS function's existing computations
   (the 4 s16 stream reads, the +1, the <0x280 compare, the 5th-arg sext) that
   hit those sites. Instrument: cc1 `.frame vars=` via
   tmp/grind/func_800481E8/s1/frame_probe.sh (takes a .c path arg).
2. **32 = 4×8?** tslLineG5Init got 8 bytes per triggering expression. Our loop
   has exactly 4 s16 stream loads → if each can be spelled to allocate one
   8-byte temp region, vars=32 falls out naturally. Probe: single-trigger form
   first (does ONE spelling give vars=8?), then replicate ×4.
3. **Fallback (only if phantom search measured dead across the expansion-site
   map): written-never-read array carve-out does NOT apply** (target has no
   dead stores — 72-byte frame with zero stores below 56). The old
   `s32 pad[8]` route stays FORBIDDEN (dead-vars-local-array, unwritten form).
   If the expansion-site map proves no honest spelling can reach vars=32,
   that evidence goes to escalation modality — not before.

## [s1] The INLINE_MOVE_ALIASING __asm__ + $16 pin prologue is reproducible in pure C via the InitHiraRmd_80047FBC sibling technique (base-copy staging + function-scope call-arg precompute + FAKE arg0=0 dead store)
- mechanism: The function-scope precompute shifts whole-function RA so GCC stages arg0 through $s0 then copies $s2=$s0; the sanctioned dead-store-fake-exception `arg0 = 0;` breaks GCC's $a0==base value association so the second pointer binds addu $s0,$s2,$v0
- probe: cc1 dump of candidate (tmp/grind/func_800481E8/s1/text1b_cc1.s) then sandbox --disable all with edits in src/text1b.c
- result: cc1 emits target's exact `move $16,$4; move $18,$16` staging; sandbox 14 -> 10; build 56/56 insns; every non-frame instruction byte-matches
- verdict: CONFIRMED

## [s1] The FAKE arg0=0 dead store is removable while keeping the staging
- mechanism: Hoped copy-prop would still bind the second pointer to base after staging
- probe: Identical form minus the dead store (tmp/grind/func_800481E8/s1/text1b_probe_nofake.c), cc1 dump
- result: Staging survives but second pointer rebinds to live $a0: `addu $16,$4,$2` vs target `addu $s0,$s2,$v0` — dead store is load-bearing (banked rejected/no-fake-dead-store.c)
- verdict: KILLED

## [s1] Naked s16 call args (probe A) or s16-typed v0v with HImode increment (probe B) trigger phantom frame slots toward vars=32
- mechanism: phantom-frame-slots-gcc272: assign_stack_temp during expansion of narrow-mode expressions reserves untouched vars bytes
- probe: cc1 .frame vars= on both spellings (tmp/grind/func_800481E8/s1/probe_A.c, probe_B.c)
- result: vars=0 both — plain HImode add/compare/convert/arg-pass paths do not allocate stack temps; the known trigger is specifically HImode BITWISE expressions (banked rejected/himode-arg-spellings-no-frame.c)
- verdict: KILLED

## [s2] H-s2-1: The phantom slot comes from assign_stack_temp during RTL expansion; mapping the call sites in expr.c/optabs.c/stmt.c yields the triggering expression shapes (frontier item 1)
- mechanism: (as written by s1, from the pre-07-13 account of the mechanism)
- probe: Reproduced the tslLineG5Init trigger standalone with -da dumps (minrepro.py): the slot appears in NO expansion dump; the lreg dump names the orphan pseudo with degenerate class ("Register 77 ... dies in 0 places; ST_REGS or none")
- result: The slot is reload alter_reg's stale-ref orphan (func_80037540 H14, gdb-confirmed there); expansion-site mapping is the wrong question — the right question is combine's deletion condition, which was then bisected (H-s2-2)
- verdict: KILLED (superseded; do not map assign_stack_temp sites for this)

## [s2] H-s2-2: A live phantom-slot spelling of the existing loop computations (4 s16 stream reads, +1, <0x280 compare, 5th-arg sext) can reach vars=32 with zero dead code (frontier item 1/2, the primary)
- mechanism: Orphan a pseudo per stream value by making combine delete a redundant sign-extension chain while the emitted bytes stay the target's 56
- probe: 14-case entry-condition bisect (minrepro.py/bisect2.py) + 11-variant hand grid on the real chassis (probe.py) + 52,043-iteration instrumented permuter campaign logging cc1 vars= per candidate (perm/, classify_hits.py)
- result: Bisect: the orphan REQUIRES a combine-deleted 2-insn sll16/sra16 chain, and every deletion route visibly changes the bytes (chain not emitted / lhu→lh / extra consumer insns). This function's target keeps all 4 lhu + all 4 sll/sra pairs live and has NO low-bit-only halfword consumer → no hostable deletion. Hand grid: all honest re-spellings vars=0 (holder forms also lh-convert = byte-break). Campaign: 0 clean forms above vars=8, 0 clean forms near byte-identity, all 6 vars=32 hits volatile cheats; only honest-score find is the volatile dead-pad cheat (banked rejected/)
- verdict: KILLED

## [s2] H-s2-3: vars=32 decomposes as 4 triggers x 8 bytes matching the 4 stream loads (frontier item 2)
- mechanism: One 8-byte orphan slot per halfword load, replicated x4
- probe: Same instruments as H-s2-2 (single-trigger hunt first, per the frontier)
- result: A fortiori dead: not even ONE byte-neutral trigger exists in this function's semantics (H-s2-2); clean vars>8 was never observed in 52k candidates
- verdict: KILLED

## [s2] CONFIRMED (project-wide instrument): the minimal phantom trigger is B3 — ONE s16 local (u16-MEM loadable) with a sign-extension-needing consumer (e.g. `v < 640`) plus a raw low-bit consumer (`v & 1`); u16/zero-extension paths can NEVER orphan (promotion is 1-insn andi or free — no intermediate pseudo exists)
- mechanism: The HImode promotion chain (sll16/sra16 via intermediate pseudo) is deleted by combine when high bits die or the load absorbs the extension; flow's ref counts go stale; regclass defaults the intermediate to ST_REGS; alter_reg gives it a frame slot
- probe: minrepro.py T0-T5 + bisect2.py B1-B8, orphans read from lreg dumps
- result: Triggers: T0, T3, B2, B3, B5, B7, B8. Non-triggers: T1, T2, T4, T5, B1, B4, B6. B5 proves a live sll/sra of the same value can coexist with the phantom
- verdict: CONFIRMED

## Frontier (for s3+)
1. **Escalation modality (the mandated next step).** Every sanctioned frame
   route is measured dead: (i) written aggregate adds stores the byte-matched
   target lacks; (ii)/(iii) volatile & address-escape are cheats; (iv) phantom
   measured dead this session; (v) unwritten tail forbidden (no dead stores in
   target → 2026-07-01 carve-out inapplicable). File the OWNER-ESCALATION entry
   in docs/grind/decisions.md citing s1+s2 evidence; note the identical pending
   family question on file_LoadSectors (owner-gated 2026-07-27) and the
   endgame-lock-disposition standing policy (2026-07-20). Residual is EXACTLY
   10 frame-offset instructions; every other byte matches.
2. If the owner sanctions an unwritten-tail-class disposition for this family,
   candidate.c + `s32 pad[8]`-class construct closes at distance 0 — FORBIDDEN
   until such a ruling exists; do not pre-build it.
3. Cross-function lead (not this function's work): B3-shaped targets (signed
   halfword compare + low-bit test of the same value) among the 28 untouched-
   frame-slack census functions may dissolve via the now-characterized minimal
   trigger.

## [s2] Phantom slots come from assign_stack_temp during RTL expansion; mapping expr.c/optabs.c/stmt.c call sites yields triggering shapes (s1 frontier item 1)
- mechanism: Pre-2026-07-13 account of the phantom mechanism, inherited by the s1 frontier
- probe: Reproduced tslLineG5Init trigger standalone with -da dumps (minrepro.py): slot in NO expansion dump; lreg names the orphan ('Register 77 ... dies in 0 places; ST_REGS or none')
- result: Slot is reload alter_reg's stale-ref orphan (= func_80037540 H14); expansion-site map is the wrong question — combine's deletion condition is the right one, and was bisected
- verdict: KILLED

## [s2] A live phantom-slot spelling of the existing loop computations can reach vars=32 with zero dead code (s1 frontier primary)
- mechanism: Orphan pseudos by making combine delete redundant sign-extension chains while emitting the target's exact 56 insns
- probe: 14-case entry-condition bisect (minrepro.py/bisect2.py) + 11-variant hand grid (probe.py) + 52,043-iteration permuter campaign with per-candidate cc1 vars= logging and vars>0 source capture (perm/, classify_hits.py)
- result: Orphan REQUIRES a combine-deleted 2-insn sll16/sra16 chain; every deletion route visibly changes bytes (chain unemitted / lhu->lh / extra consumer insns). Target keeps all 4 lhu + all 4 sll/sra live and has NO low-bit-only halfword consumer. Hand grid: all vars=0 (holder forms lh-convert = byte-break). Campaign: 8,644 vars>0 sources = 7,127 volatile + 1,457 addr-escape + 60 'clean' ALL at vars=8 with >=33 body diffs and broken semantics; all 6 vars=32 hits volatile cheats; only honest find = volatile dead-pad (score 202), banked rejected/
- verdict: KILLED

## [s2] vars=32 decomposes as 4 triggers x 8 bytes matching the 4 s16 stream loads (s1 frontier item 2)
- mechanism: One 8-byte orphan slot per halfword load, replicated x4
- probe: Single-trigger hunt via the same instruments
- result: A fortiori dead: not even ONE byte-neutral trigger exists in this function's semantics; clean vars>8 never observed in 52k candidates
- verdict: KILLED

## [s2] The minimal phantom trigger is ONE s16 local (u16-MEM loadable, lh NOT required) with a sign-extension-needing consumer plus a raw low-bit consumer; zero-extension paths can never orphan
- mechanism: HImode promotion chain (sll16/sra16 intermediate pseudo) deleted by combine post-flow when high bits die or the load absorbs the extension; stale refs -> ST_REGS default class -> alter_reg frame slot. u16 promotion is 1-insn andi or free — no intermediate pseudo exists
- probe: minrepro.py T0-T5 + bisect2.py B1-B8 with lreg orphan readout; B5 shows a live sll/sra of the same value coexists with the phantom
- result: Triggers: T0 faithful, T3 (u16-MEM cast loads — our shape), B2, B3 (minimal: 'if (v<640)' + 'if (v&1)'), B5, B7 (HImode ADD — bitwise not required), B8. Non-triggers: T1 single-consumer, T2/B6 u16-everywhere, T4 const-only, T5 two-compare, B1 extended-2nd-use, B4
- verdict: CONFIRMED

## [s3] Both endgame-lock AND-gates pass for func_800481E8 (either strong scan_hand_coded signal or a cited SOTN-master precedent for the unwritten-tail phantom-frame family)
- mechanism: Gate 1 requires S1/S2/S6 STRONG signals from tools/scan_hand_coded.py; gate 2 requires an in-hand SOTN file+line/commit cite for the specific construct family closing the diff
- probe: Re-read s1+s2 evidence and hypothesis banks (memory/grind/func_800481E8/evidence.md + hypotheses.md); apply the two-gate lens from the standing 2026-07-27 auto-ruling; cross-check against the parallel file_LoadSectors 2026-07-28 disposition already terminal in docs/grind/decisions.md
- result: Gate 1 FAILS: target is verdict C, 56/56 insns byte-matched except 10 frame-offset instructions — compiled C with a frame-equation delta (vars=0 vs vars=32), no STRONG hand-coded signals apply. Gate 2 FAILS: s2's 5-way frame taxonomy is CLOSED (written aggregate adds stores target lacks; volatile/address-escape are cheats; phantom-slot mechanism measured dead in 52,043-iter instrumented campaign + 14-case bisect + 11-variant hand grid — no combine-deletable extension can exist in this function's semantics; unwritten-tail forbidden — target has zero stores below offset 56, so 2026-07-01 carve-out does not apply). No SOTN precedent for the unwritten-tail phantom-frame family (same negative census that terminated file_LoadSectors option a).
- verdict: KILLED
