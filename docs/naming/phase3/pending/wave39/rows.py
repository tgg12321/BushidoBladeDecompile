"""wave39 local/param RESET re-derivation (PREPARE step 3), re-derived at ff76626c0 from the legacy draft at c9b8df219.

Legacy: 77 rows (wt_local_manifest.csv).  Kept 38 (35 FAKE-labelled, 3 N1 contradiction rows), dropped 39.
Dropped by reason: {'NOFAKE': 17, 'GONE': 22}
  GONE   the local no longer exists in the function (body rewritten/removed): 22
  NOFAKE the local exists but its FAKE label is gone (plain descriptive comment or none): 17
  plan refusals: 0 (each pair alone and all 38 jointly pass naming_keycheck.scoped_plan)
Kept by TU (current file): {'src/main/17AFC.c': 9, 'src/main/25788.c': 1, 'src/main/28708.c': 2, 'src/main/309CC.c': 3, 'src/main/32D04.c': 2, 'src/main/4510C.c': 2, 'src/main/4D0A8.c': 1, 'src/main/51268.c': 1, 'src/main/596CC.c': 6, 'src/main/5ED34.c': 2, 'src/main/63D2C.c': 2, 'src/main/64FD8.c': 3, 'src/main/6CF8.c': 2, 'src/main/87A0.c': 2}
Dropped by TU (legacy file): {'src/main/17AFC.c': 2, 'src/main/28708.c': 1, 'src/main/2B344.c': 2, 'src/main/31D3C.c': 2, 'src/main/32D04.c': 2, 'src/main/368E4.c': 1, 'src/main/3AB48.c': 14, 'src/main/51268.c': 11, 'src/main/9F9C.c': 4}
Functions renamed since c9b8df219 (kept rows): func_8001924C->softbody_UpdateList, func_80019568->pad_Update, func_80027AD8->fighter_ResolveBoneHit, func_800288C8->fighter_ResolveBodyOverlap, func_8002AB08->fighter_ProcessWeaponBodyHits, func_8002F2D0->math_MatrixToAngles, func_800338CC->fight_BuildOpponentOrder, func_80033D38->fight_InsertBestTime, func_80037F40->memcard_BuildSaveBlock, func_8003A728->comb_ExchangePad, func_8004428C->reloc_KeepSelectedEntries, func_8005490C->motion_UpdatePlayback, func_80065800->eff_DrawAnimatedQuads, func_8007352C->gpu_AddSpriteCells, func_80073728->gpu_AddScaledSpriteQuads, func_800759D0->menu_DrawSelGrid
Flags: see dry.txt (asm-region grants, prose-substitution hazards, claim-asserting comments, borderline words).
KEPT = (current f:old, destination, kind); DROPPED = (legacy f:old, legacy dest, reason, note).
"""

KEPT = [('fighter_ResolveBoneHit:tbl', 'var_0', 'local'),
 ('fighter_ResolveBoneHit:tbl_arg', 'var_1', 'local'),
 ('fighter_ResolveBodyOverlap:dist', 'var_13', 'local'),
 ('fighter_ResolveBodyOverlap:shift', 'var_20', 'local'),
 ('fighter_ProcessWeaponBodyHits:vec', 'var_1', 'local'),
 ('math_MatrixToAngles:work', 'var_7', 'local'),
 ('func_8002F770:work', 'var_7', 'local'),
 ('fight_InsertBestTime:rec', 'var_0', 'local'),
 ('fight_BuildOpponentOrder:one', 'var_5', 'local'),
 ('func_80035280:base', 'var_2', 'local'),
 ('memcard_BuildSaveBlock:base', 'var_3', 'local'),
 ('comb_ExchangePad:zero', 'var_7', 'local'),
 ('func_800408F8:base', 'var_4', 'local'),
 ('func_80040CB8:kind', 'var_4', 'local'),
 ('func_80040CB8:one', 'var_5', 'local'),
 ('func_80043278:mask', 'var_0', 'local'),
 ('reloc_KeepSelectedEntries:stop', 'var_9', 'local'),
 ('motion_UpdatePlayback:rot_z', 'var_6', 'local'),
 ('func_80058580:one', 'var_59', 'local'),
 ('func_8005D554:scale', 'var_10', 'local'),
 ('eff_DrawAnimatedQuads:tbl', 'var_18', 'local'),
 ('func_80069F80:tbl', 'var_4', 'local'),
 ('func_8006A1A0:tbl', 'var_4', 'local'),
 ('func_8006C21C:mode', 'var_8', 'local'),
 ('func_8006C21C:work', 'var_9', 'local'),
 ('func_8006C21C:col', 'var_10', 'local'),
 ('func_8006BEC4:pos', 'var_2', 'local'),
 ('func_8006F100:cx', 'var_10', 'local'),
 ('func_8006F100:cy', 'var_11', 'local'),
 ('gpu_AddSpriteCells:env', 'arg0', 'param'),
 ('gpu_AddScaledSpriteQuads:env', 'arg0', 'param'),
 ('menu_DrawSelGrid:zero', 'var_2', 'local'),
 ('func_8007636C:mode', 'var_5', 'local'),
 ('func_80078654:zero', 'var_4', 'local'),
 ('func_800167EC:rec', 'var_1', 'local'),
 ('main:tbl', 'var_4', 'local'),
 ('softbody_UpdateList:buf', 'var_2', 'local'),
 ('pad_Update:rec', 'var_5', 'local')]

