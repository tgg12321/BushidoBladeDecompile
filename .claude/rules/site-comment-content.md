---
name: site-comment-content
paths: [".claude/rules/site-comment-content.md"]
description: "BLOCKING (owner ruling Q118, 2026-10-06): what a site comment must carry -- the FAKE label + measured reason, the admitting ruling id, the SOTN tag / pinned-source citation, true statements about an asm island's macro (departures in its row or the landing record); every other citation a rule asks the comment for may live in the landing commit body or ledger."
metadata:
  type: rules
  tier: blocking
---

# What a site comment must carry (owner ruling Q118, 2026-10-06)

The owner chose this option: *"at the site only the FAKE label + measured reason + ruling id
(+ SOTN tag) are required; addresses, instruction cites, ledger paths and dossier prose live in
git history."*
Linked from [[completion-bar]] item 3; blocking.

## At the site (required)

1. **The label**: `/* FAKE: <its codegen effect>; <measured reason> */` (or `// !FAKE ...`, or
   `FAKE: frame layout`) when the construct has no semantic purpose ([[completion-bar]] item 3).
2. **The admitting id**, unique, most specific first: `Q<n>`; else `Ruling <n>` (bare means
   [[ordinary-c-judge-decidable]]'s Rulings 1-13; another file's numbered or lettered ruling
   names its file, e.g. `Ruling-4 (legitimate-volatile-interrupt-touched)`, `Ruling A
   (cop2-addressing-preamble-cluster)`); else the admitting rule's
   name (e.g. `(dead-store-fake-exception)`); a dated owner ruling (`owner ruling <YYYY-MM-DD>`,
   plus a topic word when another ruling shares the date) only when no rule file records it.
   The id is the ruling or route that admits the construct today; a retired grant may be
   mentioned as history, never as the admission.
3. **The source that admits it**: the `/* SOTN: <file>:<line> @<commit> */` tag for every
   construct admitted on a SOTN citation ([[sotn-precedent-suffices]]; a Ruling 7 or 8 site
   keeps the SOTN reference its comment makes, in any form), and the Ruling 10 pinned-source
   citation (`<repo>@<commit>:<path>:<line>`) where one admits it (Ruling 10 (D) keeps its
   citation; only its form is fixed here).
4. **Asm islands**: what an island comment says about its macro is true -- no "verbatim",
   "character-identical" or "as published" unless the island is that text, and a macro it
   names is the one the island implements. Its departures from the macro text are recorded in
   its `inline_asm_canonical.txt` row or the landing record; the comment may repeat them.
5. **Honesty content**: whatever the comment says about what the code does stays true and
   complete enough not to mislead, including facts a ruling names as the reason (Q76's 4.12
   factor, Q82's sign-extend pair, Q90's fold-const rewrite, Ruling 7's "what it computes").

## In the landing record instead (permitted)

Every other item any rule or ruling asks a site comment to carry, however it is worded ("at the
site", "inline", "(cited)", "the comment names / cites", "row and comment cite"): target
addresses and instructions, DMPSX placeholder and command words, ledger and evidence paths,
header releases and line ranges, frame equations, named GCC-pass write-ups, measurement tables,
dossier prose. These need not be at the site; the landing commit body, the ledger or the
`inline_asm_canonical.txt` row carries them. This overrides those clauses in every rule
(including aggregate-declaration-views Q21/Q33/Q36, reused-local-necessity Q34/Q74/Q75/Q85 and
Rulings 11-12 (F), reused-local-meaning-source Rulings 7/10, phantom-frame-pad-family item 6,
proven-spelling-class-reconstruction 3, inline-asm-policy 2026-09-23 (1) and Extension (C),
cop2-addressing-preamble-cluster check 3, ordinary-c-judge-decidable Q76/Q82/Q90). They may
also stay at the site when short.

Related: [[completion-bar]] · [[sotn-precedent-suffices]] · [[inline-asm-policy]]
