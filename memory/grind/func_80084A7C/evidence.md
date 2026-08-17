# Evidence bank — func_80084A7C

## Session 1 (recon, 2026-08-17)

### Baseline
- Honest floor at session start (`sandbox func_80084A7C --disable all`): **26**,
  `target_insns` 145, `build_insns` 146, 11 regfix rules dropped, 79 cheat-asm
  lines stripped. Floor at session end: **26** (unchanged score, but a whole
  diff axis eliminated — see below).
- `canonical` verdict is **C** (from the queue record); this is a pure-C grind,
  not an asm-routing candidate.

### The function (from `asm/funcs/func_80084A7C.s`, 145 insns)
Motion/animation state advance handler. Signature in `src/main.c` is
`void func_80084A7C(s16 a0, s16 a1)`; three call sites in the same file
(`main.c:451,503,547`) cast it to a 4-arg pointer type
`(void (*)(s16, s16, u8, u8 *))` because it is one slot of the
`D_800F3340..3350` per-status-byte dispatch table — the extra args are ignored
by this entry. That prototype contradiction is pre-existing and is NOT part of
the residual (the extra args never reach a register in the body).

Structure: `base_ptr = &((s32 *)&D_80106F28)[a0]`, `offset = (s16)a1 * 0xB0`
(strength-reduced by GCC to `x*2 +x <<2 -x <<4`), `base = (u8 *)(*base_ptr +
offset)` held in `$s0` for the whole body. `a0`/`a1` are parked in `$s2`/`$s1`
for the post-call `spu_NotifyChannel((s16)(a0 | (a1 << 8)))` recomputation.
Every access to the flag word at `+0x98` **re-loads** `*base_ptr` (`lw 0($a3)`
in target) — GCC cannot CSE it because the preceding stores through
`base`/`base+0x98` may alias the table entry. That re-load-per-site shape is
already reproduced correctly by our C.

### Tooling built this session
`tmp/grind/func_80084A7C/s1/diffit.py` — disassembles the *honest* sandbox
object (`tmp/sandbox/func_80084A7C/main.o`, i.e. rules-disabled/cheat-stripped)
and normalizes both it and `asm/funcs/func_80084A7C.s` into one canonical token
stream (objdump `v0`/decimal-offset/numeric-branch syntax vs splat
`$v0`/hex/`.L` syntax; `li`→`addiu`, `move`→`addu`, `nop`→`sll`; branch and
call targets masked). Emits a difflib opcode diff. This is what turned a
26-point scalar into the exact residual below; **re-use it, do not rebuild it.**

### The residual, exactly (after this session's edit)
The ENTIRE remaining difference is one register-allocation decision plus its
one-instruction consequence:

1. **Two-register swap.** Target keeps `base_ptr` in `$a3` and `offset` in
   `$a2`. Our build keeps `base_ptr` in `$a2` and `offset` in `$a3`/`$a1`.
   Every one of the ~18 mismatching instructions is a `lw $vX, 0($a2)` where
   target has `0($a3)`, or an `addu $vX, $a1, $vX` where target has
   `addu $vX, $a2, $vX`. Same opcode, same operand order, same immediates.
2. **One extra instruction (146 vs 145): `move $a1, $a3`** in the entry block,
   emitted between `lw $v1, 0($a2)` and `addu $s0, $v1, $a3`. It is a **failed
   coalesce**: the `*0xB0` multiply chain's final `sll` writes `$a3` (a
   block-local quantity that local-alloc assigns first), while the `offset`
   *global* allocno — live across the whole body for the eight `+0x98` sites —
   was given `$a1` by global-alloc, so a copy joins them. `$a1` is available to
   global-alloc precisely because our build sinks `move $s1, $a1` early (our
   prologue order is `sw s1 / move s1,a1 / sw ra / sw s0 / lw`, target's is
   `sw ra / sw s1 / sw s0 / lw / move s1,a1` — exactly what the inherited
   `reorder 16,14,17,18,15` regfix rule was papering over). Target's `offset`
   allocno gets `$a2` and coalesces with the multiply chain's destination, so
   no copy and 145 insns.

Read that as one causal chain: **the parameter copy `s1 = a1` retiring `$a1`
early is what lets global-alloc put `offset` in `$a1`, which both costs the
copy and pushes the `$a2`/`$a3` pair into the swapped assignment.**

