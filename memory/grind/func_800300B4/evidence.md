# Evidence bank — func_800300B4

## s1 (2026-09-02, recon) — MATCHED: sandbox 0, verify-oracle ok

**Function shape (asm/funcs/func_800300B4.s, 83 insns, one basic block, no branches).**
`o = arg0` (u8 *, copied to $s3). `playerData = game_GetPlayerData(o[6] < 1)`;
`mat = playerData[o[9]]` (lands in $a0 because MulMatrix0 takes it as arg 0).
Then three PsyQ libgte hand-asm islands, all with the cluster's
`addu $t4,<reg>,$zero` materialize-then-copy preamble and splat
`/* handwritten instruction */` tags on every cop2 op (.s lines 20-27, 37, 40-42):
1. gte_SetRotMatrix(mat) — 5-word form: lw $t5/$t6 -> ctc2 $0/$1, lw $t5/$t6/$t7 -> ctc2 $2/$3/$4.
2. gte_ldv0(o + 0x2C) SVECTOR form — lhu $t6,4 / lhu $t5,0 / sll $t6,16 / or / mtc2 $t5,$0 / lwc2 $1,8,
   then 2 unfilled nops and MVMVA sf=1,mx=rotation,v=V0,cv=none (.word 0x4A486012 = gte_rtv0).
3. gte_stlvnl(&mac) — addiu $s2,$sp,0x10 (GCC) / addu $t4,$s2 / swc2 $25/$26/$27.
Tail is ordinary C: mac[i] += mat[5+i] (sp-relative loads, i.e. direct local access);
MulMatrix0(mat, o+0xC, mtx); func_8002F2D0(mtx, dir); lookup = D_8008EB80[*(s16*)(o+2)];
func_80049718(lookup, 1, mac, dir); func_800393C8(o[10], lookup, mac, dir).
Frame 0x60: mac @0x10 (12 B), dir @0x20 (8 B), mtx @0x28 (32 B), s0-s3+ra @0x48-0x58 —
declaration order mac, dir, mtx reproduces the offsets (8-byte array alignment).

**Standing.** Owner-cluster canonical-asm member: `tools/grinder/owner_cluster_grants.txt:23`
(cop2-addressing-preamble-cluster.md; door per owner ruling 2026-08-30 §4). Siblings integrated
through the same door: func_8002FC80 (`inline_asm_canonical.txt:365`, src/code6cac_b.c:1383 —
the island spelling this candidate copies verbatim: `move $12, %0` + hardcoded $12-$15 +
clobber list, three separate `__asm__ volatile` statements) and func_8002D320 (:366).
canonical gate: ASM-PARTIAL, 11/83 insns cop2.

**Measurement 1 — plain three-island form (rejected/plain-islands-arg0-mac-seat-swap-13.c): sandbox 13.**
The whole residual is ONE register seat swap: build puts arg0 in $s2 and &mac in $s3, target has
arg0 in $s3 and &mac in $s2 (the prologue sw order and every s2/s3 use follow from it). Everything
else, including all three islands and the load-delay nop maspsx inserts between `lw $a0,0($v1)`
and the island's `addu $t4,$a0,$zero`, is byte-identical.

**Pass attribution (instrumented cc1, BB2_QTY_DEBUG; tmp/grind/func_800300B4/s1/qtydbg_all.txt).**
The function is a single basic block, so LOCAL-ALLOC (not global.c) seats every pseudo.
Suggested-register pass fails for all four call-crossing pointers (their copy suggestions are
$a0-$a3). Second pass orders by local-alloc.c:1660 qty_compare_1 =
floor_log2(n_refs)*n_refs*size / (death-birth), ties by qty number, first free hard reg from $s0:

| pseudo | what | refs | birth-death | priority x10000 | seat |
|---|---|---|---|---|---|
| 75 | lookup | 3 | 76-84 | 3750 | s0 |
| 99 | &dir | 4 | 64-90 | 3076 | s1 |
| 97 | &mtx | 3 | 48-62 | 2142 | s0 (reuse) |
| 72 | arg0 | 7 | 2-88 | 1627 | s2 (target: s3) |
| 109 | &mac | 4 | 24-62(=86) | 1290 | s3 (target: s2) |

Births/deaths are pinned by the target byte order (sched1 runs before local-alloc and the volatile
asm islands are full barriers), so the only free variable is n_refs, which flow.c life_analysis
counts on PRE-combine RTL weighted by loop_depth. Target needs &mac >= 1628: 6 refs gives
2*6/62 = 1935 (5 refs = 1612 is NOT enough); alternatively arg0 <= 5 refs (1162).

