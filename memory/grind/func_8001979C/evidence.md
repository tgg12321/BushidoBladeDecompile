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

- [s4] HOUSEKEEPING (second occurrence, now a standing expectation): src/code6cac.c was again at the session-1 HEAD form (score 24) at session start; the session-2/3 src edit does not survive the driver's commit. Re-applying memory/grind/func_8001979C/candidate.c reproduced score 20 / 77 / 77 exactly, and every session must do this before probing.

- [s4] FLOOR 20 -> 18 (target_insns 77, build_insns 77). The new edit, per bit loop: the `0x20 - bits_left` shift amount is computed into the OTHER loop's walking pointer (loop 1 -> `dst2`, loop 2 -> `dst`, each provably dead at that point) and `val` reads it back (`val = dst2;`), and `needed` is routed through a second local `nd`. Found by the session-4 permuter campaign (output-305-1 on the baseline chassis) and then minimised by hand.

- [s4] The 18-point form's decomposition is CUMULATIVE, not a single fix: carrier chain in loop 1 alone = 20 (neutral), `nd` holder in loop 1 alone = 20 (neutral), both in loop 1 only = 19, both in both loops = 18. No half is load-bearing on its own, which is the signature of an allocation-pressure effect rather than a structural one.

- [s4] The 2 points bought are in D4, NOT in D2. ALLOCDBG for the 18-point form (tmp/grind/func_8001979C/s4/qty_vE3.log) is bit-for-bit unchanged for the D2 pair: val = pseudo 83, ord 1, nrefs 19, livelen 15, pri 50666 -> hardreg 3 ($v1); hi = pseudo 78, ord 7, nrefs 8, livelen 18, pri 13333 -> hardreg 2 ($v0). D2's 14 points are fully intact.

