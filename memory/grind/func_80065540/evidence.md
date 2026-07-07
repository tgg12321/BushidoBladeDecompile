# Evidence bank — func_80065540

- Audit diagnosis (regressions.md): Codegen-coercion cast `(s16)v1` where v1:s16 — no semantic purpose, fails tests 1/2/3; verify whether dropping it changes the oracle match (if not, harmless noise; if yes, needs a clean pure-C form for the sll/sra sign-extension).  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)