DROPPED = [('func_8002C22C:rec1',
  'var_1',
  'NOFAKE',
  'the only FAKE alias label for D_80101EC8[0]/[1] now sits in func_8002C61C (src/main/17AFC.c:2209), a '
  'different function; func_8002C22C declares rec0/rec1 with no label'),
 ('func_8002D780:ax',
  'var_11',
  'NOFAKE',
  "the `ax holds cx - x0` label is gone; the only FAKE labels left in math_TestPreparedTriangleSphere's "
  'block (src/main/17AFC.c:2752) are about `flag`'),
 ('func_8003993C:next',
  'var_18',
  'GONE',
  'local `next` no longer exists: current definition replay_PlayFrame has no `next` token (body rewritten / '
  'local removed)'),
 ('func_8003C42C:base',
  'var_3',
  'GONE',
  'local `base` no longer exists: current definition func_8003C42C has no `base` token (body rewritten / '
  'local removed)'),
 ('func_8003C42C:base2',
  'var_7',
  'GONE',
  'local `base2` no longer exists: current definition func_8003C42C has no `base2` token (body rewritten / '
  'local removed)'),
 ('func_80041604:val',
  'var_1',
  'GONE',
  'local `val` no longer exists: current definition func_80041604 has no `val` token (body rewritten / local '
  'removed)'),
 ('func_80041650:val',
  'var_1',
  'GONE',
  'local `val` no longer exists: current definition func_80041650 has no `val` token (body rewritten / local '
  'removed)'),
 ('func_800432A0:idx',
  'var_1',
  'GONE',
  'local `idx` no longer exists: current definition gpu_OffsetBlockTexPrimStreams has no `idx` token (body '
  'rewritten / local removed)'),
 ('func_800432A0:base_addr',
  'var_5',
  'GONE',
  'local `base_addr` no longer exists: current definition gpu_OffsetBlockTexPrimStreams has no `base_addr` '
  'token (body rewritten / local removed)'),
 ('func_800475A4:neg',
  'var_7',
  'NOFAKE',
  'label `s16 temporary for the negated pitch` is gone (src/main/368E4.c:703 plain declaration)'),
 ('func_80054604:id',
  'var_1',
  'NOFAKE',
  'label `id (a0 + 0x131) computed at entry` is gone (src/main/43500.c:571 plain declaration)'),
 ('func_8005D46C:ret',
  'var_1',
  'GONE',
  'local `ret` no longer exists: current definition func_8005D46C has no `ret` token (body rewritten / local '
  'removed)'),
 ('func_8005D814:end_off',
  'var_6',
  'NOFAKE',
  'func_8005D814 (src/main/4D0A8.c:464): `end_off` still exists but carries no /* FAKE */ label now (the '
  'legacy /* FAKE: the returned size... */ label was replaced by the plain comment `One past the chunk this '
  'function fills; it returns end_off - argN`; legacy text absent from the function; it is a plain '
  'descriptive comment)'),
 ('func_8005E098:end_off',
  'var_6',
  'NOFAKE',
  'func_8005E098 (src/main/4D0A8.c:627): `end_off` still exists but carries no /* FAKE */ label now (the '
  'legacy /* FAKE: the returned size... */ label was replaced by the plain comment `One past the chunk this '
  'function fills; it returns end_off - argN`; legacy text absent from the function; it is a plain '
  'descriptive comment)'),
 ('func_8005E54C:end_off',
  'var_9',
  'NOFAKE',
  'func_8005E54C (src/main/4D0A8.c:777): `end_off` still exists but carries no /* FAKE */ label now (the '
  'legacy /* FAKE: the returned size... */ label was replaced by the plain comment `One past the chunk this '
  'function fills; it returns end_off - argN`; legacy text absent from the function; it is a plain '
  'descriptive comment)'),
 ('func_8005F1C8:end_off',
  'var_6',
  'NOFAKE',
  'func_8005F1C8 (src/main/4D0A8.c:1056): `end_off` still exists but carries no /* FAKE */ label now (the '
  'legacy /* FAKE: the returned size... */ label was replaced by the plain comment `One past the chunk this '
  'function fills; it returns end_off - argN`; legacy text absent from the function; it is a plain '
  'descriptive comment)'),
 ('func_8005FA98:ret',
  'var_1',
  'GONE',
  'local `ret` no longer exists: current definition func_8005FA98 has no `ret` token (body rewritten / local '
  'removed)'),
 ('func_8005FA98:start',
  'var_2',
  'GONE',
  'local `start` no longer exists: current definition func_8005FA98 has no `start` token (body rewritten / '
  'local removed)'),
 ('func_8005FA98:end',
  'var_3',
  'NOFAKE',
  'func_8005FA98 (src/main/4D0A8.c:1234): `end` still exists but carries no /* FAKE */ label now (the legacy '
  '/* FAKE: the returned size... */ label was replaced by the plain comment `One past the chunk this '
  'function fills; it returns end - argN`; legacy text absent from the function; it is a plain descriptive '
  'comment)'),
 ('func_8005FC9C:end_off',
  'var_9',
  'NOFAKE',
  'gpu_DrawImageAndAdvanceSplitWipe (src/main/4D0A8.c:1299): `end_off` still exists but carries no /* FAKE '
  '*/ label now (the legacy /* FAKE: the returned size... */ label was replaced by the plain comment `One '
  'past the chunk this function fills; it returns end_off - argN`; legacy text absent from the function; it '
  'is a plain descriptive comment)'),
 ('func_800600C8:end_off',
  'var_4',
  'NOFAKE',
  'func_800600C8 (src/main/4D0A8.c:1409): `end_off` still exists but carries no /* FAKE */ label now (the '
  'legacy /* FAKE: the returned size... */ label was replaced by the plain comment `One past the chunk this '
  'function fills; it returns end_off - argN`; legacy text absent from the function; it is a plain '
  'descriptive comment)'),
 ('func_80060414:end_off',
  'var_3',
  'NOFAKE',
  'func_80060414 (src/main/4D0A8.c:1490): `end_off` still exists but carries no /* FAKE */ label now (the '
  'legacy /* FAKE: the returned size... */ label was replaced by the plain comment `One past the chunk this '
  'function fills; it returns end_off - argN`; legacy text absent from the function; it is a plain '
  'descriptive comment)'),
 ('func_80060544:end_off',
  'var_2',
  'NOFAKE',
  'func_80060544 (src/main/4D0A8.c:1526): `end_off` still exists but carries no /* FAKE */ label now (the '
  'legacy /* FAKE: the returned size... */ label was replaced by the plain comment `One past the chunk this '
  'function fills; it returns end_off - argN`; legacy text absent from the function; it is a plain '
  'descriptive comment)'),
 ('func_80060768:end_off',
  'var_6',
  'NOFAKE',
  'func_80060768 (src/main/4D0A8.c:1612): `end_off` still exists but carries no /* FAKE */ label now (the '
  'legacy /* FAKE: the returned size... */ label was replaced by the plain comment `One past the chunk this '
  'function fills; it returns end_off - argN`; legacy text absent from the function; it is a plain '
  'descriptive comment)'),
 ('func_80064E90:last',
  'var_0',
  'GONE',
  'local `last` no longer exists: current definition func_80064E90 has no `last` token (body rewritten / '
  'local removed)'),
 ('func_80064ED8:last',
  'var_0',
  'GONE',
  'local `last` no longer exists: current definition func_80064ED8 has no `last` token (body rewritten / '
  'local removed)'),
 ('func_80064F20:last',
  'var_0',
  'GONE',
  'local `last` no longer exists: current definition func_80064F20 has no `last` token (body rewritten / '
  'local removed)'),
 ('func_80065000:last',
  'var_1',
  'GONE',
  'local `last` no longer exists: current definition func_80065000 has no `last` token (body rewritten / '
  'local removed)'),
 ('func_8006505C:last',
  'var_0',
  'GONE',
  'local `last` no longer exists: current definition func_8006505C has no `last` token (body rewritten / '
  'local removed)'),
 ('func_800650A4:last',
  'var_0',
  'GONE',
  'local `last` no longer exists: current definition func_800650A4 has no `last` token (body rewritten / '
  'local removed)'),
 ('func_800650EC:last',
  'var_0',
  'GONE',
  'local `last` no longer exists: current definition func_800650EC has no `last` token (body rewritten / '
  'local removed)'),
 ('func_80065134:last',
  'var_0',
  'GONE',
  'local `last` no longer exists: current definition func_80065134 has no `last` token (body rewritten / '
  'local removed)'),
 ('func_80065264:last',
  'var_0',
  'GONE',
  'local `last` no longer exists: current definition func_80065264 has no `last` token (body rewritten / '
  'local removed)'),
 ('func_800652AC:last',
  'var_0',
  'GONE',
  'local `last` no longer exists: current definition func_800652AC has no `last` token (body rewritten / '
  'local removed)'),
 ('func_800693CC:mask_ptr',
  'var_10',
  'GONE',
  'local `mask_ptr` no longer exists: current definition menu_UpdateRowMenu has no `mask_ptr` token (body '
  'rewritten / local removed)'),
 ('func_8001B478:cnt',
  'var_11',
  'GONE',
  'local `cnt` no longer exists: current definition camera_UpdateTrackedFighterView has no `cnt` token (body '
  'rewritten / local removed)'),
 ('func_8001BCF0:diff',
  'var_0',
  'NOFAKE',
  'label `diff computed at entry` is gone (src/main/9F9C.c:1000); the lone FAKE in the function (:1004) is '
  'about collision_pos'),
 ('func_8001BCF0:div4', 'var_1', 'NOFAKE', 'label `div4 taken first` is gone (src/main/9F9C.c:1014)'),
 ('func_80021280:mode',
  'var_10',
  'NOFAKE',
  'the `D_800A38DC read once before the loop` label is gone; only t1/t4/t3/t2 carry FAKE labels '
  '(src/main/9F9C.c:3174-3180), mode (:3182) has none')]


