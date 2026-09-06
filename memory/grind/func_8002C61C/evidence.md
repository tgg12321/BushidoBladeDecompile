# Evidence bank — func_8002C61C

## s1 (2026-09-06, recon) — chassis: HEAD asm-until-matched, cc1 -mel, no FAKE constructs anywhere

- BASELINE: retired-chassis body (`retired-chassis-2026-08/body.c`, pinned floor 44 on the old
  chassis) installed verbatim measures **46** on this chassis (`sandbox --disable all`,
  284/284 insns). canonical verdict: C (pure-C distance 46 <= 50). No duplicate/sibling lead in
  `tmp/duplicates_leads.txt`.

- OBJECT MODEL: (measured with cc1 micro-probes in `tmp/grind/func_8002C61C/s1/l3*.c` and full
  sandbox bodies; every probe's .s is in that directory)
  - `D_80101EC8` (census: char/practice record table, stride 0x44C): MATCHES as a base pointer
    (`u8 *s1 = &D_80101EC8; s0 = s1 + 0x44C;` gives the target's `$s1/$s0` base-register loads
    at +0x3C/+0x286/+0xC/+0xF8/+0x28C — that whole middle section is byte-exact). A
    struct-array index for the loop-3 stores is a MISMATCH — see next item.
  - `D_8010203C/40/44` (midpoint) and `D_80102054/58/5C` (centroid), flagged INDEXED-ACCESS
    "declare as array": MISMATCH-measured for every declaration-level spelling. The target
    addressing is `lui $at,%hi(SYM); addu $at,$at,$a2; sw %lo(SYM)($at)` with `$a2` = i*0x44C
    starting at 0 (`move a2,zero` emitted AFTER the hoisted 0x55555556 constant and the source
    giv init, i.e. `$a2` is a loop.c-reduced giv, not a source-initialised biv). Mechanism
    (loop.c): an address `(plus (reg t) (const SYM+c))` where `t` is a strength-reducible giv is
    itself recorded as a DEST_ADDR giv (simplify_giv_expr accepts symbolic invariants as USE,
    `tools/gcc-2.7.2/loop.c` simplify_giv_expr default case) and each of the six then gets its
    own `la SYM` IV (different symbols cannot be combined, combine_givs_p) — measured:
    `(&D_80102054)[i * 275]` -> 6 `la` IVs (l3b); `((Rec *)&D_80101EC8)[i].cen.x` -> one
    `la D_80101EC8` IV with +396 offsets (l3a, if_idx body score 80); `g_chars[i*0x44C+0x18C]`
    on a `u8[]` -> `la g_chars+396` IV (l3v4); record pointer `ch = (Rec*)((u8*)&D_80101EC8+off);
    ch->cen.x` -> `la D_80101EC8` IV (l3v5). The ONLY spelling producing symbol+register
    addressing with the register starting at 0 is a NAMED byte-offset local written once per
    iteration, `off = i * 0x44C;` (equivalently `i * sizeof(rec)`), used as
    `*(s32 *)((u8 *)&D_80102054 + off)` (l3v1/l3v2) or `*(s32 *)((u8 *)&D_80101EC8 + off + 0x18C)`
    (l3v6, identical linked bytes: `sw $2,D_80101EC8+396($6)`). Inline `i * 0x44C` at each use
    (l3v3) reverts to 6 `la` IVs. So the honest declaration fix the brief asks for is
    byte-infeasible on this compiler for these six fields; the census object model (fields at
    +0x174/+0x18C of the 0x44C record) is CORRECT, but it has to be spelled as base + byte
    offset. Ruling needed (see outcome).
  - `D_801020D8` (+0x210 vec3 A) / `D_801020FC` (+0x234 vec3 B): MATCHES as `Vec3i *` bases
    (`Vec3i *dst_a = (Vec3i *)&D_801020D8; dst_b = (Vec3i *)((u8 *)&D_801020D8 + 0x44C)`) with
    index loops `dst_a[i] = src[i]; dst_b[i] = src[i + 3];` — loop 1 and loop 2 byte-exact
    (loop 2: `src` base 0x1F800000 with indices i+6 / i+8; a 0x1F800048/0x1F800060 base gives
    `ori` + wrong offsets).
  - `D_80101F32` (+0x6A, u16 mode, TU-local `extern u16` at src/code6cac_b.c:46): MATCHES as a
    u16 read into a local `mode`. `D_8010237E` (P2 +0x6A): MATCHES as `extern u16` (single
    `lhu` compare against 5, byte-exact). `D_800A389C` (split piece of g_match_round_byte_b):
    the single `sh $zero,%lo(D_800A389C)` store is byte-exact under the current `extern u16`;
    aggregate-merge is a header/integration question, not a codegen residual here.
  - `D_1F80000C`: NOT a real reference. splat paired `lui $a2,0x1F80` with the loop increment
    `addiu $a2,$a2,0xC` and emitted `%lo(D_1F80000C)` at 8002C8B4 and 8002C914. The linked word
    is 0x24C6000C either way; C emits the literal 12. (Same artifact 5x in still-INCLUDE_ASM
    sibling func_80029454.s — sibling-ledger note.)

- DISPATCH HEAD (`andi $a0,$v1,0xffff` + raw `$v1` in the range subtractions): reproduced
  byte-exact by the plain equality chain
  `mode == 0xF || mode == 0x1C || mode == 0x1D || mode == 0x1E || mode == 0x1F || mode == 0x20 || mode == 0x21`
  (micro-probe d4) and by `(mode >= 0x1C && mode <= 0x1D) || ...` (d5). Mechanism: fold-const
  merges each adjacent-constant pair into `(u16)(mode - c) <= 1` in HImode, so the range tests
  use the un-extended `lhu` register while the `==` tests need the zero_extend (`andi`). The
  retired `(u32)(mode - 0x1C) < 2` spelling (d1) and direct-global reads (d2) compute in SImode
  and lose the `andi`. `switch` with the same cases gives a balanced `slt` tree (sw_ptr body
  58, sw3_ptr 62) — dead.

- LOOP 1/2 PREHEADER (`la v0; addiu t0,v0,0x44C; move a3,v0; lui a2` + `i = 0` first): comes from
  loop.c giv inits for index-based block copies — `dst_a[i]` / `dst_b[i]` become reduced IVs
  initialised from the shared `&D_801020D8` pseudo (`move a3,v0`, `addiu t0,v0,0x44C`); the
  retired pointer-increment loops had no such copies (residual 2x5 insns).

- LOOP 3 SOURCE BASE (`ori a1,a1,0xEC`, offsets -0x38..0, `a1 += 0x108`): produced by index
  access on a constant-based scratchpad struct (`((ProbeScr *)0x1F800078)[i].j[5..9]`, l3a) —
  loop.c combines the 15 load givs into one IV whose base is the LAST-recorded giv (giv list is
  built by prepending; head = last insn's giv = j[9].z at 0x1F8000EC). Equivalent: pointer biv
  `a1 = (s32 *)0x1F8000F0; a1[-15..-1]; a1 += 66` (full_B, also 4). The retired
  `a1 = 0x1F8000EC; a1[-14..0]` form uses the biv directly for a1[0], so the other 14 givs are
  reduced into a separate IV based at 0xE0 (the retired build's `$5`/`$8` split).

- RESULT: full form (d4 dispatch + index loops 1/2 + loop 3 with `off = i * 0x44C;` in the body)
  = `memory/grind/func_8002C61C/candidate.c` measures **2** (`tmp/grind/func_8002C61C/s1/body_full_C.c`).
  The 2 are exactly the two `addiu a2,a2,%lo(D_1F80000C)` vs `addiu a2,a2,12` lines
  (`tmp/grind/func_8002C61C/s1/sdiff.py` output): score.py's `_resolve_named_pair` needs a
  paired R_MIPS_HI16 `lui` to rewrite the LO16 immediate to its linked value and there is none
  (the lui is a literal `0x1F80`), so the reference immediate stays 0 while the C build emits
  the literal 12. Object-level byte proof `tmp/grind/func_8002C61C/s1/byteproof.py`: 284/284
  insns, 221 words identical, 63 differ only in unresolved relocation fields, 0 real
  mismatches; both increments are `24c6000c` in build and target. `verify-oracle` (non-rebuild)
  reported build_matches=true but against a STALE build/ object (older than the src edit);
  `verify-oracle --rebuild` refuses on dirty src (correctly — it would corrupt the sandbox
  reference). Full-build SHA1 with this C is therefore unproven in-session; the driver's own
  re-verify is the remaining gate. The sandbox CANNOT print 0 for this function from any C:
  the fix is either the two `.s` lines (asm/funcs/func_8002C61C.s, replace `%lo(D_1F80000C)`
  with `0xC`, byte-neutral) or score.py resolving an unpaired LO16 against a known absolute
  symbol — both outside session scope.
