# Evidence bank -- func_800335D8

## s1 (2026-09-08, recon) -- chassis: -mel -msoft-float, canonical cc1 tools/gcc-2.7.2/build/cc1

OBJECT MODEL: (every flagged symbol measured; body letters refer to tmp/grind/func_800335D8/s1/body*.c, scores are
`sandbox func_800335D8 --disable all` on THIS chassis)
- D_8008EBFC / D_8008EBFD -- MISMATCH (measured): the binary reads both at index cat*2 in the same block
  (asm/funcs/func_800335D8.s 0x80033660-0x80033694 and again EBFD at cat*2 after the call, 0x800336D0-0x800336D8);
  rodata 8008EBFC..EC07 = `2E 5C 2E 62 2E 62 2E 2E 32 62 2E 62` = 6 two-byte {a,b} records. Declared
  `typedef struct { u8 a; u8 b; } LeafThreshold; extern LeafThreshold D_8008EBFC[6];` (D_8008EBFD retired).
  Body A (per-use `(&D_8008EBFC)[idx]` puns with an `idx = cat*2` local) = 47; body C (identical control flow,
  struct decls) = 27. The 20-point drop is exactly the `idx` pseudo no longer living across the func_800325E0 call:
  with separate `[cat]` index expressions cse recomputes cat*2 after the multi-predecessor label (target's
  `sll $3,$17,1` after the call), and `cat` itself stays in $s1.
- D_8008EA44 / D_8008EA45 -- MISMATCH (measured, same body pair): read at (type-2)*2, same index for both bytes
  (0x8003378C-0x80033794 and 0x800337B4-0x800337BC); rodata 8008EA44..EA4D = `06 09 0F 0F 0A 0D 0A 0A 05 0F` =
  5 records for types 2..6. Declared `extern LeafThreshold D_8008EA44[5];` (D_8008EA45 retired). Note these ten
  bytes sit inside the region the D_8008EA00 16-byte-stride table spans (types 5/6 records at EA50/EA60), so the
  original rodata simply packs two tables; nothing in the function reads EA00 records 0..4.
- D_8008EBF4 -- MISMATCH (measured as part of C): indexed by cat (stride 1) at three sites; rodata
  `7C 7D 7D 7E 7C 7D 00 00` = 6 ids + word pad (GCC DATA_ALIGNMENT pads a u8[6] to the next word, EBFC).
  Declared `extern u8 D_8008EBF4[6];`.
- D_8008E914 (not flagged, TU-local extern) -- per-stage 8-byte row = 4 x {u8 type; u8 param}: `tbl = &D_8008E914 +
  stage*8`, walked at stride 2 with a second pointer at +1. Declared TU-locally `extern u8 D_8008E914[][8];` and
  used as `u8 *tbl = D_8008E914[D_800A36A4];` (byte-identical to the pointer-arithmetic spelling by construction:
  same sll 3 / addu).
- D_8008EA00 -- `extern s32 D_8008EA00[][4];` (16-byte records; `D_8008EA00[type]` decays to the s32* the callee
  takes) -- identical sll 4 / la / addu.
- D_800A3750 -- `extern s16 D_800A3750[4];` (4 entries: loop bound is base+8). Census says "~0x7B entries": WRONG,
  0x7B is the sound-slot id base (`0x7B + i`), not a size.
- D_800A3918[6] / D_80107850[6] (LeafPos) -- header decls from the func_80033550 landing are correct here too;
  D_800A391E is not referenced by the C (the loop-1 bound is D_800A3918+6, linking to the same word).

