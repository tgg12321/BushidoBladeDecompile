# SELF-VET — func_80038170

Diff under vet (this session, vs HEAD):
  - src/code6cac_c_mid.c: `INCLUDE_ASM("asm/funcs", func_80038170);` replaced by the
    C body banked in memory/grind/func_80038170/candidate.c
  - include/code6cac.h:80: `extern u8 D_8008F19C;` -> `extern u8 D_8008F19C[];`

CONSTRUCTS: (1) header declaration corrected from scalar to array for the table
symbol D_8008F19C; (2) three ordinary assignments `s3 = 0; s2 = 0; s1 = 0;` under
the declaration `s32 s1, s2, s3;`, zeroing three counters that the body increments
and later reads; (3) one-base table indexing `D_8008F19C[s3 * 2 + 0]` and
`D_8008F19C[s3 * 2 + 1]` for the two table bytes read inside the `if (s3 > 0)` arm;
(4) walking-pointer do/while loops over the output buffer; (5) an ordinary counted
loop `for (i = 0; i < 0x1B; i++)` over the enabled-flag bits. Nothing else: no dead
locals, no dead stores, no unused arrays, no volatile, no inline asm, no register
pins, no `(void)` discards, no aliases, no annotations, no regfix/asmfix rules.

## T1 semantic purpose
(1) The corrected declaration describes the object the code reads: a table with two
bytes per entry, indexed by `s3`. Without it the program is ill-typed, so it carries
semantic content by definition. (2) The three assignments zero three counters that
are incremented in the loop and READ afterwards (`out[0x22]`/`out[0x23]` from `s1`,
`out[0x3C]`/`out[0x3D]` from `s2`, the `if (s3 > 0)` arm from `s3`); delete any one
and the function's output changes. (3) The two indexed reads supply the bytes stored
to `out[0x42]`/`out[0x43]` — live data flow. (4) Each loop writes bytes of the output
record the caller consumes. (5) The counted loop is what tallies the categories.
Every construct in the diff has observable effect on the function's output; there is
no construct whose removal leaves behaviour byte-identical.

## T2 human-programmer
Yes to all five. Given only the specification — "build a 0xC4-byte record: tally the
three categories over the 0x1B enabled flags, stamp the 4-byte header, clear the
tail, copy the name string and the two lookup-table digit pairs, then blit two
constant blocks" — a human writes exactly this shape: three counters zeroed, a flag
loop, a table lookup, clear loops, copy loops. Nothing in the diff would make a
reader ask "why is this here?". The array declaration is the obvious one for an
indexed table.

## T3 GCC-internals justification
No. Every construct's reasoning is program logic. Notably this candidate REMOVED the
only construct that ever had a compiler-pass rationale (the statement-order lever the
layer-1 reviewer FAILed on 2026-08-19 21:02, docs/grind/decisions.md:7039); the
counted loop is written the plain way and the build still MATCHES the oracle, which
is the measurement proving no scheduling lever was ever required here. The `-0x38`
frame does arise as a side effect of reading one table through one base, but the
indexing is not written FOR that effect — reading a single table through its own base
symbol is the plainer C on its own merits, and the two-symbol alternative existed only
because splat emitted a per-byte auto-symbol for the table's second byte.

## T4 permuter/search provenance
No permuter, no auto-search, no solver was run on this function in this session or any
prior session (ledger s4a explicitly records that no campaign was launched). Every
form in the diff was hand-written from the target's data flow and measured directly.
Nothing here survives only because a detector misses a particular wording.

## T5 family check
No construct matches any forbidden family, by shape or by analogy. Checked explicitly
against: register-asm pins (none), hardcoded-$N asm (none), regfix/asmfix (untouched;
sandbox reports rules_dropped 0), scheduling barriers (none), volatile coercion in any
guise (no volatile anywhere in the diff), unused or dead local arrays / frame pads
(every local is written and read), dead-param-assign, dead-conditional-store,
empty-body if, `if (1)` wrapping, dead-goto label pads (none of these appear), DImode
chains (none), opaque constant variables (none — `mask` is a real load of D_80106A50,
`bit` a real shift), alias renames (none), redundant width casts (the `(u16 *)` casts
are genuine halfword-copy pointer types), and speculative `bb2.ld` reorders
(untouched). Two constructs from earlier sessions that DID land in forbidden families
— reversed declaration order and the chained `s1 = s2 = s3 = 0` — are banked under
rejected/ and neither appears here; declaration order is the natural `s32 s1, s2, s3;`.

## T6 naming-announces-intent
No name in the diff is `pad`, `dummy`, `unused`, `spill`, `tmp`, `slack`, `_buf`, or
similar. Names are `out`, `s1`, `s2`, `s3`, `i`, `j`, `mask`, `bit`, `v`, `p`, `src`,
`dst`, `outer_src`, `outer_dst` — every one is consumed by later code; none is
declaration-only, address-of-only, or discarded.

SANCTIONED-FAMILY-CLAIMS: none — the diff contains no construct that requires a
sanctioned family. It is ordinary live C throughout. (For reference only, not as a
claim: the header correction follows the same reasoning as
.claude/rules/header-type-correction-from-use-sites.md:1, but a correct extern
declaration is plain C and needs no exception.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff, and no claimed family
mandates an annotation.

MEASUREMENTS BACKING THIS VET (this session, this tree):
  sandbox func_80038170 --disable all -> score 0, target_insns 141, build_insns 141,
                                         scorable true, rules_dropped 0
  engine build                        -> build/bb2.exe sha1
                                         62efab4f73f992798c43e8c730aa43baa10bb4fa
                                         == oracle, MATCH
  canonical func_80038170             -> verdict C, asm_insns 0, distance 0

RE-MEASURED THIS SESSION (2026-08-19, synthesis, session id s4d — the diff above
re-applied verbatim from candidate.c plus the one-line include/code6cac.h edit):
  sandbox func_80038170 --disable all -> score 0, target_insns 141, build_insns 141,
                                         scorable true, rules_dropped 0
  engine build                        -> build/bb2.exe sha1
                                         62efab4f73f992798c43e8c730aa43baa10bb4fa
                                         == oracle, MATCH
  canonical func_80038170             -> verdict C, asm_insns 0, distance 0
Third independent reproduction of the solved form (s4b, s4c, s4d). No construct in
the diff changed between reproductions.
