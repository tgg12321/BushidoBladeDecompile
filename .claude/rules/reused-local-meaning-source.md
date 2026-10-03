---
name: reused-local-meaning-source
paths: [".claude/rules/reused-local-meaning-source.md"]
description: "Ordinary-C Rulings 7-10: sprintf and vmNoiseOn SOTN lines (those functions only), one meaning at several constant offsets (9, b′), verified original-source reuse (10)."
metadata:
  type: rule
---

# Reused locals: named functions, one meaning, original source

Part of [[ordinary-c-judge-decidable]]: a fresh local written more than once is admitted only
under exactly one of Rulings 5-12 (or a Q51 SOTN citation, [[sotn-precedent-suffices]]), which then
governs it exclusively. Questions and verbatim answers: docs/grind/owner-rulings-2026-09-26.md
and the docs/grind/decisions.md OWNER RULING entry named in each heading.

## Ruling 7 — sprintf's SOTN buffer-end line (sprintf only, 2026-09-23)

Only `sprintf` (`src/text1b_b.c`) may carry, exactly once, SOTN's
`bufPtr = (char*)&args - sizeof(printf_info) - 4;` (sotn `src/main/psxsdk/libc/sprintf.c:90`),
with the frame confirming `&buf[sizeof(buf)]`, an inline comment (SOTN-verbatim, what it
computes, this ruling; not SOTN's inaccurate comment), the truthful spelling recorded failing in
the ledger, and no other address derived from another local. Everything else in the body is
judged normally (do-while(0) wraps per [[do-while-zero-exception]]).

## Ruling 8 — vmNoiseOn's SOTN pan `temp` (vmNoiseOn only, 2026-09-24)

Only `vmNoiseOn` (`src/main/psxsdk/libsnd/774F8.c`) may carry one function-scope `u32 temp;` used solely by the
SOTN-verbatim three-step pan cascade (sotn `libsnd/vmanager.c:251-270` @aa53500; fields
`tone_pan`/`mpan`/`pan`), placed right after the `volr_t` scaling and before
`if (_svm_stereo_mono == 1)`, with an inline comment, and the field-direct and three-local
spellings recorded failing. Everything else is judged normally.

## Ruling 9 — one meaning, several constant offsets (2026-09-25)

Judged instead of Rulings 5/6 when ALL hold: (a) one consumer, consumer statements identical.
(b) Every write is `VAR = BASE + K;` with ONE identical BASE (a local, parameter or local
object's member, read without side effect), K a nonzero integer constant, no cast/call/other
operator; at every write BASE + K is a sub-object of ONE kind of BASE's record, shown in the
ledger with evidence independent of the byte-chasing (layout, other readers by file:line).
(c) Each write is read once, by the consumer, in the same compound statement, write first, no
`return`/`continue`/`goto` and no `break` between them, except a `break` that exits a
`for`/`while`/`do` loop lying wholly between write and consumer (clarification 2026-09-26);
that nested loop must not touch the variable. (d) Each addition is in the target's bytes at
that site. (e) Every consumer store's object is read (by a non-consumer statement) before the
next consumer store or function exit. (f) A descriptive name true of every write (not generic,
not single-letter, not true of only some writes). (g) Ruling 5 prong 2; (h) prong 3;
(i) receipts + the (b) evidence; normal review.

**(b′) Layout the code assumes (2026-09-25).** (b) may be judged by the layout the code
consistently imposes when ALL hold: (1) that layout is shown by the code's own accesses other
than the `VAR = BASE + K` writes (target instructions, other readers, cited); (2) every normal
path reaches a same-kind sub-object; (3) every other reachable path is a documented anomaly in
the original (trigger, measured bytes, why malformed), where BASE + K lies OUTSIDE BASE's
record. Past the record's end counts as outside when the address is not the start of any
referenced object (code access/walk range, pointer/table entry, used symbol), with the search
banked; landing on a referenced object or a different-kind sub-object fails. (4) Comments
describe what the code assumes, not a claim about all data.

Still banned under Ruling 9: `y1`, `c`, `src`/`idx`, `half`, prnt's `n`, func_8001BE20
`shift`, func_8005490C `obj`, func_8002A458 dx/dy/dz, func_8008B488 `rate` (except via Q51),
func_80057E84 `vtx`/`node`, func_800288C8 `tbl`, func_8002A458 `lzc_in`.

## Ruling 10 — verified original source, verbatim reuse (2026-09-25)

Judged instead of Rulings 5/6/9 when ALL hold: (A) the function's original source (or its
port's upstream) is public and pinned: archive URL at a commit/tag, its version id (e.g.
sccsid), file SHA-256, cited lines confirmed against a second independent copy. A
decompilation (SOTN, psyz, …) is NEVER an original source. (B) Statement-by-statement mapping
of the whole function with EVERY difference listed. (C) The reused local is the original's own
variable, same name, its type or the direct project equivalent, written/read only where the
original does, in order; `register` not carried over. (D) Inline comment citing source and this
ruling. (E) One-variable-per-role spelling + one other respelling measured failing; normal
review. (F) Nothing else in the body gains from this ruling.
