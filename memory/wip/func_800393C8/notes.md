# func_800393C8 (saSeInit2) — WIP (asmfix whole-body blob strike)

**File:** `src/code6cac_c_mid.c` (~line 1267). Blob: `asmfix.txt` ~13-15
(whole-body `delete_between` + `insert_before`, internal labels `.LsaSeInit2_*`).
Bytes are MASKED in the full build → improved-but-incomplete C is oracle-safe.

**What it is:** (1) linear scan of a 0xB4 x 0x10 table at `D_800F68E0` matching
mode/pos/rot, bumping `slot[2]` on a hit; (2) free-slot search from gp-rel
`D_800A3714`; (3) miss counter `D_800A3209` or a full record store.

## State (2026-08-18)

Honest sandbox `--disable all`: **baseline 57 → current 26** (122 vs 123 insns).
Current body = `tmp/393c8_S4.c`. All register-asm pins and the fabricated
`if ((u32)&frame_pad[0] == 0) { D_800A3209 = D_800A3209; }` frame hack from the
old body are DELETED — the body is now ordinary C.

## Levers that moved it (with mechanism)

1. **`u8 arg0/arg1` -> `s32`** (57→). Target does `move t4,a0` with NO mask and
   `sll v1,t4,0xc`; a `u8` param forces `andi $x,$a0,0xff` at every use. There is
   no prototype in `include/` — the two call sites in `src/code6cac_b.c` are
   implicit declarations, i.e. `int`, so `s32` is the faithful signature.
2. **`s16 rot` -> `s32 rot = raw >> 16`** — target sign-extends once
   (`sra t0,v0,0x10`) and holds it; an `s16` local re-extends at the use
   (`sll`+`sra` again).
3. **Drop the second explicit `tail` pointer** (58→53). One `slot` pointer with
   literal field offsets lets GCC's own strength reduction produce target's
   two-IV form (base for offsets 0/2, base+0xE for 4..0xE). Writing `tail`
   by hand produced a THIRD induction variable at base+0xC.
4. **`s16 idx` for the free-slot index** (53→30) — **the phantom frame slot**.
   Target's frame is `addiu sp,-8` with zero saves and zero `($sp)` refs.
   `.frame vars=` gradient (`tmp/frame_probe.sh`) over every local: `s16 idx` is
   the ONLY declaration that yields `vars= 8`; `s16` on state/want/age/cnt/i/
   mode/raw/next all give `vars= 0`. Faithful too — `D_800A3714` is `s16`.
   This one change also fixed the whole prologue: `move t4,a0` appears and the
   table base moves into `$a0`, matching target.
5. **`D_800A3714 = idx; if (D_800A3714 < 0xB4)`** (30→29) — re-reading the global
   lets cse replace the reload with a sign-extend of the stored value, which is
   target's `sh` then `sll/sra` pair.
6. **Drop the `want = state - 1` local** (29→26) — inline `state - 1` in the
   comparison; the named local perturbed the entry-block allocation.

## Killed levers (measured)

- shared `next`/`idx` split in loop 2: 30 (vs 29 for single `idx`).
- loop 1 as a goto-loop (no loop notes): 36. Loop 2 as `do{...}while`: 34.
  Both loops swapped: 41.
- loop 2 as `do { if (x) break; ... } while (y)` **or** `for(;;)` with two
  breaks: 31 / 127 insns — GCC ROTATES both, duplicating the exit test
  (`lh`+`bne` after the loop). The goto form is the only one that doesn't.
- loop 1 as `for (i = 0; i < 0xB4; i++, slot += 0x10)`: 26, identical to the
  `do/while` form. Either spelling is fine.

## Remaining residual (2 clusters, ~26 edit distance, -1 insn)

1. **Loop 1: we hoist `li t5,-1` into the preheader; target materializes
   `li v0,-1` inside the loop.** That extra live register is what swaps
   `t0`/`t1` (our high-IV vs `rot`) and `v0`/`v1` throughout loop 1 — most of
   the remaining score. Our loop 1 is a real `do/while` (gets
   `NOTE_INSN_LOOP_BEG`, so LICM fires); target's did not hoist.
2. **Loop 2: exactly mirrored** — target hoists `li t1,-1` out, we don't (our
   goto-loop gets no loop notes). Target also keeps TWO copies of the index
   (`move v1,v0` ... `move t0,v1`) where we coalesce to one; that is downstream
   of holding the hoisted `-1` live. Same cause: `mode` then lands in `$v1`
   (target) vs `$t0` (ours) for the final `or`.

Both clusters are ONE LICM decision each, in opposite directions. Simply
swapping the loop forms does not do it (measured above) — the next probe should
attack the hoist directly (loop.c `scan_loop` threshold / register pressure),
or hand the seed to decomp-permuter.

## Related
[[phantom-slot-frame-lever]] · [[phantom-frame-slots-gcc272]] · [[loop-note-fixes-delay-slot-steal]]
