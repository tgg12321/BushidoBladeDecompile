# kgm_clamp_patch_init (src/ings.c) — WIP

## TL;DR
HEAD carries a HEAVY cheat load: 4 register-asm pins (i=$a0, c=$a1, tmp=$v0,
p=$v1) + 1 `addu %0,%1,$zero` asm-move (tmp->p) + 3 inline volatile casts
(`((volatile u8*)p)[8]=0` etc.). 2026-06-14 finding: the volatile casts AND the
asm-move are UNNECESSARY for the store shape — GCC 2.7.2 emits separate `sb`
stores for `p[8]=0; p[9]=0;` on its own (does NOT combine into `sh`), and the
clean form reaches the target instruction count (31). The ONLY residual is pure
register-allocation + induction-variable base. Apply `candidate.c` to resume.

## The residual gap (full-build SHA1 mismatch; masked sandbox is misleadingly 0)
Target:
```
addiu v0,v0,27216 ; p base
move  v1,v0       ; p in $v1
...
sb zero,8(v1)     ; loop based at p=$v1, offset 8
sb zero,9(v1)     ; offset 9
sw a1,12(v1)      ; c=$a1, offset 0xC
addiu a0,a0,1     ; i=$a0
addiu v1,v1,8     ; p += 8
```
Mine (clean candidate):
```
addiu a0,v1,12    ; GCC hoists induction var to (p+0xC) in $a0
...
sb zero,-4(a0)    ; offset -4
sb zero,-3(a0)    ; offset -3
sw a2,0(a0)       ; c=$a2 (wrong: target $a1), offset 0
addiu a1,a1,1     ; i=$a1 (wrong: target $a0)
addiu a0,a0,8
```
Two diffs: (1) c -> $a2 not $a1; (2) induction var hoisted to p+0xC (offsets
-4,-3,0) instead of staying at p (offsets 8,9,0xC). The 4 pins + addu-move were
doing exactly this RA/base work.

## Tried (all keep the residual; see meta.json rejected_forms)
- clean.c, c_first_decl.c (decl order), word_first.c, idx_loop.c. Declaration
  order moved the MASKED score to 0 but NOT the full-build codegen — masked-0
  caveat in action; always confirm with full `make`.

## Next session
Resume from `candidate.c` (clean, 0 cheats). Goal: a legitimate Lever A/B/C that
(a) lands c in $a1 and (b) keeps the loop induction based at p (offsets 8,9,0xC),
WITHOUT restoring the volatile casts / asm-move / pins. Best leads: struct-of-
bytes indexing that keeps the base at p, or an instrumented-cc1 `-da .greg` probe
to see why the induction var strength-reduces to p+0xC and why c picks $a2.
Do NOT re-add the volatile/asm/pin cheats — they are the thing being retired.
```
```
## Related
- [[register-alloc-pure-c]] — Lever A/B/C playbook for the residual RA work.
- [[inline-asm-policy]] — the HEAD pins + asm-move are cheat-asm; INCOMPLETE.
