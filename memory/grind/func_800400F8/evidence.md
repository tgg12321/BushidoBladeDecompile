# Evidence bank — func_800400F8

- Audit diagnosis (regressions.md): Pre-comparison `s0 = 0;` + `if (s2[0] > s0)` is a blez-delay-slot scheduling coercion; clean fix: reorder `s0 = 0; s1 = s2;` (not `s1 = s2; s0 = 0;`) inside the if block and remove the pre-comparison assignment, which naturally puts `move s0, zero` in the delay slot without dead code.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)
