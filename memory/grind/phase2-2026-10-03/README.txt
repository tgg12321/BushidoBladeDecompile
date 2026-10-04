Phase 2 (types) census and verification harness -- state at 0d97b2706 (2026-10-03).
HARNESS (tools/, run in WSL; scratch in tmp/p2/)
  wsl bash memory/grind/phase2-2026-10-03/tools/check.sh [--clean] --base base   # once, on the unchanged tree
  wsl bash memory/grind/phase2-2026-10-03/tools/check.sh base <name>             # after each edit (~60 s)
  = make -j, SHA1 vs oracle, snap.py (linked .o files, per-TU token hashes, cc1 -Wimplicit pairs, src/ +
  include/ copies), cmp.py (section bytes / relocs / symbols per object; implicit-set changes), l2diff.py
  (layer-2 keys moved since the base: each needs a fresh layer-2 record). PASS = SHA1 + every object equal.
  Verified: unchanged tree -> 0 diffs; corrupted snapshot copy -> OBJECT DIFF + MOVED. all.sh: every TSV.

FILES
  phase2_conflicts.tsv  refreshed declaration conflicts: 89 sony-func, 39 game-local, 7 game-proto, 2
                        header-local, 9 notes. Step 5's 66 game-local rows included 27 Sony functions that
                        hoist.py rejected before its Sony test; they are now only sony-func rows.
  decl_worklist.tsv     one row per function: reference prototype (include/psxsdk -> SOTN include/psxsdk ->
                        psyz psyz/include [PsyQ 4.0 rewrite; its return types are not always Sony's] -> BB2
                        definition) and a verdict from proto_trial.tsv (each caller TU compiled by cc1, real
                        flags, under the reference; asm vs the unmodified TU): ready / casts (same asm, call
                        sites need typed args) / codegen / blocked / def-retype (the definition conflicts).
                          sony: ready 22, casts 13, def-retype 31, codegen 7, blocked 12, no-reference 4
                          game: ready 16, casts 9, codegen 2, blocked 6
  casts.tsv             4,287 raw-offset cast sites (site, kind, access type, base, root + declared type,
                        offset, struct + how determined, member at offset, FAKE flags; tools/casts.py doc).
                        Kinds: deref-load 1238, deref-store 1116, addr 588, pun-load 532, pun-store 261,
                        index-load 310, index-store 86, ptradd 87, struct-cast 69; 398 have computed offsets.
                        Aggregates: casts_by_struct.tsv, casts_by_file.tsv.
  protos.tsv            untyped pointer parameters (u8*/s32*/void*/... and pointer-sized ints with evidence):
                        typed-callers 123, body-casts 25, conflict 22, scratchpad 25, mixed 4, none 348.

CAST CENSUS -- what is determinable
  Struct named for 740 sites: caller 503, assign 114 (489 of these 617 with k == n), decl 119 (the base is
  declared as the struct and the cast discards it: item 4 directly), other 4.
  Member at offset: exact 446, addr-of 97, sign 50, size 47, inside 39, computed 22, beyond 21, pad 4.
  By struct: Unk80101EC8Record 475 (333 exact; 9F9C 236, 17AFC 176, 3AB48 44, 28708 18), GameObj 58,
  S_6A880 47, RevParamEntry 32, Rec44 22, POLY_FT4 17 (12 beyond: the base is a buffer, not a POLY_FT4),
  MotionFrame 14, SceneRec 11, Unk80101DF0Record 11, PadState 7, DRAWENV 7, DISPENV 5.
  Not determinable from existing types: 2607 untyped (u8*/s32/void* roots with no typed evidence; top:
  5ED34/51268 arg0, D_800A34B8, _spu_RXX), 671 scratchpad workspaces (0x1F800xxx), 246 unknown, 21
  ambiguous. Typing those needs new aggregates (new names = claims), not Phase 2 member access.
  FAKE: 33 sites have a FAKE comment within 3 lines above; 3 functions' comments tie a cast to codegen.
  "~1,238" equals this census's deref-load count exactly (`*(T *)(base + off)` loads, all files, incl.
  computed offsets); stores, puns, index, addr forms were evidently not counted.
  "Entity": no evidence. Obj80106A78 (12 x 0x64 at 0x80106A78) is already accessed only by member (0 raw
  sites); Unk80101EC8Record (2 x 0x44C per-character records, name reset by Q103) differs in size, table,
  layout head and consumers. Merging or renaming them would be an invented object model (item 3).