SCORER GAP (load-bearing for the disposition): engine/score.py `_symtab()` reads only cfg.LD_SYM_FILES
(undefined_funcs_auto.txt, undefined_syms_auto.txt, named_syms.txt). D_8008EBFC, D_8008EA44 and D_8008EBF4 are
defined ONLY as `dlabel`s in asm/data/7D920.data.s (lines 2705/3044/3055ff), so `_resolve_named_pair` cannot link
our `%hi/%lo(D_8008EBFC+1)` pairs, while the target side's `D_8008EBFD` (undefined_syms_auto.txt:42) resolves to
`@lo(0xebfd)`. Result: 3 pairs x 2 insns = a FALSE +6 on every pair-struct spelling
(tmp/grind/func_800335D8/s1/sdiffbodyI3.txt shows exactly those 6 lines and nothing else). The linked words are
identical: verify-oracle --rebuild --allow-dirty with the merge applied = SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa
== oracle (s1/verify_oracle_J_full.txt); the restored tree rebuilt clean afterwards (s1/verify_oracle_restore_full.txt).
[s2 note: CLOSED without an engine edit -- see the s2 entry below.]

CHASSIS: -msoft-float halves loop.c's LICM threshold (n_non_fixed_regs 60 -> 28; with-call threshold 61 -> 29).
The retired brief's two "spurious hoists" (li s5,1 and la D_800A391E in loop 1) are gone at HEAD: loop 1
(50 insns) now refuses `savings 1 life 1` movables (s1/bodyD.loop). Loop 2 (71-74 insns) refuses them too, which
is why the target's hoisted `1` ($s5) needs a SECOND use of the constant: `buf[i] = 1` in the type-1 arm. The loop
dump then shows `Insn 259: regno 155 (life 2), savings 2 moved` + `Insn 297 ... matches 259` (combine_movables),
and the store still emits `sh $3` (type reg, cse jump-equivalence), exactly target.

