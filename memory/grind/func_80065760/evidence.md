# Evidence bank — func_80065760

- Audit diagnosis (regressions.md): Flagged construct: `(s16)v1` in `if ((s16)v1 < 0x11C8)` fails test #1 — `v1` is already declared `s16`; the cast is a C-language no-op and the function's observable behavior is identical without it. May also drive the `sll $v1,$v1,16 / sra $v1,$v1,16` sign-extension pair in the assembly (test #3 concern). Next action: run `sandbox func_80065760 --disable all` twice — once with the current source and once with the cast removed — and diff the scored asm. If assembly is byte-identical, the cast is pure documentation clutter (false flag, mark reaudit-clean). If different, the cast is a codegen-coercion construct without semantic purpose; rework the C to reach the match without it (the `s16 v1` declared type mandates signed comparison under integer promotion, so removing the cast and restructuring the comparison form is the clean path).  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Construct located: src/text1b.c:14798 `if ((s16)v1 < 0x11C8)` with v1:s16 (global D_800F0BC8, +=0x1C6 - same constants as func_80065344). Member of the 10-function cluster; D_800F0BC8 is on the 65344 diagnosis's retype list; sibling func_800657B0 (14810, D_800F0BCA) has the identical unflagged construct.

- [s1] [fable-blitz 2026-07-07] Target bytes (asm/funcs/func_80065760.s:8-14): lhu / addiu +0x1C6 / sh / sll 16 / sra 16 / slti 0x11C8.

- [s1] [fable-blitz 2026-07-07] The diagnosis's test-3 concern ('cast may drive the sll/sra pair') is answered statically: the sll/sra is extendhisi2 sign-extension of the POST-ADD register value, which the DECLARED s16 type of v1 mandates for the signed compare under integer promotion - with or without the cast. The cast cannot be load-bearing for it; and lhu is movhi_internal2's hardwired unsignedp=TRUE (mips.md:3364 -> mips.c:1026), not a cast effect.

- [s1] [fable-blitz 2026-07-07] Cast inertness (UNMEASURED, high confidence): c-convert.c convert() short-circuits same-type conversions, so the cast-dropped form should be byte-identical - the diagnosis's 'false flag, mark reaudit-clean' branch.

- [s1] [fable-blitz 2026-07-07] D_800F0BC8 blast radius: zero-writer is matched func_80065264 (src/text1b.c:14604, sh $zero, type-agnostic); the alternative u16-retype final form (matching accepted func_80065680/func_800656EC style) is available if the cluster standardizes on it.
