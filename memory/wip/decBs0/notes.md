# decBs0 — WIP (current state 2026-08-05)

`src/text1a.c:1252`. HEAD baseline: honest distance **58**, 39 regfix rules, plus a
`register s16 *fp_ptr asm("fp")` pin. Measure: `wsl bash tmp/csz/d.sh decBs0 text1a`.

**Status: SOLVED in a snapshot; blocked on one pipeline issue before it can land.** The owner
APPROVED the `text1a` `-G8` adoption 2026-08-05. Migration executed in `~/bb2_g8_exp/snap`
(scripts `tmp/csz/g8_*.sh`); the snapshot was verified == oracle before any change.

## The result

With `GP_FILES := text1a`, `MASPSX_FLAGS_GP` at parity, `candidate_g8_array.diff` applied and
decBs0's 39 rules deleted: **decBs0 sandbox score 0, 134/134 insns, ZERO rules, no pin** —
COMPLETED-C shape. Candidates banked: `candidate_11.diff` (11, at `-G0`) and
`candidate_g8_array.diff` (**0** at `-G8`; 16 at `-G0`).

## Why the aggregate is the fix (mechanism, proven)

`sched_solver` (exact for text1a, 528/528 blocks) localises the whole residual to **block 19**.
Applying dependence edges by hand (`tmp/csz/bs0_edges.py`) shows **each colour load must depend
on EVERY preceding `tbl[]` store — 12 edges — to reproduce target's pass-2 order exactly**; the
one-edge-per-pair version does not. A 12-edge change is why no depth-1 atom search found it
(searched: spellable atoms and the full 2163/1906-atom sets, both passes, no vector).

`sched.c:true_dependence` (821-838) drops the edge via exclusion #1, which needs
`!MEM_IN_STRUCT_P(load) && !rtx_addr_varies_p(load)`. An **in-struct load at a FIXED address**
defeats it without triggering exclusion #2 (which needs a *varying* in-struct load). Per
`expr.c:4589-4700` a **constant-index** `ARRAY_REF` reaches the shared handler at
`expr.c:4888`, which sets `MEM_IN_STRUCT_P` on a `plus_constant(symbol, off)` — exactly that
shape. Use-site evidence for the aggregate: `func_80041E10` writes all three as R/G/B from one
packed colour, `func_800420D0` sets `[0] = -1` as a sentinel, decBs0 copies them into a
3-halfword-stride matrix column.

`-G8` is required because at `-G0` cc1's **address-cost model** (3 refs × 2 insns beats `la` +
3 × 1 insn) hoists `la $23,g_anim_select` into the prologue, burning `$s7` and displacing
fp_ptr (16, 138 insns). A struct spelling measures identically (16/138), so spelling is not the
lever; and since target's loads are fixed-address in RTL they must be `MEM_IN_STRUCT_P` for
target's dependence to exist — i.e. the original TU really was built with small data enabled.

Standing constraint: the **store side stays closed** (the stride-3 pointer walk was layer-2
REJECTED, commit a6a83d99). Cleanup on landing: stray comment at `regfix.txt:927`.

## Migration recipe (snapshot-verified except the last step)

1. **`MASPSX_FLAGS_GP` parity is mandatory.** It was a reduced set, missing `--sdata-funcs`,
   `--sdata-exclude`, `--expand-lb`, `--expand-lb-funcs`, `--multu-funcs`,
   `--expand-dest-funcs`, `--label-nop-funcs`. Parity alone collapses the blast radius from a
   diffuse −24 bytes to exactly the 2 predicted siblings.
2. `GP_FILES := text1a`; apply `candidate_g8_array.diff`; delete decBs0's 39 `regfix.txt` rules.
3. **Both siblings re-matched**, via a finding that generalises to any `-G8` adoption: at `-G8`
   cc1 treats any **≤8-byte extern** as small-data addressable and stops CSE-ing its address
   into a register — but `D_800F6318`, `D_800F6498` (`s16` scalars) and `D_800F66A0` (a 4-byte
   fn-ptr typedef) sit ~340 KB past `$gp`, far outside GPREL16 range, so maspsx cannot rewrite
   them and we pay a `lui` per use. All three are declared as scalars yet used as **table
   bases** (`&X` assigned to a walking pointer), so the fix is the honest array declaration
   (`extern s16 D_800F6318[];`) with `&X` → `X` at the use sites.

## REMAINING BLOCKER — the file-scope glabel block relocates under -G8

Full build MISMATCHes (correct size, wrong SHA1). Everything is byte-identical except:
`src/text1a.c:883`'s file-scope `__asm__("glabel save_vc_ctrl")` (0x40 bytes) moves from its
source position (object offset **0x1330**) to the **front of `.text` (0x0)**, shifting the
first 16 functions by 0x40 and cascading through the image (1255 differing words).

Ruled out as the cause: an explicit `".text\n"` directive at the top of the block (no effect);
`--dont-force-G0` (no effect); the maspsx/`as` side entirely — with `MASPSX_FLAGS_GP` made
*identical* to `MASPSX_FLAGS` and `-G8` left **only on cc1**, the shift persists. cc1's own
`.ent` emission ORDER is unchanged between `-G0` and `-G8`.

Next hypotheses: inspect the raw `cc1 -G8` `.s` immediately around the glabel block for a
preceding `.sdata`/`.sbss`/`.rdata` section directive that changes the section state; or move
`save_vc_ctrl` out of the TU into `asm/funcs/` (the project already supports
`replace_with_asmfile`), sidestepping top-level-asm placement entirely.

NB decBs0 and `func_80041E10` also show a *benign* diff — the gp addend split
(`0(gp)`+`D_800A323A` vs `2(gp)`+`g_anim_select`). Same final address, expected to link
identically, but **not yet independently confirmed** since the build has not reached SHA1
parity.

## Next

1. Resolve the glabel relocation, then full-build SHA1 in the snapshot.
2. Then the tree-wide per-file `-G` census (**NOT yet run**). Instrument is built and committed:
   `tools/objdiff.py` — normalized per-function object comparison that cancels objdump's
   `jal 0 <name>` symbol artifact and branch-target shifts. Without it every comparison is
   noise (23 false "changed" functions vs the true 4).
3. **Do NOT port to main.** The owner runs the port + gates (fresh layer-2 on decBs0,
   `queue done`, commits).
