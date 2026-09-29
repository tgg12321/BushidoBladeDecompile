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

### [s2 later, 2026-09-29] refined model — no merge needed: record table + bitfield flag word

match0/ now holds the refined zero (supersedes the struct-merge version committed earlier today):
match0/body.c + match0/tu_patch.py score 0/799 (tools/sbxp.py, with and without cheat-asm stripping), and a
whole-TU object comparison (`tools/objdiff.py` of the patched build vs build/src/text1b.o) shows 456/457 functions
identical; func_8005E54C differs only in relocation addends (`D_8009B490+8` vs `D_8009B498`, `D_8009B398+24` vs
`D_8009B3B0`, link-equivalent). The TU patch changes two declarations and respells their text1b.c consumers:
- D_8009BD24 (text1b.c:5058 `extern u8 D_8009BD24[];`) -> `Unk8009BD24Record D_8009BD24[2][5]`, a 2-byte record
  `{ u8 chr; u8 unk1; }` per player per round; consumers: func_80060414 `D_8009BD24[0][0].chr`, func_80077904's
  call `(u8 *)D_8009BD24`. The 20-byte table ends exactly at 0x8009BD38.
- D_8009BD38 (text1b.c:3272 `extern s32 D_8009BD38;`) -> a u32 bitfield word `Unk8009BD38Flags` (fields at bits
  0,4,10,12,14,15,17,18, named by bit offset). Consumers respelled: func_8005F1C8 (`.unk14 + 1`, `.unk12 == 2`
  x6), the text1b.c:5554 function (`.unk0`), func_80077894 (`D_8009BD38.unk0 = result;` replaces the `s32 *p`
  read-modify-write and reproduces its `la`-form RMW exactly), func_80077904 (`.unk0 * 2`).
