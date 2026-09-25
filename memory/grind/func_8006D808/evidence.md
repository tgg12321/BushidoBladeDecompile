# func_8006D808 evidence

- Canonical gate: C.
- Target size: 355 instructions.
- Fresh m2c reconstruction was converted from scalar stack aliases into the real
  0x2c-byte renderer descriptor passed to `func_8007352C`.
- Best honest, cheat-stripped sandbox result: 200/355 with 350 generated
  instructions. A signed-local experiment produced 202/355 with 363 generated
  instructions and is preserved in `candidate.c` because it better documents the
  intended signed decimal-digit arithmetic.
- The remaining diff is dominated by source-level differences in the nested
  three-row decimal-digit renderer: loop induction lifetimes, the paired `/ 10`
  and `% 10` lowering, and stack placement of the two `s16` digit temporaries.
- The current candidate is semantically faithful but still selects a 0x90 frame
  instead of the target 0x88 frame because `arg3` spills at the digit-local slot
  rather than remaining in `$fp`.
- No source body was changed; `src/text1b.c` remains `INCLUDE_ASM`.

## Reproduction

```powershell
& tools/wteng.ps1 main sandbox func_8006D808 --disable all --candidate memory/grind/func_8006D808/candidate.c --diff
```
