# func_8002DAD0 — evidence log

## [s1] recon session — object model + structural derivation

**OBJECT MODEL:** `D_8008D118` — census row flags this as a byte LUT
("distance-squared magnitude/log2 lookup"), no header declaration. MATCHES:
the established codebase idiom is `extern u8 D_8008D118;` used as
`*(&D_8008D118 + idx)` / `*(((u8 *)&D_8008D118) + idx)` pointer arithmetic
(confirmed at src/code6cac.c:19,756,783 and, decisively, already present
in THIS TU at src/code6cac_b.c:278 and reused by 6+ sibling functions in
the same file, including the byte-identical shape used by func_8002E838
at line 1468/1490). No declaration-fix hypothesis needed — the extern
already exists in scope for func_8002DAD0's insertion point; candidate.c
uses it unchanged. This satisfies the mandatory DATA MODEL check for this
session.

**Cluster membership confirmed.** `.claude/rules/cop2-addressing-preamble-cluster.md`
lists func_8002DAD0 as an active member (LZCS site at old L125) of the
28-function `addu $t4,$aN,$zero`+cop2 cluster (owner ruling 2026-08-17,
widened 2026-09-01). The function actually carries FIVE such preamble
islands, not one: (1) a partial gte_SetRotMatrix-shaped preamble loading
only 3 words into ctrl regs 0/2/4 (R11,R22,R33 diagonal) from vecA,
feeding (2) a GTE `OP` (outer/cross-product, opcode 0x0C, raw word
0x4B70000C — NOT the MVMVA the rest of the function uses) fed by IR1-3
loaded directly (lwc2 $9/$10/$11) from vecB, storing the raw MAC1-3 cross
product to obj+0xC8/CC/D0 via gte_stlvnl; (3) the LZCS/LZCR leading-zero
block, byte-identical to func_8001A67C (src/code6cac.c) and func_8002E838
(this file, line ~1472); (4) a FULL 5-word gte_SetRotMatrix loading the
just-computed rotation matrix (built via RotMatrixY+RotMatrixX on an
identity, at obj+0xD8) into ctrl 0-4; (5)/(6) two gte_ldv0+MVMVA
(sf=1,mx=rotation,v=V0 — word 0x4A486012, same as func_800203B4 and
func_8002E838)+gte_stlvnl sequences that rotate vecA and vecB in place.
All six islands are verbatim reuses of PsyQ SDK macro bodies already
authorized/matched in func_800203B4 (src/code6cac.c) and, even more
directly, func_8002E838 (src/code6cac_b.c:1451-1552) — func_8002E838 is
almost exactly the SECOND HALF of this function's algorithm (single-vector
version: one ratan2, no cross-product/OP stage, no second gte_ldv0/MVMVA
pass). func_8002E838's matched C was used as the direct template for the
LZCS block, the identity-matrix stores, and the RotMatrixY/RotMatrixX call
shape in this function's candidate — all copied verbatim (same field
offsets 0xD8-0xE8, 0xF8, 0xFA, same table-lookup shape for `dist`).

**Structural derivation (algorithm).** `obj` is an unspecified struct (no
existing prototype for func_8002DAD0 anywhere in the tree — first
declaration). Fields used: 0x60/0x64/0x68 = three `s32*` position
pointers (p0=self/origin, p1, p2); 0xA8/0xAC/0xB0 = vecA = p1-p0 (s32[3]);
0xB8/0xBC/0xC0 = vecB = p2-p0 (s32[3]); 0xC8/0xCC/0xD0 = cross = vecA×vecB
(computed via GTE OP, then reused in-place as scratch: >>=6 on each axis,
then ratan2 inputs); 0xD8..0xE8 = a 3x3 rotation matrix (5 packed s32
words, later reinterpreted as the standard 20-byte MATRIX layout);
0xF8/0xFA = two s16 angle fields (0x800-angle1, 0x800-angle2). Verified by
finding func_8002E838 (same file) implementing the exact same
distance/log2/RotMatrixY+X/gte_SetRotMatrix+ldv0+MVMVA+stlvnl tail with
identical field offsets and byte-identical matched C — this function is
almost certainly the two-point ("aim/orient toward two targets") sibling
of that one-point function. kengo naming hint: `sa_tan0/saTan0KiWareMoveA`
(character "santan"/"sa_tan0" move-break motion).

**Measured floor drop this session: 204 → 44 → 25** (see hypotheses below
for each step). `sandbox --disable all --diff` confirms the residual is
ENTIRELY inside one straight-line block (target insns 88-119, the
dist_sq/log2 computation immediately after the first ratan2 call) — every
island, the two vector-diff blocks, the near-zero-cross bailout, the
identity-matrix stores, both RotMatrix calls, and both tail
gte_ldv0+MVMVA+stlvnl rotations are ALREADY byte-exact (confirmed by the
hunk list touching only offsets 0/3/44-56/70-119, nothing beyond insn 119
except the tail frame-size hunks 18-23 which were ALSO fixed this
session — see hypothesis 2). Remaining hunks are 5 source-level + 5
operand-only, all inside the dist_sq scratch-block reload pattern.

