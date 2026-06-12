---
name: store-const-reload-cse
paths: ["regfix.txt"]
# broad src/*.c glob removed 2026-06-11: surfaced via codegen-technique-index
description: "A single regfix `subst` swapping a load-immediate back to a global memory load (e.g. `addiu $3,$zero,2`→`lbu $3,D_800A36FA`). Cause: source saves a reload into a local (`G=N; v=G;`) after storing constant N, so GCC forward-props N into a `li`. Fix: drop the saved-local reload and re-read the GLOBAL directly at the later test — the store kills GCC's CSE entry so it reloads only in the store path. Do NOT use `volatile` (adds addiu+nop+andi)."
metadata:
  type: reference
---

# `subst "addiu $r,$zero,N" "lbu/lw $r,GLOBAL"` — store-then-reload folded to `li`; reload the GLOBAL, don't save a local

## Symptom

A function carries a single regfix `subst` that swaps a load-immediate back into a
memory load of a global:

```
# func_XXX: GCC forward-props GLOBAL=N store to li, target reloads from memory
func_XXX: subst "addiu\s+\$3,\$zero,2" "lbu\t$3,D_800A36FA" @ 20
```

The C stores a constant to a global, then reads the global back into a local that
is later tested:

```c
s32 v3 = GLOBAL;            /* entry load -> lbu $v1 */
if (v3 == 1) {
    if (flagsA || flagsB) {
        GLOBAL = 2;         /* sb */
        v3 = GLOBAL;        /* <- reload; GCC forward-props the just-stored 2 -> li $v1,2 */
    }
}
if (v3 == 2) goto ...;
```

GCC's CSE forward-propagates the constant it just stored, emitting `addiu $v1,$zero,2`
(`li 2`) where the target re-loads from memory (`lbu $v1, GLOBAL`).

## Cause

Within a basic block GCC tracks the value just stored to `GLOBAL` and substitutes it
for the immediately following read — so `GLOBAL = 2; v3 = GLOBAL;` collapses to `v3 = 2`.
The target was compiled from source that **never saved the reload into a local**: it
re-reads the global at the *later* test, and GCC's available-expression analysis (not a
literal store-forward) decides per-path whether a reload is needed.

## Fix — drop the saved-local reload; re-read the global at the comparison

Delete the `v3 = GLOBAL;` reload line and make the later test read the **global directly**:

```c
s32 v3 = GLOBAL;            /* entry load, kept in $v1 */
if (v3 == 1) {
    if (flagsA || flagsB) {
        GLOBAL = 2;         /* sb — KILLS the CSE entry for GLOBAL */
    }
}
if (GLOBAL == 2) goto ...;  /* re-read */
```

Now GCC does exactly what the target does:

- **No-store path** — nothing wrote `GLOBAL` since the entry load, so the available
  expression is still live; GCC **reuses the entry register** (`$v1`) for the test.
  No reload (matches target's fall-through `beq $v1,$v0`).
- **Store path** — the `sb` kills the cached value, so GCC inserts a **reload**
  (`lui;lbu $v1, GLOBAL`) at the end of the store block — exactly where the target has it.

One value, one register, reload only where the store forces it. The regfix `subst`
becomes unnecessary; `retire` drops it and full SHA1 == oracle.

## Do NOT use `volatile` for this

`v3 = *(volatile u8 *)&GLOBAL;` *does* force a reload, but it is the wrong lever and
makes the function **worse** (+3 insns, measured on func_8001E404): the volatile
pointer access materializes the full address (`lui;addiu` instead of `lui;lbu %lo`),
blocks delay-slot filling (a stray `nop`), and re-masks the byte (`andi $r,$r,0xff`).
The target keeps the global in one register and reloads via plain `lui;lbu %lo` — only
the CSE-kill approach above reproduces that.

## Confirmed case — func_8001E404 (code6cac.c, 2026-05-26)

Queue top, verdict C, pure-C distance 2, one regfix `subst "addiu\s+$3,$zero,2"
"lbu\t$3,D_800A36FA" @ 20`. The body saved `v3 = D_800A36FA;` after `D_800A36FA = 2;`
and tested `if (v3 == 2)`. Removing the reload line and testing `if (D_800A36FA == 2)`
directly → `sandbox --disable all` 2→0; `retire` dropped the rule; SHA1 == oracle.
100% pure C. (The intermediate `volatile`-cast attempt scored 5 — see the warning above.)

## Related
- [[u16-global-lhu-lbu-low-byte]] — sibling single-`subst` global-read fork quirk; there
  the fix is reading the low byte, here it is *where* the read happens (CSE liveness)
- [[sandbox-zero-retire-fails]] — when a sandbox 0 hides a barrier that must be removed;
  contrast: here the sandbox 0 was honest once the source matched
- [[register-alloc-pure-c]] — global available-expression / liveness is the same lever
  family (one value, one register across paths)
