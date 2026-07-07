# Evidence bank — func_80065540

- Audit diagnosis (regressions.md): Codegen-coercion cast `(s16)v1` where v1:s16 — no semantic purpose, fails tests 1/2/3; verify whether dropping it changes the oracle match (if not, harmless noise; if yes, needs a clean pure-C form for the sll/sra sign-extension).  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Construct located: src/text1b.c:14720 `if ((s16)v1 < 0x100)` with v1:s16 (global D_800F0BB4, +=0x32). Member of the 10-function cluster analyzed in func_80065344's ledger session 1; D_800F0BB4 is on the diagnosis's ~12-global retype list.

- [s1] [fable-blitz 2026-07-07] Target bytes (asm/funcs/func_80065540.s:8-14): lhu / addiu +0x32 / sh / sll 16 / sra 16 / slti 0x100 - identical shape to the four sibling regressions.

- [s1] [fable-blitz 2026-07-07] Mechanism: lhu is NOT cast-driven - movhi_internal2 (mips.md:3364) calls mips_move_1word with unsignedp hardwired TRUE (mips.c:1026), so plain HImode loads are always lhu; the sll/sra pair is extendhisi2 on the post-add register (the compare can never fold to lh+slti because the register was mutated after the load). The cast is expected inert per c-convert.c convert() same-type short-circuit.

- [s1] [fable-blitz 2026-07-07] D_800F0BB4-specific retype blast radius: also written in matched func_8006505C (src/text1b.c:14494, `D_800F0BB4 = 0`, declared s16 at 14486) - constant zero store compiles to sh $zero regardless of signedness, so the retype should be byte-neutral there; both declaration sites (14486, 14713) must change together.

- [s1] [fable-blitz 2026-07-07] Accepted-style precedent for the retyped form: func_80065680 (src/text1b.c:14763) / func_800656EC (14779) - u16 globals + (s16) compare cast, already COMPLETED on main.
