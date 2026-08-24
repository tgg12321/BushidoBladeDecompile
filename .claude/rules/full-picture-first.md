---
name: full-picture-first
paths: ["memory/grind/**", "docs/grind/*.md", "engine/queue.py", "engine/dossier.py", "src/*.c"]
description: "Owner directive 2026-08-24: before working or investigating ANY function, run `& tools/wteng.ps1 main dossier <func>` — the mechanically-assembled full picture (aliases, queue, src, rules, ledger, record trail, consistency audit). Formalizes context-first-before-grinding."
metadata:
  type: rules
---

# Full picture first — the dossier is the entry point (owner directive 2026-08-24)

> Owner: "It's not uncommon for us to investigate an item and realize there is
> information we missed, work that slipped through the cracks, stale docs or
> general confusion and wasted time... How can we formalize things to be sure
> every time an item is worked or investigated, we have the full, most
> up-to-date picture regarding that function?"

A function's truth lives across ~10 surfaces (queue item, src representation,
rule files, grind ledger, decisions/journal/borderline, canonical list, gate
lists, wip leftovers, name aliases). Hand-gathering them misses different
pieces every time — measured failure classes: parked-but-proven (2026-08-22),
invisible directives, stale-HEAD ledger claims, alias-trapped rulings, and
records-vs-tree contradictions (all 2026-08-24). The fix: the full picture is
a BUILD ARTIFACT, not a research task.

## The mandate

1. **Any session, investigation, or review that touches a specific function
   STARTS with** `& tools/wteng.ps1 main dossier <func>`. It assembles:
   aliases (rulings often live under OLD names), the live queue item +
   directive, the verified src representation (code-line, never substring),
   every rule/gate/config membership, the ledger digest, the record-trail
   headings, and a cross-surface CONSISTENCY audit. Grinder sessions get the
   alias/consistency/directive blocks injected into their brief
   automatically (grindlib full-picture hook).
2. **`dossier --audit-all` is the standing drift detector.** Run it before
   every grinder launch and after any bulk operation (migration wave, naming
   wave, queue conversion). A warning is work: resolve it or file it — never
   scroll past it. (Its first-ever run caught regen silently dropping all 30
   owner directives from active items.)
3. **Write-discipline — don't create tomorrow's stale picture:**
   - Current-state claims in ledgers/docs ("HEAD carries X", "this IS the
     committed body", "N rules retained") must be **anchored to a commit or
     date** — bare present-tense HEAD claims are the #1 measured rot class.
   - Counts in prose are pointers-to-commands, not numbers, wherever a live
     command exists (`queue status`, `check_completion_integrity`).
   - A rename updates the borderline alias table in the same change
     (`dossier` and the brief resolve aliases from it).
   - A bulk tool that changes one surface updates EVERY coupled surface in
     the same commit (queue + src + rules + ledger + docs counts) — partial
     updates are how records end up asserting the opposite of the tree.

## Related

[[context-first-before-grinding]] (the 2026-08-20 directive this formalizes) ·
[[escalation-not-parked]] · [[verify-claims-against-main]] ·
[[grinder-stale-digest-uncommitted-ledger]]