- [s4] IMPORTANT COROLLARY for the F1 attack: the carrier reuse does NOT move references off `val`. `val = dst2;` keeps all 19 of val's refs and instead adds refs to the walker's allocno. So "re-home val's third role into an already-dead existing local" (session 3's frontier F1(c)) is now MEASURED to be unable to change val's ref count by construction - any future ordering attack must add refs to `hi` or shorten `hi`'s live range.

- [s4] Residual at 18: D2 = 14 (the seven-line $v0/$v1 mirror per bit loop) and D4 = 4 (our `move $t1,$t3` is emitted before the `sw`/`lw` preamble instead of last, in both loops). The preheader `li` ORDER, which sessions 1-3 chased twice, is no longer a difference in this form - it fell out of the allocation change rather than out of statement order, confirming session 1's H-E verdict that D4 must be attacked through the allocation, never through source order.

- [s4] PERMUTER TOOLING (reusable; the workspace build was not previously documented for this function). tmp/grind/func_8001979C/s4/mk_workspace.sh builds a cheat-invisible decomp-permuter workspace: full-TU cpp of src/code6cac.c -> cc1 (-mel) -> prologue_fix -> maspsx -> multu_pad, then the func region is extracted with awk on `^\.ent[ \t]+func_8001979C$` .. `^\.end` (maspsx output has NO leading tab - the tab-prefixed pattern copied from tools/mar_perm_workspace.sh matches nothing and silently assembles the whole TU, including every INCLUDE_ASM .include). Two gotchas: (a) pycparser rejects base.c because src/code6cac.c names GameObj in two prototypes with no visible typedef (cc1 tolerates it), so the script prepends `typedef struct GameObj GameObj;` - verified codegen-neutral; (b) tools/mar_perm_workspace.sh's cc1 line is stale (build/cc1 without -mel).

- [s4] Campaign telemetry: two campaigns, ~13k iterations each at ~8 iters/s with -j 8 (full-TU compile is only ~0.3 s). Baseline chassis (perm base_score 410) best perm-score 305; vE3 chassis (base_score 370) best 280. Both harvested with --stop. Logs: tmp/grind/func_8001979C/s4/campaign_baseline.log, campaign_vE3.log.

- [s4] PERMUTER PROPOSAL HYGIENE for this function: a large fraction of the finds are unusable and must be filtered before measurement - semantics-breakers that clobber the loop counter (`i = bits_left < 2; if (i)`) or move `bits_left = 0x20;` between the loops; nonsense pointer casts (`*(s16 *)(float)(dst2 + 0x8E)`); and OR-operand swaps of `(hi << needed) | (cur >> bits_left)`, which are FORBIDDEN by .claude/rules/or-tree-shape-shift.md and must never be measured. The permuter re-prints through pycparser, so diff its output as a normalised statement list (tmp/grind/func_8001979C/s4/odiff.py), never as raw text.

- [s4] Engine-gradient measurements of the surviving proposals (all on top of the session-3 form unless noted): 0x20-constant-holder for all four subtractions 21 (build 76), for the shift amount only 31 (build 78), feeding the carrier line 31 (build 78); `base` staged through a second local 23 (build 78); `cur` staged before the hi read 20; carrier without the `val` read-back (`hi = cur >> dst2;`) 46; carrier = the third-loop walker `out` instead of the other bit loop's walker 20; if-condition hoisted into `needed` 22; condition holder + constant holder stacked 35. All banked under memory/grind/func_8001979C/rejected/.

- [s4] src/code6cac.c is left carrying the 18-point form, re-verified this session at score 18 / target_insns 77 / build_insns 77, and memory/grind/func_8001979C/candidate.c matches it byte for byte.

- [s4] Floor is 18 with the new form in place in src/code6cac.c, re-verified this session: score 18, target_insns 77, build_insns 77 (sandbox --disable all).

- [s4] The winning edit, per bit loop: the '0x20 - bits_left' shift amount is computed into the OTHER loop's walking pointer (loop 1 -> dst2, loop 2 -> dst; each provably dead at that point because dst2 = base; re-initialises it before loop 2 and dst is never read after loop 1) and val reads it back; 'needed' is routed through a second local 'nd'.

- [s4] The 18-point form is CUMULATIVE, not a single fix: carrier chain alone 20, nd holder alone 20, both in loop 1 only 19, both in both loops 18. No half is load-bearing on its own - the signature of an allocation-pressure effect.

- [s4] The 2 points bought are in D4, not D2. ALLOCDBG for the 18-point form is bit-for-bit unchanged for the D2 pair (val = pseudo 83, ord 1, nrefs 19, livelen 15, pri 50666 -> $v1; hi = pseudo 78, ord 7, nrefs 8, livelen 18, pri 13333 -> $v0).

- [s4] Residual at 18: D2 = 14 (the seven-line $v0/$v1 mirror per bit loop) and D4 = 4 (our 'move $t1,$t3' emitted before the sw/lw preamble instead of last, both loops). The preheader 'li' ORDER, chased twice in sessions 1 and 3, is no longer a difference in this form - it fell out of the allocation change, confirming D4 must be attacked through the allocation and never through source order.

- [s4] D2 arithmetic constraint derived this session: with floor_log2(8) = 3 and nrefs 8, hi cannot exceed val's pri of 50666 by any livelen change alone (it would need livelen < 1); hi MUST gain references and cross a floor_log2 bucket (nrefs 16 -> floor_log2 4 -> pri 64000/livelen). Every future D2 probe has to satisfy that.

- [s4] Permuter workspace recipe for this function is now reusable (tmp/grind/func_8001979C/s4/mk_workspace.sh): full-TU cpp -> cc1 -mel -> prologue_fix -> maspsx -> multu_pad, function region extracted with awk on '^\.ent[ \t]+func_8001979C$' .. '^\.end' (maspsx output has NO leading tab; the tab-prefixed pattern copied from tools/mar_perm_workspace.sh matches nothing and silently assembles the whole TU including every INCLUDE_ASM .include), plus a prepended 'typedef struct GameObj GameObj;' because pycparser rejects base.c where cc1 tolerates the missing typedef (verified codegen-neutral). tools/mar_perm_workspace.sh's own cc1 line is stale (build/cc1, no -mel).

- [s4] Campaign telemetry: two campaigns, 17962 and 14401 iterations, ~8 iters/s at -j 8 (full-TU compile ~0.3 s). Both harvested with --stop and confirmed dead (permuter_campaign status: alive false for both).

- [s4] Permuter proposal hygiene for this function: many finds are unusable and must be filtered before measurement - semantics-breakers that clobber the loop counter ('i = bits_left < 2; if (i)') or move 'bits_left = 0x20;' between the loops; nonsense pointer casts ('*(s16 *)(float)(dst2 + 0x8E)'); and OR-operand swaps of '(hi << needed) | (cur >> bits_left)', which are FORBIDDEN by .claude/rules/or-tree-shape-shift.md and were deliberately not measured.

- [s4] src/code6cac.c was again found at the session-1 HEAD form (score 24) at session start - the second session in a row - so re-applying candidate.c and re-measuring is a standing first step, not a one-off.

- [s4] POLICY STATUS UNCHANGED AND NOW LARGER: two constructs need family clearance before any candidate-ready - the duplicated 'dst += 2' (byte-neutrality already proven in session 3) and the NEW session-4 carrier reuse (writing a later-live walking pointer with an unrelated value purely for allocation pressure). Neither has been cleared; both need a verbatim scope quote plus a file:line/commit precedent, or a ruling-request.

## Session 5 (permuter, 2026-08-13) - floor 18 -> 12; D2 CLOSED

- [s5] HOUSEKEEPING (third occurrence, now fully expected): src/code6cac.c was again at the session-1 HEAD form at session start. Re-applying memory/grind/func_8001979C/candidate.c reproduced score 18 / 77 / 77 exactly before any probing.

- [s5] FLOOR 18 -> 16 -> 12 (target_insns 77, build_insns 77 throughout). Two edits, both in the refill arm of each bit loop. (A) The merged field value is routed through `hi` before the store - `hi = (hi << needed) | (cur >> bits_left); *(s16 *)(dst + 0xA) = (s16)hi;` - which is session 3's F1(a) probe, listed but never executed by sessions 3 or 4. Loop 1 alone is neutral (18); both loops give 16. (B) On top of (A), the `0x20 - bits_left` shift amount is computed INTO `hi` and immediately consumed (`hi = 0x20 - bits_left; hi = cur >> hi;`) - session 3's vC tie, which cost 2 points then and buys 4 now. Both loops: 12.

- [s5] MECHANISM, measured (tmp/grind/func_8001979C/s5/qty_vF1a.log, BB2_ALLOC_DEBUG): edit (A) does exactly what the session-4 arithmetic predicted. `hi` (pseudo 78) goes from nrefs 8 / livelen 18 / pri 13333 to nrefs 16 / livelen 20 / pri 32000, crossing the floor_log2 bucket from 3 to 4, and takes hardreg 3 ($v1) at ord 2; `val` (pseudo 83, nrefs 19, livelen 15, pri 50666) now takes hardreg 2 ($v0). That is target's orientation. The session-3/4 frontier item F1 is therefore CLOSED, and the closing move was the one probe both prior sessions listed and skipped.

- [s5] D2 IS CLOSED. With `hi` in $v1 the session-3 vC tie lands correctly: our arm now emits `subu $v1,$t2,$a3 ; srlv $v1,$a2,$v1 ; ... ; sllv $v1,$v1,$a0 ; srlv $v0,$a2,$a3 ; or $v1,$v1,$v0 ; sh $v1,0xA($t1)` - byte-identical to target through the whole shift/or/store chain in BOTH bit loops. The 14-point D2 family is gone.

- [s5] The session-4 CARRIER CONSTRUCT IS GONE from the best form. Edit (B) overwrites the carrier chain (`dst2 = 0x20 - bits_left; val = dst2;`), so the ledger's open policy question about writing a later-live walking pointer with an unrelated value purely for allocation pressure no longer applies to the candidate. The remaining policy questions are only the duplicated `dst += 2` and ordinary variable reuse (`hi` as its own shift-amount carrier, `val` as the D1 intermediate + the third loop's -2 holder).

