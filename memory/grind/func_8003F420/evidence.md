# Evidence bank — func_8003F420

- Audit diagnosis (regressions.md): Dead-write cheat: `int new_var;` is assigned in `s0 = (new_var = -1);` and never read again. The write to new_var is a pure dead store — semantically equivalent to `s0 = -1;`. This is a dead-write-through-intermediate-local for RA coercion, closely analogous to the forbidden dead-param-assign family (Lever D). Not covered by any SOTN sanction (the variable-reuse sanction requires the intermediate be read downstream; new_var is never read). Worker redo: replace `s0 = (new_var = -1);` with `s0 = -1;` and remove the `int new_var;` declaration. Divmod routing (`s0 = a1/2000; s2 = s0; s0 = a1-s2*2000`) is ALLOWED under SOTN variable-reuse-for-codegen-control and does not regress; only the new_var construct fails.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct located: src/config.c:140 `int new_var;` + :155 `s0 = (new_var = -1);` -- write-only, never read again (the other new_var* hits in config.c at :296+/:305-308 and :438/:442 belong to different functions, func_8003FE40-region and func_8003FECC).

- [s1] [fable-blitz 2026-07-07] Strongest static evidence of INERTNESS: the two if-else diamonds compile IDENTICALLY in target -- slti/beqz with else-value `addiu +1` in the delay slot and fall-through `addiu -1` (asm/funcs/func_8003F420.s:35-38 for s1, s:40-43 for s0) -- yet the s1 arm's C is the PLAIN `s1 = -1;` (config.c:150). Identical target shapes from asymmetric spellings implies the plain spelling suffices for the s0 arm too.

- [s1] [fable-blitz 2026-07-07] Mechanism: `s0 = (new_var = -1)` expands to two RTL SETs; the never-read SET is deleted by flow.c life analysis pre-RA, so only pre-flow passes could see it. The one documented effect of a two-set arm (breaking jump.c store-flag's single-set precondition, per [[dead-store-fake-exception]]) is moot here: BOTH target diamonds are branchy, not store-flag-converted.

- [s1] [fable-blitz 2026-07-07] The divmod routing the judge explicitly ALLOWED (`s0 = a1/2000; s2 = s0; s0 = a1 - s2*2000;` config.c:146-148) matches [[divmod-coalesce-reuse-var]] in target: mfhi->sra->subu into $s0 (s:25-27), `addu $s2, $s0` copy (s:28), remainder reuses $s0 (s:34). Do not disturb it during the cleanup.

- [s1] [fable-blitz 2026-07-07] Diagnosis-vs-rule note (informational, likely moot): the diagnosis states the dead-write-local is 'not covered by any SOTN sanction (variable-reuse requires downstream read)', but [[dead-store-fake-exception]] (2026-07-01) sanctions FAKE-annotated never-read dead stores to LOCALS as last-resort. Only relevant if the plain respelling unexpectedly fails to match.

- [s1] [fable-blitz 2026-07-07] Rest of the function is clean: no pins, no asm, no volatile; 4 tail calls to stage_SetCollision with s3/s2/s1/s0 arg staging matching s:45-62.
