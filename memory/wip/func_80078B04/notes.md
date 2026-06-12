# func_80078B04 — WIP checkpoint (2026-06-12)

## TL;DR

Floor lowered **7 → 2** (5-insn improvement) by introducing a `u16 *p`
named-intermediate that splits the address expression from the dereference.
Cheat-reviewer PASS. Remaining 2 diffs are a coupled `addu`+`lhu` register-
direction tiebreak that none of 36 explored variants flipped. Resume by
applying `candidate.c`, confirming `sandbox --disable all == 2`, and
attacking the addu direction.

## Target asm (14 insns)

```
andi $v1, $a0, 0xFFFF
slti $v0, $v1, 3
beqz $v0, .L80078B30
 sll $v1, $v1, 4
lui  $v0, %hi(D_8009BD6C)
lw   $v0, %lo(D_8009BD6C)($v0)
nop
addu $v1, $v1, $v0           ; <-- candidate emits addu $v0,$v0,$v1
lhu  $v0, 0($v1)             ; <-- candidate emits lhu  $v0,0($v0)
j .L80078B34
 nop
.L80078B30:
addu $v0, $zero, $zero
.L80078B34:
jr $ra
 nop
```

## Mechanism (from `cc1 -da` greg dumps)

HEAD body pseudo 73 (`v`): `preferences: [4]` — allocator pins to $a0,
emits `andi $a0,$a0,0xFFFF` in-place. 7 masked diffs against target.

Candidate body adds `u16 *p;` declared in the function scope and assigned
inside the if-true branch. Pseudo 73 (`v`): `preferences: [3 4]` — $v1
takes priority; emits `andi $v1,$a0,0xFFFF` matching target.

`p` (pseudos 75 + 76) gets `preferences: [2]` — both load (lw) and addu
output land in $v0. The lhu reuses base==dest because the loaded value
goes to result which is also $v0; allocator collapses to one reg. That
collapse IS the remaining 2-insn diff: target keeps p's reg ($v1)
distinct from the lhu dest ($v0).

## What's been ruled out (do NOT re-derive)

- All `& 0xFFFF` order swaps, `(u16)arg0` casts, `u16 v` decls — score 7.
- Early-return form `if (v >= 3) return 0;` — distance 12 (extra `j`).
- `result = 0;` init outside if — distance 12.
- `s32 ofs = v*16; ofs += D_8009BD6C; result = *(u16*)ofs;` (and ptr
  variants) — score 7. The intermediate folds away post-combine.
- `v <<= 4; v += D_8009BD6C; result = *(u16 *)v;` — score 5 (in-place
  accumulate is right, but mask stays in $a0).
- Hoisting `p = (u16 *)D_8009BD6C` outside the if — score 7 (lw moves
  out of if-true branch, structural mismatch).
- Register-asm pin / single-insn `__asm__` with hardcoded $N — would be
  a forbidden cheat (inline-asm-policy.md, inline-asm-injection.md).

## What worked

`u16 *p; ... p = (u16 *)(D_8009BD6C + v * 0x10); result = *p;` — score 2,
build_insns = target_insns = 14, cheat-reviewer PASS.

## Next levers (in order of measured/plausibility)

See `meta.json` → `next_hypotheses` (kept under the compaction cap).
Short form: try byte-offset pointer arithmetic spellings; cross-reference
the file's matched siblings for an addu+lhu shape using distinct regs;
probe with instrumented cc1 (BB2_ALLOC_DEBUG / BB2_PRIO_DEBUG); explore
strict-aliasing breaks via struct-typed pointer.

## Files

- `tmp/func_80078B04_variants/` — 36 explored variants (v0–v36); winners
  (score 2): v11, v25, v28, v29, v30, v31, v33, v36 — all share the
  named-intermediate-`p` shape.
- `tmp/func_80078B04_standalone.i.greg` — HEAD greg dump (pseudo 73 prefs `[4]`).
- `tmp/v11_full.i.greg` — candidate greg dump (pseudo 73 prefs `[3 4]`).
- `tmp/v11_full.s` — candidate cc1 output (showing the 2 residual diffs).