Why bitfields: every cast the s32 spelling needed is what an `unsigned : n` field gives for free — the target
shifts LOGICALLY (srl) but compares SIGNED (slt) and shifts the extracted value arithmetically (`srav`), i.e. a
2-bit unsigned field promoted to int; `(w & 0xC00) == 0x800` is fold-const.c optimize_bit_field_compare of
`field == 2`; and a COMPONENT_REF read forces the constant address into a pseudo (explow.c:398), which is what
makes R2's body reload the word (evidence [s2] item 2). The +0x14 word is already modelled as bitfields in-tree
(`Cfg720FC`, text1b.c:12721-12726, func_800720FC's `unk14_4` store).
The "mode" tests are the same 2-bit field as the round count: unk10 = rounds - 3, and the y layout is chosen by
it (5 rounds: i*24 + 0x44; 4 rounds: i*24 + 0x4F; 3 rounds: i*34 + 0x4F).

Single-device ablations on match0 (each device removed alone; tools/sbxp.py + match0/tu_patch.py):
- without `i = y;` (y used directly in R4): 6
- without `*(s32 *)vals = 0;` (`vals[0] = vals[1] = 0;`): 2
- without the descriptor trailing members `unk2C` / `unk30[2]`: 47
- without all three: 55 = honest/body.c (the best body with only ordinary constructs, under the tu_patch
  declarations). candidate.c (88) is the best body under main's current declarations.
So the declaration model closes every source-level hunk; the three devices are each individually required and
each needs an owner-level decision (see hypotheses [s2 OPEN]).

## [s3] laneC 2026-09-29 — after owner batch 20 (Q33-Q36): the copy is gone; two granted devices remain

Record of the owner answers: tmp/orch/owner_rulings_2026-09-29_b20.md (Q33 union, Q34 copy, Q35 trailing array,
Q36 cast store). Measurements below: tools/sbxp.py + match0/tu_patch.py, `strip` = engine sandbox recipe,
`nostrip` = the same build without cheat-asm stripping (the sandbox strips an unused local array unless
engine/volatile_cheats.py `_SANCTIONED_UNWRITTEN_PADS` carries a row, so the trailing array reads 47 stripped).
Probe bodies: probes/*.c.

- **Q34 no longer needed.** Declaring the R3 y local `s16` (like every other small local here) and using it
  directly in R4 (`s.y = y + (k & 1) * 12;`) scores 0 (match0/body.c). The `move s4,s0` is not a source copy:
  it is the loop-invariant `(s32)y` of the s16 local hoisted out of the win-mark loops into its own pseudo.
  With `s32 y` and no copy: 6 (probes/nocopy.c; greg: y's pseudo 84 then conflicts with k (82, $s0) and lands in
  $s5). The `i = y;` copy form (probes/d1.c) also scores 0 but is now strictly worse (one more no-purpose
  construct). Other probes, all with the other devices in place: every value of the shared `i` split into its
  own local 77 (probes/sp_all.c); split alone: R1 counter 8, R2 24, R3 11, R6 61, R4 base as a fresh copy local
  0 (probes/sp_V*.c); y held in i with the totals counted by k 14 (st_iy); totals counted by j 11 (st_jsum).
  The loop counters `i`/`j`/`k` are reused across phases. Precedents: the owner ruled on func_8003800C's single
  counter reused for two loops (docs/grind/decisions.md:11395-11406, 2026-08-25; admitted under the frozen
  "Variable reuse for codegen control" family, FAKE-annotated at the declaration). func_8005F1C8 landed with k/j
  reused across phases and an explanatory comment at the declarations (memory/grind/func_8005F1C8/evidence.md,
  LANDED section: layer-2 accepted the counter reuse). Splitting the counters here measures 8-77 (above). The
  landing should carry a declaration comment like func_8005F1C8's; whether the FAKE annotation is also required is
  a question for review.
- **Q33 union fails; Q36 cast store needed.** `union { s16 v[2]; s32 word; } vals;` + `vals.word = 0;` measures
  197 (probes/u2.c, frame 216): a 4-byte, 4-aligned union is SImode (stor-layout.c), expand_decl keeps it in a
  pseudo, and put_var_into_stack moves it to the stack only when `vals.v[j]` makes it addressable, after
  wins and s have taken 0x18/0x20. `volatile union` 146 (u4); an 8-byte `s16 v[4]` union 0 (u3; fake size:
  the target touches only 0x18..0x1B). The target's sp+0x18 object was stack-allocated at declaration (it
  precedes wins and s), i.e. BLKmode, i.e. an s16 array. Q36 site: the only word access to 0x18..0x1B is
  0x8005EA44 `sw $zero,0x18($sp)` (asm/funcs/func_8005E54C.s:347), covering exactly `s16 vals[2]`; all other
  accesses to those bytes are halfword (sh/lh/lhu at 0x18/0x1A, or lh 0(base+j*2)).
- **Q35 trailing array: frame forensics.** Every sp-relative operand in the target
  (asm/funcs/func_8005E54C.s): 0x10 (5th arg), 0x18/0x1A (vals), 0x20/0x22 (wins), 0x28..0x53 (s, including
  0x48/0x4C scale fields and 0x50..0x53 colour bytes), 0x60..0x88 (six spill slots), 0x90..0xB4 (saves). The
  only address formations are sp+0x18 and sp+0x1A (vals, and wins via +8), and sp+0x28 (&s, passed to
  func_8007352C/func_80073728, which read fields 0x00..0x2B = sp+0x28..0x53). Nothing reaches 0x54..0x5F.
  cc1 `.frame`: with `s16 digit[3];` after s, vars=120 and the sp-offset census equals the target's exactly;
  without it, vars=112 and every spill slot shifts by -8. The object cannot be a phantom pseudo: reload's
  alter_reg assigns spill slots in regno order and the parameters' pseudos come first (arg0's slot is 0x60 in
  both target and build), so any unallocated pseudo lands at or above 0x60, never at 0x58.
  Siblings (COMPLETED-C, same file): func_8005D814 declares `Env5D814 s; s16 digit[3];` (src/text1b.c:4206-4207),
  s at sp+0x18 and digit at sp+0x48 = s+0x30 (its target accesses 0x48/0x4A/0x4C). func_8005E098 (S5E098 `s16
  d[2]` at +0x30, text1b.c:4409) and func_8005F1C8 (S5F1C8 `s16 d[3]` at +0x30, text1b.c:4559) keep their digit
  arrays at the same s+0x30 (sp+0x48 in their frames), modelled as trailing struct members. Here s is at sp+0x28,
  so s+0x30 = sp+0x58: an `s16 [3]` at exactly the untouched slot. match0/body.c declares `s16 digit[3];`
  right after `Env5E54C s;` (plain; `volatile s16 digit[3]` also measures 0 unstripped). Name and qualifier to
  follow the Q35 rule text.
- The three per-arm `tile->x0 = 0x5E;` stores are required: hoisting one `tile->x0` store above the mode
  if/else measures 10 (probes/x0h.c). Each arm sets the full (x0, y0) position; if review treats it as the
  duplicated-store family, it needs that family's annotation.

Current best (match0/body.c + match0/tu_patch.py): 0/799, whole-TU objdiff 456/457 identical (func_8005E54C
differs only in link-equivalent relocation addends). No-purpose constructs: `*(s32 *)vals = 0;` (Q36) and
`s16 digit[3];` (Q35).

## [s4] laneC 2026-09-29 — evidence packages for the Q36 cast store and the Q35 trailing array

The rule texts (pending `rules:` commit, no-new-park-categories.md: § "Amendment: one cast store on a local array",
§ "Extension: a trailing unused local array with sibling evidence") set the prongs. Probes are in probes/
(generated by tools/mkprobes.py from the plain-`digit` match0 body of commit d54803192; driver tools/v20.sh).
Scores: tools/sbxp.py + match0/tu_patch.py. `strip` is the engine sandbox recipe; `nostrip` is the same build
without cheat-asm stripping. `.frame` is cc1's `vars=` line (tools/dump.sh with tools/applytp.py).

match0/body.c is now the landing form, `volatile s16 digit[3];` right after `Env5E54C s;`.

| probe | strip | nostrip | vars |
|---|---|---|---|
| fin (volatile digit[3]) | 53 | 0 | 120 |
| fin_plain (digit[3]) | 47 | 0 | 120 |
| fin_nodigit | 47 | 47 | 112 |
| fin_union (Q33 union spelling of vals) | 228 | 197 | 152 |
| loc_wins_after (wins declared after s) | 147 | 147 | 112 |
| loc_vals_after (vals declared after s) | 211 | 211 | 112 |
| prod1_guard (R4 k loop as a guarded do-while reusing its exit test) | 64 | 64 | 112 |
| prod2_narrow (portrait char re-read as `(s16)c`) | 47 | 47 | 112 |
| prod3_named (R7 `s16 w = wins[j];` read twice) | 65 | 65 | 112 |

The strip column for fin/fin_plain is the sandbox stripping the unused array: no
`_SANCTIONED_UNWRITTEN_PADS` row exists yet. The volatile and plain declarations compile to the same object:
`sha1sum` of both text1b.o is 44053e1c799c3683312e1ac869782fe794ffd97d, objdiff shows 457/457 functions
identical, and the cc1 listings are identical.

Q36 (cast store): the one word access to vals' bytes is 0x8005EA44 `sw $zero,0x18($sp)`
(asm/funcs/func_8005E54C.s:347). It covers exactly `s16 vals[2]` (sp+0x18..0x1B). Every other access to those
bytes is a halfword. The union spelling was measured first and misses (fin_union 197 unstripped; frame vars 152
vs 120). Mechanism: a 4-byte, 4-aligned union is SImode (stor-layout.c). expand_decl gives it a pseudo, and it
reaches the stack only through put_var_into_stack once it becomes addressable, after wins and s have their slots.

Q35 (trailing array):
- (1) Census: frame_census.txt (tools/census.py). There are zero memory operands in 0x54..0x5F. The address
  formations are sp+0x18, 0x1A, 0x28 and the prologue/epilogue sp adjust. The callees read &s only at
  0x00..0x2B (func_8007352C via $s4, func_80073728 via $s2).
- The region is locals-area: the spill slots start at 0x60 with arg0, since reload assigns them in regno order.
  With the array: vars 120 and the build's `$sp` offset census equals the target's. Without it: vars 112.
- (2) Siblings (COMPLETED-C, src/text1b.c), each with the same 0x2C descriptor passed to func_8007352C and a
  real s16 array at descriptor+0x30 (sp+0x48 in their frames, s at sp+0x18):
  - func_8005D814: `Env5D814 s; s16 digit[3];` at 4206-4207, used at 4261 `digit[i] = *arg0;`.
  - func_8005F1C8: S5F1C8 `s16 d[3];` at 4559, used at 4676 `s.d[k] = arg0[2];`.
  - func_8005E098: S5E098 `s16 d[2];` at 4409, used at 4471.
- (3)/(4)/(5) The declaration copies func_8005D814's `s16 digit[3]` (type, count, identifier) as a separate
  local immediately after `Env5E54C s;`, with `volatile` added. 6 bytes rounded to the 8-byte slot is exactly
  0x58..0x5F.
- (7) Every honest producer and real local tried leaves vars at 112 (table above).

### [s4 cont.] the `vals` pair holds both round points and totals: per-value spellings measured

`s16 vals[2]` (sp+0x18) holds per-round points in R2 and R6 (two bits of arg0 per player per round) and
the per-player totals in R3/R4 (zeroed by the Q36 store, then summed). Review may treat this as a reused
local. Per-value spellings, with totals in their own `s16 total[2]` and everything else unchanged (probes/pv_*.c,
tools/mkpv.py; strip / nostrip):
- total declared after digit: 17 / 54.
- total declared first: 165 / 204.
- total declared between wins and s: 107 / 148.
- total block-scoped around R3/R4: 13 / 54 (stripped, i.e. without digit, the block's total takes the free slot
  at sp+0x58, so R3/R4 address 0x58 where the target uses 0x18).

The mechanism is frame layout, not register allocation. Locals get stack slots from stmt.c expand_decl ->
function.c assign_stack_temp(keep=1), in declaration order, and a slot is reused only after its block ends.
The target's R2, R3/R4 and R6 all address sp+0x18, which precedes wins (0x20) and s (0x28). So that slot
belongs to the first function-scope local and is live across all three regions, and a second array can never
be given it. Honest name for the pair, true of every write: points per player (round points, then total
points). A landing could rename `vals` to `points`.

## [s5] laneC 2026-09-29 — landing prepared under rules 5f69df611: prong records, measured on the landed body

The landed body is func_8005E54C's definition as spliced into src/text1b.c by tools/land.py
(final_probes/landed.c is a byte copy of it). All probes below differ from it only as named. They are generated
by tools/mkfinal.py from the spliced src and scored with the ENGINE sandbox
(`pwsh tools/sandbox_sweep.ps1 -Func func_8005E54C -Variants final_probes/<p>.c`, i.e.
`sandbox --disable all --candidate`). cc1 `.frame` and listing hashes come from tools/frames.sh (it prints its cpp
and cc1 command lines). Results are banked verbatim in final_probes/SCORES.txt.

| probe | sandbox --disable all | .frame vars |
|---|---|---|
| landed | 0 (799/799) | 120 |
| plain_digit (no `volatile`) | 0 | 120 |
| no_digit | 47 | 112 |
| union (Q33 spelling of `points`, same body) | 197 | 152 |
| loc_wins_after_s | 147 | 112 |
| loc_points_after_s | 211 | 112 |
| prod1_loop_guard (phantom producer 1) | 64 | 112 |
| prod2_himode_second_use (phantom producer 2) | 47 | 112 |
| prod3_named_local (phantom producer 3) | 65 | 112 |

Target: frame 0xB8, vars 120.

- Listing hashes: the landed and plain_digit cc1 listings are identical (sha1 prefix 6fdf965a27f4). So
  `volatile` does no codegen work (Q35 prong 4). prod2's listing equals no_digit's (f9bce23647e8), so the narrow
  second use folds away and the producer is inert.
- Full rebuild with the landing edits: build SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle.
- `sandbox func_8005E54C --disable all --diff` on the spliced src: 0, 0 source-level / 0 operand-only /
  63 not-scored hunks.
- `engine test`: 862 passed.

Q36 prongs (one cast store on a local array):
1. `s16 points[2]` is a local array, 4 bytes, s16 elements, with consumed element reads elsewhere (e.g.
   `points[0] == 3`, the `k < points[j]` loop bounds).
2. The target has one `sw $zero,0x18($sp)` at 0x8005EA44 (asm/funcs/func_8005E54C.s:347). It sits at the frame
   offset of `points` (sp+0x18, see frame_census.txt) and covers its 4 bytes. The build emits it (sandbox 0).
3. The body has exactly one cast statement, a store. Every other access is `points[i]`; the ternary takes
   `&points[0]` / `&points[1]` without a cast.
4. The union spelling on the same body measures 197 (final_probes/union.c).
5. The annotation is at the statement.

Q35 prongs (trailing unused array):
1. frame_census.txt: no memory operand reaches 0x54..0x5F. The region is 0x58..0x5F, 8 bytes. It is in the
   locals area: spill slots start at 0x60 with arg0, and alter_reg assigns them in regno order.
2. Siblings in src/text1b.c:
   - func_8005D814, `Env5D814 s; s16 digit[3];` (4230-4231), a separate local right after the same 0x2C
     descriptor. Real: `digit[i] = *arg0;` at 4285. This is the one copied.
   - func_8005F1C8, S5F1C8 trailing member `s16 d[3];` (4911) at descriptor + 0x30. Real: `s.d[k] = arg0[2];`
     at 5028. Its leading 0x2C members have the same layout and go to the same callee func_8007352C.
   - Both siblings' arrays sit at sp+0x48 = descriptor + 0x30 in their frames (s at sp+0x18).
   - Corroboration only: func_8005E098 `s16 d[2];` (4433, used at 4495).
3. Copies `s16 digit[3]`, declared immediately after `Env5E54C s;`. 6 bytes rounds to the 8-byte slot, which is
   exactly 0x58..0x5F. With it the frame and every `$sp` offset equal the target's (vars 120; sandbox 0).
4. `volatile s16 digit[3];`: no initializer, never referenced. Byte-identity without volatile is shown above.
5. The name `digit` is func_8005D814's identifier.
6. FAKE annotation at the declaration.
7. Measured above: no_digit, the three producers, and the two real locals that could move into the region
   (wins, points). All stay at vars 112.
8. engine/volatile_cheats.py `_SANCTIONED_UNWRITTEN_PADS` gets the row `"func_8005E54C": {("digit", 3)}`;
   `engine test` is green.

What each declaration correction is worth (tools/worth.py, the landing body with each correction's spelling
undone; tools/sbxp.py unstripped): both 0, record table only 14, flag word only 24, neither 38. Both are needed.

func_80077D00 (text1b_b.c) is NOT edited: the declarations stay TU-local in text1b.c (as on main), so no new
`(s32 *)` return cast. The one call that passed the decayed u8 array now passes `&D_8009BD24[0][0].chr` (u8 *),
with no cast. See consumers.md.
