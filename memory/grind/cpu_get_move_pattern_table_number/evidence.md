# Evidence bank — cpu_get_move_pattern_table_number

- Audit diagnosis (regressions.md): Line 1898: `if ((double)(D_800A3817 == 2))` — the `(double)` cast is a semantically-inert codegen trick (optimizer folds it away: target asm shows plain `bne $v1,$v0` with no float instructions). Fails tests 1/2/3/5. Worker must redo the last `if` arm without the cast.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)
