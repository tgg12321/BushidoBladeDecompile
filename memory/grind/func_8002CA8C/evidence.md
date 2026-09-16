# Evidence bank — func_8002CA8C

- [s1] OBJECT MODEL: D_800F5F68 (census g_per_char_record_440, no header decl) MATCHES — the TU-local `extern u8 D_800F5F68;` + `(u8 *)&D_800F5F68 + id * 0x1B8` spelling (same as src/code6cac.c:133/2115) reproduces the target's `lui/addiu %hi/%lo(D_800F5F68)` + `id*0x1B8` (sll/subu/sll/subu/sll) prologue byte-exact in every measured form (v1..v8a diffs show zero residual on those insns). No SIGNALS were flagged in the brief; no declaration change needed. Record layout used: +0x00 s16 flag, +0x0C/+0x0E/+0x10/+0x12 u16 (22 records x 0x14 = 0x1B8).

- [s1] canonical: verdict C (pure-C distance 95 on the draft, scan_hand_coded tier LOW, 0 cop2). Honest floor (sandbox --disable all): draft 95/179 (build 159 insns) -> 89 (s32 r) -> 55 (walking rec pointer) -> 30 (&SCR[id].j[i+4] call arg) -> **5** (shared `if (hit) seenMask |= 1 << i;` tail), build_insns 179 == target 179. Candidate = memory/grind/func_8002CA8C/candidate.c (s1 form v8a). src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) at session end.

- [s1] Address key: 0x1F8000A8 == (u8 *)SCR + 0x30 == &SCR[0].j[4] (src/code6cac_b.c:918-919 `typedef struct { Vec3i j[22]; } ProbeScr; #define SCR ((ProbeScr *)0x1F800078)`). The target's call-arg pointer is &SCR[id].j[i + 4]; the target's loads are SCR[id].j[i+4].x/y/z but must be spelled with an integer offset (`*(s32 *)((u8 *)0x1F8000A8 + off)`, off = base + i*0xC) — see the v7b kill.

- [s1] MECHANISM (read in tmp/grind/func_8002CA8C/dumps/code6cac_b.loop, v3a form): with an integer call-arg spelling `i*0xC + 0x1F8000A8 + base` (or any of the 4 spellings v3a/v4A/v4B/v4C — pointer casts included, split_tree strips NOP_EXPR at fold-const.c:893) fold's associate rule (fold-const.c:3703-3736, `(VAR+CON)+ARG1 -> VAR+(ARG1+CON)`) rewrites it to `i*12 + (base + C)`. The expander emits `r135 = C` / `r134 = base + r135` (dump insns 209/211); loop.c records both as movables ("regno 135 life 4 move-insn savings 4", "regno 134 cond forces 209") and hoists them; the call-arg becomes a giv `mult 12 add r134` reduced to its own IV (reg 175 = base+C+i*12, one `move a2,s6` per arm). The target instead keeps `lui/ori C` in each arm and does `addu a2,s7,a2 ; addu a2,fp,a2` = `(i*12 + C) + base`, with s7 = i*12 its own reduced giv.

- [s1] `&SCR[id].j[i + 4]` (ADDR_EXPR of a struct-array element) folds to `id*0x108 + (i*12 + 0x1F8000A8)` — the target's operand order. The constant is force_reg'd adjacent to its single use per arm (2 sites, no `forces` dependency), so loop.c's move test `threshold * savings * lifetime >= insn_count` (loop.c:1631; threshold = 1 + n_non_fixed_regs for a loop with calls, loop.c:532, minus 3 per prior move, loop.c:1719) fails and the constant stays in-loop; `i*12 + Creg` is then not a giv (invariant_p on a reg set inside the loop returns 0, loop.c:2758) so only `i*12` is reduced (= s7). Measured: v7a 30.

- [s1] Const-1 hoisting: v3a hoisted the `1` of `1 << i` into a register (4 identical `set r = 1` sites; combine_movables sums savings/lifetime, loop.c:1283-1284 -> savings 4 x lifetime 4 clears the move test). With the shared seen tail (2 sites) it stays in-loop as the target's `addiu $t0,$zero,1`.

- [s1] Shared tail shape confirmed: arms = `hit = callX(0, scr, &SCR[id].j[i+4], r, rec+0xE); if (hit) { if (rec0 && a2) { if (callX(1, scr, 0, rec+0x10, rec+0x12)) hitMask |= 1 << i; } }`, then ONE `if (hit != 0) seenMask |= 1 << i;` after the if/else. This reproduces the redundant `beqz $s0` at .L8002CCD8 and the a2==0 -> .L8002CCE0 edge WITHOUT any source duplication (jump threading, not cross-jump). Both else-duplication spellings measured worse (v3b 63, v8b 42).

- [s1] Residual at 5 (tmp/grind/func_8002CA8C/s1/diff_v8a_sharedseen.txt; ignore the normalizer's fp/s8, move/addu-zero, li/addiu, `-#` artifacts): (a) the AABB `rejected` flag is allocated to $a1 in ours, $s0 in target (3 insns: `addu s0,zero,zero` delay slot / `addiu s0,zero,1` / `bnez s0`); the target's $s0 is also `hit` (live across call2), so the two flags look like ONE pseudo, or a pseudo global.c seats callee-saved; (b) `lhu $a3,-6($s2)` (the r load) sits BEFORE the `lw $a0` x-load in target, AFTER it in ours (one reorder). Everything else matches, including every spill (a0/a1/a2 homed at 0x18/0x20/0x28, seenMask at 0x30($sp) with `sw $zero,0x30($sp)`), the IV set ($s3 i, $s4 base+i*12, $s5 rec, $s2 rec+0x12, $s7 i*12, $fp base) and both call arms.

- [s1] Callee signatures: func_8002D320 is defined at src/code6cac_b.c:1023 `s32 (s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq)`; func_8002D780 is INCLUDE_ASM (:1319). The candidate adds forward `extern` declarations for both above the function (the D320 one is redundant; drop it if a reviewer objects — it is byte-neutral).

- [s1] Tooling: tmp/grind/func_8002CA8C/s1/measure.ps1 applies a variant .c into src (apply.py replaces the INCLUDE_ASM line or the previous body), runs the sandbox, and writes diff_<name>.txt via mydiff.py (objdump of tmp/sandbox/func_8002CA8C/code6cac_b.o vs asm/funcs/func_8002CA8C.s). Variant files must be LF (Windows python `open('w')` writes CRLF — use newline='\n').

- [s1] OBJECT MODEL: D_800F5F68 MATCHES with the TU-local extern u8 + id*0x1B8 spelling (prologue byte-exact in every form); no declaration change needed

- [s1] 0x1F8000A8 == &SCR[0].j[4] (ProbeScr at 0x1F800078, src/code6cac_b.c:918); the call-arg pointer is &SCR[id].j[i+4]

- [s1] canonical verdict C, tier LOW, 0 cop2; sandbox floor 5/179 with build_insns == target_insns == 179

- [s1] residual at 5: rejected flag seated in $a1 (target $s0, the same seat as hit) and lhu a3,-6(s2) ordered after the x-load (target before)

- [s1] candidate.c = s1 form v8a; 6 rejected forms banked with their scores in memory/grind/func_8002CA8C/rejected/; src/code6cac_b.c restored to HEAD
