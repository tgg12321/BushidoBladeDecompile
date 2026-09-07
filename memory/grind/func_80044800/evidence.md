# Evidence bank — func_80044800

## s1 (2026-09-06, recon) — MATCHED: sandbox 0, verify-oracle ok

OBJECT MODEL: the DATA MODEL block flagged D_800A9CFE / D_800A9D00 / D_800A9D04 as
"decl NONE in include/*.h". MATCHES — all three are members of the aggregate
`Unk800A9CF8Header D_800A9CF8` already declared in `include/game.h:46-56`
(landed 2026-09-06 with the func_8004473C aggregate-merge handoff, Judge PASS in
docs/grind/decisions.md, 2026-09-06 04:37 entry): D_800A9CFE = `.unk6` (entry
count, loop bound), D_800A9D00 = `.unk8` (12-byte-stride keyframe table),
D_800A9D04 = `.unkC` (0x68-stride entry table), plus `.unk2` (frame limit),
`.unk4` (stage id) and `.unk10` (game_GetCharData() table) which the target
reaches through the hoisted base `$s5 = &D_800A9CF8 + 2` (`lh 0($s5)`,
`lh 2($s5)`, `lw 6($s5)`, `lw 0xE($s5)`; asm/funcs/func_80044800.s:35,126,131,134).
The struct spelling measured 0 with NO per-word symbol named in C — the
aggregate is the correct model; no declaration fix was needed. D_800A3820
(`extern s32`, code6cac.h) MATCHES: the target post-increment list append
(`lw v1; addiu v0,v1,4; sw v0; sw s2,0(v1)`) is produced exactly by
`list = (s32 *)D_800A3820; D_800A3820 = (s32)(list + 1); *list = (s32)rec;`
(same idiom as the sibling at src/text1a_c.c:1095). `Judge` is the TU-local
`extern s16 Judge[]` (src/text1a_c.c:192) used by every sibling; `Judge[idx]`
with the full `&Judge` hoisted into $fp measured 0 — MATCHES.

Chassis: HEAD ff5e4ece plus this session's src/text1a_c.c edits; -mel, -G0; no
FAKE constructs anywhere in the candidate; 0 regfix/asmfix rules (system retired).

Measurements (all `sandbox func_80044800 --disable all`, 202 target insns):
- v1 (rejected/biv-rec-separate-rec2-inline-products-score98.c): `rec++` biv
  with a separate `rec2` local and the four sin/cos products written inline in
  the `+=` expressions → 98 (208 insns). Three clusters: (a) loop.c strength
  reduction created a giv `s2 = rec + 96` and rewrote every field access as a
  negative offset from it, with rec itself in $s4; (b) the products were
  recomputed after the `rec->unk2C +=` store (GCC must reload sv[] after a
  store through `rec`, since &sv escaped to func_8004A348); (c) `&Judge` stayed
  in-loop in $a2 instead of hoisted to $fp (fallout of (a): $fp was taken by
  the giv). Diff: tmp/grind/func_80044800/s1/pairdiff_v1.txt.
- v2 (rejected/fold-reassoc-index-and-latch-order-score12.c): products into
  four fresh s32 temps (cz, sz, cx, sx; cos looked up first, then sin — target
  order cos, cz, sin, sz, cx, sx), and `rec` retargeted inside the stage-0x12
  block (`ent = rec; rec = (Rec4473C *)D_800A9CF8.unk10 + i; ...; rec = ent;`)
  → 12. Retargeting makes `rec` multi-set in the loop, so loop.c no longer
  treats it as a biv: no giv, field offsets straight off $s2, and the
  `move s0,s2` / `move s2,s0` pair around the block is exactly the target
  (delay-slot-filled at both ends). Remaining: (d) `(frame + (unk2 - 1))` was
  reassociated by fold to `(frame - 1) + unk2` (target: `addiu v1,v1,-1;
  addu v1,s3,v1`); (e) loop latch emitted `s6+=; s4++; ...; s2+=` versus the
  target `s2+=; lh; s4++; ...; s6+=`. Diff: s1/pairdiff_v2.txt.
