# Hypothesis ledger — save_vc_ctrl

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: active; INCLUDE_ASM, 16-insn leaf. Banked `preauth_body.c` reproduces floor 2. Authorized as canonical asm in 385126fcd (replacing a `volatile s32 sp;` frame trick), de-authorized by the owner's 2026-10-01 inline-asm audit (6a51aada9; `pre-slim-2026-10-01:docs/grind/decisions.md:30767`); 871679e71 moved it to its own file.
- CONSTRAINTS: no inline asm (no GTE/BIOS). No volatile pads / unused arrays (`.claude/rules/dead-vars-local-array.md`). The phantom-frame pad exception (`phantom-frame-pad-family.md`) needs an engine allowlist row + proof every honest option fails: last resort only.
- BLOCKER [I, strong]: the 2 differing insns are `addiu $sp,-8` / `addiu $sp,+8` - the target allocates an 8-byte frame it never touches; everything else matches. That is producer 1 of `.claude/rules/phantom-slot-frame-lever.md`: a loop-guard compare folded into a plain branch leaves a stack slot. Completed exhibit: gpu_SetDrawMoveArray (`src/code6cac_c2.c`) `s2 = a1 - 1; if (s2 != -1) do {...} while (--s2 != -1);`, asm `beqz a1 / addiu s2,a1,-1 / li s5,-1` (`asm/funcs/gpu_SetDrawMoveArray.s:13-15`), same 8-byte phantom frame. The banked body has no guard compare (`if (a2 == 0) return;`, hand-assigned `a2 = -1`).
- PLAN:
  1. `sandbox save_vc_ctrl --disable all --candidate` on: `s32 i = n - 1; if (i != -1) { p = (u8*)a1 + 0xC; do { if (*(s32*)p) *(s32*)p += a0; p += 0x68; } while (--i != -1); }`.
  2. If it misses: `while (n--) {...}` with a stride-0x68 record pointer.
  3. Read the `.frame vars=` line to separate frame vs codegen errors (procedure in phantom-slot-frame-lever.md).
  4. No caller declares a prototype (`src/text1a_pre_tu2.c:311-313`), so the signature is free. No owner ruling needed.
- DEPENDS: same mechanism as func_80044010 - do back to back.
- ODDS/LANE: ~1 hour, high [I]. Manual.
