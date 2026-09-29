# Evidence bank — func_8005E54C

- [s1] [fable-blitz 2026-07-07] Queue/park state: distance 798, verdict ASM-STRUCTURAL but PARKED with an explicit 2026-06-09 canonical-asm REJECTION ('asm reads as standard GCC-compiled C ... needs pure-C work'). Single rule = asmfix.txt:186 replace_with_asmfile; src/text1b.c:12860 is a (void)-cast stub returning 0. So floor 798 measures a missing body, not a plateau -- any plausible draft collapses it.

- [s1] [fable-blitz 2026-07-07] THE TEMPLATE: the sp+0x28 local block maps field-for-field onto `EnvA` (src/text1b.c:16909-16923): header@0x28, table@0x2C, out@0x30, semi@0x38, ot_idx@0x3C(=arg2, stored asm s:29,477), x@0x40, y@0x44, pad20@0x48(=0x100 s:625-626), pad24@0x4C(=0x400 s:627-630), has_color@0x50, col_r/g/b@0x51-53(=0x40 s:501-505). Every draw is `env.out = pkt; pkt = func_8007352C(&env);` (sw $s2,0x30 in/before jal delay, s2=ret, e.g. s:59-60,86-88,172-175). ~15 call sites + 4 func_80073728 sites (s:653,659 x2 iterations).

- [s1] [fable-blitz 2026-07-07] Signature/frame: func_8005E54C(s32 arg0 bitpack, u8 *arg1 pktbuf, s32 arg2 ot_idx) -> s32. Frame 0xB8, saves s0-s7+fp+ra (all callee-saves live). Prologue s:4-5 `sw $a1,0x68(sp); lw $s6,0x68(sp)` = a1 homed to stack AND copied to a working local (s6 advances: +0xA0 base s:21, s6+=0x10 twice s:759,782); arg1 itself re-read at the end. Derived pointer locals stack-homed: 0x78=arg1+0x898 (updated from func_80073728 returns s:661), 0x80=arg1+0xBB8 (advanced +0xC s:462-466, passed to initTexPage/ot_Link), 0x88=arg1+0xBC4 (never changed).

- [s1] [fable-blitz 2026-07-07] RETURN: s:841-844 `lw $t0,0x88(sp); lw $t1,0x68(sp); subu $v0,$t0,$t1` = return (arg1+0xBC4) - arg1 computed as a subtraction of two stack-resident locals -- NOT constant-folded because both live in stack slots (reload MEMs) across ~25 calls. C shape: `u8 *end = arg1 + 0xBC4; ... return end - arg1;` predicts this naturally; a draft must NOT pre-fold to `return 0xBC4;`.

- [s1] [fable-blitz 2026-07-07] Locals at sp+0x18: s16 vals[2] @0x18/0x1A (2-bit fields of arg0 extracted per round: srlv/andi 3, s:112-119) and s16 wins[2] @0x20/0x22 (zeroed s:471-472, accessed as `lh 0x8($a1)` off $a1=&vals[i] s:530-533,715-721 => wins sits exactly +8 bytes after vals with a 4-byte gap at 0x1C/0x1E). Layout reconstruction (what fills 0x1C) is an open probe: candidates s16[6] array (arr[i]/arr[i+4]), or vals[2]+pad[2]+wins[2] declaration order.

- [s1] [fable-blitz 2026-07-07] Global flag word D_8009BD38 re-loaded fresh at every use (~9 loads, s:31-32,61-62,100,305,314,340,467,565,790) -- never cached in a callee-save. Three field decodes recur: (f>>15)&3 then >>i &1 (per-player alt-costume bit, s:34-37); rounds = ((f>>10)&3)+3 (s:102-104,343-345,676-678); mode = f&0xC00 vs 0x800/0x400/else (6 sites: s:122-142, 319-329, 568-584, 688-698, 793-809). C must re-read the global per use (store-const-reload-cse rule family: do NOT cache in a local).

