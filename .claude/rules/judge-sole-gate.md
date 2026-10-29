---
name: judge-sole-gate
paths: ["tools/grinder/**", "engine/queue.py", "engine/cheats.py", "inline_asm_canonical.txt", "docs/grind/*.md"]
description: "Owner ruling 2026-08-18: no owner-wait states; the default-FAIL Judge is the sole autonomous acceptance gate; canonical-asm grants are driver-executed on STRONG scanner evidence; borderline questions are logged, never waited on. Includes the endgame-lock AND-gates."
metadata:
  type: rules
---

# No owner-wait states; the Judge is the sole gate (owner ruling 2026-08-18)

Owner: *"I want to remove the concept of a user-escalation/approval ... Anything borderline
can be logged somewhere that we can evaluate later down the line, but i dont want work to
just pile up or be 'parked' pending my decisions anymore."* The standards are unchanged and
permanent ("No cheats, SOTN standard, 100% C or hard evidence for inline asm"); the pipeline
executes them.

## Rules

1. **No pending-owner states.** No "awaiting owner ruling" filings, park-and-wait ESCALATE
   routing, or decision packets. Every disposition is terminal when made; an item that does
   not close stays in the worklist ([[rotation-not-foreclosure]]). A would-be question whose
   YES would lower a standard (breach [[completion-bar]] items 2-4, waive the canonical-asm
   evidence bar) is a plain FAIL, never filed.
2. **The Judge is the sole acceptance gate for autonomous work**: default-FAIL, read-only,
   bound to the static policy (`tools/grinder/roles/judge.md`, rubric [[completion-bar]]). Its only ESCALATE
   kinds are the driver-executed mechanical paths: integration handoff / scope grant
   ([[integration-handoff-self-serve]]) and rule 3. The manual path keeps the layer-2
   `cheat-reviewer` ([[review-discipline-before-commit]]).
3. **Canonical-asm authorization is pipeline-executed.** With STRONG
   `tools/scan_hand_coded.py` evidence and a Judge PASS, the DRIVER (never the Judge) writes
   the `inline_asm_canonical.txt` entry citing this ruling + the scanner evidence and appends
   a borderline entry for later audit. STRONG rests on the gate class (`GATE_CLASS`, owner
   approval 2026-10-15): S6 BIOS-jumptable delay slot, S7 unsaved callee-saved use, S8
   redundant mask before shift, S9 trapping `add`/`addi`/`sub`, S10 jump-table entry inside
   another function, S11 callee-saved register as parameter/result — each decisive on one
   hit, each measured at 0 hits over every compiled-C function (top-level `asm()` in a C file
   is not compiled C) — and
   S1/S2 with tightness corroboration. Hand-written originals stay asm; compiled originals are
   decompiled. Without STRONG evidence asm stays refused, except rule 3a and the GTE macro
   classes in [[inline-asm-policy]].
3a. **Hand-written-module neighbour (owner ruling 2026-10-16).** A function without a STRONG
   tier keeps an EXISTING `inline_asm_canonical.txt` grant (3a never creates one) when ALL
   hold: (a) in `.text` order, every function between it and its nearest STRONG function on
   EACH side is STRONG or meets (b)-(c) itself (judged in the same batch), never compiled C
   or an INCLUDE_ASM function without that evidence. The last function of a module may meet
   (a) on one side only when nop words follow its body (beyond its delay slot), its end with
   those nops falls on a 16-byte boundary, and the next `.text` function belongs to a
   different module (Sony's assembler padded each hand-written module's `.text` to 16;
   [[rodata-object-alignment]]); the other side still meets (a);
   (b) its own body shows scheduling GCC 2.7.2 cannot emit, named per function: e.g. a cop2
   op in the `jr $ra` delay slot. Hard `$t*` cop2 transfers alone do not count (the
   vendored `inline_c.h` macros emit them from compiled C), nor does `lui`+`ori` alone (this
   build emits it for a volatile absolute store); (c) no genuine likelihood of
   C: the body is not C-shaped, and no C or C-plus-GTE-macro-call candidate reproduces it
   (none within endgame distance). One cheat-reviewer checks (a)-(c) per function; the row
   gains `[re-audit <date>: hand-written module <lo>-<hi>]`. Owner per-function ruling
   2026-10-16 (this function only, not a class): sys_StoreZeroPastScratchpadOnFailedCheck
   (0x80052C10) meets (b)-(c); `$t9` is reachable from C only under register pressure forcing
   a ~110-insn framed body (memory/grind/sys_StoreZeroPastScratchpadOnFailedCheck/evidence.md
   s2). Its extra nop ends 8-aligned, not module pad.
4. **No extension requests exist (owner ruling Q91).** A construct outside the pre-cleared
   shapes in [[no-new-park-categories]] is decided directly on [[completion-bar]] item 3:
   labelled, honest, not on item 3's refused list, existing types used and the simplest
   known form ⇒ admissible (items 3-5); lying, a refused construct, a second extern / alias symbol / raw-offset cast for declared bytes (item 4),
   a heavier form, or behind the wall ⇒ FAIL. A SOTN citation
   ([[sotn-precedent-suffices]]) is supporting evidence, not a requirement.
5. **Reviewer NEEDS_USER = FAIL + a `needs-user-downgrade` borderline entry.** The agent may
   re-invoke with genuinely NEW evidence but may never re-adjudicate the recorded question.

## Endgame-lock gates (owner policy 2026-07-20, standing 2026-07-27)

For a function a few instructions short in honest C with its sanctioned levers exhausted
(usually an RA/scheduling tiebreak), the only non-C exits are two AND-gates, default refuse:

- **Gate 1 — canonical asm** only with a STRONG `scan_hand_coded --single <fn>` tier (a
  rule-3 gate-class signal); rule 3a never opens it (3a only keeps an existing grant).
  Otherwise a LOW tier is dispositive: a
  compiler-scheduling/RA artifact is ordinary GCC output, never a hand-coded signature.
  Passing ⇒ rule 3's grant path.
- **Gate 2 — a labelled FAKE spelling** under [[completion-bar]] item 3 (Q91): honest,
  annotated with its measured reason, simplest known form. Coercions that breach items 2-4
  (pins, volatile outside its catalog, invented object models) stay refused.

Both fail ⇒ the function stays `INCLUDE_ASM` and in the worklist ([[asm-until-matched]]);
nothing is asked of the owner. Owner: "My standards will never change."

## The borderline ledger — `docs/grind/borderline.md`

Informational; nothing in it is pending. Entries are appended; outdated ones may be deleted by
an owner-directed cleanup (cite a deleted entry via the pin commit named in the ledger header).

```
## YYYY-MM-DD — <function or scope> — <category>
category: canonical-asm-grant | family-candidate | needs-user-downgrade | policy-question
evidence: <pointers, not prose dumps>
disposition taken: <what the pipeline did under current policy>
```

An entry authorizes nothing. Only a later owner ruling does, landed as its own `rules:` commit
before any code spends it.

Related: [[review-discipline-before-commit]] · [[integration-handoff-self-serve]] ·
[[rotation-not-foreclosure]] · [[canonical-asm-authorization-recipe]] · [[inline-asm-policy]]
