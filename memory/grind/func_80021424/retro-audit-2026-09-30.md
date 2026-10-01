# func_80021424 — retro-audit CONCERN (owner Q49), analysis 2026-09-30 (laneG) — open, src blocked (laneE)

Archived landing ledger: memory/grind/_completed/func_80021424/. Concern (retro-audit 2026-09-29 batch_02,
23152ce9e): include/code6cac.h declares `extern s32 D_801027B0[][5];` beside the per-word handles
`D_801027B4`, `D_801027B8`, `D_801027BC[][5]`, `D_801027C0`, `D_801027D4` (same storage: e.g. D_801027B0[0][3]
== D_801027BC[0][0]), and `Tbl800A3860Entry *D_800A3860[]` beside `s32 D_800A3864` (== D_800A3860[1]):
a partial aggregate merge without prong (c) (one C handle per storage location). Minor: `const u32
D_80010428[1] = {0}` names an unowned tail word with no ownership evidence.

Consumer census (2026-09-30 grep) that a complete merge must respell (each byte-neutral, sandbox 0):
- code6cac_tu2.c: :2877 `D_800A3864 = ...` (func_80020D70) -> `D_800A3860[1]`; :2886/:2889 `D_801027C0`,
  `D_801027D4`; :3143-3169 `*(s32 *)((u8 *)&D_800A3860 + off)` / `((u8 *)&D_801027B0 + off)` casts;
  :3193/:3196 `(&D_801027B4)[idx]`, `(&D_801027B8)[idx]`; :3742/:3746 `D_801027BC[idx][0]`.
- code6cac_c2.c:959-960 `(u32 *)D_801027C0`, `(u32 *)D_801027D4`; code6cac_c_mid.c:1521-1522 (already
  `D_801027B0[t][k]`).
Proposed: one declaration each (`D_801027B0[][5]`, `D_800A3860[]`), every consumer through elements, per-word
externs and splat rows retired (or suffixed "retire with <asm sibling>" where an INCLUDE_ASM .s still names
them), and ownership evidence or removal for D_80010428. Not measured yet; waits for laneE (code6cac_tu2.c).

Consumer-to-function map (2026-09-30): :2877 func_80020D70; :2886/:2889 func_80021210; :3143/:3154/:3162/:3169
func_80021904 / func_80021974 / func_800219E4 / func_80021A3C (all four also read the 0x80101EC8 player records
through `(u8 *)&D_80101F12 + a0 * 1100`-style per-use puns, so a clean respell of them is its own cleanup);
:3193/:3196 func_80021A98; :3742/:3746 func_80022F34. Size: ~8 functions in code6cac_tu2.c + 2 in other files —
a dedicated cleanup session, each function sandbox-0 and one layer-2 over the set.

## Measured respellings (2026-09-30, sandbox --disable all; bodies in probes-2026-09-30/*.m.c, gen.py)
Every consumer respelled through the one declaration, each identical to its landed score 0:
func_80020D70 `D_800A3860[1] = (Tbl800A3860Entry *)0x80190800;` 0/27; func_80021210 `D_801027B0[0][4]` /
`D_801027B0[1][4]` 0/28; func_8003CF84 (code6cac_c2.c) `(u32 *)D_801027B0[0][4]` / `[1][4]` 0/208;
func_80021904 / func_80021974 `return D_801027B0[v1][0] + D_800A3860[v1]->f4E[v0] * 2;` 0/28 each (the
a0_2/v1_2 byte-offset intermediates and both casts gone; keeping a `base`/`tbl` statement order scores 7);
func_80021A98 `D_801027B0[a3][1]` / `[a3][2]` with the `idx = a3 * 5` stride local dropped 0/158;
func_80022F34 `D_801027B0[idx][3]` 0/70. func_8003993C already reads `D_801027B0[temp][1/2]`.
Still open: func_800219E4 / func_80021A3C read the record at +0x16 and +0x18 + a1*2 through `(u8 *)&D_800A3860
+ ...` casts; they need Tbl800A3860Entry members there (`u16 f16; u16 f18[...]`, the f18 length to be
evidenced) — not measured (header change). Then: drop the six per-word externs from include/code6cac.h;
undefined_syms_auto.txt rows D_801027B4/B8/BC/C0 stay suffixed `/* alias of D_801027B0+N; retire with
func_80020E74 */` (INCLUDE_ASM referrer), rows D_801027D4 / D_800A3864 retire (only C functions' .s name
them); D_80010428 tail word: evidence or removal.