**Measurement 2 — `s32 *pv = mac;` copy after the island, calls take pv
(rejected/pointer-copy-of-mac-cse-canonicalized-13.c): sandbox 13, &mac still 4 refs.**
cse.c canon_reg rewrites every use of a same-class pseudo to the class's first register, so any
pseudo-to-pseudo copy of &mac inside the block is dead before flow counts refs (flow deletes it).
Same reasoning kills a second `"r"(&mac)` asm operand (force_reg copy → canonicalized). cse.c:2646
find_best_addr never rewrites frame-pointer-relative MEM addresses, so `mac[i] += ...` is always
sp-direct and never a ref of the pseudo. Natural spellings of this body give &mac exactly 4 refs.

**Measurement 3 — `do { gte_stlvnl asm } while (0);` around island 3 only: sandbox 0; verify-oracle
ok (build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa).** flow.c weights reg_n_refs by loop_depth;
inside the loop notes the &mac def (force_reg of the asm operand) and the asm use count double:
2+2+1+1 = 6 refs → 1935 > 1627. Trace (qtydbg_e2_dowhile.txt:621-622): reg 109 refs=6 ord=14 got=18
($s2); reg 72 refs=7 ord=15 got=19 ($s3). No scheduling change (the loop notes sit between two
volatile-asm barriers; the sched.c:2076 no-cross rule adds nothing). Every other seat unchanged.
Single-level wrap; arg0 has no use inside the wrap so its refs stay 7 (a wrap that also covered
MulMatrix0/lookup/call2 would push arg0 to >=9 refs and floor_log2 jumps to 3 — do not widen it).

**Not tried / ruled out by arithmetic:** wrapping only if it adds arg0 uses (see above); wrapping
island 1 or 2 (no &mac refs there); moving `lookup` earlier (its s0 seat is born at 76 after
func_8002F2D0 returns; an earlier computation would have to survive two calls — bytes forbid).

**Artifacts:** tmp/grind/func_800300B4/s1/body_v1.c, diff.py, qtydbg.py, qtydbg_all.txt (v1 trace),
qtydbg_e2_dowhile.txt (matched trace), tmp/grind/func_800300B4/dumps/ (cc1 -da of v1: f.lreg/f.greg
carry the "Register N used M times across L insns" table).

## s1b (2026-09-02, recon) - layer-1 FAIL follow-up: the prescribed pack-in-C form is measured dead; precedent found

**Chassis re-check.** Banked candidate.c re-applied to src/code6cac_b.c: canonical ASM-PARTIAL (11/83 cop2),
`sandbox --disable all` score 0 (cheat_asm_stripped 33, rules_dropped 0). Floor unchanged; src restored to
`INCLUDE_ASM` at session end (asm-until-matched; no candidate-ready submitted).

**Measurement 4 - island 2 with the VXY0 pack computed in ordinary C (the layer-1 reviewer's prescribed
next action), two spellings, both sandbox 19** (tmp/grind/func_800300B4/s1/diff_packC_A.txt, diff_packC_B.txt;
forms in rejected/pack-in-c-island2-lv-index-vregs-off-s2-19.c and
rejected/pack-in-c-island2-u16-reads-vregs-off-s2-19.c):
  A: `lv = (s32 *)(arg0 + 0x2C); packed = (u16)lv[0] | (lv[1] << 16);` asm operands "r"(lv), "r"(packed),
     island = move $12,%0 / mtc2 %1,$0 / lwc2 $1,8($12) / nop / nop / .word 0x4A486012.
  B: same but `packed = *(u16 *)(arg0 + 0x2C) | (*(u16 *)(arg0 + 0x30) << 16);`.
Build emits (A) `addiu a1,s2,44; lw v0,48(s2); lhu v1,44(s2); sll v0,v0,16; or v1,v1,v0; move t4,a1; mtc2 v1,$0`
(B identical except `lhu v0,48(s2)`), vs target `addiu v0,s3,44; move t4,v0; lhu t6,4(t4); lhu t5,0(t4);
sll t6,t6,16; or t5,t5,t6; mtc2 t5,$0`. Two independent gaps, neither reachable from C:
  (i) the target's halfword loads are based on $t4 - a register that exists only inside the asm string as
      the SDK macro's `move $12,%0` copy; C code has no handle on it, so cse folds every C-side read to an
      arg0-pseudo-based offset. Any C spelling of the pack reads off arg0's register, never off $t4.
  (ii) the temps are $t5/$t6 ($13/$14). local-alloc.c:2249-2258 find_free_reg scans hard regs in numeric
      order because config/mips/mips.h defines no REG_ALLOC_ORDER (grep count 0), and $v0/$v1 are free at
      that point (both forms put the pack in v0/v1); no C-side temp can land in $13/$14 while $2/$3 are free.
  Side effect: the extra C-side arg0 reads raise arg0's reg_n_refs, so the arg0/&mac seat swap (E1) returns
  on top of the island bytes even with the E3 do-while(0) wrap in place - 19 = 7 island insns + 12 seat-swap.
