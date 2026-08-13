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

## Session 2 (structural, 2026-08-13) — floor 24 -> 20

### Headline
Instruction parity AND target's exact register assignment reached at the same
time for the first time. `sandbox --disable all` = **20**, target_insns 77,
build_insns 77. Form in place in `src/code6cac.c`; also saved as
`candidate.c`. D1 and D3 are CLOSED; the residual is D2 (a $v0/$v1 mirror
inside the two if-arms) plus D4 (preheader order / walker-init placement).

### Correction to a session-1 evidence line
s1 recorded that the third (`-2` fill) loop is $v0/$v1-mirrored at the floor.
It is not. Disassembling the floor object directly
(`tmp/grind/func_8001979C/s2/floor.txt`) shows `li v1,-2 ; li t0,3 ;
addiu v0,t3,840`, which is byte-identical to target's
`addiu $v1,$zero,-0x2 ; addiu $t0,$zero,0x3 ; addiu $v0,$t3,0x348`.
D2 is confined to the two bit loops; the third loop was never mirrored.

### The D3 mechanism, resolved exactly (reading tools/gcc-2.7.2/loop.c)
`strength_reduce` (loop.c:3779-3852) decides per giv:

    benefit  = v->benefit;
    benefit -= add_cost * bl->biv_count;          /* loop.c:3804 */
    if (v->lifetime * threshold * benefit < insn_count && ! bl->reversed)
      { v->ignore = 1; all_reduced = 0; }         /* loop.c:3823 */

with `threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)` — a large
number (loop.c:3241), so the predicate only fires when `benefit <= 0`.
`combine_givs` (loop.c:5500) pools the two identical per-arm DEST_ADDR givs
(`combine_givs_p` returns 1 immediately for equal mult_val+add_val,
loop.c:5464), doubling `benefit` and `lifetime`. `maybe_eliminate_biv` is only
attempted when `all_reduced == 1` (loop.c:4035).

So: two arms, one increment => pooled benefit ~ 2b - add_cost > 0 => reduced =>
biv eliminated => `addiu $t1,$t3,10` + `sh $x,0($t1)` (our old floor).
Two arms, TWO increments (`bl->biv_count == 2`) => 2b - 2*add_cost == 0 =>
ignored => all_reduced 0 => biv survives => `move $t1,$t3` + `sh $x,0xA($t1)`
— target's form, with both stores kept. Confirmed by measurement, not
inference (probe P1 below).

### The allocation mechanism, measured with the instrumented cc1
`tools/gcc-2.7.2/cc1` with `BB2_ALLOC_DEBUG=1` prints
`ALLOCDBG func=... ord=N pseudo=P hardreg=H nrefs=R livelen=L pri=X`, i.e.
global.c's `allocno_compare` priority `floor_log2(R)*R/L*10000` and the
resulting allocation order. Reproduce with
`bash tools/wsl.sh 'sh tmp/grind/func_8001979C/s2/alloc.sh'`
(script kept at that path; it cpp's src/code6cac.c and runs the instrumented
cc1 with the canonical -G0 flags). In-loop refs are weighted x2.

Measured allocno priorities:

| form | bits_left | counter | walker(s) | resulting regs |
|---|---|---|---|---|
| session-1 floor | 17846 | 14482 | 11739 x2 (reduced givs) | a3 / t0 / t1 = target |
| P5 one walker, 2 incs | 17313 | 13548 | 19259 (nrefs 26, livelen 54) | walker a3, bits_left t0, counter t1 |
| P6 split walkers | 17313 | 13548 | 14444 x2 (nrefs 13, livelen 27) | bits_left a3, walkers t0, counter t1 |
| **P7 (current)** | **17313** | **14000** | **13928 / 12580** | **a3 / t0 / t1 = target** |

The duplicated increment adds exactly 4 weighted refs per walker (9 -> 13),
which is enough to lift a walker above the counter. A single walker shared by
both loops can never be fixed this way: at nrefs 26 it needs livelen > 76 to
fall below the counter and the longest live range in the whole function is 69.
Splitting the walker per loop (nrefs 13) and lengthening each live range by
moving its initialisation as early as its loop allows is what closes it.

### Measured probes (scores are `sandbox --disable all`)
- **P1 — `dst += 2` duplicated into both arms, loop 1 only.** 24 -> 45,
  build_insns 75. Score worse but the loop-1 store became `sh v0,10(a3)` with
  preheader `move a3,t3` — D3 closed, at the cost of a register rotation.
- **P2 — same for both loops.** 44, build_insns 75. D3 closed in both loops.
- **P3 — P2 + session-1's named-intermediate D1 lever in both loops.** 49,
  build_insns 77 == target_insns. All 77 instructions present, same
  opcodes in the same order as target; the entire residual is register naming.
  `hi` landed in $t2 and pushed base/consts up one register.
- **P4 — P3 with `val` also carrying the `0x20 - bits_left` shift amount.**
  43, 77 insns. `hi` moved out of $t2 back into the $v0/$v1 pair; base $t3,
  consts $t2/$t4 restored.
- **P5 — P4 + a separate walker (`out`) for the third loop.** 40, 77 insns.
  Walker nrefs 33 -> 26.
- **P6 — P5 + per-loop walkers `dst` / `dst2` for the two bit loops.** 31,
  77 insns. bits_left correctly in $a3; walkers $t0 and counter $t1 still
  swapped (walker pri 14444 vs counter 13548).
- **P7 — P6 + `dst = base;` hoisted above the `sw`/`lw`/`addiu` preamble and
  `dst2 = base;` moved ahead of `i = 0;`.** **20**, 77 insns. Walker livelens
  31 / 28, counter livelen 60: priorities become 17313 / 14000 / 13928 / 12580
  and the register assignment matches target exactly.
