---
name: prologue-fix-redundant-reorder
paths: ["regfix.txt", "tools/prologue_config.json"]
# broad src/*.c glob removed 2026-06-11: surfaced via codegen-technique-index
description: "A single regfix `reorder` on the prologue + a prologue_config.json entry can be a circular cheat — cc1's natural output already matches, prologue_fix mangles it, and the reorder rule undoes the mangle. Remove both; no C change."
metadata:
  type: reference
---

# A prologue `reorder` regfix rule may just be undoing prologue_fix

## Symptom

A queue item routes `C`, the cheat-free `sandbox --disable all` distance is tiny
(2), and the only cheat is a **single regfix `reorder` rule on the prologue**:

```
func_XXX: reorder 5,10,6,7,8,9 @ 5-10   # "move addiu $sN,$sM,K (idx 10) to right after sw $sN (idx 5)"
```

`diagnose` may say CONTROL-FLOW, but the index-aligned objdump of `build/`
(canonical) vs the stripped sandbox object shows **the function is byte-identical
except ONE instruction's position** — a callee-save register's *definition*
(`addiu $sN, $sM, K`, e.g. `p = base + 0xA`) sits paired right after its *save*
(`sw $sN`) in the target, but is pushed to the end of the prologue in the
cheat-free build.

## The trap — it's circular, and prologue_fix is the culprit

`prologue_fix.py` (the `cpp | cc1 | prologue_fix | maspsx` stage, data-driven by
`tools/prologue_config.json`) is *supposed* to reorder GCC's prologue to match the
target. But its action-matcher can **mangle an already-correct cc1 prologue** when
cc1 emits a single-insn `la` where the config has the expanded `lui`+`addiu`
(index misalignment), pushing the paired def to the very end. Someone then added a
regfix `reorder` rule to move it back. Net: cc1 emits the right order →
prologue_fix breaks it → regfix fixes it. Pure debt.

## Diagnosis (decisive, ~30s)

Dump cc1's *raw* output for the function and compare to the target prologue, then
run it through prologue_fix:

```
mipsel-linux-gnu-cpp <CPP_FLAGS+DEFS> src/<file>.c | tools/gcc-2.7.2/build/cc1 <CC_FLAGS> > tmp/raw.s
grep -A14 "<func>:" tmp/raw.s | grep -E 'sw|addu|la|move|lui|addiu'        # cc1 natural
cat tmp/raw.s | python3 tools/prologue_fix.py | grep -A14 "<func>:" | grep -E 'sw|addu|la|move'  # after pf
```

If **cc1's natural prologue already matches the target** (the save+def pairing is
correct) and **prologue_fix is what moves the def away**, the config entry +
regfix rule are both redundant.

## The save+def pairing follows SOURCE STATEMENT ORDER

cc1's scheduler pairs each callee-save's `sw $sN` immediately with that register's
first definition, in the order the **defs appear in the C source**. So
`base = ...; i = 0; p = base + 0xA;` yields prologue order `s4(base) / s5(i) /
s3(p)` — exactly the target. Reordering the source statements reorders the pairs.
If cc1 isn't already producing the target order, try matching the original source
statement order first (it's usually the natural decl/init order).

## The fix — delete both, change nothing in C

1. **Remove the function's entry from `tools/prologue_config.json`** (so
   prologue_fix passes it through untouched). Validate the JSON
   (`python3 -c "import json;json.load(open('tools/prologue_config.json'))"`).
2. **`retire <func>`** — drops the regfix `reorder` rule and SHA1-gates.
   `sandbox --disable all` should already read 0 once the config entry is gone.
3. The C source needs **no change** (it was already correct).

## Confirmed case — func_80030208 (code6cac_b.c, 2026-05-28)

Queue top, verdict C, distance 2, one regfix `reorder 5,10,6,7,8,9 @ 5-10`. cc1's
natural prologue (source order `base; i=0; p=base+0xA`) emitted `sw $s3 ; addiu
$s3,$s4,0xA` paired — the target order. prologue_fix (config entry present) moved
`addiu $s3` to the end; the regfix rule moved it back. Removing the
`prologue_config.json` entry → `sandbox --disable all` 2→0; `retire` dropped the
rule; SHA1 == oracle. Pure C, zero source edits.

## The OTHER direction — regfix `reorder` with NO config entry → ADD one (sanctioned)

The redundant-circular case above is when a config entry ALREADY exists and cc1
already matches. The opposite, equally common, case: a prologue `reorder` regfix
rule exists with **NO** `prologue_config.json` entry, and cc1's natural prologue
GENUINELY does NOT match the target (a real `mips.c save_restore_insns`
iteration-order difference that source reordering can't flip — the saves of two
INDEPENDENT param→sreg pairs come out in the wrong order). Here the regfix
`reorder` is the cheat and the fix is to **MIGRATE it to prologue_config.json**
(the sanctioned action-based mechanism), not delete it:

1. Add the function's TARGET prologue (named-register asm strings, from
   `asm/funcs/<func>.s`) as a `prologue_config.json` entry. prologue_fix permutes
   GCC's own emitted save/move insns by semantic ACTION to match — it invents no
   bytes and changes no registers, so it is NOT a cheat (the whole prologue_config
   population uses it; `queue done` + `check_completion_integrity.py` accept it).
2. Remove the regfix `reorder` rule. `make` SHA1 == oracle is the proof.

**FOOTGUN (cost me a build, 2026-06-14):** write the config entry from a `.py`
FILE, never a WSL `<<EOF` heredoc — an unquoted heredoc EXPANDS `$s0`/`$sp`/`$a1`
as empty shell vars, silently writing `"sw\t, 0x30()"` (registers gone). The
symptom is `PROLOGUE_FIX: 0 reordered, N unchanged` (it found the func but
`build_target_actions` classified every stripped insn as `('other', …)` so
`action_matches` never fired). Verify with `python3 -c "import json;
print(json.load(open('tools/prologue_config.json'))['<func>'])"` — the strings
MUST still contain `$s0`, `$sp`, etc.

### Confirmed case — func_8007C4B8 (display.c, 2026-06-14)
Single `reorder 3,4,1,2 @ 1-4` regfix, NO config entry. cc1 natural prologue
`sw s1;move s1,a0;sw s0;move s0,a1`; target `sw s0;move s0,a1;sw s1;move s1,a0`
(saves the SECOND param a1→s0 first). Source-order levers don't flip it (9 tested
on twin func_8007C2A0 — the iteration-order wall). Added the 6-line target
prologue to `prologue_config.json`, removed the regfix rule → sandbox 4→0, full
SHA1 == oracle, COMPLETED-C. **Twin func_8007C2A0 (display.c) is the identical
pattern — same signature `(s32 *out, Rect *r)`, same wall — close it the same way.**

## Related
- [[sandbox-zero-retire-fails]] — the inverse coupling (a *source* cheat-asm barrier
  the rules undo); here the coupling is between a *config-stage* and regfix
- [[register-alloc-pure-c]] — when the prologue diff is a genuine reg-alloc/
  scheduling gap with no prologue_fix entry to blame
