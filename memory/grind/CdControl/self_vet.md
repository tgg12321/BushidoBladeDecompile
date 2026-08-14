# SELF-VET — CdControl

Diff under vet: `src/system.c` lines 134-183 (the whole CdControl body), HEAD
form vs the session-2 form. Honest sandbox distance 0 (78/78 instructions),
`sandbox CdControl --disable all`, measured this session with these exact edits
in place in src/.

What the diff REMOVES (all cheats, all inherited from HEAD):
  - `register s32 result asm("s7");` — a register-asm pin (forbidden family).
  - `unsigned long long new_var2; int new_var; new_var = 3; new_var2 = new_var;
    count = new_var2;` — a DImode chain for the constant 3 (forbidden family:
    "DImode chain for scheduling").
The three regfix rules (regfix.txt:58-61) are not touched by me; they are inert
under the cheat-invisible sandbox and are the operator's `retire` step.

CONSTRUCTS: do-while(0) wrap around the retry-loop body (FAKE-annotated);
`u8` declared type for the command parameter a0; two-step
`base = g_cd_sector_buf; elem = base + idx;`; initialiser-statement order
(idx, saved, count, base, elem, result).

## T1 semantic purpose
- **do-while(0) wrap**: no semantic purpose — the body executes exactly once
  with or without it. This is the one construct in the diff that is a pure match
  device, it is annotated `/* FAKE: ... */` at the construct site, and it is
  claimed under a sanctioned family below (which is exactly the family defined
  for no-semantic-purpose wraps).
- **`u8 a0`**: real semantic content. This is the actual PsyQ libcd signature
  (`int CdControl(u_char com, u_char *param, u_char *result)`), the parameter is
  a CD command byte, and the declared type is the true program fact. It is also
  the type both already-matched siblings in this file use (`CdControlF`,
  `CdControlB`, src/system.c:184,236). It changes emitted bytes for a real
  reason: a u8-typed value is not known sign/zero-extended on entry, so GCC
  masks it at its uses — which is what the target does (`andi $s3,$s4,0xFF`,
  `andi $a0,$s4,0xFF`; asm/funcs/CdControl.s:12,56).
- **two-step base/elem**: real semantic content — a named base pointer and an
  element pointer derived from it; ordinary C, no dead value.
- **init order**: statement order of six live, all-read initialisers. Every one
  of them is consumed later; nothing is dead.

## T2 human-programmer
- `u8 a0`: yes — a human decompiling libcd from the PsyQ headers writes exactly
  this, and would call the s32 form a typing bug. It removes rather than adds a
  "why is this here?" question, and it let me delete the two `& 0xFF` masks that
  the s32 form needed (measured inert at u8, so the cleaner spelling was taken).
- two-step base/elem, init order: yes — unremarkable C.
- do-while(0) wrap: **no** — a reader asks "why is this here?". That is why it
  carries the FAKE annotation and is claimed under the sanctioned family rather
  than defended as program logic.

## T3 GCC-internals justification
- `u8 a0`, base/elem, init order: justified by program logic and by the PsyQ
  library signature, not by GCC internals. (I also measured them — measurement
  is not the justification.)
- do-while(0) wrap: yes, the justification IS GCC-internal (flow.c
  `reg_n_refs += loop_depth` feeding global.c `allocno_compare`). Under the
  general checklist that is the cheat signal; the do-while(0) family is the
  named, owner-ruled exception to it for this exact construct, and its rule
  explicitly authorises "ANY codegen effect, including register allocation".
  The annotation is effect-level as the rule's prerequisite 1 requires.

## T4 permuter/search provenance
No permuter was run this session. The sweeps I ran (init order, declaration
order, mask on/off, param type, named intermediate) are enumerations of
hand-chosen structural axes, and their outputs are ordinary C spellings, not
search-discovered constructs. Every construct in the final form is one I can
justify without reference to the sweep: the wrap was inherited from session 1
and from the matched sibling CdControlF; `u8 a0` is the documented PsyQ
signature. Nothing in the diff passes a detector only because of its spelling —
the wrap is the most-detected construct in the project and is declared here.

## T5 family check
- do-while(0) wrap → SANCTIONED family, claimed below with scope + precedent.
- `u8 a0` → not a forbidden family. It is not an alias rename (one C handle,
  one symbol), not a volatile coercion, not a redundant width cast (the
  opposite: adopting the true type let me DELETE two now-redundant `& 0xFF`
  masks), not a dead store, not a pin, not asm. Nearest listed family is
  "redundant width casts (F2)"; this diff moves away from that family, it does
  not join it.
- two-step base/elem, init order → sanctioned "named-intermediate declaration
  order" / "statement order" territory, and in any case ordinary live C.
- No dead local, no dead store, no self-assign, no address-of discard, no
  unused array, no volatile, no `__asm__`, no register pin anywhere in the diff.
  The diff strictly REMOVES two forbidden-family constructs from HEAD.

## T6 naming-announces-intent
Locals are `result, idx, saved, count, base, elem`. None is `pad`, `dummy`,
`unused`, `spill`, `tmp`, `slack`, `_buf`. Each names the value it holds and
each is read: `result` is returned, `idx` guards and indexes, `saved` is stored
back to `g_cd_callback_a` on both exit paths, `count` is the retry counter,
`base` is the array base, `elem` is dereferenced in the loop. No declaration in
the diff exists solely to be declared.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) wrap
  SCOPE: "`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation."
  PRECEDENT: 589bf161
  (also `.claude/rules/do-while-zero-exception.md:23`; the same construct with
  the same mechanism is committed in the sibling CdControlF at
  `src/system.c:205`. Prerequisite 2 "prefer natural geometry first" is met
  beyond its bar: the wrap-free geometry was swept exhaustively this session —
  all 240 legal initialiser orders and all 720 declaration orders score 17, and
  session 1's honest real-loop restructures scored 13. Prerequisite 3 does not
  apply: single-level wrap, not nested.)

ANNOTATION-CONFORMANCE:
  /* FAKE: loop-note ref weighting seats count/a1/a2/idx/a0/saved/elem/result
     in s0..s7, mechanism: flow.c life analysis (reg_n_refs += loop_depth)
     feeding global.c allocno_compare, lever-exhaustion:
     memory/grind/CdControl/hypotheses.md (s1: 2x240 init-order sweeps, honest
     real-loop restructure measured worse at 13; s2: 240 wrap-free init orders
     floor 17, 720 declaration orders inert, mask/param/named-intermediate
     axes measured) */
  Carries all three: WHAT (loop-note ref weighting, with the seating it
  produces), MECHANISM (named GCC passes: flow.c life analysis, global.c
  allocno_compare), LEVER-EXHAUSTION (pointer to the hypothesis ledger with the
  specific sweeps and their measured floors). It sits inline at the construct
  site, not in file-header prose.
