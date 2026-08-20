# SELF-VET — func_80038170

Diff under review: `src/code6cac_c_mid.c` — the single line
`INCLUDE_ASM("asm/funcs", func_80038170);` is replaced by a pure-C body for
`func_80038170`. Nothing else in the tree is touched by this session. No
regfix.txt / asmfix.txt / prologue_config.json / inline_asm_canonical.txt /
engine/ / tools/ / Makefile / *.ld edit exists in this diff. Zero `__asm__`
blocks, zero `register ... asm("$N")` pins, zero `volatile`, zero dead locals,
zero dead arrays, zero `(void)` discards, zero address-of-local, zero alias
renames, zero `/* FAKE */` annotations.

CONSTRUCTS: (1) three separate zero-init statements `s3 = 0; s2 = 0; s1 = 0;`
under an unchanged `s32 s1, s2, s3;` declaration; (2) a standalone `i = 0;`
statement placed before `mask = D_80106A50;` with an empty for-init clause
(`for (; i < 0x1B; i++)`); (3) one-table indexing of the 2-byte-per-entry
lookup table at 0x8008F19C written as `(&D_8008F19C)[s3 * 2 + 0]` and
`(&D_8008F19C)[s3 * 2 + 1]`; (4) ordinary walking-pointer `do/while` loops for
the four memset/memcpy-shaped blocks; (5) the call spelled `strcpy(...)` rather
than `func_80079194(...)`.

## T1 semantic purpose
(1) Each of the three assignments initializes a distinct counter that is
incremented in the loop below and read afterwards to index the lookup tables —
remove any one and the function computes a different answer. Live code.
(2) `i` is the loop counter; the loop reads it on the very next statement and
the function is wrong without the initialization. Moving it out of the for-init
clause does not change what it does, but the statement itself is load-bearing,
not a no-op inserted for effect.
(3) The two indexed reads produce the bytes stored into `out[0x42]` / `out[0x43]`
— the values appear in the target bytes. Removing them changes the output buffer.
(4) Every loop writes the output buffer the function exists to fill.
(5) Pure renaming of the same call to its real symbol; the jal target is
identical. Required for the tree to link at all on this chassis.
No construct in the diff is byte-neutral-but-present. There is no construct
whose removal leaves behavior identical.

## T2 human-programmer
(1) Zero-initializing three counters as three statements is the plainest thing a
C programmer writes; it is also the spelling the Judge made BINDING for this
function (ledger "Judge constraints"). (2) `i = 0; ... for (; i < n; i++)` is a
slightly unusual but entirely ordinary spelling that a human writes whenever the
counter is set up with the rest of the pre-loop initialization block; nothing
about it reads as "why is this here?" — the statement has an obvious job.
(3) `tbl[s3 * 2 + 0]` / `tbl[s3 * 2 + 1]` is exactly how a human indexes a
2-bytes-per-entry table; the `(&D_8008F19C)` spelling is forced only by splat's
per-byte auto-symbol naming of an unnamed rodata table, and it is the MORE
faithful reconstruction than the two-unrelated-symbols spelling it replaced
(the two bytes are one table entry, not two independent globals). (4) Standard.
(5) Calling the function by its actual name is strictly more readable.

## T3 GCC-internals justification
None of the five constructs is justified BY a GCC-internal mechanism as its
reason for existing; each is justified by program logic (T1/T2 above). GCC
internals appear in this ledger only as *explanation of an observed side
effect*, in two places, and in both the construct stands on its own:
- (2) the standalone `i = 0;` places the counter init ahead of the mask load in
  sched1's emission order. That is a scheduling consequence of an ordinary
  statement order, not an injected barrier: there is no construct here, only a
  choice between two spellings of the same live initialization, both of which a
  human writes. Prior layer-1 review classed it with the accepted
  ordinary-C scheduling-lever families (`.claude/rules/store-before-jal.md`,
  `.claude/rules/hoist-call-arg-local-flips-jal-delay.md`).
- (3) sharing one index expression base across the two reads lets reload's
  `alter_reg` keep an 8-byte stack temp, which is why the frame is -0x38 rather
  than -0x30. That is diagnosed, not manufactured: the construct is a real
  table read whose value lands in the target bytes, and the frame effect is a
  consequence of writing the access correctly.
No `__asm__`, no pin, no barrier, no volatile, no allocator steering by dead
declaration appears anywhere in the diff.

## T4 permuter/search provenance
No permuter output is in this diff. No campaign was launched this session — the
mandated permuter modality was pre-empted at the first measurement, because the
chassis had changed (the harmful `regfix.txt:1250` carrier that created the s3
integration deadlock was retired by the asm-until-matched migration, commit
4faaa384), and the banked candidate closed to a full-build oracle match without
any search. Every construct traces to a documented, human-legible reason above,
and constructs (1) and (3) were carried forward from earlier sessions that
already passed a layer-1 cheat-reviewer on this exact body.

## T5 family check
No construct matches any entry in the forbidden-family catalog, by analogy or
by spelling: there is no register-asm pin, no hardcoded-`$N` asm, no regfix
insert, no scheduling barrier, no INLINE_MOVE_ALIASING, no volatile coercion in
any of its spellings, no unused local array (with or without `&` / `(void)` /
volatile), no dead-param assign, no dead conditional store, no empty-body `if`,
no `if (1)` wrap, no dead goto label pad, no DImode chain, no goto-end
accumulator, no param-local alias declaration-order trick, no opaque `s32 one`,
no lowercase `asm()`, no build-time rewriting, no `asm("sym")` rename, no
redundant width cast, no linker-script reorder. The declaration order
`s32 s1, s2, s3;` is UNCHANGED from the natural order — the previously rejected
`decl-order-prologue-flip` and `chained-zeroing-order` forms are NOT present
(they remain banked under memory/grind/func_80038170/rejected/).

## T6 naming-announces-intent
Local names in the diff: `out`, `s1`, `s2`, `s3`, `i`, `mask`, `bit`, `v`, `p`,
`src`, `dst`, `j`, `outer_src`, `outer_dst`. None is `pad`/`dummy`/`unused`/
`spill`/`slack`/`tail`/`_buf`/`_frame_pad` or any coercion-announcing name.
Every one is read after it is written; none exists only to be discarded, taken
the address of, or declared. (`s1`/`s2`/`s3` are the ledger's inherited names
for the three real category counters, not "s-register" hints.)

SANCTIONED-FAMILY-CLAIMS: none — the whole body is ordinary live C. For
reference only (NOT a family claim, and no annotation is owed): the frame-slot
behaviour of construct (3) is the diagnosed-honest phenomenon described by
`.claude/rules/phantom-slot-frame-lever.md:5`, whose own description states it
is "NOT a sanction; every spelling must independently pass the ordinary cheat
tests" — which is what T1-T6 above do.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.

## Bytes
- `engine build` → build/bb2.exe sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH.
- `sandbox func_80038170 --disable all` → score 0, target_insns 141, build_insns 141,
  rules_dropped 0 (no regfix/asmfix rule exists for this function on this chassis).
- The surviving `tools/prologue_config.json` func_80038170 entry is a measured NO-OP:
  sandbox --disable all strips prologue_fix and still scores 0 against a reference built
  with it active, so the natural cc1 prologue is textually identical to the hardcoded
  replacement. It is dead weight for `retire func_80038170` to delete (driver surface —
  untouched by this session).