- [s1] [fable-blitz 2026-07-07] Region map: R1 s:30-95 for(i=0;i<2;i++) two paired 8007352C calls, table picked by costume bit (D_8009B524/53C then 530+56C/548+57C), x=i*5<<6 via (s16)i strength-reduced s:50-55. R2 s:96-312 per-round row: extract vals, y by mode (i*24+0x44 / i*24+0x4F / i*34+0x4F s:122-143), both-3 => single draw glyph D_8009B4B0/4BC+lbu(D_800A3270[i])*8 s:153-175; else 4E4/514 + cmp glyph (4FC gt / 504 lt / 50C eq, s:189-234) + per-side tally loop x=i? s0*16+0x179 : (1-s0)*16+0xE2 (s:261-290). R3 s:313-380 totals: y=0xC6/0xBE/0xC2, sum vals[side]+=field skipping ==3 (s:351-372). R4 s:381-446 win-marks: x=s0/2*20+0x181 or 0xF2-s0/2*20, y=s4+(s0&1)*12. R5 s:447-466 saMotionSet(D_8009B524)+initTexPage+ot_Link. R6 s:467-681 per-round panel loop: wins[i]++ on compare (s:516-535), portrait char=lbu D_8009BD24[i*10+round] clamp(>=12 -=2) s:543-550, env.table=UesrWorkDef+char*24 s:554-560, x=(i*5)<<6+lbu D_8009B58C[char] s:561-569, y from PRE-COMPUTED fp=round*24-8 / s7=round*24+3 (s:485-488, computed BEFORE the mode branch each outer iteration) else round*34+3; char==8 extra D_8009ADC0 draw s:598-609; if vals[0]==3: 2x func_80073728 chain w/ D_8009B3B0+i*12, D_8009B490+i*16, D_8009B498+i*16 (s:617-668). R7 s:682-739 footer: y=0xC9/0xC1/0xC5, digit=D_8009B400+wins[i]*8 (its first 2 halfwords ZEROED before the call, sh s:728-729), x=i*70+0x113 (+0x116 if wins[i]==1). R8 s:740-825 three initTile/gpu_SetSemiTransp/ot_Link tile bars (0xFF,0x10,0x10; x=8/0x19D; y=0x3A; w=0xDC; h=1; third tile x/y by mode 0x5E+0xC1/0xBD/0xB9). R9 s:826-857 final saMotionSet(D_8009ADB4)+initTexPage+ot_Link+return.

- [s1] [fable-blitz 2026-07-07] All loop counters are s16 (sll/sra 16 around every increment and compare, e.g. s:89-94, 236-241). Register roles: s4=outer round counter (reused as y-base after R3! s:384 `addu $s4,$s0,$zero`), s1=middle, s0=innermost (reused as y-const holder in R3 s:324-329), s2=pkt cursor, s3=&vals (R2/R6) then D_8009B498 base (R6 tail, s:622-623), s5=&env (set once s:478), s6=arg1 working copy, s7/fp=precomputed per-round y offsets. The s4/s0 role REUSE across phases suggests the original reused named locals -- a draft with fresh locals per phase will likely mis-allocate.

- [s1] [fable-blitz 2026-07-07] Round-count loop entry guards are `beqz $v0` right after `addiu $v0,$v0,3` (s:104-105, 345-346, 475-476) -- semantically dead (count>=3) and a beqz (==0) test while all back-edges are signed slt (s:310,378). Plain `for(i=0;i<n;i++)` pre-tests usually emit blez; the beqz suggests an explicit `if (n) { ... }` guard or unsigned entry compare in the original. Cheap compile-probe will disambiguate.

- [s1] [fable-blitz 2026-07-07] Mode if/else-if/else chains compile with the assignment hoisted into the j/bne DELAY SLOT of each arm (e.g. s:321-329: `bne =>` with `sw $zero,0x40` in delay; s:692-698 const loads 0xC9/0xC1/0xC5 in delay slots) -- standard reorg.c fill from a plain 3-way chain; no exotic construct predicted.

- [s1] [fable-blitz 2026-07-07] func_8007352C itself is DECOMPILED (src/text1b.c:16925) and its body documents the EnvA field semantics (header u16 clut coords, table = 8-byte glyph entries, x/y offsets, ot_idx, has_color+RGB) -- use it to name fields in the draft. It still carries its own cheat-asm (register asm pins + inline move at :16926-17004) but that is a SEPARATE queue item; only the struct type is being borrowed.

