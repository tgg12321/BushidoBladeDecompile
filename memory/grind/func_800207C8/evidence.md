# func_800207C8 evidence

## LANDED 2026-10-02 (lane oct2-b8) — COMPLETED-INLINE-ASM-CANONICAL; ledger closed
- cheat-cleanup 5493ecebd (part A, BoneHitRec tables; rv2-207C8-A PASS on six bodies), auth 7af6d7cf5 and
  Match e7aa28e43 (rv2-207C8-B PASS, body_hash 4ab10331f96d031c, scopes auth + match, layer2.jsonl), queue
  6a7774151. Oracle SHA1 match; check_completion_integrity.py OK.
- Reviewer hygiene notes: splat D_8008D59E / D_800F5F6C rows remain; BoneHitRec unk_0C..unk_12 are
  radius / radius-squared pairs (could be named).

## State at landing (lane oct2-b8, 2026-10-02)
- `candidate.c` + `header.patch` (include/code6cac.h): byte-exact. Private harness
  (tools/h.py, run from tmp/func_800207C8/: src copy + patched header beside it, faithful pipeline): score 0, 317/317;
  the only residual is the not-scored `%lo(D_800F5F68+4)` vs splat's `D_800F5F6C` addend.
  Every other code6cac.h consumer (29 TUs) rebuilt with the patched header: 0 differing functions /
  sections. Private full link (build/ objects + the new code6cac_tu2.o): SHA1 62efab4f…fa MATCH.
- No FAKE constructs. Islands: inline_o.h 4.3 gte_SetRotMatrix / gte_ldv0 / gte_rtv0 / gte_stlvnl
  (84 statements, all recognized by engine/gtemacro.py unit_spans). The four gte_rtv0 units carry the
  post-DMPSX `.word 0x4A486012` (targets 0x80020898, 0x80020998, 0x80020A88, 0x80020B34) — needs a
  per-function grant like Q92 (func_800204C0). `canonical`: ASM-PARTIAL, 44/317 cop2.
- Q93 granted (fed205ca7). Orchestrator ruled the `(BoneHitRec *)&D_800F5F68[ch * 0x1B8]` view item-4
  blocking -> part A (landing/msg_A.txt): D_800F5F68 `BoneHitRec [2][22]`, template D_8008D59C
  `BoneHitRec [22]` (D_8008D59E extern dropped), six consumer bodies respelled to members; every
  code6cac.h consumer instruction-identical, oracle SHA1 match with A alone and with A+B.
- Landing prepared under the lock (landing/: mine_A.patch staged, mine_auth.patch + mine_B.patch in the
  working tree; tools/apply.py regenerates A / B from the tree). sandbox --disable all 0 (317/317),
  verify-oracle --rebuild SHA1 match. layer2 hash func_800207C8 4ab10331f96d031c.

## What closed it (the 2026-09-24 Codex draft sat at 99)
1. Walking pointers, not indexed arrays. Target's `a1 = out + 8` with y at -4(a1): loop.c express_from
   (loop.c:5419) only combines givs whose g1 add_val is CONST_INT. With `out[i]` indexing the add_val
   is the param register, so nothing combines that way; with `o++` the output pointer is a biv, y/z
   are DEST_ADDR givs add 4 / 8, and the z giv (first in the prepend-ordered list) absorbs y as -4.
   Same for the hit record: `&hr->ofs` (add 4) absorbs `hr->bone` as -2 and the base folds to
   D_800F5F68+4 (splat's D_800F5F6C). Indexed: 87; walking: 30.
2. Increments in the for header (`i++, hr++, o++`): i's addiu schedules first (30 -> 24).
3. `rec->unk_180 = bone_out[0]` as one struct copy (lw x3 then sw x3): needs LeafPos == Vec3i32
   (header: `typedef Vec3i32 LeafPos;`, layout-identical, all consumers byte-identical). 24 -> 16.
4. One `m` for every "current bone matrix" use incl. the two ratan2 reads (target seats it in $a2
   throughout); the unk_198 section reads the translations through `pos = bones[k]->t` (target's $a0
   pseudo). m for both: 8; bones[k]->t direct: 32-38 (reloads after the rec stores); separate MATRIX *b: 0;
   `s32 *pos`: 0 (kept — it names what is read).
5. probe = 0x1F8002B8 initialized at the declaration (target loads it in the prologue); assigned just
   before the probe loop: 43.

## Data model (header.patch)
- PracticeMenuRec: unk_198 Vec3i32[2], unk_1B0 s32[2], unk_1BA / unk_1C2 s16 (pads around them),
  unk_1EC Vec3i32. BoneHitRec (0x14: u16, s16 bone, SVec4i16 ofs, u16[4]) — the 22 records of a
  D_800F5F68 0x1B8-byte character record (func_800206B0 fills them; field offsets per its stores).
- D_8008D864 u8[8] (counts), D_8008D86C SVec4i16 *[8], D_8008D88C SVec4i16 *[32] (asm/data/7D920.data.s
  dlabels: pointer words to the D_8008D754.. SVECTOR sets), D_8008D774 SVec4i16[2], D_800A3138 SVec4i16
  ((0, 0x1000, 0), asm/data/938FE.data.s). No other C consumer of these five symbols.
- `typedef Vec3i32 LeafPos;` (was a separate identical struct).
- Part A: BoneHitRec (s16 unk_00, s16 bone, SVec4i16 ofs, u16 unk_0C/0E/10/12), D_800F5F68[2][22]
  (next symbol D_800F62E0), D_8008D59C[22] (0x8008D59C..0x8008D753). func_800206B0 respelled as a
  walking-pointer for loop (same giv mechanism) - byte-identical.

## Earlier record
- 2026-09-24 Codex session: semantic draft 320/317, score 99 (body not saved); rotated.