COMMIT PLAN (each batch: check.sh base <batch> = CHECK PASS; moved keys -> layer-2; one family/struct/file)
  Risk keys: GP = cc1 -G8 file (309CC 31548 31CFC 31D3C 24F08 26730 26940 5ED34 368E4): an extern object's
  size class moves gp; LB = EXPAND_LB file (175A4 17AFC 24BF0 24F08 25788): byte signedness moves lb/lbu;
  SIGN = a load whose member signedness differs (lh/lhu); IMPL = implicit -> prototyped call.
  1 libsnd/libspu/libcomb/libetc ready prototypes into include/psxsdk (14): asm-same measured; IMPL none.
  2 libgte casts (RotMatrixX/Y/Z, RotTrans, SetRot/TransMatrix, ApplyMatrix, ApplyRotMatrix, MulMatrix0,
    ratan2): caller locals take MATRIX/SVECTOR/VECTOR; retires game.h Obj80106A78Mat/Unk80101DF0Mat/Vec4i32
    /SVec4i16 aliases where layouts are equal. Risk: frame layout of retyped locals; GP (368E4); IMPL
    (MulMatrix0 17AFC, RotTrans/SetRot/TransMatrix 51268: asm-same measured).
  3 libgpu: ready + casts (PutDispEnv, ClearOTagR, MoveImage, _exeque); then add PsyQ DR_MODE / DR_AREA /
    DR_OFFSET / TILE / SPRT to libgpu.h (unblocks 5 rows); then def-retypes one module per commit (prim.c,
    sys.c, ext.c). GetClut (u16 return) changes 368E4 func_800477E8: measure, else keep local + FAKE.
  4 libcd: CdInit/CdMix/CdSetDebug (drop bb2.h's void spellings); CdlCB typedef; CdlLOC * for
    CdGetSector/CdIntToPos/CdPosToInt definitions (u8 * today).
  5 libapi/libetc/libc2/kernel: counter def-retypes; SetRCnt (368E4 codegen), putchar (prnt/puts codegen).
    Owner questions, not edits: rand(0x56) in 9F9C and func_800858D0(0) in 3AB48 pass arguments to (void)
    functions; EnterCriticalSection's value is used in libcomb/comb.c (SOTN: void); InterruptCallback is
    defined (void) and called with 2 args; SsUtSetReverbType's value used in 3AB48.
  6 game prototypes: ready 16 into bb2.h, then casts 9. Blocked 6 (arity: func_80019568, func_80044100,
    func_800486FC, func_80052C10, func_80040510; void value: func_8005B9FC) and codegen 2
    (game_GetPlayerData, snd_VabFakeOpen) need per-function asm evidence first.
  7 Unk80101EC8Record member access, one file per commit, smallest first: 3AB48 (41 exact, 0 SIGN),
    28708 (12 exact, 6 SIGN), 17AFC (122 exact, 12 SIGN, LB; split by function block), 9F9C (158 exact,
    8 SIGN; split). Retype the file's typed-callers parameters (protos.tsv) in the same commit. pad 4 -> new
    unk_XX members; size/inside 8 -> read the asm width first.
  8 small structs, one per commit: RevParamEntry (s_sra.c 32 exact; the type is local to s_srmp.c: move it
    to a libspu internal header first), S_6A880 (51268, 46), Rec44 (9F9C, 16+3 SIGN), MotionFrame (9F9C, 12),
    PadState (28708, 4+3 SIGN), SceneRec (2B344), Unk80101DF0Record (31D3C, GP), DRAWENV/DISPENV (ext.c,
    all SIGN).
  9 GameObj (5ED34, GP): func_80072BC4/func_80072CD4 (57 sites) write a POLY_G4 through byte casts; retype
    the parameter to POLY_G4 * rather than extending GameObj (an m2c layout, not evidence).
  Not planned: untyped/scratchpad/unknown sites; bb2.h single-TU entries (optional, any time).

LIMITS: token heuristics (macros unexpanded); struct names are a union over TUs (clashing local names, e.g.
Vec3/EnvA, take the first layout seen); check k/n of caller/assign rows before trusting them.
