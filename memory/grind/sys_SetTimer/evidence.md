# Evidence bank — sys_SetTimer

- Audit diagnosis (regressions.md): *(volatile s32 *)p write-cast is a volatile-coercion on non-volatile, non-allowlisted global g_sys_timer. Fix: change *(volatile s32 *)p = a0 to *p = a0 (pointer alone produces the lui+addiu match; volatile has no effect on MIPS sw and was never needed).  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)
