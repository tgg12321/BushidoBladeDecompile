# Rejected — exhaustive spelling-enum sweep of the base/key preamble (s5, enumerate modality)

**Region:** the `bank_off`/`a1_off`/`base`/`key` preamble (the address
computation that has been this ledger's frontier since s2). Marked with
ENUM-BEGIN/END in `tmp/grind/_SsSndCrescendo/s5/enum_candidate.c`, holding
the 6 `SS_SCORE_FLAG(a0,a1) &= ~0x10;` clear sites fixed at their existing
fresh-macro-recompute form (unchanged from the banked candidate.c).

**Tool:** `tools/spelling_enum.py` — exhaustively enumerates every
def-before-use declaration ordering, every inline-or-keep-named choice for
the 2 named locals (`bank_off`, `a1_off`), and every commutative operand
swap. 2 named locals x 2 assigns (`base`, `key`) -> 16 distinct spellings
(with swaps). Swept all 16 in one `tools/sweep_variants.py --json` call.

**Full histogram (16/16 measured):**
- 130/213 (target 200) — 6 variants (v00, v02, v10, v12, v14, v15): every
  form that either fully inlines both `bank_off` and `a1_off` into the
  `base` expression, OR keeps both named but leaves the clear sites
  untouched. Ties the banked candidate.c exactly.
- 133/213 — 4 variants (v04, v06, v08, v09)
- 137/215 — 4 variants (v01, v03, v05, v07)
- 139/215 — 2 variants (v11, v13)

**Result:** no spelling in this exhaustively-enumerated space beats 130 or
reaches build_insns=200. CLASS KILL for the preamble's own spelling space —
tools/spelling_enum.py enumerates the full def-before-use x inline x swap
space by construction (tools/spelling_enum.py:9), so this is a complete
search of that axis, not a sampled one.

**Why this matters (new information):** v00/v02 (both `bank_off` AND
`a1_off` named, clear sites untouched) tie 130 — the SAME score as full
inlining. This is a genuinely new data point vs the s4/s5-earlier
"shared-bank-off-and-a1off" rejection (`shared-bank-off-and-a1off-s5.md`,
score 143/184): that form ALSO rewrote the 6 clear sites to reuse the
cached locals. Isolating "name the locals in the preamble only" from "also
reuse them at the clear sites" shows the FIRST half is harmless-but-inert
and the SECOND half is what drove the prior result (in the wrong
direction — GCC over-shares at the clear sites when they're rewritten to
use the named locals, landing under target's insn count).

**Conclusion for next session:** the preamble block is exhausted as a
spelling-search target. The residual must be attacked at the clear sites
themselves (why does target's asm re-derive the address fresh at each
site, and in exactly what register pattern) — a register-allocation /
`.greg`-`.lreg` dump-read question, not a C-spelling question, per the
ledger's existing frontier item 1.

**Kill scope:** class (exhaustive enumeration of the region's decl-order x
inline x swap space), this chassis (candidate.c s3 form as the fixed
surrounding body), no FAKE constructs, `sandbox --disable all`.
