---
name: maspsx-noreorder-stripping
paths: ["tools/maspsx/**"]
# broad src/*.c glob removed 2026-06-11: surfaced via codegen-technique-index
description: "Maspsx silently strips TAB-form `.set noreorder` / `.set noat` / `.set reorder` / `.set at` directives from file-scope `__asm__()` blocks. The SPACE-form duplicate is required for the directive to survive to `as`. Without it: the asm body itself is in the wrong reorder mode (inserts unwanted nops in branch delay slots), AND subsequent functions in the same .c file inherit the wrong mode (their delay slots also get wrong handling). The symptom shows up as cascade drift in functions AFTER a newly-added canonical-asm block."
metadata:
  type: reference
---


# maspsx silently strips TAB-form `.set` directives

## The rule

Every file-scope `__asm__()` block that uses `glabel`/`endlabel` (the
canonical-asm form recorded in `inline_asm_canonical.txt`) MUST duplicate each
TAB-form `.set` directive with a SPACE-form version. The canonical
pattern (see `src/text1b.c` `func_8004A76C` for reference):

```c
__asm__(
    ".set\tnoat\n"        /* TAB form -- needed for maspsx state tracking */
    ".set\tnoreorder\n"
    ".set noat\n"         /* SPACE form -- needed to reach `as` */
    ".set noreorder\n"
    "glabel func_NAME\n"
    "    ...body...\n"
    "    jr     $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
```

**Why:** Maspsx's preprocessor (`tools/maspsx/maspsx/__init__.py`
around line 894) reads incoming asm one line at a time and SILENTLY
DROPS any line that exactly matches `.set\tnoreorder`,
`.set\treorder`, `.set\tnoat`, or `.set\tat` (TAB form) after using
it to update its internal `is_reorder` tracker. The SPACE form
(`.set noreorder`, etc.) is not in the drop list — it passes
through to `as`.

**Why:** Reason: maspsx wants to be in noreorder mode itself for its
own delay-slot insertion analysis, but it expects `as` downstream to
be in reorder mode (so `as` auto-fills delay slots from cc1's normal
output). Stripping the TAB form keeps maspsx and `as` decoupled on
state.

For a file-scope `__asm__()` block this decoupling is wrong: you
DO want `as` to see your `.set noreorder` so it preserves your
hand-placed delay-slot fills exactly. You also want `as` to see
your closing `.set reorder` so it returns to reorder mode for
subsequent functions in the file (which expect to be auto-filled).

## Why: How to recognize the symptom

After adding or modifying a file-scope `__asm__()` block, the build
shows SHA1 mismatch but the function you just added/edited matches.
The diff is in a function LATER in the same .c file — usually 1
instruction off in a branch offset (`bne $X, $Y, .L<N>` jumps to a
slightly different address). `dc.sh verify --all` reports something
like:

```
1 functions with diffs:
  func_<later>: 1 instruction(s)
```

The auto-repair may misdiagnose this as label drift and rewrite a
regfix rule's `.L<N>` to a different label — that "fix" is wrong
and must be reverted. The rollback safety net added 2026-05-16
catches this case automatically (see `auto_drift_repair.py`).

## How to apply: When the auto-repair complains

When `dc.sh build-active` reports SHA1 mismatch on a file-scope
asm block you just added or modified, the auto-repair (post-
2026-05-16) automatically runs `_scan_unbalanced_set_directives()`
and prints a hint if it finds the issue:

```
[auto-repair] HINT: detected file-scope `__asm__()` block(s) with
TAB-form `.set` directives MISSING SPACE-form duplicates:
[auto-repair]   src/text1b.c:10518  missing SPACE-form `.set noreorder` duplicate
[auto-repair]   src/text1b.c:10518  missing SPACE-form `.set noat` duplicate
...
```

If you see that hint, add the SPACE-form duplicates to your block
and rebuild. No regfix rewrites needed.

## Reference incident

`func_800526A0` (2026-05-16). Match attempt initially used only
TAB-form directives in the new canonical-asm block. Build passed
for `func_800526A0` itself (verify-c showed MATCH) but `func_80074B18`
later in `text1b.c` had a 1-byte bne offset diff. Auto-repair
misread this as label drift and rewrote a regfix rule from `.L1076`
→ `.L1080` — the new label resolved but pointed to the wrong byte
position (label numbering hadn't changed; the function had GROWN
by 12 instructions because `as` was in noreorder mode and couldn't
auto-fill delay slots, so the cc1-emitted bare branches stayed
bare). Adding the SPACE-form duplicates restored `as` to reorder
mode for subsequent functions and matched immediately. The
spurious `.L1076` → `.L1080` rewrite had to be reverted manually.

See also: [[bridge-is-not-decomp]] (avoid asm shortcuts as a
default), and `inline_asm_canonical.txt` for the list of
functions where file-scope asm IS canonical.
