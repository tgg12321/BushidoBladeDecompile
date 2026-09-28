# s7 sandbox measurements

Measured on main e6feac99c, project compiler and flags, 2026-09-28.
Every result used engine.sandbox.sandbox_score with disable="all",
strip_cheat_asm=True and candidate=<path>. Reproduce any banked candidate:

    & tools/wteng.ps1 main sandbox func_8006C21C --disable all --candidate memory/grind/func_8006C21C/probes/s7/<name>.c

results.json also records negative exploratory probes retained only in
tmp/codex_c21c. The initial x_s16/u16/u8 probes accidentally changed Env.x
as well as the local x; they are INVALID type experiments and excluded from
any conclusion. local_x_s16/u16 corrected this (local alone, score 53).
An initial cells_direct generation left one undeclared cells use, failed
compilation, and was corrected before the recorded score 57.
Headers inherited by scratch bodies were removed when banking probes.
All C files here are sandbox-only; the trailing prototype-masking macro
must never land in src.
