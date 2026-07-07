# Evidence bank — hirahira_w_frie

- Audit diagnosis (regressions.md): Two constructs violate the SOTN bar; primary is a confirmed param-local-alias-prologue-pair-flip (explicitly forbidden). Worker must produce a clean pure-C body that matches the prologue without aliasing the parameters in reverse order.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)
