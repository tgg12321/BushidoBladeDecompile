# Evidence bank — func_8003B3A4

- Audit diagnosis (regressions.md): Write-only pointer alias `u8 *p = &D_8010277D;` coerces GCC address pre-materialization before branch; not covered by pointer-rmw-global-sanctioned (requires load+store, this is store-only). Worker should rewrite the block as a direct conditional write (`if (a1 != 0) { D_8010277D = 0xE; } else { D_8010277D = 0x1D; }`) and find a clean pure-C structure that reaches the target's pre-branch address-in-register pattern without a write-only pointer alias.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct at src/code6cac_c_ab.c:466-473: block-scoped `u8 *p = &D_8010277D;` with `*p = 0xE;` / `*p = 0x1D;` in the two arms - store-only alias, no /* FAKE */ annotation.

- [s1] [fable-blitz 2026-07-07] LOAD-BEARING per target bytes: asm/funcs/func_8003B3A4.s:24-25 materializes the FULL address into $v1 (`lui $v1,%hi(D_8010277D)` + `addiu $v1,$v1,%lo`) BEFORE the beqz (line 26), and line 33 stores register-indirect `sb $v0, 0x0($v1)`. A direct global store (`D_8010277D = k;`) emits the cc1 macro form `sb $v0, D_8010277D` -> `lui $at` + `sb %lo(...)($at)` AFTER the value diamond - wrong placement and wrong base register class.

- [s1] [fable-blitz 2026-07-07] In-file control proving the alias is codegen-motivated, not house style: sibling func_8003B484 (src/code6cac_c_ab.c:490) and func_8003B4DC (line 500) write D_8010277D DIRECTLY and get the $at macro form; only this function needs the register-held address.

- [s1] [fable-blitz 2026-07-07] Value diamond maps cleanly: beqz $a1 skips to .L8003B410 (`addiu $v0,0x1D`, asm line 31); taken-path `j .L8003B414` with slot `addiu $v0,0xE` (asm lines 28-29); single shared sb at the join - consistent with the two-arm *p stores after cross-jump merges the identical sb (defeat-combine-symbol-fold family mechanism: the pointer variable keeps the address out of the symbolic-MEM fold).

- [s1] [fable-blitz 2026-07-07] Family analysis: judge cites pointer-rmw-global-sanctioned (requires load+store; correct that it does NOT cover store-only) but does not address pointer-alias-fake-exception (sanctioned 2026-07-01): 'C-level local pointer alias / typed re-view of a global, /* FAKE */-annotated' - SOTN tilemap = &g_Tilemap / fakeEntity = self precedents. The committed construct is that exact shape minus the annotation and lever-exhaustion record.

- [s1] [fable-blitz 2026-07-07] The byte cluster D_8010277C/277D/277E/277F/80102783 is accessed as loose u8 globals across the sibling group (src lines 461, 474, 480, 490-492, 499-501) - a struct-pointer re-model is conceivable but would leave the other fields' $at-form stores unexplained if the base pointer were reused.
