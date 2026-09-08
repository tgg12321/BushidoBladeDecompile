# Evidence bank — func_800204C0

## s1 (2026-09-08, recon)
1. OBJECT MODEL: the function touches NO named globals — every access is through `arg0`
   (fields +0x4 s16 player id, +0x350 s16 counter, +0x352 s16 index, +0x354 3xs16 vector),
   two calls (`game_GetPlayerData`, `func_80032854`) and the scratchpad literal
   0x1F8000A8 + pid*0x108 + idx*0xC (a3 = &out). No DATA MODEL flags apply; nothing to
   re-declare. MATCHES (measured score 0 with the candidate's `u8 *arg0` field reads).
2. Canonical gate: ASM-PARTIAL, 11/122 insns cop2 (ctc2 x5, mtc2, lwc2, swc2 x3, c2).
   `python3 tools/scan_hand_coded.py --single func_800204C0`: tier=LOW score 1/8 (S4 only) —
   the known GTE-wrapper misroute artifact (cop2-addressing-preamble-cluster.md, "LOW
   whole-function scan tier").
3. Function is NAMED as a confirmed handwritten-tagged carrier in the 2026-09-01 widened-anchor
   owner GRANT (docs/grind/decisions.md:18119-18121; .claude/rules/cop2-addressing-preamble-cluster.md:154-155).
   It has NO row in tools/grinder/owner_cluster_grants.txt (that grant's rows: func_80031890,
   func_8002FF20, func_80019310). Precedent path = func_80019310: candidate-ready -> Judge PASS
   (decisions.md:25003) -> operator registry row (commit 2cef233c) -> driver grant ->
   inline_asm_canonical.txt:378. The registry row is an OPERATOR step (tools/ is out of session scope).
4. Islands: three materialize-then-copy `addu $t4,$rX,$zero` preambles with sources $v1/$v0/$v0
   (asm/funcs/func_800204C0.s lines 28, 50, 59) + 8 splat "handwritten instruction" tags — the
   widened-anchor idiom exactly. Island spelling copied character-for-character from
   func_800203B4 (src/code6cac.c, inline_asm_canonical.txt:367): gte_SetRotMatrix
   (inline_c.h:297-310), gte_ldlv0 (inline_c.h:101-110; the lhu/lhu/sll/or pack IS the macro
   body — owner Ruling A 2026-09-02), .word 0x4A486012 (MVMVA sf=1 mx=rot v=V0 cv=none lm=0),
   gte_stlvnl (inline_c.h:1111-1117). Header cites verified this session against
   tmp/grind/motion_SetMotion/s7/repos/rood-reverse/include/psx/inline_c.h.
   No "memory" clobbers used (the game_GetPlayerData call already forces the post-island
   `lh 0x350` reload; measured 0 without them).
5. First candidate (`cnt = *p + 1; *p = cnt; if ((cnt & 7) == 2)`): sandbox 10, 121/122.
   Residual = frame (vars=24, target 32: prologue/epilogue offsets, 8 insns) + missing
   `move $v1,$v0` in the beqz delay slot. The two nops after lwc2 LOOKED missing in objdump
   but are present (objdump collapses repeated zero words to `...`) — NOT a residual.
6. Fix (measured with the fast cc1 gradient tmp/grind/func_800204C0/s1/try.py, then sandbox):
   `*(s16 *)(arg0 + 0x350) += 1; if ((*(s16 *)(arg0 + 0x350) & 7) == 2)` -> `.frame $sp,64`
   vars=32, the delay-slot copy appears, sandbox --disable all == 0 (122/122). Mechanism:
   cse.c store-to-load forwarding replaces the re-read with the stored pseudo (andi on $v0)
   and replaces the `+= 1` re-load of the already-compared value with a copy of the first
   load's pseudo (`move $v1,$v0`); the HImode read-modify-write on memory allocates an
   8-byte stack temp that is later register-allocated away = the phantom-frame-slot
   artifact (Claude memory phantom-frame-slots-gcc272). Ordinary C, no FAKE construct.
7. Frame probes prepared but NOT measured (moot at 0): s16 tx/ty/tz (s1/fc.c), s16 cnt
   local (s1/fd.c), both (s1/fe.c).
8. `if ((s16)ty >= 0x801)` reproduces `sll/sra 16; slti 0x801` (compare on the HImode-truncated
   value); the tail `if (*p >= 0x96) *p = 0; *p = 0;` reproduces the double `sh $zero`
   (no cross-jump merge) — same tail as the retired-chassis body.
9. Retired-chassis body (retired-chassis-2026-08/body.c, floor 52) carried register-asm pins
   (`register ... asm("$12")`) — a forbidden family; not reused.
10. Tooling: tmp/grind/func_800204C0/s1/try.py = cpp|cc1 direct gradient (prints `.frame`
    vars= and a normalized diff vs the target) in seconds; cc1 exits non-zero on the TU's two
    pre-existing stale-prototype parse errors (src/code6cac.c:788/1104 `GameObj`) but still
    emits the .s — the make pipeline ignores that exit code too.