- [s1] OBJECT MODEL: D_8008D118 MATCHES the established declared shape -- extern u8 D_8008D118; already declared in this TU (src/code6cac_b.c:278) and used via *(&D_8008D118 + idx) / *(((u8*)&D_8008D118) + idx) pointer arithmetic by 6+ sibling functions in the same file, including the byte-identical shape in the matched func_8002E838. No declaration fix needed; candidate.c reuses the existing extern unchanged.

- [s1] func_8002DAD0 is a confirmed member of the cop2-addressing-preamble-cluster (28-function, owner ruling 2026-08-17 / widened 2026-09-01) but carries SIX preamble/op islands, not the one enumerated in the static table: a partial 3-word gte_SetRotMatrix-shaped preamble (ctrl regs 0/2/4 only) feeding a GTE OP (outer/cross-product, word 0x4B70000C) fed by IR1-3 loaded directly from vecB; the LZCS/LZCR block (byte-identical to func_8001A67C and func_8002E838); a full 5-word gte_SetRotMatrix; and two gte_ldv0+MVMVA(word 0x4A486012)+gte_stlvnl sequences that rotate vecA and vecB in place.

- [s1] func_8002E838 (src/code6cac_b.c:1451-1552, already COMPLETED-C in this same file) is structurally the single-vector half of this function's algorithm -- same field offsets (0xD8-0xE8 identity matrix, 0xF8/0xFA angle fields, D_8008D118 log2 table), same RotMatrixY-then-RotMatrixX call shape, same gte_SetRotMatrix/gte_ldv0/MVMVA/gte_stlvnl tail. Its matched C was used as the direct, byte-verified template for over half of this function's candidate body.

- [s1] sandbox --disable all --diff at floor 25 shows the residual is entirely contained in one straight-line block (target insns 88-119): after the first ratan2() call, obj+0xC8 is shifted right 6 and stored, then RELOADED from memory for the subsequent multiply in target (sra v1,v1,6; sw v1,200(s1); lw v1,200(s1); mult v1,v1) -- our candidate's structurally-identical-looking C (store then re-read the same fixed offset) does NOT reproduce this reload; GCC's cse/register-allocation keeps the value in a register instead. This is the open frontier item.

- [s1] No naming/prototype for func_8002DAD0 existed anywhere in the tree before this session (first declaration); parameter is treated as u8 *obj matching the sibling func_8002E838's own parameter style in the same file.

## [s2] dist_sq scratch-block: CSE invalidation mechanism found + spent, floor 25 -> 6

**Root cause of the H3 "phantom reload" (RESOLVED, see hypotheses.md H3):**
read `tools/gcc-2.7.2/cse.c` directly this session.
`note_mem_written` (cse.c:7539-7579): for a `SET (MEM (PLUS reg const)) ...`
store (any pointer-relative address, SImode, not QImode) sets
`writes_ptr->var = 1` unconditionally, and additionally sets
`writes_ptr->nonscalar = 1` (the `all` flag stays 0 for a plain `PLUS`
address — only BLKmode/scratch-address stores set `all`).
`invalidate_from_clobbers` then calls `invalidate_memory(w)` whenever
`w->var` is set — i.e. after ANY pointer-relative store.
`invalidate_memory` (cse.c:1701-1719) removes every hash-table entry `p`
with `p->in_memory` set when `all || (nonscalar && p->in_struct) ||
cse_rtx_addr_varies_p(p->exp)` — and `cse_rtx_addr_varies_p` is TRUE for
essentially any entry whose own expression is a pointer-relative MEM,
**regardless of whether its address has any relationship to the store
that triggered invalidation.** So: ANY second pointer-relative store
(even to a totally unrelated field) wipes the cache entry created by an
earlier pointer-relative store, forcing a genuine reload on the next read
of the EARLIER address. The insertion of the CURRENT store's own MEM=reg
equivalence happens AFTER invalidation, so it always survives until the
NEXT store (of anything, any address).

**Applied structure (candidate.c, current floor-6 body):**
```c
angle1 = ratan2(*(obj+0xC8), *(obj+0xD0));
*(obj+0xC8) = *(obj+0xC8) >> 6;      /* write 1: inserts C8 cache entry */
*(obj+0xCC) = *(obj+0xCC) >> 6;      /* write 2: nukes C8 entry (any addr) */
{
    s32 dz = *(obj+0xD0) >> 6;        /* plain register read, no memory write yet */
    dist_sq = *(obj+0xC8) * *(obj+0xC8)   /* C8 read: cache MISS -> genuine reload */
            + dz * dz;                     /* dz: ordinary register reuse, no memory op */
    *(obj+0xD0) = dz;                 /* deferred D0 store -- scheduler is free to push
                                          this arbitrarily late since alias analysis
                                          proves no real conflict with C8/CC */
}
```
Measured: `sandbox --disable all --diff` -> `target_insns=204,
build_insns=204`, 9 hunks, **0 source-level**, 5 operand-only (a0<->a1
register-seat rename on dist_sq, propagated through
sltiu/addu/bltz/move/srlv), 4 not-scored (masked branch-target-address
cascade, explicitly flagged "do NOT chase").

**Two orderings measured IDENTICAL at floor 6** (both produce byte-for-byte
the same object, same score):
1. CC's write immediately after C8's write, before the `{ }` block (the
   form kept in candidate.c — simpler to read).
2. CC's write moved INSIDE the `{ }` block, after `dz`'s read but before
   the `dist_sq=` statement.
