---
name: constants-bar
paths: [".claude/rules/constants-bar.md", "include/psx.h", "include/psxsdk/*.h", "include/bb2_const.h", "tools/const_wave.py"]
description: "Owner ruling Q135 (2026-11-05): the constants / enums track. A literal becomes a named constant per use site, only where that site plays the named role; Sony spelling first; byte-neutral by preprocessed / compiled identity; a fresh naming-reviewer passes every site."
metadata:
  type: rule
  tier: hygiene
---

# The constants bar

Phase 6 of the readability work: magic numbers become named constants. Applies to every
`#define` / `enum` a constants wave lands or uses; it never decides completion
([[completion-bar]]).

## K1 — per site, by role

A literal is replaced at a site only when that site uses it in the named role: `0x10` read
against the pad buffer is a pad bit; the same `0x10` as a size stays a literal. Every site a
wave changes is listed with `file:line` and its role. One site whose code contradicts the
role fails the row.

## K2 — evidence and spelling

[[naming-bar]] N1-N4 apply to the constant's name, in N3's conflict order:
1. Sony's own name, verbatim: the PsyQ 4.0 header (`CdlSetfilter`, `CdlModeSpeed`,
   `PADLup`...) or the hardware register bit. Library code and its callers' Sony arguments
   take Sony's spelling; a game-invented name for a Sony value fails.
2. SOTN / psyz for the same Sony value.
3. Explained from the code, or runtime ([[naming-runtime-evidence]]) for game state / mode
   codes: every site that tests or stores the code agrees on what it selects.

Lacking evidence, the literal stays (N2: a literal claims nothing).

## K3 — byte-neutral, proved per TU

Scope: every TU of the tree (cpp decides what each one reads).
- Bit masks, commands and sizes: `#define`. Proof: cpp output identical to base apart from
  line markers, when the macro body is the site's own token and no `#include` is added.
- Otherwise (an `enum` for a closed set of codes stored in one variable or member, a body
  spelled differently from the site such as `(1 << 12)` for `0x1000`, an added include): cc1
  output `.s` identical to base. Do not retype the variable in the same wave.
- Never a site in a string literal, an `__asm__` template or a `.s` / `.inc` file.
- Then `verify-oracle --rebuild` (SHA1 == oracle) and the usual foundation gates.
`tools/const_wave.py` is the applier and checker: `apply <manifest>`, then
`check <manifest> --base <rev>` before committing (in WSL; refuses literals that are not one
token, unique on the line, outside directives, strings and `__asm__`; a wave never touches a
comment, and a site whose trailing comment `tools/format.py` would hoist; move that comment in
its own commit first). One `naming: const-<wave>` commit holds the manifest
(`docs/naming/phase3/pending/const-<wave>/const.tsv`) and the applied edits; a manifest under
any other subject, or a wave whose replay needs its `Lane-Replay` re-run, fails `land`.
`land` runs main's `check` on every such commit: strict, plus `--loose-lines` on the replay
when main moved.

## K4 — placement

Sony constants in `include/psxsdk/lib*.h` next to their library's prototypes (used only inside
the library: its `*_internal.h` / `libsnd_i.h`); hardware bits
in `include/psx.h`. Game constants used by several TUs in `include/bb2_const.h`, one TU's in that TU.
One definition per value-role; never a second name for the same role.

## K5 — the wave

Manifest `docs/naming/phase3/pending/const-<wave>/const.tsv`, rows
`kind where literal name role evidence` (`site` file:line, `define` / `enum` file; format in
the tool's docstring); a fresh
`naming-reviewer` CONFIRMs only the rows it PASSes against K1-K4; the applied diff gets the
post-apply pass. Commit `naming: const-<wave>`, one per wave. A completed body whose layer-2 key
moved only by the wave's sites needs no new cheat review: the K3 proof and the post-apply pass
cover it (as [[naming-bar]] wave step 6).
