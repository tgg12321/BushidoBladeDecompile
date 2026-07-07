# Evidence bank — GetAllocPacketSize

- Audit diagnosis (regressions.md): CODEGEN-CAST CHEAT at text1b.c:12269: func_80087F64((s16)v) forces a sign-extension pair (sll/sra in the jal delay slot) with no semantic purpose. Visible proto at line 12013 is void func_80087F64(s32); v is always in [1,46] so (s16)v==v. Three other call sites with the same s32 proto and runtime variable args (lines 12523, 12643, 12662) omit the cast. Worker redo: declare func_80087F64 as s16 consistently so GCC emits sign-extension naturally via integer promotion, then drop the explicit cast; confirm with retire/verify-oracle.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct: `func_80087F64((s16)v)` at src/text1b.c:12269 under `void func_80087F64(s32);` proto at src/text1b.c:12013 — the cast is semantically redundant (v = a0*3+1; judge notes v in [1,46]) and exists only to emit the truncation pair

- [s1] [fable-blitz 2026-07-07] Target bytes REQUIRE the sign-extension: asm/funcs/GetAllocPacketSize.s:12-14 emit `sll $a0,$s1,16` then `sra $a0,$a0,16` in the jal delay slot — some C spelling must produce this pair; the question is only which spelling is the honest one

- [s1] [fable-blitz 2026-07-07] The clean spelling already exists in this TU: src/text1b.c:12300 declares `void func_80087F64(s16);` (used by the call at 12316, which carries its own now-redundant cast); src/text1b.c:12580+12592 show the K&R alternative (`extern s32 func_80087F64();` no-proto + `(s16)arg1` cast, where the cast IS semantically load-bearing under default promotions)

- [s1] [fable-blitz 2026-07-07] Mechanism: GCC 2.7.2 prototyped narrow-arg conversion (expr.c/calls.c) converts an s32 argument to an s16 param via sign_extend(subreg:HI) — the SAME RTL the explicit (s16) cast produces under the s32 proto, so swapping to the s16 proto + bare `func_80087F64(v)` is expected byte-identical

- [s1] [fable-blitz 2026-07-07] Blast-radius check: the 12013 proto covers ONLY the 12269 call site (re-declared `s32` at 12278 before the next caller); the constant-arg callers (12281/12284/12292: args 2,5,1) use later decls and are unaffected; constants are compile-time-converted anyway (no truncation code emitted), so even a wider proto flip is low-risk

- [s1] [fable-blitz 2026-07-07] Policy: redundant width casts are explicitly NOT sanctioned (inline-asm-policy 2026-07-01: 'redundant width casts (F2) ... close by ordinary cleanup') — this regression is the ordinary-cleanup case, no ruling needed; NB the TU already tolerates coexisting s32/s16 decls for this symbol (12013 vs 12300 both file-scope), so the conflicting-declaration risk is empirically absent
