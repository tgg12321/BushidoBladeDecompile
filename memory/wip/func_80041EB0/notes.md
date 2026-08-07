# decBs0 — WIP (current state 2026-08-05)

`src/text1a.c:1252`. HEAD baseline: honest distance **58**, 39 regfix rules, plus a
`register s16 *fp_ptr asm("fp")` pin.

**STATUS: SOLVED end-to-end in a snapshot — full build SHA1 == oracle
(`62efab4f73f992798c43e8c730aa43baa10bb4fa`).** Owner APPROVED the `text1a` `-G8` adoption
2026-08-05. Snapshot `~/bb2_g8_exp/snap`, scripts `tmp/csz/g8_*.sh`; verified == oracle before
any change. **NOT ported to main — the owner runs the port + gates.**

Verified decBs0 end state: **0 regfix rules, 0 asmfix rules, 0 register-asm pins, 0 `__asm__`
in the body, sandbox score 0, 134/134 insns.** COMPLETED-C. `regfix.txt` 3652 → 3613 (−39).

## The migration recipe (all five steps required)

1. **`MASPSX_FLAGS_GP` parity** (`Makefile:22`). It was a reduced set, missing `--sdata-funcs`,
   `--sdata-exclude`, `--expand-lb`, `--expand-lb-funcs`, `--multu-funcs`,
   `--expand-dest-funcs`, `--label-nop-funcs`. Parity alone collapses the blast radius from a
   diffuse −24 bytes to exactly the 2 predicted siblings. (`-G8` is needed only on **cc1**;
   `--dont-force-G0` and a `-G8` on the maspsx/`as` side are both unnecessary.)
2. `GP_FILES := text1a`.
3. Apply `candidate_g8_array.diff`; delete decBs0's 39 `regfix.txt` rules.
4. **Re-match the two perturbed siblings** — see the `-G8` hazard below.
5. **Extract `save_vc_ctrl` from its file-scope `__asm__` block** — see the deferral finding.

## The two general -G8 findings (these generalise to any future adoption)

**(a) ≤8-byte externs lose their base CSE.** At `-G8` cc1 treats any extern of ≤8 bytes as
small-data addressable and stops CSE-ing its address into a register. But `D_800F6318`,
`D_800F6498` (`s16` scalars) and `D_800F66A0` (a 4-byte fn-ptr typedef) sit ~340 KB past `$gp`,
far outside GPREL16 range, so maspsx cannot rewrite them and we pay a `lui` per use. All three
are declared as scalars yet used as **table bases** (`&X` assigned to a walking pointer), so
the fix is the honest array declaration (`extern s16 D_800F6318[];`) with `&X` → `X` at the use
sites. **Screening rule for the census: a file is `-G8`-safe only if every ≤8-byte extern is
either genuinely within gp range (in `sdata_syms.txt`) or declared with a non-small type.**

**(b) `-G8` defers function bodies, so file-scope `__asm__` floats to the top.** Measured on the
raw cc1 stream: at `-G0` `glabel save_vc_ctrl` sits at line 1978 with 16 `.ent` before it
(source position); at `-G8` it is at line **89 with ZERO `.ent` before it** — cc1 emits every
`.globl`/`.size` first and defers all function bodies (first `.ent` at line 244), while
varasm's `assemble_asm` writes top-level asm immediately. The block therefore always precedes
every function and lands at `.text` offset 0, shifting the first 16 functions by 0x40 and
cascading through the image (1255 differing words).

Ruled out as fixes: an explicit `".text\n"` directive in the block; `--dont-force-G0`; the
maspsx/`as` side entirely; and **source repositioning cannot work** — the block precedes all
deferred bodies wherever it sits. The fix is to make it an ordinary (deferred) function body:
`asm/funcs/save_vc_ctrl.s` (glabel form, keeping both TAB- and SPACE-form `.set` directives per
[[maspsx-noreorder-stripping]]) + a C stub at the same source position + an `asmfix.txt`
`replace_with_asmfile` entry (140 existing precedents). An empty stub emits no `.L` labels, so
it does **not** re-trigger the label-counter shift `asmfix.txt:22-23` warns about.

## Why the aggregate is the fix (mechanism, proven)

`sched_solver` (exact for text1a, 528/528 blocks) localises the residual to **block 19**.
Applying dependence edges by hand (`tmp/csz/bs0_edges.py`): **each colour load must depend on
EVERY preceding `tbl[]` store — 12 edges — to reproduce target's pass-2 order exactly**; the
one-edge-per-pair version does not. A 12-edge change is why no depth-1 atom search found it
(spellable atoms and the full 2163/1906-atom sets, both passes, no vector).

`sched.c:true_dependence` (821-838) drops the edge via exclusion #1, which needs
`!MEM_IN_STRUCT_P(load) && !rtx_addr_varies_p(load)`. An **in-struct load at a FIXED address**
defeats it without triggering exclusion #2. Per `expr.c:4589-4700` a **constant-index**
`ARRAY_REF` reaches the handler at `expr.c:4888`, which sets `MEM_IN_STRUCT_P` on a
`plus_constant(symbol, off)` — exactly that shape. Use-site evidence for the aggregate:
`func_80041E10` writes all three as R/G/B from one packed colour, `func_800420D0` sets
`[0] = -1` as a sentinel, decBs0 copies them into a 3-halfword-stride matrix column.

`-G8` is required because at `-G0` cc1's address-cost model (3 refs × 2 insns beats `la` + 3 ×
1 insn) hoists `la $23,g_anim_select` into the prologue, burning `$s7` and displacing fp_ptr
(16, 138 insns). A struct spelling measures identically (16/138), so spelling is not the lever.

The gp addend split (`0(gp)`+`D_800A323A` vs `2(gp)`+`g_anim_select`) is **confirmed benign** —
SHA1 == oracle proves the two forms link to identical bytes.

Standing constraint: the **store side stays closed** (stride-3 pointer walk layer-2 REJECTED,
commit a6a83d99). Cleanup on landing: stray comment at `regfix.txt:927`.

## Port notes for the owner

- `asmfix.txt` gains 1 line (the `save_vc_ctrl` wiring). **`no_new_regfix_guard` will fire** on
  the net addition — it is canonical-asm wiring, not a cheat, so it needs the
  `[infra-rule: <category>]` escape.
- `func_80040D48` still sandbox-scores 50 — that is its **pre-existing** INCOMPLETE state
  (34 rules of its own), unchanged by this work; it is byte-correct in the build.
- `save_vc_ctrl` stays canonical-authorized (`inline_asm_canonical.txt:329`); only its
  representation changed, not its status.

## Next

1. Owner: port + gates (fresh layer-2 on decBs0, `queue done`, commits).
2. The tree-wide per-file `-G` census is **NOT yet run**. Instrument committed:
   `tools/objdiff.py` — normalized per-function object comparison cancelling objdump's
   `jal 0 <name>` symbol artifact and branch-target shifts (without it: 23 false "changed"
   functions vs the true 4). Screen candidates with finding (a) above before building.