- [s1] [fable-blitz 2026-07-07] Sibling callers of func_8007352C in-file for idiom reference: func_8005FBC8 (:12940), func_80060414 (:13093), func_80069AE4 (:15486), func_8006A3CC (:15639), replay_camera_attack (:16482) -- several are already pure C and show how EnvA is initialized/reused across multiple draws in matched code. tmp/duplicates_leads.txt has NO entry for this function (checked).

## [s2] manual lane laneC, 2026-09-29 — first full draft, 798 -> 88 honest; 0 reached with owner-level constructs

Tools (all in memory/grind/func_8005E54C/tools/): `sbxp.py` = the engine sandbox recipe (disable=all, cheat-asm
stripped, scored against build/src/text1b.o) plus an optional TU-wide patch file, so a declaration change elsewhere
in text1b.c can be measured without touching src/; `dump.sh` = cc1 compile (+ `DUMP=1` RTL dumps via the
instrumented tools/gcc-2.7.2/cc1); `fx.py` extracts this function from each dump; `hunks.py` prints scored hunks.

Floor trail (sandbox --disable all, or sbxp.py where a TU patch is involved):
- 202 first transcription (EnvA-layout descriptor, s16 vals[2]/wins[2], R1..R9 region map from s1).
- 189 `(u32)D_8009BD38 >> n` (srl, not sra), R1 if/else arm order swapped (fallthrough = bit clear). Loop bounds
  need `(s32)(((u32)D_8009BD38 >> 10) & 3) + 3`: target compares SIGNED (slt back-edge) but shifts LOGICALLY;
  the `beqz` entry guard is combine.c simplify_comparison turning `> 0` into `!= 0` (nonzero_bits of `(x&3)+3`
  has no sign bit), so s1 hypothesis #3 (explicit `if (rounds)`) is refuted: a plain `for` produces it.
- 118 the other player's value is `*(j ? &vals[0] : &vals[1])` (pointer select, re-evaluated at each compare);
  `vals[j ? 0 : 1]` / `(j ? vals[0] : vals[1])` measure 188.
- 107/106 portrait clamp as two statements `c = D_8009BD24[..]; if (c >= 12) c -= 2;` (ternary: 118).
- 92 R6 win logic: `if (vals[j] <= other) { has_color = (vals[j] != 3) ? 1 : 0 } else { if (vals[j] != 3)
  wins[j]++; has_color = 0; }` (fallthrough = not-win); `s.y += 0x4C;` separated from `s.y += 6;` by the other
  stores, else flow.c's last_mem_set dead-store deletion (flow.c:1740) drops the first store the target keeps.
- 88 (candidate.c, honest, standard sandbox): FT4 pair spelled naturally (`s.ft4_out = ft4; ft4 = f(); s.table =
  ..[1]; s.ft4_out = ft4; ft4 = f();`) and `ft4` initialised before `mode_off` (sched1 tie order).

What the remaining 88 is (each item measured by adding ONLY that device):
1. FRAME +8: target spill slots start at sp+0x60, ours at 0x58. The target has an 8-aligned object at
   sp+0x58 = s+0x30 that NO instruction touches (every sp-relative access in the target is 0x10, 0x18/0x1A,
   0x20/0x22, 0x28..0x53, 0x60..0x88). Measured with descriptor trailing members `s32 unk2C; s16 unk30[2];`:
   92 -> 53. Siblings func_8005D814 (Env5D814 + separate `s16 digit[3]`), func_8005E098 (S5E098 `pad2C; d[2]`),
   func_8005F1C8 (S5F1C8 `pad2C; d[3]`) all have a digit array at s+0x30; here it is declared but unused.
   Not a phantom pseudo: reload's alter_reg assigns spill slots in regno order, arg0's pseudo is first (0x60),
   so the object lies in the locals area (a BLKmode local).
