---
name: regfix-reference
paths: ["regfix.txt", "asmfix.txt", "regfix_stage2.txt", "tools/regfix*.py"]
description: "regfix.txt syntax, all operations, all known gotchas, mandatory verification procedure. Replaces ~15 fragmented regfix feedback files."
metadata:
  type: reference
---

# regfix.txt reference

`regfix.py` is a per-function asm post-pass between maspsx and `as`. Use it for register-allocation diffs and small structural fixes after C-level options are exhausted.

## Use these tools before writing rules by hand

1. **`dc.sh regfix-suggest <func>`** (added 2026-05-06) — diffs the live build pipeline vs target.s and emits ready-to-use regfix rules with correct maspsx indices. Read its HINTS first (gp-rel pseudo → `sdata_exclude.txt`, `.L<N>` label drift → manual subst with the right label). Then `--apply` the auto rules. Run BEFORE writing rules by hand; you will get indices wrong otherwise. Refuses `--apply` if the suggestion count exceeds `--max-rules` (default 40) — that means the diff is structural and you want permuter/smart_match, not 60 regfix rules.
2. **`dc.sh gen-regfix <func>`** — older auto-generator that emits ~80% of common rules from the pipeline-vs-target diff. `regfix-suggest` supersedes this for most cases.
3. **`dc.sh add-regfix <func> <op> ...`** — for hand-written rules. Pre-validates the rule against `dump_text_indices --post-regfix` BEFORE the build (catches `$0`-vs-`$zero` patterns and out-of-bounds idx in seconds, no build round-trip). If pre-validation fails, fix the rule; do not bypass with `--no-prevalidate`.

## Format

`func_name: operation "args" @ idx` — one rule per line. **NOT** INI-style sections.

```
# CORRECT
coli_HitPauseKatana_2: subst "la\t\$3," "la\t$4," @ 7

# WRONG (silently rejected, no error)
[coli_HitPauseKatana_2]
subst 7 la\t$3, -> la\t$4,
```

If you ever write rules in wrong format, the build silently produces unchanged output. ALWAYS read 2-3 existing working rules in `regfix.txt` before writing new ones.

## Indices

- **0-based per-function**, counting only TEXT instructions (not directives, not labels, not comments).
- **NOT global TEXT indices** across the file.
- **`.word` directives are SKIPPED in counting** — `.word 0xNNNNNNNN` (GTE ops, scratchpad) doesn't get an index. For functions with N preceding `.word` lines, raw_TEXT_idx − N = regfix_idx.
- Pseudo-instructions (`la`, `lb sym`, `sw reg,sym`) count as 1 in TEXT but expand to 2+ in binary. Use TEXT count.
- **Always `dc.sh dump-text <func>` before writing rules** to confirm indices.
- **`dc.sh dump-text <func> --post-regfix`** dumps post-regfix indices for follow-up rules.

## Operations

| Op | Syntax | Phase | Notes |
|----|--------|-------|-------|
| `swap` | `$X <-> $Y @ start-end` (or function-wide) | 1 | Bidirectional REGISTER swap. NOT instruction swap — for that use `reorder`. |
| `subst` | `subst "pattern" "replacement" @ idx` | 2 | Regex replacement, count=1 (only first match per index). For double-occurrence registers (`nor $8,$0,$8` → `nor $2,$0,$2`), write the rule TWICE at same idx. |
| `fill_delay` | `fill_delay @ jal_idx <- source_idx` | 3 | Move `source_idx` text into nop at `jal_idx+1`, queue source for delete. Reads source AFTER swaps+substs (register renames flow through). |
| `drain_delay` | `drain_delay @ jal_idx` | 4 | Move non-nop delay slot insn to BEFORE jal, replace slot with nop. +1 net insn. |
| `delete` | `delete @ idx` | 5 | Removes insn at idx (original idx). Subsequent indices shift -1. |
| `insert` | `insert "asm text" @ idx` | 6 | Inserts BEFORE the idx (POST-DELETE indices). Shifts subsequent +1. |
| `insert_after` | `insert_after "asm text" @ idx` | 7 | Inserts AFTER idx (POST-INSERT indices — see CRITICAL section below). |
| `insert_label` | `insert_label "label:" @ idx` | 6 | Inserts a label without giving it an instruction index. Use for synthesized branch targets like `.LCXXX:`. |
| `reorder` | `reorder N1,N2,...,Nk @ start-end` | 8 | Rearrange insns in range. Numbers are POST-INSERT indices. |

## Phase ordering (CRITICAL)

```
swaps → substs → fill_delay → drain_delay → deletes → inserts → insert_afters → reorders
```

Substs use **original** indices. Deletes use **original** indices. Inserts use **post-delete** indices. Insert_afters use **post-insert** indices. Reorders use **post-insert** indices.

If you delete idx 16, 23, 28 and want to insert before "original idx 73", count: 3 deletes before 73 → use `@ 70`. Run a dry script that simulates if uncertain.

## Phase-specific gotchas

### Substs

