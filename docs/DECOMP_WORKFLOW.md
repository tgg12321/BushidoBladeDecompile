# Decompilation workflow and standards

**Audience: any AI coding agent working a function in this repo** — Codex, Cursor, Cline,
Aider, Gemini CLI, Copilot, or a human. Claude Code loads the same standards from
`CLAUDE.md` and the `decomp-grind` / `decomp-manual` skills; this document is the
tool-agnostic equivalent of that *process*, so you do not need those skills to run the
loop correctly.

It is **not** a substitute for `.claude/rules/`. Those files — readable by any agent, not
just Claude Code — are the authority on which constructs are permitted, and §6 below is a
map of them, not a replacement. When a construct question comes up, the rule file decides.

Read [`AGENTS.md`](../AGENTS.md) first for toolchain, build pipeline and file conventions.
Read [`MATCHING.md`](MATCHING.md) when you need *techniques* (how to coax GCC 2.7.2 into a
given shape). This document is about *process and standards*: what "done" means, what is
and is not allowed in the C, and how to prove it.

---

## 1. The one rule

**A function is done when the full build links to a byte-identical executable.**

```
SHA1(build/bb2.exe) == 62efab4f73f992798c43e8c730aa43baa10bb4fa
```

Nothing else is proof. Not an isolated score, not an exit code, not "it looks right".
Every claim you make in a commit message must be backed by a command you actually ran.

---

## 2. Running the tools

All engine commands run inside WSL from the repo root:

```bash
cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile"
source .venv/bin/activate
python3 -m engine.cli <subcommand> [args]
```

From the Windows host, `bash tools/wsl.sh '<one command>'` wraps that. (Claude Code sessions
are required to use `tools/wteng.ps1` instead; other agents do not have that constraint, but
the *shell-nesting* warning in AGENTS.md applies to everyone — anything beyond one simple
command goes in a script file, never an inline `-c` string.)

| Command | What it gives you |
|---|---|
| `queue next` | the top item of the worklist (function, file, verdict, distance) |
| `queue status` | counts by status, plus the current top |
| `canonical <func>` | C-vs-asm routing. **Run before any pure-C work.** |
| `sandbox <func> --disable all` | the honest, cheat-stripped distance for that function |
| `sandbox <func> --disable all --diff` | *where* it differs, per hunk, classed source-level / operand-only / not-scored |
| `sandbox <func> --disable all --candidate FILE` | score a substituted body against a **copy** of src — `main` is never touched |
| `diagnose <func>` | classify the gap: matchable / control-flow / canonical / plateau |
| `verify-oracle --rebuild` | the authoritative gate: full build + link + SHA1 |
| `layer2 hash <func>` | the body's layer-2 key — the reviewer reports it with its verdict |
| `layer2 record <func> --verdict … --reviewer … --scope … --expect-hash …` | record the layer-2 verdict for the reviewed body (refused unless `src/` still holds it) |
| `queue done <func>` | mark complete; re-checks cheat-freedom, a layer-2 PASS on this exact body (owner ruling Q39), and SHA1, refuses otherwise |
| `test` | engine regression suite — keep green if you touch `engine/` |

Two standing audits, run both before you commit:

```bash
python3 tools/check_completion_integrity.py     # category invariants
python3 tools/audit_asm_cheats.py --check-new   # silent when clean
```

---

## 3. The three states

Every function is in exactly one. There are no gradations and no "almost done".

- **INCOMPLETE** — in `engine/queue.json`, committed on `main` as
  `INCLUDE_ASM("asm/funcs", <func>);`. No draft C and no cheat constructs reach `main`;
  work-in-progress bodies live in `memory/grind/<func>/`.
- **COMPLETED-C** — pure C, zero cheat constructs, byte-matches. **This is the default
  goal for every function.**
- **COMPLETED-INLINE-ASM-CANONICAL** — reserved for functions whose *original* code was
  hand-written assembly (GTE/cop2 transfers, BIOS/syscall trampolines). The `canonical`
  command decides what qualifies, **not you**. Listed in `inline_asm_canonical.txt`.

If `canonical` routes a function ASM-REGION or ASM-STRUCTURAL, do not grind it in pure C.
If it routes C, you may not reach for inline asm to finish it.

---

## 4. The worklist

`engine/queue.json` is the single ordered worklist, pre-sorted easiest-first.

**Work the top active item.** No cherry-picking, no triage, no deferral. If an item is
hard, change your *approach* — deeper analysis, different lever, more measurement — never
your *target*. A function that defeats you gets rotated to the back of the worklist with a
recorded reason; it is never abandoned.

