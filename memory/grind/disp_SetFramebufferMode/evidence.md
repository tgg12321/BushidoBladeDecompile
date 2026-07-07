# Evidence bank — disp_SetFramebufferMode

- Audit diagnosis (regressions.md): Active volatile coercion via *(vu8 *) casts on non-allowlisted game RAM globals; removing casts raises sandbox score 0→6; the volatile_cheats detector never caught this *(vu8 *)(ptr + N) spelling. Worker must find the pure-C scheduling/structure solution that matches without volatile.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)