VERDICT = "CONFIRM"
VERIFIER_NOTE = ("naming-reviewer pre-apply (round 1, 38/38 PASS). 35 FAKE-labelled locals reset by N1; "
    "SOTN-precedent alias labels (fight_InsertBestTime:rec, func_800167EC:rec) admit the construct, not a name; "
    "3 non-FAKE rows contradicted: func_8006BEC4:pos points at a Size2s16 read as TILE w/h; "
    "gpu_AddSpriteCells:env / gpu_AddScaledSpriteQuads:env name a sprite-walker argument block, not DRAWENV/DISPENV. "
    "No asm island in the 5 granted functions names a renamed local.")

APPLIER_NOTE = """
1. src/main/17AFC.c:838-841 (fighter_ResolveBodyOverlap): rewrite the label as
   /* FAKE: two values -- the leading-zero count, then the amount var_13 is shifted right by to
   index g_sqrt_table_u8; as one expression `var_20 = 0x16 - (lzc_out & ~1)` the count lands in
   $a0 (target: $v1 / $v0): score 4 */  (drops prose 'shift'; let format.py re-wrap).
2. src/main/32D04.c:526-527 (func_80043278): rewrite as
   /* FAKE: constant holder for 0xFFFFF000; with the literal the function is one insn longer
   (score 4) */  (prose 'mask' must not become 'var_0').
3. src/main/4510C.c:2037 'the outer one' is prose: do not substitute. Substitute only the label
   at :2443 ('a function-scope `var_59` all differ').
4. src/main/63D2C.c:156-157 gpu_AddRotatedSpriteQuads (K&R sibling) keeps its `env`; only the
   bindings at :11 and :55 and their own bodies move.
5. src/main/17AFC.c:325-326 trailing comments stay as written (true of a copy of rec).
6. Plain pair substitutions that stay true: 17AFC.c:322, :476/:499/:516, :795, :5322;
   309CC.c:292, :431; 5ED34.c:469, :477; 6CF8.c:124; 87A0.c:644.
Comments 1-2 are not pure pair substitutions: land them in a comment-only prep commit before the
wave (as wave36's fc2b07bf3) so the wave's keycheck sees only pair words.
"""