LOOP SHAPES (all measured, s1/diffbody*.txt / sdiffbody*.txt):
- Loop 1 must be index-based (`for (i = 0; i < 6; i++)` over D_800A3918[i] / D_80107850[i]): the u8 array address
  takes GCC's MIPS "constant + register" pseudo-address form, so the eliminated biv's bound is the constant
  D_800A3918+6 (target's `la $2, D_800A391E`) and the compare keeps the biv's SIGNED `slt`. The pointer walk gives
  `sltu` (2 points). B3: 47 -> 46 alone; inside D/F it is the remaining loop-1 delta.
- Loop 2 must be index-based over a POINTER LOCAL `s16 *buf = D_800A3750` (reg add_val -> bound `addiu $2,$20,8`,
  giv init `addu $16,$20,$0`, `slt`), with tbl as a pointer biv (`tbl += 2` in the for-increment) and the param
  pointer a per-iteration local `u8 *data = tbl + 1;` (a giv of tbl: its init `addiu $18,$19,1` is emitted by loop.c
  AFTER the hoisted `1`, which is what puts `li $21,1` first in the preheader and in the beqz delay slot). The
  counter is `0x7B + i` at both call sites (giv; its `li 123` init also lands after the movable). F2/F3/G1/H1: 16 -> 14 -> 14 -> 12.
- The a0/a1 seats of adj*2 and (s16)count (ra_solver: pseudo 169 pri 15000 hard_conf [2,3,4,29] -> a1; 182 pri
  13333 -> a0) are fixed by writing the ratio inline `((scount - limit) << 10) / (D_8008EA44[adj].b * 30)`: the
  subtraction then precedes the `.b` load in RTL, scount dies before the load's local pseudo takes a0, and scount
  gets a0 itself. I2/I3: 12 -> 6 (= the scorer floor). ra_solver model: tmp/ra_solver_work/func_800335D8.model.json,
  trace s1/ra_trace.txt (sort order MATCH, 16/16 dispositions).
- `(u16)buf[i] + 1` and `*(u16 *)&buf[i] + 1` are byte-identical (F3.o == F4.o); the cast form is banked.
- Removing the unused `loop_bottom:` label is byte-identical (K2.o == J.o); K2 is the banked body.

CANONICAL: verdict C (pure-C distance 47 at session start with body A; asm_insns 0). No hand-coded signals
(retired brief: tier LOW 0/8).

## s2 (2026-09-08, recon) -- chassis: -mel -msoft-float, canonical cc1 tools/gcc-2.7.2/build/cc1

BASELINE: the function was INCLUDE_ASM at HEAD (src/code6cac_b.c:3043) with the s1 integration patch NOT applied
(the ledger + scope grant were committed in 6f22a368, the code was not), which is why the driver's chassis check
printed "measurement unavailable". With the s1 patch (tmp/grind/func_800335D8/s1/integration_surfaces.patch, CRLF ->
converted to LF as s2/integration_surfaces.lf.patch before `git apply`) + the K2 body applied: canonical verdict C,
asm_insns 0, total 176, distance 0.

SCORER GAP CLOSED WITHOUT AN ENGINE EDIT (measured): engine/score.py `_symtab()` (score.py:80-93) is NAME-keyed and
reads cfg.LD_SYM_FILES only. The C side emits `%hi/%lo(D_8008EBFC+1)` and `(D_8008EA44+1)`; the reference side
(asm/funcs/func_800335D8.s assembled into build/src/code6cac_b.o, engine/sandbox.py:72) emits `%hi/%lo(D_8008EBFD)` /
`(D_8008EA45)`. `_resolve_named_pair` (score.py:109) needs BOTH names in the table to normalise both spellings to
the same @lo(0xebfd) / @lo(0xea45) tokens. Fix INSIDE the scope grant: undefined_syms_auto.txt gains
`D_8008EA44 = 0x8008EA44;` (line 37) and `D_8008EBFC = 0x8008EBFC;` (line 43) -- the addresses the
asm/data/7D920.data.s dlabels (lines 2705, 3057) already define, so the linker-script assignment and the object
definition agree (the same "row + dlabel" coexistence D_8008EBFD, D_800A3918 and D_80107850 already have:
undefined_syms_auto.txt:44/268/991 + their dlabels). The per-word rows D_8008EA45 (line 38) and D_8008EBFD (line 44)
are KEPT with the prong (c) amendment-2026-09-03 suffix (`/* alias of <base>+1; retire with func_800335D8 ... */`,
.claude/rules/no-new-park-categories.md:252) because asm/funcs/func_800335D8.s is their ONLY referrer
(`grep -rl` over asm/funcs: one file) and the sandbox reference object is assembled from it; no C code names them
(header decls removed; named_syms.txt g_leaf_action_threshold_b / g_threshold_lookup_EA45 rows removed).
  - K2 body + base rows: sandbox 0, 176/176, rules_dropped 0                  (s2/sandbox_K2_rows.txt)
  - K2 body, base rows removed, nothing else changed: sandbox 6 (ABLATION)     (s2/sandbox_K2_no_base_rows.txt)
  - verify-oracle --rebuild --allow-dirty, K2 + rows: ok true,
    build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle             (s2/verify_oracle_K2_rows_full.txt)

BODY K3 (final, in src/ and candidate.c): the brief's auto-scan flagged `(s32 *)&D_80107850[i]` (3 call sites) as a
use-site pun. The callee func_800325E0(s32, s32 *) (src/code6cac_b.c:2676, canonical-asm sibling) reads a 3-word
position through its pointer; LeafPos is `{s32 x, y, z}` (include/code6cac.h:487-491). Respelled as
`&D_80107850[i].x` (no cast) -- the record's first coordinate. Byte-identical: sandbox 0, 176/176
(s2/sandbox_K3_nocast.txt); verify-oracle --rebuild --allow-dirty ok true, SHA1 == oracle (s2/verify_oracle_K3_full.txt).

SIBLINGS (contract): func_80033550 (COMPLETED-C, on main at src/code6cac_b.c:3033-3041) and func_800325E0
(COMPLETED-INLINE-ASM-CANONICAL, src/code6cac_b.c:2676) have no memory/grind ledgers left (closed and removed);
their candidates are the bodies on main. Blocks shared with func_80033550: the declarations `extern u8 D_800A3918[6]`
/ `extern LeafPos D_80107850[6]` and the `D_80107850[i]` record indexing -- this candidate uses exactly that spelling
in loop 1, measured inside the 0. Block shared with func_800325E0: its `s32 *` position parameter -- K3 passes
`&D_80107850[i].x`.

LINE ENDINGS: all four edited build inputs (src/code6cac_b.c, include/code6cac.h, undefined_syms_auto.txt,
named_syms.txt) verified LF-only (`grep -c $'\r'` = 0) after every edit.
