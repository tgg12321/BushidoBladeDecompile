# Naming Proposals Database Audit — 2026-07-13

Audit of `docs/naming/proposals.csv` (1114 data rows) against the current
applied symbol names in `named_syms.txt` + `symbol_addrs.txt`. Purpose: identify proposals
that have already been resolved (the address now carries a semantic name) so they can be
removed from the active review tables.

**Method:** the CSV's `current_name` column is stale — every row lists a `func_XXXXXXXX`
placeholder. Resolution is determined by looking up each row's *address* in the two symbol
files and checking whether it now maps to a non-placeholder (semantic) name.

## Category counts

| Category | Count | Meaning |
|---|---:|---|
| APPLIED-AS-PROPOSED | 31 | address now carries exactly the proposed name |
| APPLIED-DIFFERENTLY | 291 | address carries a *different* semantic name than proposed |
| STILL-PENDING | 23 | real proposal, address still `func_XXXXXXXX` |
| NO-PROPOSAL-NOW-NAMED | 654 | CSV row had an **empty** proposed_name; address was named by other means |
| NO-PROPOSAL-STILL-UNNAMED | 115 | CSV row had an empty proposed_name; address still unnamed |
| **Total** | **1114** | |

**Note on the two NO-PROPOSAL buckets:** 769 of the 1114 rows (all `none` confidence)
never carried an actual proposal — `proposed_name` is blank. They are NOT proposal-engine
"misses"; the engine deliberately declined to propose. 654 of those addresses have since
been named through the decomp/naming work anyway; 115 remain unnamed. They are broken out
separately so they don't inflate the APPLIED-DIFFERENTLY figure.

## Confidence-tier reconciliation (every tier fully accounted for)

| Tier | APPLIED-AS-PROPOSED | APPLIED-DIFFERENTLY | STILL-PENDING | NO-PROP named | NO-PROP unnamed | tier total |
|---|---:|---:|---:|---:|---:|---:|
| high   | 7  | 3   | 0  | 0   | 0   | 10  |
| medium | 24 | 87  | 0  | 0   | 0   | 111 |
| low    | 0  | 201 | 23 | 0   | 0   | 224 |
| none   | 0  | 0   | 0  | 654 | 115 | 769 |

**Key finding:** every high- and medium-confidence proposal that carried a name has already
been resolved (applied as-proposed or superseded by a different applied name). **Zero** high
or medium proposals remain pending. The only real proposals still pending are 23 low-confidence
weak-evidence rows (`address_neighborhood` / `string_adjacent` `_local_`/`_func_` stubs).

## STILL-PENDING by confidence

low=23 — all 23 are low-confidence weak-evidence stubs (see
regenerated review tables note). Combined with the 115 NO-PROPOSAL-STILL-UNNAMED addresses,
**138 addresses total
still lack a semantic name.**

## APPLIED-DIFFERENTLY — owner review

These are addresses where a *different* name than the CSV proposal was ultimately applied.
The bulk (288 of 291) are medium/low `_helper_<addr>` sole-caller / neighborhood stubs that
the naming work refined into specific semantic names — expected and healthy. The **3
high-confidence** cases are the ones worth a look, since high confidence was billed as
"deterministic pattern, lowest-risk":

| address | confidence | proposed | applied | primary evidence |
|---|---|---|---|---|
| `0x80016768` | high | `psyq_memset_80016768` | `disp_SetFramebufferMode_80016768` | psyq_idiom=psyq_memset_80016768 |
| `0x8007DEE4` | high | `psyq_memset_8007DEE4` | `bb2_memset_8007DEE4` | psyq_idiom=psyq_memset_8007DEE4 |
| `0x8008393C` | high | `syscall_wrapper_break_8008393C` | `bios_FileReadRaw_8008393C` | syscall_wrapper=syscall_wrapper_break_8008393C |

Reading of the 3 high-confidence divergences:
- `0x80016768` — the `psyq_idiom=memset` detector fired, but the applied name is
  `disp_SetFramebufferMode`. This looks like a genuine **detector false-positive** (a
  framebuffer-setup routine mis-identified as a memset idiom) — worth flagging to the
  proposal engine.
- `0x8007DEE4` — proposed `psyq_memset`, applied `bb2_memset`. Same primitive; only the
  naming-convention prefix differs. Detector was essentially correct.
- `0x8008393C` — proposed `syscall_wrapper_break`, applied `bios_FileReadRaw`. It *is* a
  BIOS trampoline; the applied name is just more specific. Detector directionally correct.

Full APPLIED-DIFFERENTLY set (medium tier, 87 rows; low tier, 201 rows)
is enumerated in `proposals_resolved.csv` (filter `resolution_category=APPLIED-DIFFERENTLY`).

## Rows moved to `proposals_resolved.csv`

976 rows total (address now carries a semantic name), broken down as:
- APPLIED-AS-PROPOSED: 31
- APPLIED-DIFFERENTLY: 291
- NO-PROPOSAL-NOW-NAMED: 654

`docs/naming/proposals.csv` is **left unchanged** (per task constraint). The resolved rows
are copied — not moved — into `proposals_resolved.csv`, which adds two columns
(`resolution_category`, `applied_name`).

## Files

- **Created:** `docs/naming/proposals_audit_2026-07-13.md` (this file),
  `docs/naming/proposals_resolved.csv` (976 rows)
- **Regenerated:** `docs/naming/proposals_high_confidence.md`,
  `docs/naming/proposals_medium_confidence.md` (both now empty of pending items —
  every high/medium proposal is resolved)
- **Unchanged:** `docs/naming/proposals.csv`
