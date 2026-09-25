# func_8006D808 hypotheses

- Recover the original declaration order so the renderer descriptor occupies
  `sp+0x18..0x43`, digit halves occupy `sp+0x48/0x4a`, the loop-enabled word lands
  at `sp+0x58`, and `arg3` remains in `$fp`.
- Express the outer row and inner column loops with direct `s16` induction
  variables; the target increments after each `func_8007352C` call and sign-extends
  the new value for the loop test.
- Preserve the source's two stored `s16` copies of the selected byte before the
  signed division/remainder sequence. Combining them into scalar temporaries
  changes the GCC 2.7.2 lowering substantially.
- Use a word-sized selected-glyph variable for the `lbu`/range-adjust path, then
  cast to `s16` only at the table calculations. A byte local introduces masks not
  present in the target.
