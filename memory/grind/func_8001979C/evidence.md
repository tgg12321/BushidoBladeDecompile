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

## Session 3 (structural, 2026-08-13) — floor stays 20; D2's mechanism CORRECTED

### Housekeeping first (important for the next session)
`src/code6cac.c` at the START of this session was back at the **session-1 HEAD
form (score 24)**, not the session-2 P7 form — the s2 src edit did not survive
into the commit the driver made. The first action of this session was to
re-apply `memory/grind/func_8001979C/candidate.c` to `src/code6cac.c`; the
sandbox then reproduced score 20 / build_insns 77 exactly. Session 3 ends with
the same P7 form in place and re-verified at 20. **Always re-apply candidate.c
and re-measure before probing.**

### Tooling added (reusable, all under tmp/grind/func_8001979C/s3/)
- `apply.py` — splices a variant body into `src/code6cac.c` (LF-safe, replaces
  the whole `void func_8001979C(...) { ... }` block). Run it with
  `wsl.exe -e python3 ...` from PowerShell or `python3 ...` from Git Bash.
- `dis.sh` — objdumps `tmp/sandbox/func_8001979C/code6cac.o` into a file.
- `cmp.py` — normalized side-by-side of our disassembly vs
  `asm/funcs/func_8001979C.s` (li/move/nop canonicalised, immediates
  decimalised, branch targets + %hi/%lo masked). Prints only the real diffs.
- `qty.sh` — runs the instrumented `tools/gcc-2.7.2/cc1` with BOTH
  `BB2_QTY_DEBUG=1` (local-alloc quantities) and `BB2_ALLOC_DEBUG=1`
  (global-alloc allocnos) and captures the raw log. The QTYDBG lines carry no
  function name; take the QTYDBG run immediately preceding the first
  `ALLOCDBG func=func_8001979C` line.

### The score-20 residual, line-exact (cmp.py on the P7 form)
Exactly 20 differing instructions, which is the whole score:
- **D2 = 14** — 7 lines per bit loop: `srlv`, `subu`, `addu a3`, `sllv`,
  `srlv`, `or`, `sh`; a perfect `$v0`↔`$v1` mirror of target.
- **D4 = 6** — 3 lines per bit loop: our preheader is
  `addu t1,t3,zero ; addu t0,zero,zero ; addiu t2,zero,0x20 ; addiu t4,zero,<w>`
  where target is
  `addu t0,zero,zero ; addiu t4,zero,<w> ; addiu t2,zero,0x20 ; addu t1,t3,zero`
  (walker init LAST, width constant BEFORE 0x20).
Nothing else differs. (`lui/addiu %hi/%lo` and the two branch-target lines that
cmp.py marks are masking artifacts, not diffs.)

### MECHANISM CORRECTION — D2 is decided by GLOBAL alloc, not local alloc
Session 2's frontier item F1 attributed the `$v0`/`$v1` mirror to
`local-alloc.c` `combine_regs` tying a SET's dest to a dying source. **That is
wrong.** The QTYDBG table for the two if-arm blocks is **byte-identical**
between the P7 form and the tie-form (variant vC below) — local-alloc's
quantities and their `got=` hard regs do not change at all. The whole `$v0`
vs `$v1` outcome moves in the **ALLOCDBG (global.c) table**:

| form | pseudo 83 = `val` | pseudo 78 = `hi` | result |
|---|---|---|---|
| P7 (score 20) | ord=1 nrefs=19 livelen=15 **pri 50666 → $v1** | ord=7 nrefs=8 livelen=18 **pri 13333 → $v0** | mirror of target |
| vC tie (score 22) | ord=2 nrefs=11 livelen=11 pri 30000 → $v1 | ord=1 nrefs=16 livelen=20 pri 32000 → $v0 | still mirrored |

`val` carries a huge `nrefs` (19) because the SAME local is reused three ways:
the `0x20 - bits_left` shift amount, the `0x20 - needed` D1 intermediate, and
the third loop's `-2` holder. That ref count is what makes `val` outrank `hi`
in `allocno_compare` and take `$v1` first, leaving `$v0` for `hi` — the exact
mirror of target, where `hi` holds `$v1` and `val`/`cur >> bits_left` share
`$v0`. **The lever is the two pseudos' global priorities relative to each
other, not any local-alloc tie.**

