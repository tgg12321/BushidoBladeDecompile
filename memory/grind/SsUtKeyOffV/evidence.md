# Evidence bank — SsUtKeyOffV

## [s1] 2026-09-08 — recon — floor 30 (asm) -> 13 -> 12 -> 0

E0 OBJECT MODEL: both DATA MODEL flags resolved, measured at score 0.
  - `_snd_ev_flag` @800FF630: MATCHES. Declared `extern s32 _snd_ev_flag;` at
    src/main.c:227 (TU-local extern, no include/*.h declaration; the same TU already
    matches SsSeqCalledTbyT / SsUtGetVBaddrInSB neighbours through it). Target reads
    it as a 32-bit `lw`, writes `sw` — s32 is the right width. No header change needed.
  - `D_8010280A` @8010280A (census "g_weapon_frame_idx"): MATCHES as `_svm_cur.voice`,
    the `short voice` member at +0x1A of `struct struct_svm _svm_cur` (include/sound.h:13-28,
    base 0x801027F0, aggregate-merge family, owner ruling 2026-08-17). The census name
    "current weapon-frame index" is a misnomer — the store here is the LIBSND vmanager's
    current-voice slot written before `_SsVmKeyOffNow(0)`, exactly as psyz's SsUtKeyOff
    does (`_svm_cur.voice = voice;`). `sh $a1,%lo(D_8010280A)` reproduced byte-exact.
    No new symbol declaration was introduced; the splat name should eventually be
    retired by the naming tool, but that is outside this session's surface.

E1 Provenance: LIBSND/UT_KEYV (memory/closer/psyq-library-census.md:324): the 4 opcode
  diffs vs PsyQ 4.0 all sit in SsUtKeyOnV; SsUtKeyOffV's 30 words are in the verbatim
  part. psyz (tmp/psyq_prov/psyz/decomp/src/libsnd/ut_keyv.c:70) has SsUtKeyOffV as
  INCLUDE_ASM — no direct seed — but its matched SsUtKeyOnV (same file, lines 3-68) and
  SsUtKeyOff (ut_key.c:76-98) fix the idioms: `voice < 0 || voice >= NUM_VOICES` /
  `voice >= 0 && voice < NUM_VOICES` range checks, `_svm_cur.voice = voice;
  _SsVmKeyOffNow(0);`, `_snd_ev_flag` set/clear bracket, `short` return.

E2 Canonical gate: pure-C target (distance 13 <= 50 on the first C form).

E3 The $a0/$a1 split (target: `addu $a1,$a0,$zero` at entry, `andi $v0,$a0,0xffff` for the
  compare, `sh $a1` for the store). Mechanism (dumps tmp/grind/SsUtKeyOffV/s1/main.cse.*):
  assign_parms gives the `short` parm TWO pseudos — `reg73:SI = $a0` then
  `reg72:HI = subreg(reg73)` (DECL_RTL). `(u16)voice >= 24` expands as
  `zero_extend(reg72)` -> reg72 is the only parm reg used -> it takes $a0 itself (no copy,
  score 13). `voice < 0 || voice >= 24` goes through fold_range_test; the resulting
  unsigned compare is resolved by cse to the incoming $a0 value while the store keeps
  reg72 -> $a0 stays live to the compare, reg72 conflicts with it and lands in $a1, and
  the entry copy materialises. Measured: 13 -> 12 with this one change, disassembly
  showed exactly `move a1,a0 / andi v0,a0 / sh a1`. Same effect from
  `voice >= 0 && voice < 24` (final form).

E4 The tail layout residual (score 12 -> 0). Target pre-reorg layout is
  `if (in range) goto K; [sw 0->flag; v0=-1; jump Lend]; K: sh; call; v0=0; sw 0->flag; Lend:`
  — error block INLINE ending in a jump, success block FALLS INTO the epilogue.
  Our error-first forms produced the mirror image (`beqz -> error at tail`, success block
  ends in `j Lend`, the two `return -1` tails merged into one `li v0,-1`).
  Pass attribution from the .jump/.jump2/.greg dumps:
  - jump1's "if (foo) bar; else break;" range swap (tools/gcc-2.7.2/jump.c:1826-1900)
    fires whenever `condjump -> label1; range1 ...jump label2; label1: range2 ...jump; barrier; label2:`
    and SWAPS range1/range2 — so whichever arm is written FIRST ends up in the tail.
    With error-first source the error block becomes the fall-through into Lend.
  - With the error block adjacent to Lend it ends in no jump; sched1 keeps
    `sw zero; v0=-1` in source order (no successor to prioritise), so jump2's
    `find_cross_jump(min 2)` sees `[v0=-1; jump Lend]` in both `return -1` paths and merges
    them (jump.c:1996-2030) -> one shared `li v0,-1`, and the first-path `bne` is inverted
    to branch straight at it.
  - With SUCCESS-first source (final form) the swap moves the success block to the tail,
    the error block keeps its `jump Lend`; the jump has REG_DEP_ANTI arcs on both
    `sw zero,flag` and `v0=-1` and sched1 orders `v0=-1` ahead of the store (same rule that
    put `v0=0` ahead of the success path's store in every build); the suffixes are now
    `[sw; jump]` vs `[v0=-1; jump]` -> only the jump matches -> no cross-jump; reorg then
    steals `v0=-1` from the fall-through into the `bnez` slot and the error path's `j Lend`
    gets a nop (the `sw` is an 8-byte lui/sw macro, never a slot filler). The success
    block's `jump Lend`, now adjacent to Lend, is dropped. All 30 words match.

E5 Byte-identical-to-each-other (all score 12): flat `voice < 0 || voice >= 24` error-first;
  the same with an explicit `else` arm around the success path; the nested
  `if (_snd_ev_flag != 1) { ...; if (in range) { ...return 0; } flag = 0; } return -1;`
  psyz-SsUtKeyOff literal shape. The nesting does NOT by itself change the layout — the
  decisive lever is success-arm-first + a single trailing `return -1` after the flag clear.

Artifacts: tmp/grind/SsUtKeyOffV/s1/main.{cse,jump,jump2,greg,sched2}.SsUtKeyOffV.txt
(per-pass RTL of the score-13 form), tmp/grind/SsUtKeyOffV/dumps/ (score-0 form, full TU),
tmp/grind/SsUtKeyOffV/s1/v1_else.c v2_nested.c v5_kfirst.c, cycle.ps1 / variant.py / disasm.sh
(one-shot apply+sandbox+disassemble harness).