- [s5] RESIDUAL AT 12 = three 4-point families, none of them inside the arm's shift/or chain. D4 (unchanged since s2): our preheader emits the walker init `move $t1,$t3` BEFORE the `sw`/`lw` preamble, target emits it LAST. D5 (NEW): inside each arm target emits `sllv $a2,$a2,$a0` (`cur <<= needed`) BEFORE the `or`; we emit the `or` first. D6 (NEW): the third (-2 fill) loop is now $v0/$v1 mirrored (target `li $v1,-2` + `addiu $v0,$t3,0x348`; ours the reverse) - it matched at floors 20 and 18 and flipped when `val` became the $v0 allocno.

- [s5] D5 is NOT a source-order effect: moving `cur <<= needed;` above the store (vTieShiftEarly) is exactly codegen-neutral, 12, with a bit-identical residual. It is a scheduling decision, so the next attack on it is sched.c/INSN_PRIORITY-shaped (tools/sched_solver), not statement order.

- [s5] D6 is coupled to D1 exactly the way session 3's vL was. Every re-home of the third loop's -2 was measured on the score-12 chassis: onto `hi` -> 12 but build_insns 75; onto `needed` -> 14, build 75; onto a fresh local -> 12, build 75; onto `nd` -> 31; copied through a fresh local while `val` still holds it -> 12 neutral (build 77). Every spelling that actually removes a `val` reference costs the D1 pair. Instruction parity (77) is treated as a hard invariant, so all of them are rejected.

