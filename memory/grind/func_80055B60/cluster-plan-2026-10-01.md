# func_80055B60 cluster cleanup plan (laneA, 2026-10-01) [new-doc: execution plan for a fresh agent]

func_80055B60 itself is byte-exact (candidate.c, sandbox 0) and its reused locals PASSed layer-2 (rev-55B60-r11).
It is blocked by data-model debt it touches. Land the steps below IN ORDER, each as its own reviewed commit, then
re-splice func_80055B60. All line numbers are main @ 585c7a66a (re-grep before editing). Every changed body is
re-certified in full (a changed body re-certifies ALL its debt), so each step lists the debt it must clear.

## Tools already banked
- probes/prep_d6a78/: tucheck.py (build a modified TU through the real pipeline with an optional include dir
  ahead of include/, score EVERY function vs build/src/<stem>.o), fdiff.py (per-function side-by-side), conv.py
  (u8 *obj walk -> members), manual.py + f_*.c (hand bodies), hdr.py (record typedef), land_p1.py, chk.sh.
- probes/prep_d6a78/rejected-p1-landing.patch: the FAILED first attempt (all functions byte-exact; use as the
  starting text). code6cac_b_tu2.landed-candidate.c is that TU.
- probes/rev_d6a78b/: reviewer probes (d7c_pv2.c / d7c_abl_* / d7c_scalarish.c / mkpv2.py, d04_*, ba8_*).
- Layer-2 records already filed: memory/grind/<func>/layer2.jsonl for the 12 bodies (rev-d6a78-a FAIL x6,
  rev-d6a78-b FAIL func_80030D04, PASS func_80030900 / cpu_set_move_command_and_dir / func_80030BA8 /
  func_80030D7C / func_80031B24). PASSes are for the REJECTED landing's hashes; the bodies change again below,
  so every one needs a fresh review.

## Step 1 — D_80106A78 records, D_800A36F2, D_8008E338 (one cheat-cleanup landing)
Header (include/code6cac.h):
1a. Add `Obj80106A78` (hdr.py text) BUT type 0x0C..0x2B as the PsyQ `MATRIX` (s16 m[3][3] + pad + s32 t[3];
    func_8002FF20 stores the s16 identity with sh 0xC..0x1C and passes +0xC to RotMatrixX/Y/Z / MulMatrix0;
    func_800300B4 MulMatrix0s +0xC). Use the project's MATRIX/Mat typedef if one exists (grep include/gte.h),
    never a filler array. `extern Obj80106A78 D_80106A78[12];` (12 x 0x64 ends exactly at _ss_score: verified).
1b. DELETE code6cac.h:763-765 `extern s16 D_80106A7A; extern u8 D_80106A80; extern u8 D_80106A82;`
    (rev-d6a78-a (a): the first attempt left them dangling while claiming retirement).
1c. code6cac.h:368 `extern u8 D_800A36F2;` -> `extern u8 D_800A36F2[2];` (honest: func_8003047C stores
    [record index]; readers read [0]; the scalar read also costs func_80030D7C 20 via sched.c true_dependence
    834-836). named_syms.txt:1624 `g_char_class_state_array = 0x800A36F2` stays (same object).
1d. code6cac.h:236 `extern u8 D_8008E338;` -> `extern s8 D_8008E338[27][5];` — dlabel asm/data/7D920.data.s:1710
    spans 0x8008E338..0x8008E3BF (136 bytes = 27 x 5 + 1 alignment byte; check the last byte before choosing 27),
    read as `(s8)` in func_8003047C. Grep all consumers (only src/code6cac_b_tu2.c:4321/4336 today).
Remove `extern u8 D_80106A78;` at src/code6cac_b.c:133 and src/code6cac_b_tu2.c:37.
undefined_syms_auto.txt: delete the D_80106A7A / D_80106A80 / D_80106A82 rows (all their asm/funcs referrers
are C functions; none is INCLUDE_ASM — re-check with `grep -l` + INCLUDE_ASM before deleting).
named_syms.txt: delete g_active_slot_table_12 (0x80106A7A), g_active_slot_flags_12 (0x80106A80),
g_active_slot_flags_12_plus_2 (0x80106A82); reword the D_80106A78 row and the notes at :28 and :3306 (land_p1.py).

