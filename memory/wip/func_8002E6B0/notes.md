# func_8002E6B0 — WIP (asmfix whole-body blob strike)

**File:** `src/code6cac_b.c` (~line 1342). Blob: `asmfix.txt` lines ~11-12
(`delete_between .frame/.end` + whole-body `insert_before`). Bytes are MASKED in
the full build, so improved-but-incomplete C in the tree is oracle-safe.

**What it is:** point-in-triangle test. Centroid of 3 points via GCC's magic
`/3` (lui 0x5555/ori 0x5556/mult/mfhi/sra 31/subu), then three sign-of-cross-
product tests; `(a^b) >= 0` spelled `xor/nor/srl 31`; early exits return 0.

## State (2026-08-18)

Honest sandbox `--disable all`: **baseline 57 (93 insns) → current 64 (94 insns)**.
The score went UP but the build is structurally MUCH closer — see below. Current
body = `tmp/e6b0_G.c`.

## Levers measured

| variant | shape | score | insns | note |
|---|---|---|---|---|
| HEAD | x0..pz locals, `return 0` early exits, cross_center first | 57 | 93 | 4 callee-saved regs, `j`+`move v0,zero` tail block |
| shared-end-label | `result=0` + `goto end` | 60 | 97 | `result` lands in `$a2` + `move v0,a2` + extra `s6` save. KILLED |
| `return result` at exits | ditto but early `return result` | 57 | 93 | const-prop folds it back to HEAD's codegen. KILLED |
| exit-path return-set | `result = 0; goto end` per path | 62 | 91 | KILLED |
| **G — cross_point BEFORE cross_center** (current) | plain `return 0` exits, inline `arg[N]` derefs | **64** | **94** | **insn count == target AND all five callee-saved assignments match target exactly** |
| I — explicit per-product temps in target emission order | | 77 | 93 | KILLED |

## Why G is the better seed despite the higher score

Target uses 6 saved regs (s0-s5, `s4` a phantom) and 94 insns. HEAD used 5 saves
(4 real + phantom) / 93 insns. G reproduces target's exact allocation:
`s0=center_z, s1=center_x, s2=pz, s3=px, s5=mflo temp, s4=phantom`, frame -0x18,
94 insns. The score is edit-distance and is dominated by *caller-saved* temp
renames, which cascade off one remaining scheduling decision (below).

MIPS has no `REG_ALLOC_ORDER`, so allocation is 0..31 with call-clobbered
preferred (no calls here). `center_z` only reaches a callee-saved reg when its
live range spans to the last product — which is what putting `cross_point`
first buys.

## Remaining residual (the one open item)

Target computes `center_x` IMMEDIATELY after the two `mult`s
(`mfhi t7; ...; sra v0,v0,31; subu s1,t7,v0`). Ours holds the x-product high
word in `s0` and sinks `sra`/`subu` far down, because with cross_point first the
centroid chain has lower scheduling priority. Also target's x-sum lands in `$v0`
and z-sum in `$v1`; ours is swapped. Everything else is a rename cascade off
that.

Next move: decomp-permuter seeded from G (register/scheduling cascade is exactly
its domain) — see [[permuter-fresh-seed-discipline]]. Do NOT run it concurrently
with hand edits to `src/code6cac_b.c`.

## Related
[[shared-end-label]] · [[exit-path-return-set-cse-join]] · [[phantom-slot-frame-lever]]
