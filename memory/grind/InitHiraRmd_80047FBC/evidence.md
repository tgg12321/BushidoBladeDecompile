# Evidence bank — InitHiraRmd_80047FBC

## Session s1 (recon, 2026-07-20)

- Canonical gate: verdict=C, distance=1 (pure-C target).
- Sandbox `--disable all`: honest score=1, target=65 insns, build=65 insns.
- Current src/text1b.c:77 body ALREADY carries two forbidden constructs and STILL misses by 1 insn:
  - `s32 buf[8];` at L101 with only `(void)buf;` at L142 — WRITTEN-NEVER-READ carve-out
    ([[dead-vars-local-array]] 2026-07-01) requires actual STORES into the array; this
    array has zero stores, so it is a plain unused-array cheat (catalog entry, forbidden).
  - `arg0 = 0;` at L108 — dead self-assignment of a parameter; forbidden by
    [[inline-asm-policy]] expanded catalog unless annotated `/* FAKE */` after documented
    lever-exhaustion per [[dead-store-fake-exception]]. Neither is in place.

## The residual — sandbox vs target diff

Sandbox and target agree on 64/65 instructions. The single divergence is at target
insn #18 (offset 0x387FC, `addu $s0, $s4, $v0`) vs sandbox offset 0x24
(`addu s0, a0, v0`). Second computation of `p = (u32 *)((s32)base + (((*p) >> 2) << 2))`:

- Target uses $s4 (the callee-save copy of arg0 made in prologue: `addu $s4, $s0, $zero`).
- Sandbox uses $a0 directly — GCC copy-props `base == arg0` since $a0 still holds arg0
  at this point.