Both satisfy the same invalidation-ordering requirement (a second
pointer-relative write occurs between C8's write and C8's read), hence
identical codegen. This rules out "CC's exact textual position relative
to dz" as a lever for the remaining a0-vs-a1 tie — the next session needs
a DIFFERENT change (see frontier item 1: find what stamps `preferences: 5`
onto dist_sq's pseudo in local-alloc.c, per the `.greg` dump numbers
72/75/78/129/130/140 captured this session at
tmp/grind/func_8002DAD0/dumps/code6cac_b.greg:11463-11500).

**Sub-hypotheses tried and rejected during the search for the H3 mechanism**
(kept for the record, not to be re-tried in the same form):
- Two-shift-then-separate-`dist_sq+=`-statements (write C8, `dist_sq=C8*C8`,
  write D0, `dist_sq+=D0*D0`, write CC): floor 10 (WORSE than the 25
  baseline was 25->10 not an improvement path) — confirmed no reload
  occurs when the invalidating write comes AFTER the read it would need to
  invalidate; this measurement is what pinned down that invalidation must
  happen BEFORE the read, not merely "somewhere in the block".
- All-three-writes-upfront-then-one-combined-sum (C8, D0, CC writes, then
  one `dist_sq=C8*C8+D0*D0` expression, using a real D0 write instead of a
  deferred temp): floor 10, 3 source-level hunks (D0's own reload
  reappeared, since a THIRD write (CC) after D0's own write invalidates
  D0's entry too, before D0 is read for its own square) — this is what
  motivated H4's insight that D0 must stay as a deferred-store TEMP (never
  written to memory before its own read) rather than a second immediate
  write.

