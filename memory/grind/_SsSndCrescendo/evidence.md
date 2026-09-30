# Evidence — _SsSndCrescendo

(The grind ledger closed at 84face686 / 27bb66dc6 and was pruned. Its s7 record of the preamble,
hypotheses.md H8 and rejected/single-expression-preamble-s7.md, is readable with
`git show 84face686:memory/grind/_SsSndCrescendo/<file>`.)

## Cleanup 2026-09-30 — the FAKE bank_no / score_tbl preamble is not needed

Found while clearing the retro-audit FAIL on the sibling _SsSndDecrescendo (9de7d3e8c): the plain form
that matches Decrescendo also matches this function. The landed body (84face686) carried a
FAKE-annotated named intermediate `s32 bank_no = a0;` and a FAKE-annotated pointer alias
`s32 *score_tbl = (s32 *)&_ss_score;` feeding `s32 *bank = score_tbl + bank_no;`. The s7 exhaustion
measured 13 single-expression spellings of `bank` (all 2) but never measured dropping `bank`.

Measured (cleanup-alias/gen.py builds each variant from the landed body with only the named lines
changed; sandbox `--disable all`, whole main.c TU; results in cleanup-alias/scores.txt):

| variant | spelling | score |
|---|---|---|
| base | landed body (FAKE bank_no + score_tbl + bank) | 0 (200/200) |
| c_plus | `s32 *bank = (s32 *)&_ss_score + a0;` | 2 |
| c_sub | `&((s32 *)&_ss_score)[a0]` | 2 |
| c_idxonly | `bank_no = a0;` then `(s32 *)&_ss_score + bank_no` | 2 |
| c_aliasonly | `score_tbl = (s32 *)&_ss_score;` then `score_tbl + a0` | 2 |
| c_nobank_base | no `bank`; in-arm clears through `base + 0x98` | 61 (191/200) |
| c_nobank | no `bank` local; each `*bank` read spelled `((s32 *)&_ss_score)[a0]` | 0 (200/200) |
| **v_macro** | c_nobank + all 6 flag clears spelled `SS_SCORE_FLAG(a0, a1) &= ~0x10` | **0 (200/200)** |

The 2-point residual (the `la _ss_score` insn after the folded `ashiftrt:14`, s7 H8) only arises when
the slot address is kept in its own pointer; reading the table directly for `base` does not hit it.
c_nobank_base (61) shows the clears re-index the table fresh, as the library source does
(`_ss_score[arg0][arg1].unk90 &= ~0x10`).

LANDING: v_macro. Both FAKE constructs and their annotations removed; no FAKE construct remains.
Sandbox 0 (200/200); verify-oracle --rebuild --allow-dirty SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa.
