# evidence — func_8001979C (src/code6cac.c)

## Session 1 (recon, 2026-08-13)

### Routing / floor
- `canonical func_8001979C` → verdict **C**, asm_insns 0, total 77, distance 24.
  Pure-C target; no canonical-asm question anywhere in this function.
- `sandbox func_8001979C --disable all` → **score 24**, target_insns 77,
  build_insns 75, 22 regfix rules dropped. This is the honest floor at HEAD.
- The 22 regfix rules on this function (regfix.txt:255–279) are a *map* of the
  residual, not a cheat to preserve: 4 `subst` clusters of `$v0`↔`$v1`
  renames, 2 `subst`s turning `srl`→`srlv`-position renames, 4 `subst`s
  rewriting `sh $x,0($9)` into `sh $x,0xa($9)` / `0x8e($9)`, 2
  `insert "addu $7,$2,$0"` (the missing copies), and 2 `reorder 15,14` /
  `42,41` (the preheader `li` order). They enumerate exactly the four diff
  families below and nothing else.

### What the function is
A bit-unpacker. `base = &D_800F1B18[arg0 * 0x570]`; `arg1` is a u32 bitstream
pointer that is first stored at `base[0]`. Then two identical 0x3F-iteration
loops each pull a fixed-width field out of a 32-bit `cur` accumulator
(12 bits into `base+0xA+2i`, then 2 bits into `base+0x8E+2i`), refilling `cur`
from `*arg1++` when fewer than `nbits` bits remain. A third small loop fills
four `-2` words at `base+0x458 - 0x118*k`, then `base[0x10C] = 0`.

### Instruction-level residual at the floor (75 ours vs 77 target)
Everything else in the function is already byte-identical, including the whole
`arg0 * 0x570` shift/subtract chain, the prologue, the third loop's
`addiu $v0,$t3,0x348` + `sw $v1,0x110($v0)` shape, and the epilogue.

| # | family | target | ours |
|---|---|---|---|
| D1 | missing copy (×2, one per loop) | `subu $v0,$t2,$a0` ; `addu $a3,$v0,$zero` | `subu $a3,$t2,$a0` |
| D2 | `$v0`/`$v1` role swap (×2 loops, + the final fill loop) | `hi` chain in `$v1`, scratch in `$v0`, `or $v1,$v1,$v0`, `sh $v1` | mirror image |
| D3 | biv/displacement (×2 loops) | `addu $t1,$t3,$zero` + `sh $x,0xA($t1)` / `0x8E($t1)` | `addiu $t1,$t3,10` / `,142` + `sh $x,0($t1)` |
| D4 | preheader constant order (×2 loops) | `li $t4,0xC` ; `li $t2,0x20` | `li $t2,32` ; `li $t4,12` |

**Register ASSIGNMENT already matches target at the floor.** `$t3` = base,
`$t2` = 0x20, `$t4` = the field width, `$t1` = walking store pointer,
`$t0` = counter, `$a3` = bits_left, `$a2` = cur, `$a1` = arg1, `$a0` = needed.
Only which of `$v0`/`$v1` holds which short-lived temp differs (D2). This is a
much better starting allocation than it looks from the score of 24, and it is
fragile — see H3.

### Measured probes (all reverted; src is back at HEAD)

**Probe A — single store at the if/else join (loop 1 only).**
Replaced the two per-arm `*(s16 *)(dst + 0xA) = …` with one store after the
if/else through a local `val`. Score 24 → 29, build_insns 75 → 76.
*But the D3 fold disappeared*: the preheader became `move $t1,$t3` and the
store became `sh $v0,10($t1)` — **exactly target's biv + displacement form**.
The +1 insn was a `sll`/`sra` sign-extension pair minus the removed second
`sh`, an artifact of routing the value through an `s32` local.

**Probe B — `s16 *dp` walking pointer, `dp[5]`, `dp++` (loop 1).**
Score 24, build_insns 75 — bit-for-bit the same output as the `u32 dst`
spelling. Pointer-typed vs integer-typed walker is codegen-neutral here.

**Probe C — named intermediate for the new bits_left (loop 1 only):**
`val = 0x20 - needed; bits_left = val;` instead of
`bits_left = 0x20 - needed;`. build_insns 75 → 76, score 24 → 34.
The new instruction is `subu $v0,$t3,$a0` ; `move $a3,$v0` — **D1's missing
copy, in exactly target's position**, confirmed by disassembly.

