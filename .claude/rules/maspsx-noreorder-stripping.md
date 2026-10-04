---
name: maspsx-noreorder-stripping
paths: ["tools/maspsx/**"]
description: "maspsx silently strips TAB-form `.set noreorder/noat/reorder/at` from file-scope __asm__ blocks; add SPACE-form duplicates or later functions in the .c file drift (wrong reorder mode)."
metadata:
  type: reference
---

# maspsx silently strips TAB-form `.set` directives

Every file-scope `__asm__()` block using `glabel`/`endlabel` (canonical asm, `inline_asm_canonical.txt`) MUST
duplicate each TAB-form `.set` directive with a SPACE-form one (reference: `func_8004A76C` at `d4e3de00a^:src/text1b.c:1677-1682`):

```c
__asm__(
    ".set\tnoat\n"        /* TAB form -- maspsx state tracking */
    ".set\tnoreorder\n"
    ".set noat\n"         /* SPACE form -- reaches `as` */
    ".set noreorder\n"
    "glabel func_NAME\n"
    "    ...body...\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
```

**Mechanism.** maspsx (`tools/maspsx/maspsx/__init__.py`, ~line 894) uses the TAB-form lines to update its
`is_reorder` tracker and DROPS them; SPACE-form lines pass through to `as`. Without the SPACE form, `as` never
enters noreorder for the block (hand-placed delay slots get nop-filled) or never returns to reorder mode
afterwards (later functions' bare branches stay unfilled).

**Symptom.** After adding/editing a file-scope asm block, the edited function matches but a LATER function in
the same .c file differs (often a 1-instruction branch-offset shift; the file grew). Add the SPACE-form
duplicates and rebuild — do not "repair" labels elsewhere.