- **P8 — P7 with `hi = cur >> (0x20 - bits_left);` (anonymous temp) instead of
  reusing `val`.** 31, 77 insns. Strictly worse; the `val` reuse is
  load-bearing.

### Artifacts
- `tmp/grind/func_8001979C/s2/floor.txt` — floor disassembly (the s1 D2
  correction).
- `tmp/grind/func_8001979C/s2/p7.txt` — current-form disassembly.
- `tmp/grind/func_8001979C/s2/alloc.sh` — ALLOCDBG reproduction script.
- `tmp/grind/func_8001979C/s2/alloc_floor.txt`, `alloc_p5.txt` — priority dumps.
- `tmp/grind/func_8001979C/s2/dump.sh` + `dump/code6cac.i.greg` — cc1 -da
  register-disposition dump (note: cc1 prints unrelated parse errors for
  `GameObj` prototypes later in the file and still emits every dump; harmless).
- `memory/grind/func_8001979C/rejected/` — P4, P5, P6 saved as named forms.

### Housekeeping
`src/code6cac.c` is left carrying the P7 form (score 20). The only other dirty
file is `metrics/events.jsonl`.

- [s2] [s2] sandbox func_8001979C --disable all with the session-2 form in src/code6cac.c: score 20, target_insns 77, build_insns 77. This is the first form that has both instruction parity and target's register assignment; the previous floor was 24 at build_insns 75.

- [s2] [s2] Register assignment in the session-2 form matches target exactly: $a3 = bits_left, $t0 = counter, $t1 = walking pointer, $t2 = 0x20, $t3 = base, $t4 = field width, $a2 = cur, $a1 = arg1, $a0 = needed.

- [s2] [s2] Three source changes are jointly load-bearing and each was measured: (1) 'dst += 2' duplicated into both if-arms of both bit loops (closes D3 via loop.c biv_count); (2) the local 'val' carries BOTH 0x20 - bits_left and 0x20 - needed (the second is session 1's confirmed D1 lever; the first keeps 'hi' inside the $v0/$v1 pair); (3) per-loop walking pointers dst / dst2 / out with each initialisation placed as early as its loop allows.

- [s2] [s2] loop.c's giv reduction predicate is effectively 'benefit <= 0': threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs) (loop.c:3241) is large, so 'v->lifetime * threshold * benefit < insn_count' (loop.c:3823) can only fire when benefit is non-positive. benefit is reduced by add_cost * bl->biv_count at loop.c:3804, which is why doubling the biv increments is what flips it.

- [s2] [s2] combine_givs_p (loop.c:5464) returns 1 unconditionally for two givs with equal mult_val and add_val, so two identical per-arm DEST_ADDR givs ALWAYS pool their benefit and lifetime. There is no C shape that keeps two identical store addresses from combining; the reachable lever is biv_count, not giv count. This closes session 1's frontier item F1 as mis-aimed.

- [s2] [s2] The instrumented cc1 at tools/gcc-2.7.2/cc1 prints 'ALLOCDBG func=... ord=N pseudo=P hardreg=H nrefs=R livelen=L pri=X' under BB2_ALLOC_DEBUG=1, which is global.c's allocno_compare priority floor_log2(R)*R/L*10000 and the allocation order. Reproduction script: tmp/grind/func_8001979C/s2/alloc.sh. In-loop references are weighted x2.

- [s2] [s2] Measured allocno priorities. Floor: bits_left 17846 / counter 14482 / two reduced-giv walkers 11739. P5 one shared walker: bits_left 17313 / counter 13548 / walker 19259 (nrefs 26, livelen 54). P6 split walkers, late inits: walkers 14444 each (nrefs 13, livelen 27) still above counter 13548. P7 (current): bits_left 17313 / counter 14000 (nrefs 21, livelen 60) / dst2 13928 (livelen 28) / dst 12580 (livelen 31).

- [s2] [s2] The walker priority window is narrow: at nrefs 13 a walker needs livelen >= 29 to fall below the counter. Moving 'dst2 = base;' back after 'i = 0;' costs exactly one livelen unit and re-inverts the walker/counter order (that regression is measured as P6, score 31). This couples D4's preheader order to the allocation and means D4 cannot be fixed by statement re-ordering alone.

- [s2] [s2] Remaining residual at score 20 is two families. D2: target keeps 'hi' in $v1 SHARING the register of the 0x20 - bits_left temp ('srlv $v1,$a2,$v1') with 'val' and 'cur >> bits_left' in $v0; we emit the mirror (temp $v1, hi $v0, val $v1, cur >> bits_left $v1), because our 'val' is re-assigned later in the same arm and so is still live at the srlv. D4: target's preheader is 'move $t0,zero ; li $t4,<width> ; li $t2,0x20 ; move $t1,$t3' with the walker init last and the width constant before 0x20.

- [s2] [s2] cc1 -da dumps for this file are reproducible with tmp/grind/func_8001979C/s2/dump.sh. cc1 prints unrelated 'parse error before GameObj' diagnostics for prototypes later in code6cac.c (GameObj is not typedef'd in the preprocessed unit) and still emits every dump file; this is pre-existing and harmless.

- [s2] [s2] src/code6cac.c is left carrying the score-20 form; the same body is saved at memory/grind/func_8001979C/candidate.c with a full header explaining each lever. Rejected intermediates saved under memory/grind/func_8001979C/rejected/ as single-walker-both-loops-score40.c, single-walker-loop3-shared-score43.c and split-walkers-late-init-score31.c.
