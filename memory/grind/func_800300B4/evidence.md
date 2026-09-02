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