- [s2] sandbox --disable all --diff at the new floor 6 shows 9 hunks: 0 source-level, 5 operand-only (a0<->a1 rename on dist_sq's final pseudo), 4 not-scored (masked branch-target-address cascade artifacts) -- the C is no longer 'saying something different' from target; the remaining gap is a pure register-allocation seat choice.

- [s2] tools/gcc-2.7.2/cse.c:1701-1719 (invalidate_memory) and cse.c:7539-7579 (note_mem_written) are the exact GCC-2.7.2 CSE mechanism that explains BOTH why target genuinely reloads obj+0xC8 after storing it AND why obj+0xD0 does not reload after its own store -- read directly this session, not inferred from dumps alone.

- [s2] .greg dump captured this session (tmp/grind/func_8002DAD0/dumps/code6cac_b.greg, function header at line 11463) shows 6 pseudos needing allocation (72 75 78 129 130 140); pseudo 75 (likely dist_sq) has preferences:5 and no conflict vs hard reg 4; pseudo 78 has preferences:2,5 and DOES conflict with hard reg 4 -- this is the concrete data the next session's register-seat lever needs to trace back to its source copy/call insn in local-alloc.c.

- [s2] Two independent textual orderings of the CC-write/dz-read/dist_sq-statement sequence produced byte-IDENTICAL objects at floor 6, ruling out 'CC's exact position relative to dz' as a further lever for the remaining tie.

- [s3] Chassis-check confirmed at dispatch: memory/grind/func_8002DAD0/candidate.c measures floor 6 (matches ledger's session-2 recorded floor exactly).

- [s3] sandbox --disable all --diff (this session, baseline and after every reverted experiment): 9 hunks, 0 source-level, 5 operand-only, 4 not-scored; all 5 operand-only hunks are the same a0<->a1 register-seat tie on dist_sq's final pseudo propagated through sltiu/addu/bltz/move/srlv (target[108,113,117,119,129]).

- [s3] .lreg dump (tmp/grind/func_8002DAD0/dumps/code6cac_b.lreg, function at line 14028): dist_sq is pseudo 75, defined once at insn 166 (dist_sq = pseudo129 + pseudo130, the c8sq and dzsq products, both still unresolved pseudos at scan time), used at insns 183 (sltiu<1024 compare), 193 (true-branch table-lookup address), 225 (false-branch shift amount), 895/914 (false-branch <0 compare and table-lookup address).

- [s3] .greg dump (tmp/grind/func_8002DAD0/dumps/code6cac_b.greg, function at line 11463): allocno 75 (dist_sq) shows '75 conflicts: 72 75 140 2 3 12 29' and '75 preferences: 5'; allocno 78 (dist, the ratan2 2nd-arg value) shows '78 conflicts: 72 78 4 29' and '78 preferences: 2 5'; allocnos 129 (c8sq) and 130 (dzsq) show conflicts but NO preferences line at all.

- [s3] Hand-traced tools/gcc-2.7.2/global.c's set_preference (line 1670): insn 183's SET (dest resolves via reg_renumber to hard reg 2/v0, src=reg75 via LTU's XEXP(0) unwrap) DOES call set_preference and would stamp a preference for hard reg 2 onto allocno 75 -- but reg 2 is already in allocno 75's conflict list (75 conflicts with 2), so prune_preferences (global.c:882) removes it; insn 225 traces identically to the same pruned hard-reg-2 outcome. Neither traced insn explains the surviving hard-reg-5 preference.

- [s3] Traced expand_preferences (global.c:828, the REG_DEAD-adjacency merge): requires an insn whose SET_DEST is a global allocno (75 or 78) AND a REG_NOTES REG_DEAD entry on the OTHER allocno (78 or 75) in the SAME insn, with the two allocnos non-conflicting. Checked every insn setting 78 (insns 195, 238) and the only insn setting 75 (insn 166) -- none pairs a death of one with a set of the other, so this merge path is also ruled out for the 75<->78 pair directly.

- [s3] Ran the pre-existing BB2_ALLOC_DEBUG env-gated diagnostic (tools/gcc-2.7.2/global.c:379/605, NOT modified this session -- read-only instrumentation already present in the tree) via tmp/grind/func_8002DAD0/s3/allocdbg.py: allocation order is pseudo72(ord0,hardreg17) -> pseudo75(ord1,hardreg5,nrefs6,livelen13,pri9230) -> pseudo78(ord2,hardreg5,nrefs3,livelen5,pri6000) -> pseudo140(ord3,hardreg3) -> pseudo130(ord4,hardreg65/lo-spilled) -> pseudo129(ord5,hardreg6). seed_used (call-used registers) includes hard regs 4,5,6,7 (a0-a3) as 'ever used' baseline, not as a live-conflict fact.

- [s3] src/code6cac_b.c was reverted to the committed `INCLUDE_ASM("asm/funcs", func_8002DAD0);` line before this session ended (git status --short confirms clean, only the pre-existing metrics/events.jsonl dirt remains) -- per asm-until-matched, C lands on main only at COMPLETED-C; the working candidate lives only in memory/grind/func_8002DAD0/candidate.c (unchanged from session 2, still the floor-6 best-known form).

- [s4] Re-confirmed at session start via `sandbox func_8002DAD0 --disable all --diff` with candidate.c applied: floor still 6, 9 hunks, 0 source-level, 5 operand-only (the same a0<->a1 tie on dist_sq's final pseudo, target[108,113,117,119,129]), 4 not-scored — byte-identical to s2/s3's recorded diff shape. No drift since s2.
- [s4] Built a working per-function permuter workspace for the first time this ledger (`tmp/perm_8002DAD0/`): target.o is built from the RAW ORACLE EXE BYTES at file offset 0x1E2D0 (VA 0x8002DAD0, size 0x350) via `mipsel-linux-gnu-objcopy -I binary -O elf32-tradlittlemips -B mips:3000 --rename-section .data=.text,contents,alloc,load,readonly,code`, NOT from assembling `asm/funcs/func_8002DAD0.s` — that file's splat-emitted ASPSX GTE mnemonics (`mvmva 1,0,0,3,0`, `op 0`) have no GNU-`as`-compatible encoding and there is no maspsx-equivalent standalone-.s translator in this tree. This raw-bytes-via-objcopy technique is reusable for any future permuter workspace on a GTE-heavy function.
- [s4] Confirmed (A/B, both directions) that feeding a full-TU preprocessed `.c` file to `tools/gcc-2.7.2/build/cc1` as a FILENAME ARGUMENT (rather than piping/redirecting via stdin) can silently produce a corrupted/empty body for a LATER function in the same TU, even though the same content compiles that function correctly via stdin — reproduced specifically for func_8002DAD0 in src/code6cac_b.c (cc1's error-recovery from a pre-existing, unrelated parse error in func_80035280 earlier in the file propagates differently depending on stdin-vs-argv). The Makefile's real pipeline always pipes (`$(CPP) ... | $(CC1) ...`); any future standalone/workspace cc1 invocation on a full TU must do the same or risk silently-wrong output that still exits 0.
- [s4] No permuter search iterations were run this session (see hypotheses.md [s4] for the full blocker chain: unary_expression-only asm-operand grammar in decomp-permuter's pycparser fork, a Windows-Python CRLF-corruption trap, the stdin-vs-argv cc1 bug above, and an unresolved pycparser lexer "Illegal character" on adjacent multi-line asm string literals). The workspace is left in `tmp/perm_8002DAD0/` for a future session but its `base.c` is in a partially-edited, NOT-verified-safe state (see hypotheses.md Frontier update) — regenerate fresh rather than trusting it as-is.
- [s4] src/code6cac_b.c reverted to the committed `INCLUDE_ASM("asm/funcs", func_8002DAD0);` line before this session ended, per asm-until-matched; candidate.c (unchanged from s2/s3, still floor-6 best-known) remains the sole working copy.

- [s4] sandbox func_8002DAD0 --disable all --diff at session start: floor 6, 9 hunks, 0 source-level, 5 operand-only (a0<->a1 tie on dist_sq's final pseudo, target[108,113,117,119,129]), 4 not-scored — byte-identical to s2/s3's recorded diff shape, no drift.

- [s4] target.o for a GTE-heavy function's permuter workspace must be built from raw oracle EXE bytes via mipsel-linux-gnu-objcopy -I binary -O elf32-tradlittlemips -B mips:3000 --rename-section .data=.text,contents,alloc,load,readonly,code — NOT by assembling asm/funcs/<func>.s, since splat's ASPSX GTE mnemonics (mvmva, op) have no GNU-as encoding and no standalone-.s translator exists in this tree.

- [s4] Feeding a full-TU preprocessed .c file to cc1 as a filename ARGUMENT (vs piping/redirecting via stdin, matching the Makefile's `cpp | cc1` form) can silently produce a corrupted/empty function body for a LATER function in the same TU even though it compiles correctly via stdin — reproduced for func_8002DAD0 in src/code6cac_b.c; any future standalone cc1 invocation on a full TU must pipe, not pass a filename.

- [s4] Rewriting a workspace .c file via a bare `python3 -c` file-rewrite through the Windows-side Bash tool (not inside a WSL-launched script) silently converts the whole file to CRLF line endings, breaking every downstream line-anchored awk/regex match in the WSL toolchain — confirmed via `file` + byte-count diff (4670/4670 lines flipped). Matches the harness memory's windows-python-crlf-write-text entry; workspace scripts must only ever be rewritten from inside WSL with explicit newline='\n' / binary mode.

- [s4] decomp-permuter's bundled pycparser fork's asm_operand grammar (tools/decomp-permuter/perm_pycparser/c_parser.py:2109) accepts only unary_expression, not general additive expressions — `"r"(obj + 0xA8)` fails to parse there even though it is completely ordinary C accepted by the real cc1; a workspace-only respelling to `"r"(&obj[0xA8])` (array-subscript reduces to postfix_expression -> unary_expression) is required to get past this specific parser, verified byte-identical on the real toolchain both ways.

- [s4] src/code6cac_b.c reverted to the committed INCLUDE_ASM("asm/funcs", func_8002DAD0); line before this session ended (git status --short clean except the pre-existing metrics/events.jsonl dirt and this session's ledger edits).

## s5 (enumerate)
- Chassis reconfirmed at session start: sandbox --disable all == 6/204 with candidate.c applied (matches driver's chassis-check).
- Ran tools/spelling_enum.py + tools/sweep_variants.py over the two structurally-flat blocks that read/write dist_sq and dist: the dist_sq compute block (2 spellings: named-dz vs fully-inlined) and the shift/tbl/dist compute block (4 spellings: independent name/inline of shift and tbl). Commutative-swap axis checked for both and found inapplicable (no bare-identifier/paren-group multiplicands or additive terms in either region).
- All 6 generated spellings scored 6/204, identical to baseline. Zero improvement anywhere in either region's enumerated space.
- Gotcha recorded for future sessions: marking ENUM-BEGIN/END so it strips a block's enclosing `{ }` produces a structurally different (mis-nested) candidate that can score very differently (measured 27/191 once, purely from lost block scope) -- not a real spelling variant. Keep the markers strictly inside existing braces.
- Conclusion: the a0/a1 register-seat tie survives every source-level respelling reachable from these two blocks. Next session should use the RA solver (tools/ra_solver/inverse_compose.py classify) to get a typed verdict on the tie before further hand/enum probing, or resume the s4 permuter workspace (construction bugs fixed, only the pycparser lexer blocker remains).

- [s5] Chassis check reconfirmed: candidate.c applied to src/code6cac_b.c measures sandbox --disable all == 6/204 at session start, matching the ledger's last recorded floor.

- [s5] spelling_enum.py's flat def-before-use permuter is UNSAFE to apply across a nested if/else/compound boundary -- it treats every non-decl/non-assign line as a trailing anchor and would hoist named-local declarations out of their original block scope, producing structurally different (and in this case measurably worse, 191 vs 204 insns) code that is not a real spelling variant of the target region. Both regions swept this session were deliberately chosen to sit entirely inside one existing `{ }` block with no nested control flow, to keep every generated variant a genuine same-semantics respelling.

- [s5] 6 of 6 generated spellings across the two flat sub-blocks (2 in the dist_sq compute block, 4 in the shift/tbl/dist compute block) scored exactly 6/204 -- the residual 5 operand-only a0/a1 register-seat hunks (per s2/s3's sandbox --disable all --diff classification) are invariant under every source-level respelling this tool can generate in either region.

- [s5] This corroborates, via exhaustive enumeration rather than hand-picked sampling, s3's five manual instance kills on the same/adjacent forms (store/compute reorder, compound-split both directions, named-intermediate for the c8 term, declaration-order change) -- the tool-based sweep covers the space those probes sampled from and finds nothing outside it that helps.

## s6 (synthesis) - MATCHED. Floor 6 -> 0; full-EXE SHA1 == oracle.

**The residual was never a "tie".** It was one deterministic preference bit,
and the tree's own instrumentation names it in a single read. Sessions 2-5
attacked the residual by respelling the two flat blocks around `dist_sq`
(11 banked instance kills + an exhaustive tool sweep, all measuring 6). The
mechanism lives one level up, in WHICH VARIABLE CARRIES THE Z DELTA.

**Mechanism (read, not inferred).** `BB2_FINDREG_DEBUG=75` (the env-gated
read-only hook already present at tools/gcc-2.7.2/global.c:1008, never used by
sessions 1-5 - they used BB2_ALLOC_DEBUG, which does not print preference sets)
prints for dist_sq's allocno:

    FINDREGDBG func=func_8002DAD0 pseudo=75 alt=0 acc=0 retry=0
    FINDREGDBG  conflicts: 2 3 12 17 29
    FINDREGDBG  someone_prefers:
    FINDREGDBG  pass0_used: 0 1 2 3 12 17 18 19 20 21 22 23 26 27 28 29 30 31
    FINDREGDBG  own_copy_prefs:
    FINDREGDBG  own_full_prefs: 5

`own_copy_prefs` EMPTY + `own_full_prefs: 5` is the whole answer. find_reg
(global.c:1056-1083) first scans hard regs ascending, skipping `pass0_used`;
the lowest survivor is **4 = $a0 = what the target uses**. Then the preference
loop at global.c:1130 OVERRIDES that pick with any bit still set in
`hard_reg_preferences[allocno]` - here 5 = $a1.

Where the bit comes from (chain, each link confirmed against the -da dumps at
tmp/grind/func_8002DAD0/s6/dumps/):
  1. `dz` is a fresh block-local referenced only inside one basic block, so
     local-alloc assigns it a hard register: `;; Register dispositions:`
     in the .greg shows `125 in 5` (and `126 in 5`, the D0 load temp).
  2. insn 164 is `(set (reg 130) (mult (reg 125) (reg 125)))`. `mark_reg_store`
     -> `set_preference(dest=reg130, src=MULT)` (global.c:1484); since
     GET_RTX_FORMAT(MULT)[0]=='e', src is unwrapped to reg125 and `copy` is set
     to 0 - which is exactly why a FULL preference is stamped and no COPY
     preference is. `reg_renumber[125] == 5`, so allocno 130 gets full-pref 5.
  3. insn 166 is `(set (reg/v 75) (plus (reg 129) (reg 130)))` carrying
     `REG_DEAD reg129` and `REG_DEAD reg130`. `expand_preferences`
     (global.c:828) merges preferences between an allocno that DIES in an insn
     and the allocno that insn SETS, when they do not conflict. The .greg
     conflict lists confirm neither 75-vs-130 nor 130-vs-75 conflicts, so
     `hard_reg_preferences[75] |= hard_reg_preferences[130]` = {5}.
     (129's own pref, 3, is merged too but `prune_preferences` removes it -
     75 conflicts with 3. 5 survives because 75 does NOT conflict with 5.)
  4. find_reg's natural $a0 is overridden to $a1. Five operand-only hunks.

**The C lever that follows directly.** A variable referenced in MORE THAN ONE
BASIC BLOCK is not a local-alloc quantity at all - it is a global allocno with
`reg_renumber == -1` during `global_conflicts`, so step 2 stamps NOTHING, step 3
has nothing to merge, and find_reg keeps its natural $a0. Carrying the Z delta
in one of the function's existing multi-block locals does exactly that.

**Measurements this session (all `sandbox func_8002DAD0 --disable all`):**

| form | score |
|---|---|
| s2-s5 chassis (fresh block-local `dz`) | 6 |
| CONTROL: fresh function-scope `s32 dz2;`, still single-block | **6** |
| `dist` carries the Z delta (`dist = *(obj+0xD0) >> 6;`) | **5** |
| `angle2` carries it, same spelling | **5** |
| `dist` carries it, load split from shift (`dist = *(obj+0xD0); dist >>= 6;`) | **0** |
| `angle2` carries it, same split | **0** |

The CONTROL is the load-bearing one: it isolates "multi-basic-block carrier" as
the variable and rules out declaration scope / declaration order (which s3 had
already killed from the other side).

**The second half of the fix.** With the carrier now a global allocno, the
unsplit `dist = *(obj+0xD0) >> 6;` leaves TWO pseudos (a local load temp plus
the global carrier), so ours emitted `lw v1,208(s1); sra a1,v1,6` where the
target emits `lw a1,208(s1); sra a1,a1,6`, and the D0 store / 0xFA store then
scheduled in the wrong order (score 5, 3 source-level hunks - banked at
rejected/s6-dist-reuse-unsplit-load-score5.c). Splitting the load from the shift
onto the SAME variable routes both through one pseudo and closes everything:

    dist = *(s32 *)(obj + 0xD0);
    dist >>= 6;

Split-init / compound-assignment splits of this shape are ordinary C
([[split-init-accumulation-sanctioned]]).

**Final state.** `sandbox func_8002DAD0 --disable all` == **0** (204/204); the
only remaining `--diff` hunks are the 5 masked branch-target-address artifacts
that were always there and are classed not-scored. `verify-oracle`:
`build_matches: true`, `build_sha1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa`
== the locked oracle - the whole EXE links byte-identical with func_8002DAD0 in
pure C.

- [s6] The sibling-transplant sweep mandated by the brief was run first and did
  contribute: reading the MATCHED func_8002E838 body on main (src/code6cac_b.c,
  its `dist`/`angle`/`lzcr` block) is what made it obvious that the matched
  sibling uses a small set of long-lived function-scope locals rather than
  block-scoped temps - the shape this session's fix generalizes. func_8002E838's
  own dist_sq block needs no carrier trick because its Z term is read straight
  from memory twice (no shift, hence no `dz` temp and no dying-product
  preference chain), which is why the sibling never hit this residual.
- [s6] CITATION for future sessions: `BB2_FINDREG_DEBUG=<pseudo>` (global.c:1008)
  prints conflicts / someone_prefers / used_so_far / pass0_used / own_copy_prefs
  / own_full_prefs for ONE pseudo. When a residual is "operand-only, one register
  differs", run it BEFORE hand-tracing global.c - it answers in one read what s3
  spent a session failing to hand-derive. Driver script used:
  tmp/grind/func_8002DAD0/s6/findreg.sh (mirrors dump.ps1's cpp|cc1 pipeline with
  the instrumented cc1 at tools/gcc-2.7.2/cc1 plus arbitrary env vars, capturing
  full stderr).
- [s6] GENERALIZABLE RULE-OF-THUMB (candidate for a technique rule): if a single
  pseudo lands one register off target and `own_copy_prefs` is EMPTY while
  `own_full_prefs` is non-empty, the bit did NOT come from a copy - it came from
  set_preference's `copy=0` path (an arithmetic RHS whose first operand is a
  local-alloc'd pseudo) and was then propagated by expand_preferences through a
  REG_DEAD edge. The C-level fix is to change the BASIC-BLOCK SPAN of the
  variable feeding that arithmetic, not to respell the arithmetic.

## [s7] Re-file of the s6 match (synthesis modality) - bytes re-proven from scratch

Session 6 reached distance 0 but was DISCARDED by the driver validator for a
paperwork defect in `memory/grind/func_8002DAD0/self_vet.md`: it declared three
sanctioned families but only two of its three `SCOPE:` quotes sat on a single
physical line. `tools/grinder/grindlib.py`'s `_SCOPE_LINE` regex is
`^\s*SCOPE\s*:\s*["“](.+?)["”]\s*$` - the closing quote must be
followed only by end-of-line, so a scope sentence wrapped across physical lines
counts as ZERO quotes no matter how faithful it is. The variable-reuse family's
scope bullet in `.claude/rules/no-new-park-categories.md:185` is a five-line
bullet, which is exactly how s6 quoted it.

FIX (for every future session, not just this function): quote the scope sentence
on ONE physical line, collapsing the rule file's own line wrapping. Embedded
double quotes inside the quoted span are fine (the regex is non-greedy but
anchored at `$`, so it expands to the last quote on the line). The fix was
verified mechanically by calling the driver's own validator directly:
`python3 tmp/grind/func_8002DAD0/s6/check_vet.py` -> `VALIDATOR ok= True`.
Do this BEFORE writing the outcome JSON - it costs one turn and it is the exact
check the driver runs.

RE-MEASUREMENT (nothing was taken on trust from s6; the C body is byte-identical
to s6's, re-applied to src/code6cac_b.c at the `INCLUDE_ASM("asm/funcs",
func_8002DAD0);` site by tmp/grind/func_8002DAD0/s6/apply.py):
  - `sandbox func_8002DAD0 --disable all` -> score 0, target_insns 204,
    build_insns 204, scorable true, rules_dropped 0, cheat_asm_stripped 21.
  - The `cheat_asm_stripped: 21` is FILE-WIDE and none of it is ours:
    `engine/inlineasm.py:384 write_stripped` strips the whole TU, and the
    stripped source it actually compiled
    (tmp/sandbox/func_8002DAD0/src/code6cac_b.c) still contains all 11 `__asm__`
    tokens of func_8002DAD0's body. The 0 is therefore measured WITH this
    function's six GTE/cop2 islands intact - i.e. it is the honest pure-C
    distance for an ASM-PARTIAL function, not a score obtained by deleting the
    islands. A future session that sees a non-zero `cheat_asm_stripped` on this
    file should check the stripped source before concluding anything.
  - `canonical func_8002DAD0` -> ASM-PARTIAL, 29/204 insns canonical-asm
    (reasons: GTE/cop2 op c2 / ctc2 / lwc2 / mtc2 / swc2). Unchanged from s1.
  - `verify-oracle` -> ok true, build_matches true, build_sha1
    62efab4f73f992798c43e8c730aa43baa10bb4fa == original_sha1_locked. The whole
    EXE links byte-identical with func_8002DAD0 as C.

## [s6 · synthesis] Island provenance found: Sony PsyQ `inline_o.h` (DMPSX v3)

RE-MEASURED FIRST (chassis discontinuity in the brief was real and benign — the
banked body measures 0, the ledger's "floor 6" is from before s6 solved the
a0/a1 seat): `sandbox func_8002DAD0 --disable all` -> score 0, 204/204,
cheat_asm_stripped 21 (file-wide, ours intact). Confirmed twice this session,
before and after the comment re-cite, with `tmp/grind/func_8002DAD0/s6/apply.py`
(which now `git checkout`s src/code6cac_b.c before splicing — the earlier
version anchored onto whatever was already applied and produced ONE invalid
measurement before it was caught; if a future session sees a nonsensical score,
check the applied file first).

THE FINDING. All nine islands are verbatim expansions of NAMED Sony macros from
PsyQ's DMPSX header `inline_o.h` — "Macro definitions of DMPSX version 3",
$PSLibId: Run-time Library Release 4.5$, Copyright(C) 1996 Sony Computer
Entertainment Inc. That header spells every GTE primitive as a run of
SINGLE-instruction `__asm__ volatile` blocks that stage the address through a
hard `$12` (`move $12,%0`) and hard-code `$13/$14/$15`; `inline_c.h` spells the
same primitives `%0`-relative with no preamble. Example, `gte_ldopv1(r1)`
(inline_o.h:192-200) vs our island 1: identical instruction for instruction,
register for register. Full table (all 9 islands, header line numbers, the
header banner):  memory/grind/func_8002DAD0/psyq_inline_o_provenance.md
Header copy lives at (gitignored scratch clone)
tmp/grind/motion_SetMotion/s7/repos/rood-reverse/include/psx/inline_o.h.

Consequences:
  - The two layer-1 citation FAILs (2026-09-18 22:09 and 22:39) were both
    caused by citing the WRONG HEADER: attributing these bodies to inline_c.h
    leaves the `move`/`addu` preamble and the delay `nop`s unattributable, and
    the reviewer correctly kept finding uncited instructions. candidate.c's
    island comments now cite inline_o.h macro-by-macro (comments only — the
    body hash is unchanged).
  - It explains the whole 28-function cop2-addressing-preamble cluster: the
    idiom the cluster rule calls a handwritten signature is ordinary compiled C
    whose GTE macros expand that way. This is also why `scan_hand_coded
    --single func_8002DAD0` measures **tier=LOW, score=1/8** (S4 only) and why
    that tier will never be STRONG for this family — the STRONG-tier
    canonical-asm door cannot open for it by construction.

GATE MECHANICS (read before proposing an island respelling):
  - engine strip (`engine/inlineasm.py:_block_category`): a block survives iff
    it contains >=1 cop2 instruction. A GPR-only block is "cheat" and is
    deleted before scoring.
  - driver island gate (`tools/grinder/grind.ps1:967`, owner Ruling C):
    a MULTI-instruction block is flagged iff it contains >=1 NON-cop2/non-nop
    instruction (`tools/audit_asm_cheats.py:_is_whitelisted_insn`); single-insn
    blocks are skipped entirely.
  These two are in direct opposition: a block passes both only if it contains
  ONLY cop2/nop instructions. Every GPR preamble instruction therefore has to
  come from C — and it cannot (measured, see hypotheses s6 H1/H2/H3).
  - macro DEFINITIONS are skipped by the stripper only when the `__asm__` sits
    on a physical line whose start is `#` — in a multi-line macro definition
    only the first line qualifies. But the decisive fact is simpler:
    `write_stripped` only ever rewrites `src/<stem>.c`, so macros defined in a
    HEADER are outside the stripper's reach entirely.

DISPOSITION: filed as an INTEGRATION HANDOFF in docs/grind/decisions.md
(2026-09-18 entry, line ~28314) with the two doors that land it: (1) one row in
tools/grinder/owner_cluster_grants.txt, the same row seven sibling cluster
members already carry; (2) a Judge scope grant for include/gte.h so the Sony
macro set can live in the project header and the body can be written as macro
invocations. Door (2) is deliberately NOT self-approved — the mechanism that
makes it clear the island gate is that a header is outside the cheat-stripper's
reach, and that is a Judge question, not an agent question.

- [s6] Re-measured this session with memory/grind/func_8002DAD0/candidate.c applied to src/code6cac_b.c: `sandbox func_8002DAD0 --disable all` -> score 0, target_insns 204, build_insns 204, scorable true, rules_dropped 0, cheat_asm_stripped 21 (file-wide; this function's own islands are intact in the stripped source). Confirmed twice, before and after the comment re-cite.

- [s6] The brief's CHASSIS DISCONTINUITY is real and benign: the banked body measures 0 and the ledger's 'floor 6' predates s6's solution of the a0/a1 register seat. Floor is 0.

- [s6] PROVENANCE (new): all nine islands are verbatim expansions of named Sony PsyQ DMPSX inline_o.h macros; per-island header line numbers in memory/grind/func_8002DAD0/psyq_inline_o_provenance.md. inline_c.h, which earlier sessions cited, spells the same primitives %0-relative with no move preamble — the root cause of both layer-1 citation FAILs.

- [s6] candidate.c's island comments were re-cited to inline_o.h macro-by-macro. COMMENTS ONLY: the code lines are identical to the previous body (verified by comment-stripped diff, 139 lines both sides), so the driver's comment-insensitive body hash is unchanged and no review loop is re-opened by a respelling.

- [s6] The two gates are in direct opposition: engine/inlineasm.py:_block_category keeps a block only if it contains a cop2 instruction, while tools/audit_asm_cheats.py:_is_whitelisted_insn (the driver's island gate, grind.ps1:967) flags a multi-instruction block for any non-cop2/non-nop instruction. A block passes both only if it holds cop2/nop instructions exclusively, so every GPR preamble instruction would have to come from C — and measurement H1 shows the target's bytes contain it.

- [s6] tools/scan_hand_coded.py --single func_8002DAD0 -> tier=LOW, score=1/8 (S4 front-loads only). The STRONG-tier canonical-asm door cannot open for this family by construction; the owner-cluster registry door is the one that fits, and func_8002DAD0 is enumerated by name in the landed 2026-08-17 cluster ruling (.claude/rules/cop2-addressing-preamble-cluster.md:76) but has no row in tools/grinder/owner_cluster_grants.txt (seven sibling members do).

- [s6] memory/grind/func_8002DAD0/self_vet.md was rewritten and verified against the driver's own validator (tmp/grind/func_8002DAD0/s6/check_vet.py -> ok=True): two claimed families, each with its scope sentence quoted on ONE physical line and a file:line precedent. This clears the defect that discarded the previous session.

- [s6] tmp/grind/func_8002DAD0/s6/apply.py now restores src/code6cac_b.c from git before splicing. The earlier version anchored onto whatever was already applied and produced one mis-spliced measurement before it was caught; any future session reusing it gets a clean baseline per variant.
