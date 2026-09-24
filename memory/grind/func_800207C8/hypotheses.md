# func_800207C8 hypotheses

- Start from the phase-local score-99 shape and reproduce m2c's working-pointer
  roles (`var_a0`, `var_a1 = var_a0 + 8`, `var_a3`, then `temp_a2`) without
  extending their lifetimes across phases.
- Search declaration order for the `$s5` player-data / `$s6` fourth-output swap;
  do not use explicit register variables.
- The final three-instruction excess comes from scalar copies of `arg1[0..2]`;
  test a truthful three-word record assignment or a small vector struct.