Conclusion: the lhu/lhu/sll/or is hand-asm in the target (PsyQ inline_a.h gte_ldv0 macro body, which
hardcodes $13/$14 and reads through the $12 copy). It is not a "swallowed" compiler-expressible computation;
it is unreachable by GCC 2.7.2 from any C source.

**Precedent (on main, found this session - the s1 self-vet cited the wrong sibling).**
  - src/code6cac.c:1858-1870, func_800203B4: the SAME island character-for-character (move $12,%0; lhu $14,4($12);
    lhu $13,0($12); sll $14,$14,16; or $13,$13,$14; mtc2 $13,$0; lwc2 $1,8($12); nop; nop). Owner-GRANTED
    2026-09-01 (inline_asm_canonical.txt:367) with the grant text explicitly recording that "three thin-island
    variants that move C-expressible macro parts into C score 12/4/8 - memory/grind/func_800203B4 s6" and that
    the islands were "authorized AS UNITS after the pure-C respelling was measured unable to close".
  - src/code6cac_b.c:1245-1255, func_8002E838: cluster sibling (owner_cluster_grants.txt:22), same island,
    Judge final call PASS 2026-09-02 01:27 (docs/grind/decisions.md:20262), which found the island
    "CHARACTER-IDENTICAL to ... func_800203B4 src/code6cac.c:1839-1881 (owner grant 2026-09-01)".
  - src/code6cac_b.c:2012-2022, func_80031890: same island, Judge PASS 2026-09-02 01:34 (decisions.md:20268).
  The s1 self-vet's precedent citations (func_8002FC80, which has no gte_ldv0 island, and LoadAverageShort12
  inline_asm_canonical.txt:174) were the wrong anchors - both are now BANNED citations and are not reused.

**Remaining question (ruling-request, not re-grind).** The layer-1 FAIL applies cluster condition 3 ("in-island
GPR limited to the cop2 addressing preamble") to the gte_ldv0 macro-body pack, while the same island passed the
Judge in two cluster siblings on the same day and is owner-granted in func_800203B4. Measurement 4 shows the
prescribed alternative cannot be built. The pack island is now a BANNED construct for this function, so the
driver rejects a candidate-ready re-declaring it regardless of precedent; the only path is a ruling.

**Artifacts:** tmp/grind/func_800300B4/s1/v_packC_A.c, v_packC_B.c, diff_packC_A.txt, diff_packC_B.txt, apply.py.

## s1c (2026-09-02, recon) - ruling landed; chassis re-check 0; resubmitted

**Ruling 2026-09-02 07:30 (docs/grind/decisions.md:20474) PASS:** the gte_ldv0 macro-body pack (lhu/lhu/sll/or
through the macro's $12 copy) IS inside the sanctioned island unit; the island-2 ban is lifted. The second ban
(the s1 self-vet's func_8002FC80 / inline_asm_canonical.txt:174 LoadAverageShort12 anchors) STANDS - those
citations are removed from self_vet.md entirely; the vet now anchors on func_800203B4 (inline_asm_canonical.txt:367,
src/code6cac.c:1858-1870), func_8002E838 (decisions.md:20262) and the ruling itself (decisions.md:20474).

**Chassis re-check.** candidate.c applied to src/code6cac_b.c: canonical ASM-PARTIAL (11/83 cop2);
`sandbox --disable all` score 0, 83/83, rules_dropped 0, cheat_asm_stripped 33
(tmp/grind/func_800300B4/s1/sandbox_s1c.txt). No source change beyond the header comment. Edits left in place
in src/ for the driver's byte re-verification (candidate-ready).

**Still owed at the final call (ruling text):** the island-3 do-while(0) FAKE prerequisites (all three present:
annotation carries what + mechanism flow.c loop_depth ref weighting -> local-alloc.c qty_compare_1 + lever-exhaustion
pointer to hypotheses.md H1-H3; single-level; family sanctioned for register allocation by owner ruling 2026-07-06),
and the owner-cluster inline_asm_canonical.txt line, which the driver writes before queue done (honest bucket
COMPLETED-INLINE-ASM-CANONICAL, as func_8002E838).
