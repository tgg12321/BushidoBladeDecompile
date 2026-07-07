# Evidence bank — camera_SetMatrix_8001DBE4

- Audit diagnosis (regressions.md): Empty if-body construct (line 1618, code6cac.c) fails tests 1+2 and matches the 'empty-body if(cond){} dead-read' forbidden family. Worker should rewrite to 'if (!(D_800A38F8 > D_800A37A0)) { ... }' (or '<=' form) and run verify-oracle to confirm equivalence — if oracle passes the form was benign style; if it fails it was genuinely codegen-dependent and requires deeper analysis.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] canonical verdict=C, distance=0, 51 total insns — pure-C target confirmed

- [s1] sandbox --disable all on HEAD: score=0 (185 cheat-asm insns stripped, but those are PAD_NOPS macros from other funcs in code6cac.c; this function itself has 0 rules)

- [s1] queue meta: 0 regfix/asmfix rules, verdict C — HEAD carries the debt entirely as C-source construct (empty-if), not toolchain rules

- [s1] Target asm shape (asm/funcs/camera_SetMatrix_8001DBE4.s): outer `bne D_800A3768,0x14` early-exit; func_8003AA78(); sltu on D_800A37A0<D_800A38F8; bnez skips both loops; loop1 = `do { AA48; gdlc; VSync(2); } while(sltu-cond)` with i=0 init scheduled INTO loop1's exit delay slot (`addu s0,$0,$0`); loop2 = `do { AA48; i++; gdlc; VSync(2); } while(i<15)`; then AAB0/InitDisplay/DisableDisplay tail

- [s1] Two delay-slot nops in target: (a) initial bnez-to-tail nop and (b) loop2 bnez nop — no schedulable material available at those points, so any rewrite preserving the branch skeleton should reproduce them

- [s1] Judge constraint from regressions.md: 'Worker should rewrite to if(!(D_800A38F8 > D_800A37A0)){...} (or <= form) and run verify-oracle to confirm equivalence — if oracle passes the form was benign style' — this is the primary hypothesis the ledger already flags

- [s1] No sibling: camera_SetMatrix_8001DA8C (immediately above) has a switch-body, no analogous vsync-wait shape; tmp/duplicates_leads.txt does not exist

- [s1] Function has s32 i local whose init `i = 0;` MUST remain positioned so cc1's first-pass scheduler places it in loop1 exit's delay slot (breaking that would push floor > 0 even if branch sense matches)