## Follow-up debt (recorded 2026-09-30 at the orchestrator's direction; not landed this run)
What remains before the merge can land as one cheat-cleanup:
1. func_800219E4 reads the record at +0x16 (u16) and func_80021A3C at +0x18 + a1 * 2 (u16), both through
   `*(s32 *)((u8 *)&D_800A3860 + ...)` casts. Tbl800A3860Entry (include/code6cac.h) covers 0x16..0x4D with
   `u8 pad16[0x4E - 0x16]`, so it needs `u16 f16;` and a `u16 f18[N]` member. N is the open question: the
   reachable range of a1 at every func_80021A3C call site (and any other reader of +0x18..) must be evidenced;
   N = 27 only fills the gap to 0x4E, which is not evidence. Then respell both as `D_800A3860[v0]->f16` /
   `->f18[a1]` and measure (expected byte-neutral, not yet measured).
2. Apply the measured respellings (probes-2026-09-30/*.m.c) to func_80020D70, func_80021210, func_80021904,
   func_80021974, func_80021A98, func_80022F34 (code6cac_tu2.c) and func_8003CF84 (code6cac_c2.c).
3. Drop the six per-word externs (D_801027B4/B8/BC/C0/D4, D_800A3864) from include/code6cac.h; suffix the
   undefined_syms_auto.txt rows D_801027B4/B8/BC/C0 `/* alias of D_801027B0+N; retire with func_80020E74 */`
   (func_80020E74 is INCLUDE_ASM and names them); retire the D_801027D4 and D_800A3864 rows.
4. `const u32 D_80010428[1] = {0}` (unowned tail word): find ownership evidence or remove it.
5. verify-oracle --rebuild, sandbox 0 for every touched function, one layer-2 over the set.

## Evidence for the three open points (2026-09-30, laneG; orchestrator asked to settle the debt)
1. **f18 length N = 27.** func_80021A3C's only caller is func_8001EEB4 (code6cac_tu2.c, `func_80021A3C(D_800A3748,
   *(s16 *)(entry + 0xA))`), i.e. a1 = PracticeMenuRec.unk_0A, the character class. Its only writer in the
   binary is func_80022580's `p->unk_0A = D_8008D538[arg2];` (the INCLUDE_ASM .s files naming D_80101ED2 only
   read it: func_8001CE60.s:373, func_80023F08.s:2926). D_8008D538 (asm/data/7D920.data.s:305-342) holds the
   values 0x00..0x1A only, so a1 is 0..26; the other per-class tables are sized [27] (D_8008DE34[27][6],
   D_8008DF78[27][6], include/code6cac.h:193-194 as of the landing commit). 27 halfwords from +0x18 end at exactly +0x4E, where f4E
   begins: `u16 f18[27]` covers the gap with no filler. func_80021A3C reads it with `lhu 0x18($a1)`
   (asm/funcs/func_80021A3C.s:19).
2. **f16 = u16 at +0x16.** func_800219E4 `lhu $v0,0x16($v0)` (asm/funcs/func_800219E4.s:18); like f18 its
   value indexes the D_80102760 halfword table. Readers of the record pointers in the whole binary
   (asm/funcs naming D_800A3860/64): func_80020D70, func_80020E74 (header words +0x3/+0x4..+0x10, inside
   pad00), func_800213A0 (f14), func_80021424, func_80021904/974 (f4E), func_800219E4 (f16), func_80021A3C (f18).
   func_800213A0 also re-viewed the record as `s16 *` (`v[0x14 / 2]`); respelled `->f14`: 0/33.
3. **D_80010428 is data, placed by the bytes; D_800100E0 is redundant.** tailword.py builds code6cac_tu2.c
   with the engine's exact per-file recipe: deleting `D_800100E0` leaves .rodata (0x388, sha1 c0d15a980773,
   == build/src/code6cac_tu2.o) and .text identical, with the same relocation count — it is the pad
   `.align 3` regenerates before func_8001DA8C's table (jtbl_800100E4), so the explicit word is a
   hand-written duplicate. Deleting `D_80010428` shrinks .rodata to 0x384 (code6cac_b.o's table would land
   at 0x80010428), so that word is real: Sony's PSYLINK does not pad object ends (rodata-align evidence
   section 1, t1: object C follows B's last word directly), and code6cac_b.o's first table at 0x8001042C
   fixes that object's start under object-relative alignment. refs.py: no instruction in the binary
   references 0x80010428 (the referenced rodata items in 0x800100A4..0x8001042F are D_800100A4,
   jtbl_800100C4, jtbl_800100E4, jtbl_80010414, jtbl_8001042C). So its existence and position are proven
   but its original declaration is not recoverable; `const u32 D_80010428 = 0;` (scalar) is byte-identical
   to the `[1]` array form.

Landing script: probes-2026-09-30/land.py (copy of tmp/func_80021424/land.py; `--dry` checks every
anchor without writing; dry-run passes 2026-09-30). Still measured only at landing (header change):
func_800219E4 / func_80021A3C through f16 / f18 (`.m.c`), and whether the sandbox needs the D_801027D4 /
D_800A3864 rows to resolve func_80021210 / func_8003CF84 / func_80020D70's targets (`--rows retire` first;
`--rows keep` only if it does).

## Layer-2 round 1 (rev-21424, 2026-09-30): FAIL on one ground, fixed in the same landing
Ground: func_80021904/974/9E4/A3C read `*(s16 *)((u8 *)&D_80101F12 + a0 * 1100)` (and D_80101F4C/F4E), mid-record
per-word handles walked with a magic stride over g_practice_menu_table — the overlapping-handle class Q49 fixes.
Fix (land2.py): PracticeMenuRec gains `s16 unk_4A` (+0x4A; lh in func_800213A0 0x800213C4, func_80021904/974
`lh %lo(D_80101F12)`) and `s16 unk_86` (+0x86; lh/sh in func_800213A0 0x800213A0/0x80021418, lh in func_80021904,
sh in func_800218C8); func_800218C8 / func_80021904 / func_80021974 / func_800219E4 / func_80021A3C index
`g_practice_menu_table[a0].member`; func_800213A0 takes a `PracticeMenuRec *` (its caller in func_80021424
passes `(PracticeMenuRec *)rec`); the C externs D_80101F12/F4C/F4E are gone; D_80101F12's row stays as an alias
to retire with func_80023F08 (INCLUDE_ASM), F4C/F4E rows retire. All twelve bodies sandbox 0, full rebuild
== oracle. Not in this landing (same class, other functions): the remaining mid-record externs D_80101F04,
D_80101F08, D_80101F10, D_80101F14, D_80101F42, D_80101F5E (include/code6cac.h) and func_80021424's own
`*(s16 *)(rec + 0x4A)`-style reads through its `u8 *rec` parameter.

## 2026-09-30 late: scope widened to the full PracticeMenuRec handle cleanup (orchestrator), not landed yet
Round-2 set (round2.patch) was staged; then func_80021424 itself respelled through `PracticeMenuRec *rec`
(func_80021424.r3.c, land3.py: + `s16 unk_4C` lh 0x80021850, `s16 unk_78` sh x3) measured byte-neutral (sandbox 0
for func_80021424 / func_8001FBE8 / func_800213A0, oracle). Its in-TU caller func_8001FBE8 passes a `u8 *rec`,
so the orchestrator chose to retype func_8001FBE8's rec too and retire all six remaining mid-record externs
(D_80101F04/F08/F10/F14/F42/F5E; C users func_8001C8DC, func_8001CE60, func_8001E404, func_8001EFA0,
func_8001FBE8, func_80029454, func_8003993C) in the same landing. The src edits were reverted from main to let
laneH land first; round3_full.patch (round 2 + land3) is the work to rebase.

## PracticeMenuRec cleanup plan (orchestrator-approved staged landings) + census
Goal: no record-pointer byte-offset casts and no per-word handles over g_practice_menu_table (0x80101EC8, two
0x44C records, up to 0x80102760) left in C. Landings, one review each: L1 (this concern: 13 bodies), L2
(func_8001C8DC / CE60 / E404 / EFA0), L3 func_8003993C, L4 func_80029454; D_80101F5E / D_801023AA retire with
their last C user. Census tool: probes-2026-09-30/census_all.py (src/*.c + include/*.h).
- Baseline (main at efa946124, 2026-09-30): per-word handles 102 uses / 33 distinct in 26 functions; base
  D_80101EC8 named 38 times in 25 functions; 85 per-word externs declared in include/.
Scratch (L1, tmp/prc harness = exact per-file recipe scored against build/): all 13 bodies 0, including
func_8001FBE8 (Ruling 6 split re-measured: 14, as banked) and func_80021A98 (s0 retyped, its two FAKE do-while
wraps unchanged). func_80022F34 (CORRECTED same day: the earlier "every index spelling scores 19" was a measuring error — a
quoting slip re-scored the r3 body for each variant): goto-loop `&g_practice_menu_table[i]` 19 (73 insns), a
separate record counter 19 (77), the landed reuse kept 19 (73), a `for` loop 4 (70), a do-while loop 0 (70)
(func_80022F34.c7.c, now the r3 body), and the `(PracticeMenuRec *)((u8 *)g_practice_menu_table + offset)` /
`offset += sizeof(PracticeMenuRec)` walk 0 (c8, not used). The do-while is a real loop, so loop.c strength-
reduces `i * 0x44C` into the target's offset register with the table base re-added each pass; the goto loop
gets no loop notes, so the multiply is recomputed (the 3 extra instructions).
Loop dump (probes-2026-09-30/loopdump_22F34.txt, `cc1 <CC_FLAGS> -dL`): the do-while body (l1 = r3) is a loop
(1 LOOP_BEG note; "Loop from 14 to 170"); the giv `i * 1100` (insn 29, reg 82) is strength-reduced to an
offset register (reg 121) while the table base symbol stays a separate never-incremented value re-added each
pass — the target's s1 += 0x44C / addu with &D_80101EC8. The goto-loop index form (l1c3) has no loop notes, so
loop.c never runs on it and the i * 0x44C shift/add chain is recomputed (the 3 extra instructions).
Plan update (orchestrator, 2026-09-30): L1 lands with three disclosed call-boundary pointer casts
(`func_800324D0((u8 *)s0)`, `func_8001FB34((s32 *)rec, ...)`, `func_8001FAE4((s32 *)rec->unk_50)`).
func_8001FB34 joins L2 (parameter retyped to PracticeMenuRec *, new member s16 unk_26C; its 7 field reads become
members). Separate follow-up debt, outside the record-table plan: func_8001FAE4 (walks the motion/sound entry held
in unk_50 as integers: `(s32 *)((s32)arg0 + 0xA)`, +8/+4 steps) and func_800324D0 (code6cac_b_tu2.c, `u8 *pad`
parameter read ~25 times; prototypes in code6cac.c / code6cac_tu2.c match its definition).
Plan update 2 (orchestrator): L2 = func_8001C8DC, func_8001CE60, func_8001E404, func_8001EFA0, func_8001FB34,
func_8001E878, func_8001EA84 (the last two handed back by laneH at HEAD; rev-tables-r3 notes: D_80101F5E /
D_801023AA / D_80101F7A handles, `s32 *a0 = &D_80102030` with `(u8 *)a0 - 0x168` / `+ 0x44C` / `+ 0x2E4` record
bases, `(u8 *)&D_80101EC8`, D_8010231A, `(&D_80101F7B)[ret = (D_800A3748 == 0) * 0x44C]`, and EA84's `ret` reused
for func_8005FA98's result). func_8003C9A4 may join if laneH's Rec44 respell is not byte-neutral.
L2 scratch (2026-09-30, HEAD bodies + L1 chain + land_l2.py; harness vs build/): func_8001C8DC 0/291, func_8001CE60
0/588, func_8001E404 0/184, func_8001EFA0 0/137, func_8001E878 0/99, func_8001EA84 0/268, func_8001FB34 0/45
(retyped; members unk_26C lh, unk_B3 sb), func_8001FBE8 0/289 (cast to func_8001FB34 dropped). EA84's `ret` now holds
only func_8005FA98's result (the subscript assignment is gone). Open for L2 review: func_8001E404 also views camera
data through `s2 = (s32 *)&D_800F6608` / `*(u16 *)((u8 *)s2 + 0x10)` / `*(CamBuf *)s2` (not the practice table);
func_8001E878 passes record addresses to s32-typed prototypes (func_8001A820, func_8001B478) as `(s32)&...`.
L2 update (orchestrator answers): func_8001E404's camera view is respelled too — `Rec44 *s2` (= &D_800F6608 /
&D_800F5328, both declared Rec44), `Rec44 local` in place of the TU-local CamBuf (the same 0x44 bytes; CamBuf's
vx/vy/vz/pad0/rx/ry/rz/pad1/dist/tail are Rec44's w0/w4/w8/wC/h10/h12/h14/h16/w18/rest), member reads
`s2->w0`..`s2->w18`, `&s2->w20`, and the two limit vectors (`&s2->h30` / `&s2->h38` on HEAD's layout,
`s2->h30[0]` / `s2->h30[1]` on laneH's h30[2][4], chosen by land_l2.py from the header). The CamBuf typedef goes.
Measured 0/184 on both bases (HEAD scratch; current tree with laneH's staged edits). func_8001E878's `(s32)&...`
arguments to func_8001A820 / func_8001B478 (s32-typed prototypes) stay disclosed call-boundary debt, as do
`(s32 *)&local.h10` to func_80046BF4 / func_80061064 in func_8001E404.
Follow-ups added (orchestrator, from rev-tables-r4; disclosed in laneH's landing until then), L3 or later:
func_8001B294 and func_8001B3C0 (PracticeMenuRec read through a pointer parameter at +0xF4/F8/FC and +0x180/184/188:
retype the parameter as PracticeMenuRec * and read members); func_8001A538 (called as `func_8001A538((s32 *)cam, ...)`,
walks Rec44 at +0x10/12/14 and arg0[6]: retype as Rec44 * with members and drop the cast in func_8001A820; note
func_8001E404 also calls it with `&local.w0`, which becomes `&local` once the prototype is Rec44 *).
