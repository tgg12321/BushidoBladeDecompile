# Hypothesis ledger — func_80023F08

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: active; INCLUDE_ASM; 2983 insns (0x80023F08-0x80026DA0), frame 0x250, 226 labels, one exit, no jtbl, no cop2. 65 calls / 38 callees; only func_800204C0, func_800207C8, func_80049718 still asm (heavy: func_80021424 x10, func_80021A98 x7, func_8001F860 x4). Ledger is stale (says code6cac.c, asmfix, "raw-offset convention"); its m2c (`tmp/blitz/m2c_80023F08.c`) is GONE. Largest item in the queue.
- CONSTRAINTS: size isn't evidence of hand-written asm (`.claude/rules/canonical-gate-distance-not-evidence.md:30`) - ordinary C. PracticeMenuRec members only, no `*(T *)(rec + off)` (`memory/grind/func_80021424/HANDOFF.md` goal).
- WHAT IT IS [I]: per-player, per-frame input / move-command executor. arg1 is a 0x18-byte PadState (callers pass a copied pad record, tu2:2134-2289; prologue copies 6 words to rec+0x24..0x3B -> rec+0x24 should be a PadState member). [F] six inline 0x84-byte block moves (asm lines 1095-1150), one stack->stack (sp+0x9C -> sp+0x18) still with the runtime alignment check -> [I] struct assignment of a 0x84-byte 2-aligned (s16) record, not the ledger's memcpy guess; same-TU precedent func_80022580's SVec4i16 copies emit lwl/lwr.
- PLAN:
  1. Regenerate m2c (`tools/m2c`) into this ledger dir.
  2. Map every rec+0x24..0x84 offset to typed members; coordinate with L1/L2 header edits and func_80058580's record (StatusFlagRec).
  3. Scratch-TU probe: 0x84-byte s16 struct assigned by value vs memcpy.
  4. Draft by region (prologue/repack -> move-list walker -> direction if-chain -> state middle 0x25430-0x26290 -> tail); land the whole body in a working copy and drive the floor down with per-region `sandbox --diff`.
  5. Can't be split into separate symbols; stage it: one session data model + m2c, then sessions per region.
- DEPENDS: shares the record with func_80058580 and the L1-L4 cleanup. Calls func_800204C0 / func_800207C8 but isn't blocked by them.
- ODDS/LANE: 10+ sessions; manual scaffold first, then the Grinder for the long residual (restart needs owner approval). ~25% to 0 [I]; large drop very likely.

## [s2] 2026-10-01 laneC — next steps from the v18 scaffold (sandbox 0)
1. Data model into include/code6cac.h: PracticeMenuRec members (PadState unk_24 replacing unk_22/2C/30/34 —
   code6cac_b.c's 7 unk_2C/unk_30 reads become unk_24.held/.pressed; Pose at 0x290; u16 unk_288[2]; unk_320
   Vec3i32; SVec4i16 unk_98; …), Pose / move-script record types, ScrPad+SPAD moved out of code6cac_b_tu2.c,
   D_8008DA50/94/D8 as s16 arrays, D_800A3888 as Pose *[2] (+func_80020D70 respell, D_800A388C extern
   retired), D_800A36D8 pointer-typed (+tu2:2774/2779 respell), func_80023F08(s32, PadState *) + 3 callers.
2. Move-script record vs u16 dispatch-table view of func_80021424's return: decide an honest type (struct for
   unk_50/unk_7C byte+halfword header; u16 * for dispatch indexing) — text1b.c uses `unk_50[8]` (laneB).
3. Multi-write locals to eliminate or package (Ruling 11 / Q51): d (split), state (split), t (ternary?),
   add / nf (ternary?), a (angle then angle+0x400), bits (switch + zeroing), flags (loop + normalisation).