- **Pattern `$N` must be `\$N`** — bare `$` is regex anchor (end-of-string).
- **Replacement uses plain `$N`** — regfix calls `re.sub(pattern, lambda m: replacement, text, count=1)` so backslashes pass through literally. `\$N` in replacement = literal `\$N` in output = broken bytes. Symptom: handful of bytes wrong in region targeted by regfix; objdump shows original (unsubst) registers.
- **Parens in patterns must be escaped: `\(` `\)`.**
- **`\.L\d+`** for GCC auto-labels — file-wide labels shift between permuter and full-file build. Hardcoding `.L2` from permuter compilation breaks when build's label is `.L40`. After commit 3fcdef6, regfix WARNS at build time on no-match substs.
- **Same instruction with duplicate register refs needs duplicate rules** — e.g., to change both `$8`s in `nor $8,$0,$8` to `$2`, write the subst twice at same idx.
- **maspsx emits `addu reg,reg,imm` for register+immediate, not `addiu`** — patterns like `addiu\s+\$2,\$8` won't match; the line is actually `addu	$2,$8,4`. Either match `addu\s+...` or omit the opcode and match by register pair: `\$2,\$8` → `$3,$3`. The assembler converts `addu reg,reg,imm` to `addiu` opcode bytes regardless. Bit me on func_800485EC @ 36, 52.

### Inserts at same idx

- Sorted descending; **same-idx inserts process in file order** (stable sort on equal keys).
- First-listed insert places before idx N (call it L1).
- Second-listed insert places before L1.
- Result: second-listed lands above first-listed.
- **To get final order [A, B, original]: list B first, then A.**

### insert_after vs labels