### Measured probes this session (all `sandbox --disable all`, all 77 insns
unless noted; baseline P7 = 20)
- **vC — reuse `hi` itself as the shift-amount carrier**
  (`hi = 0x20 - bits_left; hi = cur >> hi;`). **22.** The tie DOES happen —
  the disassembly becomes `subu v0,t2,a3 ; srlv v0,a2,v0`, one register for
  both, exactly target's *shape* — but the merged quantity lands in `$v0`
  instead of `$v1`, and we lose the `subu $v1,$t2,$a3` line that P7 already
  matched. Net +2.
- **vB — same tie but through the otherwise-dead third-loop walker `out`.**
  **22.** Identical outcome to vC; the carrier's identity is irrelevant.
- **vA / vA2 — a THIRD named local (`amt`) holding only the shift amount,
  declared last / declared first.** **31 / 31.** Identical to session 2's P8
  (anonymous temp, 31). Declaration order changes nothing. This kills
  session 2's frontier probe F1(a) outright: a short-lived dedicated local is
  NOT the shape that fixes D2; it is the shape that costs 11 points.
- **vE — `val` additionally carries `cur >> bits_left`.** **20**, no change.
- **vF — vC + vE (tie `hi`, and `val` carries both other temps).** **28.**
- **vG — refill (`cur = *arg1; arg1++;`) moved below `needed = w - bits_left`.**
  **24.**
- **vG2 — refill moved all the way down to just above the store.** **26.**
- **vH — vC + vG.** **26.**
- **vI — `needed = w - bits_left;` hoisted to the TOP of the arm** (session 1's
  probe E, re-run at the settled P7 baseline as the ledger asked). **45.**
  Re-killed, and much worse than at the old baseline.
- **vK — `val` carries `hi << needed` instead.** **26**, build_insns **75**
  (the D1 copies are lost).
