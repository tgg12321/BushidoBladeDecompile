# Evidence bank — func_8006F038

- Audit diagnosis (regressions.md): saved block (src/text1b.c:16447-16451) fails Tests 1+2: `s32 saved = temp_s0; ot_Link(D_800A374C, temp_s0); temp_s0 = saved + 0x10;` is byte-identical to `ot_Link(D_800A374C, temp_s0); temp_s0 += 0x10;` — GCC coalesces saved to $s0, same delay-slot `addiu s0,s0,0x10` either way. Likely redundant (not load-bearing), not a classic bytes-from-asm cheat, but still fails the 6-test bar. Clean replacement: remove saved block, write `ot_Link(D_800A374C, temp_s0); temp_s0 += 0x10;`, verify oracle still passes. If oracle breaks, saved IS load-bearing — document why and surface to owner.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Construct located: src/text1b.c:16445-16449 `{ s32 saved = temp_s0; ot_Link(D_800A374C, temp_s0); temp_s0 = saved + 0x10; }` - a named-intermediate save/restore around a call, no FAKE annotation, no semantic purpose (saved == temp_s0 throughout).

- [s1] [fable-blitz 2026-07-07] Target bytes (asm/funcs/func_8006F038.s:25-29): `addu $a1,$s0,$zero` captures the OLD temp_s0 as ot_Link's arg BEFORE the jal, and `addiu $s0,$s0,0x10` executes in the jal DELAY SLOT. This is standard reorg.c fill_slots_from_thread behavior: a post-call increment of a callee-saved reg that is not a call arg is eligible to move into the call's delay slot.

- [s1] [fable-blitz 2026-07-07] The plain spelling `ot_Link(D_800A374C, temp_s0); temp_s0 += 0x10;` produces the same RTL after coalescing (saved's pseudo coalesces into temp_s0's $s0 anyway - flow.c/regclass copy coalescing), so the diagnosis's own prediction (byte-identical) is well-founded; the block is permuter residue, not a load-bearing device.

- [s1] [fable-blitz 2026-07-07] Secondary shape worth preserving in any edit: the `sw $s0,0x14($s1)` store (src 16450) is scheduled INTO initTexPage's arg-setup region (asm line 32, between a2=0 and the 0x10(sp) home store) - source statement order (store before the initTexPage call) is already correct for this; do not reorder those statements while cleaning.

- [s1] [fable-blitz 2026-07-07] The three D_800A3550 reloads (src 16434/16438/16441 -> asm lines 11/16/20) are honest per-use gp-rel reloads interleaved with the stores - the v1/v2/v3 named-intermediate pattern already matches target and is not part of the flagged construct; leave untouched.

- [s1] [fable-blitz 2026-07-07] If the plain form DOES regress (unlikely), the diagnosis instructs: document why saved is load-bearing and surface to owner - note that under the 2026-07-06 allowed-list, a FAKE-annotated named intermediate is a sanctionable spelling, so the surface would be an annotation request, not a park.
