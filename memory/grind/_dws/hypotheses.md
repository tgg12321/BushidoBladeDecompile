# Hypothesis ledger — _dws

## s1 (2026-09-07, recon)

### H1 — CONFIRMED. Declaration fix: the parameter is Sony's 4-halfword RECT, not `_GpuChunkHdr_CE0C`.
Mechanism: struct-member granularity drives the load/store widths and offsets that
`expand_expr` emits; an `s32 unk0` at +0x0 cannot produce `lh/sh 0x4` + `lh/sh 0x6`
alongside `lw 0x0` / `lw 0x4`.
Probe: replaced the typedef with `GpuRect { s16 x, y, w, h; }` and applied the banked
psyz body. Result: floor 143 (INCLUDE_ASM) -> 32 in one measurement, with the prologue,
both clamps, the divide expansion, the GP0 wait loop and the DMA tail already
instruction-identical. See `evidence.md` OBJECT MODEL.

### H2 — CONFIRMED. The odd-word copy loop is `while (--n != -1)`, not psyz/SOTN's `while (n--)`.
Mechanism: `while (n--)` makes GCC 2.7.2 keep a pre-decrement copy of the counter and
compare it to zero (`move` + `beqz`/`bnez`); the target instead hoists a `-1` constant into a
register above the loop and compares the post-decrement value to it. The two are semantically
identical for n >= 0 but are different RTL from `expand_expr` onward.
Probe: single-line substitution. Result: **32 -> 24, 143/143 insns** — the last real
codegen divergence in the function.

### H3 — KILLED (instance). The psyz seed's central warning — that GCC 2.7.2 would constant-fold
`var_s4 = 0; ... var_s4 ? 0xB0000000 : 0xA0000000` and therefore needed a defence — is false on
this chassis.
Mechanism claimed by the seed: jump-threading / constant propagation collapsing the always-false
branch. Probe: compiled the plain form with no defence at all. Result: GCC emits exactly the
target's `addu $s5,$zero,$zero` and `beqz $s5,...` with the `lui $a0,0xa000` of the false arm
hoisted into the wait loop's delay slots. The retired 2026-08 chassis's `register s32 var_s5
asm("s5")` pin and its empty inline-asm barrier were defending against a fold that does not
happen. Measured on: HEAD chassis 2026-09-07, zero FAKE constructs present, sandbox
`_dws --disable all`.

### H4 — KILLED (instance). The residual 24 after H2 was NOT codegen.
Mechanism: `engine/score.py::_resolve_named_pair` resolves a `%hi/%lo` pair only when the symbol
is defined in one of `cfg.LD_SYM_FILES`. The reference object (assembled from asm/funcs/_dws.s)
names the GPU register-pointer cells `D_8009BF44/48/4C/50/54`, which are `T` symbols in the asm
data segment and appear in NONE of those three files, so they keep a literal `0x0`; the C side
names them `g_gpu_*_reg`, which ARE in named_syms.txt, so they resolve to `@hi/@lo`. Twelve
`lui`+`lw` pairs x 2 lines = 24. Probe: `nm build/bb2.elf` (both names sit at the same address),
then `tmp/grind/_dws/s1/bytecheck.py`, which applies the object's own relocations with the real
linked addresses and compares against the shipped word encodings in asm/funcs/_dws.s:
`MISMATCHED WORDS: 0`. Confirmed independently by `verify-oracle --rebuild --allow-dirty`
-> `62efab4f73f992798c43e8c730aa43baa10bb4fa`, `build_matches: true`. Measured on: HEAD chassis
2026-09-07, zero FAKE constructs present.
Note this is a *general* trap for the rest of the libgpu cluster, not a `_dws` quirk.

## Frontier
Empty for `_dws` — the function is matched at 0 with no cheats and no rules. The carry-forward
work is on the two siblings `_clr` and `_drs` in the same TU (see evidence.md's durable-facts
section: same `GpuRect`, same `while (--n != -1)` loop spelling, same scorer trap).
