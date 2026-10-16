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
