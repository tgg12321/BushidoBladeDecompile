# Evidence — _SsSndDecrescendo

## Cleanup 2026-09-30 — the bank_no / score_tbl preamble (retro-audit 2026-09-29 FAIL, class A, owner Q37)

Finding (tmp/audit-2026-09-29/review/batch_00.md, _SsSndDecrescendo): the landed body (69c19b9ac)
opened with `s32 bank_no = a0; s32 *score_tbl = (s32 *)&_ss_score; s32 *bank = score_tbl + bank_no;`,
a named intermediate plus a C-level pointer alias to `_ss_score`, copied from _SsSndCrescendo without
that sibling's FAKE annotations or any lever-exhaustion record for this function.

Measured (cleanup-alias/gen.py builds every variant from the landed body with only the named lines
changed; sandbox `--disable all`, whole main.c TU; results in cleanup-alias/scores.txt):

| variant | preamble / clear-site spelling | score |
|---|---|---|
| base | landed body (bank_no + score_tbl + bank) | 0 (235/235) |
| v_plus | `s32 *bank = (s32 *)&_ss_score + a0;` | 2 |
| v_s32cast | `(s32 *)&_ss_score + (s32)a0` | 2 |
| v_sub | `&((s32 *)&_ss_score)[a0]` | 2 |
| v_bytes | `(s32 *)((u8 *)&_ss_score + a0 * 4)` | 2 |
| v_idxonly | `bank_no = a0;` then `(s32 *)&_ss_score + bank_no` | 2 |
| v_aliasonly | `score_tbl = (s32 *)&_ss_score;` then `score_tbl + a0` | 2 |
| v_alias_first | alias declared before bank_no | 2 |
| v_assign_order | bank_no assigned, then `bank = (s32 *)&_ss_score + bank_no` | 2 |
| v_sotn_score | `bank = &((s32 *)&_ss_score)[a0];` as a statement | 2 |
| v_twostmt / v_twostmt_idx | `bank = (s32 *)&_ss_score; bank += a0;` | 25 |
| v_base_direct(_amp/_nocast) | `base` from `((s32 *)&_ss_score)[a0]` first, `bank` after | 0 |
| v_nobank_base | no `bank`; in-arm clears through `base + 0x98` | 67 (223/235) |
| **v_nobank** | no `bank` local; each `*bank` read spelled `((s32 *)&_ss_score)[a0]` | **0 (235/235)** |
| **v_macro** | v_nobank + all 8 flag clears spelled `SS_SCORE_FLAG(a0, a1) &= ~0x20` | **0 (235/235)** |

Result: neither construct is needed. `base` is read straight from the table
(`((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0`) and every clear of the 0x20 flag re-indexes the table
through the TU's existing `SS_SCORE_FLAG(i, j)` macro (main.c, defined above SsSeqCalledTbyT), the
same fresh `_ss_score[arg0][arg1].unk90 &= ~0x20` re-index the library source shows at its clear
sites (SOTN src/main/psxsdk/libsnd/decre.c, an older revision of this function, read for shape only;
no construct is admitted on it). v_nobank_base (67) shows the fresh re-index is what the target does:
the clears do not go through the cached `base`.

The 2-point residual of every single-local form (v_plus .. v_sotn_score) is the one the
_SsSndCrescendo ledger recorded at s7 (the `la _ss_score` insn landing after the folded
`ashiftrt:14` instead of between the two shifts; 84face686 rejected/single-expression-preamble-s7.md).
It only arises when the slot address is kept in its own pointer; indexing the table directly for
`base` does not hit it.

LANDING: v_macro, no FAKE construct, no annotation. Sandbox 0 (235/235).

Follow-up (not applied, reported to the orchestrator): the same plain form also scores 0 (200/200)
on _SsSndCrescendo (cleanup-alias/crescendo_v_macro.c), whose FAKE-annotated bank_no / score_tbl
pair is therefore not needed either.
