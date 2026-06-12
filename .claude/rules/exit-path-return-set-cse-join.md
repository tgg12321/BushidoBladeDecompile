---
name: exit-path-return-set-cse-join
description: Set the return value in EACH exit path (not at the shared label) so cse can't canonicalize the join's op back to the callee-save — the join op then reads $v0 and cross-jump re-merges the copies
paths: [".claude/rules/exit-path-return-set-cse-join.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
---

# Set the return value in EACH exit path (not at the shared label) so the join's op reads $v0

## Symptom

A function ends with a shared finish label where a copy of a callee-save into
the return register is followed by an op consuming it:

```c
done:
    ret = (s32)s2;          /* move v0,s2 */
    v1 = ret - (s32)s4;     /* you WANT: subu v1,v0,s4 */
    *s4 = s6;
    *s5 = v1;
    return ret;
```

Target asm at the label: `move v0,s2; subu v1,v0,s4; sw; sw` — the subu reads
**$v0**. Your build instead emits `subu v1,s2,s4` (reads **$s2**) and the
scheduler floats the subu ABOVE the move (the cse substitution removed the
dependence, and the subu's longer downstream chain wins the scheduling tie).
A 2-rule regfix `reorder`/`subst` cluster papers over the 2-insn swap. A prior
agent may have "fixed" it with a dead `ret++; ret--;` pair on the
uninitialized local — that is a forbidden coercion (dead ops, zero semantic
purpose), not a match.

## Mechanism (cse block boundaries vs. pass order)

With the copy INSIDE the multi-predecessor label block, cse processes
`[move v0,s2; subu ret-s4]` in one cse basic block: the move puts v0 in s2's
quantity class, and `canon_reg` rewrites the subu's operand to the class head
(**s2** — the older register). The subu no longer depends on the move, and
sched1 reorders them.

If instead EVERY exit path assigns the return variable BEFORE the join:

```c
    if (v1 == -2) {
        ret = (s32)s2;      /* early path sets the return value */
        goto done;
    }
    ... loop ...
    ret = (s32)s2;          /* fall-through path sets it too */
done:
    v1 = ret - (s32)s4;     /* subu reads ret's pseudo — no copy in this block */
    *s4 = s6;
    *s5 = v1;
    return ret;
```

then at cse time the copies live in the PREDECESSOR blocks; the multi-pred
`done:` label starts a fresh cse block with an empty table, so the subu
genuinely reads `ret`'s pseudo (allocated $v0 — the return value). Later,
jump2's cross-jump finds the two identical `[move v0,s2; (jump)]` suffixes,
merges them into ONE move physically in front of the label, and retargets the
early conditional branch straight at the merged move. Emitted bytes:
`beq ...,<move>; ... ; move v0,s2; subu v1,v0,s4; ...` — exactly the
single-copy-at-label layout you started from, but with the subu reading $v0
and scheduled after the move (true dependence).

Net: the source spells the copy per-path; the binary still contains one copy.
cse pass order (cse → ... → jump2 cross_jump) is what makes the two spellings
emit different registers from identical final layouts.

## Why this is not a "cheats by any spelling" violation

Each `ret = (s32)s2;` is the function's REAL return value being set on a real
exit path — callers consume it in $v0. "Each exit path sets the result, then
jumps to the shared cleanup" is idiomatic C (the mixed-exit-forms family,
SOTN-accepted per [[no-new-park-categories]] / [[cross-jump-store-tail-merge]]).
Contrast [[goto-end-prologue-delay-slot]] (forbidden): there the shared label
had ZERO real work and the accumulator existed only to bend reorg.c. Here
`done:` holds three real statements of shared finish work, and the per-path
assignment is a faithful spelling of "remember the final dest, then finish".

## When this applies

1. Target's shared-end block has `move vN,sM` FIRST with the following op
   reading `vN`; your build reads `sM` directly and/or swaps the order.
2. The function has 2+ exit paths converging on the shared end.
3. The copied value is genuinely the return value (or otherwise used after
   the join) — you are relocating a real assignment, not inventing one.

Move the assignment out of the label block into each predecessor path.
Verify cross-jump re-merges the copies (insn count must NOT grow) via
sandbox, then full SHA1.

## Confirmed case — hirahira_w_frie (text1a_c.c, 2026-06-10)

Queue top, verdict C, distance 6, 4 regfix rules, plus a dead `ret++; ret--;`
coercion in HEAD (removed). Three independent levers closed it:
- prologue cluster: deleted a mangling `tools/prologue_config.json` entry per
  [[prologue-fix-redundant-reorder]] (cc1 raw was already target-ordered).
- `s0 = ((u32)s0 >> 2) << 2;` → two statements (`s0 = (u32)s0 >> 2;
  s0 = s0 << 2;`) so the srl/sll pair stays in-place in $s0 instead of
  routing through $v0. (`s0 &= ~3;` measured WORSE: GCC materializes the
  mask, +2 insns.)
- tail cluster: this rule's dual-exit return-set (+ a block-scoped
  `s32 stop = -2;` sentinel local declared before `s1 = s5;` to source-order
  the preheader's `li s7,-2` ahead of `move s1,s5`).

Also note the sandbox gotcha hit en route: `sandbox --disable all` (with
cheat-asm strip) was UNSCORABLE for this file — the file-wide strip shrinks
sibling `efc_rob_set_type_flash`'s frame (its own unused `pad[4]` + pins),
and that sibling's idx-0/1 frame-size content-swap regfix rules then misfire
into invalid asm (`lh $sp,$sp,88`). Workaround when the TARGET function
itself carries zero cheat-asm: score with `--keep-cheat-asm` (rules still
disabled), which is then the honest distance.

Sandbox 6 → 4 → 2 → 0; retire dropped all 4 rules; SHA1 == oracle;
COMPLETED-C. Cheat-reviewer PASS (2026-06-10).

## Related

- [[cross-jump-store-tail-merge]] — the family policy precedent (mixed exit
  forms); there the goal is to DEFEAT the merge, here the merge is leveraged
  to collapse the per-path copies back to one.
- [[goto-end-prologue-delay-slot]] — the FORBIDDEN sibling shape (shared
  label with no real work + synthetic accumulator); read it to keep the
  line straight.
- [[prologue-fix-redundant-reorder]] — companion lever used in the same
  confirmed case (config-stage circular debt).
- [[shared-end-label]] — the inverse constant-fold problem (you ADD a shared
  end so per-case values can't fold); this rule MOVES an assignment out of
  the shared end so cse can't canonicalize it away.