**Rotate only when truly stuck across multiple sessions** (owner, 2026-09-26: "i only want
items rotated if agents are truly stuck and we feel we are burning multiple sessions wasting
time on it"). The following are never grounds to rotate:
- one session without a match;
- one adversarial-review FAIL, which bans a construct, not the function;
- an open owner question.

A close item, such as one whose remaining diff is register assignment only, stays at the top.
Bank what you learned in `memory/grind/<func>/` and leave the item active. Full rule:
`.claude/rules/rotation-not-foreclosure.md` Ruling 4.

If you rotate or reclassify anything, **say so in the commit message**. A queue change that
appears only in the JSON diff is an undisclosed change.

---

## 5. The per-function loop

```
queue next
  └─ canonical <func>            # C or ASM? stop here if ASM-routed
     └─ sandbox <func> --disable all --diff    # read WHERE it differs first
        └─ edit src/<file>.c                   # honest C only (§6)
           └─ sandbox <func> --disable all     # iterate to 0
              └─ verify-oracle --rebuild       # the real gate
                 └─ layer-2 review + layer2 record <func>
                    └─ queue done <func>
                    └─ commit
```

Read the `--diff` output *before* choosing a lever. Chasing a hunk the scorer marks
`not-scored` is wasted work.

---

## 6. The standard: honest C

The line, stated by the project owner: *materially-irrelevant minor tricks that help the
compiler are fine; what is not fine is making something look decompiled when it really
isn't.* (`.claude/rules/do-while-zero-exception.md`)

That resolves into three tiers. **Tier 2 is not a judgement call you make from first
principles** — it is membership in a frozen list that only the project owner extends. The
authorities are `.claude/rules/no-new-park-categories.md` (the frozen SOTN-accepted list,
its prerequisites, and the explicit refusals) and `.claude/rules/ordinary-c-judge-decidable.md`
(owner ruling 2026-08-31, the two-tier test). This section is a map of them, not a
replacement — read the rule files before relying on any of it.

### Tier 0 — forbidden outright: bytes or register outcomes imposed from outside compiled C

1. Post-processing rule files that rewrite emitted bytes (the old regfix/asmfix system —
   retired at zero rules; do not reintroduce it in any form).
2. Register pins — `register T x asm("$4")`.
3. `__asm__` in any form **except** the canonical GTE / hand-written-asm category, and
   only where the `canonical` gate has authorized it.
4. Compiler or toolchain divergence — do not change flags, patch the compiler, or swap
   `cc1` to close a function.
5. **Semantic-lie C**: legal C that asserts false facts about the program. Cross-symbol
   address derivation, alias second-handles, struct declarations that misdescribe the
   object, prototypes that misdescribe the parameter, and `volatile` applied to coerce
   codegen. (Two carve-outs, both ruled 2026-07-01: `volatile` on declarations in the
   hardware I/O range 0x1F801000–0x1F802FFF is legitimate type-level semantics with no
   shape test and no annotation — `.claude/rules/mmio-volatile-type-level.md`; and the
   interrupt-touched `extern volatile` allowlist —
   `.claude/rules/legitimate-volatile-interrupt-touched.md`.)

This tier is mechanically enforced: the sandbox scores with cheat-asm stripped, so these
cannot help, and the completion gate audits the source.

### Tier 1 — constructs with a truthful semantic reading: ordinary C

Judged on the **C text**, not on your motive. Choosing among semantically-truthful
spellings after observing codegen is the *method* of matching decompilation —
"scheduling-motivated respelling" is not a FAIL ground when the spelling is truthful
(ordinary-c-judge-decidable Ruling 1(3)). Named intermediates holding a real consumed
value — **written once**; a multi-write carrier is not this class and stays under the
frozen list (`no-new-park-categories.md`, the named-intermediate entry) — plus statement
order, variable reuse, mixed exit forms, and split arithmetic that computes the real
value: all ordinary, no annotation required.

Two constraints still bind:

- **The rename test.** Neutral names must survive. A construct whose justification
  evaporates when you rename the variable never had a semantic reading.
- **Simplest known form.** When several byte-exact forms are known, the one carrying the
  fewest no-semantic-purpose constructs is the one that lands.

### Tier 2 — constructs with NO semantic purpose: frozen list only

A construct that exists only to move codegen and asserts nothing must be a **member of the
frozen SOTN-accepted list with that entry's own prerequisites met**. Non-membership is a
clean FAIL plus a borderline-ledger entry — not a judgement call, not an escalation, and
not something a reviewer may reason its way past. The list is owner-only to extend.

The standard prerequisites on the last-resort entries are all four of:

1. documented lever-exhaustion (what you tried, with measurements),
2. a named GCC-pass mechanism,
3. a mandatory `/* FAKE: ... */` annotation at the construct site naming the observed
   effect and the mechanism,
4. layer-1 *and* layer-2 (fresh adversarial) review.

`do { ... } while (0)` is the one wrapper sanctioned generally, at a single level (nested
wraps need a written, *measured* single-level-insufficient justification). Its syntactic
equivalents are **not** sanctioned by its existence: `if (1) { }`, `for (i = 0; i < 1; i++)`
and `while (1) { ...; break; }` each have to clear the SOTN-evidence bar on their own.

Explicitly refused — do not re-propose in any spelling: fabricated dead call sites
(`if (0) { f(); }`, refused 2026-08-17), the `+= 2 / -= 1` respelling, dead conditional
stores, dead parameter assignments, `(void)&local`, dead local arrays, lost-codegen
inserts, register pins, scheduling barriers, `asm("sym")` alias renames, redundant width
casts.

---

## 7. Ablation discipline — read this before you defend a construct

This is the most commonly violated standard in this repo, and it has produced real defects.

**Removing one piece of a construct cluster and seeing the score get worse does NOT prove
the construct is necessary.** Compilers are non-linear; a half-dismantled contortion
usually scores worse than either the full contortion or the plain form.

**The rule: before you keep any match-motivated construct, delete the entire cluster at
once and rewrite the passage the plainest way a programmer would write it. Measure that.**

```bash
# extract the body you are defending, produce variants, measure each:
python3 -m engine.cli sandbox <func> --disable all --candidate tmp/ablate/plain.c
```

`--candidate` scores against a copy of the tree, so this is free and safe.

### A real case from this repo

`func_8003E2D8` shipped a pointer local, a staging local, and a `goto` into the interior of
another branch's `if` body. The commit defended the pointer as the lever that took the score
from 4 to 0. Measured:

| variant | score |
|---|---|
| body as committed | 0 |
| drop the pointer only | **4** — "proof" it was load-bearing |
| drop pointer + staging local + `goto` together | **0** |

The whole stack was unnecessary. The plain form matched, and a full `verify-oracle` on it
produced the oracle SHA1. Five more functions in the same wave had the same defect.

### Removable is not the same as dishonest

A named intermediate like `s32 color = <expression>;` may also ablate to 0. That does not
make it a defect — it is ordinary, readable C that a 1998 programmer would plausibly write.
Do not strip readable code just because it is removable.

The test is not "can this be deleted?" It is: **does this assert something false, or is it a
transcription of the assembly rather than a statement of the program?**

---

## 8. Defect catalog

Every pattern below was found in landed commits in this repo, defended in a commit message,
and proven unnecessary by ablation. Do not produce them.

**Transcribed arithmetic.** Writing the compiler's instruction expansion back into C.

```c
voiceOffset = ((voice * 8 - voice) * 4 - voice) * 2;   /* the asm, in C */
voiceOffset = voice * 54;                              /* the program — same bytes */
```

**Fabricated aggregates standing in for real types.** A struct with a dead hole in it,
hand-inlining the layout of a type the project already defines:

```c
typedef struct { s16 matrix[9]; s16 pad[7]; s16 rot[3]; } _stack;  /* 14 dead bytes */
MATRIX mtx; SVECTOR rot;                                           /* include/gte.h */
```

Check `include/gte.h` before inventing a stack struct. `MATRIX`, `SVECTOR`, `VECTOR`,
`CVECTOR`, `DVECTOR` already exist and are correct.

**Raw-offset writes into declared padding.** If you are casting to reach a byte, the struct
declaration is incomplete — fix the declaration:

```c
*((u8 *)&s + 0x29) = 0x94;    /* writes into what the type calls padding */
s.byte29 = 0x94;              /* after declaring the member */
```

**Transcribed control flow.** Hand-writing the shape GCC *emits* instead of the shape a
programmer *writes*. GCC 2.7.2 compiles a `for` into a guard plus a bottom-tested loop on
its own — you do not need to write the guard:

```c
i = 0;                                    /* the compiler's output, in C */
if (i < n) { do { ...; i++; } while (i < n); }
for (i = 0; i < n; i++) { ... }           /* the program — same 208 instructions */
```

Same class: duplicating a loop-counter load before the loop *and* at the bottom of the body
to mimic a rotated test, where `while ((count = *p++) != 0)` emits it anyway.

**Lying prototypes — in both directions.** A parameter declared as a scalar and immediately
cast to a pointer, with callers passing arrays through it. And the inverse: deliberately
*under*-typing a callee to `u8 *` / `s16 *` and casting back at every call site, when the
file already includes `gte.h` and already holds the correct `MATRIX` / `SVECTOR` / `VECTOR`
objects being passed. Pointer conversions are codegen-inert on MIPS, so these casts buy
nothing and cost type safety. Declare the real signature.

**Carrier locals with no semantic content.** A variable that can only ever hold one value,
or that is a pure copy of another variable (`player = i;`), existing only to shape register
allocation.

**Per-word aliases for storage inside an aggregate.** Declaring `extern u8 D_8009BC44;`
when `D_8009BC44` is `D_8009BC40[0][2]` and the same function declares that array four
lines above. One storage location gets exactly one C handle.

---

## 9. Evidence you must leave behind

A completion is a claim. These are the receipts:

- **A ledger** in `memory/grind/<func>/` recording what you tried and what it measured —
  including the forms you rejected and why. "I tried things" is not a record.
- **Inline `/* FAKE: ... */`** at every match-motivated construct, naming effect and
  mechanism.
- **Ablation results** for anything you claim is load-bearing, per §7 — cluster removal,
  not single-piece.
- **A commit message that matches reality.** State the commands you ran and their output.
  Do not describe a construct as the closing lever unless you measured the form without it.
  Do not claim a review happened if it did not.

If a construct is genuinely load-bearing, saying so with measurements is fine. Declaring
"constructs: none" when the body carries three of them is a record defect even when the
bytes are correct.

---

## 10. The review gate is not optional

**Before any completion-class commit** (`Match:`, `cheat-cleanup:`, `auth:`), the body must
be reviewed by a *fresh adversarial reviewer* — an agent or person that did not write it,
starting from a default-FAIL posture, not crediting the author's own reasoning.

This is the single highest-value step in the process. Byte-correctness and a green SHA1
cannot catch a false object model or a contorted spelling — only reading can. In the audit
that produced this document, eight of twelve functions carried defects, and every one of
them was in a commit that lacked a genuine review of the committed body.

Three ways this gate gets faked. All three occurred:

1. **Claiming a review that did not happen.** If you cannot obtain a second opinion, say so
   plainly in the commit message. Do not write "independent review PASS" hopefully.
2. **Carrying a checkpoint review forward.** A review of an intermediate body does not
   transfer to the final one. One commit here cited a ledger PASS whose own text read
   *"for checkpoint storage only … Fresh review still required at score 0"* — and the
   committed body was two ladder steps further on. **The review must be of the bytes you
   are committing.** If the body changes after review, it needs a new review.
3. **Crediting the author's reasoning.** A reviewer who starts from the commit message's
   explanation will confirm it. The reviewer must re-derive independently, and should
   ablate (§7) rather than accept any "this construct is load-bearing" claim on its face.

---

## 11. Environment rules that will silently break the build

- **LF line endings** on `src/*.c`, `*.h`, `*.s`, `Makefile`, `*.ld` and the pipeline
  `*.txt` files. A Windows-side editor defaulting to CRLF breaks the GNU toolchain quietly.
- **Never run `make setup`.** `bb2.ld` is hand-maintained; regenerating it re-adds dead
  rodata lines and conflicts with const declarations now living in `src/*.c`.
- **Never recreate `asm/data/*.rodata*.s`.** They were deliberately deleted in 2026-06-09.
- **Do not edit `Makefile`, `engine/buildconfig.py`, gate lists, or detector configs** to
  close a function. These are owner-authorized surfaces. If you believe one genuinely needs
  to change, stop and report it rather than editing it — and if you do change one, it must
  be the subject of its own commit, never a side effect of a `Match:`.
- Scratch goes in `tmp/` (gitignored). Do not create files at the repo root.

---

## 12. Commit conventions

See [`COMMIT_CONVENTIONS.md`](COMMIT_CONVENTIONS.md) for the prefix catalog. For decomp work:

```
Match: func_XXXXXXXX — COMPLETED-C
```

The body should carry: what the function does, the decisive technique, the constructs it
carries (or "none" only if true), the ablation measurements backing any construct claim, the
verification commands and their actual output, and any change to the queue or to a shared
file. Everything in the diff should be explainable from the message.

---

## 13. Where to look next

| Need | Go to |
|---|---|
| **Is this construct allowed?** | `../.claude/rules/no-new-park-categories.md` (frozen list, owner-only) + `../.claude/rules/ordinary-c-judge-decidable.md` |
| Toolchain, build pipeline, disc layout | [`../AGENTS.md`](../AGENTS.md) |
| Matching techniques, symptom → recipe | [`MATCHING.md`](MATCHING.md) |
| Current build state and worklist top | [`STATUS.md`](STATUS.md) |
| Terminology | [`GLOSSARY.md`](GLOSSARY.md) |
| Project history and retired systems | [`HISTORY.md`](HISTORY.md) |
| Commit prefixes | [`COMMIT_CONVENTIONS.md`](COMMIT_CONVENTIONS.md) |
| Past rulings on specific constructs | `docs/grind/decisions.md` |
