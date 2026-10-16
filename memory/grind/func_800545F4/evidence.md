# Evidence bank — func_800545F4

## s1 (recon)

- Canonical gate verdict: ASM-WHOLE, citing "whole-function canonical-asm grant in
  inline_asm_canonical.txt" (`tools/wteng.ps1 main canonical func_800545F4`).
- `inline_asm_canonical.txt:112` — existing grant text: "Data-as-code fragment in text1b.c:
  4 lui/ori instructions encoding two GTE-opcode constants (0x4C808080, 0x48808080) into $t0,
  no jr $ra. Mid-body alabels (D_800545F8/FC/_80054600) referenced externally as data — engine
  reads the bytes as GTE opcode templates. Kengo's same-named function is unrelated
  (tslGlobalMemAlloc wrapper). classify_func: permanently_blocked:no_return_fragment. Same
  category as func_800790A4. User-authorized 2026-05-16."
- `engine/queue.json` entry for func_800545F4 carries `reopen_reason`: "owner 2026-10-08:
  re-audit of whole-body INCLUDE_ASM canonical entries. Decide afresh whether the original
  was hand-written asm (scan_hand_coded STRONG evidence + Judge) or compiled C that must be
  decompiled; many game entries were auto-added 2026-05-17 as handwritten_overflow_op." This
  is the actual task: re-validate the existing grant, not grind codegen.
- `asm/funcs/func_800545F4.s` body (9 lines total): `glabel func_800545F4` / lui $t0,hi(0x4C808080)
  / alabel D_800545F8 / ori $t0,$t0,lo(0x4C808080) / alabel D_800545FC / lui $t0,hi(0x48808080) /
  alabel D_80054600 / ori $t0,$t0,lo(0x48808080) / `endlabel func_800545F4`. 4 instructions,
  no `jr $ra`, $t0 is overwritten and never read before falling through to the next symbol.
- `python tools/scan_hand_coded.py --single func_800545F4 --json` → exit 1, "asm/funcs/func_800545F4.s
  has fewer than 5 instructions — not scanned". The automated STRONG/WEAK scorer the queue's
  reopen note asks for cannot render ANY verdict on this function — it is below the tool's
  floor. Any STRONG-tier claim for this entry has to come from a different, manual signal.
- OBJECT MODEL / cross-reference (decisive): the ONLY other place in the whole tree that
  references `func_800545F4` or its alabels is `asm/funcs/func_80041398.s:10-11`
  (`lui $v0,%hi(func_800545F4); lw $v0,%lo(func_800545F4)($v0)`) — a **load**, never `jal`/`jr`/
  a branch target. Grepped all of `asm/` and `src/` for `func_800545F4|D_800545F8|D_800545FC|
  D_80054600`: zero call/branch references exist anywhere.
  func_80041398 is already decompiled, at `src/main/31548.c:268-284`:
  ```c
  extern s32 *D_80015820[];
  extern s32 func_800545F4;
  extern s32 D_800545F8;
  extern s32 D_800545FC;
  extern s32 D_80054600;
  ...
  s32 t3 = (func_800545F4 & mask8) | a1;      // mask8  = ~0xFF
  s32 t2 = (D_800545F8   & mask16) | a0lo;    // mask16 = ~0xFFFF
  s32 v1 = (D_800545FC   & mask8) | a1;
  s32 a4 = (D_80054600   & mask16) | a0lo;
  ```
  i.e. the function reads these 4 words as plain `s32` data, masks out the low byte/halfword of
  each (which is exactly the immediate field of the lui/ori encoding) and ORs in a runtime value
  derived from its own argument, then stores the patched words through a table of function
  pointers (`D_80015820`). This is a self-modifying-code opcode-template patcher: the 4 words at
  func_800545F4 are never executed as code in place, only read as byte templates and the patched
  copies are the ones that eventually execute elsewhere. Confirms the grant's own description
  ("engine reads the bytes as GTE opcode templates") with a concrete, already-decompiled caller.
- `sandbox func_800545F4 --disable all` (baseline, no edits): `{"score":4,"scorable":true,
  "target_insns":4,"build_insns":0,"no_c_body":true}` — matches the driver's dispatch-time
  chassis check (4). `--diff` reports "no diff: function is still whole-body INCLUDE_ASM —
  there is no C to compare".

- [s1] canonical gate verdict: ASM-WHOLE, citing the whole-function canonical-asm grant in inline_asm_canonical.txt:112

- [s1] inline_asm_canonical.txt:112 existing rationale: 4 lui/ori instructions building two GTE-opcode constants into $t0, no jr $ra, mid-body alabels referenced externally as data, classify_func permanently_blocked:no_return_fragment, same category as func_800790A4, user-authorized 2026-05-16

- [s1] engine/queue.json reopen_reason (owner 2026-10-08) asks to re-audit whether this whole-body INCLUDE_ASM entry is genuinely hand-written asm vs compiled C that must be decompiled

- [s1] asm/funcs/func_800545F4.s is 4 instructions (lui/ori x2), no jr $ra, $t0 overwritten and never consumed before falling through

- [s1] tools/scan_hand_coded.py --single func_800545F4 refuses to score: 'fewer than 5 instructions — not scanned' -- the automated STRONG/WEAK tool the reopen note asks for cannot render a verdict here at all

- [s1] OBJECT MODEL: the only cross-reference anywhere in asm/ or src/ to func_800545F4 or its alabels D_800545F8/D_800545FC/D_80054600 is a load (asm/funcs/func_80041398.s:10-11, lui/lw), never a call or branch target -- confirmed by grepping the whole tree