The current C body's L108 `arg0 = 0;` was written specifically to sever this
copy-prop association ("breaks GCC's $a0==arg0 value association, so this reference
binds to $s4"). Measurement: **it doesn't work** — sandbox still emits `addu s0, a0, v0`.
The stated mechanism failed.

## The dead store in target — `sw $v0, 0x10($sp)` in the jal delay slot

Target insn #55 (`sw $v0, 0x10($sp)` in the delay slot of `jal efc_buki_draw_zanzou`)
stores a value into stack slot 0x10 that is never reloaded. The offset 0x10 lies
inside the 32-byte frame block above the callee-save area (buf[0..7] at sp+0..sp+0x1c
if buf were the first local — so 0x10 = buf[4]). This is strong evidence that the
original C had a WRITTEN dead-store — either:
  (a) A written-never-read local array with `buf[k] = efc_buki_draw_zanzou(...);`
      qualifying under the [[dead-vars-local-array]] 2026-07-01 carve-out; OR
  (b) An earlier-frame home for a discarded call return via
      [[restore-discarded-return-displaces-v0]] variants.

Sandbox current form has zero explicit stores at sp+0x10; the store is coming
from some other codegen path — need to inspect. (Left for s2 to trace.)

## Sibling shape

`AddTbpOfst_80047EE8` (src/text1b.c:19) — same cluster — carries `register asm` pins +
`INLINE_MOVE_ALIASING` __asm__ move + `s32 unused_slack[8]` + `(void)unused_slack;`.
That sibling is also cheat-carrying INCOMPLETE; not a template to copy.

`InitHiraRmd_800480C0` (src/text1b.c:144) — carries multiple `register asm("$N")` pins
too. Cheat-carrying.

`func_800481E8` (src/text1b.c:191) — carries `register asm("$16") cached` + a
`move %0,%1` __asm__ block (INLINE_MOVE_ALIASING). Cheat-carrying.

**Conclusion:** the entire cluster is currently living on the forbidden
INLINE_MOVE_ALIASING / dead-vars-local-array / register-asm-pin family. Cluster-wide
solution likely lies in a WRITTEN dead-store pattern that qualifies under the 2026-07-01
carve-outs, OR a genuinely different C shape that makes arg0-copy → $s4 natural.

## Artifacts
- `tmp/grind/InitHiraRmd_80047FBC/s1/sandbox_disasm.txt` — objdump of sandbox `.o`.

- [s1] canonical InitHiraRmd_80047FBC → verdict=C, distance=1 (pure-C target, 65 target insns)

- [s1] sandbox --disable all → score=1, rules_dropped=0, cheat_asm_stripped=393 (whole-file counter, not per-func)

- [s1] residual is single insn at target offset 0x387FC (insn #18) `addu $s0, $s4, $v0` vs sandbox offset 0x24 `addu s0, a0, v0` — computation of `p = base + ((*p >> 2) << 2)`

- [s1] current src/text1b.c:77 body carries TWO forbidden constructs and still misses by 1: (a) `s32 buf[8];` with only `(void)buf;` — unused array, no stores, does not qualify for the dead-vars-local-array 2026-07-01 WRITTEN-never-read carve-out; (b) `arg0 = 0;` at L108 — dead self-assignment of parameter, forbidden without /* FAKE */ + lever-exhaustion per dead-store-fake-exception

- [s1] sandbox and target BOTH emit `sw v0, 16(sp)` in the jal delay slot — GCC schedules the discarded efc_buki_draw_zanzou return value into the free slot in `buf[]` automatically; no explicit buf[k]= is required for that store to appear

- [s1] prologue and epilogue byte-match target — frame=0x50, save order $s0/$s4/$ra/$s3/$s2/$s1 all correct; the `buf[8]` declaration IS load-bearing for the 32-byte frame reservation

- [s1] sibling cluster (AddTbpOfst_80047EE8, InitHiraRmd_800480C0, func_800481E8) all remain cheat-carrying INCOMPLETE (register-asm pins, INLINE_MOVE_ALIASING __asm__ moves, unused_slack arrays) — NOT templates to copy

- [s2] [s2] baseline sandbox (as-committed src): score=1, 65/65 insns, single residual at insn #18 (target `addu $s0,$s4,$v0` vs sandbox `addu $s0,$a0,$v0`)

- [s2] [s2] the target prologue `move s0,a0; move s4,s0; addu s0,s0,a1` (staged copy through $s0) is produced BY BOTH chained-form (`p=arg0; base=p`) AND parallel-form (`p=arg0; base=arg0`) — GCC's copy-prop coalesces them

- [s2] [s2] the target prologue is NOT produced by declaration reorder that puts base first — H1a form emits `move s4,a0` directly (1 move, not staged) and flips save order

- [s2] [s2] H1d proves `arg0 = 0;` in the s1 candidate is inert — removing it leaves score=1 unchanged, so the committed candidate can be simplified to eliminate one of two cheats

- [s2] [s2] tried inner-scope base (H1c) as an alternative liveness lever; strictly worse (score 8, -2 insns) — narrower base lifetime is not the answer

- [s2] [s2] declaration type (s32 vs u32*) for `base` is codegen-neutral after copy-prop — H1b and H1d produce byte-identical .o

- [s2] [s2] the residual is a genuine RA tie-breaker: at insn #18 GCC picks $a0 (which still equals base after copy-prop) over $s4 (the callee-save copy). None of the 4 statement-level structural probes attempted budged this choice while preserving the target prologue.

- [s2] [s2] frontier: the split-init-accumulation form `base = arg0; base += shifted; base -= shifted;` proposed in s1 hypotheses is a cheat-by-any-spelling per [[no-new-park-categories]] — no-op arithmetic with the sole purpose of steering RA. NOT probed. Should be marked KILLED-BY-POLICY in future ledger updates.

- [s3] [s3] baseline sandbox with s2 candidate applied (H1d form, no arg0=0): score=1, 65/65 insns, single residual at insn #18

- [s3] [s3] `s32 buf[8]` local is load-bearing for FRAME reservation — removing it: score 1→15, frame 0x50→0x30, ~15 stack offsets shift; NOT decorative. Remains a forbidden un-written array unless legitimized under [[dead-vars-local-array]] 2026-07-01 carve-out (which requires actual STORES matching target dead stores; target's `sw v0,0x10(sp)` in the jal delay slot IS GCC-allocated on its own and does not by itself satisfy the written-not-read requirement).

- [s3] [s3] split-recompute form (`count = *(base+off); p = base+off+4`) does NOT lift base_addr's ref count as seen by local-alloc — CSE folds the two base+off computes into one before reg-alloc, so priority tiebreaker is unchanged; introduces new $a0-anchored diff.

- [s3] [s3] within-scope declaration-order and loop-body statement reorderings do not budge the insn #18 copy-prop tie (measured across 3 orderings, all score 1 or score 13).

- [s3] [s3] the residual is a robust GCC-internal tiebreaker between $a0 (arg-register-alias-via-copy-prop) and $s4 (callee-save-holding-base_addr) — five distinct structural probes have failed to invert it. This is exactly the shape the s2 frontier flagged as a permuter-directed problem rather than a manual-lever problem.

- [s3] [s3] rejected forms banked: rejected/p1_word_offset_split_recompute.c (score 3), rejected/p5_new_var_deferred_to_end.c (score 13).