- `insert_after @ N` lands at file position N+1, which is BEFORE any non-instruction lines (labels, .set directives) immediately following N.
- To insert between an instruction and the next instruction with a label between: use `insert_after @ <prev_insn_idx>` to land before the label, OR `insert @ <next_insn_idx>` to land after the label.
- For label synthesis (e.g., `.LCXXX:` for early-exit recipe), use `insert_label` (cleaner than `insert_after` with a label string — doesn't shift instruction indices).
- **Proactive hardening when replacing inline-asm with C in a TU:** before integrating, `grep -n "^<TU_func>.*\.L[0-9]" regfix.txt` for sibling-function rules that reference literal `.L<N>` labels. Adding a C body to the TU advances GCC's file-wide `.L` counter and silently breaks them. Either (a) re-resolve the label numbers in the same commit, or (b) replace literals with synthesized stable names via `insert_label ".LFOO:" @ <where>` + `subst .../.LFOO/...` or `insert "j .LFOO" @ <where>`. The stable-label approach is one-shot — future C additions to the same file won't re-break the rules. Bit me on func_8007CA00 (display.c) when matching func_8007BAB4: the `j .L87` and `j .L81+28` rules became `.L93`/`.L87+28` after adding ~6 GCC labels' worth of C body in front of it.

### Reorders

- POST-INSERT indices.
- **NEVER include idx 0 in reorder** — function symbol label (`func_NAME:`, `.ent`, `.frame`, `.mask`, `.fmask`, `.set noreorder`) attaches as "preceding lines" to the first instruction. Reorder moves them with the new idx-0 insn — function symbol ends up wherever original idx 0 lands. Build SHA1 fails; verify reports MATCH 0 diffs but symbol address is wrong by 4*N bytes.
- For first-instruction swap, use TWO substs that swap the content of idx 0 and idx N (substs replace text in place, never moving lines).
- Reorder rules get a runtime warning when a `reorder` rule spans idx 0 with idx 0 NOT first.
- **Reorder also moves attached labels** — same mechanism as the function-symbol case: a `.LN:` label sitting between idx K-1 and idx K is part of "the group for idx K" and moves with it. If a `j .LN` elsewhere targets that label, the jump now lands at a different instruction. **For two-instruction swaps in a label-attached region, use TWO substs that swap the content** (regfix subst replaces text in place without moving lines, so the label stays put). Bit me on func_800485EC @ 43-44 — the `.L43:` label was between idx 42 and 43; reorder `44,43 @ 43-44` made `j .L43` jump to the addu (new idx 44) instead of the lh (new idx 43). Fix: replaced reorder with two `subst "<old line>" "<new line>" @ idx`.

### swap is REGISTER swap only

- `swap` with a numeric arg (`swap 121 122`) is parsed as register swap and silently does the wrong thing.
- For instruction-position swaps, use `reorder B,A @ A-B`.

## Mandatory verification after regfix changes

1. **`make validate`** — catches pattern drift (regfix pattern no longer matches the instruction).
2. **`python3 tools/verify_labels.py --all`** — MANDATORY after any regfix subst that references a `.L<N>` label in its replacement. Resolves every replacement label to its actual target instruction. Catches the class of bug `make validate` cannot: pattern matches but replacement label resolves to the WRONG instruction.
3. **`dc.sh verify <func>`** — binary-level compare of built EXE function vs original. Now also checks the function's symbol address (catches reorder@0 traps and delete@0 label-drop bugs).
4. **`dc.sh verify --all`** — run after integration to catch label-renumber regressions in unrelated functions.
5. **`mipsel-linux-gnu-nm build/bb2.elf | grep <func>`** — if `verify` says MATCH 0 diffs but build SHA1 fails, the function symbol moved. Check actual address vs original.

## Smoke-test workflow (mandatory)

When adding rules to ANY config (regfix.txt, sdata_funcs.txt, expand_lb_funcs.txt, etc.):

1. Write **exactly ONE** rule.
2. Build and verify the rule had an effect (even if wrong — at least it parsed).
3. THEN write the remaining batch.

Cheap insurance: 1 rule + 1 build = ~15s. Avoids discovering format bugs after writing 43 rules in wrong format. The cost of finding out late was 2 full context windows on coli_HitPauseKatana_2.

When debugging "my change had no effect": check hypotheses in order:
1. **My change isn't being read/applied** (wrong format, wrong file, stale .o cache → `rm -f build/src/<file>.o && make`).
2. **My change is applied but has no effect** (wrong target, pattern doesn't match — check `make` output for `regfix: WARNING:` lines).
3. **My change is applied but something else overrides it.**

NEVER skip to #3 before ruling out #1 and #2. "Nothing changed" = "input wasn't consumed" until proven otherwise.

## Writing regfix rules through WSL

Bash variable expansion eats `$N` even inside quoted heredocs through `wsl bash -c`. Three options:

1. **Native Edit/Write tools** — regfix.txt is not a build file, native edit is fine.
2. **Single-quoted heredoc delimiter** to prevent shell expansion:
   ```
   wsl bash -c "cat > /tmp/script.py << '"'"'EOF'"'"'
   ... python with \$N safely ...
   EOF
   python3 /tmp/script.py"
   ```
3. **`add_regfix.py`** via `dc.sh add-regfix <func> <op> ...` — append-with-rollback, validates and reverts on failure.

Never inline Python with `wsl bash -c 'python3 -c "..."'` for content with `$N`.

## Test patterns in isolation

For new subst rules, write `/tmp/test_regfix.py`:
1. Read maspsx output (saved from `dc.sh dump-text`).
2. Apply each subst manually using `re.sub(pattern, lambda m: replacement, line, count=1)`.
3. Print BEFORE/AFTER for each affected idx.
4. Flag any line that didn't change.

Catches in one pass: `\$` vs `$`, unescaped parens, pattern not matching maspsx format.

## Common patterns / recipes

| Pattern | Recipe |
|---------|--------|
| `lui+ori+sw 0(reg)` (GCC) → `lui+sw offset($at)` (target) | 2 substs (change reg, change offset) + 1 delete (remove ori) |
| LICM unhoist | See `reference/matching-playbook.md` "LICM unhoist" |
| Call-loop family | See `reference/matching-playbook.md` "Call-loop family" |
| Early-exit alias `move v0,a0; beqz` | See `reference/matching-playbook.md` "Early-exit alias breaker"; auto-detected by `gen-regfix` |
| Varargs prologue (small wrappers) | See `reference/matching-playbook.md` "Varargs prologue"; auto-detected by `gen-regfix` |
| Frame size mismatch (mine smaller by N words) | `gen-regfix` emits HINT; add `s32 pad[N/4];` local |
| 4-cycle register rotation | Express as 3 sequential transpositions (swap rules) |
| 3-way register rename (X→Y, Y→Z) | Two overlapping range swaps in order: `Y <-> Z @ R` then `X <-> Y @ R` |

## Tools

| Tool | Purpose |
|------|---------|
| `dc.sh dump-text <func>` | Numbered TEXT indices from build pipeline |
| `dc.sh dump-text <func> --post-regfix` | Post-regfix numbered indices |
| `dc.sh validate-regfix [--func F]` | Static validation: catches unescaped `$`, syntax errors, duplicate idx |
| `dc.sh validate-regfix --live [--func F]` | Live: runs pipeline, verifies each subst pattern matches its idx |
| `dc.sh gen-regfix <func>` | Auto-generates rules from pipeline vs target diff |
| `dc.sh add-regfix <func> <op> ...` | Append + validate + auto-rollback |
| `dc.sh recipes [<func>]` | List/suggest named recipes |
| `dc.sh apply-recipe <recipe> <func>` | Print concrete add-regfix commands for a recipe |
| `python3 tools/verify_labels.py [--func F]` | Resolve label substs to actual target instruction |
| `python3 tools/regfix_preview.py <func>` | Preview regfix operations on slot numbering before editing config |

Recipe JSONs in `tools/recipes/`: licm_unhoist, lui_ori_sw_offset_fold, signed_load_widening, register_swap_simple, multi_subst_register_renames, byte_indexing_with_drain_delay, byte_indexing_with_la_fold_undo, delay_slot_fill, drain_delay_only, missing_extern_silent_elision.
