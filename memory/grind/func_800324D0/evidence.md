# Evidence bank — func_800324D0

## [s1] 2026-08-20 — recon → sandbox 0 (candidate-ready)

### Baselines (this chassis)
- `canonical func_800324D0` → verdict C, 68 target insns, distance 27.
- Pin-carrying m2c body: sandbox `--disable all` = 27, build_insns 68 == target 68.
- Pins deleted (plain locals, same structure): sandbox = 27, 68/68. The four
  `register asm()` pins were score-inert exactly as the brief predicted.

### What the 27 actually was
Full objdump diff of the sandbox .o vs target: the schedule, shape, branch
structure, `andi` at the loop head, and the single `sltiu ...,0xC` all MATCH.
The whole distance was one 3-cycle register rotation across ~27 instructions:

| pseudo (honest build) | role | ours | target |
|---|---|---|---|
| 73 | stream walker | $a2 | **$v1** |
| 75 | command (`cmd`) | $a1 | **$a2** |
| 76 | payload (`val`) | $v1 | **$a1** |
| 74 | stream byte (`c`) | $v0 | $v0 |
| 72 | param `pad` | $a0 | $a0 |
| 78 / 82 | 0xFF holder / jtbl base | $t0 / $a3 | $t0 / $a3 |

Consequently the brief's static hypotheses 2 (elided `andi`) and 3 (double
range check) were both DEAD ON MEASUREMENT: the honest build already emits the
`andi` (the u8→u32 promotion at `cmd = c` produces it) and already folds
`if (cmd < 12)` + `switch` into one `sltiu`. Frame stayed 0 (leaf, no saves).

### RA forensics (`.lreg`/`.greg` + instrumented cc1, tools/gcc-2.7.2/cc1)
- lreg: 73 walker 24 refs/62 insns; 75 cmd 14/10; 76 val 26/21 (dies 12×);
  74 byte 16/21 (dies 2×); 72 pad 37/62; 78: 3/92; 82: 3/90.
- greg allocation order (allocno_compare, pri ∝ floor_log2(refs)·refs/live):
  **76, 75, 74, 72, 73, 82, 78** — payload first, walker LAST. Ascending
  first-free then yields ours ($v1,$a1,$v0,$a0,$a2,$a3,$t0). Priority-order
  inversion is ARITHMETICALLY unreachable (walker density 1.55 vs payload
  4.95; would need ~61 weighted refs on the walker) — the whole
  "raise/lower priorities" lever class is dead for this function.
- BB2_FINDREG_DEBUG ground truth (logs in tmp/grind/func_800324D0/s1/):
  - MIPS has no LEAF_REGISTERS ⇒ `regs_used_so_far` is seeded with ALL
    call-used regs ⇒ find_reg **pass 0 is the effective pass** and
    `regs_someone_prefers` is decisive.
  - 76 val: conflicts {2,29}, someone_prefers {4} → takes 3.
  - 75 cmd: conflicts {2,3(held),29}, someone_prefers {4} → takes 5.
  - 73 walker: conflicts {2,3,4,5,29} → takes 6.
  - 74 byte: own_full_prefs {2} → takes 2 (its pref-2 arises from
    set_preference through the local-allocated scratch pseudos).
- **Single sufficient condition derived and confirmed:** if the WALKER allocno
  carries `hard_reg_full_preferences ∋ 3`, prune_preferences puts 3 into
  `regs_someone_prefers` of every higher-priority conflicting allocno
  (76, 75), they skip it in pass 0 (76→5, 75→6), and the walker takes 3.
  82→7, 78→8 fall out unchanged. That is EXACTLY the target assignment.

### The set_preference mechanism (global.c:1671)
- `set_preference` strips ONE operator level from SRC
  (`GET_RTX_FORMAT[0]=='e'` → `src = XEXP(src,0)`), so `(set walker
  (plus (reg X) 5))` and `(set d (mem (reg X)))` both generate preferences.
- It substitutes `reg_renumber` FIRST, so **local-allocated block pseudos act
  as hard registers** for preference planting. This is the honest, C-reachable
  route to a hard-reg preference in a leaf function with no calls and no
  arg/return hard regs beyond $a0.

### The closing spelling (sandbox 0, 68/68, measured twice)
Split the init into two block-0 locals with overlapping live ranges:
```c
base = *(u8 **)(pad + 0x58);   /* lw  — base: once-written, TWO real reads */
ff = 0xFF;                     /* li  — ff: read by 7 sb stores */
... 11 default stores via ff/0 ...
c = base[4];                   /* lbu v0,4(base) */
ptr = base + 5;                /* addiu — plants the walker's pref */
```
Chain: `ff` (higher local density) gets $2 from local-alloc; `base`, alive
from the first lw until `base+5` (past ff's last use), overlaps it and gets
$3. `base` survives to RA because it is NOT a copy (load) and has TWO uses
(combine cannot fold a multi-use def; cse only coalesces reg-reg copies).
`set_preference` on `(set ptr (plus (reg base→$3) 5))` gives the walker
full-pref $3 → the whole cascade above → target allocation, byte-identical.
Since base and ptr both land in $3, the emitted bytes are IDENTICAL to the
one-variable spelling (`lw $v1 / lbu $v0,4($v1) / addiu $v1,$v1,5`) — no
extra instruction, no moved instruction; 68/68 with score 0.

### Kills
- **q-alias-copy** (rejected/q-alias-copy.c): a bare pointer copy `q = v1` in
  the ≥0x80 arm. Sandbox flat 27; .greg allocno census unchanged — cse1
  coalesces reg-reg copies before RA. Generalizes: pref-planting intermediates
  need a NON-copy def and ≥2 uses.
- Priority-inversion lever class (see arithmetic above) — dead without
  structural pseudo changes.
- Brief levers 1 (byte-provenance/andi) and 2 (double range check) — moot on
  this chassis; the honest build already matches both features.

### Artifacts
- tmp/grind/func_800324D0/s1/findreg{72,73,74,75,76,78,82}.log — instrumented
  find_reg exclusion sets (BB2_FINDREG_DEBUG).
- tmp/grind/func_800324D0/s1/findreg.sh — the sweep script.
- tmp/grind/func_800324D0/s1/body.i — preprocessed TU used for the cc1 runs.
- tmp/grind/func_800324D0/dumps/ — .lreg/.greg et al. (pin-free chassis).