2. R2 flags word `D_8009BD38`: target materialises `&D_8009BD38` in a register (lui/addiu) at the loop entry and
   bottom and RE-LOADS the value in the body (`lw v0,0(a1)` after the vals stores); ours reuses the loop-test
   value (cse.c cse_around_loop / cse_set_around_loop substitute the REG_LOOP_TEST_P register). A plain VAR_DECL
   read never goes through explow.c memory_address; a COMPONENT_REF read does (constant address forced into a
   pseudo, explow.c:398), and the body MEM then no longer hashes equal to the loop test's.
3. `D_8009BD24[..]` portrait reads: target `lbu a0,%lo(D_8009BD24)(at)`; ours `la` hoisted by loop.c
   (fn.loop: "regno 474 (life 12), move-insn savings 2 moved") and spilled. A 1-D variable-index ARRAY_REF
   expands as *(&a + i*size) under EXPAND_SUM and break_out_memory_refs forces the symbol BEFORE the index
   arithmetic (life 12 -> hoisted); a nested/COMPONENT_REF access goes through get_inner_reference, whose
   variable offset is expanded with modifier 0 first (symbol forced after it, life 1, "not desirable"), and
   combine then folds `lbu sym(reg)`. Measured: `u8 D_8009BD24[2][5][2]` + `D_8009BD24[j][i][0]` (TU patch)
   44 -> 20; it also removes the D_8009B490/498 hoist cascade (the hoisted symbol had cost a register).
4. R3 totals zeroed by ONE `sw zero,0x18(sp)`. `vals[0] = vals[1] = 0` gives two `sh` (no store merging in
   GCC 2.7.2; s16[2] is BLKmode by the stor-layout.c STRICT_ALIGNMENT rule, so an initializer would call bzero).
   Only a 32-bit view writes it: `*(s32 *)vals = 0;` measured exact.
5. R4 y base: target keeps the R3 y constant (0xC6/0xC2/0xBE by mode) in $s0 and COPIES it to $s4 (`move s4,s0`)
   before the win-mark loops; $s4 is the round counter i of R1/R2/R3/R6. Reproduced only by `i = y;` then
   `s.y = i + (k & 1) * 12;` (i the s16 counter, y an s32 local). Without the copy (y used directly): +6
   (tmp probe p3b: y lands in $s5, no move); y holder as the s16 k: sign extensions appear (probe k1).

THE ZERO: memory/grind/func_8005E54C/match0/body.c with TU patch match0/tu_patch.py scores 0/799, build 799
(tools/sbxp.py, 2026-09-29). It uses: the 0x8009BD24 block as ONE struct `{ u8 chr[2][5][2]; s32 flags; }`
(flags = the word splat calls D_8009BD38, at +0x14), every flags read as `D_8009BD24.flags`, portrait reads as
`D_8009BD24.chr[j][i][0]`; `*(s32 *)vals = 0;`; `i = y;`; descriptor trailing members `unk2C`/`unk30[2]`.
Items 2+3 are one object model (a sanctioned aggregate merge if prongs (a)-(e) hold); items 1, 4, 5 each need an
owner-level decision (dead trailing frame object; word view of a local halfword pair; a bare-copy value in a
reused counter, which Ruling 11 (C)(3) refuses).

Object-model evidence for the 0x8009BD24 struct, independent of this function's bytes: func_80077904
(text1b.c:15532) passes `D_8009BD24` to func_8006E534, which reads `*(s32 *)(arg2 + 0x14) & 0xF`
(text1b.c:11308) and stores the base in D_800A3568; completed functions then read/write the flags word at
`D_800A3568 + 0x14` (text1b.c:11328, 11666, 11792, 12365, 12425, 12601, 12642-12643; 12964-12966 through the
`Cfg720FC` bitfield view), a word at +0x20 (11998) and 2-byte records at `D_800A3568 + dst` / `+ dst + 1`
(12668/12678). func_80077D00 (text1b_b.c:868-870) returns &D_8009BD24; func_80077820 (text1b.c:15479) fills it via
func_80068F70. So D_8009BD38 is member +0x14 of the D_8009BD24 object (base+offset addressing in the original
binary: func_8006E534.s) and the chr table is 2-byte records. A landing must respell every consumer (2026-09-29
run lesson 1: no byte-offset pointer arithmetic left on the merged bytes, incl. the D_800A3568-based sites):
not attempted yet.