- fold probe (tmp/grind/func_80044800/s1/fold_test.c / fold_test.s, cc1 -O2
  -mel -G0): six spellings of the index. `frame + (unk2 - 1)`,
  `unk2 - 1 + frame`, `frame + unk2 - 1`, `(unk2 - 1) + frame`, and an s16
  copy of unk2 ALL fold the constant onto `frame` (or outermost). Only a named
  local `last = D_800A9CF8.unk2 - 1; ... (frame + last) * 12` keeps `unk2 - 1`
  first and gives the `addu frame,last` operand order of the target.
- v3 (candidate.c): v2 + `last` temp + for-latch order `rec++, i++` (the same
  order as the matched sibling func_8004473C uses, `dst++, src++, i++`) → 0,
  202/202. verify-oracle --rebuild --allow-dirty: ok=true, build_sha1
  62efab4f73f992798c43e8c730aa43baa10bb4fa == expected. The final rename of
  the save pointer (`saved` → `ent`) was re-measured at 0.

Side edit (byte-neutral, same TU): the shared `Rec4473C` typedef's trailing
`s8 pad60[8]` became `s16 unk60; s8 pad62[6];` (the fade/scale field this
function reads and writes). func_8004473C does not touch that field; the full
oracle rebuild proves neutrality.

Housekeeping for the operator (outside this session's write scope): the seven
alias rows in undefined_syms_auto.txt:302-308 / named_syms.txt carry the
suffix "retire with func_80044800" from the func_8004473C handoff. With this
function in C nothing references the per-word names any more, so those rows
can be retired.

## s1 re-run (2026-09-06, recon, after the 21:12 layer-1 FAIL) -- chassis re-measured 0; alternatives measured

OBJECT MODEL (re-banked for this session): the DATA MODEL block again flags
D_800A9CFE / D_800A9D00 / D_800A9D04 as "decl NONE in include/*.h". MATCHES --
they are `.unk6` / `.unk8` / `.unkC` of `Unk800A9CF8Header D_800A9CF8`
(include/game.h:46-56); candidate.c names no per-word symbol and measures 0.
D_800A3820 (`extern s32`, code6cac.h) MATCHES (list-append idiom, measured 0
inside the candidate). `Judge` (TU-local `extern s16 Judge[]`,
src/text1a_c.c:192) MATCHES. No declaration fix exists to make; the flagged
signals are stale splat alias rows (undefined_syms_auto.txt / named_syms.txt
"retire with func_80044800"), not a model mismatch.

Chassis: HEAD 7d3c722f (the layer-1 FAIL bank commit; src/text1a_c.c is back to
INCLUDE_ASM). candidate.c applied with tmp/grind/func_80044800/s1/apply.py:
canonical = C (distance 0), `sandbox --disable all` = 0, 202/202. Floor 0 is
chassis-current. src restored to HEAD before finishing (driver reverts anyway).

Layer-1 FAIL (docs/grind/decisions.md 2026-09-06 21:12) and the two resulting
bans target the `ent = rec; rec = (Rec4473C *)D_800A9CF8.unk10 + i; ...;
rec = ent;` block. The reviewer's proposed next action ("a distinctly-named
local ... that never touches `rec`") was measured this session:

- Variant A (rejected/distinct-ent-local-rec-biv-giv-score70.c): identical to
  candidate.c except the block uses a distinct `ent` and `rec` is never
  reassigned -> 70 (204 insns). rec single-set -> pointer biv -> giv `rec+96`
  (`addiu s2,s4,96`, `lw s3,-8(s2)`), $fp consumed, `&Judge` not hoisted;
  the block's stores go through $s0 with a separate base computation
  (`lw s0,14(s6); addu s0,s0,s7; sh v0,16(s0)`). Diff: s1/r2_pairdiff_varA.txt.
- Variant B (rejected/indexed-rec-distinct-ent-score48.c): `rec = table + i`
  at loop top, distinct `ent` -> 48 (202 insns). rec becomes a giv of i:
  `i*0x68` reduced into $s5, rec recomputed each iteration from a reloaded
  `.unkC`; no latch `addiu s2,s2,0x68` biv. Diff: s1/r2_pairdiff_varB.txt.

BYTE EVIDENCE that the retarget/restore is the original's own shape (read from
asm/funcs/func_80044800.s, line numbers of the .s file):
- line 118 `addu $s0, $s2, $zero` sits in the delay slot of the
  `bne $v1, $v0, .L80044AC4` stage test (line 117): `ent = rec` is the first
  statement of the stage-0x12 block (a fall-through-block insn hoisted into the
  delay slot; $s0 is dead on the taken path).