- **vL — third loop gets its own `neg2` holder instead of reusing `val`**
  (the direct attack on `val`'s nrefs=19). **22**, build_insns **75**. Lowering
  `val`'s ref count does move the allocation, but it also destroys D1: with the
  `-2` use removed, `val`'s remaining refs no longer force the
  `subu $v0,$t2,$a0` + `addu $a3,$v0,$zero` pair, so two instructions vanish.
  **This is the coupling that makes D2 hard: the same `val` ref count that
  buys D1's two copies is what makes `val` outrank `hi` for `$v1`.**

### Policy finding (blocks one previously-listed axis)
`.claude/rules/or-tree-shape-shift.md` is unambiguous: reordering operands of
an associative+commutative `|` **is FORBIDDEN** (status 2026-06-06, two
confirmed rejected instances). Session 1's frontier probe F3(b) ("change which
operand of the `|` is written first") is therefore **dead by policy** and must
never be measured. Recorded here so no future session re-opens it.

### Byte-neutrality note for the closing form's self-vet (useful, verified)
Target's loop bottom is a single `addiu $t1,$t1,0x2` in the `bnez` delay slot
(asm/funcs/func_8001979C.s line 45 / 67), i.e. the duplicated `dst += 2` we
write in both if-arms is re-merged by cross-jump into ONE increment in the
emitted code, and our build emits exactly that single increment too
(build_insns 77 == target_insns 77 with the duplication in source). The
duplication is therefore demonstrably **byte-neutral**, which is the
prerequisite the `duplicated-statement-into-arms` family asks for.

### Artifacts
`tmp/grind/func_8001979C/s3/` — `apply.py`, `dis.sh`, `cmp.py`, `qty.sh`,
`base_p7.c`, all variant bodies (`vA`…`vL`), `vC.txt` (disassembly),
`qty_p7.log`, `qty_vC.log`.

- [s3] [s3] HOUSEKEEPING: src/code6cac.c was back at the session-1 HEAD form (score 24) at the start of this session - the session-2 src edit did not survive into the driver's commit. Re-applying memory/grind/func_8001979C/candidate.c reproduced score 20 / build_insns 77 exactly. Every future session must re-apply candidate.c and re-measure before probing.

- [s3] [s3] The score-20 residual is exactly 20 differing instructions and decomposes cleanly: D2 = 14 (seven lines per bit loop - srlv, subu, addu a3, sllv, srlv, or, sh - a perfect $v0/$v1 mirror) and D4 = 6 (three lines per bit loop of preheader ordering). Nothing else in the function differs.

- [s3] [s3] D4's exact target order is 'addu t0,zero,zero ; addiu t4,zero,<width> ; addiu t2,zero,0x20 ; addu t1,t3,zero' (walker init LAST, width constant BEFORE 0x20); ours is 'addu t1,t3,zero' first (hoisted above the sw/lw preamble in loop 1) then t0, then t2, then t4.

- [s3] [s3] MECHANISM CORRECTION to session 2's frontier item F1: the D2 mirror is NOT a local-alloc combine_regs effect. The BB2_QTY_DEBUG local-alloc quantity table for the two if-arm blocks is byte-identical between the P7 form (score 20) and the vC tie form (score 22), which have DIFFERENT register assignments. The decision is made in global.c allocno_compare.

- [s3] [s3] Measured global-alloc numbers for the P7 form: val = pseudo 83, ord 1, nrefs 19, livelen 15, pri 50666 -> $v1; hi = pseudo 78, ord 7, nrefs 8, livelen 18, pri 13333 -> $v0. Target requires the reverse assignment, i.e. hi must be allocated before val.

- [s3] [s3] val's nrefs of 19 comes from three distinct reuses of the single local: the 0x20 - bits_left shift amount, the 0x20 - needed D1 intermediate, and the third loop's -2 holder. Splitting off the -2 (vL) drops the score to 22 AND drops build_insns to 75 because the D1 copy pair stops materialising - D1 and D2 are coupled through this one ref count.

- [s3] [s3] Tying the shift amount into hi (hi = 0x20 - bits_left; hi = cur >> hi;) reproduces target's SHAPE - a single register for the subu and the srlv - but lands the merged quantity in $v0 (score 22). The same tie routed through the dead 'out' walker gives an identical 22, so the carrier's identity does not matter.

- [s3] [s3] A third named local dedicated to the shift amount scores 31 whether declared first or last, identical to session 2's anonymous-temp P8. Declaration order is codegen-neutral for this variable, and the dedicated-short-life idea is dead.

- [s3] [s3] Session 1's probe E (hoist 'needed = width - bits_left' to the top of the arm), re-run at the settled P7 baseline as the session-2 ledger asked, scores 45. D4 is not a source-order effect at any baseline tested.

- [s3] [s3] BYTE-NEUTRALITY of the duplicated 'dst += 2' is verified: target emits a single 'addiu $t1,$t1,0x2' per loop in the bnez delay slot (asm/funcs/func_8001979C.s:45 and :67) because cross-jump re-merges the arm tails, and our build with the duplication in source emits exactly one too (77 == 77). This discharges one of the three prerequisites of the duplicated-statement-into-arms family for the eventual self-vet.

- [s3] [s3] POLICY: .claude/rules/or-tree-shape-shift.md forbids reordering the operands of the (hi << needed) | (cur >> bits_left) expression (cheat-by-spelling, FORBIDDEN 2026-06-06). Session 1's frontier probe F3(b) is closed permanently and was deliberately not measured.

- [s3] [s3] Reusable tooling added under tmp/grind/func_8001979C/s3/: apply.py (LF-safe whole-function splice into src/code6cac.c), dis.sh (objdump of the sandbox object), cmp.py (normalised side-by-side vs asm/funcs/func_8001979C.s - canonicalises li/move/nop, decimalises immediates, masks branch targets and %hi/%lo, prints only real diffs), qty.sh (instrumented cc1 with BB2_QTY_DEBUG + BB2_ALLOC_DEBUG). QTYDBG lines carry no function name: take the run immediately preceding the first 'ALLOCDBG func=func_8001979C' line.

- [s3] [s3] src/code6cac.c is left carrying the score-20 P7 form, re-verified this session at score 20 / target_insns 77 / build_insns 77. candidate.c is unchanged in body and carries a new session-3 header explaining the corrected mechanism. Five new rejected forms are banked under memory/grind/func_8001979C/rejected/.