### The 11 inherited regfix rules confirm this reading
`regfix.txt:719-742` for this function is exactly: `$6 <-> $7` (the a2/a3 swap),
six `subst` rules fixing the `addu` operand order after the swap, one
`delete @ 19` (the extra `move`), and one prologue `reorder`. The rule set is a
mechanical description of the residual above and nothing else — there is no
hidden second problem lurking behind the cheats.

### Measured this session
| Form | sandbox `--disable all` | insns |
|---|---|---|
| inherited HEAD (`*base_ptr + offset + 0x98`) | 26 | 146 |
| **offset-first flag expressions (`offset + *base_ptr + 0x98`)** | **26** | 146 |
| + `shifted` intermediate inlined into `base_ptr` | 26 | 146 |
| `offset` declared before `base_ptr` (with `shifted`) | 32 | 146 |
| `offset` declared before `base_ptr` (no `shifted`) | 33 | 146 |

The offset-first flip is score-neutral but strictly better structurally: before
it, the eight flag-site `addu`s differed in BOTH operand order and register;
after it they differ only in register. It is the form saved in `candidate.c`.

- [s1] Honest floor (sandbox func_80084A7C --disable all) is 26, target_insns 145, build_insns 146, 11 regfix rules dropped, 79 cheat-asm lines stripped. Floor unchanged 26 -> 26 this session; the gain was structural, not scalar.

- [s1] Built tmp/grind/func_80084A7C/s1/diffit.py: disassembles the honest (rules-disabled, cheat-stripped) sandbox object and normalizes objdump syntax and splat asm syntax into one token stream (li->addiu, move->addu, nop->sll, hex/decimal offsets unified, branch and call targets masked), then difflib-diffs them. This is what converted the 26-point scalar into the exact residual. Re-use it; do not rebuild it.

- [s1] THE ENTIRE RESIDUAL IS ONE ALLOCATION DECISION AND ITS CONSEQUENCE: (1) target holds base_ptr in $a3 and offset in $a2, our build holds base_ptr in $a2 and offset in $a3/$a1 — every one of the ~18 mismatching instructions is 'lw $vX,0($a2)' where target has '0($a3)' or 'addu $vX,$a1,$vX' where target has 'addu $vX,$a2,$vX', same opcode, same operand order, same immediates; (2) the 146th instruction is 'move $a1,$a3' in the entry block, a failed coalesce between the *0xB0 multiply chain's final sll destination ($a3, a block-local quantity local-alloc assigns first) and the 'offset' GLOBAL allocno that global-alloc placed in $a1.

- [s1] The causal chain: our build sinks 'move $s1,$a1' early (our prologue order is sw s1 / move s1,a1 / sw ra / sw s0 / lw; target's is sw ra / sw s1 / sw s0 / lw / move s1,a1), which retires $a1 early and makes it available to global-alloc for the 'offset' allocno; that both costs the join copy and pushes the $a2/$a3 pair into the swapped assignment. Attack the chain, not the register names.

- [s1] The 11 inherited regfix rules (regfix.txt:719-742) are a mechanical restatement of exactly this residual and nothing else: '$6 <-> $7' (the a2/a3 swap), six 'subst' rules repairing addu operand order after the swap, 'delete @ 19' (the extra move), and one prologue 'reorder 16,14,17,18,15' (which is precisely the move-s1-a1 placement difference). There is no second hidden problem behind the cheats.

- [s1] Function shape: motion/animation state advance. base_ptr = &((s32*)&D_80106F28)[a0]; offset = (s16)a1 * 0xB0 (GCC strength-reduces to x*2 +x <<2 -x <<4); base = (u8*)(*base_ptr + offset) held in $s0 body-wide; a0/a1 parked in $s2/$s1 for the post-call spu_NotifyChannel((s16)(a0 | (a1 << 8))) recomputation. Target RE-LOADS *base_ptr at every +0x98 flag site ('lw 0($a3)') because the preceding stores through base may alias the table entry — our C already reproduces that re-load-per-site shape correctly, so it is NOT part of the gap.

- [s1] The declared signature 'void func_80084A7C(s16 a0, s16 a1)' is contradicted by three call sites in the same file (main.c:451,503,547) that cast it to (void (*)(s16,s16,u8,u8*)) because it is one slot of the D_800F3340..3350 dispatch table. This is pre-existing and NOT part of the residual — the extra args never reach a register in the body. Do not spend a session on it.

- [s1] SCORE IS A MISLEADING GRADIENT ON THIS FUNCTION: two separate probes (H1 confirmed, H4 killed) both left the score at exactly 26 while changing the emitted code materially in opposite directions. Future sessions must judge probes with diffit.py, not with the sandbox scalar alone.