- line 121 `addu $s2, $v0, $s6`: the char-data record address is written into
  $s2 -- rec's own register -- while rec's old value is held in $s0. Every store
  and the func_800417D0 argument inside the block use $s2 (lines 141-164).
- line 166 `addu $s2, $s0, $zero` in the delay slot of the `j .L80044AC4`
  that leaves the block: `rec = ent` is the block's last statement, restoring
  rec for the list append (`sw $s2, 0($v1)`, line 191) and the latch
  (`addiu $s2, $s2, 0x68`, line 193).
- $s6 is `i * 0x68` (init 0 at line 24, `addiu $s6, $s6, 0x68` in the latch
  delay slot, line 199): the strength-reduced form of `(Rec4473C *)unk10 + i`.
A distinct second local (variant A) puts the char-data record in a register of
its own; nothing in ordinary C except assigning the sibling record to `rec`
itself makes the block's accesses go through $s2 while rec's value survives in
$s0. The retarget is therefore not a lever chosen to defeat loop.c -- it is the
only pointer flow consistent with the bytes; the loop.c non-biv consequence is
a side effect of the original's own spelling.

## s1 (2026-09-06, recon, post Judge PASS 21:23) -- cleared body re-applied, floor 0 re-measured

OBJECT MODEL (re-banked): the DATA MODEL block again flags D_800A9CFE /
D_800A9D00 / D_800A9D04 as "decl NONE in include/*.h". MATCHES -- they are
`.unk6` / `.unk8` / `.unkC` of `Unk800A9CF8Header D_800A9CF8`
(include/game.h:46-56); the candidate names no per-word symbol and measures 0.
D_800A3820 (`extern s32`, code6cac.h) MATCHES (list-append idiom inside the
0-scoring body). `Judge` (TU-local `extern s16 Judge[]`, src/text1a_c.c:192)
MATCHES. No declaration fix exists; the flagged signals are the stale splat
alias rows in undefined_syms_auto.txt / named_syms.txt marked "retire with
func_80044800".

Chassis: HEAD 4287b19d (the Judge-ruling commit; src/text1a_c.c INCLUDE_ASM).
candidate.c applied with tmp/grind/func_80044800/s1/apply.py (also the
Rec4473C pad -> `s16 unk60; s8 pad62[6]` rename). canonical = C (distance 0,
202 insns); `sandbox func_80044800 --disable all` = 0, 202/202, rules_dropped 0.
Body hash of the applied src body and of candidate.c both = 52ecd6d8e091f34b
(tmp/grind/func_80044800/s1/hash.py via grindlib.body_hash), identical to the
JUDGE CLEARANCES entry, so layer-1 is skipped and the body was NOT respelled.
self_vet.md rewritten per the Judge constraint: primary ground = target bytes
800449C8 / 800449D4 / 80044A88 / 80044ADC (asm lines 118/121/166/191),
secondary = ordinary-c-judge-decidable.md:51 Ruling 1 step 3; no frozen-list
family claimed, no FAKE annotation. Edits left in place in src/ for the
driver's bytes verification.
