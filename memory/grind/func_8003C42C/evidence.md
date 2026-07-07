# Evidence bank — func_8003C42C

- Audit diagnosis (regressions.md): do{} while(0) at src/code6cac_c2.c:592 is completely unannotated — no '// FAKE' or '/* FAKE: ... */' comment. Per [[do-while-zero-exception]], the annotation is a mandatory transparency requirement; if absent, the exception does not apply and the construct defaults to FORBIDDEN (tests #1 and #2 fail: no semantic purpose, no human programmer would write it). The commit (8db4e1e4, 2026-03-26) predates the rule (2026-06-04) and documents no lever exhaustion and no mechanism rationale. Remediation: add '// FAKE: do{} while(0) — suppresses reorg.c LABEL_OUTSIDE_LOOP_P invert-jump peephole on bne→.L8003C520 branches' (or equivalent one-line explanation), then re-verify the exception prerequisites are met (lever exhaustion evidence, correct GCC-pass target). This is a TRANSPARENCY fix, not a full C re-match — the rest of the function body is clean.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct located: src/code6cac_c2.c:592 `do { } while (0);` (empty body, single-level, no FAKE annotation), placed immediately after `D_800A382D = v0;` (line 591) -- the C statement for the asm join label .L8003C520 (asm/funcs/func_8003C42C.s:69-70, lui/sb).

- [s1] [fable-blitz 2026-07-07] Target-byte evidence the wrap is plausibly LOAD-BEARING: TWO `bne $v0, $v1, .L8003C520` NE-condition branches (asm/funcs/func_8003C42C.s:31 and :65) each carrying the fall-through value `slt $v0, $v0, $v1` in the DELAY slot -- precisely the NE-branch-to-outside-label shape [[do-while-zero-exception]] documents reorg.c relax_delay_slots invert-jump suppression for (LABEL_OUTSIDE_LOOP_P via the wrap's NOTE_INSN_LOOP_BEG).

- [s1] [fable-blitz 2026-07-07] Ruling status: per the FINAL 2026-07-06 owner ruling in .claude/rules/do-while-zero-exception.md, do{}while(0) (any body incl. empty) is sanctioned for ANY codegen effect with a mandatory inline FAKE annotation; lever-exhaustion is NO LONGER a prerequisite for single-level wraps. The diagnosis's demand to 're-verify lever exhaustion evidence' is superseded -- remediation reduces to the annotation (+ optional inert-check).

- [s1] [fable-blitz 2026-07-07] The wrap is SINGLE-LEVEL (not nested), so prerequisite 3 (nesting justification) does not apply.

- [s1] [fable-blitz 2026-07-07] Rest of the body is clean: no pins, no __asm__, no volatile casts; two plain histogram loops over D_800A377C (src/code6cac_c2.c:553-557) and D_800F65F8 (:571-582) matching asm loops .L8003C458 and .L8003C4C8/.L8003C4D4. The regression is transparency-only, exactly as the judge diagnosed.

- [s1] [fable-blitz 2026-07-07] Commit context: 8db4e1e4 (2026-03-26) predates both the 2026-06-04 rule and the 2026-07-06 final ruling -- absence of annotation is historical, not evasion.