**Probe D — probe C applied to both loops** (file saved as
`variant_named_intermediate_77insn.c`). **build_insns 77 == target_insns 77**
for the first time; score 33. Every remaining diff in that form is register
naming plus D3. The cost is a one-allocno pressure increase that cascades the
entire allocation up a register (base `$t3`→`$t4`, consts `$t2`/`$t4` →
`$t3`/`$t5`, walker `$t1`→`$t2`) and, worst, evicts `hi` out of the caller-save
pair into `$t1`. At the floor and in target, `hi` shares its register with the
`0x20 - bits_left` sub-expression that produces it (`srlv $v1,$a2,$v1` in
target, `srlv $v0,$a2,$v0` at our floor); with the extra allocno that sharing
is lost (`subu $v0,…` ; `srlv $t1,$a2,$v0`).

**Probe E — probe D plus hoisting `needed = 0xC - bits_left;` above the
`hi = cur >> (0x20 - bits_left);` statement** (an attempt at D4's `li` order,
semantically neutral since both read the old `bits_left`). Score 33 → 43.
Strictly worse; killed.

### Mechanism notes
- **D3 is GCC's biv elimination in `loop.c`.** The walking pointer `dst` has no
  use in the loop other than the store address. With *two* `DEST_ADDR` givs at
  the identical address (one per if-arm) `combine_givs` merges them, the merged
  giv is reduced, `bl->all_reduced` becomes true and `maybe_eliminate_biv`
  deletes the biv, folding `+0xA` into its initial value. With *one* address
  giv (probe A) the giv is left as a plain displacement and the biv survives —
  which is target's form. So target's compilation had a single address giv in
  the loop, yet target's output still contains two `sh` instructions. Finding
  the C shape that has two stores but one address giv (or that otherwise blocks
  `combine_givs`/`maybe_eliminate_biv`) is the D3 job.
- **D1 is a failure-to-coalesce, and it is source-reachable**: routing the value
  through a second named local reliably produces the copy (probes C/D).
- **D2/D4 are allocation and LICM-emission ordering**, downstream of how many
  allocnos are live in the loop.

### Housekeeping
- `git checkout -- src/code6cac.c` was run at the end of the session; the tree
  is back at HEAD with only `metrics/events.jsonl` and this ledger dirty.

- [s1] canonical func_8001979C -> verdict C, asm_insns 0, total 77, distance 24. Pure-C target; there is no canonical-asm question anywhere in this function.

- [s1] sandbox func_8001979C --disable all at HEAD -> score 24, target_insns 77, build_insns 75, 22 regfix rules dropped. This is the honest floor.

- [s1] The function is a bit-unpacker: base = &D_800F1B18[arg0 * 0x570]; arg1 (a u32 bitstream pointer) is stored at base[0]; two identical 0x3F-iteration loops pull a 12-bit field into base+0xA+2i and then a 2-bit field into base+0x8E+2i out of a 32-bit 'cur' accumulator, refilling from *arg1++ when fewer than nbits remain; a third loop fills four -2 words at base+0x458-0x118*k; finally base[0x10C] = 0.

- [s1] The whole residual is exactly four families and nothing else: D1 = 2 missing insns (target 'subu $v0,$t2,$a0' + 'addu $a3,$v0,$zero' where we emit one 'subu $a3,$t2,$a0'), one per loop; D2 = a $v0/$v1 role swap through the shift/or chain in both bit loops AND in the final -2 fill loop; D3 = the folded store displacement in both bit loops; D4 = the preheader 'li' order (target 0xC then 0x20, ours 0x20 then 0xC).

- [s1] The 22 regfix rules at regfix.txt:255-279 enumerate precisely those four families (v0/v1 substs, sh 0(...) -> 0xa/0x8e substs, two 'insert addu $7,$2,$0', two reorders) and nothing else -- independent confirmation that the diff is fully characterised.

- [s1] Register ASSIGNMENT at the floor already matches target exactly: $t3 = base, $t2 = 0x20, $t4 = the field width, $t1 = walking store pointer, $t0 = counter, $a3 = bits_left, $a2 = cur, $a1 = arg1, $a0 = needed. Only which of $v0/$v1 holds each short-lived temp differs. This allocation is fragile -- one extra loop allocno cascades all of it (measured in probe D).

- [s1] At the floor and in target, 'hi' shares its register with the '0x20 - bits_left' sub-expression that produces it (target 'srlv $v1,$a2,$v1', floor 'srlv $v0,$a2,$v0'); the named-intermediate form loses that sharing ('subu $v0,...' then 'srlv $t1,$a2,$v0'), which is the specific casualty to fix in F2.

- [s1] Everything outside those four families is already byte-identical, including the arg0*0x570 shift/subtract chain, the prologue, the third loop's 'addiu $v0,$t3,0x348' + 'sw $v1,0x110($v0)' shape, and the epilogue.

- [s1] src/code6cac.c was reverted with 'git checkout --' at the end of the session; the working tree carries only metrics/events.jsonl and the memory/grind/func_8001979C/ ledger.
