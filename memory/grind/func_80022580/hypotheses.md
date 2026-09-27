# Hypothesis ledger — func_80022580

- CONFIRMED s2: full first draft from the s1 region map reaches a small residual (67), all source-level ordering/cse effects; closed to 0 in one pass (see evidence s2).
- KILLED s2: `s16 c = p->unk_0A` local for the 5-way character test (combine forms lhu; target keeps lh + move + andi).
- KILLED s2: `D_80094C68[cond ? D_800A38DE : D_8008D578[...]]` ternary index (hoists &D_80094C68 into a0).
- KILLED s2: switch with case 0 falling through / goto into default (16), case2/3-first ordering (21).