Bodies (src/code6cac_b_tu2.c), with the fixes rev-d6a78-a/-b demanded:
- func_80030208 (:4222): `for (obj = D_80106A78, i = 0; i < 12; i++, obj++)` (f_80030208.c, 0); after step 2
  call `func_800300B4(obj)` with no cast.
- func_8003043C (:4301): literals, no `neg`/`val` constant holders; plain `for` also scores 0 (reviewer).
- func_8003047C (:4313): parameter `PracticeMenuRec *` (caller src/code6cac_tu2.c:3864 passes a record) with
  members unk_04 / unk_0A / unk_330 / unk_332[5] (add the members the PracticeMenuRec header lacks); read
  `D_8008E338[idx][i]`, store `D_800A36F2[idx2]`.
- func_80030524 (:4341): literal -1 (no `neg`).
- func_80030580 (:4356): returns `Obj80106A78 *`; drop `u8 *src = (u8 *)arg0` — type the parameter
  `PracticeMenuRec *` (callers pass records: :4444, :4466, :4490) and read members unk_04 (u16 read: keep a
  value cast only where the target's lhu needs it), unk_1A, unk_1C8.vy (read both lhu and lh: one value cast),
  unk_F4.x/y/z; keep the existing volatile pad FAKE text as is. Update both externs (:4415, :4460).
- func_800307D0 (:4417): parameter `PracticeMenuRec *` (members unk_330, unk_332, unk_14, unk_88); delete the
  stale TABLED comment above it; measure `idx = (top == cur)` against the current `top ^ cur; (u32)top < 1`.
- func_80030900 (:4461), cpu_set_move_command_and_dir (:4486): first-attempt text PASSed (rev-d6a78-b); their
  a0 parameters are records too (caller :367 func_80027A58 passes one) — retype if step 2's prototype work
  touches them, else keep; re-review either way (callee return type changed).
- func_80030BA8 (:4542): pointer `for` form PASSed; keep.
- func_80030D04 (:4603): literal -1 (no `neg`; 0/17 per rev-d6a78-b).
- func_80030D7C (:4627): PASSed (Ruling 11 work/temp re-verified on the new body: PV2 21, temp-only 4,
  work-only 17 — probes/rev_d6a78b/); after step 2 the `(u8 *)&obj->unk_2C` casts go away.
- func_80031B24 (:4904): PASSed (0a0069bdf fix intact, Vec3i -> Vec3i32); after step 2 drop the casts into
  func_8002FF20 / func_80031890 / func_80032854.

## Step 2 — the record-only callees and func_80032854's prototype (same landing as step 1 or immediately after)
- func_800300B4 (:4153, COMPLETED-INLINE-ASM-CANONICAL: keep its GTE macro islands verbatim; its
  inline_asm_canonical.txt / canonical_asm_regions.json grant is keyed by region hashes — recompute with
  engine.completion.region_hashes if the island text changes; the operand `arg0 + 0x2C` becomes `&arg0->unk_2C`):
  `Obj80106A78 *arg0`, members unk_06 / unk_09 / unk_0C (MATRIX) / unk_02 / unk_0A.
- func_8002FF20 (:4047, also canonical-asm islands): `Obj80106A78 *arg0`, the identity matrix through the MATRIX
  member.
- func_80031890 (:4812, canonical-asm islands): `ent` is the record -> `Obj80106A78 *ent`; `obj` is the scratch
  block (keep u8 *, disclosed) — check what +0xD8 is before typing it.
- func_80032854 (definition :5344 `u8 *arg2`; externs :203, :4416, src/code6cac_tu2.c:2751 `s32 *arg2`):
  decide the honest pointee (positions: Vec3i32 * — confirm from func_800395B4 / func_800617C8 uses) and fix the
  definition plus every declaration; 75 call sites (code6cac_b.c 3, code6cac_b_tu2.c 69, code6cac_c_mid.c 1,
  code6cac_tu2.c 2) must compile byte-identical — pass `&obj->unk_2C` / `&SPAD->...` without casts where the
  new type fits, otherwise justify each remaining cast. This is the largest step; it can be split out as its own
  landing if the cast census grows.
Verification for steps 1-2: tucheck.py on every touched TU (all functions 0), `lock.ps1 rebuild` == oracle,
`sandbox --disable all` 0 per changed body, `layer2 hash` per body, one cheat-cleanup review listing every body.

## Step 3 — PracticeMenuRec unk_6A as u16 (rev-55B60-dm item 3)
- 96471164a (Match func_80026DA4) flipped `u16 unk_6A` -> `s16` citing docs/naming/CHAR_STRUCT_SCHEMA.md:49 and
  added `(u16)` value casts at every reader. Every target access is lhu. Under
  .claude/rules/header-type-correction-from-use-sites.md: header back to `u16 unk_6A` (prongs a-c; cite
  96471164a and the schema), func_80055B60 carries no casts; the reviewer ruled the other bodies' now-redundant
  casts cosmetic (their text does not change) — state that explicitly against prong (d). Verify byte-neutral:
  tucheck.py on code6cac_b.c, code6cac_b_tu2.c, code6cac_tu2.c (18 sites today) + full rebuild.
- OVERLAP: memory/grind/func_80021424/HANDOFF.md L1 (13 bodies, banked, not landed) also flips unk_6A to u16
  and adds PracticeMenuRec members (unk_50 as u8 *, unk_58 s32, unk_5C s16, unk_6C, unk_60/61, unk_AD/AF/B0,
  unk_272...). func_80055B60 needs unk_58 as a POINTER (`lw` then `lbu 1/2` off it) and unk_5C read with lhu:
  reconcile before either lands (u8 *unk_58; u16 unk_5C is compatible with L1's sh-only evidence). Land
  whichever goes first and rebase the other's header hunk.

## Step 4 — func_80055B60 itself (re-splice candidate.c with these fixes)
- adopt `temp = (u16)rec->unk_6A == 0x11 ? 8 : 4;` (reviewer R_add_tern, 0) and then drop all `(u16)` casts
  on unk_6A once step 3 landed (re-measure; regenerate the Ruling 11 numbers on the exact body: mkpv.py /
  abl2.py in probes/r11/).
- D_80106A78 through `Obj80106A78 *obj = &D_80106A78[i]` members (no byte walk; `extern u8 D_80106A78` gone).
- unk_3B4 typed `u8 *` (func_80055948 loads/stores a script pointer there: src/text1b.c:2483/2536).
- named_syms.txt: delete g_status_flag_record_table_80099D88_plus_5 (:2230, 0x80099D8D) and
  g_practice_lesson_init_done_plus_4 (:2373, 0x80102790, really D_80102788.held) together with the
  undefined_syms rows D_80099D8D / D_80102790 (land.py already retires those two); precedents 18cb49eea,
  f79e2153c.
- disclose (or respell) the second handle D_80101F4E (include/code6cac.h:676, = g_practice_menu_table+0x86 =
  unk_86) used by src/code6cac_tu2.c:3321/3326 (func_800218C8 / func_80021904; both are L1 bodies — prefer
  letting L1 retire it, disclose in the message meanwhile; D_80101F4C over unk_84 is the same pre-existing case).
- land.py + mkhdr.py (probes) apply the src/header/syms edits; msg template tmp/b60/msg_match.txt -> rewrite
  covering items 1-5 of rev-55B60-dm. Reviewer split: r11 (reused locals) already PASSed on a6f34d363c20ad3f;
  the body hash changes, so both reviews rerun.

## Dependency order
Step 1+2 (record cluster; can be one landing or 1 then 2) -> Step 3 (unk_6A; coordinate with L1) -> Step 4.
func_80058580 (laneB, same TU text1b.c) declares `s32 func_80058580(u8 *p)`; func_80055B60's extern matches it.
