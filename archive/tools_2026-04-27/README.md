# Archived tools (2026-04-27)

Tools moved here during the foundation rebuild. Each was either a one-off
experiment, file-specific scripting, or a now-obsolete predecessor of a
canonical tool. Kept for git history / reference.

| File | Reason |
|------|--------|
| `score_remaining.py` | Wave-13-era; hardcoded file list; superseded by `dc.sh score` + `rescore_drafts.py` |
| `test_8003553C.py` | One-off: variants for a single function |
| `test_asm_barrier.py` | Experimental: asm barrier placement; learnings folded into `feedback_*` memories |
| `test_asm_constraint.py` | Experimental: asm() constraint combinations; ditto |
| `test_decl_order.py` | Experimental: variable declaration reordering; folded into `smart_match.py` |
| `test_frame_sizes.py..4` | Experimental: frame-size search for one function |
| `test_newvar_split.py` | Experimental: temp variable splitting; folded into `smart_match.py` |
| `test_variants.py` | Experimental: multi-variant emission |
| `decompile_text1a.py` | File-specific m2c run; superseded by `auto_decompile.py` + `batch_permuter.py` |
| `fix_text1b.py` | One-off; not used in current pipeline |
| `patch_f6d8.py` | One-off; targeted single function |

## Restoration

If a tool turns out to be needed: `git mv archive/tools_2026-04-27/<x> tools/<x>`.
