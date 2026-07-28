# Evidence bank — func_80037AA4

- WIP rejected_form: no-dummy pin-free (16; absent 8-byte frame costs 2 sp-adjust insns)

- WIP rejected_form: decl reorder var_a0 first (16; swap+frame unchanged)

- == imported from memory/wip notes.md ==
# func_80037AA4 — WIP/BLOCKED (sum-and-scale over D_80102810 entries)

## TL;DR
HEAD "matches" via THREE `register asm()` pins (a2/a0/v0) PLUS an unused
`s32 sp_dummy[2]` frame-padding array. Two coupled blockers prevent a clean
pure-C close:

1. **8-byte unused stack frame.** The target has `addiu sp,sp,-8` / `+8` with
   NO sw/lw to sp (no spill, nothing stored). A leaf function with no
   used locals gets NO frame in GCC 2.7.2, so the original had a real stack
   local whose content is UNRECOVERABLE from the pure logic. The only way to
   reproduce the frame is a forbidden coercion (`sp_dummy[2]` unused array /
   address-taken / volatile) -- the dead-vars-local-array cheat
   ([[register-alloc-pure-c]] Lever D, FORBIDDEN). candidate.c (no dummy)
   produces NO frame -> -2 insns vs target.
2. **a0<->v1 register rename.** Target: accumulator var_a0 -> a0, pointer
   var_v1 -> v1. Pin-free GCC swaps them (accumulator->v1, pointer->a0). Same
   tied-priority-rename class as func_80037A20 / func_80044098.

## Measured
- HEAD honest distance (pins+dummy stripped): 14.
- candidate.c (no pins, NO dummy): 16 diffs -- WORSE, because the absent frame
  costs the 2 sp-adjust insns the dummy was supplying. Floor NOT lowered.
- decl reorder (v2): 16, swap+frame unchanged.

## Why blocked (policy, not just difficulty)
The 8-byte frame has no non-cheat pure-C reconstruction: the original stack
local's type/content is unknown, and any unused-array/address-of/volatile form
that forces the frame is a forbidden coercion. This needs either (a) recovery of
what the real stack local was (data-flow / sibling-function evidence), or (b) a
user policy call on reconstructing an unused frame. PLUS the a0<->v1 rename.

## Avenues for next session
- Investigate whether a sibling/caller reveals the original 8-byte local's
  purpose (then it's a legitimate named local, not a cheat).
- If the frame is genuinely an artifact with no real local, escalate as a
  policy question (reconstruct-unused-frame) -- not worker-closable.
- decomp-permuter for the a0<->v1 rename once the frame question is resolved.

## Floor
- HEAD: 14 (3 pins + sp_dummy[2] frame cheat). candidate.c: 16 (cheat-free, but
  short the frame). Not lowered; blocked on the frame-reconstruction policy +
  tied register rename.


## s1 (recon, 2026-07-28) — floor 14 -> 4; both WIP "blockers" dissolved

- [s1] **FLOOR 14 -> 4 with one pure-C edit.** The a0<->v1 rename is a plain
  decl-order lever: declare the POINTER first (`s8 *var_v1;` before the s32
  decls) in the guarded do-while form. WIP's killed probe had moved var_a0
  first (inert); var_v1-first was never measured. sandbox --disable all = 4
  with zero pins, zero dummy, edits in src. Remaining 4 = ONLY the missing
  8-byte frame (build 20 insns vs target 23).
- [s1] **The 8-byte frame is NOT a cheat-only construct** (WIP claim FALSE, per
  [[phantom-frame-slots-gcc272]]). Measured trigger for THIS function: any
  loop whose trip count is not provably >=1 (unguarded entry-test while/for)
  gets a phantom stack temp -> cc1 `vars= 8`, zero stores, zero dead decls.
  Guarded forms (if(n>0)+do-while / +while) always give vars=0. Instrument:
  cc1 .frame comment (frameprobe.sh / sweep*.py in tmp/grind/func_80037AA4/s1/).
- [s1] **Unguarded indexed form reproduces ALL target structure**: `for (i=0;
  i<n; i++) sum += ((CamEnt*)&D_80102810)[i].unk18;` with 0x28-size struct ->
  strength-reduced walking pointer, la AFTER blez (giv init in preheader
  between duplicated exit test and loop), lw 24(p), addu p,p,40, vars=8,
  21 cc1 insns == target. Byte-arith spelling `+ i*0x28 + 0x18` folds the
  +24 into the la (giv base D_80102810+24, lw 0(p)) — struct field spelling
  is required for the unfolded base.
- [s1] **Target tail is provably the T0 spelling** (`v0=sum; if(sum<0)
  v0=sum+0x1FFF; sum=v0>>13; return 0xF-sum;`): move $2,$4 in bgez delay slot,
  addu $2,$4,8191 (reads sum), sra $4,$2,13 (quotient coalesced into sum's
  pseudo), subu $2,$2,$4.
- [s1] **The one remaining coupling**: on the unguarded indexed base, T0's 4
  tail refs on sum make global-alloc allocate sum before the strength-
  reduction giv -> sum=$3/p=$4 (swapped). Decl order is INERT there (pointer
  is a compiler giv, not a user var — all 24 perms identical). Measured-inert
  flip levers: init orders, while/for/postinc spellings, plain-add, u8-arith,
  fresh-quotient var (coalesces the tail move away), quotient-into-i. The only
  measured flip (in-place tail, sum refs 2) breaks the tail (v0 -> $3, standalone
  move, in-place adds; score 6). Score map: guarded+pfirst=4, indexed+inplace=6,
  indexed+T0=11, old pinned honest=14, clean-unflipped=16.
- [s1] Division spelling `0xF - sum/0x2000` (semantically identical) is
  measured WORSE: perturbs loop schedule (i++ hoists above lw -> nop) and
  emits the in-place division tail. See rejected/division-tail-breaks-loop-sched.c.
- [s1] No sibling/duplicate exists (tmp/duplicates.txt full pairwise list: no
  80037AA4 entry; neighbors func_80037A20/func_80037B00 are themselves
  pin-carrying INCOMPLETE).

- [s1] Floor 4 in src NOW: guarded do-while pointer-walk with s8 *var_v1 declared first; residual = missing addiu sp,-8/+8 only (build 20 vs target 23 insns)

- [s1] cc1 .frame vars= is a direct frame gradient: pinned+dummy form vars=8 regs exact; clean guarded forms vars=0

- [s1] vars=8 trigger law (this function): loop trip count not provably >=1 => phantom stack temp, zero stores; if(n>0) guard kills it

- [s1] Unguarded for + 0x28-struct indexing strength-reduces to the exact target loop (la after blez, lw 24(p), addu p,p,40, 21 insns, vars=8); byte-arith spelling folds +24 into la (wrong)

- [s1] Target tail == T0 spelling: move $2,$4 in bgez delay, addu $2,$4,8191, sra $4,$2,13, subu $2,$2,$4

- [s1] Score map: guarded+pfirst=4, indexed+inplace=6, indexed+T0=11, old pinned honest=14, clean unflipped=16

- [s1] No duplicate/sibling lead (tmp/duplicates.txt has no 80037AA4 pair; both neighbors are pin-carrying INCOMPLETE)
