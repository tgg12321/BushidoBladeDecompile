# exec_game — WIP checkpoint (2026-08-05, session 1)

**State: honest pure-C distance 121 (unchanged). One round run, one measured kill.**
Tree at HEAD. 104 regfix rules, canonical ASM-SUSPECT, zero cheat-asm, no stack frame
(`.frame $sp,0,$31 # vars=0 regs=0/0` on both sides — there is no frame lever here).

## What the six `subst_multi` rules actually repair (hand-read)

Each replaces a register-held value with a **reload of a global plus a nop**:

| regfix line | current codegen | rule substitutes |
|---|---|---|
| 3453 @49 | `slt $2,$14,$9` | `lw $2,D_800A2D3C` / `nop` / `slt $2,$2,$9` |
| 3466 @59 | `ori $3,$3,0xffff` | `lw $3,D_800A2D40` / `nop` |
| 3472 @69 | `lw $11,D_800A2D3C` | `lw $3,D_800A2D3C` / `nop` |
| 3494 @116 | `slt $2,$25,$9` | `lw $3,D_800A2D3C` / `addu $t1,$t1,1` / `slt $2,$3,$9` |
| 3509 @158 | `lui $7,0x4000` | `lw $5,D_800A2D40` / `nop` |
| 3544 @144 | `lw $11,D_800A2D3C` | `lw $a1,D_800A2D3C` / `addu $t1,$t1,1` |

So target **re-reads `D_800A2D3C` (the count) and `D_800A2D40` (the base pointer) at
every loop condition**, where our build keeps them in registers. Two of the rules also
re-insert a lost `addu $t1,$t1,1` counter increment.

## Round 1 — direct-symbol indexing (MEASURED KILL, 121 -> 127)

Applied the [[split-read-defeats-hoist]] lever #2: replaced the cached
`base = (Entry *)D_800A2D40;` in all five phases with direct
`(Entry *)D_800A2D40 + j` / `((Entry *)D_800A2D40)[cnt]` at each use site.

**GCC CSE'd them straight back into one hoisted base register** (`$t0`), and the extra
address arithmetic cost 5 instructions (199 vs target's 194). Score 121 -> 127. Reverted.

Conclusion: direct symbol indexing does not defeat the hoist for a `s32` global holding
a pointer that is loaded once and never written in the function. The rule's mechanism
(pinning offset computations inside branch arms) does not transfer to a loop-invariant
base.

## The real blocker, located

The Phase-1 diff at insn 48-51 is the model case:

```
T: lui v0,%hi(D_800A2D3C) / lw v0,%lo(D_800A2D3C) / nop / slt v0,v0,t1
O: slt v0,t6,t1                     ($t6 = move t6,v0 at insn 12)
```

Our source **already** writes the direct global read in both places —
`if (D_800A2D3C >= 0)` as the guard and `while (D_800A2D3C >= i)` as the loop
condition — and GCC CSEs the two into one register held across the whole loop.

The [[store-const-reload-cse]] recipe works by having a store **kill** the CSE entry.
exec_game's Phase-1 loop does store (`p->w0 = 0x2FFFFFFF; cur->w1 = ...`), but only on
the merge path, and GCC still hoists — it treats the global as loop-invariant rather
than reloading on the store path.

## Next avenues (untried)

1. Find why loop-invariant motion fires despite the conditional stores through
   `Entry *` — dump `.loop` for exec_game and check whether the read is recorded as an
   invariant or whether cse2 folded it before loop.c ran. That distinguishes "defeat
   LICM" from "defeat CSE", which need different spellings.
2. The two `addu $t1,$t1,1` re-insertions say target's loop keeps a counter our build
   folds away. Recovering that counter may be the same root cause as the reload (an
   extra live value forces the reload rather than a hoist).
3. The induction-variable axis is separate and still open: target walks a pointer
   (`addiu v1,v1,8`) where we index (`sll v0,a2,0x3`).

exec_game remains the lowest expected-payoff item in the heavy-12 per the triage
ranking; the 104 rules are attractive but the residual is genuinely structural.
