# Evidence bank — func_8001FAE4

- Audit diagnosis (regressions.md): char new_var is a dead-constant codegen coercion (permuter-found trick, commit-confirmed). The variable is always 0, both comparisons are semantically identical to != 0, no asm instruction corresponds to it (fully DCE'd by GCC), and the name announces non-semantic intent. Fails all 6 checklist tests. New spelling of dead-variable coercion family not caught by volatile_cheats.py. Worker must remove char new_var and replace both uses with literal 0, then find the pure-C form that still produces score 0 without the trick (if score rises, that is the real distance to close).  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct located: `char new_var;` at src/code6cac.c:2084, `new_var = 0;` at src/code6cac.c:2095, uses at src/code6cac.c:2096 ((v1 & 0xC000) != new_var) and src/code6cac.c:2103 (v1 != new_var)

- [s1] [fable-blitz 2026-07-07] Zero-asm-footprint confirmed: both compares emit as plain zero-branches - andi $v0,$v1,0xC000 / beqz $v0 at asm/funcs/func_8001FAE4.s:10-11 and the rotated loop-back bnez $v1 with andi $v0,$v1,0x4000 in its delay slot at asm/funcs/func_8001FAE4.s:20-21; no instruction corresponds to new_var (const-propped then DCE'd), matching the judge's reading

- [s1] [fable-blitz 2026-07-07] The protected shape is plausibly the if-else arm orientation: beqz-taken goes to the +4 arm at .L8001FB14 (asm/funcs/func_8001FAE4.s:15-16) while the fall-through +8 arm ends in `j .L8001FB18` with addiu $a0,$a0,0x8 in the jump delay slot (asm lines 13-14). Register-vs-literal zero compares expand through different do_jump paths pre-cprop and can flip jump.c's arm placement / branch polarity - the switch-vs-ifchain-branch-sense + compare-operand-order-register neighborhood

- [s1] [fable-blitz 2026-07-07] Family identification: `new_var` is literally the SOTN constant-holder idiom - .claude/rules/no-new-park-categories.md (2026-07-01 additions) cites "`new_var` in 9 committed files" as named-local-fake-exception evidence; the family is SANCTIONED last-resort w/ FAKE annotation, though this instance is unannotated and permuter-found, and the judge bar demands the family gone

- [s1] [fable-blitz 2026-07-07] The loop skeleton is already explicit-goto rotated (loop:/ret_zero:/end: at src/code6cac.c:2090-2106) matching the bottom-test structure with the andi recompute in the loop-back delay slot - the skeleton itself is not implicated, only the two compares

- [s1] [fable-blitz 2026-07-07] Judge's own probe order is correct and cheap: literal-0 both uses + delete decl, then measure; if the score rises the diff will show WHERE (arm swap vs loop-back sense), which selects the lever