- [s1] func_80041398 is already COMPLETED-C (src/main/31548.c:268-284) and treats func_800545F4 and its alabels as plain extern s32 data, masking out the low byte/halfword of each (matching the lui/ori immediate field) and ORing in a runtime-computed value before writing the patched word through a function-pointer table D_80015820 -- this is a self-modifying opcode-template patcher, confirming the grant's own 'engine reads the bytes as GTE opcode templates' description with a concrete decompiled caller

- [s1] sandbox func_800545F4 --disable all baseline (no edits): score 4, no_c_body true, target_insns 4, build_insns 0 -- matches the driver's dispatch-time chassis check; --diff reports no diff since there is still no C body to compare

## s2 (structural)

- Grepped the whole tree for `section("\.text"` / `section('.text'` style attribute usage
  (any construct that would let a plain C global land inside a `.text` run): zero hits anywhere
  in `src/`, `include/`, or `engine/`. No existing precedent mechanism exists in this repo for a
  C data symbol to be placed inside `.text`.
- Read `bb2.ld`'s SECTIONS block (`bb2.ld:16-834`) directly: the script groups output in strict
  per-category blocks across ALL TUs, not interleaved per-TU — all `*.o(.rodata)` blocks first
  (e.g. `bb2.ld:67-68` for this TU and its neighbor), then ALL `*.o(.text)` blocks
  (`bb2.ld:116-117`: `build/src/main/3AB48.o(.text); build/src/main/51268.o(.text);`), then ALL
  `*.o(.data)` blocks (`bb2.ld:343-344`), then `.sdata`/`.sbss`/`.bss`. This is the AGENTS.md
  `.rodata -> .text -> .data -> .bss` section order applied per-category across every TU, not
  per-TU.
- Consequence for H1: any ordinary C global (`u32`/`s32` etc. with no `section()` attribute,
  which this repo has zero precedent for anyway) compiled from `src/main/3AB48.c` is emitted by
  cc1/as into that TU's `.data` (or `.rodata` if const), which bb2.ld places at `3AB48.o(.data)`
  (`bb2.ld:343`) -- a byte range that is NOT adjacent to, and is linked in an entirely separate
  pass from, `3AB48.o(.text)` (`bb2.ld:116`). There is no mechanism by which a same-TU plain C
  global can land at the mid-`.text` address `0x800545F4` that the oracle's layout requires. This
  is not merely "no precedent found" -- the linker script's category-grouped structure makes it
  architecturally impossible for an ordinary (non-asm, non-attribute-section) C declaration in
  this TU to satisfy the placement H1 requires. H1's own next_probe ("if none exists, treat that
  absence as evidence against H1") is answered: absence confirmed, and the absence has a named
  structural cause, not just an empirical gap.
- No src edits made this session (nothing to measure via sandbox differently; chassis is still
  the s1 value, `no_c_body: true`, floor 4 -- confirmed unchanged, see state.json chassis check
  at dispatch).

## s3 (permuter)

- Mandated modality `permuter`, but this function has `no_c_body: true` -- there is no existing
  C chassis for `tools/permuter_annotate.py`/`permuter_campaign.py` to mutate (the permuter
  needs a compilable C candidate to iterate against the target asm; this entry is still
  whole-body INCLUDE_ASM with zero C). No campaign was launched; launching one against nothing
  would have produced no artifact and no measurement.
- Instead measured H1 directly: edited `src/main/3AB48.c:657` to spell out the s1/s2 candidate
  form (4 plain `u32` globals with the exact hex values), ran
  `sandbox func_800545F4 --disable all` -> `{"score": null, "scorable": false, "error":
  "...func_800545F4 not found in tmp/sandbox/func_800545F4/main/3AB48.o..."}`
  (`tmp/grind/func_800545F4/s3/sandbox_H1_plainC.log`).
- `nm tmp/sandbox/func_800545F4/main/3AB48.o` on that build: `func_800545F4`, `D_800545F8`,
  `D_800545FC`, `D_80054600` all type `D` at .data offsets `0x0`/`0x4`/`0x8`/`0xc` -- never in
  `.text`, confirming s2's linker-script read with an actual build artifact rather than only a
  `bb2.ld` text read.
- Reverted the edit; re-ran `sandbox func_800545F4 --disable all` -> `{"score": 4, "scorable":
  true, "no_c_body": true}` (`tmp/grind/func_800545F4/s3/sandbox_baseline_restored.log`),
  confirming the chassis is unchanged and the floor stands at 4.
- Rejected form saved to `memory/grind/func_800545F4/rejected/plain_c_data_global.c`.

- [s3] This function has no_c_body: true (whole-body INCLUDE_ASM, 4 instructions) -- the mandated permuter modality's tool (tools/permuter_annotate.py / permuter_campaign.py) requires an existing compilable C chassis to mutate, which does not exist here, so no permuter campaign was launched.

- [s3] Measured the s1/s2 H1 candidate form directly by editing the source and running sandbox, producing a concrete build artifact (nm symbol table) rather than only re-reading bb2.ld, which the previous session's discard reason (missing numeric measurement) required.

- [s3] sandbox func_800545F4 --disable all with the plain-C-global edit: score null, scorable false, error 'func_800545F4 not found in tmp/sandbox/func_800545F4/main/3AB48.o'.

- [s3] nm on that build's tmp/sandbox/func_800545F4/main/3AB48.o: func_800545F4 (D, 0x0), D_800545F8 (D, 0x4), D_800545FC (D, 0x8), D_80054600 (D, 0xc) -- all .data, confirming the class-scope predicate from s2.

- [s3] Reverted the edit; sandbox func_800545F4 --disable all baseline re-measured at score 4, no_c_body true, matching the dispatch-time chassis check.

- [s3] Rejected form banked at memory/grind/func_800545F4/rejected/plain_c_data_global.c; candidate.c written documenting the function has no improved C form and the body stays INCLUDE_ASM.