- [s5] D4 re-tested at the new allocation and still dead: late walker inits (session 2's P6 shape) score 27 both-loops and 27 loop-1-only on the score-12 chassis. The walker/counter priority window is unchanged by the D2 fix.

- [s5] The `nd` holder from session 4 is still load-bearing at this floor: removing it (needed = W - bits_left; written directly where `needed = nd;` stood) costs 2 points (14).

- [s5] Two permuter campaigns this session, both harvested with --stop. (1) vF1a chassis (perm base_score 440), stopped early at 186 s / best find 415 when the hand probe found the better score-12 chassis. (2) vTie chassis (perm base_score 420), 31339 iterations over 1021 s, 32 finds, best perm score 290. Logs: tmp/grind/func_8001979C/s5/campaign_vF1a.log, campaign_vTie.log; normalised proposal diffs: tmp/grind/func_8001979C/s5/proposals_vTie.txt.

- [s5] Every surviving proposal from the vTie campaign was measured on the engine gradient and NONE beat 12: opaque holder for the field-width constant (the best perm find, 290) -> 18 both loops / 12 loop-2-only; opaque 0x20 holder -> already killed in s4; the -2 copied through a fresh local -> 12 neutral; splitting the OR into an accumulation (`hi = hi << needed; hi = hi | (cur >> bits_left);` - operand ORDER preserved, so not the forbidden or-tree reshape) -> 12 neutral; third-loop `out` initialised before `val = -2;` -> 12 neutral. The permuter's remaining finds are semantics-breakers (clobbering `i`, `i = bits_left < 2`), pycparser-artefact rewrites (`*(s16 *)(0x8E + dst2)`), or `do {} while (0)` wrappers.

- [s5] src/code6cac.c is left carrying the score-12 form, re-verified at the end of the session (score 12, target_insns 77, build_insns 77), and memory/grind/func_8001979C/candidate.c matches it byte for byte.

- [s5] FLOOR 18 -> 12 (sandbox --disable all, target_insns 77, build_insns 77). src/code6cac.c is left carrying the score-12 form and memory/grind/func_8001979C/candidate.c matches it byte for byte.

- [s5] D2 - the 14-point $v0/$v1 mirror that was the top frontier item from session 1 through session 4 - is CLOSED. Our arm now emits `subu $v1,$t2,$a3 ; srlv $v1,$a2,$v1 ; ... ; sllv $v1,$v1,$a0 ; srlv $v0,$a2,$a3 ; or $v1,$v1,$v0 ; sh $v1,0xA($t1)` in both bit loops, byte-identical to target.

- [s5] ALLOCDBG confirms the predicted mechanism exactly: `hi` (pseudo 78) nrefs 8 -> 16, livelen 18 -> 20, pri 13333 -> 32000, hardreg 2 -> 3; `val` (pseudo 83) unchanged at nrefs 19 / livelen 15 / pri 50666 and now hardreg 2. Session 4's derived condition (hi must cross the floor_log2 3->4 bucket) was correct and sufficient.

- [s5] The closing move was the single probe listed as frontier F1(a) at the end of BOTH session 3 and session 4 and skipped by both - a pipeline lesson worth more than the four points: execute the top listed probe before opening a new axis.

- [s5] The session-4 carrier construct (the 0x20 - bits_left shift amount written into the OTHER loop's live-later walking pointer and read back through `val`) is GONE from the candidate - the score-12 edit overwrites it. The ledger's open policy question about that construct no longer applies.

- [s5] Residual at 12 is three 4-point families, none of them in the arm's shift/or chain: D4 (walker init emitted before the sw/lw preamble instead of last, unchanged since session 2), D5 (NEW: target schedules `cur <<= needed` before the `or`), D6 (NEW: the third -2 fill loop is now $v0/$v1 mirrored because `val` became the $v0 allocno; it matched at floors 20 and 18).

- [s5] Instruction parity (build_insns 77) is treated as a hard invariant: three of the five D6 re-home spellings score 12 or better on paper but drop to build_insns 75 because they cost D1's `subu $v0,$t2,$a0` + `addu $a3,$v0,$zero` pair.

- [s5] The session-4 `nd` holder is still load-bearing at this floor: removing it costs 2 points (14).

- [s5] Permuter telemetry: two campaigns, both harvested with --stop and confirmed dead. (1) vF1a chassis, perm base_score 440, stopped at 186 s / best 415 once the hand probe produced a strictly better chassis. (2) vTie chassis, perm base_score 420, 31339 iterations / 1021 s / 32 finds / best perm score 290. Every surviving proposal was re-measured on the engine gradient and NONE beat 12.

- [s5] Permuter proposal hygiene for this function is unchanged from session 4 and was applied again: semantics-breakers that clobber `i` (`i = bits_left < 2; if (i)`), pycparser-artefact rewrites (`*(s16 *)(0x8E + dst2)`), `do {} while (0)` wrappers, and OR-operand swaps (FORBIDDEN by .claude/rules/or-tree-shape-shift.md, deliberately not measured).

- [s5] HOUSEKEEPING (third session running): src/code6cac.c was again at the session-1 HEAD form at session start; re-applying candidate.c reproduced 18 / 77 / 77 before any probing.

## Session 6 (forensics, 2026-08-13) - floor 12 -> 8; D1's REAL mechanism named; D6 CLOSED

- [s6] HOUSEKEEPING (fourth occurrence, standing expectation): src/code6cac.c was again at the session-1 HEAD form at session start. Re-applying memory/grind/func_8001979C/candidate.c reproduced score 12 / 77 / 77 exactly before any probing.

- [s6] FLOOR 12 -> 8 (sandbox --disable all, target_insns 77, build_insns 77). Two coupled edits, both OUTSIDE the bit loops: (A) the third loop's -2 gets its own local `neg2` instead of reusing `val`, which closes D6; (B) `val` is given a use after the third loop by carrying the final zero (`val = 0; *(s32 *)(base + 0x10C) = val;`), which restores D1. A alone is session 5's rejected form (12, build_insns 75); B alone is meaningless. Together: 8, build 77.

- [s6] D1's MECHANISM IS NAMED EXACTLY, and it is NOT the allocator. Every prior session attributed D1's `subu $v0,$t2,$a0` + `addu $a3,$v0,$zero` pair to `val`'s global-alloc reference count. The -da dumps show the copy insn is present identically in .rtl and .jump for BOTH the score-12 form and the separate-neg2 form, and DIVERGES in the .cse dump: with a separate neg2 the copy is already gone after cse (`(insn 80 ... (set (reg/v:SI 74) (minus:SI (reg:SI 97) (reg/v:SI 79))))`), while the score-12 form still carries `(insn 80 (set (reg/v:SI 83) (minus ...)))` plus `(insn 83 (set (reg/v:SI 74) (reg/v:SI 83)))`.

- [s6] The deciding code is cse.c:7454, "Special handling for (set REG0 REG1) where REG0 is the cheapest": it rewrites the pair so the PREVIOUS insn writes bits_left directly and the copy becomes a dead store, but only when `qty_first_reg[reg_qty[REGNO(val)]] == REGNO(bits_left)`. make_regs_eqv (cse.c:826-863) makes bits_left the canonical register of that quantity only if bits_left lives past the current basic block AND `uid_cuid[regno_last_uid[bits_left]] > uid_cuid[regno_last_uid[val]]` (cse.c:856). Therefore D1's copy survives EXACTLY WHEN `val` carries a reference LATER IN THE FUNCTION than bits_left's last reference. That is the whole rule, and it is a source-reachable, ref-count-free condition.

- [s6] This retro-explains every failed -2 re-home in sessions 3, 4 and 5 (vL, and the five s5 spellings): removing the third loop's -2 from `val` moved val's last reference back into loop 2's arm, i.e. earlier than bits_left's last reference, so cse.c:7454 fired and the D1 pair vanished (build_insns 75). Those sessions read the symptom (val's nrefs) rather than the cause (val's last-use position). The -2 was never load-bearing as a REFERENCE COUNT; only as a LATE reference.

- [s6] D6 IS CLOSED. With `neg2` separate, the third loop emits `addiu $v1,$zero,-2 ; addiu $t0,$zero,3 ; addiu $v0,$t3,0x348 ; sw $v1,0x110($v0)` - byte-identical to target. The .greg dump shows pseudo 82 (`out`) in hardreg 2 ($v0) and pseudo 85 (`neg2`) in hardreg 3 ($v1), and pseudo 83 (`val`) is no longer in the third loop's conflict set at all.

- [s6] The session-5 frontier's D6 plan (ADD references to `out` so it outranks `val` in allocno_compare) is KILLED as a mechanism, with the reason. global.c find_reg (tools/gcc-2.7.2/global.c:993-1084) runs a TWO-PASS scan in which pass 0 excludes every hard register not already in `regs_used_so_far` ("we never allocate a register for the first time in pass 0"). Allocating `out` FIRST therefore does not give it $v0 - it gives it whatever already-used non-conflicting register comes first, which is $v1. Measured: variant v2 lengthened val's livelen from 11 to 15 (pri 30000 -> 22000) and did flip the allocation ORDER so `out` was allocated before `val` (ord 3 vs 4) - and `out` still landed in $v1. Ordering is not the lever; the conflict graph is. Target's $v0-for-`out` comes from `out` NOT conflicting with the $v0 holder, which requires the -2 to be a different variable.

- [s6] The instrumented cc1 also exposes BB2_FINDREG_DEBUG=<pseudo> (global.c:1004-1049), which dumps that pseudo's conflicts / regs_someone_prefers / regs_used_so_far / pass-0 exclusion set / copy and full preferences / class+mode. Not needed once the pass-0 rule above was read, but it is the tool for any future find_reg question on this function.

- [s6] Measured probes this session (all sandbox --disable all, baseline 12): v1 split-init on `out` (`out = base; out += 0x348;`) 12 and the ALLOCDBG table is bit-for-bit unchanged - the split is folded before allocation, so it cannot add references. v2 `arg1++` moved between `val = 0x20 - needed;` and `bits_left = val;` 20 (livelen lever, see above). v3 separate `neg2` alone 12 / build 75. v4 v3 + `val` carrying `cur >> bits_left` 24 / build 75. v5 = v3 + the final-zero reuse 8 / build 77 (the new floor). v6 = v5 + late walker inits 23.

- [s6] D4 re-killed a THIRD time, now at the score-8 allocation: the late walker inits (session 2's P6 shape) score 23. Sessions 2, 5 and 6 have now measured it dead at floors 20, 12 and 8 respectively.

- [s6] Residual at 8 is exactly two 4-point families, confirmed line by line with cmp.py (tmp/grind/func_8001979C/s6/v5.txt): D4 - our `addu $t1,$t3,$zero` walker init is emitted FIRST in each preheader, before the `sw`/`lw` preamble, where target emits it LAST (after `addu $t0,$zero,$zero ; addiu $t4,zero,<w> ; addiu $t2,zero,0x20`); D5 - inside each refill arm target emits `sllv $a2,$a2,$a0` before the `or`, we emit the `or` first. Nothing else in the function differs.

- [s6] POLICY: the s6 edit adds ONE new construct to the pre-clearance list - routing the function's final zero through `val` (`val = 0; *(s32 *)(base + 0x10C) = val;`). The store and its value are real (not a dead store), so this is the "variable reuse for codegen control" family, but it is the construct in the current form most likely to draw a "why is this here?" from a reviewer and it has NOT been cleared with a scope quote + precedent.

- [s6] src/code6cac.c is left carrying the score-8 form, re-verified at the end of the session (score 8, target_insns 77, build_insns 77), and memory/grind/func_8001979C/candidate.c matches it byte for byte.

- [s6] Floor 12 -> 8 (sandbox --disable all, target_insns 77, build_insns 77), re-verified at end of session with the form in place in src/code6cac.c and memory/grind/func_8001979C/candidate.c matching byte for byte.

- [s6] The winning edit is two coupled changes outside the bit loops: (A) the third loop's -2 gets its own local `neg2` instead of reusing `val` (closes D6); (B) `val` is given a later last-use by carrying the function's final zero, `val = 0; *(s32 *)(base + 0x10C) = val;` (restores D1). A alone is session 5's rejected form (12, build_insns 75); B alone does nothing.

- [s6] D1's mechanism is cse.c, NOT the allocator: cse.c:7454's cheapest-register swap deletes the copy whenever make_regs_eqv (cse.c:826-863, test at cse.c:856) has made bits_left the canonical register of val's quantity, which happens iff bits_left's regno_last_uid is later than val's. This supersedes the sessions-3/4/5 account of D1 as a `val` reference-count effect.

- [s6] That rule retro-explains all six previously-rejected -2 re-homes (session 3's vL and the five session-5 spellings, all build_insns 75): each moved val's last reference back into loop 2's arm, i.e. earlier than bits_left's last reference, so cse.c:7454 fired and the copy pair vanished. The -2 was never load-bearing as a reference COUNT, only as a LATE reference.

- [s6] D6 is CLOSED: the third loop now emits `addiu $v1,$zero,-2 ; addiu $t0,$zero,3 ; addiu $v0,$t3,0x348 ; sw $v1,0x110($v0)`, byte-identical to target. The .greg dump shows pseudo 82 (`out`) in hardreg 2 ($v0) and pseudo 85 (`neg2`) in hardreg 3 ($v1).

- [s6] global.c find_reg (tools/gcc-2.7.2/global.c:993-1084) runs a two-pass hard-reg scan whose pass 0 excludes every register not already in regs_used_so_far - so being allocated FIRST does not win the lower register. Measured directly: v2 flipped the allocation order in `out`'s favour and `out` still took $v1.

- [s6] The instrumented cc1 also exposes BB2_FINDREG_DEBUG=<pseudo> (global.c:1004-1049), dumping that pseudo's conflicts, regs_someone_prefers, regs_used_so_far, the pass-0 exclusion set, its copy and full preferences, and class/mode. Documented for future find_reg questions on this function.

- [s6] Residual at 8 is exactly two 4-point families (cmp.py on tmp/grind/func_8001979C/s6/v5.txt): D4 - our `addu $t1,$t3,$zero` walker init is emitted FIRST in each preheader, ahead of the `sw`/`lw` preamble, where target emits it LAST; D5 - inside each refill arm target emits `sllv $a2,$a2,$a0` before the `or` and we emit the `or` first. Nothing else in the function differs.

- [s6] Session probe scores (baseline 12): v1 out split-init 12 (ALLOCDBG unchanged), v2 arg1++ between val's def and use 20, v3 separate neg2 alone 12 / build 75, v4 val carries cur>>bits_left 24 / build 75, v5 the new floor 8 / build 77, v6 late walker inits 23.

- [s6] POLICY: the session-6 edit adds one construct to the pre-clearance list - routing the final zero through `val`. The store and its value are real (not a dead store), so it is the 'variable reuse for codegen control' family, but it is uncleared and is the construct most likely to draw a 'why is this here?' from a reviewer. The other two uncleared constructs are unchanged: the duplicated `dst += 2` (byte-neutrality proven in session 3) and the `hi` / `val` reuse.
