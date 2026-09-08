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

## s1b (2026-09-08, recon — re-dispatch after the 387fa8f8 merge refusal; HEAD 387fa8f8)
11. OBJECT MODEL: unchanged from s1 item 1 — the function touches NO named globals (all accesses
    are `arg0` field reads at +0x4/+0x350/+0x352/+0x354, two calls, one scratchpad literal).
    No DATA MODEL flags; nothing to re-declare. MATCHES (re-measured score 0 this session).
12. Chassis re-measurement on HEAD 387fa8f8 with candidate.c applied to src/code6cac.c:
    `sandbox func_800204C0 --disable all` = score 0, 122/122, rules_dropped 0, cheat_asm_stripped 20
    (tmp/grind/func_800204C0/s1/sandbox_s1b.json). `canonical` = ASM-PARTIAL, 11/122 cop2
    (c2/ctc2/lwc2/mtc2/swc2). Body hash on the Judge PASS record: 8655cc28f3aa5cc7
    (state.json review_ledger, 2026-09-08 06:15). The candidate is byte-identical to the body
    the Judge PASSed; nothing in the C changed this session.
13. `tools/scan_hand_coded.py --single func_800204C0` = tier LOW 1/8 (S4 front loads only; none
    of S1/S2/S6) — tmp/grind/func_800204C0/s1/scan_hand_coded.txt. This is the GTE-wrapper
    misroute artifact the 2026-09-01 grant record itself names; it cannot open the STRONG door.
14. Judge-constraint branch "islands are C-expressible (respell them in C)" measured DEAD on this
    chassis: thin-island probe (gte_SetRotMatrix as five C word loads + five ctc2-only islands,
    other three islands unchanged) — GCC 2.7.2 seats the loads in $t0/$a0/$v0/$a0/$v1 as a block
    ahead of each ctc2 pair, emits no `move $t4,$v1` copy, 11 insns vs the target's 12 in the
    region, 0/12 reproduced (tmp/grind/func_800204C0/s1/thin1_try.txt;
    rejected/thin-island-c-loads-seat-t0-a0-v0-not-t5-t7.c). Same class as func_80019310 s3 H10
    (rejected/…-c-loads-take-a2-a0-a1-v1-v0-not-t5-t7.c) and func_800203B4 s6. The redundant
    materialize-then-copy preamble is the grant's defining evidence precisely because the
    compiler never produces it from C.
15. Grant-door state: `grant_canonical_asm` (tools/grinder/grindlib.py:1493) needs STRONG tier OR
    a row in tools/grinder/owner_cluster_grants.txt. Tier is LOW; the registry (read this session)
    has rows for func_80031890, func_8002FF20 (2026-09-02) and func_80019310 (2026-09-06
    foreclosed-bucket review) but NO func_800204C0 row. The 2026-09-01 widened-anchor GRANT
    names func_800204C0 by name as a confirmed handwritten-tagged carrier (decisions.md:18120;
    .claude/rules/cop2-addressing-preamble-cluster.md:155). The registry is OPERATOR-MAINTAINED
    ONLY (its header) and tools/ is outside session scope. Exact precedent: func_80019310 —
    refused 12427b10 -> handoff entry decisions.md:24678 -> owner-gated/foreclosed 2aed243e ->
    operator row 2cef233c -> unpark 0bb257ca -> Judge PASS decisions.md:25003 -> merged 3869ca31
    as COMPLETED-INLINE-ASM-CANONICAL.
16. Disposition filed this session: docs/grind/decisions.md "2026-09-08 — func_800204C0 —
    CANONICAL-ASM GRANT PATH: blocked at the operator registry row" (integration handoff, not an
    exhaustion claim). Outcome owner-gated with escalation_ref worded WITHOUT the grant-path
    keyword so the driver takes the borderline-log + silent-foreclosure branch (grind.ps1:1449
    vs :1474) exactly as func_80019310 did; re-activation trigger = the registry row.

- [s1] OBJECT MODEL: no named globals touched (all arg0 field reads at +0x4/+0x350/+0x352/+0x354, two calls, one scratchpad literal); no DATA MODEL flags apply; MATCHES (score 0 re-measured this session).

- [s1] sandbox func_800204C0 --disable all = 0 (122/122) on HEAD 387fa8f8 with candidate.c applied; canonical = ASM-PARTIAL 11/122 cop2; body identical to the Judge PASS record 8655cc28f3aa5cc7 (state.json review_ledger 2026-09-08 06:15).

- [s1] scan_hand_coded tier LOW 1/8 (S4 only): no STRONG door; grant_canonical_asm (tools/grinder/grindlib.py:1493) then needs an owner_cluster_grants.txt row, which exists for func_80031890/func_8002FF20/func_80019310 but not func_800204C0, although the 2026-09-01 widened-anchor GRANT names func_800204C0 by name (decisions.md:18120; .claude/rules/cop2-addressing-preamble-cluster.md:155).

- [s1] Refusal branch 'islands are C-expressible' measured dead: thin-island respelling of gte_SetRotMatrix scores 0/12 in the region (rejected/thin-island-c-loads-seat-t0-a0-v0-not-t5-t7.c).

- [s1] Precedent chain for the remedy: func_80019310 refused 12427b10 -> handoff entry decisions.md:24678 -> owner-gated 2aed243e -> operator row 2cef233c -> unpark 0bb257ca -> Judge PASS decisions.md:25003 -> merged 3869ca31 COMPLETED-INLINE-ASM-CANONICAL.

- [s1] Integration handoff filed: docs/grind/decisions.md:25556 with the exact one-line registry row the operator appends; src/code6cac.c restored to INCLUDE_ASM (asm-until-matched); candidate.c unchanged and ready to submit EXACTLY once the row lands.
