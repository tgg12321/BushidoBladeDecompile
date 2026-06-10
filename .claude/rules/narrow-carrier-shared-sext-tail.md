---
name: narrow-carrier-shared-sext-tail
description: A shared sign-extension return tail (j .L; sll delay / li; sll; shared sra) needs an s16 carrier — an s32 carrier emits per-path extensions that combine deletes via num_sign_bit_copies
paths: ["src/*.c", "regfix.txt"]
---

# Shared sign-extension tail needs a NARROW (s16) carrier — an s32 carrier gets combine-deleted

## Symptom

A function whose return paths include a narrowing `(s16)` conversion carries a
rule cluster manufacturing target's tail shape:

```mips
jal  callee            # pathA: s16-returning call
 addu a0,v0
j    .L584             # jump to the SHARED sra
 sll  v0,v0,16         # delay slot — pathA's own sll (dbr steal)
.L57C:                 # pathB (early-exit/error path)
li   v0,-1
sll  v0,v0,16
.L584:
sra  v0,v0,16          # ONE shared sra
.L588:                 # epilogue
```

Writing the natural `return (s16)callee(...);` + `return (s16)-1;` loses the
pathB conversion entirely: GCC constant-folds `(s16)-1` → `li v0,-1`, the
shared tail never forms, and the build is 1-2 insns short (rules then
manufacture the sll/sra split + a synthetic label).

## The trap — routing both paths through an s32 carrier still fails

The [[shared-end-label]] restructure (`raw = ...; goto end; ... raw = -1;
end: return (s16)raw;`) with an **s32 carrier** does NOT fix it. RTL-dump
evidence (saFidLoad, tmp dumps of rtl/cse/cse2/combine):

- At expand, `s32 raw = s16_returning_call(...)` emits its OWN sign-extension
  (sll/sra of the HImode call return) inside pathA — so pathA already carries
  a conversion.
- The end block's `(s16)raw` is then a narrowing ROUND-TRIP of an SImode
  pseudo. Combine's `num_sign_bit_copies` sees both defs of `raw` are already
  sign-extended (the pathA sign_extend + the constant -1) and DELETES the
  shared sll/sra, rewriting to a plain move. PathB folds to bare `li v0,-1`.
  Net: same shape as the naive form; the rules stay load-bearing.

## The fix — make the carrier the NARROW type (s16)

```c
s16 ret;
...
ret = coli_result(...);            /* s16 = s16 call — no conversion */
if (ret == idx) {
    ret = tslCDFileRead(ret);      /* s16 = s16 call — plain HImode move */
    goto end;
}
return ret;                        /* reuses the compare's sext via cse */
...
ret = -1;                          /* error sentinel, HImode li */
end:
return ret;                        /* ONE genuine sign_extend — sll/sra */
```

With an s16 carrier:
- pathA's assignment is a plain HImode move (coalesces into the call return,
  zero insns) — no per-path extension for combine to reason from.
- the end block holds a genuine `(sign_extend:SI (reg:HI ret))` — a REAL
  conversion, not a redundant round-trip. `num_sign_bit_copies` cannot
  delete it, and cse cannot fold across the multi-pred `end:` label.
- jump2 cross-jump + reorg's delay-slot steal then emit target's exact tail:
  pathA `j .L584; sll(delay)`, pathB `li -1; sll` falling into the shared
  `sra`.

The narrow type is also the semantically faithful one when the callees are
declared `s16`-returning — the carrier holds an s16 value.

## Confirmed case — saFidLoad (text1b.c, 2026-06-10)

Queue top, verdict C, distance 6, 7 regfix rules (2 reorder + insert_after
sll + delete + insert sra + insert_label + subst) manufacturing the tail,
plus a `*(volatile s32 **)p` coercion cast (removed — the intervening
may-alias store already blocks cse store-forwarding, so the plain re-read
emits identically). Three levers together:
- s16 carrier `ret` + `goto end` shared exit (this rule) — retired the 5
  tail rules.
- `idx = arg1;` + `base = (u8 *)&D_800EFC38;` named locals so the RTL order
  is [sign-extend, la, sll idx*4] matching target's scheduler output
  (HEAD's byte-offset expression put the MULT first per RTL canonical
  order) — retired the `reorder 13,12,14` prologue rule.

Sandbox 6 → 0 (58/58 insns, 0 full-register diffs); retire dropped all 7
rules; SHA1 == oracle; COMPLETED-C. Cheat-reviewer PASS (2026-06-10).

## Related

- [[shared-end-label]] — the parent recipe (defeat per-case constant fold
  via a shared end). This rule is the carrier-TYPE requirement that makes
  it work when the shared work is a sign-extension tail.
- [[exit-path-return-set-cse-join]] — sibling "set the return value in each
  exit path" cse-boundary lever.
- [[switch-break-shared-return-sched-hoist]] — sibling shared-return-tail
  rule for the switch/break shape.
- [[hoist-call-arg-local-flips-jal-delay]] / [[narrow-byte-args-packed-call]]
  — the named-intermediate declaration-order family used for the prologue
  ordering lever in the same match.
