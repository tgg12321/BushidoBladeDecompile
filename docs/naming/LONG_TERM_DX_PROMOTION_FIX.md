# Long-term fix for raw-D_X promotion in src/*.c

## Problem statement

Renaming raw `D_XXXXXXXX` references in src/*.c to their semantic
equivalents (e.g., `D_8008EB1C` → `g_winanim_event_subtable_eb1c`)
breaks GCC 2.7.2 + PsyQ codegen catastrophically.  Even a single-line
rename in `code6cac.c:2746` caused **949 functions to diverge** from
SHA1 during a 2026-05-17 test.

The symbol-only rename in `named_syms.txt` alone is safe (the linker
maps both names to the same address), so the current convention is
"keep `D_X` in src/, semantic in named_syms.txt."  But that leaves
src/ readability suboptimal — readers must grep named_syms.txt to
see what `D_8008EB1C` means.

See `memory/feedback_raw_dx_promotion_unsafe.md` for the full
incident report.

## Root cause

Symbols at addresses defined by splat-generated `asm/data/*.s` files
get a `dlabel D_X` directive, which expands (via `include/labels.inc`)
to `.global D_X + .type D_X, @object + D_X:`.  That **defines `D_X`
as a global symbol** in the data section.

When `named_syms.txt` adds `g_x = 0xADDR` for the same address, the
linker has TWO symbols at one address.  This is fine for linking, but
GCC 2.7.2 compiling a src/*.c file that references `g_x` (instead of
`D_x`) produces different `.text` bytes — apparently due to GCC's
symbol-table hash distribution affecting `.L<N>` numbering and
register allocation.  The divergence cascades through the entire
translation unit (all functions in the same .c file diverge).

## Why this only affects symbols with `dlabel`s

BSS-only symbols (no `dlabel` in asm/data/) don't have this issue —
verified by successfully promoting `D_800A26D4` → `g_alarm_secondary_cb_ptr`
in `src/main.c` (3 sites, BSS symbol, SHA1 clean).

The cascade only triggers when:
1. The symbol has a `dlabel D_X` definition in `asm/data/*.s`
2. AND `named_syms.txt` has an alias `g_x = 0xADDR` for the same address
3. AND src/*.c uses the semantic name `g_x` (rather than `D_x`)

The `dlabel` defines `D_x` in the linker symbol table; `named_syms.txt`
defines `g_x` separately at the same address.  GCC sees them as two
different external symbols, and the cross-talk between them causes the
codegen divergence.

## Safety procedure (current convention)

Before promoting any `D_X` ref in src/, check:
```bash
grep "dlabel D_XXXXXXXX" asm/data/*.s
```

| Result | Action |
|---|---|
| `dlabel` exists | **DO NOT PROMOTE**. Leave `D_X` in src/; semantic name lives only in named_syms.txt |
| No `dlabel` | Safe to promote. Use `(?<![A-Za-z0-9_])D_X(?![A-Za-z0-9_])` pattern, verify with clean rebuild |

## Long-term fix plan

The proper fix is to **eliminate the duplicate naming** by making the
semantic name primary in the data section (replace `dlabel D_X` with
`dlabel g_x`).  Then there's only one global symbol per address and
src/ can use the semantic name freely without GCC cascade.

### Step 1: Fix splat installation (BLOCKER)

Splat is currently broken in both worktrees due to a spimdisasm API
mismatch:
```
AttributeError: module 'spimdisasm.mips' has no attribute 'FunctionRodataEntry'
```

Fix by pinning compatible versions in `requirements.txt` or upgrading
splat to a version that supports the current spimdisasm.

Without working splat, **all subsequent steps are blocked**.

### Step 2: Promote names to symbol_addrs.txt

Splat consumes `symbol_addrs.txt` (252 entries) as its primary symbol
input.  Anything NOT in this file gets the default `D_XXXXXXXX` name
when splat regenerates asm/data/*.s.

For each semantic name in `named_syms.txt` that we want as primary:
1. Move/copy the entry from `named_syms.txt` to `symbol_addrs.txt`
2. Keep the entry in `named_syms.txt` too (linker still needs it)

Alternative: write a splat extension that auto-imports promotable
entries from `named_syms.txt` based on a marker (e.g., `// primary`
comment).

### Step 3: Re-run splat with renamed primaries

```bash
python -m splat split splat.yaml
```

Regenerates:
- `asm/data/*.s` — `dlabel g_winanim_event_subtable_eb1c` instead of
  `dlabel D_8008EB1C`
- `asm/funcs/*.s` — references inside function bodies use the new name
- `undefined_syms_auto.txt` — no longer contains the renamed addresses
  (they're now in symbol_addrs.txt)
- `bb2.ld` — linker script updated

Inspect the diff carefully:
- Function bodies in `asm/funcs/*.s` should ONLY differ in symbol-name
  references (the actual mnemonics + addresses are unchanged)
- `asm/data/*.s` should ONLY differ in `dlabel`/`enddlabel` names

### Step 4: Verify SHA1 after regeneration

```bash
make clean && make
```

Expected: SHA1 still matches the original.  If not, the regeneration
introduced a real change (e.g., splat re-classified a symbol's type
or section) that needs investigation.

### Step 5: Bulk-promote D_X → semantic in src/*.c

NOW the bulk promotion is safe — the OLD `D_X` symbol is no longer
defined by `dlabel` anywhere.  The only definition is the semantic
name.  src/ can be bulk-rewritten without GCC cascade.

```bash
# Use the (?<![A-Za-z0-9_])D_X(?![A-Za-z0-9_]) word-boundary pattern
# Process each rename, clean rebuild + SHA1 verify
```

### Step 6: Remove obsolete D_X entries from undefined_syms_auto.txt

After step 3, splat should naturally drop these from
`undefined_syms_auto.txt`.  If any persist (because src/*.c or
asm/*.s still references the old D_X form), the linker will keep
them around as aliases.  Cleanup can be deferred.

## Estimated effort

| Step | Effort | Blocker? |
|---|---|---|
| 1. Fix splat install | 1-2 hours | YES — blocks everything |
| 2. Symbol-addrs migration script | 1-2 hours | Needs step 1 |
| 3. Splat regen + diff inspection | 30 min | Needs step 1 |
| 4. SHA1 verification | 5 min | Needs step 3 |
| 5. Bulk src/ rewriter | 1-2 hours | Needs step 4 |
| 6. Cleanup | 30 min | Needs step 5 |

Total: ~5-7 hours of focused work, contingent on splat install fix.

## Risks

- **Splat regen may unexpectedly change `asm/funcs/*.s`** beyond
  symbol renames.  Splat's heuristics evolve between versions; an
  upgrade might re-classify code/data boundaries.  Mitigate by
  diffing before/after generation carefully.

- **Parallel decomp WIP invalidation.**  If the parallel decomp agent
  has in-progress regfix rules / asmfix entries that anchor to
  function offsets in the OLD asm files, splat regen could break
  them.  Run the regen at a quiet moment when no decomp is active.

- **Some symbols MAY have intentional dual-naming.**  E.g., a symbol
  might be referenced as `D_X` in code that wants the "raw address"
  semantics and as `g_x` in code that wants the documented role.
  Audit before bulk promotion.

## Workaround for "I want to read src/ and see semantic names"

Until the long-term fix lands:

1. **`grep <D_address> named_syms.txt`** — quick lookup
2. **IDE plugins** that resolve external symbols via the linker map
3. **`docs/engine/cross_reference.md`** — address → subsystem index
   with cluster context
4. **`docs/engine/recent_naming_findings.md`** — full cluster traces
   with semantic explanations

For the placeholder-refinement traces (§11-22), every symbol with a
non-trivial semantic role has been documented; the `D_X` form in src/
is functional.

## See also

- `memory/feedback_raw_dx_promotion_unsafe.md` — incident report +
  safety procedure
- `memory/feedback_naming_work_in_worktree.md` — convention for
  named_syms.txt-only naming work
- `include/labels.inc` — `dlabel`/`alabel`/`glabel` macro definitions
- `splat.yaml` — splat config (`symbol_addrs_path` field)
- `Makefile` — linker invocation (`-T named_syms.txt`)
