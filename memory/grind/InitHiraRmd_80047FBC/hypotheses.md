# Hypothesis ledger — InitHiraRmd_80047FBC

## Frontier (session s1 → s2)

### KEY OBSERVATION (correction from initial reading)
Sandbox at offset 0x1b4 ALREADY emits `sw v0, 16(sp)` in the jal delay slot — GCC
produces the discarded-return spill by itself (no explicit `buf[k] = call();` needed
in the C). The 32-byte `buf[8]` reserves the frame; GCC schedules the discarded v0
into the free slot in the delay slot. **Target and sandbox agree on the dead store.**
The only diff is target-insn-#18 `addu $s0, $s4, $v0` vs sandbox `addu s0, a0, v0`.

Consequence: making `buf[k] = call()` explicit is unlikely to move the residual — it
mostly re-legitimizes the frame reservation under the [[dead-vars-local-array]]
2026-07-01 carve-out (WRITTEN-never-read requirement), but does not attack the
copy-prop that keeps $a0 == arg0 at insn #18.

### H1 — force $s4 retention by making `base` visibly diverge from `arg0`
**Statement:** GCC copy-props `base == arg0` since both trace to $a0 with no
intervening redefinition. Instead of trying to break the association via a dead
self-assign to arg0 (proven KILLED, see H3), give `base` a real (byte-identical but
optimizer-opaque) reason to occupy a callee-save. Candidates:
  (a) `u32 *base; base = (u32 *)arg0; if (base != (u32*)0) { }` — trivially-true branch
      forcing a data-dependent live range on `base`.
  (b) reorder: compute `p = base + shifted_a1` in one statement, then reload `p` from
      `base + word_offset`; hope RA schedules `base` into $s4.
  (c) accumulate arg1's shift into `base` first via `base = arg0; base += shifted_a1;
      base -= shifted_a1;` — variant of split-init-accumulation (see
      [[split-init-accumulation-sanctioned]]).

**Probe (s2):** apply (a) first (least invasive); measure sandbox distance and dump
disasm. If distance stays at 1, iterate through (b) then (c).

### H2 — permuter directed pass on `base`/`arg0` binding
**Statement:** the residual is a single register-choice diff at a specific insn. This
is exactly the shape decomp-permuter's directed PERM_* macros are designed to sweep
(see [[permuter-directives]]). Auto-search may find a legal C form that the manual
levers in H1 miss.

**Probe (s2 or s3):** build a permuter workspace, seed with the current form minus
`arg0 = 0;` (which is a proven-dead lever we don't want the permuter re-discovering),
and run a directed campaign varying declaration order + copy chain.

### H2 — force $s4 retention via base-through-non-$a0 CSE
**Statement:** GCC copy-props `base == arg0` because both variables trace to $a0's
initial value and nothing between defines them differently. If `base` is initialized
from a *derived* value (e.g. `base = (u32*)arg0; base = (u32*)((s32)base + 0); base -= 0;`)
GCC may still fold — but if the assignment path passes through a callee-save
register naturally, the copy stays.

**Mechanism:** currently `p = (u32 *)arg0; base = p;` — `base` becomes an alias of $a0
after copy-prop. Alternative: assign `base` FIRST, then `p`, and make later uses of
`p` diverge from `arg0`. This might make GCC materialize `base` into a callee-save
because it's live across the loop's callees.

**Probe (s2):** rewrite as `u32 *base = (u32 *)arg0; u32 *p = base;` — declaration
order swap. Also try `u32 *p = (u32*)arg0; u32 *base = p;` vs `u32 *base = (u32*)arg0;
u32 *p = (u32*)((s32)base + shifted_a1);` (skip the intermediate `p = arg0` step).

### H3 — the current `arg0 = 0;` trick provably fails; not a viable rescue lever
**Statement:** the L108 `arg0 = 0;` in current src does NOT force $s4 usage at
insn #18 (sandbox still emits `addu s0, a0, v0`). Documenting this as a KILLED
mechanism so s2+ do not re-try the "dead self-assign breaks copy-prop" theory in this
function's shape.

**Verdict:** KILLED by s1 measurement (sandbox score=1 with the trick in place).

## Sibling context
- The whole 8004xxxx cluster (AddTbpOfst_80047EE8, InitHiraRmd_800480C0, func_800481E8)
  is cheat-carrying INCOMPLETE — do NOT copy their forms as templates.
- Target dead-store `sw $v0, 0x10($sp)` is the strongest signal in the target asm; H1
  is the primary lever for s2.

## [s1] L108 `arg0 = 0;` breaks GCC's $a0==arg0 value-association at the second `base` use, causing insn #18 to bind to $s4
- mechanism: dead self-assign-to-param as a copy-prop severing trick (Lever D variant; forbidden without /* FAKE */ + lever-exhaustion)
- probe: current src already has the construct; run sandbox --disable all and read insn #18
- result: sandbox score=1; insn #18 is `addu s0, a0, v0` (still $a0). Mechanism does not fire in this function's shape.
- verdict: KILLED

## [s1] the dead `sw $v0, 0x10($sp)` in target's jal delay slot requires an explicit `buf[k] = efc_buki_draw_zanzou(...);` capture to be legitimized under the dead-vars-local-array 2026-07-01 carve-out
- mechanism: written-never-read local array with target dead-stores matched by explicit stores in the C source
- probe: objdump sandbox .o at jal delay slot
- result: sandbox ALREADY emits `sw v0, 16(sp)` at offset 0x1b4 with no explicit buf[k]= in the C — GCC schedules the discarded return into the free frame slot on its own. The store is not the residual; the copy-prop is.
- verdict: KILLED

## [s2] H1a: assigning base FIRST (u32 *base = (u32*)arg0) then deriving p from base fixes insn #18 to $s4
- mechanism: removing the intermediate `p = arg0; base = p` chain makes base the primary reference so GCC materializes it directly into $s4
- probe: s2 sandbox --disable all: base = (u32*)arg0; p = base + shifted; p = base + word
- result: score 5, build 64 insns. Insn 0x120 IS now addu s0,s4,v0 (matches target), but prologue emits `move s4,a0` directly (single move) instead of target's staged `move s0,a0; move s4,s0` — save order flips (s4 saved before s0) and one insn is eliminated. Net: fixes one diff, introduces multiple new diffs.
- verdict: KILLED

## [s2] H1b: parallel init (p = arg0; base = arg0) produces a different codegen than the chained (p = arg0; base = p) form
- mechanism: distinct initializers might give GCC different value-numbering / copy-prop analysis
- probe: s2 sandbox --disable all: p = (u32*)arg0; base = (u32*)arg0; p = base + shifted; p = base + word
- result: score 1, byte-identical to the chained baseline. GCC folds parallel and chained inits to identical output — copy-prop collapses base==arg0 either way. Same single residual at insn #18.
- verdict: KILLED

## [s2] H1c: declaring base in an inner scope (after p is derived) shortens its lifetime and improves allocation
- mechanism: narrower live-range of base might let GCC pick a different register or avoid the copy-prop tie
- probe: s2 sandbox --disable all: outer p = arg0; p += shifted; then inner { u32 *base = (u32*)arg0; ... loop ... }
- result: score 8, build 63 insns. Inner-scope base drops the s4 setup entirely and shifts the loop-body allocation, introducing multiple new diffs. Strictly worse.
- verdict: KILLED

## [s2] H1d: declaring base as s32 base_addr (integer) instead of u32 *base (pointer) gives GCC a different aliasing/allocation profile
- mechanism: s32 vs pointer type might affect pointer-aliasing analysis, address-mode selection, or RA cost model
- probe: s2 sandbox --disable all: p = (u32*)arg0; base_addr = arg0 (as s32); p = (u32*)(base_addr + shifted); p = (u32*)(base_addr + word)
- result: score 1, byte-identical to H1b. GCC treats s32 and u32* the same after copy-prop; the address arithmetic collapses to identical MIPS. Same single residual, BUT reaches score=1 WITHOUT the arg0=0 cheat (previously believed load-bearing).
- verdict: KILLED (bytes) / CONFIRMED as inert-cheat-eliminator (progress: reduces committed candidate from 2 cheats to 1)

## [s2] H3 confirmation: the `arg0 = 0;` dead self-assignment in the s1-committed candidate is INERT for insn #18
- mechanism: prior belief (per s1) was that this KILLED the copy-prop severance mechanism; H1d now confirms score is identical WITHOUT it, so it is providing zero codegen effect
- probe: s2: removed arg0 = 0; from src, re-ran sandbox with H1d form
- result: score 1 (unchanged from with the cheat). The construct provides zero benefit. Committed candidate carries it purely as a failed hypothesis remnant.
- verdict: CONFIRMED (arg0=0 is inert here — safe to drop from candidate)
