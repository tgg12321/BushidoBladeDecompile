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
